//
// Copyright (c) 2008-2020 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once

#include "../Container/HashSet.h"
#include "../Core/Object.h"
#include "../IO/VectorBuffer.h"
#include "../Network/Connection.h"
#ifndef __EMSCRIPTEN__
#include "../Network/WS/WSServer.h"
#endif
#include "../Network/WS/WSClient.h"
#include <unordered_map>

#ifndef __ANDROID__
struct internal_socket_t;
struct libwebsocket;
#endif

namespace Urho3D
{

class HttpRequest;
class MemoryBuffer;
class Scene;

/// %Network subsystem. Manages client-server communications using the UDP protocol.
class URHO3D_API Network : public Object
{
    URHO3D_OBJECT(Network, Object);
#ifndef __EMSCRIPTEN__
    friend class WSServer;
#endif
    friend class WSClient;

public:
    /// Construct.
    explicit Network(Context* context);
    /// Destruct.
    ~Network() override;

#ifndef __EMSCRIPTEN__
    /// Handle an inbound message.
    void HandleMessage(const SLNet::AddressOrGUID& source, int packetID, int msgID, const char* data, size_t numBytes);
    /// Handle a new client connection.
    void NewConnectionEstablished(const SLNet::AddressOrGUID& connection);
#ifndef __ANDROID__
    void NewConnectionEstablished(struct libwebsocket* socket);
#endif
    /// Handle a client disconnection.
    void ClientDisconnected(const SLNet::AddressOrGUID& connection);
#ifndef __ANDROID__
    void ClientDisconnected(struct libwebsocket* socket);
#endif
#endif

    /// Set the data that will be used for a reply to attempts at host discovery on LAN/subnet.
    void SetDiscoveryBeacon(const VariantMap& data);
    /// Scan the LAN/subnet for available hosts.
    void DiscoverHosts(unsigned port);
    /// Set password for the client/server communcation.
    void SetPassword(const String& password);
    /// Set NAT server information.
    void SetNATServerInfo(const String& address, unsigned short port);
    /// Connect to a server using UDP protocol. Return true if connection process successfully started.
    bool Connect(const String& address, unsigned short port, Scene* scene, const VariantMap& identity = Variant::emptyVariantMap);
    /// Disconnect the connection to the server. If wait time is non-zero, will block while waiting for disconnect to finish.
    void Disconnect(int waitMSec = 0);

#ifndef __ANDROID__
    void WSConnect(const String& address, Scene* scene, const VariantMap& identity = Variant::emptyVariantMap);
#endif

    /// Start a server on a port using UDP protocol. Return true if successful.
    bool StartServer(unsigned short port, unsigned int maxConnections = 128);
    /// Stop the server.
    void StopServer();
    /// Start NAT punchtrough client to allow remote connections.
    void StartNATClient();
    /// Get local server GUID.
    const String& GetGUID() const { return guid_; }
    /// Attempt to connect to NAT server.
    void AttemptNATPunchtrough(const String& guid, Scene* scene, const VariantMap& identity = Variant::emptyVariantMap);
    /// Broadcast a message with content ID to all client connections.
    void BroadcastMessage(int msgID, bool reliable, bool inOrder, const VectorBuffer& msg, unsigned contentID = 0);
    /// Broadcast a message with content ID to all client connections.
    void BroadcastMessage(int msgID, bool reliable, bool inOrder, const unsigned char* data, unsigned numBytes, unsigned contentID = 0);
    /// Broadcast a remote event to all client connections.
    void BroadcastRemoteEvent(StringHash eventType, bool inOrder, const VariantMap& eventData = Variant::emptyVariantMap);
    /// Broadcast a remote event to all client connections in a specific scene.
    void BroadcastRemoteEvent(Scene* scene, StringHash eventType, bool inOrder, const VariantMap& eventData = Variant::emptyVariantMap);
    /// Broadcast a remote event with the specified node as a sender. Is sent to all client connections in the node's scene.
    void BroadcastRemoteEvent(Node* node, StringHash eventType, bool inOrder, const VariantMap& eventData = Variant::emptyVariantMap);
    /// Set network update FPS.
    void SetUpdateFps(int fps);
    /// Set simulated latency in milliseconds. This adds a fixed delay before sending each packet.
    void SetSimulatedLatency(int ms);
    /// Set simulated packet loss probability between 0.0 - 1.0.
    void SetSimulatedPacketLoss(float probability);
    /// Register a remote event as allowed to be received. There is also a fixed blacklist of events that can not be allowed in any case, such as ConsoleCommand.
    void RegisterRemoteEvent(StringHash eventType);
    /// Unregister a remote event as allowed to received.
    void UnregisterRemoteEvent(StringHash eventType);
    /// Unregister all remote events.
    void UnregisterAllRemoteEvents();
    /// Set the package download cache directory.
    void SetPackageCacheDir(const String& path);
    /// Trigger all client connections in the specified scene to download a package file from the server. Can be used to download additional resource packages when clients are already joined in the scene. The package must have been added as a requirement to the scene, or else the eventual download will fail.
    void SendPackageToClients(Scene* scene, PackageFile* package);
    /// Perform an HTTP request to the specified URL. Empty verb defaults to a GET request. Return a request object which can be used to read the response data.
    SharedPtr<HttpRequest> MakeHttpRequest(const String& url, const String& verb = String::EMPTY, const Vector<String>& headers = Vector<String>(), const String& postData = String::EMPTY);
    /// Ban specific IP addresses.
    void BanAddress(const String& address);
    /// Return network update FPS.
    int GetUpdateFps() const { return updateFps_; }

