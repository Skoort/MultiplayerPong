#ifndef ClientServer_hpp
#define ClientServer_hpp

#include "Game.hpp"
#include "NetworkAddress.hpp"

class ClientServer
: public Game
{
    public:
        ClientServer(unsigned short portIn, unsigned short portOut);

    protected:
        void UpdateWaiting(float dt) override;
        void UpdatePlaying(float dt) override;

    private:
        NetworkAddress m_remoteAddress;
};

#endif /* ClientServer_hpp */
