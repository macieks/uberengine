#include "Net/ENet/mpENet_Private.h"

void mpNode_ENet::Init(mpLib_ENet* lib, ENetAddress* addr, u8 id, ENetPeer* peer, ueBool isHost, ueBool isLocal)
{
	m_isUsed = UE_TRUE;
	m_enet = lib;
	m_enetPeer = peer;
	if (peer)
		peer->data = this;
	m_address = *addr;
	m_id = id;
	m_isHost = isHost;
	m_isLocal = isLocal;
	m_resendFlags = 0;
	m_numPlayers = 0;

	ueStrFormatS(m_debugName, "%s:%u (%u)",
		nsIP::ToString(m_address.host),
		(u32) m_address.port,
		(u32) m_id);
	m_userData = NULL;
}

void mpNode_ENet::Deinit()
{
	m_isUsed = UE_FALSE;

	if (m_enetPeer)
	{
		enet_peer_reset(m_enetPeer);
		m_enetPeer = NULL;
	}
}

ueBool mpNode_ENet::SendTo(mpSendData* _sendData)
{
	UE_ASSERT(_sendData->GetLibType() == MP_ENET_SYMBOL);
	mpSendData_ENet* sendData = (mpSendData_ENet*) _sendData;

	if (!SendPendingManagementMsgs())
		return UE_FALSE;

	return SendMsg(sendData, UE_FALSE);
}

ueBool mpNode_ENet::SendMsg(mpSendData_ENet* sendData, ueBool isManagementMsg)
{
	// Connection lost or not yet established

	return m_enet->SendMsg(m_enetPeer, sendData, isManagementMsg);
}

ueBool mpNode_ENet::SendPendingManagementMsgs()
{
	if (m_enet->IsHost())
	{
		if ((m_resendFlags & ResendFlags_SessionInfo) && !SendMsg_SessionInfo())
			return UE_FALSE;
		if ((m_resendFlags & ResendFlags_GameStarted) && !SendMsg_GameStarted())
			return UE_FALSE;
		if ((m_resendFlags & ResendFlags_GameEnded) && !SendMsg_GameEnded())
			return UE_FALSE;
	}

	for (u32 i = 0; i < m_numPlayers; i++)
	{
		mpPlayer_ENet* player = m_players[i];
		if (!player->SendPendingManagementMsgs())
			return UE_FALSE;
	}

	return UE_TRUE;
}

ueBool mpNode_ENet::SendMsg_GameStarted()
{
	UE_ASSERT(m_resendFlags & ResendFlags_GameStarted);

	IO_BIT_BUFFER_DECL(buffer, 32, ueBitBuffer::Mode_Write);
	buffer.WriteAny<u8>(mpMsgType_ENet_GameStarted);

	mpSendData_ENet sendData;
	sendData.m_buffer = buffer.GetData();
	sendData.m_size = buffer.GetSizeInBytes();
	sendData.m_sendFlags = mpSendFlags_Reliable | mpSendFlags_Ordered;
	if (!SendMsg(&sendData, UE_TRUE))
		return UE_FALSE;

	m_resendFlags &= ~ResendFlags_GameStarted;
	return UE_TRUE;
}

ueBool mpNode_ENet::SendMsg_GameEnded()
{
	UE_ASSERT(m_resendFlags & ResendFlags_GameEnded);

	IO_BIT_BUFFER_DECL(buffer, 32, ueBitBuffer::Mode_Write);
	buffer.WriteAny<u8>(mpMsgType_ENet_GameEnded);

	mpSendData_ENet sendData;
	sendData.m_buffer = buffer.GetData();
	sendData.m_size = buffer.GetSizeInBytes();
	sendData.m_sendFlags = mpSendFlags_Reliable | mpSendFlags_Ordered;
	if (!SendMsg(&sendData, UE_TRUE))
		return UE_FALSE;

	m_resendFlags &= ~ResendFlags_GameEnded;
	return UE_TRUE;
}

