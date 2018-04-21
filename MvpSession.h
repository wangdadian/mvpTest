#ifndef MVP_SESSION_H_2018
#define MVP_SESSION_H_2018

#include "defined.h"
#include "Client.h"

class CMvpSession
{
public:
	CMvpSession();
	CMvpSession(sMvpSessionInfoT *pSession);
	~CMvpSession();
	// ��������
	int SetInfo(sMvpSessionInfoT *pSession);
	// ��������
	int Start();
	// ֹͣ����
	int Stop();
	// ��ȡ״̬
	int GetStatus(MvpSessionStatusT* pStatus);

private:
	int BuildClients();
	int StartClients();
	int StopClients();
	
private:
	sMvpSessionInfoT* m_pMvpSessionInfo;
	CClient* m_listClient;
	bool m_bOK;
};

#endif

