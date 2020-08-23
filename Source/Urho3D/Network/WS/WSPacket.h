#ifndef __ANDROID__
#pragma once

#include "../../IO/VectorBuffer.h"
namespace Urho3D {
struct WSPacket {
    WSPacket(struct libwebsocket* socket, VectorBuffer& buffer):
    socket_(socket),
    buffer_(buffer)
    {
    }
    struct libwebsocket* socket_;
    VectorBuffer buffer_;
};
}
#endif