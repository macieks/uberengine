// Sent over network
struct DebugNetBroadcastMsg
{
	char m_appName[64];
	char m_computerName[64];
	char m_platform[16];
};

// Sent over network
struct DebugNetConnectionInfo
{
	u32 m_appSymbol;
	char m_computerName[64];
};

struct DebugNetConnection_Internal : public DebugNetConnection
{
	Socket m_socket;
};

struct DebugNetListener : DebugNetListenerDesc
{
	u32 m_id;
};

struct DebugNetData
{
	Socket m_listeningSocket;

	DebugNetBroadcastMsg m_broadcastMsg;
	Socket m_broadcastSocket;
	Ticks m_lastBroadcastTime;

	Thread m_recvDataThread;
	volatile bool m_stopRecvDataThread;

	u32 m_maxConnections;
	u32 m_numConnections;
	DebugNetConnection_Internal* m_connections;
	Mutex m_connectionsMutex;

	u32 m_maxListeners;
	u32 m_numListeners;
	DebugNetListener* m_listeners;
	Mutex m_listenersMutex;

	u32 m_connectionIdCounter;
	u32 m_listenerIdCounter;

	Mutex m_msgMutex;
	RingBuffer m_msgBuffer;

	int m_largestMsgSize;
	char* m_largestMsgBuffer;
};

struct MsgHeader
{
	u32 m_size;
	u32 m_symbol;
};

static DebugNetData s_data;

void DebugNet_RecvData(DebugNetConnection& conn)
{
	MsgHeader header;
	u32 msgSize = 0;
	while (1)
	{
		int len = Socket_Recv(conn.m_socket, &header, sizeof(MsgHeader), 0);

		// Nothing to read?

		if (Socket_IsWouldBlock(len))
			return;

		// Error?

		if (len < 0)
		{
			printf(stderr, "Failed to receive data: %s.", Socket_GetErrorString());
			conn.m_isValid = UE_FALSE;
			return;
		}

		if (len != sizeof(MsgHeader))
		{
			printf(stderr, "Failed to receive message size.");
			conn.m_isValid = UE_FALSE;
			return;
		}

		// 0-byte message?

		if (header.m_size == sizeof(MsgHeader))
			break;

		// Too large message?

		if (header.m_size > s_data.m_largestMsgBuffer)
		{
			printf(stderr, "Failed to receive message: size too large.");
			conn.m_isValid = UE_FALSE;
			return;
		}

		// Receive whole message

		len = Socket_Recv(conn.m_socket, s_data.m_largestMsgBuffer + sizeof(MsgHeader), header.m_size - sizeof(MsgHeader), BLOCK);
		if (len != msgSize)
		{
			printf(stderr, "Failed to receive message: size too large.");
			conn.m_isValid = UE_FALSE;
			return;
		}
	}

	// Create complete message

	*(u32*) s_data.m_largestMsgBuffer = conn.m_id;
	*((u32*) s_data.m_largestMsgBuffer + 1) = header.m_symbol;

	// Copy to ring buffer

	while (1)
	{
		MutexLock lock(s_data.m_msgMutex);
		if (s_data.m_msgBuffer.Push(s_data.m_largestMsgBuffer, header.m_size))
		{
			conn.m_numQueuedMsgs++;
			break;
		}
	}
}

void DebugNet_RecvDataThreadFunc(void* userData)
{
	while (!s_data.m_stopRecvDataThread)
	{
		Thread_Yield();

		MutexLock lock(s_data.m_connectionsMutex);
		for (u32 i = 0; i < s_data.m_numConnections;)
		{
			DebugNetConnection& conn = s_data.m_connections[i];
			if (!conn.m_isValid)
				continue;

			DebugNet_RecvData(conn);
			break;
		}
	}
}

void DebugNet_Startup(DebugNetStartupParams& params)
{
	TODO

	// Set up broadcast msg

	strcpy(s_data.m_broadcastMsg.m_appName, params.m_appName);
#if defined(WIN32)
	strcpy(s_data.m_broadcastMsg.m_computerName, GetComputerName());
#else
	strcpy(s_data.m_broadcastMsg.m_computerName, "<unnamed>");
#endif
	strcpy(s_data.m_broadcastMsg.m_platform, App_GetPlatformName());
}

void DebugNet_Shutdown()
{
	TODO
}

void DebugNet_DispatchMsg(u32 connectionId, u32 msgSymbol, const void* data, u32 dataSize)
{
	MutexLock lock(s_data.m_listenersMutex);
	for (u32 i = 0; i < s_data.m_numListeners; i++)
	{
		DebugNetListener& listener = s_data.m_listeners[i];
		if (listener.m_msgSymbol == msgSymbol)
			listener.m_recvFunc(connectionId, msgSymbol, data, dataSize, listener.m_userData);
	}
}

