#ifndef MP_LIB_H
#define MP_LIB_H

/**
 *	@defgroup mp Multiplayer
 *	@brief Low level multiplayer library with support for hosting, joining and searching (LAN only) network game sessions.
 */

#include "Base/ueProperty.h"

/**
 *	@addtogroup mp
 *	@{
 */

// Macros

#define MP_DECL_BASE_STRUCT(type) \
	private: \
		u32 m_libType; \
	public: \
		u32 GetLibType() const { return m_libType; } \
		type(u32 libType) : m_libType(libType)

#define MP_DECL_BASE_STRUCT_DEF(type) \
	MP_DECL_BASE_STRUCT(type) {}

// Types

class mpNode;
class mpPlayer;
class mpQuery;
class mpCallbacks;

//! Unique player id; unique across whole game session
typedef u16 mpPlayerId;

//! Data send flags
enum mpSendFlags
{
	mpSendFlags_Reliable	= UE_POW2(0),	//!< Reliable delivery
	mpSendFlags_Ordered		= UE_POW2(1),	//!< Strictly ordered delivery
	mpSendFlags_Commit		= UE_POW2(2)	//!< Commits sending of that message (and all other queued messages)
};

//! Network states
enum mpState
{
	mpState_Idle = 0,		//!< Nothing going on
	mpState_Hosting,		//!< Host only: started hosting process but not yet finalized
	mpState_Joining,		//!< Client only: started joining process but not yet finalized
	mpState_Lobby,			//!< Lobby state; players and nodes may leave and join
	mpState_StartingGame,	//!< Starting game but not yet finalized
	mpState_Game,			//!< The actual game
	mpState_EndingGame,		//!< Ending game but not yet finalized
	mpState_Leaving,		//!< Leaving game but not yet finalized

	mpState_MAX
};

//! Returns user friendly state name
const char* mpState_ToString(mpState state);

//! Player information
struct mpPlayerInfo
{
	MP_DECL_BASE_STRUCT_DEF(mpPlayerInfo)
};

//! Node information
struct mpNodeInfo
{
	MP_DECL_BASE_STRUCT_DEF(mpNodeInfo)
};

//! Host parameters
struct mpHostParams
{
	MP_DECL_BASE_STRUCT_DEF(mpHostParams)
};

//! Join parameters
struct mpJoinParams
{
	MP_DECL_BASE_STRUCT_DEF(mpJoinParams)
};

//! Leave parameters
struct mpLeaveParams
{
	MP_DECL_BASE_STRUCT_DEF(mpLeaveParams)
};

//! Query parameters
struct mpQueryParams
{
	MP_DECL_BASE_STRUCT_DEF(mpQueryParams)
};

//! Result of the query
struct mpQueryResult
{
	MP_DECL_BASE_STRUCT_DEF(mpQueryResult)
};

//! Data to be sent
struct mpSendData
{
	MP_DECL_BASE_STRUCT(mpSendData)
		,m_buffer(NULL),
		m_size(0),
		m_sendFlags(mpSendFlags_Reliable | mpSendFlags_Ordered)
	{}

	void* m_buffer;		//!< Data to be sent
	u32 m_size;			//!< Size of the data to be sent
	u32 m_sendFlags;	//!< Send flags (see mpSendFlags)
};

//! Received data
struct mpRecvData
{
	MP_DECL_BASE_STRUCT(mpRecvData)
		,m_srcNode(NULL),
		m_buffer(NULL),
		m_size(0)
	{}

	mpNode* m_srcNode;	//!< Node that sent this data
	void* m_buffer;		//!< Received data
	u32 m_size;			//!< Received data size
};

//! Session information
struct mpSessionInfo
{
	MP_DECL_BASE_STRUCT_DEF(mpSessionInfo)
};

//! mpLib creation parameters
struct mpCreateParams
{
	MP_DECL_BASE_STRUCT(mpCreateParams)
		,m_allocator(NULL),
		m_callbacks(NULL)
	{}

	ueAllocator* m_allocator;		//!< Allocator to be used for mpLib
	mpCallbacks* m_callbacks;		//!< User supplied callbacks
};

/**
 *	Multiplayer library callbacks for misc. network events.
 */
class mpCallbacks
{
public:
	virtual ~mpCallbacks() {}
	//! Invoked when new node connects to game
	virtual void OnNodeConnected(mpNode* node) = 0;
	//! Invoked when node disconnects
	virtual void OnNodeDisconnected(mpNode* node) = 0;
	//! Invoked when new player is added to game
	virtual void OnPlayerAdded(mpPlayer* player) = 0;
	//! Invoked when player is removed from game
	virtual void OnPlayerRemoved(mpPlayer* player) = 0;
	//! Invoked when data is received
	virtual void OnRecvData(mpRecvData* data) = 0;
	//! Invoked when query is finished
	virtual void OnQueryDone(mpQuery* query) = 0;
	//! Invoked when any player's readiness state changes
	virtual void OnPlayerReady(mpPlayer* player, ueBool ready) = 0;
	//! Invoked on each transition between states
	virtual void OnStateChange(mpState oldState, mpState newState) = 0;
	//! Invoked every time any session property changes
	virtual void OnPropertyChange(ueProperty* oldProperty, ueProperty* newProperty) = 0;
};

