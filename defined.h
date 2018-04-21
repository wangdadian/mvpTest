#ifndef DEFINED_H_2018
#define DEFINED_H_2018
//#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <stdint.h>
#include <time.h>

#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <unistd.h>
#include <stdarg.h>
#include <signal.h>
#include "datatype.h"

extern bool g_bDebugToFile;
extern bool g_bDebugToConsole;
extern FILE* g_fStdout;
// 直播成功后未收到码流的超时时间，秒
#define TIMEOUT_LIVE_NOSTREAM 10

// 请求后未收到返回的超时时间，秒
#define TIMEOUT_RECV_CMD     5

// 获取变量的名称
#define varName(x) #x

enum LOG_LEVEL
{
	INFO  = 1,
	DEBUG,
	ERROR		
};
enum STATE_TYPE
{
	STATE_NONE=0, // 未向MVP发送请求指令
	STATE_OK=1, //已向mvp发送请求指令，并返回成功
	STATE_WAITING=2, // 已向mvp发送请求指令，等待返回
	STATE_FAILED=3, // 调用API接口失败
};


enum STATUS_CMD
{
	SC_NONE=0, // 未请求
	SC_OK=1, // mvp返回信令告知请求成功
	SC_FAILED=2, // mvp返回信令告知请求失败
	SC_WAITING=3, // 已发送请求指令，等待接收mvp返回
	SC_TIMEOUT=4, // 已发送请求指令，超时未收到返回
	SC_ERROR=5, // 调用请求API失败
};

enum STATUS_LIVE
{
	LIVE_NONE=0, // 初始状态，STATUS_CMD不为SC_OK时的状态
	LIVE_OK=1, // 直播成功，并已收到码流
	LIVE_TIMEOUT=2, // 直播成功后，超时未收到流
	LIVE_WAITING=3, // 直播成功后，正在等待收流(收流超时之前的状态)
};


#pragma pack(1)


typedef struct _mvpClientInfo
{
	bool bScan; // 是否巡检, 轮询时不会针对直播失败的自动重新切换另一路相机（轮巡时忽略bRStartLive参数）
	UINT32 uiScanTimeInterval; // 巡检间隔时间(bScan为true时生效)，单位秒8-3600
	bool bReStartLive;// 针对直播失败的直播业务是否重新请求另一路直播，true为重新请求，其他否
	bool bRandomSwitch;// 是否随机切换相机列表中的相机
	char szUser[80]; // 登录用户名
	char szPass[80]; // 登录密码
	UINT32 uiMaxWndCnt; // 最大窗口数量
	UINT64 ubiWndIndex; // 窗口编号起点数，用于累加+1
	bool bReLogin; // 断开后是否重连mvp
	UINT32 uiLoginTime; // 最新成功登录时间
	UINT32 uiFirstLoginTime; // 第一次成功登录时间
	UINT32 uiLoginCount;// 重新登录成功的次数
}sClientInfoT;

typedef struct _MvpSessionInfo
{
	char szIP[80]; // mvp IP地址
	UINT32 uiPort; // mvp 端口
	UINT32 uiClients; // client数量
	sClientInfoT* pClients; // client信息
}sMvpSessionInfoT;

typedef struct __CameraInfoT
{
	UINT64 ubiCamID;
	bool bStrmMvpChecked; // mvp是否自检过，true.，自检，false，未自检
	bool bStrmMvpCheckedOK; // mvp自检结果，在线与否
	bool bVideoOK;
}CameraInfoT;

typedef struct __CameraStatusInfo
{
	UINT64 ubiCamID;
	bool bChecked;
	bool bCheckedOK;
	UINT32 uiAccessMode;//0-未设置，1-自建，2-基层接入，3-内部接入，4-社会接入，5-互联网接入，6-其他接入 
	bool bVideoOK;
}CameraStatusInfoT;


// 直播状态
typedef struct __LiveStatus
{
	// 相机ID
	UINT64 ubiCamID;
	bool bStrmMvpChecked; //mvp是否自检过， true.，自检，false，未自检
	bool bStrmMvpCheckedOK; // mvp自检结果，在线与否
	bool bMvpVideoOK; // mvp返回的码流状态
	bool bStrmMyCheckedOK; //我的自检，通过判断有无收到流
	// 窗口ID
	UINT64 ubiWndID;
	// 码流速率 单位KB/S
	float fStreamRate; 
	//请求直播的状态
	STATUS_CMD statusStartLive;
	// 码流直播状态
	STATUS_LIVE statusLive;
	// 请求直播至收到返回信令耗时(ms)，-1表示未收到返回或者未开始请求直播
	INT32 iRecvCmdTime;
	// 收到直播返回成功至收到码流耗时ms，-1表示未收到流
	INT32 iRecvStreamTime;
}LiveStatusT;

