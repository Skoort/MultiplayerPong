#ifndef InboundPacketReader_hpp
#define InboundPacketReader_hpp

#include <memory>
#include <mutex>
#include <queue>
#include <shared_mutex>
#include <thread>

#include <SFML/Network/Packet.hpp>
#include <SFML/Network/UdpSocket.hpp>

#include "PacketInfo.hpp"

class InboundPacketReader
{
    public:
        InboundPacketReader(unsigned short port = sf::Socket::AnyPort);
        ~InboundPacketReader();
    
        void Start();
        void Stop();
        bool IsStopped() const;
    
        auto GetPacket() -> std::unique_ptr<PacketInfo>;
    
        auto GetListeningPort() const -> unsigned short;
    
    private:
        void ReadInboundPackets();
    
        std::queue<std::unique_ptr<PacketInfo>> m_packets;
        bool m_isStopped;
    
        mutable std::mutex m_queueMutex;
        mutable std::shared_mutex m_statusMutex;
        std::thread m_readInboundPacketsThread;
        sf::UdpSocket m_socket;
};

#endif /* InboundPacketReader_hpp */
