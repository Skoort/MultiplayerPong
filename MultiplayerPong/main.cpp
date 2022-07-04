#include <SFML/Graphics.hpp>

#include <iostream>
#include <iterator>
#include <stdlib.h>

#include <SFML/Network/Socket.hpp>

#include "Client.hpp"
#include "ClientServer.hpp"
#include "NetworkAddress.hpp"

int main(int argc, char *argv[])
{
    srand(time(NULL));
    
//    auto server = ClientServer{54321, 54322};
//    server.Play();
//
//    return 0;
//    sf::IpAddress serverIp = sf::IpAddress{"127.0.0.1"};
//    unsigned short serverPortIn = 54321;
//    unsigned short serverPortOut = 54322;
//    auto client = Client{sf::Socket::AnyPort, serverIp, serverPortIn, serverPortOut};
//    client.Play();
//
//    return 0;
    if (argc < 2)
    {
        printf("Expected usage is either \"./%s clientServer <portIn> <portOut>\" or \"./%s client <serverIpAddress> <serverPortIn> <serverPortOut>\".", argv[0], argv[0]);
        return -1;
    } else
    if (std::string{argv[1]} == "clientServer" && argc < 4)
    {
        printf("Expected usage is \"./%s clientServer <portIn> <portOut>\".", argv[0]);
        return -1;
    } else
    if (std::string{argv[1]} == "clientServer" && argc >= 4)
    {
        unsigned short portIn = std::stoi(argv[2]);
        unsigned short portOut = std::stoi(argv[3]);
        auto server = ClientServer{portIn, portOut};
        server.Play();
    } else
    if (std::string{argv[1]} == "client" && argc < 5)
    {
        printf("Expected usage is \"./%s client <serverIpAddress> <serverPortIn> <serverPortOut>\".", argv[0]);
        return -1;
    } else
    if (std::string{argv[1]} == "client" && argc >= 5)
    {
        sf::IpAddress serverIp = sf::IpAddress{argv[2]};
        unsigned short serverPortIn = std::stoi(argv[3]);
        unsigned short serverPortOut = std::stoi(argv[4]);
        auto client = Client{sf::Socket::AnyPort, serverIp, serverPortIn, serverPortOut};
        client.Play();
    }

    return 0;
}
