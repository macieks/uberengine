#include "Net/ENet/mpENet_Private.h"

const char* enet_socket_last_error_str()
{
	return "unknown";
}

int enet_socket_get_addr(ENetSocket socket, ENetAddress* enetAddr)
{
	nsAddr addr;
	s32 len = sizeof(addr);
	s32 result = nsSocket_GetAddress((nsSocket) socket, &addr, &len);
	if (result == 0)
	{
		enetAddr->host = addr.m_ip.sn_addr;
		enetAddr->port = nsSocket_NetToHostU16(addr.m_port);
	}
	return result;
}

bool operator == (const ENetAddress& a, const ENetAddress& b)
{
	return a.host == b.host && a.port == b.port;
}
