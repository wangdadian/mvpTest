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
	// ��������
	int Start(char* szIpAddr, UINT32 uiPort);
	// ֹͣ����
	int Stop();
	// ���������Ϣ
	int SetCamInfo(CameraInfoT* pCamInfo);
	// 
	// ��ȡ״̬��Ϣ
	// pStatus-���淵�ص�״̬
	// bCareNoStreamRateֱ���ɹ�����������Ϊ0ʱ����ȡ״̬ʱ�Ƿ���ģ�Ϊtrueʱ����������Ϊ0�򷵻ش���
	// �������������pStatus�ڴ�
	int GetStauts(LiveStatusT* pStatus, bool bCareNoStreamRate=true);
	// ֪ͨ�ѷ�������ֱ��
	void NoteStartLive();
	// ֪ͨ���յ�����ֱ���ķ���
	void NoteOnStartLive();
	// ��ȡֱ��������״̬
	STATUS_LIVE GetLiveStatus();

	// ��ȡ����������ֱ����״̬
	STATUS_CMD GetStartLiveStatus();
	STATUS_CMD SetStartLiveStatus(STATUS_CMD status);
	
	UINT64 m_ubiCamID;
	UINT64 m_ubiWndID;
	
private:
	
	// ����ֱ�������ص�
	static void MyLiveStreamCallback(UINT64 ubiCameraId, UINT64 ubiWndId, void * pData, UINT32 uiLen, void * pCaller);
	// 
	static void* ThWorker(void* param);
	///////////////////////////////////////////////////
	
private:
	HANDLE m_hSdkHandle;
	
	pthread_mutex_t m_lockStream;
	pthread_t m_thWorker;
	bool m_bExit; // �߳��˳���־
	
	UINT64 m_ubiLiveStreamTime;//����ȡ��������ʱ�䣬����
	UINT64 m_ubiLiveStreamLen;// ����һ��ͳ���������յ��������ܴ�С
	UINT64 m_ubiLiveStreamLastTime; // ��һ��ͳ��ʱ���յ�ʱ�䣬����
	UINT64 m_ubiLiveStreamLastLen; // ��һ��ͳ��ʱ���յ��������ܴ�С
	float m_fLiveStreamRate; // ÿ1�����һ��ʵʱ��Ƶ�������� ��λKB/S

	// ֱ����������״̬
	STATUS_LIVE m_uiStatusLive;
	// ����ֱ����״̬
	STATUS_CMD m_uiStatusStartLive;

	// ��ʼ����ֱ����ʱ��
	struct timeval m_tvStartLive;
	// �յ�ֱ�����ص�ʱ��
	struct timeval m_tvOnStartLive;
	// ׼����ȡ������ʱ��
	struct timeval m_tvRecvStream;
	// �յ�������ʱ��
	struct timeval m_tvOnRecvStream;

	// �����������Ϣ
	CameraInfoT m_camInfo;
};

#endif


