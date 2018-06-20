#include "Net/ENet/mpENet_Private.h"

ueBool mpLib_ENet::Host(mpHostParams* _params)
{
	UE_ASSERT(_params->GetLibType() == MP_ENET_SYMBOL);
	mpHostParams_ENet* params = (mpHostParams_ENet*) _params;

	UE_ASSERT(m_state == mpState_Idle);

	// Initialize broadcast socket

	if (params->m_enableLANBroadcast)
	{
		m_broadcastSocket = enet_socket_create(ENET_SOCKET_TYPE_DATAGRAM);
		if (m_broadcastSocket == ENET_SOCKET_NULL)
		{
			ueLogE("Hosting failed, reason: failed to create broadcast socket: %s", enet_socket_last_error_str());
			return UE_FALSE;
		}

		if (enet_socket_set_option(m_broadcastSocket, ENET_SOCKOPT_BROADCAST, 1))
		{
			ueLogE("Hosting failed, reason: failed to set broadcast option");
			enet_socket_destroy(m_broadcastSocket);
			m_broadcastSocket = 0;
			return UE_FALSE;
		}
	}

	// Create enet host

	ENetAddress addr;
	addr.host = params->m_ip.sn_addr;
	addr.port = params->m_port;
	m_enetHost = enet_host_create(&addr, params->m_maxNodes - 1, params->m_maxIncomingBandwidth, params->m_maxOutgoingBandwidth);
	if (!m_enetHost)
	{
		if (params->m_enableLANBroadcast)
		{
			enet_socket_destroy(m_broadcastSocket);
			m_broadcastSocket = 0;
		}
		ueLogE("Hosting failed, reason: failed to create enet host");
		return UE_FALSE;
	}

	// Get port we're bound to

	if (enet_socket_get_addr(m_enetHost->socket, &addr) != 0)
	{
		if (params->m_enableLANBroadcast)
		{
			enet_socket_destroy(m_broadcastSocket);
			m_broadcastSocket = 0;
		}
		enet_host_destroy(m_enetHost);
		m_enetHost = NULL;
		ueLogE("Hosting failed, reason: failed to get socket address");
		return UE_FALSE;
	}
	m_port = addr.port;

	// Set up session

	m_enableLANBroadcast = params->m_enableLANBroadcast;
	m_maxNodes = params->m_maxNodes;
	m_maxPlayers = params->m_maxPlayers;
	m_props.CopyFrom(params->m_numProps, params->m_props);

	m_lastBroadcastTime = 0;

	m_playerIdGenerator.Reset();
	m_nodeIdGenerator.Reset();

	// Create node & all players for the host

	m_hostNode = m_localNode = AddNode(NULL, &addr, m_nodeIdGenerator.GenerateId(), UE_TRUE, UE_TRUE);
	for (u32 i = 0; i < params->m_numLocalPlayers; i++)
		AddPlayer(m_playerIdGenerator.GenerateId(), m_hostNode);

	ChangeState(mpState_Hosting);
	ChangeState(mpState_Lobby);
	return UE_TRUE;
}

void mpLib_ENet::StartGame()
{
	UE_ASSERT(IsHost());

	for (u32 i = 0; i < m_numNodes; i++)
	{
		mpNode_ENet* node = m_nodes[i];
		if (node == m_hostNode)
			continue;

		node->m_resendFlags |= mpNode_ENet::ResendFlags_GameStarted;
		node->SendPendingManagementMsgs();
	}

	UE_ASSERT(m_state == mpState_Lobby);

	ChangeState(mpState_StartingGame);
	ChangeState(mpState_Game);
}

void mpLib_ENet::EndGame()
{
	UE_ASSERT(IsHost());

	for (u32 i = 0; i < m_numNodes; i++)
	{
		mpNode_ENet* node = m_nodes[i];
		if (node == m_hostNode)
			continue;

		node->m_resendFlags |= mpNode_ENet::ResendFlags_GameEnded;
		node->SendPendingManagementMsgs();
	}

	UE_ASSERT(m_state == mpState_Game);

	ChangeState(mpState_EndingGame);
	ChangeState(mpState_Lobby);
}

void mpLib_ENet::BroadcastSessionInfo()
{
	if (!IsHost())
		return;

	if (ueClock_GetSecsSince(m_lastBroadcastTime) < 0.5f)
		return;

	IO_BIT_BUFFER_DECL(buffer, 1400, ueBitBuffer::Mode_Write);
	CreateSessionInfoMsg(buffer, NULL, UE_TRUE);

	ENetAddress addr;
	addr.host = ENET_HOST_BROADCAST;
	addr.port = m_broadcastPort;

	ENetBuffer enetBuffer;
	enetBuffer.data = buffer.GetData();
	enetBuffer.dataLength = buffer.GetSizeInBytes();

	if (enet_socket_send(m_broadcastSocket, &addr, &enetBuffer, 1) > 0)
		m_lastBroadcastTime = ueClock_GetCurrent();
}

