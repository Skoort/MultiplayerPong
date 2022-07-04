#include "Connection.hpp"

void Connection::Send(std::unique_ptr<sf::Packet> packet)
{
    auto wrappedPacket = std::make_unique<sf::Packet>();
    *wrappedPacket << ++m_latestSentId;
    wrappedPacket->append(packet->getData(), packet->getDataSize());
    
    // Queue the packet for sending.
}
