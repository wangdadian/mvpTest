#include "MvpSessionBuilder.h"

// ������Ϣ�Ƿ��¼�ļ�
bool g_bDebugToFile;
// ������Ϣ�Ƿ��ն˴�ӡ
bool g_bDebugToConsole;

UINT64 CMvpSessionBuilder::m_ubiWndIndex = 1;
CMvpSessionBuilder::CMvpSessionBuilder(const char* cfgfile)
{
    sprintf(m_szCfgFile, "%s", cfgfile);
    m_pXmlDoc = new TiXmlDocument(m_szCfgFile);
	m_listMvpSession = NULL;
	m_uiMvpSessionCount = 0;
	m_listMvpSessionInfo.clear();
	m_bExit = false;
	m_thWorker = 0;
	m_pMvpStatus = NULL;
	m_bOK = false;
	memset(m_szTimeCallStart, 0, sizeof(m_szTimeCallStart));
	m_sConsoleSCI.bDetail = m_sFileSCI.bDetail = false;
	m_sConsoleSCI.bEnabled = m_sFileSCI.bEnabled = false;
	m_sConsoleSCI.uiTimeInterval = m_sFileSCI.uiTimeInterval = 5;
}

CMvpSessionBuilder::~CMvpSessionBuilder()
{
    memset(m_szTimeCallStart, 0, sizeof(m_szTimeCallStart));

    DeleteMvpStatusInfo();    
    DeleteMvpSessionInfo();
    if(m_pXmlDoc != NULL)
    {
        delete m_pXmlDoc;
        m_pXmlDoc = NULL;
    }
    if(m_listMvpSession != NULL)
    {
        delete [] m_listMvpSession;
        m_listMvpSession = NULL;
    }

    m_bExit = true;
    if(m_thWorker != 0)
	{
		pthread_join(m_thWorker, NULL);
		m_thWorker = 0;
	}
}

void CMvpSessionBuilder::DeleteMvpSessionInfo()
{
    sMvpSessionInfoT* pMvpSession = NULL;
    sClientInfoT* pClientInfo = NULL;
    list<sMvpSessionInfoT*>::iterator it;
    for (it=m_listMvpSessionInfo.begin(); it!=m_listMvpSessionInfo.end(); ++it)
    {
        pMvpSession = *it;
        if(pMvpSession->pClients == NULL)
        {
            continue;
        }
        pClientInfo = (sClientInfoT*)pMvpSession->pClients;
        if(pClientInfo != NULL)
        {
            delete [] pClientInfo;
            pClientInfo = NULL;
        }
        delete pMvpSession;
        pMvpSession = NULL;
    }
    m_listMvpSessionInfo.clear();
    m_uiMvpSessionCount = 0;
}

int CMvpSessionBuilder::Start()
{
    memset(m_szTimeCallStart, 0, sizeof(m_szTimeCallStart));
    sprintf(m_szTimeCallStart, "%s", CFacility::GetCurTime());
    // ����XML
    if(LoadConfigInfo() != 0)
    {
        return -1;
    }
    // ����session
    if(BuildMvpSession() != 0)
    {
        return -1;
    }
    // ��������
    if(StartMvpSession() != 0)
    {
        return -1;
    }
    m_bOK = true;
    m_bExit = false;
    
    pthread_create( &m_thWorker, NULL, ThWorker, (void*)this);
    return 0;
}

int CMvpSessionBuilder::Stop()
{
    m_bOK = false;
    m_ubiWndIndex = 0;
    StopMvpSession();
    DeleteMvpStatusInfo();
    DeleteMvpSessionInfo();
    
    if(m_listMvpSession != NULL)
    {
        delete [] m_listMvpSession;
        m_listMvpSession = NULL;
    }

    m_bExit = true;
    if(m_thWorker != 0)
	{
		pthread_join(m_thWorker, NULL);
		m_thWorker = 0;
	}
    return 0;
}

