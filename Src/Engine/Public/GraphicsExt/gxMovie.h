#ifndef GX_MOVIE_H
#define GX_MOVIE_H

/**
 *	@addtogroup gx
 *	@{
 */

#include "Graphics/glLib.h"

/**
 *	@struct gxMovie
 *	@brief Movie
 */
struct gxMovie;

//! Starts up movie manager
void		gxMovieMgr_Startup(ueAllocator* allocator, u32 maxVideos = 4);
//! Shuts down movie manager
void		gxMovieMgr_Shutdown();

//! Creates movie object; doesn't start playback yet
gxMovie*	gxMovie_Create(const char* path);
//! Destroys movie
void		gxMovie_Destroy(gxMovie* movie);

//! Gets video width
u32			gxMovie_GetWidth(gxMovie* movie);
//! Gets video height
u32			gxMovie_GetHeight(gxMovie* movie);
//! Gets video texture format
glBufferFormat	gxMovie_GetFormat(gxMovie* movie);

//! Enables movie looping
void		gxMovie_EnableLooping(gxMovie* movie, ueBool enable);
//! Sets movie playback speed
void		gxMovie_SetPlaybackSpeed(gxMovie* movie, f32 speed);
//! Sets uedio volume
void		gxMovie_SetVolume(gxMovie* movie, f32 volume);

//! Tells whether movie is playing
ueBool		gxMovie_IsPlaying(gxMovie* movie);
//! Starts playing the movie
void		gxMovie_Play(gxMovie* movie);
//! Pauses movie
void		gxMovie_Pause(gxMovie* movie, ueBool pause);
//! Rewinds movie to given time
void		gxMovie_Rewind(gxMovie* m, f32 time);

//! Updates movie; if new video frame is hit, video data is written to texture buffer
ueBool		gxMovie_Update(gxMovie* movie, f32 dt, glTextureBuffer* tb);

//! Gets total movie time in seconds
f32			gxMovie_GetTotalSecs(gxMovie* movie);
//! Gets current movie time in seconds
f32			gxMovie_GetCurrentSecs(gxMovie* movie);

//! Movie helper class; handles double buffering frames into textures
class gxMoviePlayer
{
public:
	gxMoviePlayer();
	~gxMoviePlayer();

	//! Initializes movie playback
	ueBool Create(const char* path);
	//! Deinitializes movie playback
	void Destroy();

	//! Enables looping
	void EnableLooping(ueBool enable);
	//! Sets playback speed (1.0f default)
	void SetPlaybackSpeed(f32 speed);
	//! Sets uedio volume (1.0f default)
	void SetVolume(f32 volume);

	//! Starts or resumes playing movie
	void Play();
	//! Pauses movie
	void Pause(ueBool pause);
	//! Updates movie
	void Update(f32 dt);

	//! Gets current texture buffer
	glTextureBuffer* GetTextureBuffer();

private:
	gxMovie* m_movie;
	glTextureBuffer* m_movieTextures[2];
	u32 m_currentVideoTexture;
};

// @}

#endif // GX_MOVIE_H
