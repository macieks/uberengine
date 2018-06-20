#include "gxMovie.h"

#if defined(GX_MOVIE_USE_WMF)

#include "Graphics/glLib.h"
#include "Base/Containers/ueGenericPool.h"

#include "Wmsdk.h"

struct gxMovie
{
	uePath m_path;

	IWMSyncReader* m_syncReader; // opens the file and gets all its properties

	WMVIDEOINFOHEADER m_videoInfo;

	u32 m_numOutputs;
	s32 m_videoOutputNumber;
	s32 m_audioOutputNumber;
	s32 m_videoStreamNumber;
	s32 m_audioStreamNumber;

	u64 m_totalTimeNanoSecs;
	u64 m_currrentPostionNanoSecs;
	u64 m_nextPostionNanoSecs;

	f32 m_playbackSpeed;

	ueBool m_paused;
	ueBool m_loop;

	gxMovie() :
		m_syncReader(NULL),
		m_numOutputs(0),
		m_videoOutputNumber(0),
		m_audioOutputNumber(0),
		m_videoStreamNumber(0),
		m_audioStreamNumber(0),
		m_currrentPostionNanoSecs(0),
		m_nextPostionNanoSecs(0),
		m_totalTimeNanoSecs(0),
		m_playbackSpeed(1.0f),
		m_paused(UE_TRUE),
		m_loop(UE_FALSE)
	{}
};

struct gxMovieMgrData
{
	ueAllocator* m_allocator;
	ueGenericPool m_pool;
	gxMovieMgrData() : m_allocator(NULL) {}
};

static gxMovieMgrData s_data;

void gxMovieMgr_Startup(ueAllocator* allocator, u32 maxVideos)
{
	UE_ASSERT(!s_data.m_allocator);
	s_data.m_allocator = allocator;
	UE_ASSERT_FUNC(s_data.m_pool.Init(allocator, sizeof(gxMovie), maxVideos));
	::CoInitialize(NULL);
}

void gxMovieMgr_Shutdown()
{
	UE_ASSERT(s_data);
	s_data.m_pool.Deinit();
	::CoUninitialize();
	s_data.m_allocator = NULL;
}