// ��ȡ״̬��Ϣ
int CMvpSessionBuilder::GetStatus(MvpStatusT* pStatus)
{
    if(m_bOK != true)
    {
        return -1;
    }

    memset(&pStatus->sLiveStausCntInfo, 0, sizeof(LiveStatusCountInfoT));
    pStatus->fStreamRate = 0;
    pStatus->uiClientCount = 0;
    pStatus->uiCount = m_uiMvpSessionCount;
    if(pStatus->uiCount <= 0)
    {
        pStatus->pMvpSessionStatusData = NULL;
        return -1;
    }
    memset(&pStatus->sLiveStausCntInfo, 0, sizeof(LiveStatusCountInfoT));
    memset(&pStatus->sScanStatusInfo, 0, sizeof(ScanStatusInfoT));

    pStatus->pMvpSessionStatusData = new MvpSessionStatusT[pStatus->uiCount];
    if(pStatus->pMvpSessionStatusData == NULL)
    {
        _ERROR_("New memory failed!");
        return -1;
    }
    for(UINT32 i=0; i<pStatus->uiCount; i++)
    {
        if(m_listMvpSession[i].GetStatus(&pStatus->pMvpSessionStatusData[i]) != 0)
        {
            goto goto_failed;
        }

        // ����mvp��Ѳ��ͳ��
        pStatus->sScanStatusInfo.uiScanTimes += pStatus->pMvpSessionStatusData[i].sScanStatusInfo.uiScanTimes;
        pStatus->sScanStatusInfo.sLiveStausCntInfo.uiLiveCount += pStatus->pMvpSessionStatusData[i].sScanStatusInfo.sLiveStausCntInfo.uiLiveCount;
        pStatus->sScanStatusInfo.sLiveStausCntInfo.uiSLOKCnt += pStatus->pMvpSessionStatusData[i].sScanStatusInfo.sLiveStausCntInfo.uiSLOKCnt;
        pStatus->sScanStatusInfo.sLiveStausCntInfo.uiSLNoneCnt += pStatus->pMvpSessionStatusData[i].sScanStatusInfo.sLiveStausCntInfo.uiSLNoneCnt;
        pStatus->sScanStatusInfo.sLiveStausCntInfo.uiSLFailCnt += pStatus->pMvpSessionStatusData[i].sScanStatusInfo.sLiveStausCntInfo.uiSLFailCnt;
        pStatus->sScanStatusInfo.sLiveStausCntInfo.uiSLWaitingCnt += pStatus->pMvpSessionStatusData[i].sScanStatusInfo.sLiveStausCntInfo.uiSLWaitingCnt;
        pStatus->sScanStatusInfo.sLiveStausCntInfo.uiSLTimeoutCnt += pStatus->pMvpSessionStatusData[i].sScanStatusInfo.sLiveStausCntInfo.uiSLTimeoutCnt;
        pStatus->sScanStatusInfo.sLiveStausCntInfo.uiSLErrorCnt += pStatus->pMvpSessionStatusData[i].sScanStatusInfo.sLiveStausCntInfo.uiSLErrorCnt;
        pStatus->sScanStatusInfo.sLiveStausCntInfo.uiLiveWaitingCnt += pStatus->pMvpSessionStatusData[i].sScanStatusInfo.sLiveStausCntInfo.uiLiveWaitingCnt;
        pStatus->sScanStatusInfo.sLiveStausCntInfo.uiLiveOKCnt += pStatus->pMvpSessionStatusData[i].sScanStatusInfo.sLiveStausCntInfo.uiLiveOKCnt;
        pStatus->sScanStatusInfo.sLiveStausCntInfo.uiLiveTimeoutCnt += pStatus->pMvpSessionStatusData[i].sScanStatusInfo.sLiveStausCntInfo.uiLiveTimeoutCnt;
        pStatus->sScanStatusInfo.sLiveStausCntInfo.uiMvpOkButNotOKCnt += pStatus->pMvpSessionStatusData[i].sScanStatusInfo.sLiveStausCntInfo.uiMvpOkButNotOKCnt;
        
        // ����mvpֱ������
        pStatus->fStreamRate += pStatus->pMvpSessionStatusData[i].fStreamRate;
        
        // ����mvp�ĵ�ǰֱ��ͳ��
        pStatus->uiClientCount += pStatus->pMvpSessionStatusData[i].uiCount;
        pStatus->sLiveStausCntInfo.uiLiveCount += pStatus->pMvpSessionStatusData[i].sLiveStausCntInfo.uiLiveCount;
        pStatus->sLiveStausCntInfo.uiSLOKCnt += pStatus->pMvpSessionStatusData[i].sLiveStausCntInfo.uiSLOKCnt;
        pStatus->sLiveStausCntInfo.uiSLNoneCnt += pStatus->pMvpSessionStatusData[i].sLiveStausCntInfo.uiSLNoneCnt;
        pStatus->sLiveStausCntInfo.uiSLFailCnt += pStatus->pMvpSessionStatusData[i].sLiveStausCntInfo.uiSLFailCnt;
        pStatus->sLiveStausCntInfo.uiSLWaitingCnt += pStatus->pMvpSessionStatusData[i].sLiveStausCntInfo.uiSLWaitingCnt;
        pStatus->sLiveStausCntInfo.uiSLTimeoutCnt += pStatus->pMvpSessionStatusData[i].sLiveStausCntInfo.uiSLTimeoutCnt;
        pStatus->sLiveStausCntInfo.uiSLErrorCnt += pStatus->pMvpSessionStatusData[i].sLiveStausCntInfo.uiSLErrorCnt;
        pStatus->sLiveStausCntInfo.uiLiveWaitingCnt += pStatus->pMvpSessionStatusData[i].sLiveStausCntInfo.uiLiveWaitingCnt;
        pStatus->sLiveStausCntInfo.uiLiveOKCnt += pStatus->pMvpSessionStatusData[i].sLiveStausCntInfo.uiLiveOKCnt;
        pStatus->sLiveStausCntInfo.uiLiveTimeoutCnt += pStatus->pMvpSessionStatusData[i].sLiveStausCntInfo.uiLiveTimeoutCnt;
        pStatus->sLiveStausCntInfo.uiMvpOkButNotOKCnt += pStatus->pMvpSessionStatusData[i].sLiveStausCntInfo.uiMvpOkButNotOKCnt;
    }
    
    return 0;

goto_failed:
    if(pStatus->pMvpSessionStatusData != NULL)
    {
        delete [] pStatus->pMvpSessionStatusData;
        pStatus->pMvpSessionStatusData = NULL;
    }
    pStatus->uiCount = 0;
    return -1;
}

