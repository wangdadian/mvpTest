#include "Client.h"

CClient::CClient()
{
    memset(m_szMvpIP, 0, sizeof(m_szMvpIP));
    m_uiMvpPort = 0;
    m_pClientInfo = NULL;
    m_uiClientLiveInfoCount = 0;
    m_listClientLiveInfo = NULL;
    
    Init();
}

CClient::~CClient()
{
    m_bExit = true;
    if(m_thWorker != 0)
    {
        pthread_join(m_thWorker, NULL);
        m_thWorker = 0;
    }
    m_bRecvCmdTimeoutThreadExit = true;
    if(m_thRecvCmdTimeoutWorker != 0)
    {
        pthread_join(m_thRecvCmdTimeoutWorker, NULL);
        m_thRecvCmdTimeoutWorker = 0;
    }
    m_bReStartLiveThreadExit = true;
    if(m_thReStartLiveWorker!= 0)
    {
        pthread_join(m_thReStartLiveWorker, NULL);
        m_thReStartLiveWorker = 0;
    }
    m_bScanThreadExit = true;
    if(m_thScanWorker != 0)
    {
        pthread_join(m_thScanWorker, NULL);
        m_thScanWorker = 0;
    }
    m_bReconnectThreadExit = true;
    if(m_thReconnectWorker != 0)
    {
        pthread_join(m_thReconnectWorker, NULL);
        m_thReconnectWorker = 0;
    }

    //pthread_mutex_lock(&m_lockClientLive);
    if(m_listClientLiveInfo != NULL)
    {
        for(UINT i=0; i<m_uiClientLiveInfoCount; i++)
        {
            if(m_listClientLiveInfo[i].pClientLive != NULL)
            {
                delete m_listClientLiveInfo[i].pClientLive;
                m_listClientLiveInfo[i].pClientLive = NULL;
            }
        }
        delete [] m_listClientLiveInfo;
        m_listClientLiveInfo = NULL;
        m_uiClientLiveInfoCount = 0;
    }
    //pthread_mutex_unlock(&m_lockClientLive);

    if(m_pCamerainfo != NULL)
    {
        delete [] m_pCamerainfo;
        m_pCamerainfo = NULL;
    }
    if(m_pCameraStatusInfo != NULL)
    {
        delete [] m_pCameraStatusInfo;
        m_pCameraStatusInfo = NULL;
    }
    if(m_pUsedCamInfo != NULL)
    {
        delete [] m_pUsedCamInfo;
        m_pUsedCamInfo = NULL;
    }
    
    pthread_mutex_destroy( &m_lockClientLive );
    
    // 最后释放SDK
    if(m_hSdkHandle != NULL)
    {
        VasAPI_Uninitialize(m_hSdkHandle);
        m_hSdkHandle = NULL;
    }
}

CClient::CClient(const char* szMvpIP, UINT32 uiMvpPort, sClientInfoT* pClientInfo)
{
    sprintf(m_szMvpIP, "%s", szMvpIP);
    m_uiMvpPort = uiMvpPort;
    m_pClientInfo = pClientInfo;
    m_uiClientLiveInfoCount = 0;
    m_listClientLiveInfo = NULL;
    Init();
    InitClientLiveInfo();
}

int CClient::Init()
{
    m_bStartLiveEnd = false;
    m_hSdkHandle = NULL;
    m_bLogined = false;
    m_iStateGetCam = STATE_NONE;
    m_iStateGetCamStatus = STATE_NONE;
    m_pCamerainfo = NULL;
    m_uiCameraCount = 0;

    m_pCameraStatusInfo = NULL;
    m_uiCameraStatusCount = 0;
    m_pUsedCamInfo = NULL;
    m_uiUsedCamCount = 0;
    m_uiIndexUsedCamInfo = 0;
    
    m_bExit = false;
    m_thWorker = 0;
    m_bReStartLiveThreadExit = false;
    m_thReStartLiveWorker = 0;
    m_bScanThreadExit = false;
    m_thScanWorker = 0;
    m_bReconnectThreadExit = false;
    m_thReconnectWorker = 0;
    m_thRecvCmdTimeoutWorker = 0;
    m_bRecvCmdTimeoutThreadExit = false;

    memset(&m_sScanStatusInfo, 0, sizeof(ScanStatusInfoT));
    
    pthread_mutex_init(&m_lockClientLive, NULL);
    // 初始化SDK
    m_hSdkHandle = VasAPI_Initialize(this);
    if(m_hSdkHandle != NULL)
    {
        // 设置SDK相关回调处理函数
        VasAPI_SetReturnCallback(MyReturnCallback);
        VasAPI_SetLargeDataCallback(MyLargeDataCallback);
        VasAPI_SetDisconnectCallback(MyDisconnectCallback);
    }
    else
    {
        _ERROR_("Initialize vasapi SDK failed!");
        exit(1);
    }
    
    pthread_create(&m_thRecvCmdTimeoutWorker, NULL, ThRecvCmdTimeoutWorker, (void*)this);
    return 0;
}

// 初始化直播业务信息
int CClient::InitClientLiveInfo()
{
    if(m_pClientInfo->uiMaxWndCnt <= 0)
    {
        _ERROR_("client live max window count=0!");
        return -1;
    }

    m_listClientLiveInfo = new ClientLiveInfoT[m_pClientInfo->uiMaxWndCnt];
    if(m_listClientLiveInfo == NULL)
    {
        _ERROR_("New Memory failed!");
        return -1;
    }
    m_uiClientLiveInfoCount = m_pClientInfo->uiMaxWndCnt;
    UINT64 ubiWndId = 0;
    for(UINT32 i=0; i<m_uiClientLiveInfoCount; i++)
    {
        ubiWndId = m_pClientInfo->ubiWndIndex + i;
        m_listClientLiveInfo[i].pClientLive = new CClientLive(m_hSdkHandle, 0, ubiWndId);
        m_listClientLiveInfo[i].pClientLive->SetStartLiveStatus(SC_NONE);
        m_listClientLiveInfo[i].uiStartLiveTIme = 0;
    }
    
    return 0;
}