ueBool gxMovie_Open(gxMovie* m)
{
	HRESULT hr;

	// Step 1: Create the Reader Object
	hr = WMCreateSyncReader(NULL, 0, &m->m_syncReader);
	if (FAILED(hr))
	{
		ueLogE("Problem initializing the SyncReader.");
		return UE_FALSE;
	}

	// Step 2: Open the file
	// Note: When the synchronous reader opens the file, it assigns an output number to each stream
	WCHAR pathW[UE_MAX_PATH];
	u32 i;
	for (i = 0; m->m_path[i]; i++)
		pathW[i] = m->m_path[i];
	pathW[i] = 0;

	hr = m->m_syncReader->Open(pathW);
	if (FAILED(hr))
	{
		ueLogE("Problem opening movie file '%s'", m->m_path);
		return UE_FALSE;
	}
	
	/* Step 3: Analyse the output stream in the file   //working with outputs MSDN
	   note:A wmv file has Outputs and streams number,a stream in a wmv file starts from 1 not from 0 
			and the Output nubmers strat from 0;
			Every output has a stream refurring to it! the numbers are not the same
			There are functions to find from an Output number a stream number and also the opposite*/
			
	// 3.1: Get the number of outputs:
	DWORD numOutputs;
	m->m_syncReader->GetOutputCount(&numOutputs);
	m->m_numOutputs = numOutputs;

	// 3.2 Loop threw the streams and determine which is the uedio and wich is the video
	for (u32 i = 0; i < m->m_numOutputs; i++)
	{
		IWMOutputMediaProps* videoOutputProps = NULL;
		m->m_syncReader->GetOutputProps(i, &videoOutputProps);

		DWORD theSize;
		hr = videoOutputProps->GetMediaType(NULL, &theSize);

		WM_MEDIA_TYPE* mediaType = (WM_MEDIA_TYPE*) new(s_data.m_allocator) u8[theSize];
		hr = videoOutputProps->GetMediaType(mediaType, &theSize);
		
		if (FAILED(hr))
		{
			ueLogE("Could not query for the space needed for media type");
			return UE_FALSE;
		}

		if (mediaType->majortype == WMMEDIATYPE_Video)
		{
			m->m_videoOutputNumber = i;
			m->m_syncReader->GetStreamNumberForOutput(m->m_videoOutputNumber, (WORD*) &m->m_videoStreamNumber);

			UE_ASSERT(mediaType->formattype == WMFORMAT_VideoInfo);

			// Setting the bitmapInfoHeader by reading the WmvInfoHeader
			ueMemCpy(&m->m_videoInfo, mediaType->pbFormat, sizeof(WMVIDEOINFOHEADER));

			DWORD maxSampleSize = 0;
			hr = m->m_syncReader->GetMaxOutputSampleSize(i, &maxSampleSize);
			UE_ASSERT( SUCCEEDED(hr) );
		}
		else if (mediaType->majortype == WMMEDIATYPE_Audio)
		{
			m->m_audioOutputNumber = i;
			m->m_syncReader->GetStreamNumberForOutput(m->m_audioOutputNumber, (WORD*) &m->m_audioStreamNumber);
		}

		videoOutputProps->Release();
		if (mediaType)
			ueDelete(mediaType, s_data.m_allocator);
	}

	// Step 4: Set to receive correct sample durations.

	/*
	To ensure that the synchronous reader delivers correct sample durations for video streams,
	you must first configure the stream output. Call the IWMSyncReader::SetOutputSetting 
	method to set the g_wszVideoSampleDurations setting to TRUE.
	If True, the reader will deliver accurate sample durations.
	*/

	const BOOL value = TRUE;
	hr = m->m_syncReader->SetOutputSetting(m->m_videoOutputNumber, g_wszVideoSampleDurations, WMT_TYPE_BOOL, (const BYTE*) &value, sizeof(value));
	if (FAILED(hr))
	{
		ueLogE("Could not set Video Ouptut Sample durations");
		return UE_FALSE;
	}
	
	// Step 5: Set To receive Uncompressed Samples
	
	/*
	The SetReadStreamSamples method specifies whether samples from a stream will 
	be 	delivered compressed or uncompressed.
	setting to recive uncompressed samples
	*/

	hr = m->m_syncReader->SetReadStreamSamples(m->m_videoStreamNumber, FALSE);
	if (FAILED(hr))
	{
		ueLogE("Could not set Video Stream to give Uncompressed Samples");
		return UE_FALSE;
	}
		
	// Step 6: Get wmv Duration (total time)

	// Step 6.1 create a MetaData Editor
	IWMMetadataEditor* metadataEditor = NULL;
	hr = WMCreateEditor(&metadataEditor);
	if (FAILED(hr))
	{
		ueLogE("could not set Video Stream to give Uncompressed Samples");
		return UE_FALSE;
	}

	hr = metadataEditor->Open(pathW);
	if (FAILED(hr))
	{
		ueLogE("Could not open metadata editor");
		return UE_FALSE;
	}

	// Step 6.2: Create a HeaderInfo interface. (note: only attributes that were inserted to the header in the creation of the file could be found)
	IWMHeaderInfo3* hdrInfo = NULL;
	hr = metadataEditor->QueryInterface(IID_IWMHeaderInfo3, (void**) &hdrInfo);
	UE_ASSERT(SUCCEEDED(hr));

	WORD wStream = 0; // Any stream; FIXME: select video stream

	WMT_ATTR_DATATYPE dType;
	WORD wSize = 0;
	QWORD dwDuration;

	// Get attribute size
	hr = hdrInfo->GetAttributeByName(&wStream, L"Duration", &dType, (BYTE*) NULL, &wSize);
	UE_ASSERT(wSize == sizeof(dwDuration));

	// Get attribute value
	BYTE pValue[8];
	hr = hdrInfo->GetAttributeByName(&wStream, L"Duration", &dType, (BYTE*) pValue, &wSize);
	dwDuration = *((QWORD*)pValue);
	m->m_totalTimeNanoSecs = dwDuration * 100;

	hdrInfo->Release();
	metadataEditor->Release();

	// Done!
	m->m_currrentPostionNanoSecs = 0;
	m->m_nextPostionNanoSecs = 0;
	return UE_TRUE;
}

gxMovie* gxMovie_Create(const char* path)
{
	gxMovie* m = new (s_data.m_pool) gxMovie();
	ueStrCpyS(m->m_path, path);
	if (!gxMovie_Open(m))
	{
		ueDelete(m, s_data.m_pool);
		return NULL;
	}
	return m;
}

void gxMovie_Close(gxMovie* m)
{
	if (m->m_syncReader)
	{
		m->m_syncReader->Release();
		m->m_syncReader = NULL;
	}
}

void gxMovie_Destroy(gxMovie* m)
{
	gxMovie_Close(m);
	ueDelete(m, s_data.m_pool);
}

