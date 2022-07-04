#ifndef PacketInfo_hpp
#define PacketInfo_hpp

#include <SFML/Network/Packet.hpp>

#include "NetworkAddress.hpp"

struct PacketInfo
{
    NetworkAddress address;
    sf::Packet body;
};

#endif /* PacketInfo_hpp */
