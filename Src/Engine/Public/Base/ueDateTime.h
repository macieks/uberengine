#ifndef UE_DATE_TIME_H
#define UE_DATE_TIME_H

/**
 *	@addtogroup ue
 *	@{
 */

//! Default date and time format
#define UE_DATE_TIME_DEFAULT_FORMAT "yyyy-MM-dd hh:mm:ss"

// Clock
// ------------------------

//! Starts up engine clock
void	ueClock_Startup();
//! Shuts down engine clock
void	ueClock_Shutdown();

//! Gets current time
ueTime	ueClock_GetCurrent();
//! Gets seconds since given time
f32		ueClock_GetSecsSince(ueTime ticks);
//! Gets milliseconds since given time
f32		ueClock_GetMillisecsSince(ueTime ticks);
//! Converts time interval to seconds
f32		ueClock_GetSecs(ueTime start, ueTime end);
//! Converts time interval to seconds
f32		ueClock_GetSecs(ueTime ticks);
// Converts seconds to time
ueTime	ueClock_SecsToTime(f32 secs);
//! Converts time to micro seconds
u64		ueClock_TimeToMicroSecs(ueTime time);
//! Converts time to nano seconds
u64		ueClock_TimeToNanoSecs(ueTime time);

//! Timer used to measure time taken by some operations
class ueTimer
{
public:
	//! Starts timer
	UE_INLINE ueTimer(ueBool start = UE_TRUE) { if (start) Restart(); }
	//! Restarts timer
	UE_INLINE void Restart() { m_start = ueClock_GetCurrent(); }
	//! Gets time elapsed since timer was started
	UE_INLINE f32 GetSecs() const { return ueClock_GetSecs(m_start, ueClock_GetCurrent()); }

private:
	ueTime m_start;
};

//! Application timer
class ueAppTimer
{
public:
	//! Initialized timer
	void Init();
	//! Updates the timer
	void Tick();

	//! Gets last delta time (frame time)
	UE_INLINE f32 DT() const { return m_currDeltaF32; }
	//! Gets frames per second (calculated based on a couple past frames)
	UE_INLINE f32 FPS() const { return m_fps; }

private:
	ueTime m_startTime;

	ueTime m_lastUpdate;

	ueTime m_deltas[16];
	u32 m_nextDeltaIndex;
	f32 m_currDeltaF32;

	f32 m_fps;
};

//! Date and time
struct ueDateTime
{
	u16 m_year;			//!< Year, e.g. 2010
	u8 m_month;			//!< Month value within 1..12 range
	u8 m_dayOfMonth;	//!< Day of month value within 1..31

	u8 m_hour;			//!< Hour value within 0..23
	u8 m_minute;		//!< Minute value within 0..59
	u8 m_second;		//!< Second value within 0..59

	//! Gets current date and time
	static void GetCurrent(ueDateTime& dateTime);
	//! Gets current date and time; returns pointer to internal date and time
	static const ueDateTime& GetCurrent();

	//! Converts ANSI time to date and time
	static void ToDateTime(ueDateTime& dateTime, time_t t);

#if defined(UE_WIN32) || defined(UE_X360)
	//! Converts Win32 FILETIME to date and time
	static void ToDateTime(ueDateTime& dateTime, FILETIME t);
#endif

	//! Sets from string in given format
	ueBool FromString(const char* str, const char* format = UE_DATE_TIME_DEFAULT_FORMAT);
	//! Gets date and time as string in given format
	void ToString(char* buffer, u32 size, const char* format = UE_DATE_TIME_DEFAULT_FORMAT) const;
	//! Gets date and time as string in given format; returns pointer to internal buffer (valid until next call to ToString)
	const char* ToString(const char* format = UE_DATE_TIME_DEFAULT_FORMAT) const;

	UE_INLINE ueBool operator == (const ueDateTime& other) const { return ueMemCmp(this, &other, 7) == 0; }
	UE_INLINE ueBool operator != (const ueDateTime& other) const { return ueMemCmp(this, &other, 7) != 0; }
	ueBool operator < (const ueDateTime& other) const;
};

// @}

#endif // UE_DATE_TIME_H