void mpLib_ENet::CreateSessionInfoMsg(ueBitBuffer& buffer, mpNode_ENet* dstNode, ueBool forBroadcasting)
{
	buffer.WriteAny<u8>(mpMsgType_ENet_SessionInfo);
	buffer.WriteAny(m_titleId);
	buffer.WriteAny(m_sessionId);
	if (forBroadcasting)
		buffer.WriteAny<u16>(m_port);
	buffer.WriteAny<u8>(m_numNodes);
	buffer.WriteAny<u8>(m_maxNodes);
	buffer.WriteAny<u8>(m_numPlayers);
	buffer.WriteAny<u8>(m_maxPlayers);
	m_props.Write(buffer);

	// Write nodes

	if (!forBroadcasting)
		for (u32 i = 0; i < m_numNodes; i++)
		{
			mpNode_ENet* node = m_nodes[i];

			buffer.WriteAny<u8>(node->m_id);
			buffer.WriteAny<u8>(node->m_numPlayers);

			u8 nodeFlags = 0;
			if (node == m_hostNode)
				nodeFlags |= mpNode_ENet::MsgFlags_IsHost;
			if (node == dstNode)
				nodeFlags |= mpNode_ENet::MsgFlags_IsLocal;
			buffer.WriteAny<u8>(nodeFlags);

			buffer.WriteAny<ENetAddress>(node->m_address);

			// Write players

			for (u32 j = 0; j < node->m_numPlayers; j++)
			{
				mpPlayer_ENet* player = node->m_players[j];

				buffer.WriteAny<u8>(player->m_id);

				u8 playerFlags = 0;
				if (player->m_isReady)
					playerFlags |= mpPlayer_ENet::MsgFlags_IsReady;
				buffer.WriteAny<u8>(playerFlags);
			}
		}
}

void mpLib_ENet::MarkSessionDirty()
{
	UE_ASSERT(IsHost());
	for (u32 i = 0; i < m_numNodes; i++)
		if (m_nodes[i] != m_localNode)
			m_nodes[i]->m_resendFlags |= mpNode_ENet::ResendFlags_SessionInfo;
}

ueBool mpLib_ENet::OnRecvMsg_ChangeReadinessReq(ENetPeer* srcPeer, mpRecvData_ENet* data)
{
	mpNode_ENet* node = (mpNode_ENet*) srcPeer->data;
	if (!node)
		return UE_TRUE;

	ueBitBuffer buffer(data->m_buffer, data->m_size, ueBitBuffer::Mode_Read);

	u8 playerId;
	if (!buffer.ReadAny<u8>(playerId))
		return UE_FALSE;

	ueBool newIsReady;
	if (!buffer.ReadBit(newIsReady))
		return UE_FALSE;

	mpPlayer_ENet* player = (mpPlayer_ENet*) GetPlayerById(playerId);
	if (!player || player->m_node != node)
		return UE_TRUE;

	player->SetReady(newIsReady);
	return UE_TRUE;
}

ueBool mpLib_ENet::OnRecvMsg_ClientIntro(ENetPeer* srcPeer, mpRecvData_ENet* data)
{
	if (m_isClient && srcPeer == m_hostPeer)
	{
		ueLogE("Received client-intro message from host (this should never happen!)");
		Reset();
		return UE_FALSE;
	}

	ueBitBuffer buffer(data->m_buffer, data->m_size, ueBitBuffer::Mode_Read);

	// Remove client from wait queue

	ueBool foundPeer = UE_FALSE;
	for (u32 i = 0; i < m_waitQueueSize; i++)
		if (m_waitQueue[i].m_peer == srcPeer)
		{
			foundPeer = UE_TRUE;
			m_waitQueue[i] = m_waitQueue[--m_waitQueueSize];
			break;
		}
	if (!foundPeer)
	{
		ueLogE("Received client-intro message peer that is not in wait queue");
		Reset();
		return UE_FALSE;
	}

    u8 numPlayersToJoin = 0;
    mpNode_ENet* node = NULL;

	// Read message

	u16 port = 0;
	if (!buffer.ReadAny<u16>(port)) goto Failure;

	if (!buffer.ReadAny(numPlayersToJoin)) goto Failure;

	// Create node & all players for the new client

	node = AddNode(srcPeer, &srcPeer->address, m_nodeIdGenerator.GenerateId(), UE_FALSE, UE_FALSE);
	for (u32 i = 0; i < numPlayersToJoin; i++)
		AddPlayer(m_playerIdGenerator.GenerateId(), node);

	srcPeer->data = node;
	MarkSessionDirty();
	return UE_TRUE;

Failure:
	enet_peer_disconnect_now(srcPeer, 0);
	RemoveWaitQueue(srcPeer);
	return UE_FALSE;
}
