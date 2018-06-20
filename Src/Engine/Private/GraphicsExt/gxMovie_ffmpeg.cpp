// Nice ffmpeg reference: http://dranger.com/ffmpeg/tutorial08.html

#include "gxMovie.h"
#include "IO/ioFile.h"

#if defined(GX_MOVIE_USE_FFMPEG)

//#define GX_USE_STREAM

#include "Graphics/glLib.h"
#include "Audio/auLib.h"
#include "Base/Containers/ueGenericPool.h"

extern "C"
{
	#define UINT64_C u64
	#include "libavcodec\avcodec.h"
	#include "libavformat\avformat.h"
	#include "libswscale\swscale.h"
}

struct gxMovie_av_stream
{
	u8* m_buffer;
	ueSize m_bufferSize;
	ByteIOContext* m_ctx;
	ioFile* m_file;
};

void gxMovie_av_stream_Create(gxMovie_av_stream* s, u32 bufferSize);
void gxMovie_av_stream_Destroy(gxMovie_av_stream* s);

struct gxMovie
{
	uePath m_path;

	// Video

	AVFormatContext* m_videoFormatCtx;
	s32 m_videoStream;
	AVCodec* m_videoCodec;
	AVCodecContext* m_videoCodecCtx;
	SwsContext* m_swsCtx;

	AVFrame* m_frame;

	f32 m_videoFrameTime;
	f32 m_nextVideoFrameTime;

	glBufferFormat m_format;

#if defined(GX_USE_STREAM)
	gxMovie_av_stream m_video_av_stream;
#endif

	// Audio

	AVFormatContext* m_audioFormatCtx;
	s32 m_audioStream;
	AVCodec* m_audioCodec;
	AVCodecContext* m_audioCodecCtx;
	auDynamicSound* m_sound;

	f32 m_avgAudioBytesPerSec;
	f32 m_nextAudioFrameTime;

#if defined(GX_USE_STREAM)
	gxMovie_av_stream m_audio_av_stream;
#endif

	// Common

	f32 m_totalTime;
	f32 m_currentTime;

	f32 m_playbackSpeed;
	ueBool m_paused;
	ueBool m_loop;

	gxMovie() :
		m_videoFormatCtx(NULL),
		m_videoStream(-1),
		m_videoCodec(NULL),
		m_videoCodecCtx(NULL),
		m_swsCtx(NULL),
		m_audioFormatCtx(NULL),
		m_audioStream(-1),
		m_audioCodec(NULL),
		m_audioCodecCtx(NULL),
		m_sound(NULL),
		m_frame(NULL),
		m_playbackSpeed(1.0f),
		m_paused(UE_TRUE),
		m_loop(UE_FALSE),
		m_format(glBufferFormat_Unknown)
	{}
};

struct gxMovieMgrData
{
	ueAllocator* m_allocator;
	ueGenericPool m_pool;
};

static gxMovieMgrData* s_data = NULL;

#if defined(GX_USE_STREAM)

int gxMovie_av_stream_read_func(void *opaque, uint8_t *buf, int buf_size)
{
	gxMovie_av_stream* s = (gxMovie_av_stream*) opaque;
	return ioFile_Read(s->m_file, buf, 1, buf_size);
}

int64_t gxMovie_av_stream_seek_func(void *opaque, int64_t offset, int whence)
{
	gxMovie_av_stream* s = (gxMovie_av_stream*) opaque;
	ioFileSeekType seekType;
	switch (whence)
	{
		case SEEK_SET: seekType = ioFileSeekType_Set; break;
		case SEEK_CUR: seekType = ioFileSeekType_Offset; break;
		case SEEK_END: seekType = ioFileSeekType_End; break;
	}
	return ioFile_Seek(s->m_file, seekType, (ueSize) offset);
}

