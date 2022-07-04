#ifndef Connection_hpp
#define Connection_hpp

#include <memory>

#include <SFML/System/Clock.hpp>
#include <SFML/Network/IpAddress.hpp>
#include <SFML/Network/Packet.hpp>

class Connection
{
    public:
        enum class Status
        {
            OPENING,  // When the connection is in the opening state. Should this use the general StateMachine system too?
            OPEN,     // When the connection is active.
            CLOSING,  // You probably don't want to send any packets to this Connection anymore.
            CLOSED    // This is set when the connection is removed from ConnectionManager. Is it necessary?
        };
    
        Connection(sf::IpAddress ipAddress, unsigned short port);
        ~Connection();
    
        void Send(std::unique_ptr<sf::Packet> packet);
        std::unique_ptr<sf::Packet> Receive();
    
        float RoundTripTime() const;
    
    private:
        sf::Clock m_clock;
    
        std::vector<sf::Packet> m_packets;  // These are inbound packets. Outbound packets are stored in ConnectionManager.
    
        sf::IpAddress m_ipAddress;
        unsigned short m_port;
    
        sf::Uint32 m_latestSentId;
        sf::Uint32 m_latestReceivedId;
    
        float m_timeSincePinged;
};

#endif /* Connection_hpp */
