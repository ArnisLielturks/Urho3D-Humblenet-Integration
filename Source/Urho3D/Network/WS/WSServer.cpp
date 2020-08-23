#if !defined(__EMSCRIPTEN__) && !defined(__ANDROID__)
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

#include <libwebsockets.h>
#include <IO/MemoryBuffer.h>

#include "WSServer.h"
#include "../../IO/Log.h"
#include "../Network.h"
#include "WSEvents.h"

using namespace Urho3D;

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

//static std::unique_ptr<Server> WSServer;
static WSServer* WSServerInstance = nullptr;

int callback_server(struct libwebsocket_context *context, struct libwebsocket *wsi,
                    enum libwebsocket_callback_reasons reason, void *user, void *in, size_t len)
{

//    if (reason != 30) {
//        URHO3D_LOGINFOF("callback_default reason=%d, name=%s", reason, WS_MESSAGE_STRINGS[reason]);
//    }
	switch (reason) {
	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		break;

	case LWS_CALLBACK_FILTER_NETWORK_CONNECTION:
		break;

	case LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED:
		break;

	case LWS_CALLBACK_PROTOCOL_INIT:
		break;

	case LWS_CALLBACK_PROTOCOL_DESTROY:
		// we don't care
		break;

	case LWS_CALLBACK_WSI_CREATE:
		break;

	case LWS_CALLBACK_WSI_DESTROY:
		break;

	case LWS_CALLBACK_GET_THREAD_ID:
		// ignore
		break;

	case LWS_CALLBACK_ADD_POLL_FD:
		break;

	case LWS_CALLBACK_DEL_POLL_FD:
		break;

	case LWS_CALLBACK_CHANGE_MODE_POLL_FD:
		break;

	case LWS_CALLBACK_LOCK_POLL:
		break;

	case LWS_CALLBACK_UNLOCK_POLL:
		break;

    case LWS_CALLBACK_HTTP:
        break;

    case LWS_CALLBACK_RECEIVE: {
        Urho3D::VectorBuffer b(in, len);
        WSPacket p(wsi, b);
        if (WSServerInstance) {
            WSServerInstance->AddPacket(p);
        }
        break;
    }


    case LWS_CALLBACK_ESTABLISHED: {
//        struct sockaddr_storage addr;
//        socklen_t len = sizeof(addr);
//        size_t bufsize = std::max(INET_ADDRSTRLEN, INET6_ADDRSTRLEN) + 1;
//        std::vector<char> ipstr(bufsize, 0);
//        int port;
//
//        int socket = libwebsocket_get_socket_fd(wsi);
//        getpeername(socket, (struct sockaddr*)&addr, &len);
//
//        if (addr.ss_family == AF_INET) {
//            struct sockaddr_in *s = (struct sockaddr_in*)&addr;
//            port = ntohs(s->sin_port);
//            inet_ntop(AF_INET, &s->sin_addr, &ipstr[0], INET_ADDRSTRLEN);
//        }
//        else { // AF_INET6
//            struct sockaddr_in6 *s = (struct sockaddr_in6*)&addr;
//            port = ntohs(s->sin6_port);
//            inet_ntop(AF_INET6, &s->sin6_addr, &ipstr[0], INET6_ADDRSTRLEN);
//        }
//
//        std::string url = std::string(&ipstr[0]);
//        url += std::string(":");
//        url += std::to_string(port);

//        if (wsi) {
//            int len = lws_hdr_total_length(wsi, WSI_TOKEN_HTTP_X_FORWARDED_FOR);
//            if (len) {
//                char header[50];
//                lws_hdr_copy(wsi, header, 50, WSI_TOKEN_HTTP_X_FORWARDED_FOR);
//                header[len] = '\0';
//                LOG_INFO("Rewriting proxy IP (%s) with the real one (%s)", conn->url.c_str(), header);
//                conn->url = header;
//            }
//        }
//        if (wsi) {
//            int len = lws_hdr_total_length(wsi, WSI_TOKEN_HTTP_X_REAL_IP);
//            if (len) {
//                char header[50];
//                lws_hdr_copy(wsi, header, 50, WSI_TOKEN_HTTP_X_REAL_IP);
//                header[len] = '\0';
//                conn->url = header;
//                LOG_INFO("Rewriting proxy IP (%s) with the real one (%s)", conn->url.c_str(), header);
//            }
//        }
//        if (wsi) {
//            int len = lws_hdr_total_length(wsi, WSI_TOKEN_HTTP_X_REAL_PORT);
//            if (len) {
//                char header[50];
//                lws_hdr_copy(wsi, header, 50, WSI_TOKEN_HTTP_X_REAL_IP);
//                header[len] = '\0';
//                LOG_INFO("Rewriting proxy port (%s) with the real one (%s)", std::to_string(port).c_str(), header);
//                conn->url += ":" + std::string(header);
//            }
//        }

//        URHO3D_LOGINFOF("New connection from \"%s\"\n", url.c_str());

        if (WSServerInstance) {
            using namespace WSClientConnected;
            VariantMap data;
            data[P_SOCKET] = wsi;
            WSServerInstance->GetNetworkInstance()->AddEventToQueue(E_WSCLIENTCONNECTED, data);
        }

        break;
    }

    case LWS_CALLBACK_CLOSED:
        URHO3D_LOGINFO("Client disconnected");
        if (WSServerInstance) {
            using namespace WSClientDisconnected;
            VariantMap data;
            data[P_SOCKET] = wsi;
            WSServerInstance->GetNetworkInstance()->AddEventToQueue(E_WSCLIENTDISCONNECTED, data);
        }
        break;

	default:
		URHO3D_LOGWARNINGF("callback_default %p %p %u %p %p %u\n", context, wsi, reason, user, in, len);
		break;
	}

