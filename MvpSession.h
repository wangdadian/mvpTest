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
	// 设置属性
	int SetInfo(sMvpSessionInfoT *pSession);
	// 启动服务
	int Start();
	// 停止服务
	int Stop();
	// 获取状态
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

