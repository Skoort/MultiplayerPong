#include "InboundPacketReader.hpp"

#include <iostream>
#include <string>

InboundPacketReader::InboundPacketReader(unsigned short port)
: m_packets{}
, m_isStopped{false}
, m_statusMutex{}
, m_queueMutex{}
, m_readInboundPacketsThread{}
, m_socket{}
{
    if (m_socket.bind(port) != sf::Socket::Status::Done)
    {
        throw std::runtime_error{"Could not bind to port " + std::to_string(port)};
    }
    m_socket.setBlocking(false);
}

InboundPacketReader::~InboundPacketReader()
{
    m_socket.unbind();
}

void InboundPacketReader::Start()
{
    m_readInboundPacketsThread = std::thread{&InboundPacketReader::ReadInboundPackets, this};
}

void InboundPacketReader::Stop()
{
    std::unique_lock lock{m_statusMutex};
    m_isStopped = true;
    lock.unlock();
    if (m_readInboundPacketsThread.joinable())  // This should also work for repeated calls to Stop.
    {
        m_readInboundPacketsThread.join();
    }
}

bool InboundPacketReader::IsStopped() const
{
    std::shared_lock lock{m_statusMutex};
    return m_isStopped;
}

auto InboundPacketReader::GetPacket() -> std::unique_ptr<PacketInfo>
{
    std::unique_lock lock{m_queueMutex, std::try_to_lock};
    if (lock)
    {
        if (m_packets.size() > 0)
        {
            auto packet = std::move(m_packets.front());
            m_packets.pop();
            return packet;
        }
    }
    return nullptr;
}

auto InboundPacketReader::GetListeningPort() const -> unsigned short
{
    return m_socket.getLocalPort();
}

void InboundPacketReader::ReadInboundPackets()
{
    while (true)
    {
        std::shared_lock statusLock{m_statusMutex};
        if (m_isStopped) break;
        statusLock.unlock();
        
        auto packet = std::make_unique<PacketInfo>();
        auto status = m_socket.receive(packet->body, packet->address.ip, packet->address.port);
        if (status == sf::Socket::NotReady)
        {
            continue;
        } else
        if (status != sf::Socket::Status::Done)
        {
            std::cerr << "Something went wrong receiving a packet!" << std::endl;
            continue;
        }

        std::unique_lock queueLock{m_queueMutex};
        m_packets.push(std::move(packet));
    }
}
