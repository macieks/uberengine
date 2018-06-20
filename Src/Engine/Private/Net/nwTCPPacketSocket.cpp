#include "Net/nwTCPPacketSocket.h"

struct nwTCPPacketSocket
{
	ueAllocator* m_allocator;

	nsSocket m_socket;
	s32 m_lastError;

	u32 m_sendBufferSize;
	u32 m_sendBufferCapacity;
	u8* m_sendBuffer;
	ueTime m_lastSendTime;

	u32 m_recvBufferSize;
	u32 m_recvBufferCapacity;
	u8* m_recvBuffer;
	ueTime m_lastRecvTime;
};

nwTCPPacketSocket* nwTCPPacketSocket_Create(ueAllocator* allocator, u32 sendBufferSize, u32 recvBufferSize)
{
	u8* memory = (u8*) allocator->Alloc(sizeof(nwTCPPacketSocket) + sendBufferSize + recvBufferSize);
	if (!memory)
		return NULL;

	nwTCPPacketSocket* socket = (nwTCPPacketSocket*) memory;
	socket->m_allocator = allocator;
	socket->m_socket = 0;
	socket->m_lastError = 0;
	memory += sizeof(nwTCPPacketSocket);

	socket->m_sendBufferCapacity = sendBufferSize;
	socket->m_sendBufferSize = 0;
	socket->m_sendBuffer = sendBufferSize ? memory : NULL;
	memory += sendBufferSize;

	socket->m_recvBufferCapacity = recvBufferSize;
	socket->m_recvBufferSize = 0;
	socket->m_recvBuffer = recvBufferSize ? memory : NULL;

	return socket;
}

void nwTCPPacketSocket_Destroy(nwTCPPacketSocket* socket)
{
	socket->m_allocator->Free(socket);
}

void nwTCPPacketSocket_Reset(nwTCPPacketSocket* socket, nsSocket sock)
{
	socket->m_socket = sock;
	socket->m_lastError = 0;

	socket->m_sendBufferSize = 0;
	socket->m_lastSendTime = ueClock_GetCurrent();

	socket->m_recvBufferSize = 0;
	socket->m_lastRecvTime = ueClock_GetCurrent();
}

nsSocket nwTCPPacketSocket_GetSocket(nwTCPPacketSocket* socket)
{
	return socket->m_socket;
}

ueBool nwTCPPacketSocket_IsError(nwTCPPacketSocket* socket, s32& socketErr)
{
	socketErr = socket->m_lastError;
	return socketErr != 0;
}

ueBool nwTCPPacketSocket_BufferSend(nwTCPPacketSocket* socket, const void* data, u32 dataSize, s32& socketErr)
{
	if (socket->m_sendBufferSize + dataSize > socket->m_sendBufferCapacity)
		return UE_FALSE;

	ueMemCpy(socket->m_sendBuffer + socket->m_sendBufferSize, data, dataSize);
	return UE_TRUE;
}

ueBool nwTCPPacketSocket_BufferSendArray(nwTCPPacketSocket* socket, u32 totalSize, u32 numChunks, const void** dataArray, const u32* dataSizeArray, s32& socketErr)
{
	// Check we have enough space

	if (socket->m_sendBufferSize + totalSize > socket->m_sendBufferCapacity)
		return UE_FALSE;

	// Copy all chunks

	for (u32 i = 0; i < numChunks; i++)
	{
		ueMemCpy(socket->m_sendBuffer + socket->m_sendBufferSize, dataArray[i], dataSizeArray[i]);
		socket->m_sendBufferSize += dataSizeArray[i];
	}
	return UE_TRUE;
}

ueBool nwTCPPacketSocket_SendImmediately(nwTCPPacketSocket* socket, const void* data, u32 dataSize, s32& socketErr)
{
	// Send

	socketErr = nsSocket_Send(socket->m_socket, data, dataSize, 0);
	if (socketErr < 0)
	{
		if (nsSocket_IsWouldBlock(socketErr))
			socketErr = 0;
		else
			socket->m_lastError = socketErr;
		return UE_FALSE;
	}

	socket->m_lastSendTime = ueClock_GetCurrent();

	// If failed to send everything, put the rest into buffer

	if (socketErr < (s32) dataSize)
		ueMemCpy(socket->m_sendBuffer + socket->m_sendBufferSize, (u8*) data + socketErr, dataSize - socketErr);

	// Indicate no error

	socketErr = 0;
	return UE_TRUE;
}

ueBool nwTCPPacketSocket_Send(nwTCPPacketSocket* socket, const void* data, u32 dataSize, s32& socketErr)
{
	UE_ASSERT(dataSize <= socket->m_sendBufferCapacity);

	if (socket->m_lastError)
	{
		socketErr = socket->m_lastError;
		return UE_FALSE;
	}

	// Send old messages

	nwTCPPacketSocket_TickSend(socket, socketErr);
	if (socketErr < 0)
		return UE_FALSE;

	// Try to send immediately (only if there's no pending data)

	if (socket->m_sendBufferSize == 0)
		return nwTCPPacketSocket_SendImmediately(socket, data, dataSize, socketErr);

	// Otherwise store data in intermediate buffer

	if (!nwTCPPacketSocket_BufferSend(socket, data, dataSize, socketErr))
		return UE_FALSE;

	// Try sending again

	nwTCPPacketSocket_TickSend(socket, socketErr);
	if (socketErr < 0)
		return UE_FALSE;

	return UE_TRUE;
}

