#include "Net/nwSocket.h"
#include "Base/ueJobSys.h"
#include "Base/ueAsync.h"
#include "Base/ueThreading.h"

#if defined(UE_WIN32)
	#include <Ws2tcpip.h>
#elif defined(UE_LINUX) || defined(UE_MAC)
    #include <fcntl.h>
#endif

const char* nsIP::ToString(u32 _ip)
{
	nsIP ip;
	ip.sn_addr = _ip;

	static char buffer[32];
	ueStrFormatS(buffer, "%u.%u.%u.%u", (u32) ip.sn_b1, (u32) ip.sn_b2, (u32) ip.sn_b3, (u32) ip.sn_b4);
	return buffer;
}

const char* nsAddr::ToString() const
{
	static char buffer[64];
	ueStrFormatS(buffer, "%s:%u", m_ip.ToString(), (u32) nsSocket_NetToHostU16(m_port));
	return buffer;
}

static ueAllocator* s_allocator = NULL;
static ueMutex* s_getAddrMutex = NULL;

void nsSocket_Startup(ueAllocator* allocator)
{
	UE_ASSERT(!s_allocator);
	s_allocator = allocator;
	s_getAddrMutex = ueMutex_Create();

#if defined(UE_WIN32) || defined(UE_X360)
	WORD version = MAKEWORD(2, 2);
	WSADATA wsaData;

	UE_ASSERT_FUNC(WSAStartup(version, &wsaData));
#endif // defined(UE_WIN32) || defined(UE_X360)
}

void nsSocket_Shutdown()
{
	UE_ASSERT(s_allocator);

#if defined(UE_WIN32) || defined(UE_X360)
	WSACleanup();
#endif // defined(UE_WIN32) || defined(UE_X360)

	ueMutex_Destroy(s_getAddrMutex);
	s_allocator = NULL;
}

ueBool nsSocket_Create(s32 af, s32 type, s32 protocol, nsSocket* s)
{
	return (*s = socket(af, type, protocol)) != NS_INVALID_SOCKET;
}

ueBool nsSocket_Close(nsSocket s, s32* result)
{
	s32 r; if (!result) result = &r;
#if defined(UE_WIN32) || defined(UE_X360)
	return (*result = closesocket(s)) == 0;
#elif defined(UE_PS3)
	return (*result = socketclose(s)) == 0;
#elif defined(UE_LINUX) || defined(UE_MAC)
    return (*result = close(s)) == 0;
#endif
}

ueBool nsSocket_Shutdown(nsSocket s, s32* result, s32 how)
{
	s32 r; if (!result) result = &r;
	return (*result = shutdown(s, how)) == 0;
}

ueBool nsSocket_IsValid(nsSocket s)
{
	return s && s != NS_INVALID_SOCKET;
}

ueBool nsSocket_IsReceivePending(nsSocket s)
{
	fd_set readSet;
	FD_ZERO(&readSet);
	FD_SET(s, &readSet);

	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	return select(0, &readSet, NULL, NULL, &timeout) == 1;
}

ueBool nsSocket_Connect(nsSocket s, const nsAddr* addr, s32 addrLen, s32* result)
{
	s32 r; if (!result) result = &r;
	return (*result = connect(s, (const sockaddr*) addr, addrLen)) == 0;
}

nsSocketConnectionState nsSocket_GetConnectionState(nsSocket s, s32* result)
{
	s32 r; if (!result) result = &r;
	
	fd_set readSet, writeSet;
	FD_ZERO(&readSet);
	FD_SET(s, &readSet);
	FD_ZERO(&writeSet);
	FD_SET(s, &writeSet);

	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	const s32 readWriteReady = select(s + 1, &readSet, &writeSet, NULL, &timeout);
	if (readWriteReady < 0)
		return nsSocketConnectionState_Failed;
	if (readWriteReady == 0)
		return nsSocketConnectionState_Establishing;
	return nsSocketConnectionState_Valid;
}

ueBool nsSocket_Bind(nsSocket s, const nsAddr* addr, s32 addrLen, s32* result)
{
	s32 r; if (!result) result = &r;
	return (*result = bind(s, (const sockaddr*) addr, addrLen)) == 0;
}

ueBool nsSocket_Listen(nsSocket s, s32* result, u32 maxConnections)
{
	s32 r; if (!result) result = &r;
	return (*result = listen(s, maxConnections)) == 0;
}

ueBool nsSocket_Accept(nsSocket s, nsAddr* addr, s32* addrLen, nsSocket* accepted)
{
	return (*accepted = accept(s, (sockaddr*) addrLen, (socklen_t*) addrLen)) != NS_INVALID_SOCKET;
}

