/************************************************************************
Author:         wangdadian
Version:        1.0.0.1
Date:           2018-03-28
Description:	模拟登陆mvp并请求实时视频，记录切换结果，可设置并发数量。

History:                                                                
  1. Created by wangdadian on 2018-03-28
  2. 
**************************************************************************/

#include <stdarg.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <execinfo.h>
#include <fstream>
#include <sys/types.h>
#include <math.h>
#include <iostream>
#include <signal.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <dlfcn.h>

#include "Facility.h"
#include "defined.h"
#include "MvpSessionBuilder.h"
using namespace std;

//////////////////////////////////////////////////////////////////////////
//全局

// 程序是否退出标示
volatile bool g_bExit = false;	
// 是否daemon
volatile bool g_bDaemon = false;

// 用于打印输出重定向的描述符
FILE* g_fStdout;

//////////////////////////////////////////////////////////////////////////

void  PrintTrace (void)
{
	void *array[30];
	size_t size;
	char **strings;
	size_t i;

	size = backtrace (array, 30);
	strings = backtrace_symbols (array, size);

	char szTmp[1024]={0};
	char sDateNow[50] = {0};

	struct timeval tv;
	struct tm stm;
	gettimeofday(&tv,NULL);
	stm = *(localtime( &tv.tv_sec));
    sprintf( sDateNow, "%02d:%02d:%02d-%03ld", stm.tm_hour, stm.tm_min, stm.tm_sec,tv.tv_usec/1000);
 
	sprintf(szTmp,"Obtained %zd stack frames.", size);
	OPEN_STDOUT
	cout << szTmp << endl;
	CLOSE_STDOUT

	ofstream m_fLogFile;

	time_t t;
	time( &t );
	stm = *(localtime( &t ));

	char m_szLogFileName[256] = { 0 };
	sprintf(m_szLogFileName, "mvpTest_exception_%04d%02d%02d%02d%02d%02d.log", stm.tm_year+1900,
		    stm.tm_mon+1, stm.tm_mday, stm.tm_hour, stm.tm_min, stm.tm_sec );

	m_fLogFile.open( m_szLogFileName, ios_base::out);
	
	
	m_fLogFile << "********" << sDateNow << " Exception:" << endl; 
	m_fLogFile << szTmp << endl; 

	for (i = 0; i < size; i++)
	{
		sprintf (szTmp,"Stack #%02d, %s", (int)i, strings[i]);
		cout << szTmp << endl;	

		m_fLogFile << szTmp << endl; 
	}
	m_fLogFile.close();
	free(strings);
}

