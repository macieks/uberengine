#define SRV_FRAME_FREQUENCY (1.0f / 60.0f)

struct vec2
{
  float x, y;

  void WriteDiff(vec2 fromPos, Buffer* output)
  {
    WriteDiff(fromPos.x, x, output);
    WriteDiff(fromPos.y, y, output);
  }

  bool ReadDiff(vec2 fromPos, Buffer* output)
  {
    return
      ReadDiff(fromPos.x, x, input) &&
      ReadDiff(fromPos.y, y, input);
  }
};

MP* mp;

struct Connection
{
  bool HasAvailableBandwidth();
  bool SendUnreliable(Buffer* buffer);
};

struct GameClient
{
  Connection* connection;
  Node* client;

  bool m_migrationInfoSent;
};

struct NetGameObjectState
{
  int type;
  float pos;
  bool isAlive;

  void Srv_WriteStateDiff(NetGameObjectState* fromState, Buffer* output)
  {
    // Create

    if (!fromState)
    {
      output->Write<NetGameObjectState>(*this);
      return;
    }

    // Update

    fromState->pos.WriteDiff(output);
    output->WriteBit(isAlive);
  }

  bool Cl_ReadStateDiff(NetGameObjectState* fromState, Buffer* input)
  {
    // Create

    if (!fromState)
      return input->Read<NetGameObjectState>(*this);

    // Update

    return
      fromState->pos.ReadDiff(fromState->pos, input) &&
      input->ReadBit(isAlive);
  }
};

struct GameObject
{
  static const int TYPE = 7;

  // Network

  ObjID id;

  // Server: current state
  // Client; last state received from server

  vec2 pos;
  bool isAlive;

  struct Client
  {
    // Previous state (used for extrapolation)

    float prevPos;

    // Predicted state (the most likely to be true; used for local collision prediction etc.)

    vec2 predPos;

    // Visual state (displayed to the user; contains faked smoothing for visual purposes)

    vec2 visPos;

  } client;

  int GetType() { return TYPE; }

  bool Srv_Init(MP* mp)
  {
    id = MP_Srv_GenerateObjectID(mp);
    return id != MP_INVALID_OBJ_ID;
  }

  bool Srv_IsRelevantToClient(GameClient* client) { return true; }

  void Srv_StoreState(MP* mp)
  {
    NetGameObjectState* state = new NetGameObjectState();
    state->type = GetType();
    state->pos = pos;
    state->isAlive = isAlive;

    MP_Srv_StoreState(mp, id, state);
  }

  bool Cl_Init(ObjID id, NetGameObjectState* state)
  {
    id = id;

    isAlive = state->isAlive;
    client.prevPos = pos = client.predPos = client.visPos = state->pos;

    return true;
  }

  bool Cl_Update(NetGameObjectState* state)
  {
    // Store previous state

    client.prevPos = pos;

    // Set new state

    isAlive = state->isAlive;
    pos = state->pos;

    return true;
  }

  void Cl_Predict(f32 timeExtrapolationFactor)
  {
    client.predPos = lerp(client.prevPos, pos, timeExtrapolationFactor);
  }

  void Cl_Update(float dt)
  {
    const f32 visFollowSpeed = 1.0f;
    client.visPos += (client.predPos - client.visPos) * clamp(dt * visFollowSpeed, 0, 1);
  }

  void Render()
  {
    // TODO: Draw at visPos
  }
};

std::vector<GameObject*> objects;

void DestroyObjectState(void* state)
{
  delete (NetGameObjectState*) state;
}

// SERVER
// ==========

void Srv_DetermineRelevantObjectsForClient(GameClient* client, int maxRelevantObjects, int& numRelevantObjects, ObjID* relevantObjects)
{
  numRelevantObjects = 0;

  for (int i = 0; i < objects.size(); i++)
    if (objects[i]->Srv_IsRelevantToClient(client))
    {
      relevantObjects[numRelevantObjects++] = objects[i]->id;
      if (numRelevantObjects == maxRelevantObjects)
        break;
    }
}

template <typename TYPE>
void Srv_WriteValueDiff(const TYPE& fromValue, const TYPE& toValue, Buffer* output)
{
  if (fromValue == toValue)
    output->WriteBit(0);
  else
  {
    output->WriteBit(1);
    output->Write<TYPE>(toValue);
  }
}

void Srv_WriteStateDiff(Node* dstClient, void* fromStateData, void* toStateData, Buffer* output)
{
  NetGameObjectState* toState = (NetGameObjectState*) toStateData;
  toState->Srv_WriteStateDiff((NetGameObjectState*) fromStateData, output);
}

void Srv_UpdateGameplay()
{
  // Create object

  GameObject* obj = new GameObject();
  if (!obj->Srv_Init())
  {
    delete obj;
    return;
  }
  objects.push_back(obj);

  // Delete object

  objects.remove(obj);
  delete obj;
}

