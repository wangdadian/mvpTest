#ifndef MVP_SESSION_BUILDER_H_2018
#define MVP_SESSION_BUILDER_H_2018

#include "defined.h"
#include "tinyxml.h"
#include "MvpSession.h"
#include "Facility.h"
#include <list>
using namespace std;

typedef struct __StatisticConfigInfo
{
	bool bEnabled; // �Ƿ���Ч
	UINT32 uiTimeInterval; // ��¼���ʱ�䣬��
	bool bDetail; //�Ƿ��¼��ϸ
	
}StatisticConfigInfoT;

class CMvpSessionBuilder
{
public:
	CMvpSessionBuilder(const char* cfgfile);
	~CMvpSessionBuilder();
	// ��������
	int Start();
	// ֹͣ����
	int Stop();
	// ��ȡ״̬��Ϣ
	int GetStatus(MvpStatusT* pStatus);
	bool m_bOK;

private:
	int LoadConfigInfo();
	int BuildMvpSession();
	int StartMvpSession();
	int StopMvpSession();
	void DeleteMvpSessionInfo();
	void DeleteMvpStatusInfo();

	int LogStatisticMvpStatus();
	// 
	static void* ThWorker(void* param); 
	
private:
	TiXmlDocument *m_pXmlDoc; // XML�ĵ�����
	CMvpSession *m_listMvpSession; // ������mvp���ӻỰ
	UINT32 m_uiMvpSessionCount;
	list<sMvpSessionInfoT*> m_listMvpSessionInfo;	 // ��ȡ��mvp�Ự��Ϣ
	static UINT64 m_ubiWndIndex; // ����ID����
	char m_szCfgFile[256];
	
	pthread_t m_thWorker;
	bool m_bExit; // �߳��˳���־

	MvpStatusT* m_pMvpStatus; // ״̬��Ϣ

	// ����Start�ӿڵ�ʱ��
	char m_szTimeCallStart[80];

	// ͳ����Ϣ����
	StatisticConfigInfoT m_sFileSCI;
	StatisticConfigInfoT m_sConsoleSCI;
	
	
};

#endif