// 请求直播后的状态计数统计
typedef struct __LiveStatusCountInfo
{
	// 直播总路数，uiLiveCount = uiSLOKCnt + uiSLNoneCnt + uiSLFailCnt + uiSLWaitingCnt + uiSLTimeoutCnt + uiSLErrorCnt
	UINT32 uiLiveCount; 

	/*
	 * 请求直播的状态路数
	 */
	// 未开始直播直播路数
	UINT32 uiSLNoneCnt;
	// 已请求直播，且mvp返回成功的路数
	UINT32 uiSLOKCnt;
	// 已请求直播，但mvp返回失败的路数
	UINT32 uiSLFailCnt;
	// 已开始请求直播，等待接收信令的路数，尚未超时
	UINT32 uiSLWaitingCnt;
	// 已请求直播，但超时未收到mvp返回信令的路数
	UINT32 uiSLTimeoutCnt;
	// 调用API接口VasAPI_StartLive失败的路数
	UINT32 uiSLErrorCnt;

	// mvp自检成功且在线但本程序未收到流的路数
	UINT32 uiMvpOkButNotOKCnt;

	/*
	 * 请求直播并且mvp返回成功后，码流接收状态，uiSLOKCnt = uiLiveWaitingCnt + uiLiveOKCnt + uiLiveTimeoutCnt
	 */
	// 请求直播成功后，等待接收码流的路数，尚未超时中
	UINT32 uiLiveWaitingCnt;
	/// 请求直播成功后，成功收到流的路数
	UINT32 uiLiveOKCnt;
	// 请求直播成功后，超时未收到流的路数
	UINT32 uiLiveTimeoutCnt;
}LiveStatusCountInfoT;

// 巡检状态
typedef struct __ScanStatusInfo
{
	// 已巡检次数
	UINT32 uiScanTimes;
	// 所有已巡检的直播业务状态统计
	LiveStatusCountInfoT sLiveStausCntInfo;
}ScanStatusInfoT;

// 客户端状态
typedef struct __ClientStatus
{
	// 登录信息
	bool bLogin;
	UINT32 uiLoginTime; // 最新成功登录时间
	UINT32 uiFirstLoginTime; // 第一次成功登录时间
	UINT32 uiLoginCount;// 重新登录成功的次数
	
	// 用户信息
	char szUser[80];
	char szPass[80];
	// 巡检状态
	bool bScan;
	// 巡检间隔，秒
	UINT32 uiScanTimeSpan;
	bool bReStartLive;
	bool bRandomSwitch;// 是否随机切换相机列表中的相机
	
	/*
	 *  当前直播状态个数统计信息
	 */
	LiveStatusCountInfoT sLiveStausCntInfo;
	/*
	 * 当前巡检状态统计信息
	 */
	ScanStatusInfoT sScanStatusInfo;
	
	// 总体码流速率 单位KB/S
	float fStreamRate; 
	
	// 直播状态个数
	UINT32 uiCount;	
	// 直播状态信息
	LiveStatusT* pLiveStatusData;
}ClientStatusT;

// mvp会话状态
typedef struct __MvpSessionStatus
{
	// 会话状态
	bool bOK;
	char szIP[80]; // mvp IP地址
	UINT32 uiPort; // mvp 端口
	
	/*
	 * 客户端状态信息
	 */
	 
	/*
	 *  当前直播状态个数统计信息
	 */
	LiveStatusCountInfoT sLiveStausCntInfo;

	/*
	 * 当前巡检状态统计信息
	 */
	ScanStatusInfoT sScanStatusInfo;
	
	// 总体码流速率 单位KB/S
	float fStreamRate; 
	
	// 客户端状态个数
	UINT32 uiCount;
	ClientStatusT* pClientStatusData;
}MvpSessionStatusT;

// 整体状态
typedef struct __MvpStatus
{
	/*
	 * mvp会话状态信息
	 */
	/*
	 *  当前直播状态个数统计信息
	 */
	LiveStatusCountInfoT sLiveStausCntInfo;

	/*
	 * 当前巡检状态统计信息
	 */
	ScanStatusInfoT sScanStatusInfo;
	
	// 总体码流速率 单位KB/S
	float fStreamRate; 
	
	// 客户端总数
	UINT32 uiClientCount;

	// mvp会话个数
	UINT32 uiCount;
	MvpSessionStatusT* pMvpSessionStatusData;
}MvpStatusT;


#pragma pack()


#ifndef OPEN_STDOUT
// 会出问题，导致关闭其他的描述符，尚未解决 20180403
//#define OPEN_STDOUT {g_fStdout=freopen( "/dev/tty","w",stdout);}
#define OPEN_STDOUT 
#endif

#ifndef CLOSE_STDOUT
//#define CLOSE_STDOUT {if(g_fStdout!=NULL){fclose(g_fStdout);fclose(stdout);}}
#define CLOSE_STDOUT 
#endif

