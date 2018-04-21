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
// ֱ���ɹ���δ�յ������ĳ�ʱʱ�䣬��
#define TIMEOUT_LIVE_NOSTREAM 10

// �����δ�յ����صĳ�ʱʱ�䣬��
#define TIMEOUT_RECV_CMD     5

// ��ȡ����������
#define varName(x) #x

enum LOG_LEVEL
{
	INFO  = 1,
	DEBUG,
	ERROR		
};
enum STATE_TYPE
{
	STATE_NONE=0, // δ��MVP��������ָ��
	STATE_OK=1, //����mvp��������ָ������سɹ�
	STATE_WAITING=2, // ����mvp��������ָ��ȴ�����
	STATE_FAILED=3, // ����API�ӿ�ʧ��
};


enum STATUS_CMD
{
	SC_NONE=0, // δ����
	SC_OK=1, // mvp���������֪����ɹ�
	SC_FAILED=2, // mvp���������֪����ʧ��
	SC_WAITING=3, // �ѷ�������ָ��ȴ�����mvp����
	SC_TIMEOUT=4, // �ѷ�������ָ���ʱδ�յ�����
	SC_ERROR=5, // ��������APIʧ��
};

enum STATUS_LIVE
{
	LIVE_NONE=0, // ��ʼ״̬��STATUS_CMD��ΪSC_OKʱ��״̬
	LIVE_OK=1, // ֱ���ɹ��������յ�����
	LIVE_TIMEOUT=2, // ֱ���ɹ��󣬳�ʱδ�յ���
	LIVE_WAITING=3, // ֱ���ɹ������ڵȴ�����(������ʱ֮ǰ��״̬)
};


#pragma pack(1)


typedef struct _mvpClientInfo
{
	bool bScan; // �Ƿ�Ѳ��, ��ѯʱ�������ֱ��ʧ�ܵ��Զ������л���һ·�������Ѳʱ����bRStartLive������
	UINT32 uiScanTimeInterval; // Ѳ����ʱ��(bScanΪtrueʱ��Ч)����λ��8-3600
	bool bReStartLive;// ���ֱ��ʧ�ܵ�ֱ��ҵ���Ƿ�����������һ·ֱ����trueΪ��������������
	bool bRandomSwitch;// �Ƿ�����л�����б��е����
	char szUser[80]; // ��¼�û���
	char szPass[80]; // ��¼����
	UINT32 uiMaxWndCnt; // ��󴰿�����
	UINT64 ubiWndIndex; // ���ڱ��������������ۼ�+1
	bool bReLogin; // �Ͽ����Ƿ�����mvp
	UINT32 uiLoginTime; // ���³ɹ���¼ʱ��
	UINT32 uiFirstLoginTime; // ��һ�γɹ���¼ʱ��
	UINT32 uiLoginCount;// ���µ�¼�ɹ��Ĵ���
}sClientInfoT;

typedef struct _MvpSessionInfo
{
	char szIP[80]; // mvp IP��ַ
	UINT32 uiPort; // mvp �˿�
	UINT32 uiClients; // client����
	sClientInfoT* pClients; // client��Ϣ
}sMvpSessionInfoT;

typedef struct __CameraInfoT
{
	UINT64 ubiCamID;
	bool bStrmMvpChecked; // mvp�Ƿ��Լ����true.���Լ죬false��δ�Լ�
	bool bStrmMvpCheckedOK; // mvp�Լ������������
	bool bVideoOK;
}CameraInfoT;

typedef struct __CameraStatusInfo
{
	UINT64 ubiCamID;
	bool bChecked;
	bool bCheckedOK;
	UINT32 uiAccessMode;//0-δ���ã�1-�Խ���2-������룬3-�ڲ����룬4-�����룬5-���������룬6-�������� 
	bool bVideoOK;
}CameraStatusInfoT;


// ֱ��״̬
typedef struct __LiveStatus
{
	// ���ID
	UINT64 ubiCamID;
	bool bStrmMvpChecked; //mvp�Ƿ��Լ���� true.���Լ죬false��δ�Լ�
	bool bStrmMvpCheckedOK; // mvp�Լ������������
	bool bMvpVideoOK; // mvp���ص�����״̬
	bool bStrmMyCheckedOK; //�ҵ��Լ죬ͨ���ж������յ���
	// ����ID
	UINT64 ubiWndID;
	// �������� ��λKB/S
	float fStreamRate; 
	//����ֱ����״̬
	STATUS_CMD statusStartLive;
	// ����ֱ��״̬
	STATUS_LIVE statusLive;
	// ����ֱ�����յ����������ʱ(ms)��-1��ʾδ�յ����ػ���δ��ʼ����ֱ��
	INT32 iRecvCmdTime;
	// �յ�ֱ�����سɹ����յ�������ʱms��-1��ʾδ�յ���
	INT32 iRecvStreamTime;
}LiveStatusT;

