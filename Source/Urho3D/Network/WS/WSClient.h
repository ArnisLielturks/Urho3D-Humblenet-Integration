#ifndef __ANDROID__
#pragma once

#include "../../Container/Str.h"
#include "../../Core/Mutex.h"
#include "../../Container/List.h"
#include "../../IO/MemoryBuffer.h"

struct internal_context_t;
struct internal_socket_t;

namespace Urho3D {
class Network;
class Scene;

class WSClient {

public:
    WSClient(Urho3D::Network *networkInstance);
    ~WSClient();

    internal_socket_t* Connect(const String &address, Scene* scene, const VariantMap& identity = Variant::emptyVariantMap);

    int Disconnect();
    void Stop();

    void Process();

    Network* GetNetworkInstance() { return networkInstance_; }

    Mutex& GetMutex() { return mutex_; }

    void AddPacket(VectorBuffer& packet);
    VectorBuffer& GetPacket();
    void RemovePacket();
    int GetNumPackets() { return packets_.Size(); }
private:

    Network *networkInstance_;

    struct internal_context_t* context;
    internal_socket_t* socket_;

    Mutex mutex_;
    List<VectorBuffer> packets_;
};
}
#endif