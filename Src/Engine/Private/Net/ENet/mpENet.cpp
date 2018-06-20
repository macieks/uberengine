#include "Net/ENet/mpENet_Private.h"

mpLib* mpLib_ENet_Create(mpCreateParams* params)
{
	mpLib_ENet* lib = new(params->m_allocator) mpLib_ENet();
	if (!lib->Init(params))
	{
		ueDelete(lib, params->m_allocator);
		return NULL;
	}
	return lib;
}

ueBool mpLib_ENet::Init(mpCreateParams* _params)
{
	UE_ASSERT(_params->GetLibType() == MP_ENET_SYMBOL);
	mpCreateParams_ENet* params = (mpCreateParams_ENet*) _params;

	UE_ASSERT(params->m_allocator);
	UE_ASSERT(params->m_callbacks);

	m_allocator = params->m_allocator;
	m_callbacks = params->m_callbacks;
	m_titleId = params->m_titleId;
	m_broadcastPort = params->m_broadcastPort;

	m_numPlayers = 0;
	for (u32 i = 0; i < MP_ENET_MAX_PLAYERS; i++)
		m_playersPool[i].m_isUsed = UE_FALSE;

	m_numNodes = 0;
	for (u32 i = 0; i < MP_ENET_MAX_NODES; i++)
		m_nodesPool[i].m_isUsed = UE_FALSE;

	m_numQueries = 0;
	for (u32 i = 0; i < MP_ENET_MAX_QUERIES; i++)
		m_queriesPool[i].m_isUsed = UE_FALSE;

	m_state = mpState_Idle;

	m_isClient = UE_FALSE;

	m_broadcastSocket = 0;
	m_broadcastRecvSocket = 0;

	m_waitQueueSize = 0;

	enet_initialize();
	m_enetHost = NULL;

	return UE_TRUE;
}

void mpLib_ENet::Destroy()
{
	Reset();
	ueDelete(this, m_allocator);

	enet_deinitialize();
}

void mpLib_ENet::Leave(mpLeaveParams* _params)
{
	UE_ASSERT(_params->GetLibType() == MP_ENET_SYMBOL);
	mpLeaveParams_ENet* params = (mpLeaveParams_ENet*) _params;

	UE_ASSERT(m_state != mpState_Idle);

	if (m_state == mpState_Leaving)
		return;

	ChangeState(mpState_Leaving);

	for (u32 i = 0; i < m_numNodes; i++)
	{
		mpNode_ENet* node = m_nodes[i];
		if (node->m_enetPeer)
			enet_peer_disconnect(node->m_enetPeer, 0);
	}
}

void mpLib_ENet::Update()
{
	if (m_state == mpState_Leaving)
		UpdateLeaving();
	else
		HandleENetEvents();

	if (IsHost() || m_isClient)
		for (u32 i = 0; i < m_numNodes; i++)
			m_nodes[i]->SendPendingManagementMsgs();

	UpdateQueries();

	BroadcastSessionInfo();
}

void mpLib_ENet::GetSessionInfo(mpSessionInfo* _info)
{
	UE_ASSERT(_info->GetLibType() == MP_ENET_SYMBOL);
	mpSessionInfo_ENet* info = (mpSessionInfo_ENet*) _info;

	info->m_sessionId = m_sessionId;
	info->m_numProps = m_props.m_count;
	info->m_props = m_props.m_props;
}

mpState mpLib_ENet::GetState()
{
	return m_state;
}

void mpLib_ENet::UpdateLeaving()
{
	ENetEvent event;
	while (enet_host_service(m_enetHost, &event, 0) > 0)
		switch (event.type)
		{
		case ENET_EVENT_TYPE_RECEIVE:
			enet_packet_destroy(event.packet);
			break;

		case ENET_EVENT_TYPE_DISCONNECT:
			HandleENetDisconnect(event.peer);
			if (m_numNodes == 0)
			{
				Reset();
				return;
			}
			break;
		}

	if (ueClock_GetSecsSince(m_stateTime) > m_leaveTimeOut)
		Reset();
}

