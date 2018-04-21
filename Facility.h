#ifndef FACILITY_H_20110104
#define FACILITY_H_20110104

#include <time.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <unistd.h>
#include <linux/hdreg.h>
#include <ctype.h>
#include <netinet/in.h>
#include <sys/param.h>
using namespace std;

//////////////////////////////////////////////////////////////////////////
#define MAXINTERFACES 8
class CFacility
{
public:
    
    enum NetInfoType
    {
        NetInfoType_MAC = 0x00,
        NetInfoType_IP = 0x01,
            
    };

    /* 功能：获取系统当前时间 */
    // 返回：格式如“2008-07-31 11:00:00:666”的当前系统字符串
    static char* GetCurTime();

	/* 功能：获取系统当前时间毫秒数 */
	// 返回：时间毫秒数
	static  unsigned long long GetCurTimeMS();


    /* 功能：格式化时间“2008-07-31 11:00:00” */
    // 输入：iTime，通过time(NULL)返回后的时间值
    // 返回：返回格式化后的时间
    static char* FormatTime(int iTime);

    /* 功能：格式化时间“2008-07-31 11:00:00”为整型 */
    // 输入：szTime，类似“2008-07-31 11:00:00”
    // 返回：返回格式化后的时间
    static int FormatTime(const char *szTime);


    /* 功能：创建文件夹 */
    // 输入：szPath：文件夹的绝对路径
    // 返回：成功返回0，失败返回-1
    static int CreateDir(const char* szPath);

    /*判断是否为IP 地址*/
    static bool bIpAddress(const char* szIp);
    
    /*判断是否为组播IP 地址*/
    static bool IsMulticastIP (const char* szIP);

    // 判断是否是MAC地址
    static bool bMAC(const char* szMAC);

    // 获取IP地址,ip-输出参数，保存获取的IP地址，32字节
    static void GetIP(char *ip);
    // 获取MAC地址
    static void GetMAC(unsigned char* mac);
    /*
        设置socket为非阻塞模式
    */
    static int SetNonBlocking(int sock);
    /*
        睡眠：微秒级、毫秒级、秒级
    */
    static void SleepUsec(unsigned int ustime); // usec
    static void SleepMsec(unsigned int mstime); // ms
    static void SleepSec(unsigned int stime);   // sec

    /*
        初始化为守护进程
    */
    static int BeDaemon(const char* szCmd);
private:
    static int GetLocalNetInfo( struct ifreq *buf, enum NetInfoType iType );
};

//////////////////////////////////////////////////////////////////////////

#endif // FACILITY_H_20110104



