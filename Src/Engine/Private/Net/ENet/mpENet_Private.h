#pragma once

#include "Net/ENet/mpLib_ENet.h"
#include "IO/ueBitBuffer.h"
#include "Base/Containers/ueUniqueIdGenerator.h"

#include <enet/enet.h>

// Extra enet functions
const char* enet_socket_last_error_str();
int enet_socket_get_addr(ENetSocket socket, ENetAddress* addr);
bool operator == (const ENetAddress& a, const ENetAddress& b);

class mpLib_ENet;
class mpPlayer_ENet;
class mpNode_ENet;
class mpQuery_ENet;

enum mpMsgType_ENet
{
	mpMsgType_ENet_ReadinessChangeReq = 0,
	mpMsgType_ENet_SessionInfo,
	mpMsgType_ENet_GameStarted,
	mpMsgType_ENet_GameEnded,
	mpMsgType_ENet_ClientIntro,

	mpMsgType_ENet_MAX
};

#define MP_ENET_RELIABLE_CHANNEL	0
#define MP_ENET_UNRELIABLE_CHANNEL	1

#define MP_ENET_MANAGEMENT_MSG_BYTE	133
#define MP_ENET_USER_MSG_BYTE		166

#define MP_ENET_MAX_SESSION_PROPS			16
#define MP_ENET_MAX_SESSION_PROPS_DATA_SIZE	512

struct mpPropertySet_ENet
{
	u32 m_count;
	ueProperty m_props[MP_ENET_MAX_SESSION_PROPS];
	u8 m_data[MP_ENET_MAX_SESSION_PROPS_DATA_SIZE];
	u32 m_dataSize;

	void Reset();
	void Add(const ueProperty* prop);
	void Remove(const ueProperty* prop);
	ueBool Set(const ueProperty* prop);
	ueBool Get(ueProperty* prop);
	void CopyFrom(u32 count, const ueProperty* src);
	void CopyFrom(const mpPropertySet_ENet* src, mpCallbacks* callbacks = NULL);
	ueBool Read(ueBitBuffer& input);
	void Write(ueBitBuffer& output);
	ueBool Match(const mpPropertySet_ENet* other);
};

class mpNode_ENet;

class mpPlayer_ENet : mpPlayer
{
public:

	// Interface

	mpPlayerId GetId();
	void GetInfo(mpPlayerInfo* info);
	mpNode* GetNode();
	ueBool IsReady();
	void SetReady(ueBool ready);
	ueBool IsHost();
	ueBool IsLocal();
	const char* GetDebugName();
	void SetUserData(void* userData);
	void* GetUserData();

	// Internal
	void Init(u8 id, mpNode_ENet* node);
	void Deinit();

	ueBool SendPendingManagementMsgs();
	ueBool SendMsg_IsReady();

private:
	ueBool m_isUsed;

	mpLib_ENet* m_enet;
	mpNode_ENet* m_node;
	u8 m_id;

	char m_debugName[128];
	void* m_userData;

	ueBool m_isReady;

	// Client only

	enum ResendFlags
	{
		ResendFlags_IsReady		= UE_POW2(0)
	};
	u32 m_resendFlags;

	enum MsgFlags
	{
		MsgFlags_IsReady	= UE_POW2(0)
	};

	ueBool m_isReady_target;

	friend class mpNode_ENet;
	friend class mpLib_ENet;
};

class mpNode_ENet : public mpNode
{
public:

	// Interface

	ueBool SendTo(mpSendData* sendData);
	void CommitSendTo();
	void GetInfo(mpNodeInfo* info);
	u32 GetNumPlayers();
	mpPlayer* GetPlayer(u32 index);
	mpPlayer* GetPlayerById(mpPlayerId id);
	ueBool IsHost();
	ueBool IsLocal();
	const char* GetDebugName();
	void SetUserData(void* userData);
	void* GetUserData();

	// Internal

	void Init(mpLib_ENet* lib, ENetAddress* addr, u8 id, ENetPeer* peer, ueBool isHost, ueBool isLocal);
	void Deinit();
	ueBool SendPendingManagementMsgs();
	ueBool SendMsg(mpSendData_ENet* sendData, ueBool isManagementMsg);
	ueBool SendMsg_GameStarted();
	ueBool SendMsg_GameEnded();
	ueBool SendMsg_SessionInfo();

private:
	ueBool m_isUsed;

	mpLib_ENet* m_enet;
	ENetPeer* m_enetPeer; // NULL for host
	ENetAddress m_address;

	u8 m_id;

	char m_debugName[128];
	void* m_userData;

	ueBool m_isHost;
	ueBool m_isLocal;

	u32 m_numPlayers;
	mpPlayer_ENet* m_players[MP_ENET_MAX_PLAYERS_PER_NODE];

	enum ResendFlags
	{
		ResendFlags_GameStarted		= UE_POW2(0),
		ResendFlags_GameEnded		= UE_POW2(1),
		ResendFlags_SessionInfo		= UE_POW2(2)
	};
	u32 m_resendFlags;

	enum MsgFlags
	{
		MsgFlags_IsHost		= UE_POW2(0),
		MsgFlags_IsLocal	= UE_POW2(1)
	};

	friend class mpPlayer_ENet;
	friend class mpLib_ENet;
};

class mpQuery_ENet : public mpQuery
{
public:

	// Interface

	ueBool IsDone();
	void Destroy();
	void GetResult(mpQueryResult* result);
	const char* GetDebugName();
	void SetUserData(void* userData);
	void* GetUserData();

	// Internal
	void UpdateResults(ENetAddress* addr, void* data, u32 size);

private:
	ueBool m_isUsed;

