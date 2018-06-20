#pragma once

#include "Net/nwSocket.h"
#include "Net/mpLib.h"

// mpLib_ENet specific

#define MP_ENET_SYMBOL 0xEE77EE11

#define MP_ENET_MAX_NODES				32
#define MP_ENET_MAX_PLAYERS				32
#define MP_ENET_MAX_PLAYERS_PER_NODE	4
#define MP_ENET_MAX_QUERIES				2
#define MP_ENET_MAX_QUERY_RESULTS		16

// Specialization of the structs

struct mpPlayerInfo_ENet : mpPlayerInfo
{
	mpPlayerInfo_ENet() :
		mpPlayerInfo(MP_ENET_SYMBOL)
	{}
};

struct mpNodeInfo_ENet : mpNodeInfo
{
	mpNodeInfo_ENet() :
		mpNodeInfo(MP_ENET_SYMBOL)
	{}

	nsIP m_ip;		//!< Node ip address
	u16 m_port;		//!< Node port
};

struct mpHostParams_ENet : mpHostParams
{
	mpHostParams_ENet() :
		mpHostParams(MP_ENET_SYMBOL),
		m_enableLANBroadcast(UE_TRUE),
		m_maxNodes(8),
		m_maxPlayers(8),
		m_maxIncomingBandwidth(57600 / 8),	// 56K modem with 56 Kbps downstream bandwidth
        m_maxOutgoingBandwidth(14400 / 8),	// 56K modem with 14 Kbps upstream bandwidth
		m_numProps(0),
		m_props(NULL),
		m_numLocalPlayers(1)
	{}

	nsIP m_ip;						//!< Host ip address
	u16 m_port;						//!< Host port

	ueBool m_enableLANBroadcast;	//!< Indicates whether to start broadcasting on LAN

	u32 m_maxNodes;					//!< Max. allowed number of nodes in session
	u32 m_maxPlayers;				//!< Max. allowed number of players in session
	u32 m_maxIncomingBandwidth;		//!< Max. allowed incoming bandwidth in bytes per second
	u32 m_maxOutgoingBandwidth;		//!< Max. allowed outgoing bandwidth in bytes per second

	u32 m_numProps;					//!< Number of session properties
	ueProperty* m_props;			//!< Session properties

	u32 m_numLocalPlayers;			//!< Number of local players to be created
};

struct mpJoinParams_ENet : mpJoinParams
{
	mpJoinParams_ENet() :
		mpJoinParams(MP_ENET_SYMBOL),
		m_maxIncomingBandwidth(57600 / 8),	// 56K modem with 56 Kbps downstream bandwidth
        m_maxOutgoingBandwidth(14400 / 8),	// 56K modem with 14 Kbps upstream bandwidth
		m_numLocalPlayers(1)
	{}

	nsIP m_hostIp;					//!< Ip of the host to join
	u16 m_hostPort;					//!< Port of the host

	nsIP m_localIp;					//!< Local "enet host" ip
	u16 m_localPort;				//!< Local "enet host" port

	u32 m_maxIncomingBandwidth;		//!< Max. allowed incoming bandwidth in bytes per second
	u32 m_maxOutgoingBandwidth;		//!< Max. allowed outgoing bandwidth in bytes per second

	u32 m_numLocalPlayers;			//!< Number of local players to join session
};

struct mpLeaveParams_ENet : mpLeaveParams
{
	mpLeaveParams_ENet() :
		mpLeaveParams(MP_ENET_SYMBOL)
	{}
};

struct mpQueryParams_ENet : mpQueryParams
{
	mpQueryParams_ENet() :
		mpQueryParams(MP_ENET_SYMBOL),
		m_LAN(UE_TRUE),
		m_maxResults(5),
		m_timeOutSecs(5.0f),
		m_numProps(0),
		m_props(NULL)
	{}

	ueBool m_LAN;				//!< Indicates whether to do LAN session search

	u32 m_maxResults;			//!< Max. number of results
	f32 m_timeOutSecs;			//!< Time out for session search

	u32 m_numProps;				//!< Number of properties to search by
	ueProperty* m_props;		//!< Properties to search by
};

struct mpQueryResult_ENet : mpQueryResult
{
	mpQueryResult_ENet() :
		mpQueryResult(MP_ENET_SYMBOL),
		m_numSessions(0),
		m_sessions(NULL)
	{}

	//! Found session description
	struct Session
	{
		nsIP m_ip;					//!< Host ip
		u16 m_port;					//!< Host port

		u32 m_sessionId;			//!< Session id

		u32 m_numPlayers;			//!< Number of players
		u32 m_maxPlayers;			//!< Max. number of players

		u32 m_numNodes;				//!< Number of nodes
		u32 m_maxNodes;				//!< Max. number of nodes

		u32 m_numProps;				//!< Number of session properties
		ueProperty* m_props;	//!< Session properties
	};

	u32 m_numSessions;		//!< Number of found sessions
	Session* m_sessions;	//!< Found sessions
};

struct mpSendData_ENet : mpSendData
{
	mpSendData_ENet() :
		mpSendData(MP_ENET_SYMBOL)
	{}
};

struct mpRecvData_ENet : mpRecvData
{
	mpRecvData_ENet() :
		mpRecvData(MP_ENET_SYMBOL),
		m_reliable(UE_TRUE)
	{}

	ueBool m_reliable;		//!< Indicates whether received data was sent reliably
};

struct mpSessionInfo_ENet : mpSessionInfo
{
	mpSessionInfo_ENet() :
		mpSessionInfo(MP_ENET_SYMBOL),
		m_sessionId(0),
		m_numProps(0),
		m_props(NULL)
	{}

	u32 m_sessionId;			//!< Session id

	u32 m_numProps;				//!< Number of session properties
	ueProperty* m_props;		//!< Session properties
};

struct mpCreateParams_ENet : mpCreateParams
{
	mpCreateParams_ENet() :
		mpCreateParams(MP_ENET_SYMBOL),
		m_titleId(0),
		m_broadcastPort(3466)
	{}

	u32 m_titleId;			//!< Title id (title id must match for clients and host)
	u32 m_broadcastPort;	//!< Port to be used for host's session info broadcasting
};

//! Creates enet based network library
mpLib* mpLib_ENet_Create(mpCreateParams* params);
