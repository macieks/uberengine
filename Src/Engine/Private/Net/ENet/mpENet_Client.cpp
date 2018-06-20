#include "Net/ENet/mpENet_Private.h"

ueBool mpLib_ENet::Join(mpJoinParams* _params)
{
	UE_ASSERT(_params->GetLibType() == MP_ENET_SYMBOL);
	mpJoinParams_ENet* params = (mpJoinParams_ENet*) _params;

	UE_ASSERT(m_state == mpState_Idle);

	// Create local "enet host"

	ENetAddress localAddr;
	localAddr.host = params->m_localIp.sn_addr;
	localAddr.port = params->m_localPort;
	m_enetHost = enet_host_create(&localAddr, MP_ENET_MAX_NODES - 1, params->m_maxIncomingBandwidth, params->m_maxOutgoingBandwidth);
	if (!m_enetHost)
	{
		ueLogE("Joining failed, reason: failed to create enet host");
		return UE_FALSE;
	}

	// Get port we're bound to

	if (enet_socket_get_addr(m_enetHost->socket, &localAddr) != 0)
	{
		enet_host_destroy(m_enetHost);
		m_enetHost = NULL;
		ueLogE("Joining failed, reason: failed to get socket address");
		return UE_FALSE;
	}
	m_port = localAddr.port;

	// Start connection to (remote) host

	ENetAddress hostAddr;
	hostAddr.host = params->m_hostIp.sn_addr;
	hostAddr.port = params->m_hostPort;
	m_hostPeer = enet_host_connect(m_enetHost, &hostAddr, 2);
	if (!m_hostPeer)
	{
		enet_host_destroy(m_enetHost);
		m_enetHost = NULL;
		ueLogE("Joining failed, reason: failed to start enet connection");
		return UE_FALSE;
	}

	m_numPlayersToJoin = params->m_numLocalPlayers;
	m_props.Reset();

	m_hostNode = m_localNode = NULL;
	m_hostPlayer = NULL;

	m_isClient = UE_TRUE;
	m_joinWaitingForSessionInfo = UE_FALSE;
	ChangeState(mpState_Joining);
	return UE_TRUE;
}

void mpLib_ENet::OnHostConnected()
{
	IO_BIT_BUFFER_DECL(buffer, 32, ueBitBuffer::Mode_Write);
	buffer.WriteAny<u8>(mpMsgType_ENet_ClientIntro);
	buffer.WriteAny<u16>(m_port);
	buffer.WriteAny<u8>(m_numPlayersToJoin);

	mpSendData_ENet sendData;
	sendData.m_buffer = buffer.GetData();
	sendData.m_size = buffer.GetSizeInBytes();
	sendData.m_sendFlags = mpSendFlags_Reliable | mpSendFlags_Ordered;
	if (!SendMsg(m_hostPeer, &sendData, UE_TRUE))
	{
		ueLogE("Joining failed, reason: failed to send client intro message to host");
		Reset();
		return;
	}

	m_joinWaitingForSessionInfo = UE_TRUE;
	ueLogD("Host connected, waiting for initial session info message...");
}

