/** @file hal/micro/cortexm3/memmap.h
 * @brief EM300 series memory map definitions
 *
 * <!-- Copyright 2009 by Ember Corporation.             All rights reserved.-->
 */
#ifndef __MEMMAP_H__
#define __MEMMAP_H__

#include "fib-bootloader.h"
#include "ebl.h"
#include "memmap-m3.h"

//=============================================================================
// A union that describes the entries of the vector table.  The union is needed
// since the first entry is the stack pointer and the remainder are function
// pointers.
//
// Normally the vectorTable below would require entries such as:
//   { .topOfStack = x },
//   { .ptrToHandler = y },
// But since ptrToHandler is defined first in the union, this is the default
// type which means we don't need to use the full, explicit entry.  This makes
// the vector table easier to read because it's simply a list of the handler
// functions.  topOfStack, though, is the second definition in the union so
// the full entry must be used in the vectorTable.
//=============================================================================
typedef union
{
  void (*ptrToHandler)(void);
  void *topOfStack;
} HalVectorTableType;


// The start of any EmberZNet image will always contain the following 
// data in flash:
//    Top of stack pointer            [4 bytes]
//    Reset vector                    [4 bytes]
//    NMI handler                     [4 bytes]
//    Hard Fault handler              [4 bytes]
//    Address Table Type              [2 bytes]
//    Address Table Version           [2 bytes]
//    Pointer to full vector table    [4 bytes]
//  Following this will be additional data depending on the address table type

// The address tables take the place of the standard cortex interrupt vector
// tables.  They are designed such that the first entries are the same as the 
// first entries of the cortex vector table.  From there, the address tables
// point to a variety of information, including the location of the full
// cortex vector table, which is remapped to this location in cstartup


// ****************************************************************************
// If any of these address table definitions ever need to change, it is highly
// desirable to only add new entries, and only add them on to the end of an
// existing address table... this will provide the best compatibility with
// any existing code which may utilize the tables, and which may not be able to 
// be updated to understand a new format (example: bootloader which reads the 
// application address table)

// Generic Address table definition which describes leading fields which
// are common to all address table types
typedef struct {
  void *topOfStack;
  void (*resetVector)(void);
  void (*nmiHandler)(void);
  void (*hardFaultHandler)(void);
  int16u type;
  int16u version;
  const HalVectorTableType *vectorTable;
  // Followed by more fields depending on the specific address table type
} HalBaseAddressTableType;

#define IMAGE_INFO_MAXLEN 32

// Description of the Application Address Table (AAT)
// The application address table recieves somewhat special handling, as the
//   first 128 bytes of it are treated as the EBL header with ebl images.
//   as such, any information required by the bootloader must be present in
//   those first 128 bytes.  Other information that is only used by applications 
//   may follow.
//   Also, some of the fields present within those first 128 bytes are filled
//   in by the ebl generation process, either as part of em3xx_convert or
//   when an s37 is loaded by em3xx_load.  An application using these
//   values should take caution in case the image was loaded in some way that
//   did not fill in the auxillary information.
typedef struct {
  HalBaseAddressTableType baseTable;
  // The following fields are used for ebl and bootloader processing.
  //   See the above description for more information.
  int8u platInfo;   // type of platform, defined in micro.h
  int8u microInfo;  // type of micro, defined in micro.h
  int8u phyInfo;    // type of phy, defined in micro.h
  int8u aatSize;    // size of the AAT itself
  int16u softwareVersion;  // EmberZNet SOFTWARE_VERSION
  int16u reserved;  // reserved 16 bits of padding
  int32u timestamp; // Unix epoch time of .ebl file, filled in by ebl gen
  int8u imageInfo[IMAGE_INFO_MAXLEN];  // string, filled in by ebl generation
  int32u imageCrc;  // CRC over following pageRanges, filled in by ebl gen
  pageRange_t pageRanges[6];  // Flash pages used by app, filled in by ebl gen
  
  void *simeeBottom;
  
  // reserve the remainder of the first 128 bytes of the AAT in case we need
  //  to go back and add any values that the bootloader will need to reference,
  //  since only the first 128 bytes of the AAT become part of the EBL header.
  int32u bootloaderReserved[10];
  
  //////////////
  // Any values after this point are still part of the AAT, but will not
  //   be included as part of the ebl header

  void *debugChannelBottom;
  void *noInitBottom;
  void *appRamTop;
  void *globalTop;
  void *cstackTop;  
  void *initcTop;
  void *codeTop;
} HalAppAddressTableType;

