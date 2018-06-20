#ifndef NS_DEBUG_CONN_H
#define NS_DEBUG_CONN_H

/**
 *	@addtogroup nw
 *	@{
 */

WORK IN PROGRESS

#define DEBUG_NET_ALL_CONNECTIONS_ID		(~0)
#define DEBUG_NET_DEFAULT_BROADCAST_PORT	13579

struct DebugNetConnection
{
	u32 m_id;
	u32 m_appSymbol;
	char m_computerName[64];
	bool m_isValid;
	u32 m_numQueuedMsgs;
};

typedef void (*DebugNetRecvFunc)(const DebugNetConnection& conn, u32 msgSymbol, void* userData);
typedef void (*DebugNetConnectedFunc)(const DebugNetConnection& conn, void* userData);
typedef void (*DebugNetDisconnectedFunc)(const DebugNetConnection& conn, void* userData);

struct DebugNetStartupParams
{
	Allocator* m_allocator;

	u32 m_maxConnections;
	f32 m_connectionTimeOutSecs;

	unsigned short m_broadcastPort;
	float m_broadcastFreqSecs;

	u32 m_syncMsgQueueSize;

	const char* m_appName;
	u32 m_appSymbol;
	u32 m_appVersion;

	DebugNetStartupParams() :
		m_allocator(NULL),
		m_maxConnections(4),
		m_connectionTimeOutSecs(60.0f),
		m_broadcastPort(DEBUG_NET_DEFAULT_BROADCAST_PORT),
		m_broadcastFreqSecs(0.5f),
		m_syncMsgQueueSize(1 << 20),
		m_appName(NULL),
		m_appSymbol('none'),
		m_appVersion(12345)
	{}
};

struct DebugNetListenerDesc
{
	u32 m_msgSymbol;
	bool m_isSynchronous;

	DebugNetRecvFunc m_recvFunc;
	DebugNetConnectedFunc m_connectedFunc;
	DebugNetDisconnectedFunc m_disconnectedFunc;
	void* m_userData;

	DebugNetListenerDesc() :
		m_msgSymbol('none'),
		m_recvFunc(NULL),
		m_connectedFunc(NULL),
		m_disconnectedFunc(NULL),
		m_userData(NULL),
		m_isSynchronous(true)
	{}
};

void DebugNet_Startup(DebugNetStartupParams& params);
void DebugNet_Shutdown();

void DebugNet_Update();

u32 DebugNet_RegisterListener(DebugNetListenerDesc& desc);
void DebugNet_UnregisterListener(u32 listenerId);

bool DebugNet_Send(u32 connectionId, u32 msgSymbol, const void* data, u32 dataSize);

// @}

#endif // NS_DEBUG_CONN_H