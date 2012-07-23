
/* Mutual LAC definitions */

#ifndef _LACP_BASE_H__
#define _LACP_BASE_H__

#include <stdlib.h>
#include <string.h>
//#define __VXWORKS__
#define DEBUG


#ifdef DEBUG
#  define LAC_DBG 1
#endif
//#include "suma_os_support.h"


#ifdef __VXWORKS__
#  include <stddef.h>
#  include <stdio.h>
#  include <netinet/in.h>
#  include "bitmap.h"
#  include "protocol/LAC_pub.h"
#  include "uid_LAC.h"
#  define Print printf


#else
#  include <stddef.h>
#  include <stdio.h>
#  include <netinet/in.h>
#include <arpa/inet.h>
#  include "bitmap.h"
#  define Print printf
#endif
#include "stdarg.h"

#ifndef INOUT
#  define IN      /* consider as comments near 'input' parameters */
#  define OUT     /* consider as comments near 'output' parameters */
#  define INOUT   /* consider as comments near 'input/output' parameters */
#endif

#ifndef Zero
#  define Zero        0
#  define One         1
#endif

#ifndef Bool
#  define Bool        unsigned int
#  define False       0
#  define True        1
#endif

typedef unsigned short USHORT;
typedef unsigned long ULONG;

#ifndef uint16_t
typedef unsigned int uint16_t;
#endif

#ifndef uint32_t
typedef unsigned int uint32_t;
#endif

typedef unsigned char	Octet;

#ifndef __LINUX__
extern char* strdup (const char *s);

extern USHORT Ntohs (USHORT n);
extern ULONG Htonl (ULONG h);
extern USHORT Htons (USHORT h);
extern ULONG Ntohl (ULONG n);

#define htonl Htonl
#define htons Htons
#define ntohl Ntohl
#define ntohs Ntohs

#endif
#if 0
#ifndef __VXWORKS__
extern char* strdup (const char *s);

extern USHORT Ntohs (USHORT n);
extern ULONG Htonl (ULONG h);
extern USHORT Htons (USHORT h);
extern ULONG Ntohl (ULONG n);

#define htonl Htonl
#define htons Htons
#define ntohl Ntohl
#define ntohs Ntohs

#endif
#endif

#ifdef __VXWORKS__
#define LACP_FATAL(TXT, MSG, EXCOD)                      \
      {lacp_trace ("FATAL:%s failed: %s:%d", TXT, MSG, EXCOD);  \
      exit (EXCOD);}
#else
#define LACP_FATAL(TXT, MSG, EXCOD)                      \
      Print ("FATAL: %s code %s:%d\n", TXT, MSG, EXCOD)
#endif

#define LACP_MALLOC(PTR, TYPE, MSG)              \
  {                                             \
    PTR = (TYPE*) calloc (1, sizeof (TYPE));    \
    if (! PTR) {                                \
      LACP_FATAL("malloc", MSG, -6);             \
    }                                           \
  }

#define LACP_FREE(PTR, MSG)              \
  {                                     \
    if (! PTR) {                        \
      LACP_FATAL("free", MSG, -66);      \
    }                                   \
    free (PTR);                         \
    PTR = NULL;                         \
  }

char* strdup(const char* str);

#define LACP_STRDUP(PTR, SRC, MSG)       \
  {                                     \
    PTR = strdup (SRC);                 \
    if (! PTR) {                        \
      LACP_FATAL("strdup", MSG, -7);     \
    }                                   \
  }

#define LACP_NEW_IN_LIST(WHAT, TYPE, LIST, MSG)  \
  {                                             \
    LACP_MALLOC(WHAT, TYPE, MSG);                \
    WHAT->next = LIST;                          \
    LIST = WHAT;                                \
  }


#include "lacp_type.h"

#endif /*  _LACP_BASE_H__ */

