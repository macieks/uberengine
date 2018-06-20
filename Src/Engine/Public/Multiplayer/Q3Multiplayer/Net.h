struct Buffer;
struct Allocator;

////////////////////

typedef int ObjID;
typedef int Timestamp;

#define MP_INVALID_OBJ_ID       0xFFFFFFFF
#define MP_MAX_RELEVANT_OBJECTS 1024

struct MP;
struct Node;

typedef void (*MP_Srv_WriteStateDiffFunc)(Node* dstClient, void* fromStateData, void* toStateData, Buffer* diffOutput);

typedef void* (*MP_Cl_CreateStateFromDiffFunc)(void* fromStateData, Buffer* diffInput);
typedef void* (*MP_Cl_CreateObjectFunc)(ObjID id, void* stateData);
typedef bool (*MP_Cl_UpdateObjectFunc)(ObjID id, void* objectData, void* stateData);
typedef void (*MP_Cl_DestroyObjectFunc)(ObjID id, void* objectData);

typedef void (*MP_DestroyStateFunc)(void* state, void* stateData);

// Q: Having separate local server and local client worlds means we need to run 2 physics simulations on the server

struct MP_StartupParams
{
  Allocator* m_allocator;

  int m_maxClients;
  int m_maxObjects;
  int m_maxFrames;

  // Invoked during MP_Srv_GenerateUpdateForClient()

  MP_Srv_WriteStateDiffFunc m_writeStateDiffFunc;

  // Invoked during MP_Cl_ReceiveUpdateFromServer()

  MP_Cl_CreateStateFromDiffFunc m_createStateFromDiffFunc;
  MP_Cl_CreateObjectFunc m_createObjectFunc;
  MP_Cl_UpdateObjectFunc m_updateObjectFunc;
  MP_Cl_DestroyObjectFunc m_destroyObjectFunc;  // Also invoked on reset & shutdown

  // Invoked during MP_Srv_EndFrame() on server or MP_Cl_ReceiveUpdateFromServer() on client

  MP_DestroyStateFunc m_destroyStateFunc;  // Also invoked on reset & shutdown
};

MP*       MP_Startup(MP_StartupParams* params);
void      MP_Shutdown(MP* mp);

void      MP_Reset(MP* mp);

float     MP_SecsBetween(Timestamp start, Timestamp end);

// Node
// ------

// WHAT ABOUT TIME SYNCING ???

void      MP_Cl_GetTimestamps(MP* mp, Timestamp& prevTimestamp, Timestamp& lastTimestamp, Timestamp& currEstimatedTimestamp);

void      MP_Cl_GenerateAckForServer(MP* mp, Buffer* output);
bool      MP_Cl_ReceiveUpdateFromServer(MP* mp, Buffer* input);

bool      MP_Cl_ReceiveMigrationInfoFromOldServer(Buffer* input);

// Server
// ------

//--------
Node*     MP_AddLocalNode(MP* mp, bool isServer, void* data);
Node*     MP_AddRemoteNode(MP* mp, bool isServer, void* data);
void      MP_RemoveNode(Node* node);

bool      MP_IsLocal(Node* node);
Node*     MP_GetLocalNode(MP* mp);
bool      MP_IsServer(MP* mp);
bool      MP_IsServer(Node* node);
Node*     MP_GetServer(MP* mp);

// DOES CLIENT NEED ABILITY TO ENUM ALL NODES (or only needs itself and server?)
int       MP_GetNumNodes(MP* mp);
Node*     MP_GetNode(MP* mp, int index);
void*     MP_GetNodeData(Node* node);
void      MP_SetNodeData(Node* node, void* data);

//--------

ObjID     MP_Srv_GenerateObjectID(MP* mp);

void      MP_Srv_ProgressTime(MP* mp, float dt);
float     MP_Srv_TimeSinceLastFrame(MP* mp);

void      MP_Srv_BeginFrame(MP* mp);
void      MP_Srv_StoreState(MP* mp, ObjID id, void* stateData);
void      MP_Srv_EndFrame(MP* mp);

bool      MP_Srv_ReceiveAckFromClient(Node* client, Buffer* input);
//! Data generated contains timestamp and thus shall be sent immediately for time synchronization reasons
void      MP_Srv_GenerateUpdateForClient(Node* client, Buffer* output);

void      MP_Srv_GenerateMigrationInfoForNewServer(Node* newServer, Buffer* output);
void      MP_Srv_GenerateMigrationInfoForClient(Node* client, Node* newServer, Buffer* output);