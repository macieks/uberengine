#ifndef NW_SOCKET_H
#define NW_SOCKET_H

#include "Base/ueBase.h"

struct ueAsync;

/**
 *	@addtogroup nw
 *	@{
 */

#if defined(UE_WIN32) || defined(UE_X360)
	//! Socket handle
	typedef SOCKET nsSocket;
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
	//! Socket handle
	typedef s32 nsSocket;
#endif

#define NS_INVALID_SOCKET			(-1)			//!< Invalid socket

#define NS_IP_ANY					(0)				//!< Any IP
#define NS_IP_BROADCAST				(~0)			//!< LAN broadcast IP

#define NS_AF_INET					AF_INET
#define NS_SOCK_STREAM				SOCK_STREAM		//!< Stream based socket
#define NS_SOCK_DGRAM				SOCK_DGRAM		//!< Datagram based socket
#define NS_IPPROTO_TCP				IPPROTO_TCP		//!< TCP protocol

#define NS_SOCKET_MAX_LISTEN_CONN	SOMAXCONN		//!< Max. number of connections to listen for

#if defined(LINUX)

#define NS_SOCKET_SHUTDOWN_BOTH		SHUT_RDWR

#else

#define NS_SOCKET_SHUTDOWN_RECV		SD_RECEIVE		//!< Shut down socket for receiving
#define NS_SOCKET_SHUTDOWN_SEND		SD_SEND			//!< Shut down socket for sending
#define NS_SOCKET_SHUTDOWN_BOTH		SD_BOTH			//!< Shut down socket for sending and receiving

#endif

//! Possible socket connection states
enum nsSocketConnectionState
{
	nsSocketConnectionState_Establishing = 0,		//!< Connection in progress
	nsSocketConnectionState_Valid,					//!< Connection established
	nsSocketConnectionState_Failed,					//!< Connection failed

	nsSocketConnectionState_MAX
};

//! Ip address
struct nsIP
{
	union
	{
		struct
		{
			u8 sn_b1;
			u8 sn_b2;
			u8 sn_b3;
			u8 sn_b4;
		};
		u32 sn_addr; //!< IP address value
	};

	//! Gets IP address as string; not thread safe
	UE_INLINE const char* ToString() const { return ToString(sn_addr); }
	//! Gets IP address as string; not thread safe
	static const char* ToString(u32 ip);
};

//! Network address
struct nsAddr
{
	u16 m_family;		//!< Address family
	u16 m_port;			//!< Address port
	nsIP m_ip;			//!< Address ip
	u8 reserved[8];		//!< Reserved address data

	//! Gets address as string; not thread safe
	const char* ToString() const;
};

void nsSocket_Startup(ueAllocator* allocator);
void nsSocket_Shutdown();

//! Creates socket
ueBool nsSocket_Create(s32 af, s32 type, s32 protocol, nsSocket* s);
//! Closes socket
ueBool nsSocket_Close(nsSocket s, s32* result = NULL);
//! Shuts down socket
ueBool nsSocket_Shutdown(nsSocket s, s32* result = NULL, s32 how = NS_SOCKET_SHUTDOWN_BOTH);
//! Tells whether socket is valid
ueBool nsSocket_IsValid(nsSocket s);
//! Tells whether there's any bytes pending on receive queue
ueBool nsSocket_IsReceivePending(nsSocket s);
//! Starts connection to given address; for non-blocking sockets use nsSocket_GetConnectionState to check completion state
ueBool nsSocket_Connect(nsSocket s, const nsAddr* addr, s32 addrLen, s32* result = NULL);
//! Gets asynchronous connection state
nsSocketConnectionState nsSocket_GetConnectionState(nsSocket s, s32* result = NULL);
//! Binds the socket
ueBool nsSocket_Bind(nsSocket s, const nsAddr* addr, s32 addrLen, s32* result = NULL);
//! Listens on a socket
ueBool nsSocket_Listen(nsSocket s, s32* result = NULL, u32 maxConnections = NS_SOCKET_MAX_LISTEN_CONN);
//! Accepts connection
ueBool nsSocket_Accept(nsSocket s, nsAddr* addr, s32* addrLen, nsSocket* accepted);
//! Sets socket option
ueBool nsSocket_SetSockOpt(nsSocket s, s32 level, s32 optionName, const void* optionValue, s32 optionLength, s32* result = NULL);
//! Sets non-blocking socket mode
ueBool nsSocket_SetNonBlocking(nsSocket s, ueBool enable, s32* result = NULL);
//! Tells whether socket is non-blocking
ueBool nsSocket_IsNonBlocking(nsSocket s);
//! Tells whether socket would block based on given error
ueBool nsSocket_IsWouldBlock(s32 errorCode);
//! Tells whether socket would block based on last error
ueBool nsSocket_IsWouldBlock();
//! Sets broadcast socket mode
ueBool nsSocket_SetBroadcast(nsSocket s, ueBool enable, s32* result = NULL);
//! Sets up address reusing option on a socket
ueBool nsSocket_SetReuseAddress(nsSocket s, ueBool enable, s32* result = NULL);
//! Sets linger option on a socket
ueBool nsSocket_SetLinger(nsSocket s, ueBool enable, u32 milliseconds, s32* result = NULL);
//! Receives data from a socket
s32 nsSocket_Receive(nsSocket s, void* buffer, s32 length, s32 flags);
//! Receives data from connectionless socket (and gets sender address)
s32 nsSocket_ReceiveFrom(nsSocket s, void* buffer, s32 length, s32 flags, nsAddr* fromAddr, s32* fromAddrLen);
//! Sends data to socket
s32 nsSocket_Send(nsSocket s, const void* buffer, s32 length, s32 flags);
//! Sends data to connection less socket (to specific address)
s32 nsSocket_SendTo(nsSocket s, const void* buffer, s32 length, s32 flags, const nsAddr* toAddr, s32 toAddrLen);
//! Gets socket address
ueBool nsSocket_GetAddress(nsSocket s, nsAddr* addr, s32* addrLen, s32* result = NULL);
//! Gets address info; if async is not-NULL asynchronous query is performed
ueBool nsSocket_GetAddrInfo(const char* addressString, nsIP* ip, ueAsync** async = NULL);
//! Gets last socket error
s32 nsSocket_GetLastError();
//! Gets last socket error as string
const char* nsSocket_GetLastErrorString();
//! Gets socket error as string
const char* nsSocket_GetErrorString(s32 error);

//! Converts host u16 to network byte order
u16 nsSocket_HostToNetU16(u16 v);
//! Converts host s32 to network byte order
s32 nsSocket_HostToNetS32(s32 v);
//! Converts network u16 to host byte order
u16 nsSocket_NetToHostU16(u16 v);
//! Converts network u16 to host byte order
s32 nsSocket_NetToHostS32(s32 v);

// @}

#endif // NW_SOCKET_H