ueBool gxMovie_av_stream_Create(gxMovie_av_stream* s, const char* path, ueSize bufferSize)
{
	s->m_file = ioFile_Open(path, ioFileOpenFlags_Read);
	if (!s->m_file)
	{
		ueLogE("Failed to open video/audio file for reading, path = '%s'", path);
		return UE_FALSE;
	}

	s->m_bufferSize = bufferSize;
	s->m_buffer = (u8*) s_data->m_allocator->Alloc(bufferSize);
	UE_ASSERT(s->m_buffer);
#if 1
	s->m_ctx = av_alloc_put_byte(s->m_buffer, s->m_bufferSize, 0, s, gxMovie_av_stream_read_func, NULL, gxMovie_av_stream_seek_func);
#else
	init_put_byte(&s->m_ctx, s->m_buffer, s->m_bufferSize, 0, s, gxMovie_av_stream_read_func, NULL, gxMovie_av_stream_seek_func);
#endif

	return UE_TRUE;
}

void gxMovie_av_stream_Destroy(gxMovie_av_stream* s)
{
	ioFile_Close(s->m_file);
	UE_ASSERT(s->m_buffer);
	s_data->m_allocator->Free(s->m_buffer);
	s->m_buffer = NULL;
}

#endif // (GX_USE_STREAM)

void gxMovieMgr_Startup(ueAllocator* allocator, u32 maxVideos)
{
	UE_ASSERT(!s_data);

	s_data = new(allocator) gxMovieMgrData();
	UE_ASSERT(s_data);
	s_data->m_allocator = allocator;

	UE_ASSERT_FUNC(s_data->m_pool.Init(allocator, sizeof(gxMovie), maxVideos));

	av_register_all();
}

void gxMovieMgr_Shutdown()
{
	UE_ASSERT(s_data);
	s_data->m_pool.Deinit();
	ueDelete(s_data, s_data->m_allocator);
	s_data = NULL;
}

ueBool gxMovie_OpenVideo(gxMovie* m)
{
	// Find video stream
#if defined(GX_USE_STREAM)

	if (!gxMovie_av_stream_Create(&m->m_video_av_stream, m->m_path, 1 << 16))
		return UE_FALSE;

	AVFormatParameters ap;
	memset(&ap, 0, sizeof(ap));
	ap.prealloced_context = 1;
	m->m_videoFormatCtx = av_alloc_format_context();// avformat_alloc_context();

	// FIXME: Fails !!!!
	if (av_open_input_stream(&m->m_videoFormatCtx, m->m_video_av_stream.m_ctx, "stream", m->m_videoFormatCtx->iformat, &ap) != 0)
	{
		ueLogE("Failed to play video from file (file exists, but is invalid or unsupported format), path = '%s'", m->m_path);
		gxMovie_av_stream_Destroy(&m->m_video_av_stream);
		return UE_FALSE;
	}
#else
	if (av_open_input_file(&m->m_videoFormatCtx, m->m_path, NULL, NULL, NULL) != 0)
	{
		ueLogE("Failed to play movie file (video), path = '%s'", m->m_path);
		return UE_FALSE;
	}
#endif

	if (av_find_stream_info(m->m_videoFormatCtx) < 0)
	{
		ueLogE("No video streams info, path = '%s'", m->m_path);
		return UE_FALSE;
	}

	m->m_videoStream = -1;
	for (u32 i = 0; i < m->m_videoFormatCtx->nb_streams; i++)
	{
		AVStream* stream = m->m_videoFormatCtx->streams[i];
		if (stream->codec->codec_type == CODEC_TYPE_VIDEO)
		{
			m->m_videoStream = i;
			m->m_videoFrameTime = (f32) ((f64) stream->r_frame_rate.den / (f64) stream->r_frame_rate.num);
			m->m_videoCodecCtx = stream->codec;

			m->m_totalTime = (f32) ((f64) stream->duration * av_q2d(stream->time_base));
			UE_ASSERT(m->m_totalTime > 0.0f);
			break;
		}
	}
	if (m->m_videoStream == -1)
	{
		ueLogE("No video streams, path = '%s'", m->m_path);
		return UE_FALSE;
	}

	// Set up video stream

	m->m_videoCodec = avcodec_find_decoder(m->m_videoCodecCtx->codec_id);
	if (!m->m_videoCodec)
	{
		ueLogE("No video codec availaible, path = '%s'", m->m_path);
		return UE_FALSE;
	}

	if (avcodec_open(m->m_videoCodecCtx, m->m_videoCodec) <0)
	{
		ueLogE("Video codec could not be opened, path = '%s'", m->m_path);
		return UE_FALSE;
	}

	m->m_frame = avcodec_alloc_frame();
	UE_ASSERT(m->m_frame);

	m->m_nextVideoFrameTime = 0.0f;
	av_seek_frame(m->m_videoFormatCtx, m->m_videoStream, 0, 0);

	// Create scaling utility

	s32 swsFormat;
#ifdef GL_OPENGL_ES
	swsFormat = PIX_FMT_RGB24;
	m->m_format = glBufferFormat_R8G8B8A8;
#elif defined(GL_OPENGL)
	swsFormat = PIX_FMT_BGR24;
	m->m_format = glBufferFormat_B8G8R8A8;
#else
	swsFormat = PIX_FMT_RGBA32;
	m->m_format = glBufferFormat_Native_R8G8B8A8;
#endif

	m->m_swsCtx = sws_getContext(m->m_videoCodecCtx->width, m->m_videoCodecCtx->height, m->m_videoCodecCtx->pix_fmt, m->m_videoCodecCtx->width, m->m_videoCodecCtx->height, swsFormat, SWS_FAST_BILINEAR, NULL, NULL, NULL);
	UE_ASSERT(m->m_swsCtx);

	return UE_TRUE;
}

