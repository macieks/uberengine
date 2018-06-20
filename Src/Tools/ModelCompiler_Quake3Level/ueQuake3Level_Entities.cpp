#include "ModelCompiler_Common/ueToolModel.h"

#include "ueQuake3Level.h"

std::vector<entity_t> s_entities;

// Parsing

#define	BSPMAXTOKEN	1024
#define	BSPMAX_KEY				32
#define	BSPMAX_VALUE			1024

typedef struct
{
	char	filename[1024];
	char    *buffer,*script_p,*end_p;
	int     line;
} BSPScript;

#define	MAX_INCLUDES	8
BSPScript	scriptstack[MAX_INCLUDES];
BSPScript	*script;
int			scriptline;

char token[BSPMAXTOKEN];
bool endofscript;
bool tokenready;                     // only true if UnQ3_GetToken was just called

bool Q3_GetToken(bool crossline);

/*
==============
parseFromMemory
==============
*/
void Q3_StartParsing(char *buffer, int size)
{
	script = scriptstack;
	script++;
	ueStrCpyS(script->filename, "memory buffer");

	script->buffer = buffer;
	script->line = 1;
	script->script_p = script->buffer;
	script->end_p = script->buffer + size;

	endofscript = false;
	tokenready = false;
}


bool Q3_IsEndOfScript(bool crossline)
{
	if (!crossline && !strcmp(script->filename, "memory buffer"))
	{
		endofscript = true;
		return false;
	}

	if (script == scriptstack+1)
	{
		endofscript = true;
		return false;
	}
	script--;
	scriptline = script->line;
	return Q3_GetToken(crossline);
}

/*

==============
Q3_GetToken
==============
*/
bool Q3_GetToken(bool crossline)
{
	char    *token_p;

	if (tokenready)                         // is a token allready waiting?
	{
		tokenready = false;
		return true;
	}

	if (script->script_p >= script->end_p)
		return Q3_IsEndOfScript (crossline);

	//
	// skip space
	//
skipspace:
	while (*script->script_p <= 32)
	{
		if (script->script_p >= script->end_p)
			return Q3_IsEndOfScript (crossline);
		if (*script->script_p++ == '\n')
		{
			if (!crossline)
			{
				//printf("Line %i is incomplete\n",scriptline);
			}
			scriptline = script->line++;
		}
	}

	if (script->script_p >= script->end_p)
		return Q3_IsEndOfScript (crossline);

	// ; # // comments
	if (*script->script_p == ';' || *script->script_p == '#'
		|| ( script->script_p[0] == '/' && script->script_p[1] == '/') )
	{
		if (!crossline)
		{
			//printf("Line %i is incomplete\n",scriptline);
		}
		while (*script->script_p++ != '\n')
			if (script->script_p >= script->end_p)
				return Q3_IsEndOfScript (crossline);
		scriptline = script->line++;
		goto skipspace;
	}

	// /* */ comments
	if (script->script_p[0] == '/' && script->script_p[1] == '*')
	{
		if (!crossline)
		{
			//printf("Line %i is incomplete\n",scriptline);
		}
		script->script_p+=2;
		while (script->script_p[0] != '*' && script->script_p[1] != '/')
		{
			if ( *script->script_p == '\n' ) {
				scriptline = script->line++;
			}
			script->script_p++;
			if (script->script_p >= script->end_p)
				return Q3_IsEndOfScript (crossline);
		}
		script->script_p += 2;
		goto skipspace;
	}

	//
	// copy token
	//
	token_p = token;

	if (*script->script_p == '"')
	{
		// quoted token
		script->script_p++;
		while (*script->script_p != '"')
		{
			*token_p++ = *script->script_p++;
			if (script->script_p == script->end_p)
				break;
			if (token_p == &token[BSPMAXTOKEN])
			{
				//printf ("Token too large on line %i\n",scriptline);
			}
		}
		script->script_p++;
	}
	else	// regular token
		while ( *script->script_p > 32 && *script->script_p != ';')
		{
			*token_p++ = *script->script_p++;
			if (script->script_p == script->end_p)
				break;
		}

		*token_p = 0;

		if (!strcmp (token, "$include"))
		{
			return false;
		}

		return true;
}

bool Q3_ParseEntity()
{
	if (!Q3_GetToken(true))
		return false;

	if (strcmp(token, "{"))
		return false;

	entity_t& entity = vector_push(s_entities);

	while (1)
	{
		Q3_GetToken(true);
		if (!strcmp(token, "}"))
			break;

		std::string key = token;
		Q3_GetToken(false);
		entity.epairs[key] = token;
	};

	return true;
}

void Q3_ParseEntities(char* buffer, int bufferSize)
{
	s_entities.clear();
	Q3_StartParsing(buffer, bufferSize);
	while (Q3_ParseEntity()) {}	
}

// Utils

const char* Q3_GetEntityValue(const entity_t& e, const char* name)
{
	const std::string* value = map_find(e.epairs, name);
	return value ? value->c_str() : NULL;
}

f32 Q3_GetEntityValue_Float(const entity_t& e, const char* name)
{
	const char* value = Q3_GetEntityValue(e, name);
	if (!value)
		return 0;
	return !value ? 0 : (f32) atof(value);
}

void Q3_GetEntityValue_Vec3(const entity_t& e, const char* name, f32* vec)
{
	const char* value = Q3_GetEntityValue(e, name);
	if (!value)
		vec[0] = vec[1] = vec[2] = 0;
	else
		ueStrScanf(value, "%f %f %f", &vec[0], &vec[1], &vec[2]);
}

const char* Q3_GetEntityClass(const entity_t& e)
{
	return Q3_GetEntityValue(e, "classname");
}

// Spawn points

static const char s_spawnPointPrefix[] = "info_player_";
static const int s_spawnPointPrefixLength = sizeof(s_spawnPointPrefix) - 1;

void Q3_GetEntities(const char* buffer, int bufferSize, std::vector<entity_t>& entities)
{
	s_entities.clear();
	Q3_ParseEntities(const_cast<char*>(buffer), bufferSize);
	entities = s_entities;
}

void Q3_GetSpawnPoints(const char* buffer, int bufferSize, std::vector<spawnpoint_t>& points)
{
	s_entities.clear();
	Q3_ParseEntities(const_cast<char*>(buffer), bufferSize);

	for (u32 i = 0; i < s_entities.size(); i++)
	{
		const entity_t& curr = s_entities[i];

		const char* className = Q3_GetEntityClass(curr);

		if (className && !strncmp(className, s_spawnPointPrefix, s_spawnPointPrefixLength))
		{
			spawnpoint_t& point = vector_push(points);
			Q3_GetEntityValue_Vec3(curr, "origin", point.m_pos);
			point.m_angleY = Q3_GetEntityValue_Float(curr, "angle");
		}
	}
}
