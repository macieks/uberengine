// Quake 3 inspired multiplayer

struct q3Server;
struct q3Client;
struct q3Node;

struct ueBitBuffer;
struct ueAllocator;

////////////////////

typedef u32 q3ObjID;
typedef u32 q3Timestamp;

#define q3Lib_INVALID_OBJ_ID       0xFFFFFFFF
#define q3Lib_MAX_RELEVANT_OBJECTS 1024

struct q3Lib;
struct q3Node;

typedef void	(*q3Server_WriteStateDiffFunc)(q3Node* dstClient, void* fromStateData, void* toStateData, ueBitBuffer* diffOutput);

typedef void*	(*q3Client_CreateStateFromDiffFunc)(void* fromStateData, ueBitBuffer* diffInput);
typedef void	(*q3Client_CreateObjectFunc)(q3ObjID id, void* stateData, void*& outObjectData);
typedef ueBool	(*q3Client_UpdateObjectFunc)(q3ObjID id, void* objectData, void* stateData);
typedef void	(*q3Client_DestroyObjectFunc)(q3ObjID id, void* objectData);

typedef void	(*q3Lib_DestroyStateFunc)(void* stateData);

// Q: Having separate local server and local client worlds means we need to run 2 physics simulations on the server

struct q3Lib_StartupParams
{
	ueAllocator* m_allocator;

	u32 m_maxClients;
	u32 m_maxObjects;
	u32 m_maxFrames;

	// Invoked during q3Server_GenerateUpdateForClient()

	q3Server_WriteStateDiffFunc m_writeStateDiffFunc;

	// Invoked during q3Client_ReceiveUpdateFromServer()

	q3Client_CreateStateFromDiffFunc m_createStateFromDiffFunc;
	q3Client_CreateObjectFunc m_createObjectFunc;
	q3Client_UpdateObjectFunc m_updateObjectFunc;
	q3Client_DestroyObjectFunc m_destroyObjectFunc;  // Also invoked on reset & shutdown

	// Invoked during q3Server_EndFrame() on server or q3Client_ReceiveUpdateFromServer() on client

	q3Lib_DestroyStateFunc m_destroyStateFunc;  // Also invoked on reset & shutdown
};

// Common
// ------

q3Lib*		q3Lib_Startup(q3Lib_StartupParams* params);
void		q3Lib_Shutdown(q3Lib* q3Lib);

void		q3Lib_Reset(q3Lib* q3Lib);

f32			q3Lib_SecsBetween(q3Timestamp start, q3Timestamp end);

q3Node*		q3Lib_AddLocalNode(q3Lib* q3Lib, ueBool isServer, void* data);
q3Node*		q3Lib_AddRemoteNode(q3Lib* q3Lib, ueBool isServer, void* data);
void		q3Lib_RemoveNode(q3Node* q3Node);

ueBool		q3Lib_IsLocal(q3Node* q3Node);
q3Node*		q3Lib_GetLocalNode(q3Lib* q3Lib);
ueBool		q3Lib_IsServer(q3Lib* q3Lib);
ueBool		q3Lib_IsServer(q3Node* q3Node);
q3Node*		q3Lib_GetServer(q3Lib* q3Lib);

u32			q3Lib_GetNumNodes(q3Lib* q3Lib);
q3Node*		q3Lib_GetNode(q3Lib* q3Lib, u32 index);
void*		q3Lib_GetNodeData(q3Node* q3Node);
void		q3Lib_SetNodeData(q3Node* q3Node, void* data);

void		q3Lib_SetObjectData(q3ObjID id, void* data);
void*		q3Lib_GetObjectData(q3ObjID id);

// Server
// ------

q3ObjID		q3Server_GenerateObjectID(q3Lib* q3Lib);

void		q3Server_ProgressTime(q3Lib* q3Lib, f32 dt);
f32			q3Server_TimeSinceLastFrame(q3Lib* q3Lib);

void		q3Server_BeginFrame(q3Lib* q3Lib);
void		q3Server_StoreState(q3Lib* q3Lib, q3ObjID id, void* stateData);
void		q3Server_EndFrame(q3Lib* q3Lib);

ueBool		q3Server_ReceiveUpdateFromClient(q3Node* client, ueBitBuffer* input);
//! Data generated contains timestamp and thus shall be sent immediately for time synchronization reasons
void		q3Server_GenerateUpdateForClient(q3Node* client, ueBitBuffer* output);

void		q3Server_GenerateMigrationInfoForNewServer(q3Node* newServer, ueBitBuffer* output);
void		q3Server_GenerateMigrationInfoForClient(q3Node* client, q3Node* newServer, ueBitBuffer* output);

// Client
// ------

void		q3Client_GetTimestamps(q3Lib* q3Lib, q3Timestamp& prevTimestamp, q3Timestamp& lastTimestamp, q3Timestamp& currEstimatedTimestamp);

void		q3Client_GenerateUpdateForServer(q3Lib* q3Lib, ueBitBuffer* output);
ueBool		q3Client_ReceiveUpdateFromServer(q3Lib* q3Lib, ueBitBuffer* input);

ueBool		q3Client_ReceiveMigrationInfoFromOldServer(ueBitBuffer* input);