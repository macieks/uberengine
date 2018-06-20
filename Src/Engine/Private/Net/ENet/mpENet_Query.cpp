#include "Net/ENet/mpENet_Private.h"

ueBool mpQuery_ENet::IsDone()
{
	return m_isDone;
}

void mpQuery_ENet::Destroy()
{
	m_enet->DestroyQuery(this);
}

void mpQuery_ENet::GetResult(mpQueryResult* _result)
{
	UE_ASSERT(_result->GetLibType() == MP_ENET_SYMBOL);
	mpQueryResult_ENet* result = (mpQueryResult_ENet*) _result;

	UE_ASSERT(m_isDone);

	result->m_numSessions = m_numSessions;
	result->m_sessions = m_sessions;
}

void mpQuery_ENet::UpdateResults(ENetAddress* addr, void* data, u32 size)
{
	// Read first session info part

	ueBitBuffer buffer(data, size, ueBitBuffer::Mode_Read);

	u8 msgType = 0;
	if (!buffer.ReadAny<u8>(msgType) || msgType != mpMsgType_ENet_SessionInfo) return;

	u32 titleId = 0;
	if (!buffer.ReadAny(titleId)) return;
	if (titleId != m_enet->m_titleId) return;

	u32 sessionId = 0;
	if (!buffer.ReadAny(sessionId)) return;

	u16 port = 0;
	if (!buffer.ReadAny(port)) return;

	// Get storage for session info

	ueBool newSession = UE_FALSE;
	mpQueryResult_ENet::Session* session = NULL;
	mpPropertySet_ENet* sessionProps = NULL;
	for (u32 i = 0; i < m_numSessions; i++)
		if (m_sessions[i].m_sessionId == sessionId)
		{
			session = &m_sessions[i];
			sessionProps = &m_sessionProps[i];
			break;
		}
	if (!session)
	{
		if (m_numSessions == m_maxResults)
			return;
		session = &m_sessions[m_numSessions];
		sessionProps = &m_sessionProps[m_numSessions];
		newSession = UE_TRUE;
	}

	// Read the rest of session info

	u8 numNodes = 0;
	if (!buffer.ReadAny(numNodes)) return;
	u8 maxNodes = 0;
	if (!buffer.ReadAny(maxNodes)) return;

	u8 numPlayers = 0;
	if (!buffer.ReadAny(numPlayers)) return;
	u8 maxPlayers = 0;
	if (!buffer.ReadAny(maxPlayers)) return;

	mpPropertySet_ENet props;
	if (!props.Read(buffer)) return;

	// Match properties

	if (!m_props.Match(&props)) return;

	// Success! - copy session information

	sessionProps->CopyFrom(&props);

	session->m_ip.sn_addr = addr->host;
	session->m_port = port;
	session->m_sessionId = sessionId;
	session->m_numPlayers = numPlayers;
	session->m_maxPlayers = maxPlayers;
	session->m_numNodes = numNodes;
	session->m_maxNodes = maxNodes;
	session->m_numProps = sessionProps->m_count;
	session->m_props = sessionProps->m_props;

	if (newSession)
		m_numSessions++;
}

void mpQuery_ENet::SetUserData(void* userData)
{
	m_userData = userData;
}

void* mpQuery_ENet::GetUserData()
{
	return m_userData;
}

// mpENet

