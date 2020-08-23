#ifndef __ANDROID__

#include <signal.h>

#include <cstdlib>
#include <cstring>

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <fstream>
#include <algorithm>

#ifdef _WIN32
#	define WIN32_LEAN_AND_MEAN
#	define NOMINMAX
#	include <windows.h>
#else  // posix
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <arpa/inet.h>

#	include <sys/types.h>
#	include <sys/stat.h>
#	include <unistd.h>
#endif

#include <IO/MemoryBuffer.h>

#include "libsocket.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#ifdef __EMSCRIPTEN__
#include "libwebsockets_asmjs.h"
#else
#include "libwebsockets_native.h"
#endif

#include "WSClient.h"
#include "../../IO/Log.h"
#include "../Network.h"
#include "WSEvents.h"

using namespace Urho3D;

struct internal_socket_t {
    bool closing;
    struct libwebsocket *wsi;
};

static const char* WS_MESSAGE_STRINGS[] = {
        "LWS_CALLBACK_ESTABLISHED",
        "LWS_CALLBACK_CLIENT_CONNECTION_ERROR",
        "LWS_CALLBACK_CLIENT_FILTER_PRE_ESTABLISH",
        "LWS_CALLBACK_CLIENT_ESTABLISHED",
        "LWS_CALLBACK_CLOSED",
        "LWS_CALLBACK_CLOSED_HTTP",
        "LWS_CALLBACK_RECEIVE",
        "LWS_CALLBACK_CLIENT_RECEIVE",
        "LWS_CALLBACK_CLIENT_RECEIVE_PONG",
        "LWS_CALLBACK_CLIENT_WRITEABLE",
        "LWS_CALLBACK_SERVER_WRITEABLE",
        "LWS_CALLBACK_HTTP",
        "LWS_CALLBACK_HTTP_BODY",
        "LWS_CALLBACK_HTTP_BODY_COMPLETION",
        "LWS_CALLBACK_HTTP_FILE_COMPLETION",
        "LWS_CALLBACK_HTTP_WRITEABLE",
        "LWS_CALLBACK_FILTER_NETWORK_CONNECTION",
        "LWS_CALLBACK_FILTER_HTTP_CONNECTION",
        "LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED",
        "LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION",
        "LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS",
        "LWS_CALLBACK_OPENSSL_LOAD_EXTRA_SERVER_VERIFY_CERTS",
        "LWS_CALLBACK_OPENSSL_PERFORM_CLIENT_CERT_VERIFICATION",
        "LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER",
        "LWS_CALLBACK_CONFIRM_EXTENSION_OKAY",
        "LWS_CALLBACK_CLIENT_CONFIRM_EXTENSION_SUPPORTED",
        "LWS_CALLBACK_PROTOCOL_INIT",
        "LWS_CALLBACK_PROTOCOL_DESTROY",
        "LWS_CALLBACK_WSI_CREATE",
        "LWS_CALLBACK_WSI_DESTROY",
        "LWS_CALLBACK_GET_THREAD_ID",
        "LWS_CALLBACK_ADD_POLL_FD",
        "LWS_CALLBACK_DEL_POLL_FD",
        "LWS_CALLBACK_CHANGE_MODE_POLL_FD",
        "LWS_CALLBACK_LOCK_POLL",
        "LWS_CALLBACK_UNLOCK_POLL",
        "LWS_CALLBACK_USER",
};

//static std::unique_ptr<Server> WSClient;
static WSClient* WSClientInstance = nullptr;