int CMvpSessionBuilder::LoadConfigInfo()
{
    if(m_pXmlDoc == NULL)
    {
        _ERROR_("xml object is NULL!");
        return -1;
    }
    if(m_pXmlDoc->LoadFile() != true)
    {
        _ERROR_("load file [%s] failed: %s!", m_szCfgFile, m_pXmlDoc->ErrorDesc());
        return -1;
    }
    DeleteMvpSessionInfo();
    m_ubiWndIndex = 1;
  
    TiXmlElement *pRootElement = m_pXmlDoc->RootElement();  //��Ԫ��, doc  
    //printf("[root name]  %s\n", RootElement->Value());

    //debug��¼����
    g_bDebugToFile = true;
    g_bDebugToConsole = true;
    TiXmlElement *logElement = pRootElement->FirstChildElement("debug");
    if(strcmp("true", logElement->Attribute("file")) != 0)
    {
        g_bDebugToFile = false;
    }
    if(strcmp("true", logElement->Attribute("console")) != 0)
    {
        g_bDebugToConsole = false;
    }

    // ͳ����Ϣ����
    TiXmlElement *statisticElement = pRootElement->FirstChildElement("statistic");
    if(statisticElement != NULL)
    {
        TiXmlElement *fileElement = statisticElement->FirstChildElement("file");
        TiXmlElement *consoleElement = statisticElement->FirstChildElement("console");
        if(fileElement != NULL)
        {
            if(strcmp("true", fileElement->Attribute("enabled")) == 0)
            {
                m_sFileSCI.bEnabled = true;
            }
            else
            {
                m_sFileSCI.bEnabled = false;
            }
            if(strcmp("true", fileElement->Attribute("detail")) == 0)
            {
                m_sFileSCI.bDetail= true;
            }
            else
            {
                m_sFileSCI.bDetail= false;
            }
            fileElement->QueryUnsignedAttribute("timeInterval", &m_sFileSCI.uiTimeInterval);
            if(m_sFileSCI.uiTimeInterval < 2)
            {
                m_sFileSCI.uiTimeInterval = 2;
            }
        }

        if(consoleElement != NULL)
        {
            if(strcmp("true", consoleElement->Attribute("enabled")) == 0)
            {
                m_sConsoleSCI.bEnabled = true;
            }
            else
            {
                m_sConsoleSCI.bEnabled = false;
            }
            if(strcmp("true", consoleElement->Attribute("detail")) == 0)
            {
                m_sConsoleSCI.bDetail= true;
            }
            else
            {
                m_sConsoleSCI.bDetail= false;
            }
            consoleElement->QueryUnsignedAttribute("timeInterval", &m_sConsoleSCI.uiTimeInterval);
            if(m_sConsoleSCI.uiTimeInterval < 2)
            {
                m_sConsoleSCI.uiTimeInterval = 2;
            }
        }
    }
      
    //�����ý���µ�mvp
    for(TiXmlElement *mvpElement = pRootElement->FirstChildElement("mvp");//��һ��mvp��Ԫ��  
        mvpElement != NULL;  
        mvpElement = mvpElement->NextSiblingElement("mvp"))//��һ��mvpԪ��  
    {  
        if(strcmp("true", mvpElement->Attribute("enabled")) != 0)
        {
            continue;
        }
        
        sMvpSessionInfoT *pMvp = new sMvpSessionInfoT;
        memset(pMvp, 0, sizeof(sMvpSessionInfoT));
        
        sprintf(pMvp->szIP, "%s", mvpElement->Attribute("ip"));
        mvpElement->QueryUnsignedAttribute("port", &pMvp->uiPort);
        if( !CFacility::bIpAddress(pMvp->szIP) || pMvp->uiPort<=0 || pMvp->uiPort>65525 )
        {
            _ERROR_("mvp ip or port[%s:%d] is invalid!", pMvp->szIP, pMvp->uiPort);
            exit(2);
        }

        // client��Ԫ��
        // ��ȡclient����
        UINT32 uiClientCount = 0;
        for(TiXmlElement *clientElement = mvpElement->FirstChildElement("client");
            clientElement != NULL;
            clientElement = clientElement->NextSiblingElement("client"))
        {
            if(strcmp("true", clientElement->Attribute("enabled")) != 0)
            {
                continue;
            }
            uiClientCount++;
        }
        
        if(uiClientCount == 0)
        {
            delete pMvp;
            pMvp = NULL;
            continue;
        }
        
        pMvp->uiClients = uiClientCount;
        pMvp->pClients = new sClientInfoT[uiClientCount];
        memset(pMvp->pClients, 0, sizeof(sClientInfoT)*uiClientCount);

        int iIndex = 0;
        for(TiXmlElement *clientElement = mvpElement->FirstChildElement("client");
            clientElement != NULL;
            clientElement = clientElement->NextSiblingElement("client"))
        {
            if(strcmp("true", clientElement->Attribute("enabled")) != 0)
            {
                continue;
            }
            // ����
            if(strcmp("true", clientElement->Attribute("scan")) == 0)
            {
                pMvp->pClients[iIndex].bScan = true;
            }
            else
            {
                pMvp->pClients[iIndex].bScan = false;
            }
            
            UINT32 uiScanTimeInterval = 0;
            clientElement->QueryUnsignedAttribute("scanTimeInterval", &uiScanTimeInterval);
            if(uiScanTimeInterval < 15)
            {
                uiScanTimeInterval = 15;
            }
            if(uiScanTimeInterval > 3600)
            {
                // ����ȡ��20180405
                //uiScanTimeInterval = 3600;
            }
            pMvp->pClients[iIndex].uiScanTimeInterval = uiScanTimeInterval;

            if(strcmp("true", clientElement->Attribute("reStartLive")) == 0)
            {
                pMvp->pClients[iIndex].bReStartLive = true;
            }
            else
            {
                pMvp->pClients[iIndex].bReStartLive = false;
            }

            if(strcmp("true", clientElement->Attribute("randomSwitch")) == 0)
            {
                pMvp->pClients[iIndex].bRandomSwitch = true;
            }
            else
            {
                pMvp->pClients[iIndex].bRandomSwitch = false;
            }
            
            // ��Ԫ��
            sprintf(pMvp->pClients[iIndex].szUser, "%s", clientElement->FirstChildElement("user")->GetText());
            sprintf(pMvp->pClients[iIndex].szPass, "%s", clientElement->FirstChildElement("pass")->GetText());
            UINT32 uiMaxWnd = 0;
            uiMaxWnd = (UINT32)atoi(clientElement->FirstChildElement("maxWnd")->GetText());
            if(uiMaxWnd <= 0)
            {
                uiMaxWnd = 1;
            }
            if(uiMaxWnd > 999)
            {
                uiMaxWnd = 10;
            }
            pMvp->pClients[iIndex].uiMaxWndCnt = uiMaxWnd;
            pMvp->pClients[iIndex].ubiWndIndex = m_ubiWndIndex;
            m_ubiWndIndex += pMvp->pClients[iIndex].uiMaxWndCnt;
            
            char szReLogin[80] = {0};
            sprintf(szReLogin, "%s", clientElement->FirstChildElement("reLogin")->GetText());
            if(strcmp("true", szReLogin) == 0)
            {
                pMvp->pClients[iIndex].bReLogin = true;
            }
            else
            {
                pMvp->pClients[iIndex].bReLogin = false;
            }

            // ��ʼ����������
            pMvp->pClients[iIndex].uiFirstLoginTime = 0;
            pMvp->pClients[iIndex].uiLoginTime = 0;
            pMvp->pClients[iIndex].uiLoginCount = 0;
            
            // ��������
            iIndex++;
        }
        
        m_listMvpSessionInfo.push_back(pMvp);
    }

    if(m_listMvpSessionInfo.empty())
    {
        _ERROR_("No invalid configure infomation from config file [%s]!", m_szCfgFile);
        return -1;
    }
    
    // ��ӡ����¼��־
    list<sMvpSessionInfoT*>::iterator it;
    sMvpSessionInfoT* pMvpSession = NULL;
    sClientInfoT* pClientInfo = NULL;
    _DEBUG_("CONFIG INFO[%s]:", m_szCfgFile);
    for(it=m_listMvpSessionInfo.begin(); it!=m_listMvpSessionInfo.end(); it++)
    {
        pMvpSession = *it;
        if(pMvpSession != NULL)
        {
            _DEBUG_("mvp IP=%s Port=%d Clients=%d", pMvpSession->szIP, pMvpSession->uiPort, pMvpSession->uiClients);
        }
        
        if(pMvpSession->pClients != NULL)
        {
            pClientInfo = (sClientInfoT*)pMvpSession->pClients;
            for(unsigned int i=0; i<pMvpSession->uiClients; i++)
            {
                _DEBUG_("client user=%s pass=%s maxWnd=%d wndIndex=%llu", pMvpSession->pClients[i].szUser,  \
                         pMvpSession->pClients[i].szPass, pMvpSession->pClients[i].uiMaxWndCnt, \
                         pMvpSession->pClients[i].ubiWndIndex);
            }
        }
    }

    return 0;
}