mpQuery* mpLib_ENet::CreateQuery(mpQueryParams* _params)
{
	UE_ASSERT(_params->GetLibType() == MP_ENET_SYMBOL);
	mpQueryParams_ENet* params = (mpQueryParams_ENet*) _params;

	UE_ASSERT(params->m_maxResults <= MP_ENET_MAX_QUERY_RESULTS);

	if (m_numQueries == MP_ENET_MAX_QUERIES)
		return NULL;

	if (m_numQueries == 0)
	{
		// Set up broadcasting socket

		UE_ASSERT(!m_broadcastRecvSocket);
		m_broadcastRecvSocket = enet_socket_create(ENET_SOCKET_TYPE_DATAGRAM);
		if (m_broadcastRecvSocket == ENET_SOCKET_NULL)
		{
			ueLogE("Failed to create broadcast-recv socket: %s", enet_socket_last_error_str());
			return NULL;
		}

		if (enet_socket_set_option(m_broadcastRecvSocket, ENET_SOCKOPT_NONBLOCK, 1))
		{
			ueLogE("Failed to set broadcast-recv socket to non-blocking mode.");
			enet_socket_destroy(m_broadcastRecvSocket);
			m_broadcastRecvSocket = 0;
			return NULL;
		}

		if (enet_socket_set_option(m_broadcastRecvSocket, ENET_SOCKOPT_REUSEADDR, 1))
		{
			ueLogE("Failed to set broadcast-recv socket to reuse-addr mode.");
			enet_socket_destroy(m_broadcastRecvSocket);
			m_broadcastRecvSocket = 0;
			return NULL;
		}

		ENetAddress addr;
		addr.host = ENET_HOST_ANY;
		addr.port = m_broadcastPort;
		if (enet_socket_bind(m_broadcastRecvSocket, &addr) < 0)
		{
			ueLogE("Failed to bind broadcast-recv socket: %s", enet_socket_last_error_str());
			enet_socket_destroy(m_broadcastRecvSocket);
			m_broadcastRecvSocket = 0;
			return NULL;
		}
	}

	// Alloc query

	mpQuery_ENet* query = NULL;
	for (u32 i = 0; i < MP_ENET_MAX_QUERIES; i++)
		if (!m_queriesPool[i].m_isUsed)
		{
			query = &m_queriesPool[i];
			break;
		}
	UE_ASSERT(query);

	// Set up query

	query->m_isUsed = UE_TRUE;
	query->m_enet = this;
	query->m_startTime = ueClock_GetCurrent();
	query->m_maxResults = params->m_maxResults;
	query->m_timeOutSecs = params->m_timeOutSecs;
	query->m_isDone = UE_FALSE;
	query->m_numSessions = 0;
	query->m_props.CopyFrom(params->m_numProps, params->m_props);
	query->m_userData = NULL;

	m_queries[m_numQueries++] = query;

	return query;
}

void mpLib_ENet::DestroyQuery(mpQuery_ENet* query)
{
	for (u32 i = 0; i < m_numQueries; i++)
		if (m_queries[i] == query)
		{
			m_queries[i] = m_queries[--m_numQueries];
			break;
		}

	query->m_isUsed = UE_FALSE;

	if (m_numQueries == 0)
	{
		enet_socket_destroy(m_broadcastRecvSocket);
		m_broadcastRecvSocket = 0;
	}
}

void mpLib_ENet::UpdateQueries()
{
	u32 numActiveQueries = 0;
	for (u32 i = 0; i < m_numQueries; i++)
	{
		mpQuery_ENet* query = m_queries[i];
		if (!query->m_isDone)
			numActiveQueries++;
	}
	if (numActiveQueries == 0)
		return;

	while (numActiveQueries)
	{
		ENetAddress addr;
		ueMemSet(&addr, 0, sizeof(addr));
		s32 addrSize = sizeof(addr);

		u8 data[1400];

		ENetBuffer enetBuffer;
		enetBuffer.data = data;
		enetBuffer.dataLength = UE_ARRAY_SIZE(data);

		s32 len = enet_socket_receive(m_broadcastRecvSocket, &addr, &enetBuffer, 1);
		if (len <= 0)
			break;

		for (u32 i = 0; i < m_numQueries; i++)
		{
			mpQuery_ENet* query = m_queries[i];
			if (query->m_isDone)
				continue;
			query->UpdateResults(&addr, data, len);
		}
	}

	for (u32 i = 0; i < m_numQueries; i++)
	{
		mpQuery_ENet* query = m_queries[i];
		if (query->m_isDone)
			continue;

		if (ueClock_GetSecsSince(query->m_startTime) > query->m_timeOutSecs)
		{
			query->m_isDone = UE_TRUE;
			m_callbacks->OnQueryDone(query);
		}
	}
}