ueBool mpNode_ENet::SendMsg_SessionInfo()
{
	UE_ASSERT(m_resendFlags & mpNode_ENet::ResendFlags_SessionInfo);
	UE_ASSERT(m_enet->IsHost());

	IO_BIT_BUFFER_DECL(buffer, 1400, ueBitBuffer::Mode_Write);
	m_enet->CreateSessionInfoMsg(buffer, this, UE_FALSE);

	mpSendData_ENet sendData;
	sendData.m_buffer = buffer.GetData();
	sendData.m_size = buffer.GetSizeInBytes();
	sendData.m_sendFlags = mpSendFlags_Reliable | mpSendFlags_Ordered;
	if (!SendMsg(&sendData, UE_TRUE))
		return UE_FALSE;

	m_resendFlags &= ~mpNode_ENet::ResendFlags_SessionInfo;
	return UE_TRUE;
}

void mpNode_ENet::CommitSendTo()
{
	enet_host_flush(m_enet->m_enetHost);
}

void mpNode_ENet::GetInfo(mpNodeInfo* _info)
{
	UE_ASSERT(_info->GetLibType() == MP_ENET_SYMBOL);
	mpNodeInfo_ENet* info = (mpNodeInfo_ENet*) _info;
	if (m_isHost)
	{
		info->m_ip.sn_addr = m_enet->m_enetHost->address.host;
		info->m_port = m_enet->m_enetHost->address.port;
	}
	else if (m_enetPeer)
	{
		info->m_ip.sn_addr = m_enetPeer->address.host;
		info->m_port = m_enetPeer->address.port;
	}
	else
	{
		info->m_ip.sn_addr = 0;
		info->m_port = 0;
	}
}

u32 mpNode_ENet::GetNumPlayers()
{
	return m_numPlayers;
}

mpPlayer* mpNode_ENet::GetPlayer(u32 index)
{
	UE_ASSERT(index < m_numPlayers);
	return m_players[index];
}

mpPlayer* mpNode_ENet::GetPlayerById(mpPlayerId id)
{
	for (u32 i = 0; i < m_numPlayers; i++)
		if (m_players[i]->m_id == id)
			return m_players[i];
	return NULL;
}

ueBool mpNode_ENet::IsHost()
{
	return m_isHost;
}

ueBool mpNode_ENet::IsLocal()
{
	return m_isLocal;
}

const char* mpNode_ENet::GetDebugName()
{
	return m_debugName;
}

void mpNode_ENet::SetUserData(void* userData)
{
	m_userData = userData;
}

void* mpNode_ENet::GetUserData()
{
	return m_userData;
}

// mpLib_ENet

u32 mpLib_ENet::GetNumNodes()
{
	return m_numNodes;
}

mpNode* mpLib_ENet::GetNode(u32 index)
{
	UE_ASSERT(index < m_numNodes);
	return m_nodes[index];
}

mpNode_ENet* mpLib_ENet::FindNodeById(u8 id)
{
	for (u32 i = 0; i < m_numNodes; i++)
		if (m_nodes[i]->m_id == id)
			return m_nodes[i];
	return NULL;
}

mpNode* mpLib_ENet::GetLocalNode()
{
	return m_localNode;
}

mpNode* mpLib_ENet::GetHostNode()
{
	return m_hostNode;
}

void mpLib_ENet::DropNode(mpNode* _node)
{
	mpNode_ENet* node = (mpNode_ENet*) _node;

	while (node->m_numPlayers > 0)
		DropPlayer(node->m_players[node->m_numPlayers - 1]);
}

mpNode_ENet* mpLib_ENet::AddNode(ENetPeer* peer, ENetAddress* addr, u8 id, ueBool isHost, ueBool isLocal)
{
	UE_ASSERT(m_numNodes < MP_ENET_MAX_NODES);

	mpNode_ENet* node = NULL;
	for (u32 i = 0; i < MP_ENET_MAX_NODES; i++)
		if (!m_nodesPool[i].m_isUsed)
		{
			node = &m_nodesPool[i];
			break;
		}
	UE_ASSERT(node);

	m_nodes[m_numNodes++] = node;

	node->Init(this, addr, id, peer, isHost, isLocal);

	m_callbacks->OnNodeConnected(node);
	return node;
}