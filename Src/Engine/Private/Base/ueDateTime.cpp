#include "Base/ueBase.h"
#include "Base/ueDateTime.h"

#include <time.h>

#if defined(UE_LINUX) || defined(UE_MAC)
    #include <sys/time.h>
#endif

struct ueClockData
{
    ueBool m_isInitialized;
    ueTime m_startTicks;

    s64 m_frequency;
    f32 m_ticksToSecs;
    f32 m_ticksToMilliSecs;

    ueClockData() : m_isInitialized(UE_FALSE) {}
};

static ueClockData s_data;

void ueClock_Startup()
{
	UE_ASSERT(!s_data.m_isInitialized);

#if defined(UE_WIN32) || defined(UE_X360)

	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);

	s_data.m_frequency = frequency.QuadPart;
	s_data.m_ticksToSecs = 1.0f / (f32) frequency.QuadPart;
	s_data.m_ticksToMilliSecs = 1000.0f / (f32) frequency.QuadPart;

	LARGE_INTEGER startTime;
	QueryPerformanceCounter(&startTime);
	s_data.m_startTicks = startTime.QuadPart;

#else

    s_data.m_frequency = 1000;
    s_data.m_ticksToSecs = 1.0f / 1000000.0f;
    s_data.m_ticksToMilliSecs = 1.0f / 1000.0f;

#endif

	s_data.m_isInitialized = UE_TRUE;
}

void ueClock_Shutdown()
{
	UE_ASSERT(s_data.m_isInitialized);
	s_data.m_isInitialized = UE_FALSE;
}

ueTime ueClock_GetCurrent()
{
	UE_ASSERT(s_data.m_isInitialized);

#if defined(UE_WIN32) || defined(UE_X360)

	LARGE_INTEGER t;
	QueryPerformanceCounter(&t);
	return t.QuadPart;

#elif defined(UE_MARMALADE)

	return s3eTimerGetMs() * 1000;

#else

    timeval curr;
	gettimeofday(&curr, NULL);
	return (ueTime) curr.tv_usec + (ueTime) curr.tv_sec * 1000000; // Microsecs

#endif
}

const ueDateTime& ueDateTime::GetCurrent()
{
	static ueDateTime dt;
	GetCurrent(dt);
	return dt;
}

f32 ueClock_GetSecsSince(ueTime ticks)
{
	return (ueClock_GetCurrent() - ticks) * s_data.m_ticksToSecs;
}

f32 ueClock_GetMillisecsSince(ueTime ticks)
{
	return (ueClock_GetCurrent() - ticks) * s_data.m_ticksToMilliSecs;
}

f32 ueClock_GetSecs(ueTime ticks)
{
	return ticks * s_data.m_ticksToSecs;
}

f32 ueClock_GetSecs(ueTime start, ueTime end)
{
	return (end - start) * s_data.m_ticksToSecs;
}

ueTime ueClock_SecsToTime(f32 secs)
{
	return (ueTime) ((f64) secs / (f64) s_data.m_ticksToSecs);
}

u64 ueClock_TimeToMicroSecs(ueTime time)
{
	return 1000000 * time / s_data.m_frequency;
}

u64 ueClock_TimeToNanoSecs(ueTime time)
{
	return 1000000000 * time / s_data.m_frequency;
}

// ueAppTimer

void ueAppTimer::Init()
{
	m_startTime = m_lastUpdate = ueClock_GetCurrent();
	m_fps = 30.0f;
	m_currDeltaF32 = 1.0f / m_fps;
	m_deltas[0] = ueClock_SecsToTime(m_currDeltaF32);
	for (u32 i = 1; i < UE_ARRAY_SIZE(m_deltas); i++)
		m_deltas[i] = m_deltas[0];
	m_nextDeltaIndex = 0;
}

void ueAppTimer::Tick()
{
	const ueTime time = ueClock_GetCurrent();
	m_deltas[m_nextDeltaIndex] = time - m_lastUpdate;
	m_currDeltaF32 = ueClock_GetSecs(m_deltas[m_nextDeltaIndex]);
	m_nextDeltaIndex = (m_nextDeltaIndex + 1) % UE_ARRAY_SIZE(m_deltas);
	m_lastUpdate = time;

	// Calc smoothed fps
	ueTime timeAcc = 0;
	for (u32 i = 0; i < UE_ARRAY_SIZE(m_deltas); i++)
		timeAcc += m_deltas[i];
	m_fps = 1.0f / (ueClock_GetSecs(timeAcc) / (f32) UE_ARRAY_SIZE(m_deltas));
}

// ueDateTime

void ueDateTime::GetCurrent(ueDateTime& dateTime)
{
	ToDateTime(dateTime, time(NULL));
}

void ueDateTime::ToDateTime(ueDateTime& dateTime, time_t t)
{
	struct tm fullTime;

#if defined(UE_WIN32) || defined(UE_X360)
	localtime_s(&fullTime, &t);
#else
    const time_t currTime = time(NULL);
    fullTime = *localtime(&currTime);
#endif

	dateTime.m_year = fullTime.tm_year + 1900;
	dateTime.m_month = fullTime.tm_mon + 1;
	dateTime.m_dayOfMonth = fullTime.tm_mday;
	dateTime.m_hour = fullTime.tm_hour;
	dateTime.m_minute = fullTime.tm_min;
	dateTime.m_second = fullTime.tm_sec;
}

