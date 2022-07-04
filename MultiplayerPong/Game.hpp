#ifndef Game_hpp
#define Game_hpp

#include <map>
#include <string>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>

#include "GameObject.hpp"
#include "GameState.hpp"
#include "InboundPacketReader.hpp"
#include "OutboundPacketWriter.hpp"

class Game
{
    public:
        Game(unsigned short portIn, unsigned short portOut, std::string windowName);
        virtual ~Game();
        
        void Play();
        void Close();
    
    private:
        void Update(float dt);
    
    protected:
        virtual void UpdateWaiting(float dt) = 0;
        virtual void UpdatePlaying(float dt) = 0;
    
        void HandleBallCollisions(std::unique_ptr<GameObject> const &ball);
    
        auto CreatePaddle(int playerId, bool shouldFlipPositions) const -> std::unique_ptr<GameObject>;
        auto CreateScoreText(int playerId, bool shouldSwapPositions) const -> std::unique_ptr<sf::Text>;
        void ResetBall(GameObject &ball, bool randomizeVelocity);
    
    protected:
        sf::RenderWindow m_window;
    
        sf::Font m_font;
        std::unique_ptr<sf::Text> m_scoreText1;
        std::unique_ptr<sf::Text> m_scoreText2;
    
        GameState m_state;
        std::map<sf::Uint32, std::unique_ptr<GameObject>> m_gameObjects;
    
        InboundPacketReader m_networkIn;
        OutboundPacketWriter m_networkOut;
        sf::Uint32 m_latestOutboundId;
        sf::Uint32 m_latestInboundId;
        float m_elapsedTime;
        float m_packetTimer;
    
        int m_score1;
        int m_score2;
        float m_ballSpawnProgress;
    
        bool m_isAuthority;
};

#endif /* Game_hpp */
