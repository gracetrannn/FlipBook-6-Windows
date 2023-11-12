#if defined(IMPLEMENT_WINDOWS_APIS) || defined(__APPLE__)

#include "CTime.h"
#include <chrono>

using sys_clock = std::chrono::system_clock;

CTime CTime::GetCurrentTime() {
	CTime time;
	time._time = sys_clock::to_time_t(sys_clock::now());
	return time;
}

CString CTime::Format(const char* format) const
{
	char buffer[1024] = { 0 };
	std::tm* ptm = std::localtime(&_time);
	std::strftime(buffer, 1024, format, ptm);
	return CString(buffer);
}

#endif // IMPLEMENT_WINDOWS_APIS