extern const HalAppAddressTableType halAppAddressTable;


// Description of the Bootloader Address Table (BAT)
typedef struct {
  HalBaseAddressTableType baseTable;
  int16u bootloaderType;
  int16u bootloaderVersion;  
  const HalAppAddressTableType *appAddressTable;
  
  // plat/micro/phy info added in version 0x0104
  int8u platInfo;   // type of platform, defined in micro.h
  int8u microInfo;  // type of micro, defined in micro.h
  int8u phyInfo;    // type of phy, defined in micro.h
  int8u reserved;   // reserved for future use
  
  // moved to this location after plat/micro/phy info added in version 0x0104
  void (*eblProcessInit)(EblConfigType *config,
                         void *dataState,
                         int8u *tagBuf,
                         int16u tagBufLen,
                         boolean returnBetweenBlocks);  
  BL_Status (*eblProcess)(const EblDataFuncType *dataFuncs, 
                          EblConfigType *config,
                          const EblFlashFuncType *flashFuncs);
  EblDataFuncType *eblDataFuncs; 
 
  // these eeprom routines happen to be app bootloader specific
  // added in version 0x0105
  void (*eepromInit)(void);
  int8u (*eepromRead)( int32u address, int8u *data, int16u len);
  int8u (*eepromWrite)( int32u address, int8u *data, int16u len);

  //pointer to reset info area?

  // Left for reference.  These items were exposed on the 2xx. Do we want
  //  to add them to the 3xx?
  //void *        bootCodeBaseW;                  // $??CODE_LO
  //int16u        bootCodeSizeW;                  // $??CODE_SIZEW
  //void *        bootConstBaseW;                 // $??CONST_LO Relocated
  //int16u        bootConstSize;                  // $??CONST_SIZE
  //int8u *       bootName;                       //=>const int8u bootName[];

} HalBootloaderAddressTableType;

extern const HalBootloaderAddressTableType halBootloaderAddressTable;


// Description of the Fixed Address Table (FAT)
typedef struct {
  HalBaseAddressTableType baseTable;
  void *CustomerInformationBlock;
  HalBootloaderAddressTableType *bootloaderAddressTable;
  void *startOfUnusedRam;
  // ** pointers to shared functions **
  FibStatus (* fibFlashWrite)(int32u address, int8u *data,
                              int32u writeLength, int32u verifyLength);
  FibStatus (* fibFlashErase)(FibEraseType eraseType, int32u address);
} HalFixedAddressTableType;

extern const HalFixedAddressTableType halFixedAddressTable; 

// Description of the Ramexe Address Table (RAT)
typedef struct {
  HalBaseAddressTableType baseTable;
  void *startAddress;
  void *endAddress;
} HalRamexeAddressTableType;

extern const HalRamexeAddressTableType halRamAddressTable; 

#define APP_ADDRESS_TABLE_TYPE          (0x0AA7)
#define BOOTLOADER_ADDRESS_TABLE_TYPE   (0x0BA7)
#define FIXED_ADDRESS_TABLE_TYPE        (0x0FA7)
#define RAMEXE_ADDRESS_TABLE_TYPE       (0x0EA7)

// The current versions of the address tables.
// Note that the major version should be updated only when a non-backwards
// compatible change is introduced (like removing or rearranging fields)
// adding new fields is usually backwards compatible, and their presence can
// be indicated by incrementing only the minor version
#define AAT_VERSION                     (0x0104)
#define AAT_MAJOR_VERSION               (0x0100)
#define AAT_MAJOR_VERSION_MASK          (0xFF00)

#define BAT_NULL_VERSION                (0x0000)
#define BAT_VERSION                     (0x0105)
#define BAT_MAJOR_VERSION               (0x0100)
#define BAT_MAJOR_VERSION_MASK          (0xFF00)

#define FAT_VERSION                     (0x0003)
#define FAT_MAJOR_VERSION               (0x0000)
#define FAT_MAJOR_VERSION_MASK          (0xFF00)

#define RAT_VERSION                     (0x0101)

#endif //__MEMMMAP_H__