ueBool gxMovie_IsPlaying(gxMovie* m) { return !m->m_paused; }
u32 gxMovie_GetWidth(gxMovie* m) { return m->m_videoInfo.bmiHeader.biWidth; }
u32 gxMovie_GetHeight(gxMovie* m) { return m->m_videoInfo.bmiHeader.biHeight; }

ueBool gxMovie_NextFrame(gxMovie* m, glTextureBuffer* texture)
{
	QWORD cnsSampleTime = 0;
	QWORD cnsSampleDuration = 0;
	DWORD dwFlags = 0;
//	DWORD dwOutputNumber;
	INSSBuffer* nssBuffer = NULL;

	HRESULT hr = m->m_syncReader->GetNextSample(
		m->m_videoStreamNumber,
		&nssBuffer,
		&cnsSampleTime,
		&cnsSampleDuration,
		&dwFlags,
		NULL, //&dwOutputNumber,
		NULL);

	if (hr == NS_E_NO_MORE_SAMPLES)
		return UE_FALSE;

	if (FAILED(hr))
		return UE_FALSE;

	if (dwFlags == WM_SF_CLEANPOINT)
	{
		u8* srcData = NULL;
		DWORD srcDataSizeDWORD;
		nssBuffer->GetBufferAndLength(&srcData, &srcDataSizeDWORD);

		const u32 width = m->m_videoInfo.bmiHeader.biWidth;
		const u32 height = m->m_videoInfo.bmiHeader.biHeight;

#if defined(UE_DEBUG)
		const glTextureBufferDesc* texDesc = glTextureBuffer_GetDesc(texture);
		UE_ASSERT(texDesc->m_width == width && texDesc->m_height == height);
#endif

		// Copy to texture

		glTextureData rect;
		UE_ASSERT_FUNC( glTextureBuffer_Lock(texture, &rect, 0, glTextureBufferLockFlags_WriteOnly) );

		const u32 srcRowSize = 3 * width;
		UE_ASSERT(24 == m->m_videoInfo.bmiHeader.biBitCount);

		for (u32 y = 0; y < height; y++)
		{
			u8* dst = (u8*) rect.m_data + rect.m_pitch * y;
			const u8* src = srcData + srcRowSize * y;
			for (u32 x = 0; x < width; x++)
			{
				dst[0] = src[0];
				dst[1] = src[1];
				dst[2] = src[2];
				dst[3] = 255;

				dst += 4;
				src += 3;
			}
		}

		glTextureBuffer_Unlock(texture, 0);
	}

	// Update current position in seconds

	m->m_currrentPostionNanoSecs = cnsSampleTime * 100;
	m->m_nextPostionNanoSecs = (cnsSampleTime + cnsSampleDuration) * 100;

	// Cleaning up before reading next sample
	nssBuffer->Release();
	nssBuffer = NULL;
	
	return UE_TRUE;
}

ueBool gxMovie_Update(gxMovie* m, f32 dt, glTextureBuffer* texture)
{
	if (m->m_paused)
		return UE_FALSE;

	UE_PROF_SCOPE("gxMovie_Update");

	const u64 dtNanoSecs = (u64) ((f64) dt * (f64) m->m_playbackSpeed * 1000000000.0);
	m->m_currrentPostionNanoSecs += dtNanoSecs;

	if (m->m_currrentPostionNanoSecs < m->m_nextPostionNanoSecs)
		return UE_FALSE;

	if (gxMovie_NextFrame(m, texture))
		return UE_TRUE;

	if (m->m_loop)
	{
		gxMovie_Close(m);
		gxMovie_Open(m);

		return gxMovie_NextFrame(m, texture);
	}

	return UE_FALSE;
}

void gxMovie_EnableLooping(gxMovie* m, ueBool enable)
{
	m->m_loop = enable;
}

void gxMovie_SetPlaybackSpeed(gxMovie* m, f32 speed)
{
	m->m_playbackSpeed = speed;
}

void gxMovie_Play(gxMovie* m)
{
	m->m_paused = UE_FALSE;
}

void gxMovie_Pause(gxMovie* m, ueBool pause)
{
	m->m_paused = pause;
}

f32			gxMovie_GetTotalSecs(gxMovie* m) { return (f32) (m->m_totalTimeNanoSecs / 1000000000.0); }
f32			gxMovie_GetCurrentSecs(gxMovie* m) { return (f32) (m->m_currrentPostionNanoSecs / 1000000000.0); }

#else // defined(GX_MOVIE_USE_WMF)
	UE_NO_EMPTY_FILE
#endif