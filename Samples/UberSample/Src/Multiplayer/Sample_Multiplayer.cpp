#include "Graphics/glLib.h"
#include "Input/inSys.h"
#include "Base/ueRand.h"
#include "Base/Containers/ueGenericPool.h"
#include "IO/ueBitBuffer.h"
#include "GraphicsExt/gxTextRenderer.h"
#include "GraphicsExt/gxFont.h"
#include "GraphicsExt/gxShapeDraw.h"
#include "Multiplayer/mpLib.h"
#include "SampleApp.h"

#define EXPIRING_MSG_TIME 3.0f

#define MIN_POS 0.0f
#define MAX_POS 1.0f
#define NUM_POS_COMPRESSION_BITS 10

/**
 *	Demonstrates multiplayer matchmaking including:
 *	- searching for available games in local network (LAN)
 *	- hosting & joining games
 *	- changing player's "ready" state (on / off)
 *	- starting & ending game (by the host)
 *	- sending & receiving compressed messages between nodes
 *	- simple "move the box" multiplayer game
 */
class ueSample_Multiplayer : public ueSample, mpCallbacks
{
public:
	ueBool Init()
	{
		// Initialize mpLib

		mpCreateParams_ENet createParams;
		createParams.m_allocator = g_app.GetFreqAllocator();
		createParams.m_broadcastPort = 1345;
		createParams.m_callbacks = this;
		createParams.m_titleId = 'chat';
		m_mpLib = mpLib_ENet_Create(&createParams);
		UE_ASSERT_FUNC(m_mpLib);

		m_mpQuery = NULL;

		m_msg[0] = 0;
		m_expiringMsg[0] = 0;

		m_playerDataPool.Init(g_app.GetStackAllocator(), sizeof(PlayerData), MP_ENET_MAX_PLAYERS);

		m_lastSendTime = 0;

		return UE_TRUE;
	}

	void Deinit()
	{
		m_mpLib->Destroy();
		m_mpLib = NULL;
	}

	void DoFrame(f32 dt)
	{
		Update(dt);
		Draw();
	}