void mpLib_ENet::HandleENetEvents()
{
	if (!m_enetHost)
		return;

	ENetEvent event;
	while (m_enetHost && enet_host_service(m_enetHost, &event, 0) > 0)
		switch (event.type)
		{
			case ENET_EVENT_TYPE_CONNECT:
			{
				ueLogD("%s:%u connected",
					nsIP::ToString(event.peer->address.host),
					(u32) event.peer->address.port);

				if (m_isClient)
				{
					if (m_state == mpState_Joining && event.peer == m_hostPeer)
					{
						OnHostConnected();
						break;
					}

					mpNode_ENet* node = NULL;
					for (u32 i = 0; i < m_numNodes; i++)
						if (m_nodes[i]->m_address == event.peer->address)
						{
							node = m_nodes[i];
							break;
						}

					if (node)
					{
						node->m_enetPeer = event.peer;
						break;
					}
				}

				// Add to queue
				if (AddWaitQueue(event.peer))
				{
					event.peer->data = NULL;
					ueLogD("Node queued & waiting...");
				}
				else
				{
					ueLogD("Not enough slots in wait queue - dropping enet peer.");
					enet_peer_disconnect_now(event.peer, 0);
					break;
				}

				break;
			}

			case ENET_EVENT_TYPE_RECEIVE:
			{
#if 0
				ueLogD("A packet of length %u containing %x was received from %s:%u on channel %u.",
					event.packet->dataLength,
					event.packet->data,
					ueIp::ToString(event.peer->address.host),
					(u32) event.peer->address.port,
					event.channelID);
#endif

				UE_ASSERT(event.packet->dataLength > 0);

				const u8 managementByte = *event.packet->data;

				mpRecvData_ENet recvData;
				recvData.m_buffer = (u8*) event.packet->data + 1;
				recvData.m_size = (u32) event.packet->dataLength - 1;
				recvData.m_reliable = event.channelID == MP_ENET_RELIABLE_CHANNEL;
				recvData.m_srcNode = (mpNode*) event.peer->data;

				switch (managementByte)
				{
					case MP_ENET_MANAGEMENT_MSG_BYTE:
						OnRecvMsg(event.peer, &recvData);
						break;
					case MP_ENET_USER_MSG_BYTE:
					{
						if (!recvData.m_srcNode)
						{
							ueLogW("Received msg from peer which isn't node yet.");
							break;
						}

						m_callbacks->OnRecvData(&recvData);
						break;
					}
				}

				enet_packet_destroy(event.packet);
				break;
			}

			case ENET_EVENT_TYPE_DISCONNECT:
			{
				HandleENetDisconnect(event.peer);
				break;
			}
		}
}

ueBool mpLib_ENet::AddWaitQueue(ENetPeer* peer)
{
	if (m_waitQueueSize == UE_ARRAY_SIZE(m_waitQueue))
		return UE_FALSE;
					
	WaitingPeer* waitingPeer = &m_waitQueue[m_waitQueueSize++];
	waitingPeer->m_peer = peer;
	waitingPeer->m_queueTime = ueClock_GetCurrent();
	return UE_TRUE;
}

void mpLib_ENet::RemoveWaitQueue(ENetPeer* peer)
{
	for (u32 i = 0; i < m_waitQueueSize; i++)
		if (m_waitQueue[i].m_peer == peer)
		{
			m_waitQueue[i] = m_waitQueue[--m_waitQueueSize];
			return;
		}
}

void mpLib_ENet::HandleENetDisconnect(ENetPeer* peer)
{
	if (m_isClient && peer == m_hostPeer)
	{
		ueLogD("Host node (%s:%u) disconnected", nsIP::ToString(peer->address.host), (u32) peer->address.port);
		Reset();
		return;
	}

	ueLogD("%s:%u disconnected", nsIP::ToString(peer->address.host), (u32) peer->address.port);
	
	mpNode_ENet* node = (mpNode_ENet*) peer->data;
	if (node)
		DropNode(node);
	else
		RemoveWaitQueue(peer);
}

