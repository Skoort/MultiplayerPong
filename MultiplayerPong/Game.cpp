#include "Game.hpp"

#include <iostream>
#include <stdlib.h>
#include <cmath>

#include <SFML/System/Clock.hpp>
#include <SFML/System/String.hpp>
#include <SFML/Window/VideoMode.hpp>

#include "GameRules.hpp"

Game::Game(unsigned short portIn, unsigned short portOut, std::string windowName)
: m_window{sf::VideoMode{GameRules::windowSize}, sf::String{windowName}}
, m_state{GameState::WAITING_FOR_SECOND_PLAYER}
, m_gameObjects{}
, m_networkIn{portIn}
, m_networkOut{portOut}
, m_font{}
, m_scoreText1{}
, m_scoreText2{}
{
    m_latestInboundId = 0;
    m_latestOutboundId = 0;
    m_elapsedTime = 0;
    m_packetTimer = 0;
    m_score1 = 0;
    m_score2 = 0;
    
    if (!m_font.loadFromFile("arial.ttf"))
    {
        std::cerr << "Failed to load font!" << std::endl;
    }
}

Game::~Game()
{
    Close();
}

void Game::Play()
{
    m_networkIn.Start();
    m_networkOut.Start();
    
    sf::Clock gameClock{};
    
    while (m_window.isOpen())
    {
        float dt = gameClock.restart().asSeconds();
        
        Update(dt);
        
        m_elapsedTime += dt;
    }
}

void Game::Close()
{
    // Consider first stopping them and then joining them. But that is relatively minor for now.
    m_window.close();
    m_networkIn.Stop();
    m_networkOut.Stop();
}

void Game::Update(float dt)
{
    switch (m_state)
    {
        case GameState::WAITING_FOR_SECOND_PLAYER:
        {
            UpdateWaiting(dt);
            break;
        }
        case GameState::PLAYING:
        {
            UpdatePlaying(dt);
            break;
        }
    }
}

