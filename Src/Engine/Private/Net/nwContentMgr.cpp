#include "Base/ueAsync_Private.h"
#include "Net/nwContentMgr.h"
#include "Net/nwSocket.h"
#include "IO/ioFile.h"
#include "IO/ioXml.h"
#include "Base/Allocators/ueStackAllocator.h"

struct nsQuery : ueAsync, ueList<nsQuery>::Node
{
	// State

	ueTime m_createTime;
	nsSocket m_socket;

	ueTime m_lastRecvTime;

	u32 m_serverIpLookupCounter;

	// Input

	nsQueryDesc* m_desc;

	s8* m_requestBuffer;
	u32 m_requestSize;
	u32 m_sentRequestSize;

	// Output

	char* m_buffer;
	u32 m_bufferSize;
	u32 m_bufferCapacity;

	nsQueryResult m_result;
};

struct nsContentMgrData
{
	ueAllocator* m_allocator;

	ueBool m_serverIpFound;
	uePath m_serverDomain;
	uePath m_serverUrl;
	nsAddr m_serverAddr;

	uePath m_queryTemplateDir;

	u32 m_maxQueries;
	ueList<nsQuery> m_queries;

	ueList<nsQueryTemplate> m_templates;

	u8* m_templateMemory;
	u32 m_templateMemorySize;

	ueAsync* m_serverLookupAsync;

	nsContentMgrData() : m_allocator(NULL) {}
};

void nsContentMgr_UpdateQuery(nsQuery* query);
void* nsContentMgr_GetQueryResult(nsQuery* query);

nsContentMgrData s_data;

const ueValue* nsQueryResult::GetValue(u32 row, const char* name, ueValueType type) const
{
	UE_ASSERT(row < m_numRows);
	for (u32 i = 0; i < m_template->m_numOutputs; i++)
		if (!ueStrCmp(m_template->m_outputs[i].m_name, name))
			return m_template->m_outputs[i].m_type == type ? &m_rows[row].m_values[i] : NULL;
	return NULL;
}

ueBool nsContentMgr_ParseTemplateParams(u32& count, nsParamDesc*& params, ueAllocator* allocator, ioXmlNode* queryNode, const char* parentNodeName, const char* paramNodeName)
{
	ioXmlNode* parentNode = ioXmlNode_GetFirstNode(queryNode, parentNodeName);
	if (!parentNode)
		return UE_FALSE;

	count = ioXmlNode_CalcNumNodes(parentNode, paramNodeName);
	params = (nsParamDesc*) allocator->Alloc(sizeof(nsParamDesc) * count);
	if (!params)
		return UE_FALSE;

	u32 index = 0;
	for (ioXmlNode* paramNode = ioXmlNode_GetFirstNode(parentNode, paramNodeName); paramNode; paramNode = ioXmlNode_GetNext(paramNode, paramNodeName))
	{
		const char* paramName = ioXmlNode_GetAttrValue(paramNode, "name");
		const char* paramTypeName = ioXmlNode_GetAttrValue(paramNode, "type");
		if (!paramName || !paramTypeName)
			return UE_FALSE;

		ueValueType paramType;
		if (!ueStrCmp(paramTypeName, "s32")) paramType = ueValueType_S32;
		else if (!ueStrCmp(paramTypeName, "s64")) paramType = ueValueType_S64;
		else if (!ueStrCmp(paramTypeName, "f32")) paramType = ueValueType_F32;
		else if (!ueStrCmp(paramTypeName, "f64")) paramType = ueValueType_F64;
		else if (!ueStrCmp(paramTypeName, "string")) paramType = ueValueType_String;
		else if (!ueStrCmp(paramTypeName, "binary")) paramType = ueValueType_Binary;
		else return UE_FALSE;

		nsParamDesc& param = params[index++];
		param.m_type = paramType;
		param.m_name = ueStrDup(allocator, paramName);
		if (!param.m_name)
			return UE_FALSE;
	}

	return UE_TRUE;
}

