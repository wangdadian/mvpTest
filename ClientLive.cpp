#include "ClientLive.h"

CClientLive::CClientLive()
{
    m_hSdkHandle = NULL;
    m_ubiCamID = 0;
    m_ubiWndID = 0;
    m_uiStatusStartLive = SC_NONE;
    m_uiStatusLive = LIVE_NONE;

    m_camInfo.bStrmMvpChecked = false;
    m_camInfo.bStrmMvpCheckedOK = false;
    m_camInfo.ubiCamID = m_ubiCamID;

    m_bExit = false;
	m_ubiLiveStreamTime = 0;
	m_thWorker = 0;
	m_ubiLiveStreamLastTime = 0;
	m_ubiLiveStreamLastLen = 0;
	m_fLiveStreamRate = 0;
	m_ubiLiveStreamLen = 0;
	m_tvStartLive.tv_sec = m_tvOnStartLive.tv_sec = m_tvRecvStream.tv_sec = 0;
	m_tvStartLive.tv_usec = m_tvOnStartLive.tv_usec = m_tvRecvStream.tv_usec = 0;
	pthread_mutex_init(&m_lockStream, NULL);
}

CClientLive::CClientLive(HANDLE hSdk, UINT64 ubiCameraId, UINT64 ubiWndId)
{
    m_hSdkHandle = hSdk;
    m_ubiCamID = ubiCameraId;
    m_ubiWndID = ubiWndId;
    m_uiStatusStartLive = SC_NONE;
    m_uiStatusLive = LIVE_NONE;
    m_camInfo.bStrmMvpChecked = false;
    m_camInfo.bStrmMvpCheckedOK = false;
    m_camInfo.ubiCamID = m_ubiCamID;
    m_bExit = false;
	m_ubiLiveStreamTime = 0;
	m_thWorker = 0;
	m_ubiLiveStreamLastTime = 0;
	m_ubiLiveStreamLastLen = 0;
	m_fLiveStreamRate = 0;
	m_ubiLiveStreamLen = 0;
	m_tvStartLive.tv_sec = m_tvOnStartLive.tv_sec = m_tvRecvStream.tv_sec = m_tvOnRecvStream.tv_sec = 0;
	m_tvStartLive.tv_usec = m_tvOnStartLive.tv_usec = m_tvRecvStream.tv_usec = m_tvOnRecvStream.tv_usec = 0;
	pthread_mutex_init(&m_lockStream, NULL);
}
CClientLive::~CClientLive()
{
    m_hSdkHandle = NULL;
    m_ubiCamID = 0;
    m_ubiWndID = 0;
    m_uiStatusStartLive = SC_NONE;
    m_uiStatusLive = LIVE_NONE;

	m_ubiLiveStreamTime = 0;
    m_bExit = true;
    m_ubiLiveStreamLastTime = 0;
	m_ubiLiveStreamLastLen = 0;
	m_fLiveStreamRate = 0;
	m_ubiLiveStreamLen = 0;
	m_tvStartLive.tv_sec = m_tvOnStartLive.tv_sec = m_tvRecvStream.tv_sec = m_tvOnRecvStream.tv_sec = 0;
	m_tvStartLive.tv_usec = m_tvOnStartLive.tv_usec = m_tvRecvStream.tv_usec = m_tvOnRecvStream.tv_usec = 0;
	if (m_thWorker != 0)
	{
		pthread_join(m_thWorker, NULL);
		m_thWorker = 0;
	}
	pthread_mutex_destroy(&m_lockStream);
}

// 设置相机信息
int CClientLive::SetCamInfo(CameraInfoT* pCamInfo)
{
    if(pCamInfo == NULL)
    {
        m_camInfo.bStrmMvpChecked = false;
        m_camInfo.bStrmMvpCheckedOK = false;
        m_camInfo.ubiCamID = 0;
    }
    else
    {
        memcpy(&m_camInfo, pCamInfo, sizeof(CameraInfoT));
    }
    return 0;
}

// 启动服务
int CClientLive::Start(char* szIpAddr, UINT32 uiPort)
{
    // 设置准备接收码流的时间
    gettimeofday(&m_tvRecvStream, NULL);
    // 重置收到码流的时间
    m_tvOnRecvStream.tv_sec = m_tvOnRecvStream.tv_usec = 0;

    int iRet = VasAPI_StartGetLiveStream(m_hSdkHandle, m_ubiCamID, m_ubiWndID, szIpAddr, uiPort, 2, MyLiveStreamCallback, (void*)this);
    if(iRet != 0)
    {
        _ERROR_("Call VASAPI failed[cam:%llu window:%llu]", m_ubiCamID, m_ubiWndID);
        //return -1;
    }

    m_uiStatusLive = LIVE_WAITING;
    m_bExit = false;
    m_ubiLiveStreamTime = 0;
    m_ubiLiveStreamLastTime = 0;
	m_ubiLiveStreamLastLen = 0;
	m_fLiveStreamRate = 0;
	m_ubiLiveStreamLen = 0;
	pthread_create( &m_thWorker, NULL, ThWorker, (void*)this);
    return 0;
}