int CMvpSessionBuilder::BuildMvpSession()
{
    if(m_listMvpSessionInfo.empty())
    {
        return -1;
    }

    m_uiMvpSessionCount = m_listMvpSessionInfo.size();
    m_listMvpSession = new CMvpSession[m_uiMvpSessionCount];
    list<sMvpSessionInfoT*>::iterator it;
    sMvpSessionInfoT* pMvpSession = NULL;
    int iIndex = 0;
    for(it=m_listMvpSessionInfo.begin(); it!=m_listMvpSessionInfo.end(); it++)
    {
        pMvpSession = *it;
        if(pMvpSession != NULL)
        {
            m_listMvpSession[iIndex].SetInfo(pMvpSession);
        }
        else
        {
            continue;
        }
        iIndex++;
    }
    
    return 0;
}

int CMvpSessionBuilder::StartMvpSession()
{
    for(unsigned int i=0; i<m_uiMvpSessionCount; i++)
    {
        m_listMvpSession[i].Start();
    }
    
    return 0;
}

int CMvpSessionBuilder::StopMvpSession()
{
    for(unsigned int i=0; i<m_uiMvpSessionCount; i++)
    {
        m_listMvpSession[i].Stop();
    }

    return 0;
}

void CMvpSessionBuilder::DeleteMvpStatusInfo()
{
    if(m_pMvpStatus != NULL)
    {
        if(m_pMvpStatus->pMvpSessionStatusData != NULL)
        {
            for(UINT32 i=0; i<m_pMvpStatus->uiCount; i++)
            {
                if(m_pMvpStatus->pMvpSessionStatusData[i].pClientStatusData != NULL)
                {
                    for(UINT32 j=0; j<m_pMvpStatus->pMvpSessionStatusData[i].uiCount; j++)
                    {
                        if(m_pMvpStatus->pMvpSessionStatusData[i].pClientStatusData[j].pLiveStatusData != NULL)
                        {
                            delete [] m_pMvpStatus->pMvpSessionStatusData[i].pClientStatusData[j].pLiveStatusData;
                            m_pMvpStatus->pMvpSessionStatusData[i].pClientStatusData[j].pLiveStatusData = NULL;
                        }
                    }
                    
                    delete [] m_pMvpStatus->pMvpSessionStatusData[i].pClientStatusData;
                    m_pMvpStatus->pMvpSessionStatusData[i].pClientStatusData = NULL;
                }
            }
            
            delete [] m_pMvpStatus->pMvpSessionStatusData;
            m_pMvpStatus->pMvpSessionStatusData = NULL;
        }

        delete m_pMvpStatus;
        m_pMvpStatus = NULL;
    }
}

void* CMvpSessionBuilder::ThWorker(void* param)
{
    CMvpSessionBuilder* pThis = (CMvpSessionBuilder*)param;
    UINT32 uiTimeStart = time(NULL); // ����ʱ��
    UINT32 uiTimeNow = time(NULL); // ��ǰʱ��
    while(pThis->m_bExit == false)
    {
        uiTimeNow = time(NULL);
        // ͳ��״̬��Ϣ������ӡ����¼�ļ�(��һ��ͳ����������ʱʱ��֮��ʼͳ��)
        if((uiTimeNow - uiTimeStart) >= TIMEOUT_LIVE_NOSTREAM)
        {
            pThis->LogStatisticMvpStatus();
        }

        CFacility::SleepMsec(100);
    }
    return NULL;
}