	mpLib_ENet* m_enet;

	ueTime m_startTime;

	u32 m_maxResults;
	f32 m_timeOutSecs;

	ueBool m_isDone;

	u32 m_numSessions;
	mpQueryResult_ENet::Session m_sessions[MP_ENET_MAX_QUERY_RESULTS];
	mpPropertySet_ENet m_sessionProps[MP_ENET_MAX_QUERY_RESULTS];

	mpPropertySet_ENet m_props;

	void* m_userData;

	friend class mpLib_ENet;
};

class mpLib_ENet : public mpLib
{
public:

	// Interface

	void Destroy();
	mpQuery* CreateQuery(mpQueryParams* params);
	ueBool Host(mpHostParams* params);
	void StartGame();
	void EndGame();
	ueBool IsEveryoneReady();
	void DropNode(mpNode* node);
	ueBool Join(mpJoinParams* params);
	void Leave(mpLeaveParams* params);
	void Update();
	void GetSessionInfo(mpSessionInfo* info);
	mpState GetState();
	u32 GetLibSymbol();
	u32 GetNumNodes();
	mpNode* GetNode(u32 index);
	u32 GetNumPlayers();
	mpPlayer* GetPlayer(u32 index);
	mpPlayer* GetPlayerById(mpPlayerId id);
	ueBool IsHost();
	mpNode* GetLocalNode();
	mpNode* GetHostNode();
	mpPlayer* GetHostPlayer();
	void SetProperty(ueProperty* prop);
	ueBool GetProperty(ueProperty* prop);

	// Internal
	mpLib_ENet();
	~mpLib_ENet();

	ueBool Init(mpCreateParams* params);

	void Reset();
	void MarkSessionDirty();
	void ChangeState(mpState state);
	void BroadcastSessionInfo();

	void HandleENetEvents();
	void HandleENetDisconnect(ENetPeer* peer);

	void OnHostConnected();

	ueBool SendMsg(ENetPeer* peer, mpSendData_ENet* sendData, ueBool isManagementMsg);

	void UpdateQueries();
	void DestroyQuery(mpQuery_ENet* query);

	void UpdateLeaving();

	void DropPlayer(mpPlayer_ENet* player);
	mpPlayer_ENet* AddPlayer(u8 id, mpNode_ENet* node);
	mpNode_ENet* AddNode(ENetPeer* peer, ENetAddress* addr, u8 id, ueBool isHost, ueBool isLocal);

	void CreateSessionInfoMsg(ueBitBuffer& buffer, mpNode_ENet* dstNode, ueBool forBroadcasting);

	ueBool OnRecvMsg(ENetPeer* srcPeer, mpRecvData_ENet* data);
	ueBool OnRecvMsg_ChangeReadinessReq(ENetPeer* srcPeer, mpRecvData_ENet* data);
	ueBool OnRecvMsg_SessionInfo(ENetPeer* srcPeer, mpRecvData_ENet* data);
	ueBool OnRecvMsg_GameStarted(ENetPeer* srcPeer, mpRecvData_ENet* data);
	ueBool OnRecvMsg_GameEnded(ENetPeer* srcPeer, mpRecvData_ENet* data);
	ueBool OnRecvMsg_ClientIntro(ENetPeer* srcPeer, mpRecvData_ENet* data);

	ueBool AddWaitQueue(ENetPeer* peer);
	void RemoveWaitQueue(ENetPeer* peer);

	mpNode_ENet* FindNodeById(u8 id);

private:
	ueAllocator* m_allocator;
	mpCallbacks* m_callbacks;

	// State

	mpState m_state;
	ueTime m_stateTime;

	// Configuration

	f32 m_leaveTimeOut;

	// Enet specific

	ENetHost* m_enetHost;
	u16 m_port;

	u32 m_waitQueueSize;
	struct WaitingPeer
	{
		ENetPeer* m_peer;
		ueTime m_queueTime;
	};
	WaitingPeer m_waitQueue[MP_ENET_MAX_NODES];

	// Client only

	ueBool m_isClient;

	ueBool m_joinWaitingForSessionInfo;
	ENetPeer* m_hostPeer;

	u32 m_numPlayersToJoin;

	ENetSocket m_broadcastRecvSocket;

	// Host only

	ueBool m_enableLANBroadcast;

	u32 m_maxNodes;
	u32 m_maxPlayers;

	ueUniqueIdGenerator m_playerIdGenerator;
	ueUniqueIdGenerator m_nodeIdGenerator;

	ENetSocket m_broadcastSocket;
	u16 m_broadcastPort;
	ueTime m_lastBroadcastTime;

	// Session

	u32 m_titleId;
	u32 m_sessionId;

	mpPropertySet_ENet m_props;

	u32 m_numNodes;
	mpNode_ENet* m_nodes[MP_ENET_MAX_NODES];
	mpNode_ENet m_nodesPool[MP_ENET_MAX_NODES];
	mpNode_ENet* m_localNode;
	mpNode_ENet* m_hostNode;

	u32 m_numPlayers;
	mpPlayer_ENet* m_players[MP_ENET_MAX_PLAYERS];
	mpPlayer_ENet m_playersPool[MP_ENET_MAX_PLAYERS];
	mpPlayer_ENet* m_hostPlayer;

	// Queries

	u32 m_numQueries;
	mpQuery_ENet* m_queries[MP_ENET_MAX_QUERIES];
	mpQuery_ENet m_queriesPool[MP_ENET_MAX_QUERIES];

	friend class mpNode_ENet;
	friend class mpPlayer_ENet;
	friend class mpQuery_ENet;
};