// 设置信息
int CClient::SetInfo(const char* szMvpIP, UINT32 uiMvpPort, sClientInfoT* pClientInfo)
{
    if(m_szMvpIP[0] == 0)
    {
        sprintf(m_szMvpIP, "%s", szMvpIP);
    }
    else
    {
        // 是否重设，待定
    }

    if(m_uiMvpPort == 0)
    {
        m_uiMvpPort = uiMvpPort;
    }
    else
    {
        // 是否重设，待定
    }
    
    if(m_pClientInfo == NULL)
    {
        m_pClientInfo = pClientInfo;
        InitClientLiveInfo();
    }
    else
    {
        // 是否重设，待定
    }
    
    
    return 0;
}

// 启动服务
int CClient::Start()
{
    if(m_bLogined)
    {
        return 0;
    }
    // 登录
    m_bLogined = false;
    if( 0 != Login() )
    {
        /*_ERROR_("Start Client [mvp_ip:%s mvp_port:%d user=%s pass=%s maxWnd:%d wndIndex:%llu] Failed!", \
                m_szMvpIP, m_uiMvpPort, m_pClientInfo->szUser, m_pClientInfo->szPass, \
                m_pClientInfo->uiMaxWndCnt, m_pClientInfo->ubiWndIndex);*/
        _ERROR_("################ Login failed, EXIT!");
        
        exit(2);
    }
    // 第一步，启动线程进行后续的异步处理工作，获取相机、直播等
    StartThread();

    //第二步，启动直播重连、巡检线程
    StartLiveThread();
    StartScanThread();
    
    _DEBUG_("Start Client [mvp_ip:%s mvp_port:%d user=%s pass=%s maxWnd:%d wndIndex:%llu] OK!", \
             m_szMvpIP, m_uiMvpPort, m_pClientInfo->szUser, m_pClientInfo->szPass, \
             m_pClientInfo->uiMaxWndCnt, m_pClientInfo->ubiWndIndex);
    return 0;
}

// 停止服务
int CClient::Stop()
{
    m_bExit = true;
    if (m_thWorker != 0)
    {
        pthread_join(m_thWorker, NULL);
        m_thWorker = 0;
    }
    m_bReStartLiveThreadExit = true;
    if(m_thReStartLiveWorker != 0)
    {
        pthread_join(m_thReStartLiveWorker, NULL);
        m_thReStartLiveWorker = 0;
    }
    m_bScanThreadExit = true;
    if(m_thScanWorker != 0)
    {
        pthread_join(m_thScanWorker, NULL);
        m_thScanWorker = 0;
    }
    m_bReconnectThreadExit = true;
    if(m_thReconnectWorker != 0)
    {
        pthread_join(m_thReconnectWorker, NULL);
        m_thReconnectWorker = 0;
    }
    // 停止直播
    if(0 != StopLive())
    {
        
    }
    // 注销
    if( 0 != Logout() )
    {
        
    }
    
    _DEBUG_("Stop Client [mvp_ip:%s mvp_port:%d user=%s pass=%s maxWnd:%d wndIndex:%llu] OK!", \
             m_szMvpIP, m_uiMvpPort, m_pClientInfo->szUser, m_pClientInfo->szPass, \
             m_pClientInfo->uiMaxWndCnt, m_pClientInfo->ubiWndIndex);
    return 0;
}

// 获取状态信息
int CClient::GetStatus(ClientStatusT* pStatus)
{
    //pthread_mutex_lock(&m_lockClientLive);
    pStatus->bLogin = m_bLogined;
    sprintf(pStatus->szUser, "%s", m_pClientInfo->szUser);
    sprintf(pStatus->szPass, "%s", m_pClientInfo->szPass);
    pStatus->bScan = m_pClientInfo->bScan;
    pStatus->uiScanTimeSpan = m_pClientInfo->uiScanTimeInterval;
    pStatus->bReStartLive = m_pClientInfo->bReStartLive;
    pStatus->bRandomSwitch = m_pClientInfo->bRandomSwitch;
    memset(&pStatus->sLiveStausCntInfo, 0, sizeof(LiveStatusCountInfoT));
    pStatus->fStreamRate = 0;
    pStatus->uiCount = m_bLogined ? m_uiClientLiveInfoCount : 0;
    pStatus->sLiveStausCntInfo.uiLiveCount = m_bLogined ? m_uiClientLiveInfoCount : 0;
    pStatus->uiFirstLoginTime = m_pClientInfo->uiFirstLoginTime;
    pStatus->uiLoginTime = m_pClientInfo->uiLoginTime;
    pStatus->uiLoginCount = m_pClientInfo->uiLoginCount;

    if(pStatus->uiCount <= 0 || pStatus->bLogin == false)
    {
        //pthread_mutex_unlock(&m_lockClientLive);
        pStatus->pLiveStatusData = NULL;
        return 0; 
    }
    
    pStatus->pLiveStatusData = new LiveStatusT[m_uiClientLiveInfoCount];
    if(pStatus->pLiveStatusData == NULL)
    {
        //pthread_mutex_unlock(&m_lockClientLive);
        _ERROR_("New memory failed!");
        return -1;
    }
    // client的巡检状态
    memcpy(&pStatus->sScanStatusInfo, &m_sScanStatusInfo, sizeof(ScanStatusInfoT));
    
    for(UINT32 i=0; i<m_uiClientLiveInfoCount; i++)
    {
        if(m_listClientLiveInfo[i].pClientLive->GetStauts(&pStatus->pLiveStatusData[i]) != 0)
        {
            goto goto_failed;
        }
        // client的直播速率
        pStatus->fStreamRate += pStatus->pLiveStatusData[i].fStreamRate;
        // client的直播状态统计
        switch(pStatus->pLiveStatusData[i].statusStartLive)
        {
            case SC_NONE:
                pStatus->sLiveStausCntInfo.uiSLNoneCnt++;
                break;
            case SC_OK:
                pStatus->sLiveStausCntInfo.uiSLOKCnt++;
                break;
            case SC_FAILED:
                pStatus->sLiveStausCntInfo.uiSLFailCnt++;
                break;
            case SC_WAITING:
                pStatus->sLiveStausCntInfo.uiSLWaitingCnt++;
                break;
            case SC_TIMEOUT:
                pStatus->sLiveStausCntInfo.uiSLTimeoutCnt++;
                break;
            case SC_ERROR:
                pStatus->sLiveStausCntInfo.uiSLErrorCnt++;
                break;
            default:
                break;
        }
        
        switch(pStatus->pLiveStatusData[i].statusLive)
        {
            case LIVE_OK:
                pStatus->sLiveStausCntInfo.uiLiveOKCnt++;
                break;
            case LIVE_WAITING:
                pStatus->sLiveStausCntInfo.uiLiveWaitingCnt++;
                break;
            case LIVE_TIMEOUT:
                pStatus->sLiveStausCntInfo.uiLiveTimeoutCnt++;
                break;
            default:
                break;
        }
        // 统计MVP已自检且在线或者码流正常，但本程序自检不成功的个数
        if(
            ((pStatus->pLiveStatusData[i].bStrmMvpChecked && pStatus->pLiveStatusData[i].bStrmMvpCheckedOK) ||
             pStatus->pLiveStatusData[i].bMvpVideoOK)
            && 
            pStatus->pLiveStatusData[i].bStrmMyCheckedOK == false)
        {
            pStatus->sLiveStausCntInfo.uiMvpOkButNotOKCnt++;
        }
    }
    //pthread_mutex_unlock(&m_lockClientLive);
    return 0;

goto_failed:
    if(pStatus->pLiveStatusData != NULL)
    {
        delete [] pStatus->pLiveStatusData;
        pStatus->pLiveStatusData = NULL;
    }
    pStatus->uiCount = 0;
    return -1;    
}