ueBool mpLib_ENet::OnRecvMsg_SessionInfo(ENetPeer* srcPeer, mpRecvData_ENet* data)
{
	if (srcPeer != m_hostPeer)
	{
		ueLogW("Received session info from non-host, dropping the node...");
		enet_peer_disconnect_now(srcPeer, 0);
		RemoveWaitQueue(srcPeer);
		return UE_FALSE;
	}

	if (m_state == mpState_Joining)
	{
		if (!m_joinWaitingForSessionInfo)
		{
			ueLogD("Join state mismatch, dropping the node...");
			enet_peer_disconnect_now(srcPeer, 0);
			RemoveWaitQueue(srcPeer);
			return UE_FALSE;
		}
		ueLogD("Finalizing join, processing initial session info message...");
	}

	ueBitBuffer buffer(data->m_buffer, data->m_size, ueBitBuffer::Mode_Read);

    u32 numPlayersToDelete = 0;
	u8 numNodes = 0;
	u8 maxNodes = 0;
	u8 numPlayers = 0;
	u8 maxPlayers = 0;
	mpPropertySet_ENet props;
	mpPlayer_ENet* playersToDelete[MP_ENET_MAX_PLAYERS];

	u32 titleId = 0;
	if (!buffer.ReadAny(titleId)) goto Failure;
	if (titleId != m_titleId) goto Failure;

	if (!buffer.ReadAny(m_sessionId)) goto Failure;

	// FIXME: Also send/receive host state to client (client who joins while
	// session is in "starting" or "game" state has wrong state until next state
	// change message is received from host)

	if (!buffer.ReadAny(numNodes)) goto Failure;
	if (!buffer.ReadAny(maxNodes)) goto Failure;
	m_maxNodes = maxNodes;

	if (!buffer.ReadAny(numPlayers)) goto Failure;
	if (!buffer.ReadAny(maxPlayers)) goto Failure;
	m_maxPlayers = maxPlayers;

	if (!props.Read(buffer)) goto Failure;
	m_props.CopyFrom(&props, (m_state != mpState_Joining) ? m_callbacks : NULL);

	// Read nodes

	numPlayersToDelete = m_numPlayers;
	ueMemCpy(playersToDelete, m_players, m_numPlayers * sizeof(mpPlayer_ENet*));

	for (u32 i = 0; i < numNodes; i++)
	{
		u8 id = 0;
		if (!buffer.ReadAny<u8>(id)) goto Failure;

		u8 numNodePlayers = 0;
		if (!buffer.ReadAny<u8>(numNodePlayers)) goto Failure;

		u8 nodeFlags = 0;
		if (!buffer.ReadAny<u8>(nodeFlags)) goto Failure;
		const ueBool isHost = (nodeFlags & mpNode_ENet::MsgFlags_IsHost) ? UE_TRUE : UE_FALSE;
		const ueBool isLocal = (nodeFlags & mpNode_ENet::MsgFlags_IsLocal) ? UE_TRUE : UE_FALSE;

		ENetAddress address;
		if (!buffer.ReadAny<ENetAddress>(address)) goto Failure;
		if (isHost)
			address = m_hostPeer->address;

		mpNode_ENet* node = FindNodeById(id);
		const ueBool isNewNode = !node;
		if (isNewNode)
		{
			ENetPeer* peer = NULL;
			if (isHost)
				peer = m_hostPeer;
			else
				for (u32 i = 0; i < m_waitQueueSize; i++)
					if (m_waitQueue[i].m_peer->address == address)
					{
						peer = m_waitQueue[i].m_peer;
						m_waitQueue[i] = m_waitQueue[--m_waitQueueSize];
						break;
					}

			node = AddNode(peer, &address, id, isHost, isLocal);
			if (isLocal)
				m_localNode = node;
			if (isHost)
				m_hostNode = node;
		}

		// Read players

		for (u32 j = 0; j < numNodePlayers; j++)
		{
			u8 playerId = 0;
			if (!buffer.ReadAny<u8>(playerId)) goto Failure;

			u8 playerFlags = 0;
			if (!buffer.ReadAny<u8>(playerFlags)) goto Failure;

			mpPlayer_ENet* player = (mpPlayer_ENet*) GetPlayerById((mpPlayerId) playerId);
			if (player && player->m_node != node)
				goto Failure;

			const ueBool newReady = (playerFlags & mpPlayer_ENet::MsgFlags_IsReady) != 0;

			if (!player)
			{
				player = AddPlayer(playerId, node);
				player->m_isReady = player->m_isReady_target = newReady;

				if (isHost && node->m_numPlayers == 1)
					m_hostPlayer = player;
			}
			else
			{
				for (u32 k = 0; k < numPlayersToDelete; k++)
					if (playersToDelete[k] == player)
					{
						playersToDelete[k] = playersToDelete[--numPlayersToDelete];
						break;
					}

				if ((!isLocal || newReady == player->m_isReady_target) && player->m_isReady != newReady)
				{
					player->m_isReady = newReady;
					m_callbacks->OnPlayerReady(player, newReady);
				}
			}
		}
	}

	// Delete players that weren't included in session info

	for (u32 i = 0; i < numPlayersToDelete; i++)
		DropPlayer(playersToDelete[i]);

	// Finalize join operation

	if (m_state == mpState_Joining)
	{
		ueLogD("Join finalized successfully");
		ChangeState(mpState_Lobby);
	}
	return UE_TRUE;

Failure:
	ueLogE("Received invalid session info");
	Reset();
	return UE_FALSE;
}

ueBool mpLib_ENet::OnRecvMsg_GameStarted(ENetPeer* srcPeer, mpRecvData_ENet* data)
{
	UE_ASSERT(m_isClient);
	UE_ASSERT(srcPeer == m_hostPeer);
	UE_ASSERT(m_state == mpState_Lobby);

	ChangeState(mpState_StartingGame);
	ChangeState(mpState_Game);

	return UE_TRUE;
}

ueBool mpLib_ENet::OnRecvMsg_GameEnded(ENetPeer* srcPeer, mpRecvData_ENet* data)
{
	UE_ASSERT(m_isClient);
	UE_ASSERT(srcPeer == m_hostPeer);
	UE_ASSERT(m_state == mpState_Game);

	ChangeState(mpState_EndingGame);
	ChangeState(mpState_Lobby);

	return UE_TRUE;
}
