#ifndef CLIENTLIVE_H_2018
#define CLIENTLIVE_H_2018

#include "defined.h"
#include "Facility.h"
#include "vasapi.h"

class CClientLive
{
public:
	CClientLive();
	CClientLive(HANDLE hSdk, UINT64 ubiCameraId, UINT64 ubiWndId);
	~CClientLive();
	// 启动服务
	int Start(char* szIpAddr, UINT32 uiPort);
	// 停止服务
	int Stop();
	// 设置相机信息
	int SetCamInfo(CameraInfoT* pCamInfo);
	// 
	// 获取状态信息
	// pStatus-储存返回的状态
	// bCareNoStreamRate直播成功后码流速率为0时，获取状态时是否关心，为true时若码流速率为0则返回错误
	// 调用者申请参数pStatus内存
	int GetStauts(LiveStatusT* pStatus, bool bCareNoStreamRate=true);
	// 通知已发送请求直播
	void NoteStartLive();
	// 通知已收到请求直播的返回
	void NoteOnStartLive();
	// 获取直播后收流状态
	STATUS_LIVE GetLiveStatus();

	// 获取、设置请求直播的状态
	STATUS_CMD GetStartLiveStatus();
	STATUS_CMD SetStartLiveStatus(STATUS_CMD status);
	
	UINT64 m_ubiCamID;
	UINT64 m_ubiWndID;
	
private:
	
	// 接收直播码流回调
	static void MyLiveStreamCallback(UINT64 ubiCameraId, UINT64 ubiWndId, void * pData, UINT32 uiLen, void * pCaller);
	// 
	static void* ThWorker(void* param);
	///////////////////////////////////////////////////
	
private:
	HANDLE m_hSdkHandle;
	
	pthread_mutex_t m_lockStream;
	pthread_t m_thWorker;
	bool m_bExit; // 线程退出标志
	
	UINT64 m_ubiLiveStreamTime;//最后获取到码流的时间，毫秒
	UINT64 m_ubiLiveStreamLen;// 自上一次统计至最新收到的码流总大小
	UINT64 m_ubiLiveStreamLastTime; // 上一次统计时接收的时间，毫秒
	UINT64 m_ubiLiveStreamLastLen; // 上一次统计时接收到的码流总大小
	float m_fLiveStreamRate; // 每1秒计算一次实时视频接收速率 单位KB/S

	// 直播后收流的状态
	STATUS_LIVE m_uiStatusLive;
	// 请求直播的状态
	STATUS_CMD m_uiStatusStartLive;

	// 开始请求直播的时间
	struct timeval m_tvStartLive;
	// 收到直播返回的时间
	struct timeval m_tvOnStartLive;
	// 准备收取码流的时间
	struct timeval m_tvRecvStream;
	// 收到码流的时间
	struct timeval m_tvOnRecvStream;

	// 关联的相机信息
	CameraInfoT m_camInfo;
};

#endif


