#include "MvpSession.h"

CMvpSession::CMvpSession()
{
    m_listClient = NULL;
    m_pMvpSessionInfo = NULL;
    m_bOK = false;
}

CMvpSession::CMvpSession(sMvpSessionInfoT *pSession)
{
    m_pMvpSessionInfo = pSession;
    m_listClient = new CClient[m_pMvpSessionInfo->uiClients];
}

CMvpSession::~CMvpSession()
{
    if(m_listClient != NULL)
    {
        delete [] m_listClient;
        m_listClient = NULL;
    }
}

int CMvpSession::SetInfo(sMvpSessionInfoT *pSession)
{
    if(m_pMvpSessionInfo == NULL)
    {
        m_pMvpSessionInfo = pSession;
    }
    else
    {
        // 是否重设，待定
    }
    
    if(m_listClient == NULL)
    {
        m_listClient = new CClient[m_pMvpSessionInfo->uiClients];
    }
    else
    {
        // 是否重设，待定
    }
    return 0;
}


// 启动服务
int CMvpSession::Start()
{
    if(BuildClients() != 0)
    {
        m_bOK = false;
        return -1;
    }
    if(StartClients() != 0)
    {
        m_bOK = false;
        return -1;
    }
    m_bOK = true;
    return 0;
}

// 停止服务
int CMvpSession::Stop()
{
    StopClients();
    m_bOK = false;
    return 0;
}