	void Update(f32 dt)
	{
		if (m_expiringMsgTimeLeft > 0)
			m_expiringMsgTimeLeft -= dt;

		if (m_mpQuery)
		{
			if (!m_mpQuery->IsDone())
			{
				ueStrCpyS(m_msg, "Please wait - searching for games...\n<X> - stop searching");
				if (inKeyboard_WasPressed(inKey_X))
				{
					m_mpQuery->Destroy();
					m_mpQuery = NULL;
				}
			}
			else
			{
				mpQueryResult_ENet result;
				m_mpQuery->GetResult(&result);
				if (result.m_numSessions == 0)
				{
					ueStrCpyS(m_msg, "No games found\n<B> - go back");
					if (inKeyboard_WasPressed(inKey_B))
					{
						m_mpQuery->Destroy();
						m_mpQuery = NULL;
					}
				}
				else
				{
					ueStrFormatS(m_msg, "Found %u game(s)\n<1> - join first\n<2> - join second (if available)\n<3> - join third (if available)\n\n", result.m_numSessions);

					u32 msgLen = ueStrLen(m_msg);
					for (u32 i = 0; i < result.m_numSessions; i++)
					{
						const mpQueryResult_ENet::Session& session = result.m_sessions[i];
						ueStrFormat(m_msg + msgLen, UE_ARRAY_SIZE(m_msg) - msgLen, "  [%u] %s:%u", i, session.m_ip.ToString(), (u32) session.m_port);
						msgLen += ueStrLen(m_msg + msgLen);
					}

					u32 indexToJoin = U32_MAX;
					if (inKeyboard_WasPressed(inKey_1)) indexToJoin = 0;
					else if (inKeyboard_WasPressed(inKey_2)) indexToJoin = 1;
					else if (inKeyboard_WasPressed(inKey_3)) indexToJoin = 2;
					if (indexToJoin != U32_MAX && indexToJoin < result.m_numSessions)
					{
						mpQueryResult_ENet::Session* session = &result.m_sessions[indexToJoin];

						mpJoinParams_ENet joinParams;
						joinParams.m_localIp.sn_addr = 0;
						joinParams.m_localPort = 0; // Any port
						joinParams.m_hostIp = session->m_ip;
						joinParams.m_hostPort = session->m_port;

						if (!m_mpLib->Join(&joinParams))
						{
							ueStrCpyS(m_expiringMsg, "Failed to join game.");
							m_expiringMsgTimeLeft = EXPIRING_MSG_TIME;
						}

						m_mpQuery->Destroy();
						m_mpQuery = NULL;
					}
				}
			}
		}
		else switch (m_mpLib->GetState())
		{
			case mpState_Idle:
			{
				ueStrCpyS(m_msg, "Idle state\n<H> - host game\n<S> - search for games");
				if (inKeyboard_WasPressed(inKey_H))
				{
					ueProperty props[1];
					props[0].m_id = 'diff';
					props[0].m_value.m_type = ueValueType_S32;
					props[0].m_value.m_s32 = 'easy';

					mpHostParams_ENet hostParams;
					hostParams.m_ip.sn_addr = NS_IP_ANY;
					hostParams.m_port = 0;
					hostParams.m_enableLANBroadcast = UE_TRUE;
					hostParams.m_maxNodes = 32;
					hostParams.m_maxPlayers = 32;
					hostParams.m_props = props;
					hostParams.m_numProps = UE_ARRAY_SIZE(props);

					if (!m_mpLib->Host(&hostParams))
					{
						ueStrCpyS(m_expiringMsg, "Failed to host game");
						m_expiringMsgTimeLeft = EXPIRING_MSG_TIME;
					}
				}
				else if (inKeyboard_WasPressed(inKey_S))
				{
					ueProperty props[1];
					props[0].m_id = 'diff';
					props[0].m_value.m_type = ueValueType_S32;
					props[0].m_value.m_s32 = 'easy';

					mpQueryParams_ENet queryParams;
					queryParams.m_LAN = UE_TRUE;
					queryParams.m_maxResults = 10;
					queryParams.m_props = props;
					queryParams.m_numProps = UE_ARRAY_SIZE(props);

					m_mpQuery = m_mpLib->CreateQuery(&queryParams);
					if (!m_mpQuery)
					{
						ueStrCpyS(m_expiringMsg, "Failed to search for games");
						m_expiringMsgTimeLeft = EXPIRING_MSG_TIME;
					}
				}
				break;
			}

			case mpState_Hosting:
				ueStrCpyS(m_msg, "Hosting in progress...");
				break;

			case mpState_Joining:
				ueStrCpyS(m_msg, "Joining in progress...");
				break;

			case mpState_Lobby:
				if (m_mpLib->IsHost())
				{
					ueStrCpyS(m_msg, "Lobby\n<S> - start game\n<R> - ready on / off\n<L> - leave game");
					if (inKeyboard_WasPressed(inKey_S))
						m_mpLib->StartGame();
				}
				else
					ueStrCpyS(m_msg, "Lobby\n<R> - ready on / off\n<L> - leave game");
				if (inKeyboard_WasPressed(inKey_R))
				{
					mpPlayer* player = m_mpLib->GetLocalNode()->GetPlayer(0);
					player->SetReady( !player->IsReady() );
				}
				else if (inKeyboard_WasPressed(inKey_L))
				{
					mpLeaveParams_ENet leaveParams;
					m_mpLib->Leave(&leaveParams);
				}
				break;

			case mpState_StartingGame:
				ueStrCpyS(m_msg, "Starting game...");
				break;

			case mpState_Game:
			{
				if (m_mpLib->IsHost())
				{
					ueStrCpyS(m_msg, "Game in progress\n<S> - end game\n<L> - leave game");
					if (inKeyboard_WasPressed(inKey_S))
						m_mpLib->EndGame();
				}
				else
					ueStrCpyS(m_msg, "Game in progress\n<L> - leave game");
				if (inKeyboard_WasPressed(inKey_L))
				{
					mpLeaveParams_ENet leaveParams;
					m_mpLib->Leave(&leaveParams);
				}
				else
				{
					IO_BIT_BUFFER_DECL(buffer, 512, ueBitBuffer::Mode_Write);
					const f32 speed = 0.1f;

					// Game-play update

					mpNode* localNode = m_mpLib->GetLocalNode();
					buffer.WriteS32RangeCompressed(localNode->GetNumPlayers(), 1, MP_ENET_MAX_PLAYERS_PER_NODE);

					for (u32 i = 0; i < localNode->GetNumPlayers(); i++)
					{
						mpPlayer* player = localNode->GetPlayer(i);
						PlayerData* pd = (PlayerData*) player->GetUserData();

						if (inKeyboard_IsDown(inKey_Left))
							pd->x -= dt * speed;
						else if (inKeyboard_IsDown(inKey_Right))
							pd->x += dt * speed;
						else if (inKeyboard_IsDown(inKey_Up))
							pd->y -= dt * speed;
						else if (inKeyboard_IsDown(inKey_Down))
							pd->y += dt * speed;

						buffer.WriteAny<mpPlayerId>(player->GetId());
						buffer.WriteF32Compressed(pd->x, MIN_POS, MAX_POS, NUM_POS_COMPRESSION_BITS);
						buffer.WriteF32Compressed(pd->y, MIN_POS, MAX_POS, NUM_POS_COMPRESSION_BITS);
						buffer.WriteAny<ueColor32>(pd->m_color);
					}

					// Send (unreliable) update to everyone

					if (ueClock_GetSecsSince(m_lastSendTime) > 0.05f)
					{
						mpSendData_ENet sendData;
						sendData.m_buffer = buffer.GetData();
						sendData.m_size = buffer.GetSizeInBytes();
						sendData.m_sendFlags = mpSendFlags_Ordered | mpSendFlags_Commit;

						for (u32 i = 0; i < m_mpLib->GetNumNodes(); i++)
						{
							mpNode* node = m_mpLib->GetNode(i);
							if (node != localNode)
								node->SendTo(&sendData);
						}

						m_lastSendTime = ueClock_GetCurrent();
					}
				}
				break;
			}
			case mpState_EndingGame:
				ueStrCpyS(m_msg, "Ending game...");
				break;

			case mpState_Leaving:
				ueStrCpyS(m_msg, "Leaving game...");
				break;
		}

		m_mpLib->Update();
	}