void OnExit( int sigid )
{
    // 程序退出
	g_bExit = true;
    OPEN_STDOUT
	switch( sigid )
    {
    	case SIGINT:
				cout << "\n####  exiting [interrupt]... ####\n" << endl;
            	break;
    	case SIGILL:
 				cout << "\n####  exiting [illegal instruction - invalid function image]...####\n" << endl;
				break;
    	case SIGFPE:
				cout << "####  exiting [floating point exception]...####\n" << endl;
				break;
    	case SIGSEGV:
                cout << "\n####  exiting [segment violation]...####\n" << endl;
				PrintTrace();
                exit(1);
				break;		
		case SIGABRT:
				cout << "\n####  exiting [Abort violation]...####\n" << endl;
				PrintTrace();
                exit(1);
				break;
    	case SIGQUIT:
				cout << "\n####  exiting [Software Quit ]...####\n" << endl;
				break;
    	case SIGTERM:
				cout << "\n####  exiting [Software termination signal from kill]... ####\n" << endl;
				break;
    	case SIGHUP:
				cout << "\n####  exiting [hang-up]... ####\n" << endl;
				break;
        case SIGTSTP:
                cout << "\n####  exiting [suspend]... ####\n" << endl;
                break;
		default:
				cout << "\n####  Unknown Signal[" << sigid<< "] ...####\n" << endl;
				break;
	}
    CLOSE_STDOUT
}
void restoreSignal()
{
//    signal(SIGKILL, SIG_DFL);
    signal(SIGILL,SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
	signal(SIGINT, SIG_DFL);
	signal(SIGFPE, SIG_DFL);
	signal(SIGSEGV, SIG_DFL);
	signal(SIGTERM, SIG_DFL);
	signal(SIGHUP, SIG_DFL);
	signal(SIGABRT, SIG_DFL);
	signal(SIGPIPE, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);
}
void initSignal()
{
    signal(SIGILL, OnExit);
    signal(SIGQUIT, OnExit);
	signal(SIGINT, OnExit);
	signal(SIGFPE, OnExit);
	signal(SIGSEGV, OnExit);
	signal(SIGTERM, OnExit);
//    signal(SIGHUP, OnExit);
    signal(SIGHUP, SIG_IGN);
	signal(SIGABRT, OnExit);
	signal(SIGPIPE, SIG_IGN);
    signal(SIGTSTP, OnExit);
}

// 打印帮助
int PrintUsage()
{
    printf("\nUsage: \n");
    printf("    -f [filename]   set config file(.xml).The defalut file is config.xml in the current directory\n");
    //printf("    -n [seconds]    determines the refresh interval of the display in seconds[3-60]. The default value of interval is 5.\n");
    printf("    -v              show version information\n");
    return 0;
}

// 打印版本信息
int PrintVersion()
{
    // 程序名称
    //printf("Name      :  %s\n", MODULENAME);
    // 版本号
#ifdef VERSION
    char version[64] = {0};
    sprintf(version, "%s", VERSION);
    printf("Version   :  %s\n", version);
#endif //VERSION

#ifdef BUILDDATE
    // Build日期
    char szBuild[64] = {0};
    sprintf(szBuild, "%s", BUILDDATE);
    printf("Build Date:  %s\n", szBuild);
#endif //BUILDDATE

    return 0;
}

//////////////////////////////////////////////////////////////////////////
// main 函数
//////////////////////////////////////////////////////////////////////////

int main(int argc,char **argv)
{
    char szCfgFile[256] = "./config.xml";
    unsigned int uiInterval = 5;
    // 解析参数
    if ( argc > 1 )
    {
        // 从第2个参数开始，第一个参数为命令
        int index = 1;
        for ( index = 1; index < argc; index++ )
        {
            // config file
            if ( strcmp(argv[index], "-f") == 0)
            {
                sprintf(szCfgFile, "%s", argv[++index]);
            }
            else if ( strcmp (argv[index], "-n") == 0)
            {
                int iValue = atoi(argv[++index]);
                if(iValue<1) iValue = 3;
                if(iValue>60) iValue = 60;
                uiInterval = iValue;
            }
            // version
            else if ( strcmp (argv[index], "-v") == 0)
            {
                PrintVersion();
                exit(0);
            }
            else // error
            {
                PrintUsage();
                exit(1);
            }
        }
    }

    // 重定向标准输出、错误输出，即屏蔽输出打印
    /*
    int fd1_stdout, fd2_stderr;
    close(1);// stdout
    close(2);// stderr
    // 使1、2定向到/dev/null
    fd1_stdout  = open("/dev/null", O_RDWR);
    fd2_stderr = dup(fd1_stdout);
    */
    initSignal();

    // 启动服务
    CMvpSessionBuilder *pServer = new CMvpSessionBuilder(szCfgFile);
    if(pServer == NULL)
    {
        OPEN_STDOUT
        printf("new memory failed!\n");
        CLOSE_STDOUT
        restoreSignal();
        return 1;
    }
    OPEN_STDOUT
    printf("Waiting for start...\n");
    CLOSE_STDOUT
    if( pServer->Start() != 0)
    {
        OPEN_STDOUT
        printf("start failed!\n");
        CLOSE_STDOUT
        restoreSignal();
        return 2;
    }
    
	while ( g_bExit == false )
	{
		CFacility::SleepMsec(1000);
	}
    
    restoreSignal();

    pServer->Stop();
    delete pServer;
    pServer = NULL;
    
    OPEN_STDOUT
    printf("#### Exit! ####\n");
    CLOSE_STDOUT
	return 0;
}