ueBool nsContentMgr_TemplateFileCallback(const ioEnumInfo* info, void* userData)
{
	ueStackAllocator* allocator = (ueStackAllocator*) userData;

	// Put mem marker here, so we can unwind all allocations on failure

	ueStackAllocatorRewinder allocatorRewinder(allocator);

	// Get query name

	uePath name;
	ioPath_ExtractFileNameWithoutExtension(info->m_path, name, UE_ARRAY_SIZE(name));
	for (u32 i = 0; name[i]; i++)
		if (name[i] == '.')
		{
			name[i] = 0;
			break;
		}
	const s32 nameLen = ueStrLen(name);

	ueLogI("Parsing query template '%s'...", name);

	// Load Xml

	ioXmlDoc* doc = ioXmlDoc_Load(s_data.m_allocator, info->m_path);
	if (!doc)
	{
		ueLogE("Failed to parse query (path = '%s')", info->m_path);
		return UE_TRUE;
	}
	ioXmlDocScopedDestructor docDestructor(doc);

	// Create query (in place) based on Xml description

	nsQueryTemplate* t = new(allocator) nsQueryTemplate();
	if (!t)
		return UE_FALSE;

	t->m_name = ueStrDup(allocator, name);
	if (!t->m_name)
		return UE_FALSE;

	ioXmlNode* queryNode = ioXmlDoc_GetFirstNode(doc, "query");
	if (!queryNode)
		return UE_FALSE;

	if (!nsContentMgr_ParseTemplateParams(t->m_numInputs, t->m_inputs, allocator, queryNode, "inputs", "input") ||
		!nsContentMgr_ParseTemplateParams(t->m_numOutputs, t->m_outputs, allocator, queryNode, "outputs", "output"))
		return UE_FALSE;

	// Success!

	ueLogI("Query template added");

	allocatorRewinder.Disable();
	s_data.m_templates.PushBack(t);
	return UE_TRUE;
}

void nsContentMgr_LoadTemplates()
{
	// Load from XML

	ueStackAllocator allocator;
	allocator.InitMem(s_data.m_templateMemory, s_data.m_templateMemorySize);

	ioFileSys_Enumerate(ioFileSys_GetDefault(), s_data.m_queryTemplateDir, "*.query.xml", nsContentMgr_TemplateFileCallback, ioEnumFlags_Files, &allocator);
}

void nsContentMgr_UnloadTemplates()
{
	s_data.m_templates.Clear();
}

ueBool nsContentMgr_UpdateConnection(nsQuery* query)
{
	// Start asynchronous TCP connection

	if (!query->m_socket)
	{
		// Create socket

		nsSocket socket;
		if (!nsSocket_Create(NS_AF_INET, NS_SOCK_STREAM, NS_IPPROTO_TCP, &socket))
		{
			ueLogE("Failed to create socket, error = %s", nsSocket_GetLastErrorString());
			query->m_state = ueAsyncState_Failed;
			return UE_FALSE;
		}

		if (!nsSocket_SetNonBlocking(socket, UE_TRUE))
		{
			ueLogE("Failed to set socket into non-blocking state, error = %s", nsSocket_GetLastErrorString());
			nsSocket_Close(socket);
			query->m_state = ueAsyncState_Failed;
			return UE_FALSE;
		}

		// Connect (asynchronous)

		s32 connResult = 0;
		if (!nsSocket_Connect(socket, &s_data.m_serverAddr, sizeof(s_data.m_serverAddr), &connResult) &&
			!nsSocket_IsWouldBlock())
		{
			ueLogE("Failed to start async connection to server (address = %s), error = %s", s_data.m_serverDomain, nsSocket_GetErrorString(connResult));
			nsSocket_Close(socket);
			query->m_state = ueAsyncState_Failed;
			return UE_FALSE;
		}

		query->m_socket = socket;
	}

	// Check connection state

	nsSocketConnectionState state = nsSocket_GetConnectionState(query->m_socket);
	switch (state)
	{
		case nsSocketConnectionState_Establishing:
			return UE_FALSE;
		case nsSocketConnectionState_Failed:
			ueLogE("Failed to establish connection to server (address = %s), error = %s", s_data.m_serverAddr.ToString(), nsSocket_GetLastErrorString());
			query->m_state = ueAsyncState_Failed;
			return UE_FALSE;
		case nsSocketConnectionState_Valid:
			return UE_TRUE;
	}
	return UE_FALSE;
}