#ifndef _DEBUG_
#define _DEBUG_(...) \
	do { \
		if(g_bDebugToConsole==false&&g_bDebugToFile==false)\
		{ \
			break; \
		} \
		char _log_szBuf[2048]={0}; \
		struct timeval tv; \
		struct tm stm; \
		struct tm stmres; \
		gettimeofday(&tv,NULL); \
		stm=*(localtime_r(&tv.tv_sec,&stmres)); \
		snprintf(_log_szBuf,sizeof(_log_szBuf),__VA_ARGS__); \
		if(g_bDebugToConsole) \
		{ \
			char szNow[32]={0};\
			sprintf(szNow,"%04d-%02d-%02d %02d:%02d:%02d:%06ld",stm.tm_year+1900,stm.tm_mon+1,stm.tm_mday,stm.tm_hour,stm.tm_min,stm.tm_sec,tv.tv_usec); \
			OPEN_STDOUT \
			printf("[_DEBUG_] [%s] [%s:%d %s] %s\n",szNow,__FILE__,__LINE__,__FUNCTION__,_log_szBuf); \
			CLOSE_STDOUT \
		} \
		if(g_bDebugToFile) \
        { \
    		FILE *logfile; \
    		char szFile[80]={0};\
    		char szTime[32]={0}; \
    		sprintf(szFile,"mvpTest_%04d%02d%02d.log",stm.tm_year+1900,stm.tm_mon+1,stm.tm_mday); \
    		sprintf(szTime,"%02d:%02d:%02d:%06ld",stm.tm_hour,stm.tm_min,stm.tm_sec,tv.tv_usec); \
    		logfile=fopen(szFile, "a+"); \
    		if(logfile!=NULL) \
    		{ \
    			fprintf(logfile,"[_DEBUG_] [%s] [%s:%d %s] %s\n",szTime,__FILE__,__LINE__,__FUNCTION__,_log_szBuf); \
    			fclose(logfile); \
    		} \
        } \
	} while(0)
#endif

#ifndef _ERROR_
#define _ERROR_(...) \
	do { \
		if(g_bDebugToConsole==false&&g_bDebugToFile==false)\
		{ \
			break; \
		} \
		char _log_szBuf[2048]={0}; \
		struct timeval tv; \
		struct tm stm; \
		struct tm stmres; \
		gettimeofday(&tv,NULL); \
		stm=*(localtime_r(&tv.tv_sec,&stmres)); \
		snprintf(_log_szBuf,sizeof(_log_szBuf),__VA_ARGS__); \
		if(g_bDebugToConsole) \
		{ \
			char szNow[32]={0};\
			sprintf(szNow,"%04d-%02d-%02d %02d:%02d:%02d:%06ld",stm.tm_year+1900,stm.tm_mon+1,stm.tm_mday,stm.tm_hour,stm.tm_min,stm.tm_sec,tv.tv_usec); \
			OPEN_STDOUT \
			printf("[_ERROR_] [%s] [%s:%d %s] %s\n",szNow,__FILE__,__LINE__,__FUNCTION__,_log_szBuf); \
			fflush(stdout); \
			CLOSE_STDOUT \
		} \
		if(g_bDebugToFile) \
        { \
    		FILE *logfile; \
    		char szFile[80]={0};\
    		char szTime[32]={0}; \
    		sprintf(szFile,"mvpTest_%04d%02d%02d.log",stm.tm_year+1900,stm.tm_mon+1,stm.tm_mday); \
    		sprintf(szTime,"%02d:%02d:%02d:%06ld",stm.tm_hour,stm.tm_min,stm.tm_sec,tv.tv_usec); \
    		logfile=fopen(szFile,"a+"); \
    		if (logfile!=NULL) \
    		{ \
    			fprintf(logfile,"[_ERROR_] [%s] [%s:%d %s] %s\n",szTime,__FILE__,__LINE__,__FUNCTION__,_log_szBuf); \
    			fclose(logfile); \
    		} \
        } \
	} while(0)
#endif


#ifndef _INFO_
#define _INFO_(...) \
	do { \
		if(g_bDebugToConsole==false)\
		{ \
			break; \
		} \
		char szNow[32]={0};\
		char _log_szBuf[2048]={0}; \
		struct timeval tv; \
		struct tm stm; \
		struct tm stmres; \
		gettimeofday(&tv,NULL); \
		stm=*(localtime_r(&tv.tv_sec,&stmres)); \
		sprintf(szNow,"%04d-%02d-%02d %02d:%02d:%02d:%06ld",stm.tm_year+1900,stm.tm_mon+1,stm.tm_mday,stm.tm_hour,stm.tm_min,stm.tm_sec,tv.tv_usec); \
		snprintf(_log_szBuf,sizeof(_log_szBuf),__VA_ARGS__); \
		OPEN_STDOUT \
		printf("[_INFO_] [%s] [%s:%d %s] %s\n",szNow,__FILE__,__LINE__,__FUNCTION__,_log_szBuf); \
		fflush(stdout); \
		CLOSE_STDOUT \
	} while(0)
#endif


#endif


