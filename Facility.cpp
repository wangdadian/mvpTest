//////////////////////////////////////////////////////////////////////////
#include "Facility.h"

//////////////////////////////////////////////////////////////////////////

// 获取系统当前时间
char* CFacility::GetCurTime()
{
/*    
    time_t timeNow;
    timeNow = time(NULL);
    
    return FormatTime(timeNow);
*/

    static char sTimeNow[64] = {0};
    memset(sTimeNow, 0, 64);
    struct timeval tv;
    struct tm stm;
    gettimeofday(&tv, NULL);
    stm = *( localtime(&tv.tv_sec) );
    sprintf( sTimeNow, "%04d/%02d/%02d-%02d:%02d:%02d:%06ld", stm.tm_year+1900, stm.tm_mon+1, \
                        stm.tm_mday, stm.tm_hour, stm.tm_min, stm.tm_sec, tv.tv_usec);

    return sTimeNow;
}

/* 功能：获取系统当前时间毫秒数 */
	// 返回：时间毫秒数
unsigned long long CFacility::GetCurTimeMS()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return ((unsigned long long)tv.tv_sec * 1000 + (unsigned long long)tv.tv_usec / 1000);
}


// 格式化时间
char* CFacility::FormatTime(int iTime)
{
	time_t t = iTime;
	tm Time;
    //setenv("TZ", "GMT-8", 1);
    //tzset();
    static char szBuf[64]={0};
    memset(szBuf, 0, 64);
	localtime_r((time_t*)&t, &Time);
	sprintf(szBuf, "%02i-%02i-%02i %02i:%02i:%02i", Time.tm_year + 1900, \
                        Time.tm_mon + 1, Time.tm_mday, \
                        Time.tm_hour, Time.tm_min, Time.tm_sec);
	return szBuf;
}

int CFacility::FormatTime(const char* szTime)
{
    
    int itime=0;
    struct tm mytm;
    memset(&mytm, 0, sizeof(tm));
    sscanf(szTime, "%d-%d-%d %d:%d:%d", &mytm.tm_year,&mytm.tm_mon,&mytm.tm_mday, \
        &mytm.tm_hour, &mytm.tm_min, &mytm.tm_sec);

    mytm.tm_year-=1900;    
    mytm.tm_mon-=1;    
    itime=mktime(&mytm);
    return itime;
}

// 创建文件夹
int CFacility::CreateDir(const char* szPath)
{
    if ( *szPath == '\0' )
        return -1;

    char szCmd[128] = {0};
    sprintf(szCmd, "mkdir -p -m a=rwx %s", szPath);
    return system(szCmd);

    /*
    int iRet = mkdir(szPath, 0755); // rwxr-xr-x
    if(iRet != 0)
    {
        perror("CreateDir");
    }
    */
    return 0;
}

// 判断是否为IP地址，是返回true
bool CFacility::bIpAddress(const char* szIp)
{
    if ( NULL == szIp )
    {
        return false;
    }
    struct in_addr addr;
    if ( inet_aton ( szIp, &addr ) == 0 )
    {
        return false;
    }
    
    char* destIp = ( char* ) inet_ntoa ( addr );
    if ( 0 != strcmp ( szIp, destIp ) )
    {
        return false;
    }
    
    return true;
}
/*判断是否为IP 地址*/
bool CFacility::IsMulticastIP (const char* szIP)
{
    unsigned long dwIP = inet_addr(szIP);
    unsigned char chMinClassD_IP[4] = {224,0,0,0};
    unsigned char chMaxClassD_IP[4] = {239,255,255,255};
    return (((unsigned char *) & dwIP) [0] >= chMinClassD_IP [0] &&
            ((unsigned char *) & dwIP) [0] <= chMaxClassD_IP [0]) ;
}

bool CFacility::bMAC(const char* szMAC)
{
    //00:16:17:30:A1:68
    if (szMAC == NULL || strlen(szMAC)<11) {
        return false;
    }
    char szTmpMAC[64]={0};
    strcpy(szTmpMAC, szMAC);
    char szValue[6][2];
    strncpy(szValue[0], strtok(szTmpMAC, ":"), 2);
    char *ptr = NULL;
    int i=1;
    while ( (ptr=strtok(NULL, ":")) ) 
    {
        strncpy(szValue[i], ptr, 2);
        i++;
    }
    if ( i != 6 ) 
    {
        return false;
    }
    

    return true;
}