void nsContentMgr_Startup(nsContentMgrStartupParams* params)
{
	s_data.m_allocator = params->m_allocator;

	ueStrCpyS(s_data.m_serverDomain, params->m_serverDomain);
	ueStrCpyS(s_data.m_serverUrl, params->m_serverUrl);
	ueStrCpyS(s_data.m_queryTemplateDir, params->m_queryTemplateDir);

	s_data.m_maxQueries = params->m_maxQueries;

	// Set up templates

	s_data.m_templateMemorySize = params->m_maxTemplateMemorySize;
	s_data.m_templateMemory = (u8*) s_data.m_allocator->Alloc(params->m_maxTemplateMemorySize);
	UE_ASSERT(s_data.m_templateMemory);

	nsContentMgr_LoadTemplates();

	// Set up TCP address of the server

	s_data.m_serverLookupAsync = NULL;
	s_data.m_serverIpFound = UE_FALSE;
}

void nsContentMgr_Shutdown()
{
	UE_ASSERT(s_data.m_allocator);

	if (s_data.m_serverLookupAsync)
	{
		ueAsync_Destroy(s_data.m_serverLookupAsync, UE_TRUE);
		s_data.m_serverLookupAsync = NULL;
	}

	nsContentMgr_DestroyAllQueries();

	s_data.m_allocator->Free(s_data.m_templateMemory);
	nsContentMgr_UnloadTemplates();

	s_data.m_allocator = NULL;
}

void nsContentMgr_DestroyAllQueries()
{
	while (nsQuery* query = s_data.m_queries.Front())
		ueAsync_Destroy(query, UE_TRUE);
}

void nsContentMgr_UpdateServerIpLookup(nsQuery* query)
{
	// Check running query

	if (s_data.m_serverLookupAsync)
	{
		const ueAsyncState state = ueAsync_GetState(s_data.m_serverLookupAsync);
		switch (state)
		{
			case ueAsyncState_Failed:
				s_data.m_serverIpFound = UE_FALSE;
				ueAsync_Destroy(s_data.m_serverLookupAsync);
				s_data.m_serverLookupAsync = NULL;
				ueLogW("Failed to look up server ip address (domain = '%s').", s_data.m_serverDomain);
				break;
			case ueAsyncState_Succeeded:
				s_data.m_serverIpFound = UE_TRUE;
				ueAsync_Destroy(s_data.m_serverLookupAsync);
				s_data.m_serverLookupAsync = NULL;
				ueLogD("Successfully got server ip (%s = %s)", s_data.m_serverDomain, s_data.m_serverAddr.m_ip.ToString());
				break;
			case ueAsyncState_InProgress:
				break;
		}
		return;
	}

	// Is limit exceeded for the IP lookup query?

	if (query->m_serverIpLookupCounter == 0)
	{
		ueLogE("Exceeded number of server ip lookups");
		query->m_state = ueAsyncState_Failed;
		return;
	}
	query->m_serverIpLookupCounter--;

	// Start new IP lookup query

	const u16 httpPort = 80;

	ueMemSet(&s_data.m_serverAddr, 0, sizeof(s_data.m_serverAddr));
	s_data.m_serverAddr.m_family = NS_AF_INET;
	s_data.m_serverAddr.m_port = nsSocket_HostToNetU16(httpPort);

	if (!nsSocket_GetAddrInfo(s_data.m_serverDomain, &s_data.m_serverAddr.m_ip, &s_data.m_serverLookupAsync))
	{
		s_data.m_serverIpFound = UE_FALSE;
		ueLogW("Failed to start server ip look up task (domain = '%s').", s_data.m_serverDomain);
	}
}

nsQueryTemplate* nsContentMgr_GetQueryTemplate(const char* name)
{
	nsQueryTemplate* t = s_data.m_templates.Front();
	while (t)
	{
		if (!ueStrCmp(name, t->m_name))
			return t;
		t = t->Next();
	}
	return NULL;
}

void nsContentMgr_DestroyQuery(nsQuery* query, ueBool /*block*/)
{
	s_data.m_queries.Remove(query);

	if (nsSocket_IsValid(query->m_socket))
	{
		nsSocket_Shutdown(query->m_socket);
		nsSocket_Close(query->m_socket);
	}

	if (query->m_buffer)
		s_data.m_allocator->Free(query->m_buffer);
	if (query->m_requestBuffer)
		s_data.m_allocator->Free(query->m_requestBuffer);
	if (query->m_result.m_rows)
		s_data.m_allocator->Free(query->m_result.m_rows);
	s_data.m_allocator->Free(query);
}