	return 0;
}

// TODO: would one callback be enough?
struct libwebsocket_protocols protocols2[] = {
	  { "default", callback_server, 1 }
	, { NULL, NULL, 0 }
};

WSServer::WSServer(Urho3D::Network* networkInstance):
networkInstance_(networkInstance)
{
    WSServerInstance = this;
}

WSServer::~WSServer()
{
    WSServerInstance = nullptr;
}

//int main(int argc, char *argv[]) {
int WSServer::Start()
{
	struct lws_context_creation_info info;
	memset(&info, 0, sizeof(info));
	info.protocols = protocols2;
	info.port = 9090;//config.port;
	info.gid = -1;
	info.uid = -1;
//#define WS_OPT(member, var) info.member = (!var.empty()) ? var.c_str() : NULL
//	WS_OPT(iface, config.iface);
//	WS_OPT(ssl_cert_filepath, config.sslCertFile);
//	WS_OPT(ssl_private_key_filepath, config.sslPrivateKeyFile);
//	WS_OPT(ssl_ca_filepath, config.sslCACertFile);
//	WS_OPT(ssl_cipher_list, config.sslCipherList);
//    WS_OPT(http_proxy_address, config.httpProxyAddress);
//#undef WS_OPT

	context = libwebsocket_create_context(&info);


	initialized_ = true;
	return 0;
}

int WSServer::Stop()
{
    libwebsocket_context_destroy(context);
    initialized_ = false;
}

void WSServer::Process()
{
    if (initialized_) {
        libwebsocket_service(context, 0);
    }
}

void WSServer::AddPacket(WSPacket& packet)
{
    Urho3D::MutexLock lock(GetMutex());
    packets_.push_back(packet);
}

WSPacket& WSServer::GetPacket()
{
    return packets_.front();
}

void WSServer::RemovePacket()
{
    if (GetNumPackets() > 0) {
        Urho3D::MutexLock lock(GetMutex());
        packets_.front().buffer_.Clear();
        packets_.pop_front();
    }
}

#endif