int CFacility::GetLocalNetInfo( struct ifreq *buf, enum NetInfoType iType )
{
	int fd, intrface, retn = 0;
	//struct ifreq buf[MAXINTERFACES];
	//struct arpreq arp;
	struct ifconf ifc;

    int i = 0;
	for ( i = 0; i < MAXINTERFACES; i++ )
	{
		memset( &buf[i], 0, sizeof( struct ifreq ) );
	}

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0)
	{
		ifc.ifc_len = sizeof(struct ifreq) * MAXINTERFACES;
		ifc.ifc_buf = (caddr_t) buf;
		
		if (!ioctl(fd, SIOCGIFCONF, (char *) &ifc))
		{
			intrface = ifc.ifc_len / sizeof(struct ifreq);
			//printf( "interface num is intrface=%d\n", intrface);
			while (intrface-- > 0)
			{
				retn++;
				//printf( "net device %s. ", buf[intrface].ifr_name);
				/*Jugde whether the net card status is promisc */
				if (!(ioctl(fd, SIOCGIFFLAGS, (char *) &buf[intrface])))
				{
					if (buf[intrface].ifr_flags & IFF_PROMISC)
					{
						//printf( "the interface is PROMISC ");
						
					}
				}
				else
				{
					char str[256];

					sprintf(str, "cpm: ioctl device %s. ", 
						buf[intrface].ifr_name);

					perror(str);
				}

				/*Jugde whether the net card status is up */
				if (buf[intrface].ifr_flags & IFF_UP)
				{
					//printf( "the interface status is UP. ");
				}
				else
				{
					//printf( "the interface status is DOWN. ");
				}

				switch( (int)iType )
				{
					case (int)NetInfoType_MAC:
					{
						/*Get HW ADDRESS of the net card */
						if (!(ioctl(fd, SIOCGIFHWADDR, (char *) &buf[intrface])))
						{
							/*printf( "HW address is: %02x:%02x:%02x:%02x:%02x:%02x. ",
								(unsigned char) buf[intrface].ifr_hwaddr.sa_data[0],
								(unsigned char) buf[intrface].ifr_hwaddr.sa_data[1],
								(unsigned char) buf[intrface].ifr_hwaddr.sa_data[2],
								(unsigned char) buf[intrface].ifr_hwaddr.sa_data[3],
								(unsigned char) buf[intrface].ifr_hwaddr.sa_data[4],
								(unsigned char) buf[intrface].ifr_hwaddr.sa_data[5]);*/
						}
						else
						{
							char str[256];
							sprintf(str, "cpm: ioctl device %s. ", buf[intrface].ifr_name);
							perror(str);
						}
						break;
					}
					case (int)NetInfoType_IP:
					{
						/*Get IP of the net card */
						if (!(ioctl(fd, SIOCGIFADDR, (char *) &buf[intrface])))
						{
							/*
							printf( "IP address is: %s.", inet_ntoa(((struct sockaddr_in *)
							(&buf[intrface].ifr_addr))-> sin_addr));
							*/
							//puts (buf[intrface].ifr_addr.sa_data);
						}
						else
						{
							char str[256];

							sprintf(str, "cpm: ioctl device %s. ",
								buf[intrface].ifr_name);

							perror(str);
						}
						break;
					}
					default:
						break;
				}

				
				//printf( "\n" );
			}
		}
		else
			perror( "cpm: ioctl ");
	}
	else
		perror( "cpm: socket ");

	close(fd);

	return retn;
}

void CFacility::GetIP(char *ip)	//读取系统实际IP
{
    struct ifreq *buf;
    buf = (struct ifreq *)malloc(sizeof(struct ifreq)*MAXINTERFACES);
    int count = 0;
    count = GetLocalNetInfo( buf, NetInfoType_IP );
    int i = 0;
    for ( i = 0; i < count; i++ )
    {
        if ( strcmp( buf[i].ifr_name, "lo" ) == 0 )
        {
            continue;
        }
    
        if (! buf[i].ifr_flags & IFF_UP )
        {
            continue;
        }
    
        strcpy( ip, inet_ntoa(((struct sockaddr_in *)(&buf[i].ifr_addr))-> sin_addr) );
     }
    if ( buf != NULL )	
        free(buf);
}