// Interface

/**
 *	Multiplayer library player representation.
 *	Each player belongs to some node (@see mpNode).
 */
class mpPlayer
{
protected:
	virtual ~mpPlayer() {}
public:
	//! Gets unique (per session) player id
	virtual mpPlayerId GetId() = 0;
	//! Gets player information
	virtual void GetInfo(mpPlayerInfo* info) = 0;
	//! Gets owning node
	virtual mpNode* GetNode() = 0;
	//! Gets whether player is ready
	virtual ueBool IsReady() = 0;
	//! Sets player's state to ready; for remote players can only be invoked by the host
	virtual void SetReady(ueBool ready) = 0;
	//! Tells whether this player is a host player
	virtual ueBool IsHost() = 0;
	//! Tells whether this player is local
	virtual ueBool IsLocal() = 0;
	//! Gets debug player name (to be used for debugging only)
	virtual const char* GetDebugName() = 0;
	//! Sets user data
	virtual void SetUserData(void* userData) = 0;
	//! Gets user data
	virtual void* GetUserData() = 0;
};

/**
 *	Multiplayer library node representation.
 *	Node contains players.
 */
class mpNode
{
protected:
	virtual ~mpNode() {}
public:
	//! Sends data to this node; can only be invoked for remote nodes
	virtual ueBool SendTo(mpSendData* sendData) = 0;
	//! Commits sending data to this node
	virtual void CommitSendTo() = 0;
	//! Gets node information
	virtual void GetInfo(mpNodeInfo* info) = 0;
	//! Gets number of players on this node
	virtual u32 GetNumPlayers() = 0;
	//! Gets player by index (from 0 to GetNumPlayers() - 1)
	virtual mpPlayer* GetPlayer(u32 index) = 0;
	//! Gets player by unique player id
	virtual mpPlayer* GetPlayerById(mpPlayerId id) = 0;
	//! Tells whether node is a host
	virtual ueBool IsHost() = 0;
	//! Tells whether node is local
	virtual ueBool IsLocal() = 0;
	//! Gets node debug name (to be used for debugging only)
	virtual const char* GetDebugName() = 0;
	//! Sets user data
	virtual void SetUserData(void* userData) = 0;
	//! Gets user data
	virtual void* GetUserData() = 0;
};

//! Query
class mpQuery
{
protected:
	virtual ~mpQuery() {}
public:
	//! Indicates whether query has finished
	virtual ueBool IsDone() = 0;
	//! Destroys query (cancels work-in-progress query)
	virtual void Destroy() = 0;
	//! Gets query result
	virtual void GetResult(mpQueryResult* result) = 0;
	//! Sets user data
	virtual void SetUserData(void* userData) = 0;
	//! Gets user data
	virtual void* GetUserData() = 0;
};

/**
 *	Main network library interface.
 */
class mpLib
{
protected:
	virtual ~mpLib() {}
public:
	//! Shuts down library
	virtual void Destroy() = 0;
	//! Creates query
	virtual mpQuery* CreateQuery(mpQueryParams* params) = 0;
	//! Starts hosting
	virtual ueBool Host(mpHostParams* params) = 0;
	//! Starts the game
	virtual void StartGame() = 0;
	//! Ends the game
	virtual void EndGame() = 0;
	//! Tells whether everyone is ready
	virtual ueBool IsEveryoneReady() = 0;
	//! Drops (kicks out) the node
	virtual void DropNode(mpNode* node) = 0;
	//! Starts joining
	virtual ueBool Join(mpJoinParams* params) = 0;
	//! Leaves session
	virtual void Leave(mpLeaveParams* params) = 0;
	//! Updates library (should be called regularly to keep connections active)
	virtual void Update() = 0;
	//! Gets session information
	virtual void GetSessionInfo(mpSessionInfo* info) = 0;
	//! Gets library state
	virtual mpState GetState() = 0;
	//! Gets library symbol
	virtual u32 GetLibSymbol() = 0;
	//! Gets number of nodes in game
	virtual u32 GetNumNodes() = 0;
	//! Gets node by index (from 0 to GetNumNodes() - 1)
	virtual mpNode* GetNode(u32 index) = 0;
	//! Gets number of players in game
	virtual u32 GetNumPlayers() = 0;
	//! Gets player by index (from 0 to GetNumPlayers() - 1)
	virtual mpPlayer* GetPlayer(u32 index) = 0;
	//! Gets player by unique id
	virtual mpPlayer* GetPlayerById(mpPlayerId id) = 0;
	//! Tells whether local node is host
	virtual ueBool IsHost() = 0;
	//! Gets local node
	virtual mpNode* GetLocalNode() = 0;
	//! Gets host node
	virtual mpNode* GetHostNode() = 0;
	//! Gets host player
	virtual mpPlayer* GetHostPlayer() = 0;
	//! Sets session property
	virtual void SetProperty(ueProperty* prop) = 0;
	//! Gets session property
	virtual ueBool GetProperty(ueProperty* prop) = 0;
};

#if defined(MP_USE_ENET)
	#include "Net/ENet/mpLib_ENet.h"
#endif

// @}

#endif // MP_LIB_H