void DebugNet_Update()
{
	// Remove invalid connections
	{
		MutexLock lock(s_data.m_connectionsMutex);
		for (u32 i = 0; i < s_data.m_numConnections;)
			if (!s_data.m_connections[i].m_isValid)
				DebugNet_RemoveConnection_Index(i);
			else
				i++;
	}

	// Broadcast info

	if (Time_SecsSince(s_data.m_lastBroadcastTime))
	{
		s_data.m_lastBroadcastTime = Time_Current();
		if (Socket_Send(s_data.m_broadcastSocket, &s_data.m_broadcastMsg, sizeof(s_data.m_broadcastMsg)) != sizeof(s_data.m_broadcastMsg))
			printf("Failed to send broadcast msg.");
	}

	// Accept new connections

	while (1)
	{
		SocketAddr addr;
		u32 addrSize = sizeof(addr);
		Socket accepted = Socket_Accept(s_data.m_listeningSocket, &addr, addrSize);

		if (accepted == SOCKET_INVALID)
			break;

		MutexLock lock(s_data.m_connectionsMutex);

		if (s_data.m_numConnections == s_data.m_maxConnections)
		{
			printf("Failed to accept new connection - max connection count reached.");
			Socket_Shutdown(accepted);
			Socket_Close(accepted);
			break;
		}

		DebugNetConnection& conn = s_data.m_connections[s_data.m_numConnections];
		const f32 timeOut = 0.5f;
		if (Socket_RecvBlocking(accepted, (DebugNetConnection*) &conn, sizeof(DebugNetConnection), timeOut) != sizeof(DebugNetConnection))
		{
			printf("Failed to receive connection info.");
			Socket_Shutdown(accepted);
			Socket_Close(accepted);
			break;
		}

		s_data.m_numConnections++;

		conn.m_isValid = true;
		conn.m_id = s_data.m_connectionIdCounter++;
		conn.m_socket = accepted;

		// Let all listeners know about new connection

		MutexLock lock(s_data.m_listenersMutex);
		for (u32 i = 0; i < s_data.m_numListeners; i++)
		{
			DebugNetListener& listener = s_data.m_listeners[i];
			listener.m_connectedFunc(newConn, listener.m_userData);
		}
	}

	// Dispatch messages

	while (1)
	{
		MutexLock lock(s_data.m_msgMutex);

		u32 dataSize = 0;
		unsigned char* data = s_data.m_msgBuffer.Pop(dataSize);

		if (data)
		{
			const u32 connectionId = *(u32*) data;
			data += sizeof(u32);

			const u32 msgSymbol = FromLittleEndian(*(u32*) data);
			data += sizeof(u32);

			DebugNet_DispatchMsg(connectionId, msgSymbol, data, dataSize - sizeof(u32) * 2);
		}
	}
}

u32 DebugNet_RegisterListener(DebugNetListenerDesc& desc)
{
	MutexLock lock(s_data.m_listenersMutex);

	DebugNetListener listener;
	*(DebugNetListenerDesc*) &listener = desc;
	listener.m_id = s_data.m_listenerIdCounter++;

	array_push_back(listener, s_data.m_listeners, s_data.m_numListeners, s_data.m_maxListeners);
}

void DebugNet_UnregisterListener(u32 listenerId)
{
	MutexLock lock(s_data.m_listenersMutex);
	for (u32 i = 0; i < s_data.m_numListeners; i++)
		if (s_data.m_listeners[i].m_id == listenerId)
		{
			array_remove_at_pop(s_data.m_listeners, s_data.m_numListeners, i);
			return;
		}
}

void DebugNet_RemoveConnection_Index(u32 connectionIndex)
{
	DebugNetConnection& conn = s_data.m_connections[connectionIndex];

	// Let all listeners know about disconnection
	{
		MutexLock lock(s_data.m_listenersMutex);
		for (u32 i = 0; i < s_data.m_numListeners; i++)
		{
			DebugNetListener& listener = s_data.m_listeners[i];
			listener.m_connectedFunc(conn.m_id, conn.m_info.m_appSymbol, listener.m_userData);
		}
	}

	Socket_Shutdown(conn.m_socket);
	Socket_Close(conn.m_socket);

	array_remove_at_pop(s_data.m_connections, s_data.m_maxConnections, s_data.m_numConnections, connectionIndex)
}

bool DebugNet_Send_Index(u32 connectionIndex, u32 msgSymbol, const unsigned char* data, u32 dataSize)
{
	DebugNetConnection& conn = s_data.m_connections[connectionIndex];

	u32 sent = 0;
	while (sent < dataSize)
	{
		const u32 sentNow = Socket_Send(conn.m_socket, data + sent, dataSize - sent);

		if (Socket_IsWouldBlock(sentNow))
		{
			Thread_Yield();
			continue;
		}
		else if (sentNow < 0)
		{
			printf("Failed to send data, reason: %s", Socket_ErrorString(sentNow));
			return UE_FALSE;
		}

		sent += sentNow;
	}

	return true;
}

bool DebugNet_Send(u32 connectionId, u32 msgSymbol, const void* data, u32 dataSize)
{
	MutexLock lock(s_data.m_connectionsMutex);

	if (connectionId == DEBUG_NET_ALL_CONNECTIONS_ID)
	{
		bool success = true;
		for (u32 i = 0; i < s_data.m_numConnections;)
			if (!DebugNet_Send_Index(i, msgSymbol, data, dataSize))
			{
				DebugNet_RemoveConnection_Index(i);
				success = UE_FALSE;
			}
			else
				i++;
		return success;
	}

	for (u32 i = 0; i < s_data.m_numConnections; i++)
		if (s_data.m_connections[i].m_id == connectionId)
			return DebugNet_Send_Index(i, msgSymbol, data, dataSize);

	return UE_FALSE;
}