#if defined(UE_WIN32) || defined(UE_X360)

void ueDateTime::ToDateTime(ueDateTime& dateTime, FILETIME t)
{
	SYSTEMTIME sysTime;
	if (!FileTimeToSystemTime(&t, &sysTime))
		return;

	dateTime.m_year = sysTime.wYear;
	dateTime.m_month = (u8) sysTime.wMonth;
	dateTime.m_dayOfMonth = (u8) sysTime.wDay;
	dateTime.m_hour = (u8) sysTime.wHour;
	dateTime.m_minute = (u8) sysTime.wMinute;
	dateTime.m_second = (u8) sysTime.wSecond;
}

#endif
/*
void ueDateTime::ToString(ueString& result)
{
	result.AssignV("%04d-%02d-%02d %02d:%02d:%02d", m_year, m_month, m_dayOfMonth, m_hour, m_minute, m_second);
}
*/

enum Field
{
	Field_Year = 0,
	Field_Month,
	Field_Day,
	Field_Hour,
	Field_Minute,
	Field_Second,

	Field_Unknown
};

void ueDateTime::ToString(char* buffer, u32 size, const char* format) const
{
	UE_ASSERT(ueStrLen(format) < size);

	char* b = buffer;
	const char* c = format;
	while (*c)
	{
		Field field = Field_Unknown;
		switch (*c)
		{
			case 'y': field = Field_Year; break;
			case 'M': field = Field_Month; break;
			case 'd': field = Field_Day; break;
			case 'h': field = Field_Hour; break;
			case 'm': field = Field_Minute; break;
			case 's': field = Field_Second; break;
		}

		if (field != Field_Unknown)
		{
			const u32 symbol = *c;
			u32 fieldLen = 1;
			c++;
			while ((u32) *c == symbol)
			{
				fieldLen++;
				c++;
			}

			UE_ASSERT(fieldLen <= 9);

			char numberFormat[5];
			numberFormat[0] = '%';
			numberFormat[1] = '0';
			numberFormat[2] = '0' + fieldLen;
			numberFormat[3] = 'u';
			numberFormat[4] = 0;

			char numberAsString[16];

			switch (field)
			{
				case Field_Year: ueStrFormatS(numberAsString, numberFormat, (u32) m_year); break;
				case Field_Month: ueStrFormatS(numberAsString, numberFormat, (u32) m_month); break;
				case Field_Day: ueStrFormatS(numberAsString, numberFormat, (u32) m_dayOfMonth); break;
				case Field_Hour: ueStrFormatS(numberAsString, numberFormat, (u32) m_hour); break;
				case Field_Minute: ueStrFormatS(numberAsString, numberFormat, (u32) m_minute); break;
				case Field_Second: ueStrFormatS(numberAsString, numberFormat, (u32) m_second); break;
			}

			const u32 numberAsStringLen = ueStrLen(numberAsString);
			for (u32 i = 0; i < numberAsStringLen; i++)
				*b++ = numberAsString[i];
		}
		else
			*b++ = *c++;
	}

	*b = 0;
}

const char* ueDateTime::ToString(const char* format) const
{
	static char buffer[64];
	ToString(buffer, UE_ARRAY_SIZE(buffer), format);
	return buffer;
}

ueBool ueDateTime::FromString(const char* str, const char* format)
{
	if (!str) return UE_FALSE;

	const char* fieldStart = NULL;
	Field field = Field_Unknown;
	while (*format && *str)
	{
		Field nextField = Field_Unknown;
		switch (*format)
		{
			case 'y': nextField = Field_Year; break;
			case 'M': nextField = Field_Month; break;
			case 'd': nextField = Field_Day; break;
			case 'h': nextField = Field_Hour; break;
			case 'm': nextField = Field_Minute; break;
			case 's': nextField = Field_Second; break;
		}

		if (field != nextField || !*(format + 1))
		{
			if (field != Field_Unknown)
			{
				u32 number = 0;
				ueStrScanf(fieldStart, "%u", &number);

				switch (field)
				{
					case Field_Year: m_year = number; break;
					case Field_Month: m_month = number; break;
					case Field_Day: m_dayOfMonth = number; break;
					case Field_Hour: m_hour = number; break;
					case Field_Minute: m_minute = number; break;
					case Field_Second: m_second = number; break;
				}
			}

			fieldStart = str;
		}

		field = nextField;

		++str;
		++format;
	}

	return UE_TRUE;
}

ueBool ueDateTime::operator < (const ueDateTime& other) const
{
	if (m_year != other.m_year)
		return m_year < other.m_year;
	if (m_month != other.m_month)
		return m_month < other.m_month;
	if (m_dayOfMonth != other.m_dayOfMonth)
		return m_dayOfMonth < other.m_dayOfMonth;
	if (m_hour != other.m_hour)
		return m_hour < other.m_hour;
	if (m_minute != other.m_minute)
		return m_minute < other.m_minute;
	return m_second < other.m_second;
}