int CClient::Login()
{
    int iRet = VasAPI_ConnectToServer(m_hSdkHandle, m_szMvpIP, m_uiMvpPort);
    if(0 != iRet)
    {
        _ERROR_("Connect to mvp server[mvp %s:%d] failed!", m_szMvpIP, m_uiMvpPort);
        return -1;
    }
    iRet = VasAPI_UserLogin(m_hSdkHandle, m_pClientInfo->szUser, m_pClientInfo->szPass);
    if(0 != iRet)
    {
        _ERROR_("Login to mvp server[mvp %s:%d] failed!", m_szMvpIP, m_uiMvpPort);
        return -1;
    }
    
    _DEBUG_("Login [mvp %s:%d] [user:%s], waiting.", m_szMvpIP, m_uiMvpPort, m_pClientInfo->szUser);
    return 0;
}

int CClient::Logout()
{
    if( ! m_bLogined )
    {
        return 0;
    }
    m_bLogined = false;
    int iRet = VasAPI_UserLogout(m_hSdkHandle);
    if( 0 != iRet)
    {
        _ERROR_("Logout from mvp server[mvp %s:%d] failed!", m_szMvpIP, m_uiMvpPort);
        return -1;
    }
    _DEBUG_("Logout [mvp %s:%d] [user:%s], waiting.", m_szMvpIP, m_uiMvpPort, m_pClientInfo->szUser);
    
    return 0;
}

// 请求获取相机信息
int CClient::GetCamera()
{
    int iRet = VasAPI_GetCamera(m_hSdkHandle);
    if( 0 != iRet)
    {
        _ERROR_("Get Camera from mvp server[mvp %s:%d] failed!", m_szMvpIP, m_uiMvpPort);
        m_iStateGetCam = STATE_FAILED;
        return -1;
    }
    m_iStateGetCam = STATE_WAITING;
    _DEBUG_("Get Camera from [mvp %s:%d] [user:%s], waiting.", m_szMvpIP, m_uiMvpPort, m_pClientInfo->szUser);
    return 0;
}

// 请求获取相机状态信息
int CClient::GetCameraStatus()
{
    int iRet = VasAPI_GetCameraStatus(m_hSdkHandle, 0);
    if( 0 != iRet)
    {
        _ERROR_("Get Camera status from mvp server[mvp %s:%d] failed!", m_szMvpIP, m_uiMvpPort);
        m_iStateGetCamStatus = STATE_FAILED;
        return -1;
    }
    m_iStateGetCamStatus = STATE_WAITING;
    _DEBUG_("Get Camera status from [mvp %s:%d] [user:%s], waiting.", m_szMvpIP, m_uiMvpPort, m_pClientInfo->szUser);
    return 0;
}

