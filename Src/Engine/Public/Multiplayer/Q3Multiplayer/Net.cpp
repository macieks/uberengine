// MISC

typedef int FrameID;

struct ObjState
{
  ObjID m_id;
  void* m_data;
};

struct Obj
{
  ObjID m_id;
  void* m_data;
};

// SERVER

struct Server
{
  struct ServerFrame
  {
    FrameID m_frameID;
    std::set<ObjState> m_objStates;
  };

  struct ClientState
  {
    Client* m_client;

    int m_firstFrame;
    int m_numFrames;
    ServerFrame* m_frames;
  };

  struct World
  {
    std::set<Obj> m_objects;
  };

  int m_numClients;
  ClientState* m_clientStates;

  ObjID m_objIDCounter;
  FrameID m_frameIDCounter;

  World m_world;
};

// CLIENT

struct Client
{
  MP* m_mp;

  bool m_isLocal;
  void* m_data;
};

struct LocalClient : Client
{
  Server* m_server;

  struct ClientFrame
  {
    FrameID m_frameID;
    std::set<ObjState> m_objStates;
  };

  struct World
  {
    std::set<Obj> m_objects;
  };

  int m_firstFrame;
  int m_numFrames;
  ClientFrame* m_frames;

  World m_world;
};

struct RemoteClient : Client
{
  bool m_isServer;
};

// MP

struct MP
{
  int m_maxClients;
  int m_numClients;
  RemoteClient* m_remoteClients;
  LocalClient* m_localClient;
  Client* m_server;
};

// FUNCS

MP* MP_Startup(MP_StartupParams* params)
{
  int memorySize = sizeof(MP);

  m_allocator->Alloc(memorySize);
}

void MP_Shutdown(MP* mp)
{
  m_allocator->Free(mp);
}

Client*   MP_AddLocalClient(MP* mp, bool isServer, void* data);
Client*   MP_AddRemoteClient(MP* mp, bool isServer, void* data);
void      MP_RemoveClient(Client* client);

bool      MP_IsLocal(Client* client);
bool      MP_IsServer(Client* client);
Client*   MP_GetServer(MP* mp);

int       MP_GetNumClients(MP* mp);
Client*   MP_GetClient(MP* mp, int index);
void*     MP_GetClientData(Client* client);
void      MP_SetClientData(Client* client, void* data);

///////////////////////
// SECOND VERSION
///////////////////////

// on server:

struct Srv_LocalClientAndServer
{

};

struct Srv_RemoteClient
{

};

// on client

struct Cl_LocalClient
{

};

struct Cl_RemoteClient
{
};

struct Cl_RemoteClientAndServer
{

};