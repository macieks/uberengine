#ifndef NW_TCP_PACKET_SOCKET_H
#define NW_TCP_PACKET_SOCKET_H

/**
 *	@addtogroup nw
 *	@{
 */

#include "Net/nwSocket.h"

/**
 *	@struct nwTCPPacketSocket
 *	@brief TCP socket helper used to send and receive whole packets (all or nothing) - 
 *		something that isn't normally supported by (stream oriented) TCP sockets.
 *
 *	@note for all nwTCPPacketSocket functions that return socketErr:
 *		- returned socketErr is always either 0 or negative
 *		- 0 indicates no critical socket error (the socket is still valid)
 *		- when nwTCPPacketSocket_Send or nwTCPPacketSocket_Recv fails and socketErr is 0 it means there was not enough room in either:
 *			- custom buffer
 *			- internal socket buffer (got wouldblock error from send/recv)
 *		- negative numbers are standard socket errors
 */
struct nwTCPPacketSocket;

//! Creates TCP packet socket helper
nwTCPPacketSocket* nwTCPPacketSocket_Create(ueAllocator* allocator, u32 sendBufferSize, u32 recvBufferSize);
//! Destroys TCP packet socket helper
void nwTCPPacketSocket_Destroy(nwTCPPacketSocket* socket);

//! Resets TCP packet socket helper with socket handle
void nwTCPPacketSocket_Reset(nwTCPPacketSocket* socket, nsSocket sock);
//! Gets socket handle
nsSocket nwTCPPacketSocket_GetSocket(nwTCPPacketSocket* socket);
//! Tells whether there was some critical socket error
ueBool nwTCPPacketSocket_IsError(nwTCPPacketSocket* socket, s32& socketErr);

//! Sends the data; on success whole buffer is sent, on failure nothing is sent; note, apart from calling nwTCPPacketSocket_Send, it's required to call nwTCPPacketSocket_TickSend regularly to make sure all (buffered) messages are eventually sent
ueBool nwTCPPacketSocket_Send(nwTCPPacketSocket* socket, const void* data, u32 dataSize, s32& socketErr);
//! Sends array of data chunks; on success all buffers are sent, on failure none is sent; note, apart from calling nwTCPPacketSocket_Send, it's required to call nwTCPPacketSocket_TickSend regularly to make sure all (buffered) messages are eventually sent
ueBool nwTCPPacketSocket_SendArray(nwTCPPacketSocket* socket, u32 numChunks, const void** dataArray, const u32* dataSizeArray, s32& socketErr);
//! Ticks internal sending of pending messages
void nwTCPPacketSocket_TickSend(nwTCPPacketSocket* socket, s32& socketErr);
//! Gets last time of successful send
ueTime nwTCPPacketSocket_GetLastSendTime(nwTCPPacketSocket* socket);

//! Tells whether nwTCPPacketSocket_Recv with specific number of bytes would succeed; on success dataPtr is set to the - already received - data
ueBool nwTCPPacketSocket_WouldRecv(nwTCPPacketSocket* socket, const void** dataPtr, u32 numBytes, s32& socketErr);
//! Receives the data; on success whole buffer is received, on failure nothing is received
ueBool nwTCPPacketSocket_Recv(nwTCPPacketSocket* socket, void* buffer, u32 numBytes, s32& socketErr);
//! Gets last time of successful receive
ueTime nwTCPPacketSocket_GetLastRecvTime(nwTCPPacketSocket* socket);

// @}

#endif // NW_TCP_PACKET_SOCKET_H