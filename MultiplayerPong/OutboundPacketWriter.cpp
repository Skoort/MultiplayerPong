#include "OutboundPacketWriter.hpp"

#include <iostream>
#include <string>

OutboundPacketWriter::OutboundPacketWriter(unsigned short port)
: m_packets{}
, m_isStopped{false}
, m_statusMutex{}
, m_queueMutex{}
, m_writeOutboundPacketsThread{}
, m_socket{}
{
    if (m_socket.bind(port) != sf::Socket::Status::Done)
    {
        throw std::runtime_error{"Could not bind to port"};
    }
}

OutboundPacketWriter::~OutboundPacketWriter()
{
    m_socket.unbind();
}

void OutboundPacketWriter::Start()
{
    m_writeOutboundPacketsThread = std::thread{&OutboundPacketWriter::WriteOutboundPackets, this};
}

void OutboundPacketWriter::Stop()
{
    std::unique_lock lock{m_statusMutex};
    m_isStopped = true;
    lock.unlock();
    if (m_writeOutboundPacketsThread.joinable())
    {
        m_writeOutboundPacketsThread.join();
    }
}

bool OutboundPacketWriter::IsStopped() const
{
    std::shared_lock lock{m_statusMutex};
    return m_isStopped;
}

void OutboundPacketWriter::QueuePacket(std::unique_ptr<PacketInfo> packet)
{
    std::unique_lock lock{m_queueMutex};
    m_packets.push(std::move(packet));
}

auto OutboundPacketWriter::GetPort() const -> unsigned short
{
    return m_socket.getLocalPort();
}

void OutboundPacketWriter::WriteOutboundPackets()
{
    while (true)
    {
        std::shared_lock statusLock{m_statusMutex};
        if (m_isStopped) break;
        statusLock.unlock();
        
        std::unique_lock lock{m_queueMutex};
        if (m_packets.size() <= 0)
        {
            continue;
        }
        auto packet = std::move(m_packets.front());
        m_packets.pop();
        lock.unlock();
        
        auto status = m_socket.send(packet->body, packet->address.ip, packet->address.port);
        if (status != sf::Socket::Status::Done)
        {
            std::cerr << "Something went wrong sending a packet!" << std::endl;
        }
    }
}

