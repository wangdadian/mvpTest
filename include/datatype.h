#ifndef DATATYPE_H
#define DATATYPE_H

#ifndef WIN32
#include <pthread.h>

typedef  long long	INT64, *PINT64;
typedef unsigned long long	UINT64, *PUINT64;

#ifndef TCHAR
typedef char TCHAR;
#endif	

typedef void *HANDLE;

#ifndef FIELD_OFFSET
#define FIELD_OFFSET(type, field)    ((LONG)(long)&(((type *)0)->field))
#endif 

#else 
typedef __int64 INT64, *PINT64;
typedef unsigned __int64 UINT64, *PUINT64;
#endif 



#ifndef CHAR
typedef char CHAR;
#endif

#define VOID				void
//#define FAR                 far
#define NEAR                near
#define CONST               const
//#define NULL				0
#define MAX_PATH			260

typedef short SHORT;
typedef long LONG;
typedef int INT;

typedef signed char         INT8, *PINT8;
typedef signed short        INT16, *PINT16;
typedef signed int          INT32, *PINT32;

typedef unsigned char       UINT8, *PUINT8;
typedef unsigned short      UINT16, *PUINT16;
typedef unsigned int        UINT32, *PUINT32, UINT;

typedef unsigned long		ULONG;
typedef ULONG *PULONG;
typedef unsigned short USHORT;
typedef USHORT *PUSHORT;
typedef unsigned char UCHAR;
typedef UCHAR *PUCHAR;
typedef char *PSZ;


typedef unsigned long       DWORD;
typedef unsigned char       CBOOL;
typedef unsigned char		BYTE;

typedef CHAR *PCHAR, *LPCH, *PCH;
typedef CONST CHAR *LPCCH, *PCCH;
typedef  CHAR *NPSTR, *LPSTR, *PSTR;
typedef  PSTR *PZPSTR;
typedef  CONST PSTR *PCZPSTR;
typedef  CONST CHAR *LPCSTR, *PCSTR;
typedef  PCSTR *PZPCSTR;

typedef BYTE            *LPBYTE;
typedef float               FLOAT;
typedef double              DOUBLE;
typedef int                 ARRAY_COUNT;
typedef void* LPVOID;

#ifndef MACOS
typedef int BOOL;
#endif

#ifndef WIN32
typedef pthread_mutex_t CRITICAL_SECTION;
#define InitializeCriticalSection(p) pthread_mutex_init((p), NULL)
#define EnterCriticalSection pthread_mutex_lock
#define LeaveCriticalSection pthread_mutex_unlock
#define DeleteCriticalSection pthread_mutex_destroy
typedef void * (* PThread_Start_Routine)(void *);
#else
#include <windows.h>
#endif

	
#endif
