#include "ClientServer.hpp"

#include <iostream>
#include <stdlib.h>

#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Network/Packet.hpp>
#include <SFML/Network/IpAddress.hpp>

#include "GameRules.hpp"

ClientServer::ClientServer(unsigned short portIn, unsigned short portOut)
: Game{portIn, portOut, "Server"}
{
    auto fullSize = GameRules::windowSize;
    auto halfSize = sf::Vector2f{fullSize} * 0.5F;
    
    // Player's Paddle
    m_gameObjects[0] = CreatePaddle(1, false);
    // Opponent's Paddle
    m_gameObjects[1] = CreatePaddle(2, false);
    // Ball
    m_gameObjects[2] = std::make_unique<GameObject>();
    m_gameObjects[2]->SetDimensions(50, 50);
    m_gameObjects[2]->SetOrigin(25, 25);
    ResetBall(*m_gameObjects[2], true);
    
    m_scoreText1 = CreateScoreText(1, false);
    m_scoreText2 = CreateScoreText(2, false);
    
    m_isAuthority = true;
}

void ClientServer::UpdateWaiting(float dt)
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
    
    auto joinRequest = m_networkIn.GetPacket();
    if (joinRequest != nullptr)
    {
        sf::Uint8 messageType;
        sf::Uint16 clientsListeningPort;
        joinRequest->body
            >> messageType
            >> clientsListeningPort;
        
        m_remoteAddress = joinRequest->address;
        m_remoteAddress.port = clientsListeningPort;
        
        m_state = GameState::PLAYING;
    }
}

void ClientServer::UpdatePlaying(float dt)
{
    sf::Event event;
    while (m_window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            Close();
            return;
        }
        if (event.type == sf::Event::GainedFocus)
        {
            GameRules::isFocused = true;
        }
        if (event.type == sf::Event::LostFocus)
        {
            GameRules::isFocused = false;
        }
    }
    
    int input = 0;
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
            sf::Uint8 packetType;
            tempPacket->body >> packetType;
            if (packetType == 0)  // Ignore the previous state's packets.
            {
                continue;
            }
            sf::Uint32 id;
            tempPacket->body >> id;
            if (id > m_latestInboundId)
            {
                m_latestInboundId = id;
                latestPacket = std::move(tempPacket);  // We will eventually store a history of these for the last second to interpolate between.
            }
        }
        if (latestPacket && !latestPacket->body.endOfPacket())
        {
            float opponentInput;
            latestPacket->body >> opponentInput;
            
            m_gameObjects[1]->SetVelocity(opponentInput * (GameRules::windowSize.x * 0.5), 0);
        }
        
        // Write to the outbound queue.
        auto packet = std::make_unique<PacketInfo>();
        packet->address = m_remoteAddress;
        packet->body
            << ++m_latestOutboundId
            << static_cast<sf::Uint16>(m_score1) << static_cast<sf::Uint16>(m_score2);
        for (auto const &[id, gameObject] : m_gameObjects)
        {
            packet->body
                << id
                << gameObject->GetPosition().x << gameObject->GetPosition().y << gameObject->GetVelocity().x << gameObject->GetVelocity().y;
        }
        m_networkOut.QueuePacket(std::move(packet));
        
        m_packetTimer += 1.0 / GameRules::packetsPerSecond;
    }
    
    // The player can move the paddle belonging to them.
    m_gameObjects[0]->SetVelocity(input * (GameRules::windowSize.x * 0.5), 0);
    
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