// 停止服务
int CClientLive::Stop()
{
    if(m_uiStatusLive == LIVE_NONE || m_uiStatusStartLive == SC_NONE)
    {
        //return 0;
    }

    int iRet = VasAPI_StopLive(m_hSdkHandle, m_ubiCamID, m_ubiWndID);
    if(iRet != 0)
    {
        _ERROR_("Stop Live[cam:%llu window:%llu] failed!", m_ubiCamID, m_ubiWndID);
        //return -1;
    }

    if (m_thWorker != 0)
	{
	    m_bExit = true;
		pthread_join(m_thWorker, NULL);
		m_thWorker = 0;
	}
	_DEBUG_("Stop Live[cam:%llu window:%llu] OK!", m_ubiCamID, m_ubiWndID);

	m_uiStatusLive = LIVE_NONE;
	m_ubiLiveStreamTime = 0;
	m_ubiLiveStreamLastTime = 0;
	m_ubiLiveStreamLastLen = 0;
	m_fLiveStreamRate = 0;
	m_ubiLiveStreamLen = 0;
	m_tvStartLive.tv_sec = m_tvOnStartLive.tv_sec = m_tvRecvStream.tv_sec = m_tvOnRecvStream.tv_sec = 0;
	m_tvStartLive.tv_usec = m_tvOnStartLive.tv_usec = m_tvRecvStream.tv_usec = m_tvOnRecvStream.tv_usec = 0;
    return 0;
}

void* CClientLive::ThWorker(void * param)
{
    CClientLive* pThis = (CClientLive*)param;
    UINT64 ubiTimeStart = CFacility::GetCurTimeMS();
    UINT64 ubiTimeNow = 0;
    INT64 biInterval = 0;
    float fInterval = 0;
    float fLen = 0;
    _DEBUG_("Thread START [cam:%llu window:%llu][param:0x%x]", pThis->m_ubiCamID, pThis->m_ubiWndID, (long)param);
    while(pThis->m_bExit == false)
    {
        //pthread_mutex_lock(&pThis->m_lockStream);
        ubiTimeNow = CFacility::GetCurTimeMS();
        if(pThis->m_ubiLiveStreamTime == 0)
        {
            biInterval = ubiTimeNow - ubiTimeStart;
        }
        else
        {
            biInterval = ubiTimeNow - pThis->m_ubiLiveStreamTime;
        }

        // 超时未收到流
        if(biInterval > TIMEOUT_LIVE_NOSTREAM*1000)
        {
            pThis->m_fLiveStreamRate = 0.0;
            pThis->m_uiStatusLive = LIVE_TIMEOUT;

            pThis->m_ubiLiveStreamTime = 0;
            pThis->m_ubiLiveStreamLastTime = 0;
            pThis->m_ubiLiveStreamLen = 0;
            pThis->m_ubiLiveStreamLastLen = 0;
        }
        else
        {
            // 计算速率
            if(pThis->m_ubiLiveStreamLastTime == 0) // 第一次接收到数据时的处理
            {
                pThis->m_ubiLiveStreamLastTime = pThis->m_ubiLiveStreamTime;
                pThis->m_ubiLiveStreamLastLen = pThis->m_ubiLiveStreamLen;
            }

            // 计算速率
            if( (pThis->m_ubiLiveStreamTime - pThis->m_ubiLiveStreamLastTime) >= 1000 )
            {
                // 计算速率
                fInterval = (float)((pThis->m_ubiLiveStreamTime - pThis->m_ubiLiveStreamLastTime) * 1.0 / 1000.0); // S
                fLen = (float)((pThis->m_ubiLiveStreamLen - pThis->m_ubiLiveStreamLastLen) * 1.0 / 1024.0); // KB
                pThis->m_fLiveStreamRate = fLen / fInterval; // KB/S
                //if(time(NULL)%10 == 0)
                //{
                //    _DEBUG_("Live stream[cam:%llu window:%llu], Rate: %.2f KB/S", pThis->m_ubiCamID, pThis->m_ubiWndID, pThis->m_fLiveStreamRate);
                //}

                // 重置上一次统计数据
                pThis->m_ubiLiveStreamLastTime = pThis->m_ubiLiveStreamTime;
                pThis->m_ubiLiveStreamLen = 0;
                pThis->m_ubiLiveStreamLastLen = pThis->m_ubiLiveStreamLen;
            }
        }

        //pthread_mutex_unlock(&pThis->m_lockStream);

        CFacility::SleepMsec(200);
    }
    _DEBUG_("Thread EXIT [cam:%llu window:%llu][param:0x%x]", pThis->m_ubiCamID, pThis->m_ubiWndID, (long)param);
    return NULL;
}