// ����ֱ�����״̬����ͳ��
typedef struct __LiveStatusCountInfo
{
	// ֱ����·����uiLiveCount = uiSLOKCnt + uiSLNoneCnt + uiSLFailCnt + uiSLWaitingCnt + uiSLTimeoutCnt + uiSLErrorCnt
	UINT32 uiLiveCount; 

	/*
	 * ����ֱ����״̬·��
	 */
	// δ��ʼֱ��ֱ��·��
	UINT32 uiSLNoneCnt;
	// ������ֱ������mvp���سɹ���·��
	UINT32 uiSLOKCnt;
	// ������ֱ������mvp����ʧ�ܵ�·��
	UINT32 uiSLFailCnt;
	// �ѿ�ʼ����ֱ�����ȴ����������·������δ��ʱ
	UINT32 uiSLWaitingCnt;
	// ������ֱ��������ʱδ�յ�mvp���������·��
	UINT32 uiSLTimeoutCnt;
	// ����API�ӿ�VasAPI_StartLiveʧ�ܵ�·��
	UINT32 uiSLErrorCnt;

	// mvp�Լ�ɹ������ߵ�������δ�յ�����·��
	UINT32 uiMvpOkButNotOKCnt;

	/*
	 * ����ֱ������mvp���سɹ�����������״̬��uiSLOKCnt = uiLiveWaitingCnt + uiLiveOKCnt + uiLiveTimeoutCnt
	 */
	// ����ֱ���ɹ��󣬵ȴ�����������·������δ��ʱ��
	UINT32 uiLiveWaitingCnt;
	/// ����ֱ���ɹ��󣬳ɹ��յ�����·��
	UINT32 uiLiveOKCnt;
	// ����ֱ���ɹ��󣬳�ʱδ�յ�����·��
	UINT32 uiLiveTimeoutCnt;
}LiveStatusCountInfoT;

// Ѳ��״̬
typedef struct __ScanStatusInfo
{
	// ��Ѳ�����
	UINT32 uiScanTimes;
	// ������Ѳ���ֱ��ҵ��״̬ͳ��
	LiveStatusCountInfoT sLiveStausCntInfo;
}ScanStatusInfoT;

// �ͻ���״̬
typedef struct __ClientStatus
{
	// ��¼��Ϣ
	bool bLogin;
	UINT32 uiLoginTime; // ���³ɹ���¼ʱ��
	UINT32 uiFirstLoginTime; // ��һ�γɹ���¼ʱ��
	UINT32 uiLoginCount;// ���µ�¼�ɹ��Ĵ���
	
	// �û���Ϣ
	char szUser[80];
	char szPass[80];
	// Ѳ��״̬
	bool bScan;
	// Ѳ��������
	UINT32 uiScanTimeSpan;
	bool bReStartLive;
	bool bRandomSwitch;// �Ƿ�����л�����б��е����
	
	/*
	 *  ��ǰֱ��״̬����ͳ����Ϣ
	 */
	LiveStatusCountInfoT sLiveStausCntInfo;
	/*
	 * ��ǰѲ��״̬ͳ����Ϣ
	 */
	ScanStatusInfoT sScanStatusInfo;
	
	// ������������ ��λKB/S
	float fStreamRate; 
	
	// ֱ��״̬����
	UINT32 uiCount;	
	// ֱ��״̬��Ϣ
	LiveStatusT* pLiveStatusData;
}ClientStatusT;

// mvp�Ự״̬
typedef struct __MvpSessionStatus
{
	// �Ự״̬
	bool bOK;
	char szIP[80]; // mvp IP��ַ
	UINT32 uiPort; // mvp �˿�
	
	/*
	 * �ͻ���״̬��Ϣ
	 */
	 
	/*
	 *  ��ǰֱ��״̬����ͳ����Ϣ
	 */
	LiveStatusCountInfoT sLiveStausCntInfo;

	/*
	 * ��ǰѲ��״̬ͳ����Ϣ
	 */
	ScanStatusInfoT sScanStatusInfo;
	
	// ������������ ��λKB/S
	float fStreamRate; 
	
	// �ͻ���״̬����
	UINT32 uiCount;
	ClientStatusT* pClientStatusData;
}MvpSessionStatusT;

// ����״̬
typedef struct __MvpStatus
{
	/*
	 * mvp�Ự״̬��Ϣ
	 */
	/*
	 *  ��ǰֱ��״̬����ͳ����Ϣ
	 */
	LiveStatusCountInfoT sLiveStausCntInfo;

	/*
	 * ��ǰѲ��״̬ͳ����Ϣ
	 */
	ScanStatusInfoT sScanStatusInfo;
	
	// ������������ ��λKB/S
	float fStreamRate; 
	
	// �ͻ�������
	UINT32 uiClientCount;

	// mvp�Ự����
	UINT32 uiCount;
	MvpSessionStatusT* pMvpSessionStatusData;
}MvpStatusT;


#pragma pack()


#ifndef OPEN_STDOUT
// ������⣬���¹ر�����������������δ��� 20180403
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