ueBool nwTCPPacketSocket_SendArray(nwTCPPacketSocket* socket, u32 numChunks, const void** dataArray, const u32* dataSizeArray, s32& socketErr)
{
	if (socket->m_lastError)
	{
		socketErr = socket->m_lastError;
		return UE_FALSE;
	}

	// Get total size of all chunks

	u32 totalSize = 0;
	for (u32 i = 0; i < numChunks; i++)
		totalSize += dataSizeArray[i];

	UE_ASSERT(totalSize <= socket->m_sendBufferCapacity);

	// Send old messages

	nwTCPPacketSocket_TickSend(socket, socketErr);
	if (socketErr < 0)
		return UE_FALSE;

	// Store all data chunks in intermediate buffer

	if (!nwTCPPacketSocket_BufferSendArray(socket, totalSize, numChunks, dataArray, dataSizeArray, socketErr))
		return UE_FALSE;

	// Try sending again

	nwTCPPacketSocket_TickSend(socket, socketErr);
	if (socketErr < 0)
		return UE_FALSE;

	return UE_TRUE;
}

void nwTCPPacketSocket_TickSend(nwTCPPacketSocket* socket, s32& socketErr)
{
	if (socket->m_lastError)
	{
		socketErr = socket->m_lastError;
		return;
	}

	// Nothing to send?

	if (socket->m_sendBufferSize == 0)
	{
		socketErr = 0;
		return;
	}

	// Send

	socketErr = nsSocket_Send(socket->m_socket, socket->m_sendBuffer, socket->m_sendBufferSize, 0);
	if (socketErr < 0)
	{
		if (nsSocket_IsWouldBlock(socketErr))
			socketErr = 0;
		else
			socket->m_lastError = socketErr;
		return;
	}

	socket->m_lastSendTime = ueClock_GetCurrent();

	// Pop the rest of pending send data to the beginning of the buffer

	socket->m_sendBufferSize -= socketErr;
	if (socket->m_sendBufferSize > 0)
		memmove(socket->m_sendBuffer, socket->m_sendBuffer + socketErr, socket->m_sendBufferSize);

	// Indicate no error

	socketErr = 0;
}

ueTime nwTCPPacketSocket_GetLastSendTime(nwTCPPacketSocket* socket)
{
	return socket->m_lastRecvTime;
}

ueBool nwTCPPacketSocket_WouldRecv(nwTCPPacketSocket* socket, const void** dataPtr, u32 numBytes, s32& socketErr)
{
	UE_ASSERT(numBytes <= socket->m_recvBufferCapacity);

	if (socket->m_lastError)
	{
		socketErr = socket->m_lastError;
		return UE_FALSE;
	}

	// Check if we've already received enough data

	if (numBytes <= socket->m_recvBufferSize)
	{
		if (dataPtr) *dataPtr = socket->m_recvBuffer;
		return UE_TRUE;
	}

	// Attempt to receive the data

	socketErr = nsSocket_Receive(socket->m_socket, socket->m_recvBuffer + socket->m_recvBufferSize, numBytes - socket->m_recvBufferSize, 0);
	if (socketErr < 0)
	{
		if (nsSocket_IsWouldBlock(socketErr))
			socketErr = 0;
		else
			socket->m_lastError = socketErr;
		return UE_FALSE;
	}

	// Data received

	socket->m_recvBufferSize += socketErr;
	socket->m_lastRecvTime = ueClock_GetCurrent();

	if (numBytes <= socket->m_recvBufferSize)
	{
		if (dataPtr) *dataPtr = socket->m_recvBuffer;
		return UE_TRUE;
	}

	// Still not enough data received

	return UE_FALSE;
}

ueBool nwTCPPacketSocket_Recv(nwTCPPacketSocket* socket, void* buffer, u32 numBytes, s32& socketErr)
{
	if (socket->m_lastError)
	{
		socketErr = socket->m_lastError;
		return false;
	}

	// Check if we'd receive all requested bytes

	if (!nwTCPPacketSocket_WouldRecv(socket, NULL, numBytes, socketErr))
		return UE_FALSE;

	// Copy result

	ueMemCpy(buffer, socket->m_recvBuffer, numBytes);

	// Pop the rest of pending received data to the beginning of the buffer

	socket->m_recvBufferSize -= numBytes;
	if (socket->m_recvBufferSize > 0)
		ueMemMove(socket->m_recvBuffer, socket->m_recvBuffer + numBytes, socket->m_recvBufferSize);

	return UE_TRUE;
}

ueTime nwTCPPacketSocket_GetLastRecvTime(nwTCPPacketSocket* socket)
{
	return socket->m_lastRecvTime;
}