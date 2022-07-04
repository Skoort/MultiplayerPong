#include "Client.hpp"

#include <iostream>

#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Network/Packet.hpp>
#include <SFML/Network/IpAddress.hpp>

#include "GameRules.hpp"

Client::Client(unsigned short listeningPort, sf::IpAddress serverIp, unsigned short serverPortIn, unsigned short serverPortOut)
: Game{listeningPort, sf::Socket::AnyPort, "Client"}
, m_serverIp{serverIp}
, m_serverPortIn{serverPortIn}
, m_serverPortOut{serverPortOut}
{
    auto fullSize = GameRules::windowSize;
    auto halfSize = sf::Vector2f{fullSize} * 0.5F;
    
    // Opponents's Paddle
    m_gameObjects[0] = CreatePaddle(1, GameRules::shouldSwapPositions);
    
    // Players's Paddle
    m_gameObjects[1] = CreatePaddle(2, GameRules::shouldSwapPositions);
    
    // Ball
    m_gameObjects[2] = std::make_unique<GameObject>();
    m_gameObjects[2]->SetDimensions(50, 50);
    m_gameObjects[2]->SetOrigin(25, 25);
    ResetBall(*m_gameObjects[2], false);
    
    m_scoreText1 = CreateScoreText(1, GameRules::shouldSwapPositions);
    m_scoreText2 = CreateScoreText(2, GameRules::shouldSwapPositions);
    
    m_isAuthority = false;
}

void Client::UpdateWaiting(float dt)
{
    sf::Event event;
    while (m_window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            Close();
            return;
        }
    }
    
    if (m_elapsedTime >= m_packetTimer)
    {
        auto joinResponse = m_networkIn.GetPacket();
        if (joinResponse != nullptr && joinResponse->address.ip == m_serverIp && joinResponse->address.port == m_serverPortOut)
        {
            m_state = GameState::PLAYING;
        }
        else
        {
            auto joinRequest = std::make_unique<PacketInfo>();
            joinRequest->address.ip = m_serverIp;
            joinRequest->address.port = m_serverPortIn;
            joinRequest->body
                << sf::Uint8{0}                                 // Tells the server that this is a join request.
                << sf::Uint16{m_networkIn.GetListeningPort()};  // Tells the server what port the client is listening on.
            m_networkOut.QueuePacket(std::move(joinRequest));
        }
        
        m_packetTimer += (1 / GameRules::packetsPerSecond);
    }
}

void Client::UpdatePlaying(float dt)
{
    sf::Event event;
    while (m_window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            Close();
            return;
        } else
        if (event.type == sf::Event::GainedFocus)
        {
            GameRules::isFocused = true;
        } else
        if (event.type == sf::Event::LostFocus)
        {
            GameRules::isFocused = false;
        }
    }
    
    float input = 0;
    if (GameRules::isFocused)
    {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
        {
            --input;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
        {
            ++input;
        }
    }
    
    if (m_elapsedTime > m_packetTimer)
    {
        std::unique_ptr<PacketInfo> latestPacket = nullptr;
        std::unique_ptr<PacketInfo> tempPacket = nullptr;
        while ((tempPacket = m_networkIn.GetPacket()) != nullptr)
        {
            sf::Uint32 id;
            sf::Uint16 score1, score2;
            tempPacket->body >> id >> score1 >> score2;
            if (id > m_latestInboundId)
            {
                m_latestInboundId = id;
                latestPacket = std::move(tempPacket);  // We will eventually store a history of these for the last second to interpolate between.
                m_score1 = score1;
                m_scoreText1->setString(std::to_string(m_score1));
                m_score2 = score2;
                m_scoreText2->setString(std::to_string(m_score2));
            }
        }
        if (latestPacket)
        {
            while (!latestPacket->body.endOfPacket())
            {
                sf::Uint32 id;
                float x, y, dx, dy;
                latestPacket->body
                    >> id
                    >> x >> y >> dx >> dy;
                auto &gameObject = m_gameObjects[id];
                if (GameRules::shouldSwapPositions)
                {
                    x = GameRules::windowSize.x - x;
                    y = GameRules::windowSize.y - y;
                    dx *= -1;
                    dy *= -1;
                }
                gameObject->SetPosition(x, y);
                gameObject->SetVelocity(dx, dy);
            }
        }
        
        // Write to the outbound queue.
        auto packet = std::make_unique<PacketInfo>();
        packet->address.ip = m_serverIp;
        packet->address.port = m_serverPortIn;
        packet->body
            << sf::Uint8{1}                                        // Tells the server that this is a data packet (player's keyboard state).
            << ++m_latestOutboundId                                // The ID of this packet.
            << (GameRules::shouldSwapPositions ? -input : input);  // The player's input.
        m_networkOut.QueuePacket(std::move(packet));
        
        m_packetTimer += (1.0 / GameRules::packetsPerSecond);
    }
    
    // The player can move the paddle belonging to them.
    m_gameObjects[1]->SetVelocity(input * (GameRules::windowSize.x * 0.5), 0);
    
    if (m_ballSpawnProgress < 1)
    {
        m_ballSpawnProgress += dt * 1;
        if (m_ballSpawnProgress > 1)
        {
            m_ballSpawnProgress = 1;
        }
        m_gameObjects[2]->SetScale(m_ballSpawnProgress, m_ballSpawnProgress);
    }
    
    for (auto const &[id, gameObject] : m_gameObjects)
    {
        if (gameObject == m_gameObjects[2])
        {
            if (m_ballSpawnProgress >= 1)
            {
                gameObject->Move(dt);
            }
            HandleBallCollisions(gameObject);
        }
        else
        {
            gameObject->Move(dt);
        }
        gameObject->ClampPosition(0, GameRules::windowSize.x, -500, GameRules::windowSize.y + 500);
    }
    
    m_window.clear();
    for (auto &keyVal : m_gameObjects)
    {
        keyVal.second->Draw(m_window);
    }
    m_window.draw(*m_scoreText1);
    m_window.draw(*m_scoreText2);
    m_window.display();
}