ueBool nsSocket_SetSockOpt(nsSocket s, s32 level, s32 optionName, const void* optionValue, s32 optionLength, s32* result)
{
	s32 r; if (!result) result = &r;
	return (*result = setsockopt(s, level, optionName, (const char*) optionValue, optionLength)) == 0;
}

ueBool nsSocket_SetNonBlocking(nsSocket s, ueBool enable, s32* result)
{
	s32 r; if (!result) result = &r;
	unsigned long value = enable ? 1 : 0;
#if defined(UE_LINUX) || defined(UE_MAC)
	return (*result = fcntl(s, O_NONBLOCK, &value)) == 0;
#else
	return (*result = ioctlsocket(s, FIONBIO, &value)) == 0;
#endif
}

ueBool nsSocket_IsNonBlocking(nsSocket s)
{
	UE_NOT_IMPLEMENTED();
	return UE_TRUE;
}

ueBool nsSocket_IsWouldBlock(s32 errorCode)
{
#if defined(UE_WIN32) || defined(UE_X360)
	return errorCode == WSAEWOULDBLOCK;
#else
	return errorCode == EWOULDBLOCK;
#endif
}

ueBool nsSocket_IsWouldBlock()
{
	return nsSocket_IsWouldBlock( nsSocket_GetLastError() );
}

ueBool nsSocket_SetBroadcast(nsSocket s, ueBool enable, s32* result)
{
	const s32 value = enable ? 1 : 0;
	return nsSocket_SetSockOpt(s, SOL_SOCKET, SO_BROADCAST, &value, sizeof(value), result);
}

ueBool nsSocket_SetReuseAddress(nsSocket s, ueBool enable, s32* result)
{
	const s32 value = enable ? 1 : 0;
	return nsSocket_SetSockOpt(s, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value), result);
}

ueBool nsSocket_SetLinger(nsSocket s, ueBool enable, u32 milliseconds, s32* result)
{
#ifdef WIN32
	linger ling;
#else
	LINGER ling;
#endif
	ling.l_onoff = enable ? 1 : 0;
	ling.l_linger = (u_short) milliseconds;
	return nsSocket_SetSockOpt(s, SOL_SOCKET, SO_LINGER, (char*) &ling, sizeof(linger), result);
}

s32 nsSocket_Receive(nsSocket s, void* buffer, s32 length, s32 flags)
{
	return recv(s, (char*) buffer, length, flags);
}

s32 nsSocket_ReceiveFrom(nsSocket s, void* buffer, s32 length, s32 flags, nsAddr* fromAddr, s32* fromAddrLen)
{
	return recvfrom(s, (char*) buffer, length, flags, (sockaddr*) fromAddr, (socklen_t*) fromAddrLen);
}

s32 nsSocket_Send(nsSocket s, const void* buffer, s32 length, s32 flags)
{
	return send(s, (char*) buffer, length, flags);
}

s32 nsSocket_SendTo(nsSocket s, const void* buffer, s32 length, s32 flags, const nsAddr* toAddr, s32 toAddrLen)
{
	return sendto(s, (char*) buffer, length, flags, (const sockaddr*) toAddr, (socklen_t) toAddrLen);
}

ueBool nsSocket_GetAddress(nsSocket s, nsAddr* addr, s32* addrLen, s32* result)
{
	s32 r; if (!result) result = &r;
	return (*result = getsockname(s, (sockaddr*) addr, (socklen_t*) addrLen)) == 0;
}

struct nsGetAddrInfo_AsyncData
{
	const char* m_addressString;
	nsIP* m_ip;
};

void nsSocket_GetAddrInfo_WorkFunc(ueAsync* async, void* userData)
{
	nsGetAddrInfo_AsyncData* data = (nsGetAddrInfo_AsyncData*) userData;
	const ueBool result = nsSocket_GetAddrInfo(data->m_addressString, data->m_ip, NULL);
	ueAsync_SetDone(async, result);
}

void nsSocket_GetAddrInfo_DestroyFunc(ueAsync* async, void* userData)
{
	nsGetAddrInfo_AsyncData* data = (nsGetAddrInfo_AsyncData*) userData;
	ueDelete(data, s_allocator);
}