ueBool gxMovie_OpenAudio(gxMovie* m)
{
	// Find audio stream

	if (av_open_input_file(&m->m_audioFormatCtx, m->m_path, NULL, NULL, NULL) != 0)
	{
		ueLogE("Failed to open movie file (audio), path = '%s'", m->m_path);
		return UE_FALSE;
	}

	if (av_find_stream_info(m->m_audioFormatCtx) < 0)
	{
		ueLogE("No audio stream info, path = '%s'", m->m_path);
		return UE_FALSE;
	}

	m->m_audioStream = -1;
	for (u32 i = 0; i < m->m_audioFormatCtx->nb_streams; i++)
	{
		AVStream* stream = m->m_audioFormatCtx->streams[i];
		if (stream->codec->codec_type == CODEC_TYPE_AUDIO)
		{
			m->m_audioStream = i;
			m->m_audioCodecCtx = stream->codec;
			break;
		}
	}
	if (m->m_audioStream == -1)
	{
		ueLogE("No audio streams, path = '%s'", m->m_path);
		return UE_FALSE;
	}

	// Set up audio stream

	m->m_audioCodec = avcodec_find_decoder(m->m_audioCodecCtx->codec_id);
	if (!m->m_audioCodec)
	{
		ueLogE("No audio codec availaible, path = '%s'", m->m_path);
		return UE_FALSE;
	}

	if (avcodec_open(m->m_audioCodecCtx, m->m_audioCodec) <0)
	{
		ueLogE("Audio codec could not be opened, path = '%s'", m->m_path);
		return UE_FALSE;
	}

	const u32 bitsPerSample = 16;
	m->m_avgAudioBytesPerSec = (f32) (m->m_audioCodecCtx->sample_rate * m->m_audioCodecCtx->channels * bitsPerSample / 8);

	m->m_nextAudioFrameTime = 0.0f;
	av_seek_frame(m->m_audioFormatCtx, m->m_audioStream, 0, 0);

	// Create dynamic sound

	auDynamicSoundParams dynSoundParams;

	dynSoundParams.m_allocator = s_data->m_allocator;
	dynSoundParams.m_ringBufferSize = AVCODEC_MAX_AUDIO_FRAME_SIZE; // Hmm...  1 << 16;
	dynSoundParams.m_maxBufferChunks = 16;

	dynSoundParams.m_numChannels = m->m_audioCodecCtx->channels;
	dynSoundParams.m_samplesPerSec = m->m_audioCodecCtx->sample_rate;
	dynSoundParams.m_bitsPerSample = bitsPerSample;
	dynSoundParams.m_blockAlign = dynSoundParams.m_numChannels * dynSoundParams.m_bitsPerSample / 8;
	dynSoundParams.m_avgBytesPerSec = (u32) m->m_avgAudioBytesPerSec;

	m->m_sound = auDynamicSound_Create(dynSoundParams);
	if (!m->m_sound)
	{
		avcodec_close(m->m_audioCodecCtx);
		m->m_audioCodecCtx = NULL;

		av_close_input_file(m->m_audioFormatCtx);
		m->m_audioFormatCtx = NULL;

		return UE_FALSE;
	}

	return UE_TRUE;
}