void Srv_HandleHostMigration()
{
  // Send migration info to new server and all clients

  for (int i = 0; i < MP_GetNumNodes(mp); i++)
  {
    Node* client = MP_GetNode(mp, i);
    if (MP_IsLocal(client))
      continue;

    // Already sent? - skip

    GameClient* gameClient = (GameClient*) MP_GetNodeData(client);
    if (gameClient->m_migrationInfoSent)
      continue;

    // Generate migration info

    if (client == migrationInfo.m_newServer)
      MP_Srv_GenerateMigrationInfoForNewServer(client, buffer);
    else
      MP_Srv_GenerateMigrationInfoForClient(client, migrationInfo.m_newServer, buffer);

    // Send

    gameClient->m_migrationInfoSent = gameClient->connection->SendReliable(&buffer);
  }

  // Stop migration process after some time

  if (TimSince(migrationInfo.m_migrationStartTime) > SRV_MIGRATION_TIME)
    Srv_ResetGame();
}

void Srv_Update(float dt)
{
  // TODO: Receive acks from clients; call MP_Srv_ReceiveAckFromClient for each

  // Handle host migration

  if (migrationInfo.m_isMigrating)
  {
    Srv_HandleHostMigration();
    return;
  }

  // Update gameplay by the server

  Srv_UpdateGameplay(dt);

  // Progress server time

  MP_Srv_ProgressTime(mp, dt);

  // Create next server frame

  if (MP_Srv_TimeSinceLastFrame(mp) >= SRV_FRAME_FREQUENCY)
  {
    MP_Srv_BeginFrame(mp);
    for (int i = 0; i < objects.size(); i++)
      objects[i]->Srv_StoreState(mp);
    MP_Srv_EndFrame(mp);
  }

  // Send world updates to client

  Srv_SendUpdatesToClients();
}

void Srv_SendUpdatesToClients()
{
  Buffer buffer;

  int numRelevantObjects;
  ObjID relevantObjects[MP_MAX_RELEVANT_OBJECTS];

  for (int i = 0; i < MP_GetNumNodes(mp); i++)
  {
    Node* client = MP_GetNode(mp, i);
    if (MP_IsLocal(client))
      continue;

    // Check bandwidth

    GameClient* gameClient = (GameClient*) MP_GetNodeData(client);
    if (!gameClient->connection->HasAvailableBandwidth())
      continue;

    // Determine objects relevant to client

    Srv_DetermineRelevantObjectsForClient(client, MP_MAX_RELEVANT_OBJECTS, numRelevantObjects, relevantObjects);

    // Generate update

    MP_Srv_GenerateUpdateForClient(client, numRelevantObjects, relevantObjects, buffer);

    // Send

    gameClient->connection->SendUnreliable(&buffer);
  }
}

// CLIENT
// ==========

template <typename TYPE>
bool Cl_ReadValueDiff(const TYPE& fromValue, TYPE& toValue, Buffer* input)
{
  // Check for value change

  bool hasChanged;
  if (!input->ReadBit(hasChanged))
    return false;

  // Copy from old value if no change

  if (!hasChanged)
  {
    toValue = fromValue;
    return true;
  }

  // Read new value

  return input->Read<TYPE>(toValue);
}

void* Cl_CreateStateFromDiffFunc(void* fromStateData, Buffer* input)
{
  NetGameObjectState* toState = new NetGameObjectState();
  if (!toState->Cl_ReadStateDiff((NetGameObjectState*) fromStateData, input))
  {
    delete toState;
    return NULL;
  }
  return toState;
}

void* Cl_CreateObject(ObjID id, void* stateData)
{
  NetGameObjectState* state = (NetGameObjectState*) stateData;

  if (state->type == GameObject::TYPE)
  {
    GameObject* obj = new GameObject();
    if (!obj->Cl_Init(id, state))
    {
      delete obj;
      return NULL;
    }
    objects.push_back(obj);
    return obj;
  }

  return NULL;
}

bool Cl_UpdateObject(ObjID id, void* objectData, void* stateData)
{
  GameObject* obj = (GameObject*) objectData;
  NetGameObjectState* state = (NetGameObjectState*) stateData;
  return obj->Cl_Update(state);
}

void Cl_DestroyObject(ObjID id, void* objectData)
{
  GameObject* obj = (GameObject*) objectData;
  objects.remove(obj);
  delete obj;
}

void Cl_Update(f32 dt)
{
  // TODO: Receive updates from server:
  //  * call MP_Cl_ReceiveUpdateFromServer for regular updates
  //  * call MP_Cl_ReceiveMigrationInfoFromOldServer for when host migration occurs

  // Send ack to server

  Node* server = MP_GetServer(mp);
  GameClient* gameServer = (GameClient*) MP_GetNodeData(server);

  if (gameServer->connection->HasAvailableBandwidth())
  {
    Buffer output;
    MP_Cl_GenerateAckForServer(mp, &output);

    gameServer->connection->SendUnreliable(&output);
  }

  // Client side prediction

  Timestamp t0, t1, t2;
  MP_Cl_GetTimestamps(mp, t0, t1, t2);
  const f32 timeExtrapolationFactor = MP_SecsBetween(t0, t2) / MP_SecsBetween(t0, t1);

  for (int i = 0; i < objects.size(); i++)
    objects[i]->Cl_Predict(timeExtrapolationFactor);

  for (int i = 0; i < objects.size(); i++)
    objects[i]->Cl_Update(dt);
}

///////////////////////////

void Update(f32 dt)
{
  if (MP_IsServer(mp))
    Srv_Update(dt);

  Cl_Update(dt);
}