// 请求直播
int CClient::StartLive()
{
    if( !m_bLogined )
    {
        return -1;
    }
    
    if(m_listClientLiveInfo == NULL)
    {
        return -1;
    }

    m_bStartLiveEnd = false;
    m_bExit = false;
    //pthread_mutex_lock(&m_lockClientLive);
    int iRet = 0;
    int iUsedCamInfoIndex = 0;
    for(UINT32 i=0; i<m_uiClientLiveInfoCount; i++)
    {
        // 取下一个可用相机
        iUsedCamInfoIndex = GetNextUsedCamInfoIndex();
        if(iUsedCamInfoIndex == -1)
        {
            return -1;
        }
        m_listClientLiveInfo[i].pClientLive->m_ubiCamID = m_pUsedCamInfo[iUsedCamInfoIndex].ubiCamID;
        m_listClientLiveInfo[i].uiStartLiveTIme = time(NULL);
        m_listClientLiveInfo[i].pClientLive->NoteStartLive();
        m_listClientLiveInfo[i].pClientLive->SetCamInfo(&m_pUsedCamInfo[iUsedCamInfoIndex]);
        // 请求直播
        iRet = VasAPI_StartLive(m_hSdkHandle, m_listClientLiveInfo[i].pClientLive->m_ubiCamID, \
                                  m_listClientLiveInfo[i].pClientLive->m_ubiWndID,  0, 4, "", 0, 0);
        if(iRet != 0)
        {
            m_listClientLiveInfo[i].pClientLive->SetStartLiveStatus(SC_ERROR);
            _ERROR_("Call VasAPI_StartLive failed [cam:%llu window:%llu]", m_listClientLiveInfo[i].pClientLive->m_ubiCamID, \
                     m_listClientLiveInfo[i].pClientLive->m_ubiWndID);
        }
        _DEBUG_("Start live [cam:%llu window:%llu]", m_listClientLiveInfo[i].pClientLive->m_ubiCamID, \
                 m_listClientLiveInfo[i].pClientLive->m_ubiWndID);
        m_listClientLiveInfo[i].pClientLive->SetStartLiveStatus(SC_WAITING);
        // 每次请求直播间隔
        CFacility::SleepMsec(50);
    }
    //pthread_mutex_unlock(&m_lockClientLive);
    m_bStartLiveEnd = true;
    return 0;
}

// 请求停止直播
int CClient::StopLive()
{
    //pthread_mutex_lock(&m_lockClientLive);
    for(UINT32 i=0; i<m_uiClientLiveInfoCount; i++)
    {
        _DEBUG_("Stop live [cam:%llu window:%llu]", m_listClientLiveInfo[i].pClientLive->m_ubiCamID, m_listClientLiveInfo[i].pClientLive->m_ubiWndID);
        m_listClientLiveInfo[i].pClientLive->Stop();
        m_listClientLiveInfo[i].pClientLive->SetStartLiveStatus(SC_NONE);
        m_listClientLiveInfo[i].pClientLive->SetCamInfo(NULL);
        // 每次停止直播间隔
        CFacility::SleepMsec(50);
    }
    //pthread_mutex_unlock(&m_lockClientLive);
    return 0;
}


void CClient::MyReturnCallback( void * handle, char * szXmlRet, UINT32 uiMessageType, void * body, void * pCaller )
{
    CClient* pThis = (CClient*)pCaller;
    if (pThis == NULL) return;
    switch (uiMessageType)
    {
    case RET_USER_LOGIN:
        pThis->OnLogin(body);
        break;
    case RET_USER_LOGOUT:
        pThis->OnLogout(body);
        break;
    case RET_START_LIVE:
        pThis->OnStartLive(body);
        break;
    case RET_STOP_LIVE:
        pThis->OnStopLive(body);
        break;
    }    
}

void CClient::MyLargeDataCallback( void * handle, UINT32 uiDataType, void * pData, UINT32 uiLen, void * pCaller )
{
    if (pCaller == NULL)
    {
        return;
    }
    CClient* pThis = (CClient*)pCaller;
    switch (uiDataType)
    {
        case LData_RetCamera:
            pThis->OnGetCamera(uiLen, pData);
            break;
        case LData_RetCameraStatus:
            pThis->OnGetCameraStatus(uiLen, pData);
        break;
    }
}

void CClient::MyDisconnectCallback( void * handle, void * pCaller )
{
    if (pCaller == NULL)
    {
        return;
    }
    CClient* pThis = (CClient*)pCaller;
    _DEBUG_("####### Disconnect from mvp[%s:%d] user[%s] ######", pThis->m_szMvpIP, pThis->m_uiMvpPort, pThis->m_pClientInfo->szUser);
    pThis->m_bLogined = false;
    // 停止服务，清理资源
    pThis->Stop();
    // 结束重连线程并重启重连线程
    pThis->StartReconnectThread();
}

// 重连mvp
void* CClient::ThReconnectWorker(void* param)
{
    CClient* pThis = (CClient*)param;
    _DEBUG_("Reconnect work start!");
    while(pThis->m_bReconnectThreadExit == false)
    {
        // 每10秒重连一次
        for(UINT32 i=0; i<100; i++)
        {
            if(pThis->m_bReconnectThreadExit)
            {
                _DEBUG_("Reconnect work exit1!");
                return NULL;
            }
            CFacility::SleepMsec(100);
        }
        // 重连
        _DEBUG_("Start reconnect to mvp[%s:%d]...", pThis->m_szMvpIP, pThis->m_uiMvpPort);
        pThis->m_bLogined = false;
        // 重新登录
        if( 0 == pThis->Login() )
        {
            _DEBUG_("Reconnect and relogin to mvp[%s:%d], OK!", pThis->m_szMvpIP, pThis->m_uiMvpPort);
            // 启动业务线程
            pThis->StartThread();
            // 启动直播重连线程
            pThis->StartLiveThread();
            // 启动巡检线程
            pThis->StartScanThread();
            break;
        }
        _DEBUG_("Reconnect to mvp[%s:%d], Failed!", pThis->m_szMvpIP, pThis->m_uiMvpPort);
    }
    _DEBUG_("Reconnect work exit2!");
    return NULL;
}

void CClient::OnGetCamera(UINT32 uiCamCount, void *pBody)
{
    CameraRes* pCams = (CameraRes*)pBody;
    // 释放之前的资源
    if (m_pCamerainfo)
    {
        delete [] m_pCamerainfo;
        m_pCamerainfo = NULL;
    }
    m_uiCameraCount = 0;

    if (uiCamCount <= 0)
    {
        return;
    }
    m_uiCameraCount = uiCamCount;

    m_pCamerainfo = new CameraInfoT[m_uiCameraCount];
    if (m_pCamerainfo == NULL) 
    {
        m_uiCameraCount = 0;
        return;
    }
    for (UINT32 i=0; i<m_uiCameraCount; i++)
    {
        m_pCamerainfo[i].ubiCamID = pCams[i].ubiCameraId;
        m_pCamerainfo[i].bStrmMvpChecked = false;
        m_pCamerainfo[i].bStrmMvpCheckedOK = false;
        m_pCamerainfo[i].bVideoOK = false;
    }
    _DEBUG_("Camera count: %d", m_uiCameraCount);
    m_iStateGetCam = STATE_OK;

}

