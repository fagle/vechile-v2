#include "mem.h"

/*******************************************************************************
* Function Name  : void sea_memcpy ( const void * dest, void * src, u16 bytes )
* Description    : A version of memcopy that can handle overlapping source and
                   destination regions.
* Input          : input data: val; number of bytes: bytes
* Output         : destination: dest
* Return         : None
*******************************************************************************/
void sea_memcpy ( const void * dest, void * src, u16 bytes )
{
    u8 *d = (u8 *)dest;
    u8 *s = (u8 *)src;

    if (d > s)
    {
        d += bytes - 0x01;
        s += bytes - 0x01;
        while (bytes >= 0x04)
        {
            bytes -= 4;
            *d-- = *s--;
            *d-- = *s--;
            *d-- = *s--;
            *d-- = *s--;
        }
        for (; bytes; bytes --)
            *d-- = *s--;
    } 
    else
    {
        while (bytes >= 0x04) 
        {
            bytes -= 0x04;
	    *d++ = *s++;
	    *d++ = *s++;
	    *d++ = *s++;
	    *d++ = *s++;
        }
        for (; bytes; bytes --) 
            *d++ = *s++;
    }
}

/*******************************************************************************
* Function Name  : void sea_memset ( void * dest, u8 val, u16 bytes )
* Description    : Set memory
* Input          : input data: val; number of bytes: bytes
* Output         : destination: dest
* Return         : None
*******************************************************************************/
void sea_memset ( void * dest, u8 val, u16 bytes )
{
    u8 *d = (u8 *)dest;

    for (; bytes; bytes --)
        *d++ = val;
}

/*******************************************************************************
* Function Name  : s8 sea_memcomp ( const void * str1, const void * str2, u16 bytes )
* Description    : Compare memory
* Input          : data to be compared: source0, source1;
                   number of bytes: bytes
* Output         : 0: equal;
                   else: source are different.
* Return         : None
*******************************************************************************/
s8 sea_memcomp ( const void * str1, const void * str2, u16 bytes )
{
    u8 *s1 = (u8 *)str1;
    u8 *s2 = (u8 *)str2;

    for (; bytes > 0x00; bytes --, s1 ++, s2 ++)
    {
        if (*s1 != *s2)
	    return *s1 - *s2;
    }
    return 0x00;
}

/*******************************************************************************
* Function Name  : s8 sea_memstr (const void * str1, const void * str2, u16 bytes )
* Description    : Compare two strings
* Input          : data to be compared: source0, source1;
                   number of bytes: bytes
* Output         : 0: equal;
                   else: source are different.
* Return         : None
*******************************************************************************/
s8 sea_memstr ( const void * str1, const void * str2, u16 bytes )
{
    u8 *s1 = (u8 *)str1;
    u8 *s2 = (u8 *)str2;
    
    for (; bytes > 0x00; bytes --, s1 ++)
    {
        if (*s1 == *s2)   break;
    }
    
    if (bytes) 
        return sea_memcomp(s1, s2, strlen(str2));
    
    return 0xff;
}

/*******************************************************************************
* Function Name  : s8 sea_memstr (const void * str1, const void * str2, u16 bytes )
* Description    : Compare two strings
* Input          : data to be compared: source0, source1;
                   number of bytes: bytes
* Output         : 0: equal;
                   else: source are different.
* Return         : None
*******************************************************************************/
u16 sea_strlen ( const void * str1 )
{
    u16 i;
    u8 *s1 = (u8 *)str1;
    
    for (i = 0x00; i < 0xffff; i ++, s1 ++)
        if (*s1 == '\0')   
          return i;
    return 0xffff;
}

///////////////////////////////////////////////////////////////////////////////////////
#if 0   // the codes for ads_alloc, iar define heap in stm*.icf file to link 
// memory allocate
//
static u8 mem_allocate[4096];
void MEM_Configuration ( void ) 
{
    u32 start = (u32)mem_allocate; 
    
    _init_alloc(start, start + 4096);
}
#endif

#if !defined(XAP2B) && !defined(UNIX)
// A version of memcopy that can handle overlapping source and
// destination regions.
void halCommonMemCopy(void *dest, const void *source, int8u bytes)
{
  int8u *d = (int8u *)dest;
  int8u *s = (int8u *)source;

  if (d > s) {
    d += bytes - 1;
    s += bytes - 1;
    while(bytes >= 4) {
      bytes -= 4;
      *d-- = *s--;
      *d-- = *s--;
      *d-- = *s--;
      *d-- = *s--;
    }
    for(; bytes; bytes--) {
      *d-- = *s--;
    }
  } else {
    while(bytes >= 4) {
      bytes -= 4;
      *d++ = *s++;
      *d++ = *s++;
      *d++ = *s++;
      *d++ = *s++;
    }
    for(; bytes; bytes--) {
      *d++ = *s++;
    }
  }
}

// Same as above except copies from Program space to RAM.
void halCommonMemPGMCopy(void* dest, void PGM *source, int8u bytes)
{
  int8u *d = (int8u *)dest;
  PGM_PU s = (PGM_PU)source;

  if (d > s) {
    d += bytes - 1;
    s += bytes - 1;
    while(bytes >= 4) {
      bytes -= 4;
      *d-- = *s--;
      *d-- = *s--;
      *d-- = *s--;
      *d-- = *s--;
    }
    for(; bytes; bytes--) {
      *d-- = *s--;
    }
  } else {
    while(bytes >= 4) {
      bytes -= 4;
      *d++ = *s++;
      *d++ = *s++;
      *d++ = *s++;
      *d++ = *s++;
    }
    for(; bytes; bytes--) {
      *d++ = *s++;
    }
  }
}

void halCommonMemSet(void *dest, int8u val, int16u bytes)
{
  int8u *d=(int8u *)dest;

  for(;bytes;bytes--) {
    *d++ = val;
  }
}

int8s halCommonMemCompare(const void *source0, const void *source1, int8u bytes)
{
  int8u *s0 = (int8u *)source0;
  int8u *s1 = (int8u *)source1;

  for(; 0 < bytes; bytes--, s0++, s1++) {
    int8u b0 = *s0;
    int8u b1 = *s1;
    if (b0 != b1)
      return b0 - b1;
  }
  return 0;
}

// Test code for halCommonMemCompare().  There is no good place for unit tests
// for this file.  If you change the function you should probably rerun the
// test.
//  {
//    int8u s0[3] = { 0, 0, 0};
//    int8u s1[3] = { 0, 0, 0};
//    int8u i;
//    assert(halCommonMemCompare(s0, s1, 0) == 0);
//    assert(halCommonMemCompare(s0, s1, 3) == 0);
//    for (i = 0; i < 3; i++) {
//      s0[i] = 1;
//      assert(halCommonMemCompare(s0, s1, 3) > 0);
//      s1[i] = 2;
//      assert(halCommonMemCompare(s0, s1, 3) < 0);
//      s0[i] = 2;
//    }
//  }

// Same again, except that the second source is in program space.

int8s halCommonMemPGMCompare(const void *source0, void PGM *source1, int8u bytes)
{
  int8u *s0 = (int8u *)source0;
  int8u PGM *s1 = (int8u PGM *)source1;

  for(; 0 < bytes; bytes--, s0++, s1++) {
    int8u b0 = *s0;
    int8u b1 = *s1;
    if (b0 != b1)
      return b0 - b1;
  }
  return 0;
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////