int CMvpSessionBuilder::LogStatisticMvpStatus()
{
    // ��ӡ����¼�ļ���δ��Ч
    if(!m_sFileSCI.bEnabled && !m_sConsoleSCI.bEnabled)
    {
        return 0;
    }

    // ��ǰʱ��
    UINT32 uiTimeNow = time(NULL);
    // �ϴμ�¼�ļ�ʱ��
    static UINT32 uiTimeFileLast = time(NULL);
    // �ϴδ�ӡʱ��
    static UINT32 uiTimeConsoleLast = time(NULL);
    // ��ǰ�Ƿ����д���ļ�
    bool bLogToFileNow = false;
    // ��ǰ�Ƿ���ն˴�ӡ
    bool bLogToConsoleNow = false;

    // �ļ���¼����
    if(uiTimeNow - uiTimeFileLast >= m_sFileSCI.uiTimeInterval)
    {
        bLogToFileNow = true;
    }
    // ��ӡ����
    if(uiTimeNow - uiTimeConsoleLast >= m_sConsoleSCI.uiTimeInterval)
    {
        bLogToConsoleNow = true;
    }
    // δ����ӡ/��¼ʱ��㣬ֱ�ӷ���
    if(!bLogToConsoleNow && !bLogToFileNow)
    {
        return 0;
    }

    // ͳ����Ϣ��ӡ/��¼�ļ�
    DeleteMvpStatusInfo();
    m_pMvpStatus = new MvpStatusT;
    if(m_pMvpStatus == NULL)
    {
        _ERROR_("New memory failed!");
        return -1;
    }
    // ��ȡ״̬ʧ��
    if(0 != GetStatus(m_pMvpStatus))
    {
        delete m_pMvpStatus;
        m_pMvpStatus = NULL;
        return -1;
    }
    
    static char szSummaryLogBuff[4096]={0};
    memset(szSummaryLogBuff, 0, sizeof(szSummaryLogBuff));
    static char szDetailLogBuff[1024*1024]={0};
    memset(szDetailLogBuff, 0, sizeof(szDetailLogBuff));

    MvpSessionStatusT* pSessionStatusPtr = NULL;
    ClientStatusT* pClientStatusPtr = NULL;
    LiveStatusT* pLiveStatusPtr = NULL;
    
    // �ָ���
    sprintf(szSummaryLogBuff, "%s\n", "\n####################################################################################################");
    // ʱ��
    //sprintf(szSummaryLogBuff+strlen(szSummaryLogBuff), "Current Time: %s\n", CFacility::GetCurTime());
    sprintf(szSummaryLogBuff+strlen(szSummaryLogBuff), "%s\n", CFacility::GetCurTime());
    //sprintf(szSummaryLogBuff+strlen(szSummaryLogBuff), "Start   Time: %s\n", m_szTimeCallStart);

    // ������Ϣ
    sprintf(szSummaryLogBuff+strlen(szSummaryLogBuff), "\n%s\n", "Summary:");
    sprintf(szSummaryLogBuff+strlen(szSummaryLogBuff), "%d mvp, %d clients, %d lives, Total Rate: %.2f MByte/s\n", \
            m_pMvpStatus->uiCount, m_pMvpStatus->uiClientCount, m_pMvpStatus->sLiveStausCntInfo.uiLiveCount, \
            m_pMvpStatus->fStreamRate/1024.0);
    // ֱ��״̬���ͳ����Ϣ
    if(m_pMvpStatus->sLiveStausCntInfo.uiLiveCount > 0)
    {
        sprintf(szSummaryLogBuff+strlen(szSummaryLogBuff), "  StartLive: %d\n", m_pMvpStatus->sLiveStausCntInfo.uiLiveCount);
        UINT32 uiSuccessCount = m_pMvpStatus->sLiveStausCntInfo.uiLiveOKCnt + m_pMvpStatus->sLiveStausCntInfo.uiLiveWaitingCnt + \
                                  m_pMvpStatus->sLiveStausCntInfo.uiLiveTimeoutCnt;
        if(uiSuccessCount > 0)
        {
            sprintf(szSummaryLogBuff+strlen(szSummaryLogBuff), "    -Success: %d\n", uiSuccessCount);
            if(m_pMvpStatus->sLiveStausCntInfo.uiLiveOKCnt > 0)
            {
                sprintf(szSummaryLogBuff+strlen(szSummaryLogBuff), "      -%s: %d\n", varName(LIVE_OK), \
                        m_pMvpStatus->sLiveStausCntInfo.uiLiveOKCnt);
            }
            if(m_pMvpStatus->sLiveStausCntInfo.uiLiveTimeoutCnt > 0)
            {
                sprintf(szSummaryLogBuff+strlen(szSummaryLogBuff), "      -%s: %d\n", varName(LIVE_TIMEOUT), \
                        m_pMvpStatus->sLiveStausCntInfo.uiLiveTimeoutCnt);
            }
            if(m_pMvpStatus->sLiveStausCntInfo.uiLiveWaitingCnt > 0)
            {
                sprintf(szSummaryLogBuff+strlen(szSummaryLogBuff), "      -%s: %d\n", varName(LIVE_WAITING), \
                        m_pMvpStatus->sLiveStausCntInfo.uiLiveWaitingCnt);
            }
        }
        UINT32 uiFailedCount = m_pMvpStatus->sLiveStausCntInfo.uiSLNoneCnt + m_pMvpStatus->sLiveStausCntInfo.uiSLFailCnt + \
                                 m_pMvpStatus->sLiveStausCntInfo.uiSLWaitingCnt + m_pMvpStatus->sLiveStausCntInfo.uiSLErrorCnt + \
                                 m_pMvpStatus->sLiveStausCntInfo.uiSLTimeoutCnt;
        if(uiFailedCount > 0)
        {
            sprintf(szSummaryLogBuff+strlen(szSummaryLogBuff), "    -Failed: %d\n", uiFailedCount);
            if(m_pMvpStatus->sLiveStausCntInfo.uiSLTimeoutCnt > 0)
            {
                sprintf(szSummaryLogBuff+strlen(szSummaryLogBuff), "      -%s: %d\n", varName(SC_TIMEOUT), \
                        m_pMvpStatus->sLiveStausCntInfo.uiSLTimeoutCnt);
            }
            if(m_pMvpStatus->sLiveStausCntInfo.uiSLFailCnt > 0)
            {
                sprintf(szSummaryLogBuff+strlen(szSummaryLogBuff), "      -%s: %d\n", varName(SC_FAILED), \
                        m_pMvpStatus->sLiveStausCntInfo.uiSLFailCnt);
            }
            if(m_pMvpStatus->sLiveStausCntInfo.uiSLWaitingCnt > 0)
            {
                sprintf(szSummaryLogBuff+strlen(szSummaryLogBuff), "      -%s: %d\n", varName(SC_WAITING), \
                        m_pMvpStatus->sLiveStausCntInfo.uiSLWaitingCnt);
            }
            if(m_pMvpStatus->sLiveStausCntInfo.uiSLErrorCnt > 0)
            {
                sprintf(szSummaryLogBuff+strlen(szSummaryLogBuff), "      -%s: %d\n", varName(SC_ERROR), \
                        m_pMvpStatus->sLiveStausCntInfo.uiSLErrorCnt);
            }
            if(m_pMvpStatus->sLiveStausCntInfo.uiSLNoneCnt > 0)
            {
                sprintf(szSummaryLogBuff+strlen(szSummaryLogBuff), "      -%s: %d\n", varName(SC_NONE), \
                        m_pMvpStatus->sLiveStausCntInfo.uiSLNoneCnt);
            }
        }
        if(m_pMvpStatus->sLiveStausCntInfo.uiMvpOkButNotOKCnt > 0)
        {
            sprintf(szSummaryLogBuff+strlen(szSummaryLogBuff), "    NOTE: The state of mvp checked is online but not online: %d\n", \
                    m_pMvpStatus->sLiveStausCntInfo.uiMvpOkButNotOKCnt);
        }
    }

    // Ѳ��״̬ͳ��
    if(m_pMvpStatus->sScanStatusInfo.uiScanTimes > 0)
    {
        sprintf(szSummaryLogBuff+strlen(szSummaryLogBuff), "  Scan: %d times, %d lives\n", m_pMvpStatus->sScanStatusInfo.uiScanTimes, \
                m_pMvpStatus->sScanStatusInfo.sLiveStausCntInfo.uiLiveCount);
        UINT32 uiSuccessCount = m_pMvpStatus->sScanStatusInfo.sLiveStausCntInfo.uiLiveOKCnt + \
                                  m_pMvpStatus->sScanStatusInfo.sLiveStausCntInfo.uiLiveWaitingCnt + \
                                  m_pMvpStatus->sScanStatusInfo.sLiveStausCntInfo.uiLiveTimeoutCnt;
        if(uiSuccessCount > 0)
        {
            sprintf(szSummaryLogBuff+strlen(szSummaryLogBuff), "    -Success: %d\n", uiSuccessCount);
            if(m_pMvpStatus->sScanStatusInfo.sLiveStausCntInfo.uiLiveOKCnt > 0)
            {
                sprintf(szSummaryLogBuff+strlen(szSummaryLogBuff), "      -%s: %d\n", varName(LIVE_OK), \
                        m_pMvpStatus->sScanStatusInfo.sLiveStausCntInfo.uiLiveOKCnt);
            }
            if(m_pMvpStatus->sScanStatusInfo.sLiveStausCntInfo.uiLiveTimeoutCnt > 0)
            {
                sprintf(szSummaryLogBuff+strlen(szSummaryLogBuff), "      -%s: %d\n", varName(LIVE_TIMEOUT), \
                        m_pMvpStatus->sScanStatusInfo.sLiveStausCntInfo.uiLiveTimeoutCnt);
            }
            if(m_pMvpStatus->sScanStatusInfo.sLiveStausCntInfo.uiLiveWaitingCnt > 0)
            {
                sprintf(szSummaryLogBuff+strlen(szSummaryLogBuff), "      -%s: %d\n", varName(LIVE_WAITING), \
                        m_pMvpStatus->sScanStatusInfo.sLiveStausCntInfo.uiLiveWaitingCnt);
            }
        }
        UINT32 uiFailedCount = m_pMvpStatus->sScanStatusInfo.sLiveStausCntInfo.uiSLNoneCnt + \
                                 m_pMvpStatus->sScanStatusInfo.sLiveStausCntInfo.uiSLFailCnt + \
                                 m_pMvpStatus->sScanStatusInfo.sLiveStausCntInfo.uiSLWaitingCnt + \
                                 m_pMvpStatus->sScanStatusInfo.sLiveStausCntInfo.uiSLErrorCnt + \
                                 m_pMvpStatus->sScanStatusInfo.sLiveStausCntInfo.uiSLTimeoutCnt;
        if(uiFailedCount > 0)
        {
            sprintf(szSummaryLogBuff+strlen(szSummaryLogBuff), "    -Failed: %d\n", uiFailedCount);
            if(m_pMvpStatus->sScanStatusInfo.sLiveStausCntInfo.uiSLTimeoutCnt > 0)
            {
                sprintf(szSummaryLogBuff+strlen(szSummaryLogBuff), "      -%s: %d\n", varName(SC_TIMEOUT), \
                        m_pMvpStatus->sScanStatusInfo.sLiveStausCntInfo.uiSLTimeoutCnt);
            }
            if(m_pMvpStatus->sScanStatusInfo.sLiveStausCntInfo.uiSLFailCnt > 0)
            {
                sprintf(szSummaryLogBuff+strlen(szSummaryLogBuff), "      -%s: %d\n", varName(SC_FAILED), \
                        m_pMvpStatus->sScanStatusInfo.sLiveStausCntInfo.uiSLFailCnt);
            }
            if(m_pMvpStatus->sScanStatusInfo.sLiveStausCntInfo.uiSLWaitingCnt > 0)
            {
                sprintf(szSummaryLogBuff+strlen(szSummaryLogBuff), "      -%s: %d\n", varName(SC_WAITING), \
                        m_pMvpStatus->sScanStatusInfo.sLiveStausCntInfo.uiSLWaitingCnt);
            }
            if(m_pMvpStatus->sScanStatusInfo.sLiveStausCntInfo.uiSLErrorCnt > 0)
            {
                sprintf(szSummaryLogBuff+strlen(szSummaryLogBuff), "      -%s: %d\n", varName(SC_ERROR), \
                        m_pMvpStatus->sScanStatusInfo.sLiveStausCntInfo.uiSLErrorCnt);
            }
            if(m_pMvpStatus->sScanStatusInfo.sLiveStausCntInfo.uiSLNoneCnt > 0)
            {
                sprintf(szSummaryLogBuff+strlen(szSummaryLogBuff), "      -%s: %d\n", varName(SC_NONE), \
                        m_pMvpStatus->sScanStatusInfo.sLiveStausCntInfo.uiSLNoneCnt);
            }
        }
        if(m_pMvpStatus->sScanStatusInfo.sLiveStausCntInfo.uiMvpOkButNotOKCnt > 0)
        {
            sprintf(szSummaryLogBuff+strlen(szSummaryLogBuff), "    NOTE: The state of mvp checked is online but not online: %d\n", \
                    m_pMvpStatus->sScanStatusInfo.sLiveStausCntInfo.uiMvpOkButNotOKCnt);
        }
    }

    // ��ϸ��Ϣ����MVP��Ϣ
    char szLoginTime[80] = {0};
    char szFirstLoginTime[80] = {0};
    sprintf(szSummaryLogBuff+strlen(szSummaryLogBuff), "\n%s\n", "Detail:");
    for(UINT32 uiMvpSessionIndex=0; uiMvpSessionIndex<m_pMvpStatus->uiCount; uiMvpSessionIndex++)
    {
        pSessionStatusPtr = &m_pMvpStatus->pMvpSessionStatusData[uiMvpSessionIndex];
        sprintf(szSummaryLogBuff+strlen(szSummaryLogBuff), "mvp[%d] %s:%d, %d clients, %d lives, Total Rate: %.2f MByte/s\n", \
                uiMvpSessionIndex+1, pSessionStatusPtr->szIP, pSessionStatusPtr->uiPort, pSessionStatusPtr->uiCount, \
                pSessionStatusPtr->sLiveStausCntInfo.uiLiveCount, pSessionStatusPtr->fStreamRate/1024.0);
        
        // ��client��Ϣ
        for(UINT32 uiClientIndex=0; uiClientIndex<pSessionStatusPtr->uiCount; uiClientIndex++)
        {
            pClientStatusPtr = &pSessionStatusPtr->pClientStatusData[uiClientIndex];
            memset(szLoginTime, 0, sizeof(szLoginTime));
            memset(szFirstLoginTime, 0, sizeof(szFirstLoginTime));
            sprintf(szLoginTime, "%s", CFacility::FormatTime(pClientStatusPtr->uiLoginTime));
            sprintf(szFirstLoginTime, "%s", CFacility::FormatTime(pClientStatusPtr->uiFirstLoginTime));
            // ��¼��Ϣ
            sprintf(szSummaryLogBuff+strlen(szSummaryLogBuff), "  -client[%d] user: %s, login: %s, last login: %s, first login: %s, login times: %d\n", \
                    uiClientIndex+1, pClientStatusPtr->szUser, (pClientStatusPtr->bLogin) ? "yes" : "no", \
                    pClientStatusPtr->uiLoginTime==0 ? "-" : szLoginTime, pClientStatusPtr->uiFirstLoginTime==0 ? "-" : szFirstLoginTime, \
                    pClientStatusPtr->uiLoginCount);
            if(! pClientStatusPtr->bLogin)
            {
                // δ��¼��չʾֱ��������Ϣ����ϸ
                
            }
            else
            {
                // client����ֱ����Ϣ
                sprintf(szSummaryLogBuff+strlen(szSummaryLogBuff), "    %d lives, scan: %s/%d, reStartLive: %s, randomSwitch: %s, Total Rate: %.2f MByte/s\n", \
                        pClientStatusPtr->sLiveStausCntInfo.uiLiveCount, \
                        pClientStatusPtr->bScan ? "yes" : "no", pClientStatusPtr->uiScanTimeSpan, \
                        pClientStatusPtr->bReStartLive ? "yes" : "no", pClientStatusPtr->bRandomSwitch ? "yes" : "no", \
                        pClientStatusPtr->fStreamRate/1024.0);
                // ��ֱ��ҵ����ϸ��Ϣ
                if(m_sFileSCI.bDetail || m_sConsoleSCI.bDetail)
                {
                    // ������
                    sprintf(szDetailLogBuff+strlen(szDetailLogBuff), "    %-5s%-12s%-12s%-14s%-10s%-11s%-8s%-7s%-9s\n", "NO.", "CAMERA", \
                            "START_LIVE", "LIVE", "RECV_CMD", "RECV_STRM", "MVP_OL", "MY_OL", "RATE");
                    for(UINT32 uiLiveIndex=0; uiLiveIndex<pClientStatusPtr->uiCount; uiLiveIndex++)
                    {
                        pLiveStatusPtr = &pClientStatusPtr->pLiveStatusData[uiLiveIndex];
                        // ֱ����š������
                        sprintf(szDetailLogBuff+strlen(szDetailLogBuff), "    %-5d%-12llu", uiLiveIndex+1, pLiveStatusPtr->ubiCamID);
                        // ����ֱ������״̬
                        switch(pLiveStatusPtr->statusStartLive)
                        {
                            case SC_NONE:
                                sprintf(szDetailLogBuff+strlen(szDetailLogBuff), "%-12s", varName(SC_NONE));
                                break;
                            case SC_OK:
                                sprintf(szDetailLogBuff+strlen(szDetailLogBuff), "%-12s", varName(SC_OK));
                                break;
                            case SC_FAILED:
                                sprintf(szDetailLogBuff+strlen(szDetailLogBuff), "%-12s", varName(SC_FAILED));
                                break;
                            case SC_WAITING:
                                sprintf(szDetailLogBuff+strlen(szDetailLogBuff), "%-12s", varName(SC_WAITING));
                                break;
                            case SC_TIMEOUT:
                                sprintf(szDetailLogBuff+strlen(szDetailLogBuff), "%-12s", varName(SC_TIMEOUT));
                                break;
                            case SC_ERROR:
                                sprintf(szDetailLogBuff+strlen(szDetailLogBuff), "%-12s", varName(SC_ERROR));
                                break;
                            default:
                                sprintf(szDetailLogBuff+strlen(szDetailLogBuff), "%-12s", "--");
                                break;
                        }

                        // ֱ��״̬
                        switch(pLiveStatusPtr->statusLive)
                        {
                            case LIVE_NONE:
                                sprintf(szDetailLogBuff+strlen(szDetailLogBuff), "%-14s", varName(LIVE_NONE));
                                break;
                            case LIVE_OK:
                                sprintf(szDetailLogBuff+strlen(szDetailLogBuff), "%-14s", varName(LIVE_OK));
                                break;
                            case LIVE_WAITING:
                                sprintf(szDetailLogBuff+strlen(szDetailLogBuff), "%-14s", varName(LIVE_WAITING));
                                break;
                            case LIVE_TIMEOUT:
                                sprintf(szDetailLogBuff+strlen(szDetailLogBuff), "%-14s", varName(LIVE_TIMEOUT));
                                break;
                            default:
                                sprintf(szDetailLogBuff+strlen(szDetailLogBuff), "%-14s", "--");
                                break;
                        }
                        // �ӷ�������ֱ�����յ���������ʱ��
                        if(pLiveStatusPtr->iRecvCmdTime == -1)
                        {
                            sprintf(szDetailLogBuff+strlen(szDetailLogBuff), "%-10s", "-");
                        }
                        else
                        {
                            sprintf(szDetailLogBuff+strlen(szDetailLogBuff), "%-10d", pLiveStatusPtr->iRecvCmdTime);
                        }
                        // ���յ�ֱ�����ص��յ�����ʱ�� ms
                        if(pLiveStatusPtr->iRecvStreamTime == -1)
                        {
                            sprintf(szDetailLogBuff+strlen(szDetailLogBuff), "%-11s", "-");
                        }
                        else
                        {
                            sprintf(szDetailLogBuff+strlen(szDetailLogBuff), "%-11d", pLiveStatusPtr->iRecvStreamTime);
                        }
                        // mvp�Լ�����״̬
                        if(pLiveStatusPtr->bStrmMvpChecked)
                        {
                            sprintf(szDetailLogBuff+strlen(szDetailLogBuff), "%-8s", pLiveStatusPtr->bStrmMvpCheckedOK?"yes":"no");
                        }
                        else
                        {
                            // δ�Լ�
                            sprintf(szDetailLogBuff+strlen(szDetailLogBuff), "%-8s", "-");
                        }
                        // �������Լ�����״̬
                        sprintf(szDetailLogBuff+strlen(szDetailLogBuff), "%-7s", pLiveStatusPtr->bStrmMyCheckedOK?"yes":"no");
                        // ����
                        sprintf(szDetailLogBuff+strlen(szDetailLogBuff), "%-9.2f", pLiveStatusPtr->fStreamRate);
                        // ��һ��ֱ��ҵ����ϸ����
                        sprintf(szDetailLogBuff+strlen(szDetailLogBuff), "%s", "\n");
                    }
                }
            }
        }
    }

    if(m_sConsoleSCI.bEnabled && bLogToConsoleNow)
    {
        // ����
        system("clear");
        // ��ӡ
        OPEN_STDOUT
        printf("%s", szSummaryLogBuff);
        fflush(stdout);
        CLOSE_STDOUT
        if(m_sConsoleSCI.bDetail)
        {
            OPEN_STDOUT
            printf("%s", szDetailLogBuff);
            fflush(stdout);
            CLOSE_STDOUT
        }
        // �����ϴμ�¼ʱ��
        uiTimeConsoleLast = uiTimeNow;
    }
    
    // д���ļ�
    if(m_sFileSCI.bEnabled && bLogToFileNow)
    {
        FILE *logfile; 
    	char szFile[80] = {0};
        struct timeval tv;
        struct tm stm;
        struct tm stmres;
        gettimeofday(&tv,NULL);
        stm = *(localtime_r(&tv.tv_sec,&stmres));
    	sprintf(szFile, "mvpTest_statistic_%04d%02d%02d.log", stm.tm_year+1900, stm.tm_mon+1, stm.tm_mday); 
    	logfile = fopen(szFile, "a+"); 
    	if (logfile != NULL) 
    	{
    		fprintf(logfile, "%s", szSummaryLogBuff);
    		if(m_sFileSCI.bDetail)
            {
                fprintf(logfile, "%s", szDetailLogBuff);
            }
    		fflush(logfile);
    		fclose(logfile);
    	}
    	// �����ϴμ�¼ʱ��
        uiTimeFileLast = uiTimeNow;
	}
    
    return 0;
}