void nsContentMgr_BuildRequest(nsQueryDesc* desc, char* request, u32& requestLength)
{
	char* requestStart = request;
	requestLength = 0;

	nsQueryTemplate* templ = desc->m_template;

/*
	Sample HTTP GET request for reference:

	GET /path/script.cgi?field1=value1&field2=value2 HTTP/1.0
	From: someuser@jmarshall.com
	User-Agent: HTTPTool/1.0
	[blank line here]
*/

	ueStrCat2(request, "GET ");
	ueStrCat2(request, s_data.m_serverUrl);
	ueStrCat2(request, "?");
	for (u32 i = 0; i < templ->m_numInputs; i++)
	{
		const ueValue& paramValue = desc->m_paramValues[i];

		ueStrCat2(request, templ->m_inputs[i].m_name);
		ueStrCat2(request, "=");

		switch (paramValue.m_type)
		{
			case ueValueType_S32:
			{
				char temp[32];
				ueStrFormatS(temp, "%d", paramValue.m_s32);
				ueStrCat2(request, temp);
				break;
			}
			case ueValueType_S64:
			{
				char temp[64];
				ueStrFormatS(temp, "%lld", paramValue.m_s64);
				ueStrCat2(request, temp);
				break;
			}
			case ueValueType_F32:
			{
				char temp[32];
				ueStrFormatS(temp, "%f", paramValue.m_f32);
				ueStrCat2(request, temp);
				break;
			}
			case ueValueType_F64:
			{
				char temp[64];
				ueStrFormatS(temp, "%lf", (f32) paramValue.m_f64);
				ueStrCat2(request, temp);
				break;
			}
			case ueValueType_DateTime:
			{
				// TODO: Convert to universal time

				char temp[32];
				paramValue.m_dateTime.ToString(temp, UE_ARRAY_SIZE(temp), "yyyy_MM_dd_hh_mm_ss");
				ueStrCat2(request, temp);
				break;
			}
			case ueValueType_String:
				ueStrCat2(request, paramValue.m_string);
				break;
			UE_INVALID_CASE(paramValue.m_type);
		}

		ueStrCat2(request, "&");
	}
	ueStrCat2(request, "password=secret_password&user_name=unknown&query=");
	ueStrCat2(request, templ->m_name);

	ueStrCat2(request, "\nFrom: UberEngine\n");
	ueStrCat2(request, "User-Agent: HTTPTool/1.0\n\n");

	requestLength = (ueSize) request - (ueSize) requestStart + 1;
}

ueAsync* nsContentMgr_DoQuery(nsQueryDesc* desc)
{
	if (s_data.m_queries.Length() == s_data.m_maxQueries)
	{
		ueLogW("Failed to create query (max queries = %u).", s_data.m_maxQueries);
		return NULL;
	}

	// Verify query validity

	UE_ASSERT(desc->m_template);

	if (desc->m_template->m_numInputs != desc->m_numParamValues)
	{
		ueLogE("Template inputs count and query params count don't match (%d != %d)", desc->m_template->m_numInputs, desc->m_numParamValues);
		return NULL;
	}
	for (u32 i = 0; i < desc->m_template->m_numInputs; i++)
		if (desc->m_template->m_inputs[i].m_type != desc->m_paramValues[i].m_type)
		{
			ueLogE("Template input and query parameter %d types don't match (%d != %d)", i, desc->m_template->m_inputs[i].m_type, desc->m_paramValues[i].m_type);
			return NULL;
		}

	// Create query

	nsQuery* query = new(s_data.m_allocator) nsQuery();
	if (!query)
		return NULL;

	query->m_pollFunc = (ueAsync_PollFunc) nsContentMgr_UpdateQuery;
	query->m_destroyFunc = (ueAsync_DestroyFunc) nsContentMgr_DestroyQuery;
	query->m_getDataFunc = (ueAsync_GetDataFunc) nsContentMgr_GetQueryResult;

	s_data.m_queries.PushBack(query);

	query->m_desc = desc;
	query->m_userData = desc->m_userData;
	query->m_socket = 0;
	query->m_createTime = ueClock_GetCurrent();
	query->m_buffer = NULL;
	query->m_bufferSize = 0;
	query->m_bufferCapacity = 0;
	query->m_result.m_rows = NULL;
	query->m_requestBuffer = NULL;
	query->m_requestSize = 0;
	query->m_sentRequestSize = 0;

	query->m_serverIpLookupCounter = 3;

	// Do one-step update (to initiate asynchronous operations)

	nsContentMgr_UpdateQuery(query);

	// Return

	return query;
}