void gxMovie_Close(gxMovie* m);

ueBool gxMovie_Open(gxMovie* m)
{
	UE_ASSERT(s_data);

	// Open video stream

	if (!gxMovie_OpenVideo(m))
	{
		gxMovie_Close(m);
		return UE_FALSE;
	}

	// Open audio stream (optional)

	gxMovie_OpenAudio(m);

	// Start playback

	m->m_currentTime = 0.0f;
	return UE_TRUE;
}

gxMovie* gxMovie_Create(const char* path)
{
	UE_ASSERT(s_data);

	gxMovie* m = new (s_data->m_pool) gxMovie();
	ueStrCpyS(m->m_path, path);
	if (!gxMovie_Open(m))
	{
		ueDelete(m, s_data->m_pool);
		return NULL;
	}
	return m;
}

void gxMovie_Close(gxMovie* m)
{
	UE_ASSERT(s_data);

	// Close audio

	if (m->m_audioCodecCtx)
	{
		avcodec_close(m->m_audioCodecCtx);
		m->m_audioCodecCtx = NULL;
	}

	if (m->m_sound)
	{
		auDynamicSound_Destroy(m->m_sound);
		m->m_sound = NULL;
	}

	if (m->m_audioFormatCtx)
	{
		av_close_input_file(m->m_audioFormatCtx);
		m->m_audioFormatCtx = NULL;
	}

	// Close video

	if (m->m_swsCtx)
	{
		av_free(m->m_swsCtx);
		m->m_swsCtx = NULL;
	}

	if (m->m_frame)
	{
		av_free(m->m_frame);
		m->m_frame = NULL;
	}

	if (m->m_videoCodecCtx)
	{
		avcodec_close(m->m_videoCodecCtx);
		m->m_videoCodecCtx = NULL;
	}

	if (m->m_videoFormatCtx)
	{
#if defined(GX_USE_STREAM)
		av_close_input_stream(m->m_videoFormatCtx);
		gxMovie_av_stream_Destroy(&m->m_video_av_stream);
#else
		av_close_input_file(m->m_videoFormatCtx);
#endif
		m->m_videoFormatCtx = NULL;
	}
}

void gxMovie_Destroy(gxMovie* m)
{
	UE_ASSERT(s_data);
	gxMovie_Close(m);
	ueDelete(m, s_data->m_pool);
}

u32 gxMovie_GetWidth(gxMovie* m)
{
	UE_ASSERT(s_data);
	return m->m_videoCodecCtx->width;
}

u32 gxMovie_GetHeight(gxMovie* m)
{
	UE_ASSERT(s_data);
	return m->m_videoCodecCtx->height;
}

glBufferFormat	gxMovie_GetFormat(gxMovie* movie)
{
	return movie->m_format;
}

ueBool gxMovie_NextVideoFrame(gxMovie* m, glTextureBuffer* texture)
{
	UE_ASSERT(s_data);

	if (m->m_currentTime < m->m_nextVideoFrameTime)
		return UE_FALSE;

	s32 res;
	AVPacket packet;

	res = av_read_frame(m->m_videoFormatCtx, &packet);
	if (res < 0)
		return UE_FALSE;
	if (packet.stream_index != m->m_videoStream)
		return UE_FALSE;

	s32 frameFinished;
	avcodec_decode_video(m->m_videoCodecCtx, m->m_frame, &frameFinished, packet.data, packet.size);
	if (frameFinished)
	{
#if defined(UE_ENABLE_ASSERTION)
		const glTextureBufferDesc* texDesc = glTextureBuffer_GetDesc(texture);
		UE_ASSERT(texDesc->m_width == m->m_videoCodecCtx->width && texDesc->m_height == m->m_videoCodecCtx->height);
#endif

		glTextureData rect;
		UE_ASSERT_FUNC( glTextureBuffer_Lock(texture, &rect, 0, 0, glBufferLockFlags_Write | glBufferLockFlags_Discard, NULL) );

		uint8_t* data[4] = {0};
		data[0] = (uint8_t*) rect.m_data;
		s32 linesize[4] = {0};
		linesize[0] = rect.m_rowPitch;
		sws_scale(m->m_swsCtx, m->m_frame->data, m->m_frame->linesize, 0, m->m_videoCodecCtx->height, data, linesize);

		glTextureBuffer_Unlock(texture, 0, 0);
	}

	m->m_nextVideoFrameTime += m->m_videoFrameTime;

	av_free_packet(&packet);

	return UE_TRUE;
}

