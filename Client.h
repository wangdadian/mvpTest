#ifndef CLIENT_H_2018
#define CLIENT_H_2018

#include "defined.h"
#include "vasapi.h"
#include "ClientLive.h"
#include <list>
using namespace std;


// ֱ����Ϣ
typedef struct __ClientLiveInfo
{
	CClientLive* pClientLive; // ֱ������
	//STATUS_CMD status; // ֱ��״̬
	UINT32 uiStartLiveTIme; // ��ʼ����ֱ����ʱ��
}ClientLiveInfoT;


class CClient
{
public:
	CClient();
	CClient(const char* szMvpIP, UINT32 uiMvpPort, sClientInfoT* pClientInfo);
	~CClient();
	// ������Ϣ
	int SetInfo(const char* szMvpIP, UINT32 uiMvpPort, sClientInfoT* pClientInfo);
	// ��������
	int Start();
	// ֹͣ����
	int Stop();
	// ��ȡ״̬��Ϣ
	int GetStatus(ClientStatusT* pStatus);
	
private:
	// ��ʼ����Դ
	int Init();
	// �����¼
	int Login();
	// ����ע��
	int Logout();
	// �����ȡ�����Ϣ
	int GetCamera();
	// �����ȡ���״̬��Ϣ
	int GetCameraStatus();
	// ����ֱ��
	int StartLive();
	// ����ֱֹͣ��
	int StopLive();
	
	// ���շ�����Ϣ�ص�
	static void MyReturnCallback( void * handle, char * szXmlRet, UINT32 uiMessageType, void * body, void * pCaller );
	static void MyLargeDataCallback( void * handle, UINT32 uiDataType, void * pData, UINT32 uiLen, void * pCaller );
	static void MyDisconnectCallback( void * handle, void * pCaller );
	//static void MyStreamCallback(void * handle, UINT64 ubiCameraId, UINT32 uiDataType, UINT64 ubiSessionId, void * pData, UINT32 uiCount, UINT64 ubiTimeStamp, void * pCaller);

	// �յ����������Ĵ���
	void OnGetCamera(UINT32 uiCamCount, void *pBody);
	void OnGetCameraStatus(UINT32 uiLen, void *pBody);
	void OnStartLive(void * pBody);
	void OnStopLive(void * pBody);
	void OnLogin(void * pBody);
	void OnLogout(void * pBody);

	// ����������첽�ȴ�����������أ������к���ҵ����������ֱ��ҵ����˳��߳�
	static void* ThWorker(void* param); 
	// ֱ�������̣߳�����������Ѳ���²�����
	static void* ThReStartLiveWorker(void* param);
	// Ѳ��
	static void* ThScanWorker(void* param);
	// ����mvp
	static void* ThReconnectWorker(void* param); 
	// ��������ֱ��״̬�߳�
	static void* ThRecvCmdTimeoutWorker(void* param); 
	
	///////////////////////////////////////////////////
	// ��ȡ��Ч�������Ϣ
	// ����: ����ģʽδ���û��Խ����룬δ�Լ���Լ������
	int GetUsedCamList();
	// ��ȡ��һ�����õ������Ϣ������
	int GetNextUsedCamInfoIndex();
	// ���������߳�
	int StartReconnectThread();
	// ����Ѳ���߳�
	int StartScanThread();
	// ����ֱ�������߳�
	int StartLiveThread();
	// ��¼�����������߳�
	int StartThread();
	// ��ʼ��ֱ��ҵ����Ϣ�����ڹ������SetInfoʱ����һ�Σ��Һ���Init����
	int InitClientLiveInfo();
	// ͳ��Ѳ����Ϣ
	int CountScanStatusInfo();

private:
	pthread_mutex_t m_lockClientLive;
	pthread_t m_thWorker;
	bool m_bExit; // �߳��˳���־

	pthread_t m_thReStartLiveWorker;
	bool m_bReStartLiveThreadExit;
	
	pthread_t m_thScanWorker;
	bool m_bScanThreadExit;

	pthread_t m_thRecvCmdTimeoutWorker;
	bool m_bRecvCmdTimeoutThreadExit;

	// ����MVP�߳�
	pthread_t m_thReconnectWorker;
	bool m_bReconnectThreadExit;

	// ����Ƿ�����ֱ�����������
	bool m_bStartLiveEnd;
	
	char m_szMvpIP[80]; // ��¼��MVP��ַ
	UINT32 m_uiMvpPort; // ��½��MVP�˿�
	sClientInfoT* m_pClientInfo; // �ͻ�����Ϣ
	HANDLE	m_hSdkHandle; // SDK��ʼ����ľ��

	bool m_bLogined; // �Ƿ��¼�ɹ�
	STATE_TYPE m_iStateGetCam; // ��ȡ����б��״̬
	STATE_TYPE m_iStateGetCamStatus; // ��ȡ���״̬�Ƿ�ɹ�
	
	CameraInfoT* m_pCamerainfo; // ��¼���ȡ�������Ϣ
	UINT32  m_uiCameraCount; // ��¼���ȡ���������
	CameraStatusInfoT* m_pCameraStatusInfo; //��¼���ȡ�����״̬��Ϣ
	UINT32 m_uiCameraStatusCount; // ��¼���ȡ�����״̬��Ϣ����

	CameraInfoT* m_pUsedCamInfo; // �����˺�Ŀ��������Ϣ����������ֱ����
	UINT32  m_uiUsedCamCount; // �����˺�Ŀ����������
	UINT32 m_uiIndexUsedCamInfo; 
	//CameraInfoT
	ClientLiveInfoT* m_listClientLiveInfo;
	UINT32 m_uiClientLiveInfoCount;

	// Ѳ��ͳ����Ϣ
	ScanStatusInfoT m_sScanStatusInfo;
};

#endif