void Game::HandleBallCollisions(std::unique_ptr<GameObject> const &ball)
{
    bool hitWall = false;
    bool hitPaddle = false;
//    bool wasHit = false;
    auto position = ball->GetPosition();
    auto velocity = ball->GetVelocity();
    auto newVelocity = velocity;
    
    
    for (auto const &[id, gameObject] : m_gameObjects)
    {
        if (gameObject == ball)
        {
            continue;
        }
        
        if (ball->Collides(*gameObject))
        {
            //auto fromTo = ball->GetCenter() - gameObject->GetCenter();
            auto fromTo = ball->GetPosition() - gameObject->GetPosition();
            //auto sameness = fromTo.dot(ball->GetVelocity());
            
            auto penetration = ball->GetCollisionPenetration(*gameObject);
            if (penetration.x >= penetration.y)
            {
                //if (sameness < 0)  // I don't think this sameness guarantees that the next time you touch it it won't set wasHit.
                if (fromTo.y * ball->GetVelocity().y < 0)
                {
                    hitPaddle = true;
//                    newVelocity.y *= -1;
                    // Bounce the ball back and if it is near the center it is more straight.
                    auto dirCenter = sf::Vector2f{0, -newVelocity.y / std::abs(newVelocity.y)};
                    auto dirEdge = sf::Vector2f{fromTo.x / std::abs(fromTo.x), dirCenter.y};
                    float dist = std::abs(fromTo.x) / (gameObject->GetDimensions().x * 0.5f);
                    if (dist > 1)
                    {
                        dist = 1;
                    }
                    newVelocity = (dirCenter * (1 - dist) + dirEdge * dist).normalized() * 300.0F;
                }
                auto sign = fromTo.y / std::abs(fromTo.y);
                ball->SetPosition(position.x, position.y + penetration.y * sign);
            }
            else
            {
                //if (sameness < 0)
                if (fromTo.x * ball->GetVelocity().x < 0)
                {
                    hitPaddle = true;
//                    wasHit = true;
                    newVelocity.x *= -1;
                }
                auto sign = fromTo.x / std::abs(fromTo.x);
                ball->SetPosition(position.x + penetration.x * sign, position.y);
                
                // If the paddle is moving towards the ball, it imparts the ball with some speed.
                if (gameObject->GetVelocity().length() > 0.01
                 && gameObject->GetVelocity().dot(fromTo) > 0
                 && std::abs(gameObject->GetVelocity().x) > std::abs(velocity.x))
                {
                    newVelocity.x = gameObject->GetVelocity().x;
                }
            }
        }
    }
    
    if ((position.x < ball->GetOrigin().x)
     || (position.x > GameRules::windowSize.x - (ball->GetDimensions().x - ball->GetOrigin().x)))
    {
        //wasHit = true;
        hitWall = true;
        newVelocity.x *= -1;
    }
//    if ((position.y < ball->GetOrigin().y)
//     || (position.y > GameRules::windowSize.y - (ball->GetDimensions().y - ball->GetOrigin().y)))
    if ((position.y < (0 - (ball->GetDimensions().x - ball->GetOrigin().y)))
     || (position.y > (GameRules::windowSize.y + ball->GetOrigin().y)))
    {
        //wasHit = true;
        hitWall = true;
        newVelocity.y *= -1;
        if (m_isAuthority)
        {
            if (position.y < GameRules::windowSize.y * 0.5F)
            {
                ++m_score1;
                m_scoreText1->setString(std::to_string(m_score1));
            } else
            if (position.y > GameRules::windowSize.y * 0.5F)
            {
                ++m_score2;
                m_scoreText2->setString(std::to_string(m_score2));
            }
        }
        ResetBall(*ball, m_isAuthority);
    }
    
    //if (wasHit)
    if (hitPaddle)
    {
        float angle = ((static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 5 - 2.5F) * M_PI / 180.0;
        float newX = newVelocity.x * std::cos(angle) - newVelocity.y * std::sin(angle);
        float newY = newVelocity.x * std::sin(angle) + newVelocity.y * std::cos(angle);
        newVelocity = sf::Vector2f{newX, newY};
    }
    
    ball->SetVelocity(newVelocity);
}

auto Game::CreatePaddle(int playerId, bool shouldFlipPositions) const -> std::unique_ptr<GameObject>
{
    auto paddle = std::make_unique<GameObject>();
    paddle->SetDimensions(100, 50);
    if ((playerId == 1 && shouldFlipPositions) || (playerId == 2 && !shouldFlipPositions))
    {
        paddle->SetOrigin(50, 0);
        paddle->SetPosition(GameRules::windowSize.x * 0.5F, 0);
    } else
    if ((playerId == 1 && !shouldFlipPositions) || (playerId == 2 && shouldFlipPositions))
    {
        paddle->SetOrigin(50, 50);
        paddle->SetPosition(GameRules::windowSize.x * 0.5F, GameRules::windowSize.y);
    }
    return paddle;
}

auto Game::CreateScoreText(int playerId, bool shouldFlipPositions) const -> std::unique_ptr<sf::Text>
{
    auto text = std::make_unique<sf::Text>();
    text->setFont(m_font);
    text->setString(std::to_string(m_score1));
    text->setCharacterSize(42); // in pixels
    sf::FloatRect textRect = text->getLocalBounds();
    text->setOrigin({textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f});
//    if (playerId == 1)
//    {
//        text->setFillColor(sf::Color{0, 0, 255});
//    } else
//    if (playerId == 2)
//    {
//        text->setFillColor(sf::Color{255, 0, 0});
//    }
    if ((playerId == 1 && shouldFlipPositions) || (playerId == 2 && !shouldFlipPositions))
    {
        text->setPosition(sf::Vector2f{50, GameRules::windowSize.y * 0.5F - 35});
        text->setFillColor(sf::Color{255, 0, 0});
    } else
    if ((playerId == 1 && !shouldFlipPositions) || (playerId == 2 && shouldFlipPositions))
    {
        text->setPosition(sf::Vector2f{50, GameRules::windowSize.y * 0.5F + 35});
        text->setFillColor(sf::Color{0, 0, 255});
    }
    return text;
}

void Game::ResetBall(GameObject &ball, bool randomizeVelocity)
{
    m_ballSpawnProgress = 0;
    ball.SetScale(0.1F, 0.1F);
    ball.SetPosition(GameRules::windowSize.x * 0.5F, GameRules::windowSize.y * 0.5F);
    ball.SetVelocity(0, 0);
    if (randomizeVelocity)
    {
        float x = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 2 - 1;
        float y = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 2 - 1;
        y += (y / std::abs(y)) * 1;  // Y is between [-2, -1] or [1, 2].
        auto velocity = sf::Vector2f(x, y).normalized() * 300.0F;
        ball.SetVelocity(velocity);
    }
}