void CClient::OnGetCameraStatus(UINT32 uiLen, void *pBody)
{
    CameraStatusRes* pCamStatus = (CameraStatusRes*)pBody;
    if(m_pCameraStatusInfo != NULL)
    {
        delete [] m_pCameraStatusInfo;
        m_pCameraStatusInfo = NULL;
        m_uiCameraStatusCount = 0;
    }
    
    _DEBUG_("Camera Status number:%d", uiLen);
    if(uiLen <= 0)
    {
        return;
    }
    
    m_pCameraStatusInfo = new CameraStatusInfoT[uiLen];
    if(m_pCameraStatusInfo == NULL)
    {
        _ERROR_("New memory failed!");
        return;
    }
    m_uiCameraStatusCount = uiLen;
    for(UINT32 i=0; i<m_uiCameraStatusCount; i++)
    {
        m_pCameraStatusInfo[i].bChecked = pCamStatus[i].bStrmCheckFlag== 1? true : false;
        m_pCameraStatusInfo[i].bCheckedOK = pCamStatus[i].iStrmCheckResult==0 ? true : false;
        m_pCameraStatusInfo[i].ubiCamID = pCamStatus[i].ubiCamraId;
        m_pCameraStatusInfo[i].uiAccessMode = pCamStatus[i].uiAccessMode;
        m_pCameraStatusInfo[i].bVideoOK = pCamStatus[i].bVideoLost==0 ? true : false;
    }
    m_iStateGetCamStatus = STATE_OK;
}

void CClient::OnStartLive(void * pBody)
{
    RetStartLive* pRetLive = (RetStartLive*)pBody;
    UINT32 iIndex = 0;
    //pthread_mutex_lock(&m_lockClientLive);
    for(UINT32 i=0; i<m_uiClientLiveInfoCount; i++)
    {
        if(m_listClientLiveInfo[i].pClientLive->m_ubiCamID == pRetLive->ubiCameraId &&
           m_listClientLiveInfo[i].pClientLive->m_ubiWndID == pRetLive->ubiWndId
        )
        {
            iIndex = i;
            break;
        }
    }
    if(iIndex >= m_uiClientLiveInfoCount)
    {
        //pthread_mutex_unlock(&m_lockClientLive);
        _ERROR_("can not find live info [cam:%llu window:%llu]", pRetLive->ubiCameraId, pRetLive->ubiWndId);
        return;
    }

    m_listClientLiveInfo[iIndex].pClientLive->NoteOnStartLive();
    if(pRetLive->ret.iValue != 0)
    {
        _ERROR_("Start live failed[cam:%llu window:%llu], [%d]:%s", pRetLive->ubiCameraId, pRetLive->ubiWndId, pRetLive->ret.iValue, pRetLive->ret.description);
        m_listClientLiveInfo[iIndex].pClientLive->SetStartLiveStatus(SC_FAILED);
        //pthread_mutex_unlock(&m_lockClientLive);
        return;
    }

    _DEBUG_("Start live OK[cam:%llu window:%llu]", pRetLive->ubiCameraId, pRetLive->ubiWndId);
    m_listClientLiveInfo[iIndex].pClientLive->SetStartLiveStatus(SC_OK);
    // 控制接收码流
    m_listClientLiveInfo[iIndex].pClientLive->Start(pRetLive->szPlayAddr, pRetLive->uiPlayPort);
    //pthread_mutex_unlock(&m_lockClientLive);
}

void CClient::OnStopLive(void * pBody)
{
    
}

void CClient::OnLogin(void * pBody)
{
    RetUserLogin *pRetLogin = (RetUserLogin*)pBody;
    if(pRetLogin->ret.iValue != 0)
    {
        _ERROR_("Login [mvp %s:%d] [user:%s] Failed, [%d]: %s", m_szMvpIP, m_uiMvpPort, m_pClientInfo->szUser, pRetLogin->ret.iValue, pRetLogin->ret.description);
        m_bLogined = false;
        // 重连
        // 结束重连线程并重启重连线程
        StartReconnectThread();
    }
    else
    {
        _DEBUG_("Login [mvp %s:%d] [user:%s] OK!", m_szMvpIP, m_uiMvpPort, m_pClientInfo->szUser);
        m_bLogined = true;
        if(m_pClientInfo->uiFirstLoginTime == 0)
        {
            m_pClientInfo->uiFirstLoginTime = time(NULL);
        }
        m_pClientInfo->uiLoginTime = time(NULL);
        m_pClientInfo->uiLoginCount++;
    }
}

void CClient::OnLogout(void * pBody)
{
    RetUserLogout *pRetLogout = (RetUserLogout*)pBody;
    m_bLogined = false;
    if(pRetLogout->ret.iValue != 0)
    {
        _ERROR_("Logout [mvp %s:%d] [user:%s] Failed, [%d]: %s", m_szMvpIP, m_uiMvpPort, m_pClientInfo->szUser, pRetLogout->ret.iValue, (char*)pRetLogout->ret.description);
    }
    else
    {
         _DEBUG_("Logout [mvp %s:%d] [user:%s] OK!", m_szMvpIP, m_uiMvpPort, m_pClientInfo->szUser);
        
    }
}

