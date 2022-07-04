#ifndef Client_hpp
#define Client_hpp

#include <SFML/Network/IpAddress.hpp>

#include "Game.hpp"
#include "NetworkAddress.hpp"

class Client : public Game
{
    public:
        Client(unsigned short listeningPort, sf::IpAddress serverIp, unsigned short serverPortIn, unsigned short serverPortOut);
    
    protected:
        void UpdateWaiting(float dt) override;
        void UpdatePlaying(float dt) override;
    
    private:
        sf::IpAddress m_serverIp;
        unsigned short m_serverPortIn;
        unsigned short m_serverPortOut;
};

#endif /* Client_hpp */