ueBool nsSocket_GetAddrInfo(const char* addressString, nsIP* ip, ueAsync** asyncPtr)
{
	// Asynchronous way

	if (asyncPtr)
	{
		nsGetAddrInfo_AsyncData* data = new(s_allocator) nsGetAddrInfo_AsyncData;
		data->m_addressString = addressString;
		data->m_ip = ip;

		ueJobDesc jobDesc;
		jobDesc.m_workFunc = nsSocket_GetAddrInfo_WorkFunc;
		jobDesc.m_userData = data;
		jobDesc.m_destroyFunc = nsSocket_GetAddrInfo_DestroyFunc;
		*asyncPtr = thGlobalJobSys_StartJob(&jobDesc);
		if (!*asyncPtr)
		{
			ueDelete(data, s_allocator);
			return UE_FALSE;
		}

		return UE_TRUE;
	}

	// Synchronous way

#if defined(UE_WIN32)

	// Prevent multiple calls to getaddrinfo (not thread safe!)

	ueMutexLock lock(s_getAddrMutex);

	// Lookup IP address

	PADDRINFOA result;
	if (!getaddrinfo(addressString, NULL, NULL, &result))
	{
		sockaddr_in* addr = (sockaddr_in*) result->ai_addr;
		ip->sn_addr = addr->sin_addr.S_un.S_addr;
		return UE_TRUE;
	}
	return UE_FALSE;
#else
	UE_NOT_IMPLEMENTED();
	return UE_FALSE;
#endif
}

u16 nsSocket_HostToNetU16(u16 v)
{
	return htons(v);
}

u32 nsSocket_HostToNetS32(u32 v)
{
	return htonl(v);
}

u16 nsSocket_NetToHostU16(u16 v)
{
	return htons(v);
}

u32 nsSocket_NetToHostS32(u32 v)
{
	return htonl(v);
}

s32 nsSocket_GetLastError()
{
#if defined(UE_WIN32) || defined(UE_X360)
	return WSAGetLastError();
#else
	return errno;
#endif
}

const char* nsSocket_GetLastErrorString()
{
	return nsSocket_GetErrorString(nsSocket_GetLastError());
}

const char* nsSocket_GetErrorString(s32 error)
{
#define CASE_ERR(e) case e: return #e;
	switch (error)
	{
#if defined(UE_WIN32) || defined(UE_X360)
		/*
		 * Windows Sockets definitions of regular Microsoft C error constants
		 */
		CASE_ERR(WSAEINTR)
		CASE_ERR(WSAEBADF)
		CASE_ERR(WSAEACCES)
		CASE_ERR(WSAEFAULT)
		CASE_ERR(WSAEINVAL)
		CASE_ERR(WSAEMFILE)

		/*
		 * Windows Sockets definitions of regular Berkeley error constants
		 */
		CASE_ERR(WSAEWOULDBLOCK)
		CASE_ERR(WSAEINPROGRESS)
		CASE_ERR(WSAEALREADY)
		CASE_ERR(WSAENOTSOCK)
		CASE_ERR(WSAEDESTADDRREQ)
		CASE_ERR(WSAEMSGSIZE)
		CASE_ERR(WSAEPROTOTYPE)
		CASE_ERR(WSAENOPROTOOPT)
		CASE_ERR(WSAEPROTONOSUPPORT)
		CASE_ERR(WSAESOCKTNOSUPPORT)
		CASE_ERR(WSAEOPNOTSUPP)
		CASE_ERR(WSAEPFNOSUPPORT)
		CASE_ERR(WSAEAFNOSUPPORT)
		CASE_ERR(WSAEADDRINUSE)
		CASE_ERR(WSAEADDRNOTAVAIL)
		CASE_ERR(WSAENETDOWN)
		CASE_ERR(WSAENETUNREACH)
		CASE_ERR(WSAENETRESET)
		CASE_ERR(WSAECONNABORTED)
		CASE_ERR(WSAECONNRESET)
		CASE_ERR(WSAENOBUFS)
		CASE_ERR(WSAEISCONN)
		CASE_ERR(WSAENOTCONN)
		CASE_ERR(WSAESHUTDOWN)
		CASE_ERR(WSAETOOMANYREFS)
		CASE_ERR(WSAETIMEDOUT)
		CASE_ERR(WSAECONNREFUSED)
		CASE_ERR(WSAELOOP)
		CASE_ERR(WSAENAMETOOLONG)
		CASE_ERR(WSAEHOSTDOWN)
		CASE_ERR(WSAEHOSTUNREACH)
		CASE_ERR(WSAENOTEMPTY)
		CASE_ERR(WSAEPROCLIM)
		CASE_ERR(WSAEUSERS)
		CASE_ERR(WSAEDQUOT)
		CASE_ERR(WSAESTALE)
		CASE_ERR(WSAEREMOTE)

		CASE_ERR(WSAEDISCON)

		/*
		 * Extended Windows Sockets error constant definitions
		 */
		CASE_ERR(WSASYSNOTREADY)
		CASE_ERR(WSAVERNOTSUPPORTED)
		CASE_ERR(WSANOTINITIALISED)
#else

#endif
		default:
		{
			static char buffer[32];
			ueStrFormatS(buffer, "<unknown = %u>", error);
			return buffer;
		}
	}
}
