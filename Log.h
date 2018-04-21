#ifndef LOG_H_2018
#define LOG_H_2018

#include "Facility.h"
#include "defined.h"


class CLog
{
public:
	static CLog* GetInstance(bool bFile=true);	
	void SetWriteToFile(bool bFile);	
	void INFO(const char* format, ...);
	void DEBUG(const char* format, ...);
	void ERROR(const char* format, ...);
private:
	CLog(bool bFile=true);
	~CLog();
	void Log(const char* level,const char* format, va_list argList);
private:
	static CLog *m_pInstance;
	bool m_bFile;
};

#endif

