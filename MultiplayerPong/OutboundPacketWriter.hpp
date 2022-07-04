#ifndef OutboundPacketWriter_hpp
#define OutboundPacketWriter_hpp

#include <memory>
#include <mutex>
#include <queue>
#include <shared_mutex>
#include <thread>

#include <SFML/Network/Packet.hpp>
#include <SFML/Network/UdpSocket.hpp>

#include "PacketInfo.hpp"

class OutboundPacketWriter
{
    public:
        OutboundPacketWriter(unsigned short port = sf::Socket::AnyPort);;
        ~OutboundPacketWriter();
    
        void Start();
        void Stop();
        bool IsStopped() const;
    
        void QueuePacket(std::unique_ptr<PacketInfo> packet);
    
        auto GetPort() const -> unsigned short;
    
    private:
        void WriteOutboundPackets();
    
        std::queue<std::unique_ptr<PacketInfo>> m_packets;
        bool m_isStopped;
    
        mutable std::mutex m_queueMutex;
        mutable std::shared_mutex m_statusMutex;
        std::thread m_writeOutboundPacketsThread;
        sf::UdpSocket m_socket;
};

#endif /* OutboundPacketWriter_hpp */
