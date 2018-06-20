#include "SampleApp.h"
#include "Input/inSys.h"
#include "Net/nwContentMgr.h"
#include "Base/ueAsync.h"
#include "GraphicsExt/gxShapeDraw.h"
#include "GraphicsExt/gxTextRenderer.h"

/**
 *	Demonstrates online leaderboards (requires connection to the internet).
 */
class ueSample_OnlineLeaderboards : public ueSample
{
public:
	ueBool Init()
	{
		// Startup online content manager

		nsContentMgrStartupParams params;
		params.m_allocator = g_app.GetFreqAllocator();
		params.m_queryTemplateDir = "online_leaderboards/";
		params.m_serverDomain = "www.uberengine.com";
		params.m_serverUrl = "http://www.uberengine.com/test_leaderboards/test_leaderboards.php";

		nsContentMgr_Startup(&params);

		// Create query description

		m_queryParams[0].m_type = ueValueType_S32;
		m_queryParams[0].m_s32 = 0; // First row (will be set when making query)
		m_queryParams[1].m_type = ueValueType_S32;
		m_queryParams[1].m_s32 = UE_ARRAY_SIZE(m_rows); // Max rows

		m_queryDesc.m_template = nsContentMgr_GetQueryTemplate("best_timers");
		UE_ASSERT(m_queryDesc.m_template);
		m_queryDesc.m_numParamValues = UE_ARRAY_SIZE(m_queryParams);
		m_queryDesc.m_paramValues = m_queryParams;
		m_queryDesc.m_timeOutSecs = 10.0f; // Set 0.0f for infinite
		m_queryDesc.m_userData = this;

		// Start query

		m_firstRankIndex = 0;
		m_numRows = 0;
		m_isError = UE_FALSE;
		m_query = NULL;

		StartQuery();

		// Initialize input

		m_inputConsumerId = inSys_RegisterConsumer("online leaderboards sample", 0.0f);
		m_inputEvents.m_startStopQuery = inSys_RegisterEvent(m_inputConsumerId, "start / stop query", &inBinding(inDev_Keyboard, inKey_Space), &inBinding(inDev_Gamepad, inGamepadButton_A));
		m_inputEvents.m_scrollUp = inSys_RegisterEvent(m_inputConsumerId, "scroll up", &inBinding(inDev_Keyboard, inKey_Up), &inBinding(inDev_Gamepad, inGamepadButton_LeftStick_Up));
		m_inputEvents.m_scrollDown = inSys_RegisterEvent(m_inputConsumerId, "scroll down", &inBinding(inDev_Keyboard, inKey_Down), &inBinding(inDev_Gamepad, inGamepadButton_LeftStick_Down));

		return UE_TRUE;
	}

	void Deinit()
	{
		inSys_UnregisterConsumer(m_inputConsumerId);
		nsContentMgr_Shutdown();
	}