void* CClient::ThWorker(void* param)
{
    CClient* pThis = (CClient*)param;
    while(pThis->m_bExit == false)
    {
        if(pThis->m_bLogined)
        {
            if(pThis->m_iStateGetCam == STATE_OK)
            {
                if(pThis->m_iStateGetCamStatus == STATE_OK)
                {
                    // 请求直播
                    // 获取可用相机列表
                    if(0 != pThis->GetUsedCamList())
                    {
                        continue;
                    }
                    pThis->StartLive();
                    break;
                }
                else if(pThis->m_iStateGetCamStatus == STATE_NONE)
                {
                    //获取相机状态
                    pThis->GetCameraStatus();
                }
            }
            else if(pThis->m_iStateGetCam == STATE_NONE)
            {
                // 获取相机列表
                pThis->GetCamera();
            }
        }
        CFacility::SleepMsec(1000);
    }
    
    return NULL;
}

void* CClient::ThRecvCmdTimeoutWorker(void* param)
{
    CClient* pThis = (CClient*)param;
    UINT32 uiTimeNow = 0;
    UINT32 uiTime = 0; //时间差
    while(pThis->m_bRecvCmdTimeoutThreadExit == false)
    {
        if(! pThis->m_bLogined)
        {
            CFacility::SleepMsec(100);
            continue;
        }
        // 判断请求直播是否超时
        //pthread_mutex_lock(&pThis->m_lockClientLive);
        if(pThis->m_uiClientLiveInfoCount > 0 && pThis->m_listClientLiveInfo != NULL)
        {
            uiTimeNow = time(NULL);
            for(UINT32 i=0; i<pThis->m_uiClientLiveInfoCount; i++)
            {
                uiTime = uiTimeNow - pThis->m_listClientLiveInfo[i].uiStartLiveTIme;
                // 如果正等待接收直播返回，并且等待时长超时
                if(pThis->m_listClientLiveInfo[i].pClientLive->GetStartLiveStatus()==SC_WAITING && uiTime > TIMEOUT_RECV_CMD)
                {
                    pThis->m_listClientLiveInfo[i].pClientLive->SetStartLiveStatus(SC_TIMEOUT);
                    _DEBUG_("Recv OnStartLive timeout[cam:%llu wnd:%llu]", pThis->m_listClientLiveInfo[i].pClientLive->m_ubiCamID, pThis->m_listClientLiveInfo[i].pClientLive->m_ubiWndID);
                }
            }
        }
        //pthread_mutex_unlock(&pThis->m_lockClientLive);
        
        CFacility::SleepMsec(100);
    }

    return NULL;
}


// 直播控制线程，用于重连，巡检下不重连
void* CClient::ThReStartLiveWorker(void* param)
{
    CClient* pThis = (CClient*)param;
    int iRet = 0;
    UINT64 ubiCameraId,ubiWndId = 0;
    INT32 iUsedCamInfoIndex = 0;
    while(pThis->m_bReStartLiveThreadExit == false && pThis->m_pClientInfo->bScan == false && pThis->m_pClientInfo->bReStartLive)
    {
        if(pThis->m_bStartLiveEnd == false)
        {
            // 当前所有请求直播业务未结束时，不进行重连直播
            CFacility::SleepMsec(100);
            continue;
        }
        //pthread_mutex_lock(&pThis->m_lockClientLive);
        for(UINT32 i=0; i<pThis->m_uiClientLiveInfoCount; i++)
        {
            if( ! pThis->m_bLogined )
            {
                break;
            }
            if(pThis->m_bReStartLiveThreadExit)
            {
                //pthread_mutex_unlock(&pThis->m_lockClientLive);
                return NULL;
            }

            // 仅针对请求直播成功，同时超时未收到流的进行重新切换另一路直播
            if(pThis->m_listClientLiveInfo[i].pClientLive->GetStartLiveStatus() == SC_OK &&
               pThis->m_listClientLiveInfo[i].pClientLive->GetLiveStatus() == LIVE_TIMEOUT)
            {
                // 获取下一个有效相机ID
                iUsedCamInfoIndex = pThis->GetNextUsedCamInfoIndex();
                if(iUsedCamInfoIndex == -1)
                {
                    break;
                }
                ubiCameraId = pThis->m_pUsedCamInfo[iUsedCamInfoIndex].ubiCamID;
                // 窗口ID，维持不变
                ubiWndId = pThis->m_listClientLiveInfo[i].pClientLive->m_ubiWndID;
                // 先停止之前的直播
                pThis->m_listClientLiveInfo[i].pClientLive->Stop();
                pThis->m_listClientLiveInfo[i].pClientLive->m_ubiCamID = 0;
                pThis->m_listClientLiveInfo[i].pClientLive->SetStartLiveStatus(SC_NONE);
                pThis->m_listClientLiveInfo[i].uiStartLiveTIme = 0;
                pThis->m_listClientLiveInfo[i].pClientLive->SetCamInfo(NULL);
                CFacility::SleepMsec(100);
                pThis->m_listClientLiveInfo[i].pClientLive->m_ubiCamID = ubiCameraId;
                pThis->m_listClientLiveInfo[i].uiStartLiveTIme = time(NULL);
                pThis->m_listClientLiveInfo[i].pClientLive->NoteStartLive();
                pThis->m_listClientLiveInfo[i].pClientLive->SetCamInfo(&pThis->m_pUsedCamInfo[iUsedCamInfoIndex]);
                // 请求直播
                _DEBUG_("restart live...[cam:%llu window:%llu]", ubiCameraId, ubiWndId);
                iRet = VasAPI_StartLive(pThis->m_hSdkHandle, ubiCameraId, ubiWndId,  0, 4, "", 0, 0);
                if(iRet != 0)
                {
                    pThis->m_listClientLiveInfo[i].pClientLive->SetStartLiveStatus(SC_ERROR);
                    _ERROR_("restart live failed, Call VasAPI_StartLive failed[cam:%llu window:%llu]", ubiCameraId, ubiWndId);
                }
                pThis->m_listClientLiveInfo[i].pClientLive->SetStartLiveStatus(SC_WAITING);
                CFacility::SleepMsec(10);
            }
        }
        //pthread_mutex_unlock(&pThis->m_lockClientLive);
        
        CFacility::SleepMsec(100);
    }
    
    return NULL;

}
// 巡检
void* CClient::ThScanWorker(void* param)
{
    CClient* pThis = (CClient*)param;
    while(pThis->m_bScanThreadExit == false && pThis->m_pClientInfo->bScan == true)
    {
        // 已登录并且所有直播已请求完成，则开始巡检
        if(pThis->m_bLogined && pThis->m_bStartLiveEnd)
        {
            // 循环睡眠等待巡检切换
            for(UINT32 i=0; i<(pThis->m_pClientInfo->uiScanTimeInterval * 5); i++)
            {
                if(pThis->m_bScanThreadExit)
                {
                    return NULL;
                }
                else
                {
                    CFacility::SleepMsec(200);
                }
            }
            // 巡检统计
            pThis->CountScanStatusInfo();
            // 开始下一次巡检
            _DEBUG_("######### Start Scan[mvp:%s user:%s timespan:%ds].", pThis->m_szMvpIP, pThis->m_pClientInfo->szUser, pThis->m_pClientInfo->uiScanTimeInterval);
            pThis->StopLive();
            CFacility::SleepMsec(1000);
            pThis->StartLive();
        }
        else
        {
            CFacility::SleepMsec(100);
        }
    }
    
    return NULL;
}

