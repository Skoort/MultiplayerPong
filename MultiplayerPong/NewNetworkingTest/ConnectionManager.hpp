#ifndef ConnectionManager_hpp
#define ConnectionManager_hpp

#include <map>
#include <memory>

#include <SFML/Network/IpAddress.hpp>
#include <SFML/Network/Packet.hpp>

#include "Connection.hpp"

class ConnectionManager
{
    public:
        ConnectionManager(
            unsigned short portIn,
            unsigned short portOut,
            bool canAcceptConnections);
        ~ConnectionManager();
    
        void Connect(sf::IpAddress ipAddress, unsigned short port);
    
        void SendToAll(sf::Packet packet, int exceptId=-1);
    
    private:
        unsigned short m_portIn;
        unsigned short m_portOut;
        std::map<int, std::shared_ptr<Connection>> m_connections;
};

#endif /* ConnectionManager_hpp */
