#pragma once

#include "Base/ueBase.h"

#include <vector>
#include <map>
#include <set>
#include <string>

// std::vector

template <class TYPE>
inline void vector_push_back_array(std::vector<TYPE>& v, const TYPE* elements, u32 count)
{
	for (u32 i = 0; i < count; i++)
		v.push_back(elements[i]);
}

template <class TYPE>
inline void vector_remove_at(std::vector<TYPE>& v, u32 index)
{
	v[index] = v[v.size() - 1];
	v.pop_back();
}

template <class TYPE>
inline void vector_remove_at_preserve_order(std::vector<TYPE>& v, u32 index)
{
	for (u32 i = index; i < v.size() - 1; i++)
		v[i] = v[i + 1];
	v.pop_back();
}

template <class TYPE>
inline void vector_push_back_vector(std::vector<TYPE>& v, std::vector<TYPE>& w)
{
	for (u32 i = 0; i < w.size(); i++)
		v.push_back(w[i]);
}

template <class TYPE>
inline void vector_set_from(std::vector<TYPE>& dst, const std::vector<TYPE>& src, u32 start, u32 count)
{
	dst.clear();
	for (u32 i = 0; i < count; i++)
		dst.push_back(src[start + i]);
}

template <class TYPE>
inline const TYPE* vector_data(const std::vector<TYPE>& v)
{
	return v.size() ? &v[0] : NULL;
}

template <class TYPE>
inline TYPE* vector_data(std::vector<TYPE>& v)
{
	return v.size() ? &v[0] : NULL;
}

template <class TYPE>
inline TYPE& vector_last(std::vector<TYPE>& v)
{
	UE_ASSERT(v.size());
	return v[ v.size() - 1 ];
}

template <class TYPE>
inline TYPE& vector_push(std::vector<TYPE>& v)
{
	v.push_back(TYPE());
	return v[ v.size() - 1 ];
}

inline bool vector_contains(const std::vector<std::string>& v, const char* value)
{
	for (u32 i = 0; i < v.size(); i++)
		if (v[i] == value)
			return true;
	return false;
}

template <class TYPE>
inline bool vector_contains(const std::vector<TYPE>& v, const TYPE& value)
{
	for (u32 i = 0; i < v.size(); i++)
		if (v[i] == value)
			return true;
	return false;
}

// std::string

inline void string_split(std::vector<std::string>& tokens, const std::string& str, const std::string& delimiters = " ")
{
	tokens.clear();

    // Skip delimiters at beginning.
	std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    std::string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (std::string::npos != pos || std::string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}

inline void string_format(std::string& s, const char* format, ...)
{
	static char buffer[8192];

	va_list args;
	va_start(args, format);
	const s32 result = ueStrFormatVArgs(buffer, UE_ARRAY_SIZE(buffer), format, args);
	va_end(args);

	s = buffer;
}

inline void string_to_lower(std::string& s)
{
	ueStrToLower(&s[0]);
}

inline void string_appendf(std::string& s, const char* format, ...)
{
	static char buffer[8192];

	va_list args;
	va_start(args, format);
	const s32 result = ueStrFormat(buffer, UE_ARRAY_SIZE(buffer), format, args);
	va_end(args);

	s.append(buffer);
}

inline bool string_starts_with(const std::string& s, const std::string& prefix)
{
	return s.find(prefix) == 0;
}

inline bool string_ends_with(std::string s, std::string suffix, bool caseSensitive)
{
	if (s.length() < suffix.length())
		return false;

	if (!caseSensitive)
	{
		string_to_lower(s);
		string_to_lower(suffix);
	}
		
	return !strcmp(&s[0] + s.length() - suffix.length(), &suffix[0]);
}

int string_replace_all(std::string& s, const std::string& src, const std::string& dst);

template <typename KEY_TYPE, typename VAL_TYPE, typename PRED_TYPE>
inline VAL_TYPE* map_find(std::map<KEY_TYPE, VAL_TYPE, PRED_TYPE>& map, const KEY_TYPE& key)
{
    typedef typename std::map<KEY_TYPE, VAL_TYPE, PRED_TYPE>::iterator iter_type;
    iter_type iter = map.find(key);
	return (iter != map.end()) ? &iter->second : NULL;
}

template <class KEY_TYPE, class VAL_TYPE, class PRED_TYPE>
inline const VAL_TYPE* map_find(const std::map<KEY_TYPE, VAL_TYPE, PRED_TYPE>& map, const KEY_TYPE& key)
{
	typedef typename std::map<KEY_TYPE, VAL_TYPE, PRED_TYPE>::const_iterator iter_type;
    iter_type iter = map.find(key);
	return (iter != map.end()) ? &iter->second : NULL;
}

template <class KEY_TYPE, class VAL_TYPE, class PRED_TYPE>
inline const VAL_TYPE* map_find(const std::map<KEY_TYPE, VAL_TYPE, PRED_TYPE>& map, const char* key)
{
    typedef typename std::map<KEY_TYPE, VAL_TYPE, PRED_TYPE>::const_iterator iter_type;
    iter_type iter = map.find(key);
	return (iter != map.end()) ? &iter->second : NULL;
}

bool vector_u8_load_file(const char* path, std::vector<u8>& data);
void vector_u8_compress(u32& compressionLib, const std::vector<u8>& src, std::vector<u8>& dst);
void vector_u8_compress_chunked(u32& compressionLib, const std::vector<u8>& src, std::vector<u8>& dst, u32 chunkSize, std::vector<u32>& chunkOffsets, std::vector<u32>& chunkSizes);
void vector_u8_push_data(std::vector<u8>& v, const void* data, u32 size);

bool io_create_dir_chain(const char* filePath, bool isFilePath = true);
bool io_get_full_path(std::string& dst, const char* src);
std::string io_get_exe_path();

void* dll_load(const char* name);
void* dll_get_func(void* dll, const char* name);
void dll_unload(void* dll);

/*
template <class TYPE>
inline bool file_write_vector(FILE* f, std::vector<TYPE>& v, bool writeSize)
{
	const u32 size = v.size();
	if (writeSize && fwrite(&size, sizeof(size), 1, f) != 1)
		return false;
	return fwrite(vector_data(v), sizeof(TYPE) * size, 1, f) == 1;
}
*/