int CClient::CountScanStatusInfo()
{
    if( !m_bLogined || !m_pClientInfo->bScan )
    {
        return -1;
    }
    m_sScanStatusInfo.sLiveStausCntInfo.uiLiveCount += m_uiClientLiveInfoCount;
    if(m_sScanStatusInfo.sLiveStausCntInfo.uiLiveCount <= 0)
    {
        return -1;
    }
    LiveStatusT* pLiveStatusData = new LiveStatusT[m_sScanStatusInfo.sLiveStausCntInfo.uiLiveCount];
    if(pLiveStatusData == NULL)
    {
        _ERROR_("New memory failed!");
        return -1;
    }
    
    m_sScanStatusInfo.uiScanTimes++;
    for(UINT32 i=0; i<m_uiClientLiveInfoCount; i++)
    {
        if(m_listClientLiveInfo[i].pClientLive->GetStauts(&pLiveStatusData[i], false) != 0)
        {
            continue;
        }
        switch(pLiveStatusData[i].statusStartLive)
        {
            case SC_NONE:
                m_sScanStatusInfo.sLiveStausCntInfo.uiSLNoneCnt++;
                break;
            case SC_OK:
                m_sScanStatusInfo.sLiveStausCntInfo.uiSLOKCnt++;
                break;
            case SC_FAILED:
                m_sScanStatusInfo.sLiveStausCntInfo.uiSLFailCnt++;
                break;
            case SC_WAITING:
                m_sScanStatusInfo.sLiveStausCntInfo.uiSLWaitingCnt++;
                break;
            case SC_TIMEOUT:
                m_sScanStatusInfo.sLiveStausCntInfo.uiSLTimeoutCnt++;
                break;
            case SC_ERROR:
                m_sScanStatusInfo.sLiveStausCntInfo.uiSLErrorCnt++;
                break;
            default:
                break;
        }
        
        switch(pLiveStatusData[i].statusLive)
        {
            case LIVE_OK:
                m_sScanStatusInfo.sLiveStausCntInfo.uiLiveOKCnt++;
                break;
            case LIVE_WAITING:
                m_sScanStatusInfo.sLiveStausCntInfo.uiLiveWaitingCnt++;
                break;
            case LIVE_TIMEOUT:
                m_sScanStatusInfo.sLiveStausCntInfo.uiLiveTimeoutCnt++;
                break;
            default:
                break;
        }
        // 统计MVP已自检且在线或者码流正常，但本程序自检不成功的个数
        if(
            ((pLiveStatusData[i].bStrmMvpChecked && pLiveStatusData[i].bStrmMvpCheckedOK) ||
             pLiveStatusData[i].bMvpVideoOK)
            && 
            pLiveStatusData[i].bStrmMyCheckedOK == false)
        {
            m_sScanStatusInfo.sLiveStausCntInfo.uiMvpOkButNotOKCnt++;
        }
    }
    
    if(pLiveStatusData != NULL)
    {
        delete [] pLiveStatusData;
        pLiveStatusData = NULL;
    }
    return 0;   

}