// 接收直播码流回调
void CClientLive::MyLiveStreamCallback(UINT64 ubiCameraId, UINT64 ubiWndId, void * pData, UINT32 uiLen, void * pCaller)
{
    if(pCaller == NULL)
    {
        _ERROR_("Live [cam:%llu window:%llu], pCaller is NULL!", ubiCameraId, ubiWndId);
        return;
    }
    CClientLive* pThis = (CClientLive*)pCaller;
    if(pThis->m_ubiCamID != ubiCameraId || pThis->m_ubiWndID != ubiWndId)
    {
        _ERROR_("Received live stream ERROR, local[cam:%llu window:%llu], received[cam:%llu window:%llu]", \
                 pThis->m_ubiCamID, pThis->m_ubiWndID, ubiCameraId, ubiWndId);
        return;
    }
    if(pThis->m_tvOnRecvStream.tv_sec==0 && pThis->m_tvOnRecvStream.tv_usec==0)
    {
        gettimeofday(&pThis->m_tvOnRecvStream, NULL);
    }
    //
    if(pThis->m_uiStatusLive != LIVE_OK)
    {
        pThis->m_uiStatusLive = LIVE_OK;
    }
    pThis->m_ubiLiveStreamTime = CFacility::GetCurTimeMS();
    pThis->m_ubiLiveStreamLen += uiLen;

}

// pStatus-储存返回的状态
// bCareNoStreamRate直播成功后码流速率为0时，获取状态时是否关心，为true时若码流速率为0则返回错误
int CClientLive::GetStauts(LiveStatusT* pStatus, bool bCareNoStreamRate)
{
    if(pStatus == NULL)
    {
        return -1;
    }


    // 当请求直播处于等待或者未请求时，当直播状态处于等待或者未直播时，不允许获取状态
    if( m_uiStatusStartLive==SC_NONE || m_uiStatusStartLive==SC_WAITING ||
        m_uiStatusLive==LIVE_WAITING || m_uiStatusLive==LIVE_NONE
    )
    {
        return -1;
    }
    // 如果直播状态为OK，但速率尚为0，也禁止获取状态
    if(bCareNoStreamRate && m_uiStatusLive == LIVE_OK && m_fLiveStreamRate <= 0.0001f)
    {
        return -1;
    }


    pStatus->ubiCamID = m_ubiCamID;
    pStatus->ubiWndID = m_ubiWndID;
    pStatus->statusStartLive = m_uiStatusStartLive;
    pStatus->statusLive = m_uiStatusLive;
    pStatus->fStreamRate = m_fLiveStreamRate;
	INT32 iRecvCmdTime = 0;
	INT32 iRecvStreamTime = 0;
	INT64 biTimeStart = 0;
	INT64 biTimeEnd = 0;
	if(m_uiStatusStartLive==SC_OK || m_uiStatusStartLive==SC_FAILED)
	{
	    biTimeStart = m_tvStartLive.tv_sec*1000 + m_tvStartLive.tv_usec/1000;
	    biTimeEnd = m_tvOnStartLive.tv_sec*1000 + m_tvOnStartLive.tv_usec/1000;
	    iRecvCmdTime = (INT32)(biTimeEnd - biTimeStart);
	}
	else
	{
	    iRecvCmdTime = -1;
	}

	if(m_uiStatusLive == LIVE_OK)
	{
	    biTimeStart = m_tvRecvStream.tv_sec*1000 + m_tvRecvStream.tv_usec/1000;
	    biTimeEnd = m_tvOnRecvStream.tv_sec*1000 + m_tvOnRecvStream.tv_usec/1000;
	    iRecvStreamTime = (INT32)(biTimeEnd - biTimeStart);
	}
	else
	{
	    iRecvStreamTime = -1;
	}
	pStatus->iRecvCmdTime = iRecvCmdTime;
	pStatus->iRecvStreamTime = iRecvStreamTime;
	pStatus->bStrmMvpChecked = m_camInfo.bStrmMvpChecked;
	pStatus->bStrmMvpCheckedOK = m_camInfo.bStrmMvpCheckedOK;
	pStatus->bMvpVideoOK = m_camInfo.bVideoOK;
	pStatus->bStrmMyCheckedOK = (m_uiStatusLive==LIVE_OK) ? true : false;

    return 0;
}

// 通知已发送请求直播
void CClientLive::NoteStartLive()
{
    gettimeofday(&m_tvStartLive, NULL);
    m_tvOnStartLive.tv_sec = m_tvOnStartLive.tv_usec = 0;
}
// 通知已收到请求直播的返回
void CClientLive::NoteOnStartLive()
{
    gettimeofday(&m_tvOnStartLive, NULL);
}


// 获取直播后收流状态
STATUS_LIVE CClientLive::GetLiveStatus()
{
    return m_uiStatusLive;
}

// 获取、设置请求直播的状态
STATUS_CMD CClientLive::GetStartLiveStatus()
{
    return m_uiStatusStartLive;
}

STATUS_CMD CClientLive::SetStartLiveStatus(STATUS_CMD status)
{
    m_uiStatusStartLive = status;
    return m_uiStatusStartLive;
}





