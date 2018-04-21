#include "Log.h"

CLog* CLog::m_pInstance = NULL;

CLog* CLog::GetInstance(bool bFile)
{
    if(m_pInstance == NULL)
    {
        m_pInstance = new CLog(bFile);
    }
    else
    {
        m_pInstance->SetWriteToFile(bFile);
    }
    
    return m_pInstance;
}

CLog::CLog(bool bFile)
{
    m_bFile = bFile;
}
CLog::~CLog()
{
    
}
void CLog::SetWriteToFile(bool bFile)
{
    m_bFile = bFile;
}

void CLog::INFO(const char* format, ...)
{
    va_list arglist;
    va_start(arglist, format);
    Log("INFO ", format, arglist);
    va_end(arglist);

}

void CLog::DEBUG(const char* format, ...)
{
    va_list arglist;
    va_start(arglist, format);
    Log("DEBUG", format, arglist);
    va_end(arglist);
}

void CLog::ERROR(const char* format, ...)
{
    va_list arglist;
    va_start(arglist, format);
    Log("ERROR", format, arglist);
    va_end(arglist);

}

void CLog::Log(const char* level,const char* format, va_list argList)
{
    char log[2048] = {0};
    char arglog[2000] = {0};
    vsprintf(arglog, format, argList);
    sprintf(log, "[%s] %s", level, arglog);

    char szTime[80]={0};
    sprintf(szTime, "%s", CFacility::GetCurTime());
	
    //Ð´ÈëÎÄ¼þ
    if(m_bFile)
    {
        FILE *logfile;
    	logfile = fopen( "mvpTest.log", "a+" );
    	if ( logfile != NULL ) 
    	{
    		fprintf( logfile, "[%s] %s\n", szTime, log);
    		fclose( logfile ); 
    	}
	}
    else
    {
        
    }
}

	