void gxMovie_UpdateAudio(gxMovie* m)
{
	UE_ASSERT(s_data);

	s32 res;
	AVPacket packet;

	while (m->m_nextAudioFrameTime <= m->m_currentTime)
	{
		// Read

		res = av_read_frame(m->m_audioFormatCtx, &packet);
		if (res < 0)
			return;
		if (packet.stream_index != m->m_audioStream)
			continue;

		// Decode and submit

		static u8 buffer[AVCODEC_MAX_AUDIO_FRAME_SIZE];
		s32 decodedBytes = AVCODEC_MAX_AUDIO_FRAME_SIZE;

		if (avcodec_decode_audio2(m->m_audioCodecCtx, (int16_t*) buffer, &decodedBytes, packet.data, packet.size) <= 0)
			ueLogE("Error decoding sound from '%s'", m->m_path);
		else
			auDynamicSound_SubmitBuffer(m->m_sound, buffer, decodedBytes);

		m->m_nextAudioFrameTime += decodedBytes / m->m_avgAudioBytesPerSec;

		av_free_packet(&packet);
	}
}

void gxMovie_Rewind(gxMovie* m, f32 time)
{
	UE_ASSERT(s_data);
	if (m->m_videoFormatCtx)
		av_seek_frame(m->m_videoFormatCtx, m->m_videoStream, 0, 0);

	if (m->m_audioFormatCtx)
	{
		auDynamicSound_FlushBuffers(m->m_sound);
		av_seek_frame(m->m_audioFormatCtx, m->m_audioStream, 0, 0);
	}

	m->m_currentTime = time;
	m->m_nextAudioFrameTime = time;
	m->m_nextVideoFrameTime = time;
}

ueBool gxMovie_Update(gxMovie* m, f32 dt, glTextureBuffer* texture)
{
	UE_ASSERT(s_data);
	if (m->m_paused)
		return UE_FALSE;

	UE_PROF_SCOPE("gxMovie_Update");

	dt = ueMod(dt, m->m_totalTime);

	m->m_currentTime += dt * m->m_playbackSpeed;
	if (m->m_currentTime >= m->m_totalTime)
	{
		if (m->m_loop)
		{
			gxMovie_Rewind(m, 0.0f);
			return gxMovie_Update(m, dt, texture);
		}
		else
		{
			m->m_paused = UE_TRUE;
			return UE_FALSE;
		}
	}

	const ueBool gotVideoFrame = gxMovie_NextVideoFrame(m, texture);
	if (m->m_audioFormatCtx)
		gxMovie_UpdateAudio(m);
	return gotVideoFrame;
}


void gxMovie_EnableLooping(gxMovie* m, ueBool enable)
{
	UE_ASSERT(s_data);
	m->m_loop = enable;
}

void gxMovie_SetPlaybackSpeed(gxMovie* m, f32 speed)
{
	UE_ASSERT(s_data);
	m->m_playbackSpeed = speed;
}

void gxMovie_SetVolume(gxMovie* m, f32 volume)
{
	UE_ASSERT(s_data);
	if (m->m_sound)
		auDynamicSound_SetVolume(m->m_sound, volume);
}

void gxMovie_Play(gxMovie* m)
{
	UE_ASSERT(s_data);
	m->m_paused = UE_FALSE;
}

void gxMovie_Pause(gxMovie* m, ueBool pause)
{
	UE_ASSERT(s_data);
	m->m_paused = pause;
}

#else // defined(GX_MOVIE_USE_FFMPEG)
	UE_NO_EMPTY_FILE
#endif