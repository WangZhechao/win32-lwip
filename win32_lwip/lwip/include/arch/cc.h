#ifndef __CC_H__
#define __CC_H__

#include "stdio.h"
#include "stdlib.h"


typedef		unsigned char	u8_t;
typedef		signed char		s8_t;
typedef		unsigned short	u16_t;
typedef		signed short 	s16_t;
typedef		unsigned int	u32_t;
typedef		signed int		s32_t;

typedef		u32_t		mem_ptr_t;


#define BYTE_ORDER   LITTLE_ENDIAN  

#define LWIP_PROVIDE_ERRNO

#define PACK_STRUCT_FIELD(x) x//; #pragma STRUCT_ALIGN(x,1)
#define PACK_STRUCT_STRUCT
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_END


#ifndef LWIP_PLATFORM_DIAG
#define LWIP_PLATFORM_DIAG(x)	do {printf x;} while(0)
#endif


#ifndef LWIP_PLATFORM_ASSERT
#define LWIP_PLATFORM_ASSERT(x) do {printf("Assertion \"%s\" failed at line %d in %s\n", x, __LINE__, __FILE__); fflush(NULL); abort();} while(0)
#endif


#define U16_F "u"
#define U32_F "u"
#define S16_F "d"
#define S32_F "d"
#define X16_F "X"
#define X32_F "X"


#endif /* __CC_H__ */