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
	bool bEnabled; // 是否生效
	UINT32 uiTimeInterval; // 记录间隔时间，秒
	bool bDetail; //是否记录明细
	
}StatisticConfigInfoT;

class CMvpSessionBuilder
{
public:
	CMvpSessionBuilder(const char* cfgfile);
	~CMvpSessionBuilder();
	// 启动服务
	int Start();
	// 停止服务
	int Stop();
	// 获取状态信息
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
	TiXmlDocument *m_pXmlDoc; // XML文档对象
	CMvpSession *m_listMvpSession; // 创建的mvp连接会话
	UINT32 m_uiMvpSessionCount;
	list<sMvpSessionInfoT*> m_listMvpSessionInfo;	 // 获取的mvp会话信息
	static UINT64 m_ubiWndIndex; // 窗口ID索引
	char m_szCfgFile[256];
	
	pthread_t m_thWorker;
	bool m_bExit; // 线程退出标志

	MvpStatusT* m_pMvpStatus; // 状态信息

	// 调用Start接口的时间
	char m_szTimeCallStart[80];

	// 统计信息配置
	StatisticConfigInfoT m_sFileSCI;
	StatisticConfigInfoT m_sConsoleSCI;
	
	
};

#endif