	void Draw()
	{
		glCtx* ctx = g_app.BeginDrawing();
		if (!ctx)
			return;
		DrawScene(ctx);
		g_app.DrawAppOverlay();
		g_app.EndDrawing();
	}

	void DrawScene(glCtx* ctx)
	{
		const gxFont* font = gxTextRenderer_GetDefaultFont();
		UE_ASSERT(font);

		gxText text;
		text.m_x = 100.0f;

		// Draw expiring message

		if (m_expiringMsgTimeLeft > 0)
		{
			text.m_color = ueColor32::Red;
			text.m_y = 50.0f;
			text.m_utf8Buffer = m_expiringMsg;
			gxTextRenderer_Draw(ctx, &text);
		}

		// Draw main message

		text.m_color = ueColor32::Green;
		text.m_y = 100.0f;
		text.m_utf8Buffer = m_msg;
		gxTextRenderer_Draw(ctx, &text);

		// Show player list

		if (m_mpLib->GetState() == mpState_Lobby ||
			m_mpLib->GetState() == mpState_StartingGame ||
			m_mpLib->GetState() == mpState_Game ||
			m_mpLib->GetState() == mpState_EndingGame)
		{
			char buffer[512];
			text.m_utf8Buffer = buffer;
			text.m_color = ueColor32::Yellow;

			for (u32 i = 0; i < m_mpLib->GetNumPlayers(); i++)
			{
				mpPlayer* player = m_mpLib->GetPlayer(i);
				mpNode* node = player->GetNode();

				mpNodeInfo_ENet nodeInfo;
				node->GetInfo(&nodeInfo);

				ueStrFormatS(buffer, " [%u] %s %s (%s:%u)", i,
					player->IsReady() ? "READY" : "NOT READY",
					player->GetDebugName(),
					nodeInfo.m_ip.ToString(), (u32) nodeInfo.m_port);
				text.m_y = 250.0f + gxFont_GetSizeInPixels(font) * i;
				gxTextRenderer_Draw(ctx, &text);
			}
		}

		// Draw players

		if (m_mpLib->GetState() == mpState_Game)
		{
			gxShapeDrawParams shapeDrawParams;
			shapeDrawParams.m_2DCanvas.Set(0.0f, 0.0f, 1.0f, 1.0f);
			gxShapeDraw_SetDrawParams(&shapeDrawParams);
			gxShapeDraw_Begin(ctx);
			{
				for (u32 i = 0; i < m_mpLib->GetNumPlayers(); i++)
				{
					mpPlayer* player = m_mpLib->GetPlayer(i);
					PlayerData* pd = (PlayerData*) player->GetUserData();

					gxShape_Rect rect;
					rect.m_wireFrame = UE_FALSE;
					rect.m_color = pd->m_color;
					const f32 pointHalfSize = 0.02f;
					rect.m_rect.Set(pd->x - pointHalfSize, pd->y - pointHalfSize, pd->x + pointHalfSize, pd->y + pointHalfSize);

					gxShapeDraw_DrawRect(rect);
				}
			}
			gxShapeDraw_End();
		}
	}

	// mpCallbacks implementation

