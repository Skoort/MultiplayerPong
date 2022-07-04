#ifndef NetworkAddress_hpp
#define NetworkAddress_hpp

#include <SFML/Network/IpAddress.hpp>

struct NetworkAddress
{
    sf::IpAddress ip;
    unsigned short port;
};

#endif /* NetworkAddress_hpp */
