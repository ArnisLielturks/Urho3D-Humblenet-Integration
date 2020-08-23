#if !defined(__EMSCRIPTEN__) && !defined(__ANDROID__)
#pragma once
#include "WSPacket.h"
#include "../../Core/Mutex.h"
#include "../../Container/List.h"
#include "../../IO/MemoryBuffer.h"
#include <list>

struct libwebsocket_context;
struct libwebsocket;

namespace Urho3D {
class Network;

class WSServer {

public:
    WSServer(Urho3D::Network *networkInstance);
    ~WSServer();

    int Start();

    int Stop();

    void Process();

    Network* GetNetworkInstance() { return networkInstance_; }

    Mutex& GetMutex() { return mutex_; }

    int GetNumPackets() { return packets_.size(); }
    void AddPacket(WSPacket& packet);
    WSPacket& GetPacket();
    void RemovePacket();
private:

    bool initialized_{false};

    Network *networkInstance_;

    struct libwebsocket_context *context;
    Mutex mutex_;
    std::list<WSPacket> packets_;
};
}
#endif