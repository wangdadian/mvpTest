#ifndef CLIENT_H_2018
#define CLIENT_H_2018

#include "defined.h"
#include "vasapi.h"
#include "ClientLive.h"
#include <list>
using namespace std;


// 直播信息
typedef struct __ClientLiveInfo
{
	CClientLive* pClientLive; // 直播对象
	//STATUS_CMD status; // 直播状态
	UINT32 uiStartLiveTIme; // 开始请求直播的时间
}ClientLiveInfoT;


class CClient
{
public:
	CClient();
	CClient(const char* szMvpIP, UINT32 uiMvpPort, sClientInfoT* pClientInfo);
	~CClient();
	// 设置信息
	int SetInfo(const char* szMvpIP, UINT32 uiMvpPort, sClientInfoT* pClientInfo);
	// 启动服务
	int Start();
	// 停止服务
	int Stop();
	// 获取状态信息
	int GetStatus(ClientStatusT* pStatus);
	
private:
	// 初始化资源
	int Init();
	// 请求登录
	int Login();
	// 请求注销
	int Logout();
	// 请求获取相机信息
	int GetCamera();
	// 请求获取相机状态信息
	int GetCameraStatus();
	// 请求直播
	int StartLive();
	// 请求停止直播
	int StopLive();
	
	// 接收返回消息回调
	static void MyReturnCallback( void * handle, char * szXmlRet, UINT32 uiMessageType, void * body, void * pCaller );
	static void MyLargeDataCallback( void * handle, UINT32 uiDataType, void * pData, UINT32 uiLen, void * pCaller );
	static void MyDisconnectCallback( void * handle, void * pCaller );
	//static void MyStreamCallback(void * handle, UINT64 ubiCameraId, UINT32 uiDataType, UINT64 ubiSessionId, void * pData, UINT32 uiCount, UINT64 ubiTimeStamp, void * pCaller);

	// 收到返回信令后的处理
	void OnGetCamera(UINT32 uiCamCount, void *pBody);
	void OnGetCameraStatus(UINT32 uiLen, void *pBody);
	void OnStartLive(void * pBody);
	void OnStopLive(void * pBody);
	void OnLogin(void * pBody);
	void OnLogout(void * pBody);

	// 启动服务后，异步等待服务器信令返回，并进行后续业务处理，请求完直播业务后退出线程
	static void* ThWorker(void* param); 
	// 直播控制线程，用于重连，巡检下不重连
	static void* ThReStartLiveWorker(void* param);
	// 巡检
	static void* ThScanWorker(void* param);
	// 重连mvp
	static void* ThReconnectWorker(void* param); 
	// 控制请求直播状态线程
	static void* ThRecvCmdTimeoutWorker(void* param); 
	
	///////////////////////////////////////////////////
	// 获取有效的相机信息
	// 条件: 接入模式未设置或自建接入，未自检或自检后在线
	int GetUsedCamList();
	// 获取下一个可用的相机信息的索引
	int GetNextUsedCamInfoIndex();
	// 启动重连线程
	int StartReconnectThread();
	// 启动巡检线程
	int StartScanThread();
	// 启动直播重连线程
	int StartLiveThread();
	// 登录后启动工作线程
	int StartThread();
	// 初始化直播业务信息，尽在构造或者SetInfo时调用一次，且后于Init调用
	int InitClientLiveInfo();
	// 统计巡检信息
	int CountScanStatusInfo();

private:
	pthread_mutex_t m_lockClientLive;
	pthread_t m_thWorker;
	bool m_bExit; // 线程退出标志

	pthread_t m_thReStartLiveWorker;
	bool m_bReStartLiveThreadExit;
	
	pthread_t m_thScanWorker;
	bool m_bScanThreadExit;

	pthread_t m_thRecvCmdTimeoutWorker;
	bool m_bRecvCmdTimeoutThreadExit;

	// 重连MVP线程
	pthread_t m_thReconnectWorker;
	bool m_bReconnectThreadExit;

	// 标记是否所有直播已请求完毕
	bool m_bStartLiveEnd;
	
	char m_szMvpIP[80]; // 登录的MVP地址
	UINT32 m_uiMvpPort; // 登陆的MVP端口
	sClientInfoT* m_pClientInfo; // 客户端信息
	HANDLE	m_hSdkHandle; // SDK初始化后的句柄

	bool m_bLogined; // 是否登录成功
	STATE_TYPE m_iStateGetCam; // 获取相机列表的状态
	STATE_TYPE m_iStateGetCamStatus; // 获取相机状态是否成功
	
	CameraInfoT* m_pCamerainfo; // 登录后获取的相机信息
	UINT32  m_uiCameraCount; // 登录后获取的相机个数
	CameraStatusInfoT* m_pCameraStatusInfo; //登录后获取的相机状态信息
	UINT32 m_uiCameraStatusCount; // 登录后获取的相机状态信息个数

	CameraInfoT* m_pUsedCamInfo; // 经过滤后的可用相机信息，用于请求直播用
	UINT32  m_uiUsedCamCount; // 经过滤后的可用相机个数
	UINT32 m_uiIndexUsedCamInfo; 
	//CameraInfoT
	ClientLiveInfoT* m_listClientLiveInfo;
	UINT32 m_uiClientLiveInfoCount;

	// 巡检统计信息
	ScanStatusInfoT m_sScanStatusInfo;
};

#endif