    /// Return simulated latency in milliseconds.
    int GetSimulatedLatency() const { return simulatedLatency_; }

    /// Return simulated packet loss probability.
    float GetSimulatedPacketLoss() const { return simulatedPacketLoss_; }

#ifndef __EMSCRIPTEN__
    /// Return a client or server connection by RakNet connection address, or null if none exist.
    Connection* GetConnection(const SLNet::AddressOrGUID& connection) const;
#endif
    /// Return the connection to the server. Null if not connected.
    Connection* GetServerConnection() const;
    /// Return all client connections.
    Vector<SharedPtr<Connection> > GetClientConnections() const;
    /// Return whether the server is running.
    bool IsServerRunning() const;
    /// Return whether a remote event is allowed to be received.
    bool CheckRemoteEvent(StringHash eventType) const;

    /// Return the package download cache directory.
    const String& GetPackageCacheDir() const { return packageCacheDir_; }

    /// Process incoming messages from connections. Called by HandleBeginFrame.
    void Update(float timeStep);
    /// Send outgoing messages after frame logic. Called by HandleRenderUpdate.
    void PostUpdate(float timeStep);

#ifndef __ANDROID__
    void HandleIncomingPacket(struct libwebsocket* socket, VectorBuffer& buffer, bool fromServer);
#endif

    void AddEventToQueue(const StringHash& event, const VariantMap& eventData = Variant::emptyVariantMap);

private:

    /// Handle begin frame event.
    void HandleBeginFrame(StringHash eventType, VariantMap& eventData);
    /// Handle render update frame event.
    void HandleRenderUpdate(StringHash eventType, VariantMap& eventData);
#ifndef __EMSCRIPTEN__
    /// Handle server connection.
    void OnServerConnected(const SLNet::AddressOrGUID& address);
    /// Handle server disconnection.
    void OnServerDisconnected(const SLNet::AddressOrGUID& address);
    /// All incoming packages are handled here.
    void HandleIncomingPacket(SLNet::Packet* packet, bool fromServer);
#endif
    /// Reconfigure network simulator parameters on all existing connections.
    void ConfigureNetworkSimulator();

#ifndef __EMSCRIPTEN__
    /// SLikeNet peer instance for server connection.
    SLNet::RakPeerInterface* rakPeer_;
    /// SLikeNet peer instance for client connection.
    SLNet::RakPeerInterface* rakPeerClient_;
#endif
    /// Client's server connection.
    SharedPtr<Connection> serverConnection_;
    /// Server's client connections.
#ifndef __EMSCRIPTEN__
    HashMap<SLNet::AddressOrGUID, SharedPtr<Connection> > clientConnections_;
#endif
#ifndef __ANDROID__
    std::unordered_map<struct libwebsocket*, SharedPtr<Connection>> clientConnections2_;
#endif
    /// Allowed remote events.
    HashSet<StringHash> allowedRemoteEvents_;
    /// Remote event fixed blacklist.
    HashSet<StringHash> blacklistedRemoteEvents_;
    /// Networked scenes.
    HashSet<Scene*> networkScenes_;
    /// Update FPS.
    int updateFps_;
    /// Simulated latency (send delay) in milliseconds.
    int simulatedLatency_;
    /// Simulated packet loss probability between 0.0 - 1.0.
    float simulatedPacketLoss_;
    /// Update time interval.
    float updateInterval_;
    /// Update time accumulator.
    float updateAcc_;
    /// Package cache directory.
    String packageCacheDir_;
    /// Whether we started as server or not.
    bool isServer_;
    /// Server/Client password used for connecting.
    String password_;
    /// Scene which will be used for NAT punchtrough connections.
    Scene* scene_;
    /// Client identify for NAT punchtrough connections.
    VariantMap identity_;
#ifndef __EMSCRIPTEN__
    /// NAT punchtrough server information.
    SLNet::SystemAddress* natPunchServerAddress_;
    /// NAT punchtrough client for the server.
    SLNet::NatPunchthroughClient* natPunchthroughServerClient_;
    /// NAT punchtrough client for the client.
    SLNet::NatPunchthroughClient* natPunchthroughClient_;
    /// Remote GUID information.
    SLNet::RakNetGUID* remoteGUID_;
#endif
    /// Local server GUID.
    String guid_;

#ifndef __ANDROID__
#ifndef __EMSCRIPTEN__
    WSServer* wsServer_{nullptr};
#endif
    WSClient* wsClient_{nullptr};
#endif

    Mutex eventMutex_;
    List<Pair<StringHash, VariantMap>> eventQueue_;

};

/// Register Network library objects.
void URHO3D_API RegisterNetworkLibrary(Context* context);

}