int CClient::GetUsedCamList()
{
    if(m_pUsedCamInfo != NULL)
    {
        delete [] m_pUsedCamInfo;
        m_pUsedCamInfo = NULL;
        m_uiUsedCamCount = 0;
    }
    m_uiIndexUsedCamInfo = 0;
    if(m_pCamerainfo == NULL)
    {
        return -1;
    }
    else
    {
        if(m_pCameraStatusInfo == NULL)
        {
            m_pUsedCamInfo = new CameraInfoT[m_uiCameraCount];
            if(m_pUsedCamInfo == NULL)
            {
                _DEBUG_("New Memory failed!");
                return -1;
            }
            m_uiUsedCamCount = m_uiCameraCount;
            memcpy(m_pUsedCamInfo, m_pCamerainfo, sizeof(CameraInfoT)*m_uiUsedCamCount);
            return 0;
        }
    }

    list<CameraInfoT*> listUsedCamInfo;
    listUsedCamInfo.clear();
    UINT32 i,j = 0;
    for(i=0; i<m_uiCameraCount; i++)
    {
        for(j=0; j<m_uiCameraStatusCount; j++)
        {
            if(m_pCamerainfo[i].ubiCamID == m_pCameraStatusInfo[j].ubiCamID && 
               (m_pCameraStatusInfo[j].uiAccessMode==0 || m_pCameraStatusInfo[j].uiAccessMode==1) //接入类型未设置或者未自建接入的
            )
            {
                
                if( (m_pCameraStatusInfo[j].bChecked && m_pCameraStatusInfo[j].bCheckedOK) ||
                    m_pCameraStatusInfo[j].bChecked == false
                )
                {
                    CameraInfoT *pTmpCamInfo = new CameraInfoT;
                    pTmpCamInfo->ubiCamID = m_pCameraStatusInfo[j].ubiCamID;
                    pTmpCamInfo->bStrmMvpChecked = m_pCameraStatusInfo[j].bChecked;
                    pTmpCamInfo->bStrmMvpCheckedOK = m_pCameraStatusInfo[j].bCheckedOK;
                    pTmpCamInfo->bVideoOK = m_pCameraStatusInfo[j].bVideoOK;
                    listUsedCamInfo.push_back(pTmpCamInfo);
                    break;
                }
            }
        }
    }

    list<CameraInfoT*>::iterator it;
    CameraInfoT* pCamInfo = NULL;
    // 如果未找到有效相机，则使用登录后获取到的相机列表
    if(listUsedCamInfo.size() > 0)
    {
        m_pUsedCamInfo = new CameraInfoT[listUsedCamInfo.size()];
        if(m_pUsedCamInfo == NULL)
        {
            _DEBUG_("New Memory failed!");

            for(it=listUsedCamInfo.begin(); it!=listUsedCamInfo.end(); it++)
            {
                pCamInfo = *it;
                if(pCamInfo != NULL)
                {
                    delete pCamInfo;
                    pCamInfo = NULL;
                }
            }
            listUsedCamInfo.clear();
            
            return -1;
        }
        
        m_uiUsedCamCount = listUsedCamInfo.size();
        UINT32 iIndex = 0;
        for(it=listUsedCamInfo.begin(); it!=listUsedCamInfo.end(); it++)
        {
            pCamInfo = *it;
            memcpy(&m_pUsedCamInfo[iIndex++], pCamInfo, sizeof(CameraInfoT));
        }
    }
    else
    {
        m_pUsedCamInfo = new CameraInfoT[m_uiCameraCount];
        if(m_pUsedCamInfo == NULL)
        {
            _ERROR_("New Memory failed!");
            return -1;
        }
        m_uiUsedCamCount = m_uiCameraCount;
        memcpy(m_pUsedCamInfo, m_pCamerainfo, sizeof(CameraInfoT)*m_uiUsedCamCount);
    }
    
    for(it=listUsedCamInfo.begin(); it!=listUsedCamInfo.end(); it++)
    {
        pCamInfo = *it;
        if(pCamInfo != NULL)
        {
            delete pCamInfo;
            pCamInfo = NULL;
        }
    }
    listUsedCamInfo.clear();

    // 打印有效的相机列表
    _INFO_("###### Get used Camera List[%d]: ", m_uiUsedCamCount);
    /*
    for(i=0; i<m_uiUsedCamCount; i++)
    {
        _INFO_("[%d] cam: %llu, checked: %s, online: %s", i+1, m_pUsedCamInfo[i].ubiCamID, \
                m_pUsedCamInfo[i].bStrmMvpChecked ? "yes" : "no", m_pUsedCamInfo[i].bStrmMvpCheckedOK ? "yes" : "no");
    }
    */
    
    return 0;
}

int CClient::GetNextUsedCamInfoIndex()
{
    if(m_pUsedCamInfo == NULL)
    {
        return -1;
    }
    UINT32 uiRetIndex = 0;

    // 顺序切换
    if( m_pClientInfo->bRandomSwitch == false)
    {
        uiRetIndex = m_uiIndexUsedCamInfo;
        m_uiIndexUsedCamInfo++;
        if(m_uiIndexUsedCamInfo >= m_uiUsedCamCount)
        {
            m_uiIndexUsedCamInfo = 0;
        }
    }
    else // 随机切换
    {
        // 取当前时间微秒数作为随机种子
        struct timeval tv;
        gettimeofday(&tv, NULL);
        srand(tv.tv_usec);
        uiRetIndex = rand() % m_uiUsedCamCount; // 0至可用相机数
    }
    return uiRetIndex;
}

// 启动重连线程
int CClient::StartReconnectThread()
{
    // 结束重连线程并重启重连线程
    m_bReconnectThreadExit = true;
    if(m_thReconnectWorker != 0)
    {
        pthread_join(m_thReconnectWorker, NULL);
        m_thReconnectWorker = 0;
    }
    
    if(! m_pClientInfo->bReLogin)
    {
        _ERROR_("Disconnect from MVP[%s:%d], ##### EXIT ####", m_szMvpIP, m_uiMvpPort);
        exit(1);
    }
    
    m_bReconnectThreadExit = false;
    pthread_create(&m_thReconnectWorker, NULL, ThReconnectWorker, (void*)this);

    return 0;
}

// 启动巡检线程
int CClient::StartScanThread()
{
    // 启动巡检线程
    if( m_pClientInfo->bScan )
    {
        m_bScanThreadExit = true;
        if(m_thScanWorker != 0)
        {
            pthread_join(m_thScanWorker, NULL);
            m_thScanWorker = 0;
        }
        m_bScanThreadExit = false;
        pthread_create(&m_thScanWorker, NULL, ThScanWorker, (void*)this);
    }
    return 0;
}

// 启动直播重连线程
int CClient::StartLiveThread()
{
    // 启动直播重连线程
    if( m_pClientInfo->bScan == false && m_pClientInfo->bReStartLive)
    {
        m_bReStartLiveThreadExit = true;
        if(m_thReStartLiveWorker != 0)
        {
            pthread_join(m_thReStartLiveWorker, NULL);
            m_thReStartLiveWorker = 0;
        }
        m_bReStartLiveThreadExit = false;
        pthread_create(&m_thReStartLiveWorker, NULL, ThReStartLiveWorker, (void*)this);
    }
    return 0;
}

int CClient::StartThread()
{
    m_bExit = true;
    if (m_thWorker != 0)
    {
        pthread_join(m_thWorker, NULL);
        m_thWorker = 0;
    }
    m_bExit = false;
    pthread_create(&m_thWorker, NULL, ThWorker, (void*)this);

    return 0;
}