ueBool mpLib_ENet::OnRecvMsg(ENetPeer* srcPeer, mpRecvData_ENet* data)
{
	ueBitBuffer buffer(data->m_buffer, data->m_size, ueBitBuffer::Mode_Read);

	u8 msgType;
	if (!buffer.ReadAny(msgType))
		return UE_FALSE;

	mpRecvData_ENet dataNoMsgType;
	dataNoMsgType = *data;
	dataNoMsgType.m_buffer = (u8*) dataNoMsgType.m_buffer + 1;
	dataNoMsgType.m_size--;

	switch (msgType)
	{
		case mpMsgType_ENet_ReadinessChangeReq: return OnRecvMsg_ChangeReadinessReq(srcPeer, &dataNoMsgType);
		case mpMsgType_ENet_SessionInfo: return OnRecvMsg_SessionInfo(srcPeer, &dataNoMsgType);
		case mpMsgType_ENet_GameStarted: return OnRecvMsg_GameStarted(srcPeer, &dataNoMsgType);
		case mpMsgType_ENet_GameEnded: return OnRecvMsg_GameEnded(srcPeer, &dataNoMsgType);
		case mpMsgType_ENet_ClientIntro: return OnRecvMsg_ClientIntro(srcPeer, &dataNoMsgType);
	}

	ueLogE("Unknown message type %d", (u32) msgType);
	return UE_FALSE;
}

ueBool mpLib_ENet::IsHost()
{
	return m_hostNode && m_hostNode->m_isLocal;
}

// Internal 

mpLib_ENet::mpLib_ENet() :
	m_allocator(NULL),
	m_state(mpState_Idle),
	m_localNode(NULL),
	m_hostNode(NULL),
	m_numPlayers(0),
	m_hostPlayer(NULL)
{
}

mpLib_ENet::~mpLib_ENet()
{
	UE_ASSERT(m_state == mpState_Idle);
	UE_ASSERT(m_numQueries == 0);
}

void mpLib_ENet::Reset()
{
	while (m_numNodes > 0)
		DropNode(m_nodes[0]);

	while (m_numQueries > 0)
		m_queries[0]->Destroy();

	m_waitQueueSize = 0;

	if (m_broadcastSocket && m_broadcastSocket != ENET_SOCKET_NULL)
	{
		enet_socket_destroy(m_broadcastSocket);
		m_broadcastSocket = 0;
	}

	if (m_enetHost)
	{
		enet_host_destroy(m_enetHost);
		m_enetHost = NULL;
	}

	m_waitQueueSize = 0;

	m_isClient = UE_FALSE;

	ChangeState(mpState_Idle);
}

void mpLib_ENet::ChangeState(mpState state)
{
	if (m_state == state)
		return;

	mpState oldState = m_state;
	m_state = state;
	m_callbacks->OnStateChange(oldState, state);
}

ueBool mpLib_ENet::SendMsg(ENetPeer* peer, mpSendData_ENet* sendData, ueBool isManagementMsg)
{
	UE_ASSERT(!isManagementMsg || (mpSendFlags_Reliable | mpSendFlags_Ordered));

	ENetPacket* packet = enet_packet_create(
		NULL,
		sendData->m_size + 1,
		((sendData->m_sendFlags & mpSendFlags_Reliable) ? ENET_PACKET_FLAG_RELIABLE : 0) |
		((sendData->m_sendFlags & mpSendFlags_Ordered) ? 0 : ENET_PACKET_FLAG_UNSEQUENCED));
	if (!packet)
		return UE_FALSE;

	packet->data[0] = isManagementMsg ? MP_ENET_MANAGEMENT_MSG_BYTE : MP_ENET_USER_MSG_BYTE;
	memcpy(packet->data + 1, sendData->m_buffer, sendData->m_size);

	if (enet_peer_send(peer, (sendData->m_sendFlags & mpSendFlags_Reliable) ? MP_ENET_RELIABLE_CHANNEL : MP_ENET_UNRELIABLE_CHANNEL, packet) != 0)
	{
		enet_packet_destroy(packet);
		return UE_FALSE;
	}

	if (sendData->m_sendFlags & mpSendFlags_Commit)
		enet_host_flush(m_enetHost);

	return UE_TRUE;
}

u32 mpLib_ENet::GetLibSymbol()
{
	return MP_ENET_SYMBOL;
}