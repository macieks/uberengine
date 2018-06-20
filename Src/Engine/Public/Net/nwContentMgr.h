#ifndef NS_CONTENT_MGR_H
#define NS_CONTENT_MGR_H

#include "Base/ueProperty.h"
#include "Base/Containers/ueList.h"

struct ueAsync;

/**
 *	@addtogroup nw
 *	@{
 */

struct nsQueryResult;
struct nsQueryTemplate;

//! Query description
struct nsQueryDesc
{
	nsQueryTemplate* m_template;	//!< Query template
	u32 m_numParamValues;			//!< Number of query parameters; must match number of template parameters
	ueValue* m_paramValues;			//!< Query parameter values
	void* m_userData;				//!< User data
	f32 m_timeOutSecs;				//!< Max. query time

	nsQueryDesc() :
		m_template(NULL),
		m_numParamValues(0),
		m_paramValues(NULL),
		m_userData(NULL),
		m_timeOutSecs(10.0f)
	{}
};

//! Query parameter description
struct nsParamDesc
{
	char* m_name;			//!< Parameter name
	ueValueType m_type;		//!< Parameter type
};

//! Query template
struct nsQueryTemplate : ueList<nsQueryTemplate>::Node
{
	char* m_name;			//!< Query template name

	// Input description

	u32 m_numInputs;		//!< Number of input parameters
	nsParamDesc* m_inputs;	//!< Input parameter descriptions

	// Output description

	u32 m_numOutputs;		//!< Number of output parameters
	nsParamDesc* m_outputs;	//!< Output parameter descriptions
};

//! Query result row
struct nsQueryResultRow
{
	u32 m_numValues;		//!< Number of values
	ueValue* m_values;		//!< Values
};

//! Query result
struct nsQueryResult
{
	nsQueryTemplate* m_template;	//!< Template of the query
	u32 m_numRows;					//!< Number of result rows
	nsQueryResultRow* m_rows;		//!< Result rows

	f32 m_time;						//!< Time it took to complete query

	//! Gets result value for a given row and parameter name
	const ueValue* GetValue(u32 row, const char* name, ueValueType type) const;
};

//! Content manager startup parameters
struct nsContentMgrStartupParams
{
	ueAllocator* m_allocator;		//!< Allocator

	const char* m_serverDomain;		//!< Domain of the server to connect to, e.g. www.myserver.com
	const char* m_serverUrl;		//!< Full url of the server to connect to, e.g. http://www.myserver.com/my_page.php

	u32 m_maxQueries;				//!< Max. simulateneous queries

	const char* m_queryTemplateDir;	//!< Directory containing all query templates
	u32 m_maxTemplateMemorySize;	//!< Size of the buffer to contain all query templates

	nsContentMgrStartupParams() :
		m_allocator(NULL),
		m_serverDomain(NULL),
		m_serverUrl(NULL),
		m_maxQueries(5),
		m_queryTemplateDir(NULL),
		m_maxTemplateMemorySize(1 << 16)
	{}
};

//! Starts up content manager
void				nsContentMgr_Startup(nsContentMgrStartupParams* params);
//! Shuts down content manager
void				nsContentMgr_Shutdown();
//! Updates content manager
void				nsContentMgr_Update();

//! Gets query template
nsQueryTemplate*	nsContentMgr_GetQueryTemplate(const char* name);
//! Starts query; use ueAsync_* interface to manage returned asynchronous query handle
ueAsync*			nsContentMgr_DoQuery(nsQueryDesc* desc);
//! Destroys all pending queries
void				nsContentMgr_DestroyAllQueries();

// @}

#endif // NS_CONTENT_MGR_H