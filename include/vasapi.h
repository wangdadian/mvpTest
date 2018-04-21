#ifndef VAS_API_H_
#define VAS_API_H_
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the VASAPI_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// VASAPI_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.

#ifdef _WIN32
#include "stdafx.h" 
#define STDCALL  STDCALL

#ifdef VASAPI_EXPORTS
#define VASAPI_API extern "C" __declspec(dllexport) 
#else
#define VASAPI_API extern "C" __declspec(dllimport) 
#endif

#else
#define VASAPI_API 
#define STDCALL 

#include "datatype.h"
#endif

typedef void  ( STDCALL *ReturnCallback)( void * /*handle*/, char * /*szXmlRet*/, UINT32 /*uiMessageType*/ , void * /*body*/, void * /*pCaller*/ );
typedef void  ( STDCALL *LargeDataCallback)( void * /*handle*/,   UINT32 /*uiDataType*/, void * /*pData*/, UINT32 /*uiLen*/, void * /*pCaller*/ );
typedef void  ( STDCALL *StreamCallback)( void * /*handle*/, UINT64 /*ubiCameraId*/, UINT32 /*uiDataType*/, UINT64 /*ubiSessionId*/, void * /*pData*/, UINT32 /*uiCount*/, UINT64 /*ubiTimeStamp*/, void * /*pCaller*/);
typedef void  ( STDCALL *DisconnectCallback)( void * /*handle*/, void * /*pCaller*/ );
// 直播码流回调
typedef void ( STDCALL *LiveStreamCallback)(UINT64 /*ubiCameraId*/, UINT64 /*ubiWndId*/, void * /*pData*/, UINT32 /*uiLen*/, void * /*pCaller*/);

enum eLargeDataType
{
	LData_RetCamera, 
	LData_RetCameraStatus,
};


//协议
enum eProtocolType
{
	//基本协议, catelog = 0
	RET_USER_LOGIN = 0x0002,
	RET_USER_LOGOUT = 0x0006,

	//business 协议
	RET_START_LIVE = 0x0605,
	RET_STOP_LIVE = 0x0607,
	RET_GET_CAMERA = 0x061E,
};

//码流格式
enum eCODEC_VIDEO_FORMAT 
{
	UNKNOWN_VIDEO = 0,
	MPEG2_VIDEO = 1,		//1: Mpeg2
	MPEG4_VIDEO,			//2: Mpeg4
	H264_VIDEO,				//3: H.264
	MJPG_VIDEO,				//4: MJPG
	JPEG2000_VIDEO,			//5: JPEG2000
	HK_H264_VIDEO,			//6: HK_H264 海康h264
	HD_H246_VIDEO,			//7: HD_H246 高清h264

	DH_H264_VIDEO,          //8: DH_H264 大华h264


	SK_H264_VIDEO,          //10: sikeyuan h.264格式
	SK_JPEG_VIDEO, 			//11: sikeyuan jpeg 格式
	UNIVERSAL_VIDEO = 0xFF,        //9: 万能视频格式
};
// 分辨率
enum eCODEC_RESOLUTION_TYPE
{
	RES_CIF = 1, 
	RES_4CIF,
	RES_Half_D1,
	RES_D1,
	RES_720P,
	RES_1080P,
	RES_UXGA	

};
// 码流封装格式
enum eCODEC_VIDEO_Head
{
	ES_Head_Flag = 0,            //裸码流
	RTP_Head_Flag = 1,           //RTP头
	PS_Head_Flag = 2,			 // PS头
	TS_Head_Flag = 4,			 // TS头
	BOCOM_Head_Flag = 8,		 // BOCOM头
};

#pragma pack(1)

typedef int ARRAY_COUNT;

typedef struct __CommonRet
{
	INT32 iValue;	//返回类型 0为正确，非零表示错误代码
	CHAR description[50];//错误描述
}CommonRet;

typedef struct __RetUserLogin
{
	CommonRet ret;
	CHAR szVersion[32];		//服务端版本号
	CHAR szBuildData[32];	//服务端编译时间
	UINT64 ubiUserId;	//用户逻辑id
	CHAR szUserName[20];	//用户名
	UINT32 uiPresetStart;
	UINT32 uiPresetEnd;
	UINT64 ubiPrivilege;
	UINT32 uiMaxWnd;
	UINT32 uiClientTimeOut;
	CHAR szUrlMap[256];
	CHAR szUrlNowStatus[256];
	CHAR szUrlHistoryStatus[256];
	CHAR szUrlShareList[256];
	CHAR szUrlDevRegister[256];
	CHAR szUrlReserved1[256];
	CHAR szUrlReserved2[256];
	CHAR szUrlReserved3[256];
	CHAR szUrlReserved4[256];
	CHAR szUrlReserved5[256];
	CHAR szRealName[120];
	CHAR szDeptName[120];
	CHAR szPresetBlock[200];
	CHAR szNodeName[100];	

}RetUserLogin;

typedef struct __RetUserLogout
{
	CommonRet ret;
}RetUserLogout;