int callback_client(struct libwebsocket_context *context
				  , struct libwebsocket *wsi
				  , enum libwebsocket_callback_reasons reason
				  , void *user, void *in, size_t len) {

//    if (reason != 30) {
//        URHO3D_LOGINFOF("callback_client reason=%d, name=%s", reason, WS_MESSAGE_STRINGS[reason]);
//    }
	switch (reason) {
//	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
//		break;
//
//	case LWS_CALLBACK_FILTER_NETWORK_CONNECTION:
//		break;
//
//	case LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED:
//		break;
//
//	case LWS_CALLBACK_PROTOCOL_INIT:
//		break;
//
//	case LWS_CALLBACK_PROTOCOL_DESTROY:
//		// we don't care
//		break;
//
//	case LWS_CALLBACK_WSI_CREATE:
//		break;
//
	case LWS_CALLBACK_WSI_DESTROY:
	    if (WSClientInstance) {
            WSClientInstance->Stop();
        }
		break;
//
//	case LWS_CALLBACK_GET_THREAD_ID:
//		// ignore
//		break;
//
//	case LWS_CALLBACK_ADD_POLL_FD:
//		break;
//
//	case LWS_CALLBACK_DEL_POLL_FD:
//		break;
//
//	case LWS_CALLBACK_CHANGE_MODE_POLL_FD:
//		break;
//
//	case LWS_CALLBACK_LOCK_POLL:
//		break;
//
//	case LWS_CALLBACK_UNLOCK_POLL:
//		break;

//    case LWS_CALLBACK_HTTP:

//        break;

    case LWS_CALLBACK_CLIENT_RECEIVE: {
        Urho3D::VectorBuffer b(in, len);
        if (WSClientInstance) {
            WSClientInstance->AddPacket(b);
        }
        break;
    }

    case LWS_CALLBACK_CLIENT_ESTABLISHED: {
        URHO3D_LOGINFO("Connected to web server!");
        if (WSClientInstance) {
            WSClientInstance->GetNetworkInstance()->AddEventToQueue(E_WSSERVERCONNECTED);
        }
        break;
    }

    case LWS_CALLBACK_CLOSED: {
        URHO3D_LOGINFOF("Disconnected from server [WSClient]");
        if (WSClientInstance) {
            WSClientInstance->Stop();
        }
    }

	default:
//		URHO3D_LOGWARNINGF("callback_client %p %p %u %p %p %u\n", context, wsi, reason, user, in, len);
		break;
	}

	return 0;
}

// TODO: would one callback be enough?
struct libwebsocket_protocols protocols3[] = {
	  { "default", callback_client, 1 }
	, { NULL, NULL, 0 }
};

WSClient::WSClient(Urho3D::Network* networkInstance):
networkInstance_(networkInstance)
{
    WSClientInstance = this;
}

WSClient::~WSClient()
{
    WSClientInstance = nullptr;
}

//int main(int argc, char *argv[]) {
internal_socket_t* WSClient::Connect(const String& address, Scene* scene, const VariantMap& identity)
{

    URHO3D_LOGINFO("Network::WSConnect");
    internal_callbacks_t callbacks;
//    callbacks.on_writable = OnWSWriteable;
//    callbacks.on_connect = OnWSConnect;
//    callbacks.on_data = OnWSData;
//    callbacks.on_destroy = OnWSDestroyed;
    context = internal_init_custom_protocol(&callbacks, protocols3);
    socket_ = internal_connect_websocket(address.CString(), "default");
//    internal_set_callbacks(socket, &callbacks);

    return socket_;
}

int WSClient::Disconnect()
{
    Urho3D::MutexLock lock(GetMutex());
#ifdef __EMSCRIPTEN__
//    EM_ASM({
//      Module.__libwebsocket.sockets.get(1).close();
//    });
#else
    URHO3D_LOGINFO("WSClient::Disconnect 111");
    if (!socket_->closing) {
        URHO3D_LOGINFO("WSClient::Disconnect 222");
        internal_close_socket(socket_);
    }
    if (context) {
        internal_deinit(context);
    }
#endif
}

void WSClient::Stop()
{
    Urho3D::MutexLock lock(GetMutex());
    context = nullptr;
    WSClientInstance = nullptr;
    GetNetworkInstance()->AddEventToQueue(E_WSSERVERDISCONNECTED);
}

void WSClient::Process()
{
//    if (initialized_) {
//        libwebsocket_service(context, 200);
//    }
}

void WSClient::AddPacket(VectorBuffer& packet)
{
    Urho3D::MutexLock lock(GetMutex());
    packets_.Push(packet);
}

VectorBuffer& WSClient::GetPacket()
{
    return packets_.Front();
}

void WSClient::RemovePacket()
{
    Urho3D::MutexLock lock(GetMutex());
    packets_.PopFront();
}

#endif // __ANDROID__