	void DoFrame(f32 dt)
	{
		// Handle user input

		if (inSys_WasPressed(m_inputEvents.m_startStopQuery))
		{
			if (m_query)
			{
				ueAsync_Destroy(m_query);
				m_query = NULL;
			}
			else
				StartQuery();
		}
		else if (inSys_WasPressed(m_inputEvents.m_scrollUp))
		{
			if (m_firstRankIndex > 0)
			{
				m_firstRankIndex = m_firstRankIndex - 1;
				StartQuery();
			}
		}
		else if (inSys_WasPressed(m_inputEvents.m_scrollDown))
		{
			m_firstRankIndex = m_firstRankIndex + 1;
			StartQuery();
		}

		// Update content manager

		nsContentMgr_Update();

		// Check query state

		if (m_query)
		{
			const ueAsyncState queryState = ueAsync_GetState(m_query);
			switch (queryState)
			{
				case ueAsyncState_InProgress:
					m_secsWaiting += dt;
					break;

				case ueAsyncState_Failed:

					ueLogE("Query failed");

					ueAsync_Destroy(m_query);
					m_query = NULL;

					m_isError = UE_TRUE;
					m_numRows = 0;
					break;

				case ueAsyncState_Succeeded:
				{
					const nsQueryResult* result = (nsQueryResult*) ueAsync_GetData(m_query);

					ueLogD("Query done (%u rows, %.3f sec)", result->m_numRows, result->m_time);

					m_numRows = ueMin(result->m_numRows, UE_ARRAY_SIZE(m_rows));
					for (u32 i = 0; i < m_numRows; i++)
					{
						if (const ueValue* value = result->GetValue(i, "userName", ueValueType_String))
							ueStrCpyS(m_rows[i].m_userName, value->m_string);
						if (const ueValue* value = result->GetValue(i, "rank", ueValueType_S32))
							m_rows[i].m_rank = value->m_s32;
						if (const ueValue* value = result->GetValue(i, "bestTime", ueValueType_S32))
							m_rows[i].m_bestTime = value->m_s32;
					}

					ueAsync_Destroy(m_query);
					m_query = NULL;

					m_isError = UE_FALSE;
					break;
				}
			}
		}

		// Draw

		glCtx* ctx = g_app.BeginDrawing();
		if (!ctx)
			return;

		char buffer[4096];
		buffer[0] = 0;

		if (m_query)
			ueStrFormatS(buffer, "Please wait - performing query [%.1f secs]...\nPress <space> to cancel refresh.\n\n", m_secsWaiting);
		else if (m_isError)
			ueStrCpyS(buffer, "Failed to get stats (no internet connection?).\nPress <space> to refresh.\n\n");
		else
			ueStrCpyS(buffer, "Press <space> to refresh.\n\n");

		if (!m_isError)
		{
			char leaderboardHeader[64];
			ueStrFormatS(leaderboardHeader, "Leaderboard [%u -> %u%s]:\n", m_firstRankIndex + 1, m_firstRankIndex + m_numRows, m_query ? " ... loading" : "");
			ueStrCatS(buffer, leaderboardHeader);

			if (m_numRows == 0)
				ueStrCatS(buffer, "<no entries>");
			else
			{
				char rowBuffer[128];
				for (u32 i = 0; i < m_numRows; i++)
				{
					ueStrFormatS(rowBuffer, "%4d %5d ms %s\n", m_rows[i].m_rank, m_rows[i].m_bestTime, m_rows[i].m_userName);
					ueStrCatS(buffer, rowBuffer);
				}
			}
		}

		gxText text;
		text.m_x = 150;
		text.m_y = 150;
		text.m_utf8Buffer = buffer;
		gxTextRenderer_Draw(ctx, &text);

		g_app.DrawAppOverlay();
		g_app.EndDrawing();
	}

private:
	void StartQuery()
	{
		// Kill old query

		if (m_query)
		{
			ueAsync_Destroy(m_query);
			m_query = NULL;
		}

		// Start new query

		m_secsWaiting = 0.0f;

		m_queryParams[0].m_s32 = m_firstRankIndex;
		m_query = nsContentMgr_DoQuery(&m_queryDesc);
		if (!m_query)
			m_isError = UE_TRUE;
	}

	ueBool m_isError;
	char m_errorMsg[128];
	f32 m_secsWaiting;

	// Query

	ueAsync* m_query;

	// Query description

	u32 m_firstRankIndex;
	ueValue m_queryParams[2];
	nsQueryDesc m_queryDesc;

	// Currently viewed leaderboard chunk

	struct Row
	{
		char m_userName[64];
		s32 m_rank;
		s32 m_bestTime;
	};

	u32 m_numRows;
	Row m_rows[4];

	// Input

	inConsumerId m_inputConsumerId;
	struct InputEvents
	{
		inEventId m_startStopQuery;
		inEventId m_scrollUp;
		inEventId m_scrollDown;
	} m_inputEvents;

};

UE_DECLARE_SAMPLE(ueSample_OnlineLeaderboards, "OnlineLeaderboards")
