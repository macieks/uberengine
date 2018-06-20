#include "gxMovie.h"
#include "Graphics/glLib.h"

gxMoviePlayer::gxMoviePlayer() :
	m_movie(NULL)
{
	m_movieTextures[0] = m_movieTextures[1] = NULL;
}

gxMoviePlayer::~gxMoviePlayer()
{
	Destroy();
}

ueBool gxMoviePlayer::Create(const char* path)
{
	// Open movie stream

	m_movie = gxMovie_Create(path);
	if (!m_movie)
		return UE_FALSE;

	// Create 2 textures for double buffering the movie

	glTextureBufferDesc texDesc;
	texDesc.m_width = gxMovie_GetWidth(m_movie);
	texDesc.m_height = gxMovie_GetHeight(m_movie);
	texDesc.m_format = gxMovie_GetFormat(m_movie);
	texDesc.m_flags = glTextureBufferFlags_IsTexture | glTextureBufferFlags_IsDynamic;
	texDesc.m_numLevels = 1;
	for (u32 i = 0; i < 2; i++)
	{
		m_movieTextures[i] = glTextureBuffer_Create(&texDesc);
		if (!m_movieTextures[i])
		{
			ueLogE("Failed to create movie texture (width = %d, height = %d)", texDesc.m_width, texDesc.m_height);
			Destroy();
			return UE_FALSE;
		}

		glTextureBuffer_SetDebugName(m_movieTextures[i], i == 0 ? "movie-0" : "movie-1");
	}
	m_currentVideoTexture = 0;

	return UE_TRUE;
}

void gxMoviePlayer::Destroy()
{
	if (m_movie)
	{
		gxMovie_Destroy(m_movie);
		m_movie = NULL;
	}

	for (u32 i = 0; i < 2; i++)
		if (m_movieTextures[i])
		{
			glTextureBuffer_Destroy(m_movieTextures[i]);
			m_movieTextures[i] = NULL;
		}
}

void gxMoviePlayer::EnableLooping(ueBool enable)
{
	gxMovie_EnableLooping(m_movie, enable);
}

void gxMoviePlayer::SetPlaybackSpeed(f32 speed)
{
	gxMovie_SetPlaybackSpeed(m_movie, speed);
}

void gxMoviePlayer::SetVolume(f32 volume)
{
	gxMovie_SetVolume(m_movie, volume);
}

void gxMoviePlayer::Play()
{
	gxMovie_Play(m_movie);
}

void gxMoviePlayer::Pause(ueBool pause)
{
	gxMovie_Pause(m_movie, pause);
}

void gxMoviePlayer::Update(f32 dt)
{
	if (gxMovie_Update(m_movie, dt, m_movieTextures[1 - m_currentVideoTexture]))
		m_currentVideoTexture = 1 - m_currentVideoTexture;
}

glTextureBuffer* gxMoviePlayer::GetTextureBuffer()
{
	return m_movieTextures[m_currentVideoTexture];
}