void CFacility::GetMAC(unsigned char* mac)//获取MAC
{
    struct ifreq *buf;
    buf = (struct ifreq *)malloc(sizeof(struct ifreq)*MAXINTERFACES);
    int count = 0;
    count = GetLocalNetInfo( buf, NetInfoType_MAC );
    int i = 0;
    for ( i = 0; i < count; i++ )
    {
        if ( strcmp( buf[i].ifr_name, "lo" ) == 0 )
        {
            continue;
        }
    
        if (! buf[i].ifr_flags & IFF_UP )
        {
            continue;
        }
        mac[0] = (unsigned char) buf[i].ifr_hwaddr.sa_data[0];
        mac[1] = (unsigned char) buf[i].ifr_hwaddr.sa_data[1];
        mac[2] = (unsigned char) buf[i].ifr_hwaddr.sa_data[2];
        mac[3] = (unsigned char) buf[i].ifr_hwaddr.sa_data[3];
        mac[4] = (unsigned char) buf[i].ifr_hwaddr.sa_data[4];
        mac[5] = (unsigned char) buf[i].ifr_hwaddr.sa_data[5];
    }
    if ( buf != NULL )	
        free(buf);

}

int CFacility::SetNonBlocking(int sock)
{
    int opts;
    opts=fcntl(sock, F_GETFL);
    if( opts < 0 )
    {
        perror("fcntl(sock,GETFL)");
        return -1;
    }
    opts = opts|O_NONBLOCK;
    if( fcntl(sock, F_SETFL, opts) < 0 )
    {
        perror("fcntl(sock,SETFL,opts)");
        return -1;
    }
    return 0;
}

void CFacility::SleepUsec(unsigned int ustime) // usec
{
	   /*struct timeval tv;
	   tv.tv_sec = ustime/1000000;
	   tv.tv_usec = ustime%1000000;
	   select(0, NULL, NULL, NULL, &tv);*/
	   usleep(ustime);
}

void CFacility::SleepMsec(unsigned int mstime) // ms
{
	   /*struct timeval tv;
	   tv.tv_sec = mstime/1000;
	   tv.tv_usec = (mstime % 1000) * 1000;
	   select(0, NULL ,NULL, NULL, &tv);*/
	   usleep(1000 * mstime);
}

void CFacility::SleepSec(unsigned int stime)  // sec
{
	   /*struct timeval tv;
	   tv.tv_sec = stime;
	   tv.tv_usec = 0;
       select(0, NULL, NULL, NULL, &tv);
       */
       sleep(stime);
}

//  初始化为守护进程
int CFacility::BeDaemon(const char* szCmd)
{
    pid_t pid;
    struct sigaction  sa;

    /* clear file creation mask */
    umask(0);

    /* become a session leader to lose controlling TTY */
    if ( (pid = fork()) < 0 )
    {
        fprintf(stderr, "#Error: %s can't fork!\n", szCmd);
        return -1;
    }
    else if ( pid != 0 ) /* parent process */
    {
        exit(0);
    }
    setsid();

    /* ensure future opens won't allocate controlling TTYs */
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if ( sigaction(SIGHUP, &sa, NULL) < 0 )
    {
        fprintf(stderr, "#Error: %s can't ignore SIGHUP!\n", szCmd);
        return -1;
    }
    if ( (pid = fork()) < 0 )
    {
        fprintf(stderr, "#Error: %s can't fork!\n", szCmd);
        return -1;
    }
    else if ( pid != 0 ) /* parent process */
    {
        exit(0);
    }

    /* change the current working directory to the root so
     * we won't prevent file systems from being unmounted.
     */
    if ( chdir("/") < 0 )
    {
        fprintf(stderr, "#Error: %s can't change working directory to \"/\"!\n", szCmd);
        return -1;
    }

    return 0;
}

