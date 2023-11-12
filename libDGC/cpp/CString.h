#pragma once

#include <string>

class CString : public std::string {
public:
	CString();
	CString(const char* str);
	operator char* () const;
    operator const char* () const;
	void ChangeSuffix(std::string newSuffix);
	void Format(const char* inFormat, ...);
	bool LoadString(unsigned int id);
	int GetLength() const;
};