// 获取状态
int CMvpSession::GetStatus(MvpSessionStatusT* pStatus)
{
    pStatus->bOK = m_bOK;
    sprintf(pStatus->szIP, "%s", m_pMvpSessionInfo->szIP);
    pStatus->uiPort = m_pMvpSessionInfo->uiPort;
    memset(&pStatus->sLiveStausCntInfo, 0, sizeof(LiveStatusCountInfoT));
    memset(&pStatus->sScanStatusInfo, 0, sizeof(ScanStatusInfoT));
    pStatus->fStreamRate = 0;
    pStatus->uiCount = m_pMvpSessionInfo->uiClients;
    if(pStatus->uiCount <= 0)
    {
        pStatus->pClientStatusData = NULL;
        return 0;
    }
    
    pStatus->pClientStatusData = new ClientStatusT[pStatus->uiCount];
    if(pStatus->pClientStatusData == NULL)
    {
        _ERROR_("New memory failed!");
        return -1;
    }
    
    for(UINT32 i=0; i<pStatus->uiCount; i++)
    {
        if(m_listClient[i].GetStatus(&pStatus->pClientStatusData[i]) != 0)
        {
            goto goto_failed;
        }

        //mvp的巡检状态统计
        pStatus->sScanStatusInfo.uiScanTimes += pStatus->pClientStatusData[i].sScanStatusInfo.uiScanTimes;
        pStatus->sScanStatusInfo.sLiveStausCntInfo.uiLiveCount += pStatus->pClientStatusData[i].sScanStatusInfo.sLiveStausCntInfo.uiLiveCount;
        pStatus->sScanStatusInfo.sLiveStausCntInfo.uiSLOKCnt += pStatus->pClientStatusData[i].sScanStatusInfo.sLiveStausCntInfo.uiSLOKCnt;
        pStatus->sScanStatusInfo.sLiveStausCntInfo.uiSLNoneCnt += pStatus->pClientStatusData[i].sScanStatusInfo.sLiveStausCntInfo.uiSLNoneCnt;
        pStatus->sScanStatusInfo.sLiveStausCntInfo.uiSLFailCnt += pStatus->pClientStatusData[i].sScanStatusInfo.sLiveStausCntInfo.uiSLFailCnt;
        pStatus->sScanStatusInfo.sLiveStausCntInfo.uiSLWaitingCnt += pStatus->pClientStatusData[i].sScanStatusInfo.sLiveStausCntInfo.uiSLWaitingCnt;
        pStatus->sScanStatusInfo.sLiveStausCntInfo.uiSLTimeoutCnt += pStatus->pClientStatusData[i].sScanStatusInfo.sLiveStausCntInfo.uiSLTimeoutCnt;
        pStatus->sScanStatusInfo.sLiveStausCntInfo.uiSLErrorCnt += pStatus->pClientStatusData[i].sScanStatusInfo.sLiveStausCntInfo.uiSLErrorCnt;
        pStatus->sScanStatusInfo.sLiveStausCntInfo.uiLiveWaitingCnt += pStatus->pClientStatusData[i].sScanStatusInfo.sLiveStausCntInfo.uiLiveWaitingCnt;
        pStatus->sScanStatusInfo.sLiveStausCntInfo.uiLiveOKCnt += pStatus->pClientStatusData[i].sScanStatusInfo.sLiveStausCntInfo.uiLiveOKCnt;
        pStatus->sScanStatusInfo.sLiveStausCntInfo.uiLiveTimeoutCnt += pStatus->pClientStatusData[i].sScanStatusInfo.sLiveStausCntInfo.uiLiveTimeoutCnt;
        pStatus->sScanStatusInfo.sLiveStausCntInfo.uiMvpOkButNotOKCnt += pStatus->pClientStatusData[i].sScanStatusInfo.sLiveStausCntInfo.uiMvpOkButNotOKCnt;
        
        // mvp的码流接收速率
        pStatus->fStreamRate += pStatus->pClientStatusData[i].fStreamRate;
        // mvp的直播信息状态统计
        pStatus->sLiveStausCntInfo.uiLiveCount += pStatus->pClientStatusData[i].sLiveStausCntInfo.uiLiveCount;
        pStatus->sLiveStausCntInfo.uiSLOKCnt += pStatus->pClientStatusData[i].sLiveStausCntInfo.uiSLOKCnt;
        pStatus->sLiveStausCntInfo.uiSLNoneCnt += pStatus->pClientStatusData[i].sLiveStausCntInfo.uiSLNoneCnt;
        pStatus->sLiveStausCntInfo.uiSLFailCnt += pStatus->pClientStatusData[i].sLiveStausCntInfo.uiSLFailCnt;
        pStatus->sLiveStausCntInfo.uiSLWaitingCnt += pStatus->pClientStatusData[i].sLiveStausCntInfo.uiSLWaitingCnt;
        pStatus->sLiveStausCntInfo.uiSLTimeoutCnt += pStatus->pClientStatusData[i].sLiveStausCntInfo.uiSLTimeoutCnt;
        pStatus->sLiveStausCntInfo.uiSLErrorCnt += pStatus->pClientStatusData[i].sLiveStausCntInfo.uiSLErrorCnt;
        pStatus->sLiveStausCntInfo.uiLiveWaitingCnt += pStatus->pClientStatusData[i].sLiveStausCntInfo.uiLiveWaitingCnt;
        pStatus->sLiveStausCntInfo.uiLiveOKCnt += pStatus->pClientStatusData[i].sLiveStausCntInfo.uiLiveOKCnt;
        pStatus->sLiveStausCntInfo.uiLiveTimeoutCnt += pStatus->pClientStatusData[i].sLiveStausCntInfo.uiLiveTimeoutCnt;
        pStatus->sLiveStausCntInfo.uiMvpOkButNotOKCnt += pStatus->pClientStatusData[i].sLiveStausCntInfo.uiMvpOkButNotOKCnt;
    }
    
    return 0;

goto_failed:
    if(pStatus->pClientStatusData != NULL)
    {
        delete [] pStatus->pClientStatusData;
        pStatus->pClientStatusData = NULL;
    }
    pStatus->uiCount = 0;
    return -1;
}

int CMvpSession::BuildClients()
{
    if(m_listClient == NULL)
    {
        return -1;
    }
    for(unsigned int i=0; i<m_pMvpSessionInfo->uiClients; i++)
    {
        m_listClient[i].SetInfo(m_pMvpSessionInfo->szIP, m_pMvpSessionInfo->uiPort, &m_pMvpSessionInfo->pClients[i]);
    }
    return 0;
}

int CMvpSession::StartClients()
{
    for(unsigned int i=0; i<m_pMvpSessionInfo->uiClients; i++)
    {
        m_listClient[i].Start();
        CFacility::SleepMsec(20);
    }

    return 0;
}

int CMvpSession::StopClients()
{
    for(unsigned int i=0; i<m_pMvpSessionInfo->uiClients; i++)
    {
        m_listClient[i].Stop();
        CFacility::SleepMsec(20);
    }
    return 0;
}



