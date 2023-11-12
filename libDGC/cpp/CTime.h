#pragma once

#include "CString.h"
#include <ctime>

class CTime {
private:
	std::time_t _time;

public:
	static CTime GetCurrentTime();
	CString Format(const char* format) const;
};