typedef struct __RetGetCamera
{	
	CHAR szSync[5];
	ARRAY_COUNT uiCount;
	CHAR *szData;	
}RetGetCamera;

typedef struct __CameraRes
{
	UINT64 ubiCameraGroupId;
	UINT64 ubiCameraId;
	CHAR   szName[100];
	UINT32 uiPtzAble;
	CHAR szKeyword[256];

}CameraRes;

typedef struct _RetStartLive
{
	CommonRet ret;
	UINT64 ubiCameraId;
	UINT64 ubiWndId;
	UINT32 uiCodecRate;
	UINT32 uiVideoFormat;
	UINT32 uiCodecType;
	UINT32 uiFPS;
	UINT32 uiResloution;
	CHAR   uiPlayType;
	CHAR   szPlayAddr[20];
	UINT32 uiPlayPort;
	CHAR   szEncAddr[20];

	/*CommonRet ret;
	UINT64 ubiCameraId;		//摄像机编号
	UINT64 ubiWndId;		//窗口号
	UINT32 uiCodecRate;
	UINT32 uiVideoFormat;	//编码器类型
	UINT32 uiCodecType;		//码流类型
	UINT32 uiFPS;			//视频码率
	UINT32 uiResloution;	//视频分辨率
	CHAR   uiPlayType;		//网络视频播放模式
	CHAR   szPlayAddr[20];	//网络视频播放IP地址
	UINT32 uiPlayPort;		//网络视频播放端口
	CHAR   szEncAddr[20];	//摄像机IP地址
	*/
}RetStartLive;

typedef struct _RetStopLive
{
	CommonRet ret;
	UINT64 ubiCameraId;
	UINT64 ubiWndId;
}RetStopLive;

typedef struct __CameraStatusRes
{
	UINT64 ubiCamraId;
	CHAR bLocked;
	CHAR szLockedBy[20];
	UINT32 uiLockTime;
	CHAR bRecordFlag;
	CHAR bRecordStatus;
	CHAR bGuardFlag;
	CHAR bVideoLost; // 0-在线
	CHAR bStrmCheckFlag; // 1.，自检，0，未自检
	UINT32 uiStrmCheckTime;
	INT32 iStrmCheckResult; // 0成功，非零为错误码
	UINT32 uiAccessMode;// 
	CHAR szLoginUser[32]; 
	CHAR szLoginPassword[32]; 
	CHAR szCheckResultDesc[256];
	CHAR szCameraIP[32];//bt0904
	CHAR szGisCode[100];//bt0904
	CHAR szPreOneDayStorBreak[64];//bt0904
	CHAR szPreThreeDayStorBreak[64];//bt0904
	CHAR szPreSevenDayStorBreak[64];//bt0904
}CameraStatusRes;


typedef struct __RetGetCameraStatus
{
	CHAR szSync[5];
	ARRAY_COUNT uiCount;
	CHAR *szData;
}RetGetCameraStatus;


#pragma pack()


VASAPI_API HANDLE  VasAPI_Initialize(void *pCaller);
VASAPI_API int  VasAPI_Uninitialize(HANDLE lInstance);
VASAPI_API int  VasAPI_ConnectToServer(HANDLE lInstance, char *szServerAddr, unsigned short uiPlayPort);

VASAPI_API int  VasAPI_UserLogin(HANDLE lInstance, char *szUsername, char *szPasswd);
VASAPI_API int  VasAPI_UserLogout(HANDLE lInstance);

VASAPI_API int  VasAPI_StopLive( HANDLE lInstance, UINT64 ubiCameraId, UINT64 ubiWndId );
VASAPI_API int  VasAPI_StartLive(HANDLE lInstance, UINT64 ubiCameraId, UINT64 ubiWndId, UINT32 uiDispatchType
								 , UINT32 uiPlayType, char *szIpAddr, UINT32 uiPlayPort, UINT32 uiPlayStream );

// uiFlag: 1-PS 2-ES
// 在VasAPI_StartLive接口中，uiPlayType=4才能调用此接口
VASAPI_API int  VasAPI_StartGetLiveStream( HANDLE lInstance, 
										  INT64 ubiCameraId,
										  UINT64 ubiWndId, 
											 char *szIpAddr, 
											 UINT32 uiPort, 
											 UINT32 uiFlag,
											 LiveStreamCallback callback,
											 void* pCaller);
                                             

VASAPI_API void  VasAPI_SetReturnCallback( ReturnCallback callback);
VASAPI_API void  VasAPI_SetStreamCallback( StreamCallback callback );
VASAPI_API void  VasAPI_SetDisconnectCallback( DisconnectCallback callback );
VASAPI_API void  VasAPI_SetLargeDataCallback(  LargeDataCallback callback );


VASAPI_API int  VasAPI_GetCameraStatus(  HANDLE lInstance, UINT64 ubiCameraId );

VASAPI_API char*  VasAPI_GetLocalIP( HANDLE lInstance );

VASAPI_API int  VasAPI_GetCamera( HANDLE lInstance);
VASAPI_API int  VasAPI_GetCameraGroup( HANDLE lInstance);

#endif