	void OnNodeConnected(mpNode* node)
	{
		ueLogD("%s %s node %s connected", node->IsLocal() ? "Local" : "Remote", node->IsHost() ? "host" : "client", node->GetDebugName());
	}

	void OnNodeDisconnected(mpNode* node)
	{
		ueLogD("%s %s node %s disconnected", node->IsLocal() ? "Local" : "Remote", node->IsHost() ? "host" : "client", node->GetDebugName());
	}

	void OnPlayerAdded(mpPlayer* player)
	{
		ueLogD("Player (id: %d) added to node %s", player->GetId(), player->GetNode()->GetDebugName());

		PlayerData* pd = new(m_playerDataPool) PlayerData();
		if (player->IsLocal())
		{
			ueRand* rand = ueRand_GetGlobal();

			pd->m_color.r = 100 + (u8) ueRand_U32(rand, 155);
			pd->m_color.g = 100 + (u8) ueRand_U32(rand, 155);
			pd->m_color.b = 100 + (u8) ueRand_U32(rand, 155);
		}
		else
			pd->m_color = ueColor32::White; // Default color
		pd->x = pd->y = 0.5f;

		player->SetUserData(pd);
	}

	void OnPlayerRemoved(mpPlayer* player)
	{
		ueLogD("Player (id: %d) removed from node %s", player->GetId(), player->GetNode()->GetDebugName());

		PlayerData* pd = (PlayerData*) player->GetUserData();
		ueDelete(pd, m_playerDataPool);
	}

	void OnPlayerReady(mpPlayer* player, ueBool ready)
	{
		ueLogD("Player (id: %d) became %s", player->GetId(), ready ? "READY" : "UNREADY");
	}

	void OnRecvData(mpRecvData* data)
	{
	//	ueLogD("RECV from %s: %s", data->m_srcNode->GetDebugName(), data->m_buffer);

		ueBitBuffer buffer(data->m_buffer, data->m_size, ueBitBuffer::Mode_Read);

		s32 numPlayers;
		if (!buffer.ReadS32RangeCompressed(numPlayers, 1, MP_ENET_MAX_PLAYERS_PER_NODE))
			return; // Corrupted data

		for (s32 i = 0; i < numPlayers; i++)
		{
			mpPlayerId playerId;
			f32 x, y;
			ueColor32 color;
			if (!buffer.ReadAny<mpPlayerId>(playerId) ||
				!buffer.ReadF32Compressed(x, MIN_POS, MAX_POS, NUM_POS_COMPRESSION_BITS) ||
				!buffer.ReadF32Compressed(y, MIN_POS, MAX_POS, NUM_POS_COMPRESSION_BITS) ||
				!buffer.ReadAny<ueColor32>(color))
				return; // Corrupted data

			mpPlayer* player = data->m_srcNode->GetPlayerById(playerId);
			if (!player)
				continue; // Player may have disconnected

			PlayerData* pd = (PlayerData*) player->GetUserData();
			pd->x = x;
			pd->y = y;
			pd->m_color = color;
		}
	}

	void OnQueryDone(mpQuery* query)
	{
		mpQueryResult_ENet result;
		query->GetResult(&result);
		ueLogD("Query done (%u results):", result.m_numSessions);
		for (u32 i = 0; i < result.m_numSessions; i++)
		{
			mpQueryResult_ENet::Session* session = &result.m_sessions[i];

			ueLogD("  [%u] ip: %s:%u, players: %u / %u, nodes: %u / %u",
				i,
				session->m_ip.ToString(), (u32) session->m_port,
				session->m_numPlayers, session->m_maxPlayers,
				session->m_numNodes, session->m_maxNodes);
		}
	}

	void OnStateChange(mpState oldState, mpState newState)
	{
		ueLogD("Changed state from %s to %s", mpState_ToString(oldState), mpState_ToString(newState));
	}

	void OnPropertyChange(ueProperty* oldProperty, ueProperty* newProperty)
	{
		if (!oldProperty)
			ueLogD("Property %u was added", newProperty->m_id);
		else if (!newProperty)
			ueLogD("Property %u was removed", oldProperty->m_id);
		else
			ueLogD("Property %u changed", oldProperty->m_id);
	}

	char m_msg[512];

	f32 m_expiringMsgTimeLeft;
	char m_expiringMsg[512];

	struct PlayerData
	{
		f32 x, y;
		ueColor32 m_color;
	};
	ueGenericPool m_playerDataPool;

	ueTime m_lastSendTime;

	mpLib* m_mpLib;
	mpQuery* m_mpQuery;
};

UE_DECLARE_SAMPLE(ueSample_Multiplayer, "Multiplayer")
