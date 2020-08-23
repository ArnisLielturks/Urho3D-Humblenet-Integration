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

#include "../Precompiled.h"

#include "../Core/Context.h"
#include "../Core/CoreEvents.h"
#include "../Core/Profiler.h"
#include "../Engine/EngineEvents.h"
#include "../IO/FileSystem.h"
#include "../Input/InputEvents.h"
#include "../IO/IOEvents.h"
#include "../IO/Log.h"
#include "../IO/MemoryBuffer.h"
#include "../Network/HttpRequest.h"
#include "../Network/Network.h"
#include "../Network/NetworkEvents.h"
#include "../Network/NetworkPriority.h"
#include "../Network/Protocol.h"
#include "../Scene/Scene.h"

#ifndef __EMSCRIPTEN__
#include <SLikeNet/MessageIdentifiers.h>
#include <SLikeNet/NatPunchthroughClient.h>
#include <SLikeNet/peerinterface.h>
#include <SLikeNet/statistics.h>
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#ifndef __ANDROID__
#include "libsocket.h"
#endif

#ifdef SendMessage
#undef SendMessage
#endif
#include "WS/WSEvents.h"

#include "../DebugNew.h"

const int MAX_MESSAGE_SIZE = 512;
namespace Urho3D
{

static const char* RAKNET_MESSAGEID_STRINGS[] = {
    "ID_CONNECTED_PING",  
    "ID_UNCONNECTED_PING",
    "ID_UNCONNECTED_PING_OPEN_CONNECTIONS",
    "ID_CONNECTED_PONG",
    "ID_DETECT_LOST_CONNECTIONS",
    "ID_OPEN_CONNECTION_REQUEST_1",
    "ID_OPEN_CONNECTION_REPLY_1",
    "ID_OPEN_CONNECTION_REQUEST_2",
    "ID_OPEN_CONNECTION_REPLY_2",
    "ID_CONNECTION_REQUEST",
    "ID_REMOTE_SYSTEM_REQUIRES_PUBLIC_KEY",
    "ID_OUR_SYSTEM_REQUIRES_SECURITY",
    "ID_PUBLIC_KEY_MISMATCH",
    "ID_OUT_OF_BAND_INTERNAL",
    "ID_SND_RECEIPT_ACKED",
    "ID_SND_RECEIPT_LOSS",
    "ID_CONNECTION_REQUEST_ACCEPTED",
    "ID_CONNECTION_ATTEMPT_FAILED",
    "ID_ALREADY_CONNECTED",
    "ID_NEW_INCOMING_CONNECTION",
    "ID_NO_FREE_INCOMING_CONNECTIONS",
    "ID_DISCONNECTION_NOTIFICATION",
    "ID_CONNECTION_LOST",
    "ID_CONNECTION_BANNED",
    "ID_INVALID_PASSWORD",
    "ID_INCOMPATIBLE_PROTOCOL_VERSION",
    "ID_IP_RECENTLY_CONNECTED",
    "ID_TIMESTAMP",
    "ID_UNCONNECTED_PONG",
    "ID_ADVERTISE_SYSTEM",
    "ID_DOWNLOAD_PROGRESS",
    "ID_REMOTE_DISCONNECTION_NOTIFICATION",
    "ID_REMOTE_CONNECTION_LOST",
    "ID_REMOTE_NEW_INCOMING_CONNECTION",
    "ID_FILE_LIST_TRANSFER_HEADER",
    "ID_FILE_LIST_TRANSFER_FILE",
    "ID_FILE_LIST_REFERENCE_PUSH_ACK",
    "ID_DDT_DOWNLOAD_REQUEST",
    "ID_TRANSPORT_STRING",
    "ID_REPLICA_MANAGER_CONSTRUCTION",
    "ID_REPLICA_MANAGER_SCOPE_CHANGE",
    "ID_REPLICA_MANAGER_SERIALIZE",
    "ID_REPLICA_MANAGER_DOWNLOAD_STARTED",
    "ID_REPLICA_MANAGER_DOWNLOAD_COMPLETE",
    "ID_RAKVOICE_OPEN_CHANNEL_REQUEST",
    "ID_RAKVOICE_OPEN_CHANNEL_REPLY",
    "ID_RAKVOICE_CLOSE_CHANNEL",
    "ID_RAKVOICE_DATA",
    "ID_AUTOPATCHER_GET_CHANGELIST_SINCE_DATE",
    "ID_AUTOPATCHER_CREATION_LIST",
    "ID_AUTOPATCHER_DELETION_LIST",
    "ID_AUTOPATCHER_GET_PATCH",
    "ID_AUTOPATCHER_PATCH_LIST",
    "ID_AUTOPATCHER_REPOSITORY_FATAL_ERROR",
    "ID_AUTOPATCHER_CANNOT_DOWNLOAD_ORIGINAL_UNMODIFIED_FILES",
    "ID_AUTOPATCHER_FINISHED_INTERNAL",
    "ID_AUTOPATCHER_FINISHED",
    "ID_AUTOPATCHER_RESTART_APPLICATION",
    "ID_NAT_PUNCHTHROUGH_REQUEST",
    "ID_NAT_CONNECT_AT_TIME",
    "ID_NAT_GET_MOST_RECENT_PORT",
    "ID_NAT_CLIENT_READY",
    "ID_NAT_TARGET_NOT_CONNECTED",
    "ID_NAT_TARGET_UNRESPONSIVE",
    "ID_NAT_CONNECTION_TO_TARGET_LOST",
    "ID_NAT_ALREADY_IN_PROGRESS",
    "ID_NAT_PUNCHTHROUGH_FAILED",
    "ID_NAT_PUNCHTHROUGH_SUCCEEDED",
    "ID_READY_EVENT_SET",
    "ID_READY_EVENT_UNSET",
    "ID_READY_EVENT_ALL_SET",
    "ID_READY_EVENT_QUERY",
    "ID_LOBBY_GENERAL",
    "ID_RPC_REMOTE_ERROR",
    "ID_RPC_PLUGIN",
    "ID_FILE_LIST_REFERENCE_PUSH",
    "ID_READY_EVENT_FORCE_ALL_SET",
    "ID_ROOMS_EXECUTE_FUNC",
    "ID_ROOMS_LOGON_STATUS",
    "ID_ROOMS_HANDLE_CHANGE",
    "ID_LOBBY2_SEND_MESSAGE",
    "ID_LOBBY2_SERVER_ERROR",
    "ID_FCM2_NEW_HOST",
    "ID_FCM2_REQUEST_FCMGUID",
    "ID_FCM2_RESPOND_CONNECTION_COUNT",
    "ID_FCM2_INFORM_FCMGUID",
    "ID_FCM2_UPDATE_MIN_TOTAL_CONNECTION_COUNT",
    "ID_FCM2_VERIFIED_JOIN_START",
    "ID_FCM2_VERIFIED_JOIN_CAPABLE",
    "ID_FCM2_VERIFIED_JOIN_FAILED",
    "ID_FCM2_VERIFIED_JOIN_ACCEPTED",
    "ID_FCM2_VERIFIED_JOIN_REJECTED",
    "ID_UDP_PROXY_GENERAL",
    "ID_SQLite3_EXEC",
    "ID_SQLite3_UNKNOWN_DB",
    "ID_SQLLITE_LOGGER",
    "ID_NAT_TYPE_DETECTION_REQUEST",
    "ID_NAT_TYPE_DETECTION_RESULT",
    "ID_ROUTER_2_INTERNAL",
    "ID_ROUTER_2_FORWARDING_NO_PATH",
    "ID_ROUTER_2_FORWARDING_ESTABLISHED",
    "ID_ROUTER_2_REROUTED",
    "ID_TEAM_BALANCER_INTERNAL",
    "ID_TEAM_BALANCER_REQUESTED_TEAM_FULL",
    "ID_TEAM_BALANCER_REQUESTED_TEAM_LOCKED",
    "ID_TEAM_BALANCER_TEAM_REQUESTED_CANCELLED",
    "ID_TEAM_BALANCER_TEAM_ASSIGNED",
    "ID_LIGHTSPEED_INTEGRATION",
    "ID_XBOX_LOBBY",
    "ID_TWO_WAY_AUTHENTICATION_INCOMING_CHALLENGE_SUCCESS",
    "ID_TWO_WAY_AUTHENTICATION_OUTGOING_CHALLENGE_SUCCESS",
    "ID_TWO_WAY_AUTHENTICATION_INCOMING_CHALLENGE_FAILURE",
    "ID_TWO_WAY_AUTHENTICATION_OUTGOING_CHALLENGE_FAILURE",
    "ID_TWO_WAY_AUTHENTICATION_OUTGOING_CHALLENGE_TIMEOUT",
    "ID_TWO_WAY_AUTHENTICATION_NEGOTIATION",
    "ID_CLOUD_POST_REQUEST",
    "ID_CLOUD_RELEASE_REQUEST",
    "ID_CLOUD_GET_REQUEST",
    "ID_CLOUD_GET_RESPONSE",
    "ID_CLOUD_UNSUBSCRIBE_REQUEST",
    "ID_CLOUD_SERVER_TO_SERVER_COMMAND",
    "ID_CLOUD_SUBSCRIPTION_NOTIFICATION",
    "ID_LIB_VOICE",
    "ID_RELAY_PLUGIN",
    "ID_NAT_REQUEST_BOUND_ADDRESSES",
    "ID_NAT_RESPOND_BOUND_ADDRESSES",
    "ID_FCM2_UPDATE_USER_CONTEXT",
    "ID_RESERVED_3",
    "ID_RESERVED_4",
    "ID_RESERVED_5",
    "ID_RESERVED_6",
    "ID_RESERVED_7",
    "ID_RESERVED_8",
    "ID_RESERVED_9",
    "ID_USER_PACKET_ENUM"
};

static const int DEFAULT_UPDATE_FPS = 30;
static const int SERVER_TIMEOUT_TIME = 10000;

static Network* networkInstance;

Network::Network(Context* context) :
    Object(context),
    updateFps_(DEFAULT_UPDATE_FPS),
    simulatedLatency_(0),
    simulatedPacketLoss_(0.0f),
    updateInterval_(1.0f / (float)DEFAULT_UPDATE_FPS),
    updateAcc_(0.0f),
    isServer_(false),
    scene_(nullptr)
#ifndef __EMSCRIPTEN__
    ,
    natPunchServerAddress_(nullptr),
    remoteGUID_(nullptr)
#endif
{
    networkInstance = this;
#ifndef __EMSCRIPTEN__
    rakPeer_ = SLNet::RakPeerInterface::GetInstance();
    rakPeerClient_ = SLNet::RakPeerInterface::GetInstance();
    rakPeer_->SetTimeoutTime(SERVER_TIMEOUT_TIME, SLNet::UNASSIGNED_SYSTEM_ADDRESS);
#endif
    SetPassword("");
    SetDiscoveryBeacon(VariantMap());

#ifndef __EMSCRIPTEN__
    natPunchthroughClient_ = new SLNet::NatPunchthroughClient;
    natPunchthroughServerClient_ = new SLNet::NatPunchthroughClient;
#endif

    SetNATServerInfo("127.0.0.1", 61111);

    // Register Network library object factories
    RegisterNetworkLibrary(context_);

    SubscribeToEvent(E_BEGINFRAME, URHO3D_HANDLER(Network, HandleBeginFrame));
    SubscribeToEvent(E_RENDERUPDATE, URHO3D_HANDLER(Network, HandleRenderUpdate));

    // Blacklist remote events which are not to be allowed to be registered in any case
    blacklistedRemoteEvents_.Insert(E_CONSOLECOMMAND);
    blacklistedRemoteEvents_.Insert(E_LOGMESSAGE);
    blacklistedRemoteEvents_.Insert(E_BEGINFRAME);
    blacklistedRemoteEvents_.Insert(E_UPDATE);
    blacklistedRemoteEvents_.Insert(E_POSTUPDATE);
    blacklistedRemoteEvents_.Insert(E_RENDERUPDATE);
    blacklistedRemoteEvents_.Insert(E_ENDFRAME);
    blacklistedRemoteEvents_.Insert(E_MOUSEBUTTONDOWN);
    blacklistedRemoteEvents_.Insert(E_MOUSEBUTTONUP);
    blacklistedRemoteEvents_.Insert(E_MOUSEMOVE);
    blacklistedRemoteEvents_.Insert(E_MOUSEWHEEL);
    blacklistedRemoteEvents_.Insert(E_KEYDOWN);
    blacklistedRemoteEvents_.Insert(E_KEYUP);
    blacklistedRemoteEvents_.Insert(E_TEXTINPUT);
    blacklistedRemoteEvents_.Insert(E_JOYSTICKCONNECTED);
    blacklistedRemoteEvents_.Insert(E_JOYSTICKDISCONNECTED);
    blacklistedRemoteEvents_.Insert(E_JOYSTICKBUTTONDOWN);
    blacklistedRemoteEvents_.Insert(E_JOYSTICKBUTTONUP);
    blacklistedRemoteEvents_.Insert(E_JOYSTICKAXISMOVE);
    blacklistedRemoteEvents_.Insert(E_JOYSTICKHATMOVE);
    blacklistedRemoteEvents_.Insert(E_TOUCHBEGIN);
    blacklistedRemoteEvents_.Insert(E_TOUCHEND);
    blacklistedRemoteEvents_.Insert(E_TOUCHMOVE);
    blacklistedRemoteEvents_.Insert(E_GESTURERECORDED);
    blacklistedRemoteEvents_.Insert(E_GESTUREINPUT);
    blacklistedRemoteEvents_.Insert(E_MULTIGESTURE);
    blacklistedRemoteEvents_.Insert(E_DROPFILE);
    blacklistedRemoteEvents_.Insert(E_INPUTFOCUS);
    blacklistedRemoteEvents_.Insert(E_MOUSEVISIBLECHANGED);
    blacklistedRemoteEvents_.Insert(E_EXITREQUESTED);
    blacklistedRemoteEvents_.Insert(E_SERVERCONNECTED);
    blacklistedRemoteEvents_.Insert(E_SERVERDISCONNECTED);
    blacklistedRemoteEvents_.Insert(E_CONNECTFAILED);
    blacklistedRemoteEvents_.Insert(E_CLIENTCONNECTED);
    blacklistedRemoteEvents_.Insert(E_CLIENTDISCONNECTED);
    blacklistedRemoteEvents_.Insert(E_CLIENTIDENTITY);
    blacklistedRemoteEvents_.Insert(E_CLIENTSCENELOADED);
    blacklistedRemoteEvents_.Insert(E_NETWORKMESSAGE);
    blacklistedRemoteEvents_.Insert(E_NETWORKUPDATE);
    blacklistedRemoteEvents_.Insert(E_NETWORKUPDATESENT);
    blacklistedRemoteEvents_.Insert(E_NETWORKSCENELOADFAILED);
}

Network::~Network()
{
#ifndef __EMSCRIPTEN__
    rakPeer_->DetachPlugin(natPunchthroughServerClient_);
    rakPeerClient_->DetachPlugin(natPunchthroughClient_);
#endif
    // If server connection exists, disconnect, but do not send an event because we are shutting down
    Disconnect(100);
    serverConnection_.Reset();

#ifndef __EMSCRIPTEN__
    clientConnections_.Clear();

    delete natPunchthroughServerClient_;
    natPunchthroughServerClient_ = nullptr;
    delete natPunchthroughClient_;
    natPunchthroughClient_ = nullptr;
    delete remoteGUID_;
    remoteGUID_ = nullptr;
    delete natPunchServerAddress_;
    natPunchServerAddress_ = nullptr;

    SLNet::RakPeerInterface::DestroyInstance(rakPeer_);
    SLNet::RakPeerInterface::DestroyInstance(rakPeerClient_);
    rakPeer_ = nullptr;
    rakPeerClient_ = nullptr;
#endif
}

#ifndef __EMSCRIPTEN__
void Network::HandleMessage(const SLNet::AddressOrGUID& source, int packetID, int msgID, const char* data, size_t numBytes)
{
    // Only process messages from known sources
    Connection* connection = GetConnection(source);
    if (connection)
    {
        MemoryBuffer msg(data, (unsigned)numBytes);
        if (connection->ProcessMessage((int)msgID, msg))
            return;
    }
    else
        URHO3D_LOGWARNING("Discarding message from unknown MessageConnection " + String(source.ToString()));
}
#endif

#ifndef __EMSCRIPTEN__
void Network::NewConnectionEstablished(const SLNet::AddressOrGUID& connection)
{
    // Create a new client connection corresponding to this MessageConnection
    SharedPtr<Connection> newConnection(new Connection(context_, true, connection, rakPeer_));
    newConnection->ConfigureNetworkSimulator(simulatedLatency_, simulatedPacketLoss_);
    clientConnections_[connection] = newConnection;
    URHO3D_LOGINFO("Client " + newConnection->ToString() + " connected");

    using namespace ClientConnected;

    VariantMap& eventData = GetEventDataMap();
    eventData[P_CONNECTION] = newConnection;
    newConnection->SendEvent(E_CLIENTCONNECTED, eventData);
}
#endif

#if !defined(__EMSCRIPTEN__) && !defined(__ANDROID__)
void Network::NewConnectionEstablished(struct libwebsocket* socket)
{
    // Create a new client connection corresponding to this MessageConnection
    SharedPtr<Connection> newConnection(new Connection(context_, true, socket));
    newConnection->ConfigureNetworkSimulator(simulatedLatency_, simulatedPacketLoss_);
    clientConnections2_[socket] = newConnection;
    URHO3D_LOGINFO("Client " + newConnection->ToString() + " connected");

    using namespace ClientConnected;

    VariantMap& eventData = GetEventDataMap();
    eventData[P_CONNECTION] = newConnection;
    newConnection->SendEvent(E_CLIENTCONNECTED, eventData);
}
#endif

#ifndef __EMSCRIPTEN__
void Network::ClientDisconnected(const SLNet::AddressOrGUID& connection)
{
    // Remove the client connection that corresponds to this MessageConnection
    HashMap<SLNet::AddressOrGUID, SharedPtr<Connection> >::Iterator i = clientConnections_.Find(connection);
    if (i != clientConnections_.End())
    {
        Connection* connection = i->second_;
        URHO3D_LOGINFO("Client " + connection->ToString() + " disconnected");

        using namespace ClientDisconnected;

        VariantMap& eventData = GetEventDataMap();
        eventData[P_CONNECTION] = connection;
        connection->SendEvent(E_CLIENTDISCONNECTED, eventData);

        clientConnections_.Erase(i);
    }
}

#ifndef __ANDROID__
void Network::ClientDisconnected(struct libwebsocket* socket)
{
// Remove the client connection that corresponds to this MessageConnection
    auto i = clientConnections2_.find(socket);
    if (i != clientConnections2_.end())
    {
        Connection* connection = i->second;
        URHO3D_LOGINFO("Client " + connection->ToString() + " disconnected");

        using namespace ClientDisconnected;

        VariantMap& eventData = GetEventDataMap();
        eventData[P_CONNECTION] = connection;
        connection->SendEvent(E_CLIENTDISCONNECTED, eventData);

        clientConnections2_.erase(i);
    }
}
#endif
#endif

void Network::SetDiscoveryBeacon(const VariantMap& data)
{
    VectorBuffer buffer;
    buffer.WriteVariantMap(data);
    if (buffer.GetSize() > 400)
        URHO3D_LOGERROR("Discovery beacon of size: " + String(buffer.GetSize()) + " bytes is too large, modify MAX_OFFLINE_DATA_LENGTH in RakNet or reduce size");
#ifndef __EMSCRIPTEN__
    rakPeer_->SetOfflinePingResponse((const char*)buffer.GetData(), buffer.GetSize());
#endif
}

void Network::DiscoverHosts(unsigned port)
{
#ifndef __EMSCRIPTEN__
    // JSandusky: Contrary to the manual, we actually do have to perform Startup first before we can Ping
    if (!rakPeerClient_->IsActive())
    {
        SLNet::SocketDescriptor socket;
        // Startup local connection with max 1 incoming connection(first param) and 1 socket description (third param)
        rakPeerClient_->Startup(1, &socket, 1);
    }
    rakPeerClient_->Ping("255.255.255.255", port, false);
#endif
}

void Network::SetPassword(const String& password)
{
#ifndef __EMSCRIPTEN__
    rakPeer_->SetIncomingPassword(password.CString(), password.Length());
#endif
    password_ = password;
}

#ifndef __ANDROID__
void Network::WSConnect(const String& address, Scene* scene, const VariantMap& identity)
{
    wsClient_ = new WSClient(this);
    internal_socket_t* socket = wsClient_->Connect(address, scene, identity);
    serverConnection_ = new Urho3D::Connection(context_, false, socket);
    serverConnection_->SetScene(scene);
    serverConnection_->SetIdentity(identity);
    serverConnection_->SetConnectPending(true);
}
#endif

bool Network::Connect(const String& address, unsigned short port, Scene* scene, const VariantMap& identity)
{
    URHO3D_PROFILE(Connect);

#ifndef __EMSCRIPTEN__
    if (!rakPeerClient_->IsActive())
    {
        URHO3D_LOGINFO("Initializing client connection...");
        SLNet::SocketDescriptor socket;
        // Startup local connection with max 2 incoming connections(first param) and 1 socket description (third param)
        rakPeerClient_->Startup(2, &socket, 1);
    }

    //isServer_ = false;
    SLNet::ConnectionAttemptResult connectResult = rakPeerClient_->Connect(address.CString(), port, password_.CString(), password_.Length());
    if (connectResult == SLNet::CONNECTION_ATTEMPT_STARTED)
    {
        serverConnection_ = new Connection(context_, false, rakPeerClient_->GetMyBoundAddress(), rakPeerClient_);
        serverConnection_->SetScene(scene);
        serverConnection_->SetIdentity(identity);
        serverConnection_->SetConnectPending(true);
        serverConnection_->ConfigureNetworkSimulator(simulatedLatency_, simulatedPacketLoss_);

        URHO3D_LOGINFO("Connecting to server " + address + ":" + String(port) + ", Client: " + serverConnection_->ToString());
        return true;
    }
    else if (connectResult == SLNet::ALREADY_CONNECTED_TO_ENDPOINT) {
        URHO3D_LOGWARNING("Already connected to server!");
        SendEvent(E_CONNECTIONINPROGRESS);
        return false;
    }
    else if (connectResult == SLNet::CONNECTION_ATTEMPT_ALREADY_IN_PROGRESS) {
        URHO3D_LOGWARNING("Connection attempt already in progress!");
        SendEvent(E_CONNECTIONINPROGRESS);
        return false;
    }
    else
    {
        URHO3D_LOGERROR("Failed to connect to server " + address + ":" + String(port) + ", error code: " + String((int)connectResult));
        SendEvent(E_CONNECTFAILED);
        return false;
    }
#endif
    return false;
}

void Network::Disconnect(int waitMSec)
{
    if (!serverConnection_)
        return;

#ifndef __ANDROID__
    if (wsClient_) {
        serverConnection_->SetWSSocket(0);
//        wsClient_->Disconnect();

#ifdef __EMSCRIPTEN__
       EM_ASM({
           Module.__libwebsocket.sockets.get(1).close();
       });
#endif
    }
#endif

    URHO3D_PROFILE(Disconnect);
    serverConnection_->Disconnect(waitMSec);
    serverConnection_.Reset();
}

bool Network::StartServer(unsigned short port, unsigned int maxConnections)
{
    if (IsServerRunning())
        return true;

    URHO3D_PROFILE(StartServer);

#if !defined(__EMSCRIPTEN__) && !defined(__ANDROID__)
    wsServer_ = new WSServer(this);
    wsServer_->Start();
#endif

#ifndef __EMSCRIPTEN__
    SLNet::SocketDescriptor socket;//(port, AF_INET);
    socket.port = port;
    socket.socketFamily = AF_INET;
    // Startup local connection with max 128 incoming connection(first param) and 1 socket description (third param)
    SLNet::StartupResult startResult = rakPeer_->Startup(maxConnections, &socket, 1);
    if (startResult == SLNet::RAKNET_STARTED)
    {
        URHO3D_LOGINFO("Started server on port " + String(port));
        rakPeer_->SetMaximumIncomingConnections(maxConnections);
        isServer_ = true;
        rakPeer_->SetOccasionalPing(true);
        rakPeer_->SetUnreliableTimeout(1000);
        //rakPeer_->SetIncomingPassword("Parole", (int)strlen("Parole"));
        return true;
    }
    else
    {
        URHO3D_LOGINFO("Failed to start server on port " + String(port) + ", error code: " + String((int)startResult));
        return false;
    }
#endif
    return false;
}

void Network::StopServer()
{

#ifndef __EMSCRIPTEN__
#ifndef __ANDROID__
    if (wsServer_) {
        wsServer_->Stop();
        delete wsServer_;
        wsServer_ = nullptr;
    }
#endif

    clientConnections_.Clear();

    if (!rakPeer_)
        return;

    if (!IsServerRunning())
        return;
    // Provide 300 ms to notify
    rakPeer_->Shutdown(300);
#endif
    URHO3D_PROFILE(StopServer);

    URHO3D_LOGINFO("Stopped server");
}

void Network::SetNATServerInfo(const String& address, unsigned short port)
{
#ifndef __EMSCRIPTEN__
    if (!natPunchServerAddress_)
        natPunchServerAddress_ = new SLNet::SystemAddress;

    natPunchServerAddress_->FromStringExplicitPort(address.CString(), port);
#endif
}

void Network::StartNATClient()
{
#ifndef __EMSCRIPTEN__
    if (!rakPeer_) {
        URHO3D_LOGERROR("Unable to start NAT client, client not initialized!");
        return;
    }
    if (natPunchServerAddress_->GetPort() == 0) {
        URHO3D_LOGERROR("NAT master server address incorrect!");
        return;
    }

    rakPeer_->AttachPlugin(natPunchthroughServerClient_);
    guid_ = String(rakPeer_->GetGuidFromSystemAddress(SLNet::UNASSIGNED_SYSTEM_ADDRESS).ToString());
    URHO3D_LOGINFO("GUID: " + guid_);
    rakPeer_->Connect(natPunchServerAddress_->ToString(false), natPunchServerAddress_->GetPort(), nullptr, 0);
#endif
}

void Network::AttemptNATPunchtrough(const String& guid, Scene* scene, const VariantMap& identity)
{
#ifndef __EMSCRIPTEN__
    scene_ = scene;
    identity_ = identity;
    if (!remoteGUID_)
        remoteGUID_ = new SLNet::RakNetGUID;

    remoteGUID_->FromString(guid.CString());
    rakPeerClient_->AttachPlugin(natPunchthroughClient_);
    if (rakPeerClient_->IsActive()) {
        natPunchthroughClient_->OpenNAT(*remoteGUID_, *natPunchServerAddress_);
    }
    else {
        SLNet::SocketDescriptor socket;
        // Startup local connection with max 2 incoming connections(first param) and 1 socket description (third param)
        rakPeerClient_->Startup(2, &socket, 1);
    }

    rakPeerClient_->Connect(natPunchServerAddress_->ToString(false), natPunchServerAddress_->GetPort(), nullptr, 0);
#endif
}

void Network::BroadcastMessage(int msgID, bool reliable, bool inOrder, const VectorBuffer& msg, unsigned contentID)
{
    BroadcastMessage(msgID, reliable, inOrder, msg.GetData(), msg.GetSize(), contentID);
}

void Network::BroadcastMessage(int msgID, bool reliable, bool inOrder, const unsigned char* data, unsigned numBytes,
    unsigned contentID)
{
#ifndef __EMSCRIPTEN__
    if (!rakPeer_) 
        return;

    VectorBuffer msgData;
    msgData.WriteUByte((unsigned char)ID_USER_PACKET_ENUM);
    msgData.WriteUInt((unsigned int)msgID);
    msgData.Write(data, numBytes);

    if (isServer_)
        rakPeer_->Send((const char*)msgData.GetData(), (int)msgData.GetSize(), HIGH_PRIORITY, RELIABLE, (char)0, SLNet::UNASSIGNED_RAKNET_GUID, true);
    else
        URHO3D_LOGERROR("Server not running, can not broadcast messages");
#endif
}

void Network::BroadcastRemoteEvent(StringHash eventType, bool inOrder, const VariantMap& eventData)
{
#if !defined(__EMSCRIPTEN__) && !defined(__ANDROID__)
    for (HashMap<SLNet::AddressOrGUID, SharedPtr<Connection> >::Iterator i = clientConnections_.Begin(); i != clientConnections_.End(); ++i)
        i->second_->SendRemoteEvent(eventType, inOrder, eventData);
    for (auto it = clientConnections2_.begin(); it != clientConnections2_.end(); ++it)
        it->second->SendRemoteEvent(eventType, inOrder, eventData);
#endif
}

void Network::BroadcastRemoteEvent(Scene* scene, StringHash eventType, bool inOrder, const VariantMap& eventData)
{
#if !defined(__EMSCRIPTEN__) && !defined(__ANDROID__)
    for (HashMap<SLNet::AddressOrGUID, SharedPtr<Connection> >::Iterator i = clientConnections_.Begin();
         i != clientConnections_.End(); ++i)
    {
        if (i->second_->GetScene() == scene)
            i->second_->SendRemoteEvent(eventType, inOrder, eventData);
    }
    for (auto it = clientConnections2_.begin();
         it != clientConnections2_.end(); ++it)
    {
        if (it->second->GetScene() == scene)
            it->second->SendRemoteEvent(eventType, inOrder, eventData);
    }
#endif
}

void Network::BroadcastRemoteEvent(Node* node, StringHash eventType, bool inOrder, const VariantMap& eventData)
{
    if (!node)
    {
        URHO3D_LOGERROR("Null sender node for remote node event");
        return;
    }
    if (!node->IsReplicated())
    {
        URHO3D_LOGERROR("Sender node has a local ID, can not send remote node event");
        return;
    }

    Scene* scene = node->GetScene();
#if !defined(__EMSCRIPTEN__) && !defined(__ANDROID__)
    for (HashMap<SLNet::AddressOrGUID, SharedPtr<Connection> >::Iterator i = clientConnections_.Begin();
         i != clientConnections_.End(); ++i)
    {
        if (i->second_->GetScene() == scene)
            i->second_->SendRemoteEvent(node, eventType, inOrder, eventData);
    }
    for (auto it = clientConnections2_.begin();
         it != clientConnections2_.end(); ++it)
    {
        if (it->second->GetScene() == scene)
            it->second->SendRemoteEvent(node, eventType, inOrder, eventData);
    }
#endif
}

void Network::SetUpdateFps(int fps)
{
    updateFps_ = Max(fps, 1);
    updateInterval_ = 1.0f / (float)updateFps_;
    updateAcc_ = 0.0f;
}

void Network::SetSimulatedLatency(int ms)
{
    simulatedLatency_ = Max(ms, 0);
    ConfigureNetworkSimulator();
}

void Network::SetSimulatedPacketLoss(float probability)
{
    simulatedPacketLoss_ = Clamp(probability, 0.0f, 1.0f);
    ConfigureNetworkSimulator();
}

void Network::RegisterRemoteEvent(StringHash eventType)
{
    if (blacklistedRemoteEvents_.Find(eventType) != blacklistedRemoteEvents_.End())
    {
        URHO3D_LOGERROR("Attempted to register blacklisted remote event type " + String(eventType));
        return;
    }

    allowedRemoteEvents_.Insert(eventType);
}

void Network::UnregisterRemoteEvent(StringHash eventType)
{
    allowedRemoteEvents_.Erase(eventType);
}

void Network::UnregisterAllRemoteEvents()
{
    allowedRemoteEvents_.Clear();
}

void Network::SetPackageCacheDir(const String& path)
{
    packageCacheDir_ = AddTrailingSlash(path);
}

void Network::SendPackageToClients(Scene* scene, PackageFile* package)
{
    if (!scene)
    {
        URHO3D_LOGERROR("Null scene specified for SendPackageToClients");
        return;
    }
    if (!package)
    {
        URHO3D_LOGERROR("Null package specified for SendPackageToClients");
        return;
    }
#ifndef __EMSCRIPTEN__
    for (HashMap<SLNet::AddressOrGUID, SharedPtr<Connection> >::Iterator i = clientConnections_.Begin();
         i != clientConnections_.End(); ++i)
    {
        if (i->second_->GetScene() == scene)
            i->second_->SendPackageToClient(package);
    }
#endif
}

SharedPtr<HttpRequest> Network::MakeHttpRequest(const String& url, const String& verb, const Vector<String>& headers,
    const String& postData)
{
    URHO3D_PROFILE(MakeHttpRequest);

    // The initialization of the request will take time, can not know at this point if it has an error or not
    SharedPtr<HttpRequest> request(new HttpRequest(url, verb, headers, postData));
    return request;
}

void Network::BanAddress(const String& address)
{
#ifndef __EMSCRIPTEN__
    rakPeer_->AddToBanList(address.CString(), 0);
#endif
}

#ifndef __EMSCRIPTEN__
Connection* Network::GetConnection(const SLNet::AddressOrGUID& connection) const
{
    if (serverConnection_ && serverConnection_->GetAddressOrGUID() == connection)
        return serverConnection_;
    else
    {
        HashMap<SLNet::AddressOrGUID, SharedPtr<Connection> >::ConstIterator i = clientConnections_.Find(connection);
        if (i != clientConnections_.End())
            return i->second_;
        else
            return nullptr;
    }
}
#endif

Connection* Network::GetServerConnection() const
{
    return serverConnection_;
}

Vector<SharedPtr<Connection> > Network::GetClientConnections() const
{
    Vector<SharedPtr<Connection> > ret;
#ifndef __EMSCRIPTEN__
    for (HashMap<SLNet::AddressOrGUID, SharedPtr<Connection> >::ConstIterator i = clientConnections_.Begin();
         i != clientConnections_.End(); ++i)
        ret.Push(i->second_);
#endif

#if !defined(__EMSCRIPTEN__) && !defined(__ANDROID__)
    for (auto i = clientConnections2_.begin();
         i != clientConnections2_.end(); ++i)
        ret.Push(i->second);
#endif

    return ret;
}


bool Network::IsServerRunning() const
{
#ifndef __EMSCRIPTEN__
    if (!rakPeer_)
        return false;
    return rakPeer_->IsActive() && isServer_;
#endif
    return false;
}

bool Network::CheckRemoteEvent(StringHash eventType) const
{
    return allowedRemoteEvents_.Contains(eventType);
}

#ifndef __ANDROID__
void Network::HandleIncomingPacket(struct libwebsocket* socket, VectorBuffer& buffer, bool fromServer)
{
//    VectorBuffer buffer(buff, size);
    // First byte is used by slikenet
    buffer.ReadUByte();

    unsigned int messageID = buffer.ReadUInt();
//    URHO3D_LOGINFOF("MESSAGE ID %u", messageID);
    unsigned dataStart = sizeof(unsigned int) + sizeof(unsigned char);

    MemoryBuffer msg(buffer.GetData() + dataStart, buffer.GetSize() - dataStart);
    if (!fromServer)
    {
        // Only process messages from known sources
        if (clientConnections2_.find(socket) != clientConnections2_.end()) {
            Connection *connection = clientConnections2_[socket];
            if (connection) {
                if (connection->ProcessMessage((int) messageID, msg))
                    return;

                // If message was not handled internally, forward as an event
                using namespace NetworkMessage;

                VariantMap &eventData = GetEventDataMap();
                eventData[P_CONNECTION] = connection;
                eventData[P_MESSAGEID] = (int) messageID;
                eventData[P_DATA].SetBuffer(msg.GetData(), msg.GetSize());
                connection->SendEvent(E_NETWORKMESSAGE, eventData);
            } else
                URHO3D_LOGWARNINGF("Discarding message from unknown MessageConnection %d", socket);
        }
    }
    else
    {
//        MemoryBuffer buffer(buff + dataStart, size - dataStart);
        bool processed = serverConnection_ && serverConnection_->ProcessMessage((int) messageID, msg);
    }
}
#endif

#ifndef __EMSCRIPTEN__
void Network::HandleIncomingPacket(SLNet::Packet* packet, bool isServer)
{
    unsigned char packetID = packet->data[0];
    bool packetHandled = false;

    // Deal with timestamped backents
    unsigned dataStart = sizeof(char);
    if (packetID == ID_TIMESTAMP)
    {
        dataStart += sizeof(SLNet::Time);
        packetID = packet->data[dataStart];
        dataStart += sizeof(char);
    }

    if (packetID == ID_NEW_INCOMING_CONNECTION)
    {
        if (isServer)
        {
            NewConnectionEstablished(packet->systemAddress);
            packetHandled = true;
        }
    }
    else if (packetID == ID_ALREADY_CONNECTED)
    {
        if (natPunchServerAddress_ && packet->systemAddress == *natPunchServerAddress_) {
            URHO3D_LOGINFO("Already connected to NAT server! ");
            if (!isServer)
            {
                natPunchthroughClient_->OpenNAT(*remoteGUID_, *natPunchServerAddress_);
            }
        }
        packetHandled = true;
    }
    else if (packetID == ID_CONNECTION_REQUEST_ACCEPTED) // We're a client, our connection as been accepted
    {
        if(natPunchServerAddress_ && packet->systemAddress == *natPunchServerAddress_) {
            URHO3D_LOGINFO("Succesfully connected to NAT punchtrough server! ");
            SendEvent(E_NATMASTERCONNECTIONSUCCEEDED);
            if (!isServer)
            {
                natPunchthroughClient_->OpenNAT(*remoteGUID_, *natPunchServerAddress_);
            }
        } else {
            if (!isServer)
            {
                OnServerConnected(packet->systemAddress);
            }
        }
        packetHandled = true;
    }
    else if (packetID == ID_NAT_TARGET_NOT_CONNECTED)
    {
        URHO3D_LOGERROR("Target server not connected to NAT master server!");
        packetHandled = true;
    }
    else if (packetID == ID_CONNECTION_LOST) // We've lost connectivity with the packet source
    {
        if (isServer)
        {
            ClientDisconnected(packet->systemAddress);
        }
        else
        {
            OnServerDisconnected(packet->systemAddress);
        }
        packetHandled = true;
    }
    else if (packetID == ID_DISCONNECTION_NOTIFICATION) // We've lost connection with the other side
    {
        if (isServer)
        {
            ClientDisconnected(packet->systemAddress);
        }
        else
        {
            OnServerDisconnected(packet->systemAddress);
        }
        packetHandled = true;
    }
    else if (packetID == ID_CONNECTION_ATTEMPT_FAILED) // We've failed to connect to the server/peer
    {
        if (natPunchServerAddress_ && packet->systemAddress == *natPunchServerAddress_) {
            URHO3D_LOGERROR("Connection to NAT punchtrough server failed!");
            SendEvent(E_NATMASTERCONNECTIONFAILED);

        } else {

            if (!isServer)
            {
                OnServerDisconnected(packet->systemAddress);
            }
        }
        packetHandled = true;
    }
    else if (packetID == ID_NAT_PUNCHTHROUGH_SUCCEEDED)
    {
        SLNet::SystemAddress remotePeer = packet->systemAddress;
        URHO3D_LOGINFO("NAT punchtrough succeeded! Remote peer: " + String(remotePeer.ToString()));
        if (!isServer)
        {
            using namespace NetworkNatPunchtroughSucceeded;
            VariantMap eventMap;
            eventMap[P_ADDRESS] = remotePeer.ToString(false);
            eventMap[P_PORT] = remotePeer.GetPort();
            SendEvent(E_NETWORKNATPUNCHTROUGHSUCCEEDED, eventMap);
            URHO3D_LOGINFO("Connecting to server behind NAT: " + String(remotePeer.ToString()));
            Connect(String(remotePeer.ToString(false)), remotePeer.GetPort(), scene_, identity_);
        }
        packetHandled = true;
    }
    else if (packetID == ID_NAT_PUNCHTHROUGH_FAILED)
    {
        URHO3D_LOGERROR("NAT punchtrough failed!");
        SLNet::SystemAddress remotePeer = packet->systemAddress;
        using namespace NetworkNatPunchtroughFailed;
        VariantMap eventMap;
        eventMap[P_ADDRESS] = remotePeer.ToString(false);
        eventMap[P_PORT] = remotePeer.GetPort();
        SendEvent(E_NETWORKNATPUNCHTROUGHFAILED, eventMap);
        packetHandled = true;
    }
    else if (packetID == ID_CONNECTION_BANNED) // We're a client and we're on the ban list
    {
        URHO3D_LOGERROR("Connection failed, you're banned!");
        SendEvent(E_NETWORKBANNED);
        packetHandled = true;
    }
    else if (packetID == ID_INVALID_PASSWORD) // We're a client, and we gave an invalid password
    {
        URHO3D_LOGERROR("Invalid password provided for connection!");
        SendEvent(E_NETWORKINVALIDPASSWORD);
        packetHandled = true;
    }
    else if (packetID == ID_DOWNLOAD_PROGRESS) // Part of a file transfer
    {
        //URHO3D_LOGINFO("101010");
    }
    else if (packetID == ID_UNCONNECTED_PING)
    {
        packetHandled = true;
    }
    else if (packetID == ID_UNCONNECTED_PONG) // Host discovery response
    {
        if (!isServer)
        {
            using namespace NetworkHostDiscovered;
            
            dataStart += sizeof(SLNet::TimeMS);
            VariantMap& eventMap = context_->GetEventDataMap();
            if (packet->length > packet->length - dataStart) {
                VectorBuffer buffer(packet->data + dataStart, packet->length - dataStart);
                VariantMap srcData = buffer.ReadVariantMap();
                eventMap[P_BEACON] = srcData;
            }
            else {
                eventMap[P_BEACON] = VariantMap();
            }

            eventMap[P_ADDRESS] = String(packet->systemAddress.ToString(false));
            eventMap[P_PORT] = (int)packet->systemAddress.GetPort();
            SendEvent(E_NETWORKHOSTDISCOVERED, eventMap);
        }
        packetHandled = true;
    }

    // Urho3D messages
    if (packetID >= ID_USER_PACKET_ENUM)
    {
        unsigned int messageID = *(unsigned int*)(packet->data + dataStart);
        dataStart += sizeof(unsigned int);

        if (isServer)
        {
            HandleMessage(packet->systemAddress, 0, messageID, (const char*)(packet->data + dataStart), packet->length - dataStart);
        }
        else
        {
            MemoryBuffer buffer(packet->data + dataStart, packet->length - dataStart);
            bool processed = serverConnection_ && serverConnection_->ProcessMessage(messageID, buffer);
            if (!processed)
            {
                HandleMessage(packet->systemAddress, 0, messageID, (const char*)(packet->data + dataStart), packet->length - dataStart);
            }
        }
        packetHandled = true;
    }

    if (!packetHandled && packetID < sizeof(RAKNET_MESSAGEID_STRINGS))
        URHO3D_LOGERROR("Unhandled network packet: " + String(RAKNET_MESSAGEID_STRINGS[packetID]));
    else if (!packetHandled)
        URHO3D_LOGERRORF("Unhandled network packet: %i", packetID);

}
#endif

void Network::Update(float timeStep)
{
    static int oldServerPeerId = 0;
    int sId = 0;
    URHO3D_PROFILE(UpdateNetwork);

#if !defined(__EMSCRIPTEN__) && !defined(__ANDROID__)
    if (wsServer_) {
        wsServer_->Process();
    }
#endif

#ifndef __EMSCRIPTEN__
    //Process all incoming messages for the server
    if (rakPeer_->IsActive())
    {

        while (SLNet::Packet* packet = rakPeer_->Receive())
        {
            HandleIncomingPacket(packet, true);
            rakPeer_->DeallocatePacket(packet);
        }
    }

    // Process all incoming messages for the client
    if (rakPeerClient_->IsActive())
    {
        while (SLNet::Packet* packet = rakPeerClient_->Receive())
        {
            HandleIncomingPacket(packet, false);
            rakPeerClient_->DeallocatePacket(packet);
        }
    }
#endif

#ifndef __ANDROID__
#ifndef __EMSCRIPTEN__
    if (wsServer_) {
        while(wsServer_->GetNumPackets() > 0) {
            WSPacket &p = wsServer_->GetPacket();
            HandleIncomingPacket(p.socket_, p.buffer_, false);
            wsServer_->RemovePacket();
        }
    }
#endif

    if (wsClient_) {
        while(wsClient_->GetNumPackets() > 0) {
            HandleIncomingPacket(nullptr, wsClient_->GetPacket(), true);
            if (wsClient_) {
                wsClient_->RemovePacket();
            }
        }
    }

    if (!eventQueue_.Empty()) {
        MutexLock lock(eventMutex_);
        const StringHash& event = eventQueue_.Front().first_;
        if (event == E_WSSERVERCONNECTED) {
            serverConnection_->SetConnectPending(false);
            URHO3D_LOGINFO("Connected to server!");

            // Send the identity map now
            VectorBuffer msg;
            msg.WriteVariantMap(serverConnection_->GetIdentity());
            serverConnection_->SendMessage(MSG_IDENTITY, true, true, msg);
            SendEvent(E_SERVERCONNECTED);
        } else if (event == E_WSSERVERDISCONNECTED) {
            SendEvent(E_SERVERDISCONNECTED);
            if (wsClient_) {
                delete wsClient_;
                wsClient_ = nullptr;
            }
        }
#ifndef __EMSCRIPTEN__
        else if (event == E_WSCLIENTDISCONNECTED) {
            using namespace WSClientDisconnected;
            struct libwebsocket* socket = static_cast<struct libwebsocket*>(eventQueue_.Front().second_[P_SOCKET].GetVoidPtr());
            ClientDisconnected(socket);
        } else if (event == E_WSCLIENTCONNECTED) {
            using namespace WSClientConnected;
            struct libwebsocket* socket = static_cast<struct libwebsocket*>(eventQueue_.Front().second_[P_SOCKET].GetVoidPtr());
            NewConnectionEstablished(socket);
        }
#endif
        eventQueue_.PopFront();
    }
#endif
}

void Network::PostUpdate(float timeStep)
{
    URHO3D_PROFILE(PostUpdateNetwork);

    // Check if periodic update should happen now
    updateAcc_ += timeStep;
    bool updateNow = updateAcc_ >= updateInterval_;
    if (updateNow)
    {
        // Notify of the impending update to allow for example updated client controls to be set
        SendEvent(E_NETWORKUPDATE);
        updateAcc_ = fmodf(updateAcc_, updateInterval_);

        if (IsServerRunning())
        {
            // Collect and prepare all networked scenes
            {
                URHO3D_PROFILE(PrepareServerUpdate);

                networkScenes_.Clear();
#ifndef __EMSCRIPTEN__
                for (HashMap<SLNet::AddressOrGUID, SharedPtr<Connection> >::Iterator i = clientConnections_.Begin();
                     i != clientConnections_.End(); ++i)
                {
                    Scene* scene = i->second_->GetScene();
                    if (scene)
                        networkScenes_.Insert(scene);
                }
#endif

#ifndef __ANDROID__
                for (auto i = clientConnections2_.begin();
                     i != clientConnections2_.end(); ++i)
                {
                    Scene* scene = i->second->GetScene();
                    if (scene)
                        networkScenes_.Insert(scene);
                }
#endif

                for (HashSet<Scene*>::ConstIterator i = networkScenes_.Begin(); i != networkScenes_.End(); ++i)
                    (*i)->PrepareNetworkUpdate();
            }

            {
                URHO3D_PROFILE(SendServerUpdate);

#ifndef __EMSCRIPTEN__
                // Then send server updates for each client connection
                for (HashMap<SLNet::AddressOrGUID, SharedPtr<Connection> >::Iterator i = clientConnections_.Begin();
                     i != clientConnections_.End(); ++i)
                {
                    i->second_->SendServerUpdate();
                    i->second_->SendRemoteEvents();
                    i->second_->SendPackages();
                    i->second_->SendAllBuffers();
                }
#endif
#ifndef __ANDROID__
                for (auto i = clientConnections2_.begin();
                     i != clientConnections2_.end(); ++i)
                {
                    i->second->SendServerUpdate();
                    i->second->SendRemoteEvents();
                    i->second->SendPackages();
                    i->second->SendAllBuffers();
                }
#endif
            }
        }

        if (serverConnection_)
        {
            // Send the client update
            serverConnection_->SendClientUpdate();
            serverConnection_->SendRemoteEvents();
            serverConnection_->SendAllBuffers();
        }

        // Notify that the update was sent
        SendEvent(E_NETWORKUPDATESENT);
    }
}

void Network::HandleBeginFrame(StringHash eventType, VariantMap& eventData)
{
    using namespace BeginFrame;

    Update(eventData[P_TIMESTEP].GetFloat());
}

void Network::HandleRenderUpdate(StringHash eventType, VariantMap& eventData)
{
    using namespace RenderUpdate;

    PostUpdate(eventData[P_TIMESTEP].GetFloat());
}

#ifndef __EMSCRIPTEN__
void Network::OnServerConnected(const SLNet::AddressOrGUID& address)
{
    serverConnection_->SetConnectPending(false);
    serverConnection_->SetAddressOrGUID(address);
    URHO3D_LOGINFO("Connected to server!");

    // Send the identity map now
    VectorBuffer msg;
    msg.WriteVariantMap(serverConnection_->GetIdentity());
    serverConnection_->SendMessage(MSG_IDENTITY, true, true, msg);
    SendEvent(E_SERVERCONNECTED);
}
#endif

void Network::AddEventToQueue(const StringHash& event, const VariantMap& eventData)
{
    MutexLock lock(eventMutex_);
    eventQueue_.Push({event, eventData});
}

#ifndef __EMSCRIPTEN__
void Network::OnServerDisconnected(const SLNet::AddressOrGUID& address)
{
    if (natPunchServerAddress_ && *natPunchServerAddress_ == address.systemAddress) {
        SendEvent(E_NATMASTERDISCONNECTED);
        return;
    }

    // Differentiate between failed connection, and disconnection
    bool failedConnect = serverConnection_ && serverConnection_->IsConnectPending();
    serverConnection_.Reset();

    if (!failedConnect)
    {
        URHO3D_LOGINFO("Disconnected from server");
        SendEvent(E_SERVERDISCONNECTED);
    }
    else
    {
        URHO3D_LOGERROR("Failed to connect to server");
        SendEvent(E_CONNECTFAILED);
    }
}
#endif

void Network::ConfigureNetworkSimulator()
{
    if (serverConnection_)
        serverConnection_->ConfigureNetworkSimulator(simulatedLatency_, simulatedPacketLoss_);

#ifndef __EMSCRIPTEN__
    for (HashMap<SLNet::AddressOrGUID, SharedPtr<Connection> >::Iterator i = clientConnections_.Begin();
         i != clientConnections_.End(); ++i)
        i->second_->ConfigureNetworkSimulator(simulatedLatency_, simulatedPacketLoss_);
#endif
}

void RegisterNetworkLibrary(Context* context)
{
    NetworkPriority::RegisterObject(context);
}

}
