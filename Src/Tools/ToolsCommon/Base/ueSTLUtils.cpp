#include "Base/ueSTLUtils.h"
#include "Utils/utCompression.h"
#include "IO/ioFile.h"

#if defined(UE_LINUX)
    #include <dlfcn.h>
#endif

void vector_u8_compress(u32& compressionLib, const std::vector<u8>& src, std::vector<u8>& dst)
{
	if (compressionLib == UT_NONE_COMPRESSION_SYMBOL)
		return;

	if (src.size() == 0)
	{
		compressionLib = UT_NONE_COMPRESSION_SYMBOL;
		return;
	}

	const ueSize sizeEstimate = utCompression_EstimateCompressedSize(compressionLib, &src[0], src.size());
	dst.resize(sizeEstimate);
	ueSize compressedSize = sizeEstimate;
	UE_ASSERT_FUNC( utCompression_Compress(compressionLib, &dst[0], compressedSize, &src[0], src.size()) );
	if (compressionLib == UT_NONE_COMPRESSION_SYMBOL)
		dst.clear();
	else
		dst.resize(compressedSize);
}

void vector_u8_compress_chunked(u32& compressionLib, const std::vector<u8>& src, std::vector<u8>& dst, u32 chunkSize, std::vector<u32>& chunkOffsets, std::vector<u32>& chunkSizes)
{
	if (compressionLib == UT_NONE_COMPRESSION_SYMBOL)
		return;

	// Try all compression libs and select best (this migth be slow!)

	if (compressionLib == UT_BEST_COMPRESSION_SYMBOL)
	{
		const u32 compressionThreshold = 32;

		u32 bestCompressionLib = UT_NONE_COMPRESSION_SYMBOL;
		u32 bestCompressionSize = (u32) src.size();
		for (u32 i = 0; i < utCompression_GetNumLibs(); i++)
		{
			utCompressionLib* lib = utCompression_GetLibByIndex(i);
			vector_u8_compress_chunked(lib->m_symbol, src, dst, chunkSize, chunkOffsets, chunkSizes);

			if (bestCompressionSize > dst.size())
			{
				bestCompressionLib = lib->m_symbol;
				bestCompressionSize = (u32) dst.size();
			}
		}

		if (bestCompressionLib == UT_NONE_COMPRESSION_SYMBOL ||
			bestCompressionSize + compressionThreshold > (u32) src.size())
		{
			dst.clear();
			chunkOffsets.clear();
			chunkSizes.clear();
			compressionLib = UT_NONE_COMPRESSION_SYMBOL;
			return;
		}

		compressionLib = bestCompressionLib;
		if (bestCompressionLib == utCompression_GetLibByIndex(utCompression_GetNumLibs() - 1)->m_symbol)
			return;
	}

	// Compress using specific compression lib

	UE_ASSERT(compressionLib != UT_NONE_COMPRESSION_SYMBOL &&
		compressionLib != UT_BEST_COMPRESSION_SYMBOL);

	dst.clear();
	chunkOffsets.clear();
	chunkSizes.clear();

	std::vector<u8> chunkSrc;
	std::vector<u8> chunkDst;

	u32 offset = 0;
	u32 compressedOffset = 0;
	while (offset < src.size())
	{
		vector_set_from(chunkSrc, src, offset, ueMin(chunkSize, (u32) src.size() - offset));

		vector_u8_compress(compressionLib, chunkSrc, chunkDst);
		vector_push_back_vector(dst, chunkDst);

		chunkOffsets.push_back(compressedOffset);
		chunkSizes.push_back((u32) chunkDst.size());
		compressedOffset += (u32)chunkDst.size();

		offset += chunkSize;
	}
}

bool vector_u8_load_file(const char* path, std::vector<u8>& data)
{
	ioFilePtr f;
	if (!f.Open(path, ioFileOpenFlags_Read))
		return false;

	const ueSize size = f.GetSize();

	data.resize(size);
	const bool success = f.Read(&data[0], size, 1) == 1;
	if (!success)
		data.clear();

	return success;
}

void vector_u8_push_data(std::vector<u8>& v, const void* data, u32 size)
{
	for (u32 i = 0; i < size; i++)
		v.push_back(*((u8*) data + i));
}

int string_replace_all(std::string& s, const std::string& src, const std::string& dst)
{
	int numReplacements = 0;
	size_t lookHere = 0;
	size_t foundHere;
	while ((foundHere = s.find(src, lookHere)) != std::string::npos)
	{
		s.replace(foundHere, src.size(), dst);
		lookHere = foundHere + dst.size();
		numReplacements++;
	}
	return numReplacements;
}

bool io_create_dir_chain(const char* path, bool isFilePath)
{
	UE_ASSERT(!ueStrStr(path, "\\"));

	std::vector<std::string> parts;
	string_split(parts, path, "/");
	if (isFilePath)
		parts.pop_back();

	std::string dir;
	for (u32 i = 0; i < parts.size(); i++)
	{
		if (i != 0)
			dir += "/";
		dir += parts[i];

		if (parts[i] != ".." && parts[i] != "." && !ioDir_Create(ioFileSys_GetDefault(), dir.c_str(), UE_FALSE))
			return false;
	}

	return true;
}

bool io_get_full_path(std::string& dst, const char* src)
{
#if defined(UE_WIN32)
    char fullPath[512];
	if (GetFullPathName(src, UE_ARRAY_SIZE(fullPath), fullPath, NULL) != 0)
	{
	    dst = fullPath;
	    return true;
	}
	return false;
#elif defined(UE_LINUX)
    dst = src;
    return true;
#endif
}

std::string io_get_exe_path()
{
#if defined(UE_WIN32)
	STARTUPINFO info;
	GetStartupInfo(&info);
	return info.lpTitle;
#elif defined(UE_LINUX)
    char currentExePath[1024];
    const ssize_t count = readlink("/proc/self/exe", currentExePath, UE_ARRAY_SIZE(currentExePath));
    UE_ASSERT(count > 0);
    return currentExePath;
#endif
}

void* dll_load(const char* name)
{
#if defined(UE_WIN32)
    HMODULE handle = LoadLibraryA(name);
	if (!handle)
		// TODO: See http://blogs.msdn.com/b/junfeng/archive/2006/11/20/debugging-loadlibrary-failures.aspx for more debug info
		ueLogE("Failed to load DLL '%s', error = 0x%x", name, GetLastError());
	return handle;
#elif defined(UE_LINUX)
    return dlopen(name, RTLD_LAZY);
#endif
}

void* dll_get_func(void* dll, const char* name)
{
#if defined(UE_WIN32)
    return GetProcAddress((HMODULE) dll, name);
#elif defined(UE_LINUX)
    return dlsym(dll, name);
#endif
}

void dll_unload(void* dll)
{
#if defined(UE_WIN32)
    FreeLibrary((HMODULE) dll);
#elif defined(UE_LINUX)
    dlclose(dll);
#endif
}