ueBool nsContentMgr_ParseResult(nsQuery* query)
{
	nsQueryResult& result = query->m_result;
	result.m_template = query->m_desc->m_template;
	result.m_numRows = 0;
	result.m_rows = NULL;
	result.m_time = ueClock_GetSecsSince(query->m_createTime);

	// Estimate result memory size

	char* tokenPtrs[128];
	u32 numTokens = UE_ARRAY_SIZE(tokenPtrs);
	if (!ueStrTokenize(query->m_buffer, " ", tokenPtrs, &numTokens))
		return UE_FALSE;
	if (numTokens == 0)
		return UE_TRUE; // Empty result

	nsQueryTemplate* t = query->m_desc->m_template;

	if (numTokens % t->m_numOutputs != 0)
		return UE_FALSE;
	result.m_numRows = numTokens / t->m_numOutputs;

	u32 memorySize = (sizeof(nsQueryResultRow) + sizeof(ueValue) * t->m_numOutputs) * result.m_numRows;

	for (u32 i = 0; i < result.m_numRows; i++)
		for (u32 j = 0; j < t->m_numOutputs; j++)
			if (t->m_outputs[j].m_type == ueValueType_Binary)
			{
				const char* srcToken = tokenPtrs[i * t->m_numOutputs + j];
				UE_NOT_IMPLEMENTED();
			}

	// Create result

	u8* memory = (u8*) s_data.m_allocator->Alloc(memorySize);
	UE_ASSERT(memory);

	result.m_rows = (nsQueryResultRow*) memory;
	memory += sizeof(nsQueryResultRow) * result.m_numRows;

	for (u32 i = 0; i < result.m_numRows; i++)
	{
		nsQueryResultRow& row = result.m_rows[i];
		row.m_numValues = t->m_numOutputs;
		row.m_values = (ueValue*) memory;
		memory += sizeof(ueValue) * t->m_numOutputs;

		for (u32 j = 0; j < t->m_numOutputs; j++)
		{
			char* srcToken = tokenPtrs[i * t->m_numOutputs + j];

			ueValue& dstValue = row.m_values[j];
			dstValue.m_type = t->m_outputs[j].m_type;

			switch (dstValue.m_type)
			{
			case ueValueType_S32:
				ueStrScanf(srcToken, "%d", &dstValue.m_s32);
				break;
			case ueValueType_S64:
				ueStrScanf(srcToken, "%lld", &dstValue.m_s64);
				break;
			case ueValueType_F32:
				ueStrScanf(srcToken, "%f", &dstValue.m_f32);
				break;
			case ueValueType_F64:
				ueStrScanf(srcToken, "%lf", &dstValue.m_f64);
				break;
			case ueValueType_String:
				dstValue.m_string = srcToken;
				break;
			case ueValueType_Binary:
				UE_NOT_IMPLEMENTED();
				break;
			}
		}
	}

	return UE_TRUE;
}

void nsContentMgr_CheckQueryTimeout(nsQuery* query)
{
	if (query->m_desc->m_timeOutSecs != 0.0f &&
		ueClock_GetSecsSince(query->m_createTime) >= query->m_desc->m_timeOutSecs)
	{
		ueLogW("Query '%s' timed out", query->m_desc->m_template->m_name);
		query->m_state = ueAsyncState_Failed;
	}
}

