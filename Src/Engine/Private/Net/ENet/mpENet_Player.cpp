#include "Net/ENet/mpENet_Private.h"

void mpPlayer_ENet::Init(u8 id, mpNode_ENet* node)
{
	m_isUsed = UE_TRUE;
	m_enet = node->m_enet;
	m_node = node;
	m_id = id;
	m_isReady = UE_FALSE;
	m_resendFlags = 0;
	m_isReady_target = UE_FALSE;
	m_userData = NULL;

	ueStrFormatS(m_debugName, "%u.%u", (u32) node->m_id, (u32) m_id);
}

void mpPlayer_ENet::Deinit()
{
	m_isUsed = UE_FALSE;
}

mpPlayerId mpPlayer_ENet::GetId()
{
	return m_id;
}

void mpPlayer_ENet::GetInfo(mpPlayerInfo* _info)
{
	UE_ASSERT(_info->GetLibType() == MP_ENET_SYMBOL);
	mpPlayerInfo_ENet* info = (mpPlayerInfo_ENet*) _info;
}

mpNode* mpPlayer_ENet::GetNode()
{
	return m_node;
}

ueBool mpPlayer_ENet::IsReady()
{
	return m_isReady;
}

void mpPlayer_ENet::SetReady(ueBool ready)
{
	UE_ASSERT(m_node->m_isLocal || m_enet->IsHost());

	if (m_enet->IsHost())
	{
		if (m_isReady == ready)
			return;

		m_isReady = ready;
		m_enet->m_callbacks->OnPlayerReady(this, ready);

		m_enet->MarkSessionDirty();
	}
	else
	{
		if (m_isReady_target == ready)
			return;

		m_isReady_target = ready;
		m_resendFlags |= ResendFlags_IsReady;
		m_node->SendPendingManagementMsgs();
	}
}

ueBool mpPlayer_ENet::IsHost()
{
	return m_node->m_isHost;
}

ueBool mpPlayer_ENet::IsLocal()
{
	return m_node->m_isLocal;
}

ueBool mpPlayer_ENet::SendPendingManagementMsgs()
{
	if (!m_enet->IsHost())
	{
		if ((m_resendFlags & ResendFlags_IsReady) && !SendMsg_IsReady())
			return UE_FALSE;
	}

	return UE_TRUE;
}

const char* mpPlayer_ENet::GetDebugName()
{
	return m_debugName;
}

ueBool mpPlayer_ENet::SendMsg_IsReady()
{
	UE_ASSERT(m_resendFlags & mpPlayer_ENet::ResendFlags_IsReady);
	UE_ASSERT(!m_enet->IsHost());

	IO_BIT_BUFFER_DECL(buffer, 32, ueBitBuffer::Mode_Write);
	buffer.WriteAny<u8>(mpMsgType_ENet_ReadinessChangeReq);
	buffer.WriteAny<u8>(m_id);
	buffer.WriteBit(m_isReady_target);

	mpSendData_ENet sendData;
	sendData.m_buffer = buffer.GetData();
	sendData.m_size = buffer.GetSizeInBytes();
	sendData.m_sendFlags = mpSendFlags_Reliable | mpSendFlags_Ordered;
	if (!m_enet->SendMsg(m_enet->m_hostPeer, &sendData, UE_TRUE))
		return UE_FALSE;

	m_resendFlags &= ~mpPlayer_ENet::ResendFlags_IsReady;
	return UE_TRUE;
}

void mpPlayer_ENet::SetUserData(void* userData)
{
	m_userData = userData;
}

void* mpPlayer_ENet::GetUserData()
{
	return m_userData;
}

// mpLib_ENet

u32 mpLib_ENet::GetNumPlayers()
{
	return m_numPlayers;
}

mpPlayer* mpLib_ENet::GetPlayer(u32 index)
{
	UE_ASSERT(index < m_numPlayers);
	return m_players[index];
}

mpPlayer* mpLib_ENet::GetHostPlayer()
{
	return m_hostPlayer;
}

mpPlayer* mpLib_ENet::GetPlayerById(mpPlayerId id)
{
	for (u32 i = 0; i < m_numPlayers; i++)
		if (m_players[i]->m_id == id)
			return m_players[i];
	return NULL;
}

ueBool mpLib_ENet::IsEveryoneReady()
{
	for (u32 i = 0; i < m_numPlayers; i++)
		if (!m_players[i]->m_isReady)
			return UE_FALSE;
	return UE_TRUE;
}

void mpLib_ENet::DropPlayer(mpPlayer_ENet* player)
{
	mpNode_ENet* node = player->m_node;

	m_callbacks->OnPlayerRemoved(player);

	for (u32 i = 0; i < node->m_numPlayers; i++)
		if (node->m_players[i] == player)
		{
			node->m_players[i] = node->m_players[--node->m_numPlayers];
			break;
		}

	for (u32 i = 0; i < m_numPlayers; i++)
		if (m_players[i] == player)
		{
			m_players[i] = m_players[--m_numPlayers];
			break;
		}

	player->Deinit();

	// Also destroy node

	if (node->m_numPlayers == 0)
	{
		m_callbacks->OnNodeDisconnected(node);

		if (IsHost())
			m_nodeIdGenerator.ReleaseId(node->m_id);

		if (node == m_localNode)
			m_localNode = NULL;
		if (node == m_hostNode)
			m_hostNode = NULL;

		for (u32 i = 0; i < m_numNodes; i++)
			if (m_nodes[i] == node)
			{
				m_nodes[i] = m_nodes[--m_numNodes];
				break;
			}

		node->Deinit();
	}
}

mpPlayer_ENet* mpLib_ENet::AddPlayer(u8 id, mpNode_ENet* node)
{
	UE_ASSERT(m_numPlayers < MP_ENET_MAX_PLAYERS);
	UE_ASSERT(node->m_numPlayers < MP_ENET_MAX_PLAYERS_PER_NODE);

	mpPlayer_ENet* player = NULL;
	for (u32 i = 0; i < MP_ENET_MAX_PLAYERS; i++)
		if (!m_playersPool[i].m_isUsed)
		{
			player = &m_playersPool[i];
			break;
		}
	UE_ASSERT(player);

	m_players[m_numPlayers++] = player;
	node->m_players[node->m_numPlayers++] = player;

	player->Init(id, node);
	if (node == m_hostNode && node->m_numPlayers == 0)
		m_hostPlayer = player;

	m_callbacks->OnPlayerAdded(player);
	return player;
}