void nsContentMgr_UpdateQuery(nsQuery* query)
{
	if (query->m_state == ueAsyncState_Failed)
		return;

	// Lookup server ip (if not done before)

	if (!s_data.m_serverIpFound)
	{
		nsContentMgr_UpdateServerIpLookup(query);
		if (!s_data.m_serverIpFound)
		{
			nsContentMgr_CheckQueryTimeout(query);
			return;
		}
	}

	// Connect to server

	if (!nsContentMgr_UpdateConnection(query))
	{
		nsContentMgr_CheckQueryTimeout(query);
		return;
	}

	// Build request

	if (!query->m_requestSize)
	{
		char request[1 << 12];
		u32 requestSize = UE_ARRAY_SIZE(request);
		nsContentMgr_BuildRequest(query->m_desc, request, requestSize);

		query->m_requestBuffer = (s8*) s_data.m_allocator->Alloc(requestSize);
		if (!query->m_requestBuffer)
		{
			query->m_state = ueAsyncState_Failed;
			return;
		}
		ueMemCpy(query->m_requestBuffer, request, requestSize);

		query->m_requestSize = requestSize;
		query->m_sentRequestSize = 0;
	}

	// Send request

	if (query->m_sentRequestSize < query->m_requestSize)
	{
		const s32 sendResult = nsSocket_Send(query->m_socket, query->m_requestBuffer + query->m_sentRequestSize, query->m_requestSize - query->m_sentRequestSize, 0);
		if (sendResult < 0)
		{
			if (nsSocket_IsWouldBlock())
				nsContentMgr_CheckQueryTimeout(query);
			else
			{
				ueLogE("Failed to send query, reason: nsSocket_Send returned %s", nsSocket_GetLastErrorString());
				query->m_state = ueAsyncState_Failed;
			}
			return;
		}
		else
		{
			query->m_sentRequestSize += sendResult;
			if (query->m_sentRequestSize < query->m_requestSize)
			{
				nsContentMgr_CheckQueryTimeout(query);
				return;
			}

			ueLogD("Successfully sent query request (name = %s)", query->m_desc->m_template->m_name);
		}
	}

	// Receive response

	while (1)
	{
		// Resize the buffer on demand

		#define NW_CONTENT_MGR_RESULT_BUFFER_CHUNK (1 << 12)

		if (query->m_bufferSize + NW_CONTENT_MGR_RESULT_BUFFER_CHUNK > query->m_bufferCapacity)
		{
			u32 newCapacity = query->m_bufferCapacity == 0 ? NW_CONTENT_MGR_RESULT_BUFFER_CHUNK : query->m_bufferCapacity * 2;
			while (query->m_bufferSize + NW_CONTENT_MGR_RESULT_BUFFER_CHUNK > newCapacity)
				newCapacity *= 2;

			char* newBuffer = (char*) s_data.m_allocator->Alloc(newCapacity);
			UE_ASSERT(newBuffer);

			if (query->m_bufferSize > 0)
			{
				ueMemCpy(newBuffer, query->m_buffer, query->m_bufferSize);
				s_data.m_allocator->Free(query->m_buffer);
			}

			query->m_buffer = newBuffer;
			query->m_bufferCapacity = newCapacity;
		}

		// Receive data

		const s32 recvLen = nsSocket_Receive(query->m_socket, (char*) query->m_buffer + query->m_bufferSize, query->m_bufferCapacity - query->m_bufferSize, 0);

		// Nothing received?

		if (recvLen == 0)
			break;

		// Error?

		if (recvLen < 0)
		{
			if (nsSocket_IsWouldBlock())
				nsContentMgr_CheckQueryTimeout(query);
			else
			{
				ueLogE("Failed to complete '%s' query, reason: nsSocket_Receive failed with %s", query->m_desc->m_template->m_name, nsSocket_GetLastErrorString());
				query->m_state = ueAsyncState_Failed;
			}
			return;
		}

		// Received more bytes!

		if (recvLen > 0)
		{
			query->m_bufferSize += recvLen;
			query->m_lastRecvTime = ueClock_GetCurrent();
		}
	}

	// Check if we've received whole HTTP response

	static const char* endingTag = "<UE_END_OF_QUERY_RESULT>";
	static const u32 endingTagLen = ueStrLen(endingTag);

	if (query->m_bufferSize >= endingTagLen && !ueMemCmp(query->m_buffer + query->m_bufferSize - endingTagLen, endingTag, endingTagLen))
	{
		query->m_buffer[query->m_bufferSize - endingTagLen] = 0;

		// Parse result & dispatch

		if (nsContentMgr_ParseResult(query))
			query->m_state = ueAsyncState_Succeeded;
		else
			query->m_state = ueAsyncState_Failed;
		return;
	}

	nsContentMgr_CheckQueryTimeout(query);
}

void nsContentMgr_Update()
{
	nsQuery* query = s_data.m_queries.Front();
	while (query)
	{
		nsContentMgr_UpdateQuery(query);
		query = query->Next();
	}
}

void* nsContentMgr_GetQueryResult(nsQuery* query)
{
	return &query->m_result;
}