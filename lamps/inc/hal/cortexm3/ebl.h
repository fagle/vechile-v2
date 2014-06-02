/*
 * File: ebl.h
 * Description: em35x common ebl processing code
 *
 * Author(s): David Iacobone, diacobone@ember.com
 *            Lee Taylor, lee@ember.com
 *
 * Copyright 2009 by Ember Corporation. All rights reserved.                *80*
 */

#ifndef __EBL_H__
#define __EBL_H__

#include "bootloader-common.h"  // for BL_Status

/////////////////////
// Definitions related to the EBL file format

#define IMAGE_SIGNATURE 0xE350

#define EBL_MAX_TAG_SIZE         (2048+32)
#define EBL_MIN_TAG_SIZE         (128)

#define EBLTAG_HEADER           0x0000
#define EBLTAG_PROG             0xFE01
#define EBLTAG_MFGPROG          0x02FE
#define EBLTAG_ERASEPROG        0xFD03
#define EBLTAG_END              0xFC04

// Current version of the ebl format.   The minor version (LSB) should be
//  incremented for any changes that are backwards-compatible.  The major
//  version (MSB) should be incremented for any changes that break 
//  backwards compatibility. The major version is verified by the ebl 
//  processing code of the bootloader.
#define EBL_VERSION              (0x0201)
#define EBL_MAJOR_VERSION        (0x0200)
#define EBL_MAJOR_VERSION_MASK   (0xFF00)


// EBL data fields are stored in network (big) endian order
//  however, fields referenced within the aat of the ebl header are stored
//  in native (little) endian order.
typedef struct      pageRange_s
{
  int8u       begPg;          /* First flash page in range    */
  int8u       endPg;          /* Last  flash page in range    */
}                   pageRange_t;

typedef struct eblHdr3xx_s
{
  int16u      tag;            /* = EBLTAG_HEADER              */
  int16u      len;            /* =                            */
  int16u      version;        /* Version of the ebl format    */
  int16u      signature;      /* Magic signature: 0xE350      */
  int32u      flashAddr;      /* Address where the AAT is stored */
  int32u      aatCrc;         /* CRC of the ebl header portion of the AAT */
  // aatBuff is oversized to account for the potential of the AAT to grow in
  //  the future.  Only the first 128 bytes of the AAT can be referenced as
  //  part of the ebl header, although the AAT itself may grow to 256 total
  int8u       aatBuff[128];   /* buffer for the ebl portion of the AAT    */
} eblHdr3xx_t;

typedef struct      eblProg3xx_s
{
  int16u      tag;            /* = EBLTAG_[ERASE|MFG]PROG     */
  int16u      len;            /* = 2..65534                   */
  int32u      flashAddr;      /* Starting addr in flash       */
  int16u      flashData[2/*len/2*/];
}                   eblProg3xx_t;

typedef struct      eblEnd_s
{
  int16u      tag;            /* = EBLTAG_END                 */
  int16u      len;            /* = 4                          */
  int32u      eblCrc;         /* .ebl file CRC -Little-Endian-*/
}                   eblEnd_t;


/////////////////////
// ebl.c APIs


typedef struct {
  BL_Status (*eblGetData)(void *state, int8u *dataBytes, int16u len);
  BL_Status (*eblDataFinalize)(void *state);
} EblDataFuncType;

typedef struct {
  int32u fileCrc;
  boolean headerValid;
  eblHdr3xx_t eblHeader;
} EblProcessStateType;

typedef struct {
  void *dataState;
  int8u *tagBuf;
  int16u tagBufLen;
  boolean returnBetweenBlocks;
  EblProcessStateType eblState;
} EblConfigType;

typedef int8u (*flashReadFunc)(int32u address);

// int8u is used as the return type below to avoid needing to include 
//   ember-types.h for EmberStatus
typedef struct {
  int8u (*erase)(int8u eraseType, int32u address);
  int8u (*write)(int32u address, int16u * data, int32u length);
  flashReadFunc read;
} EblFlashFuncType;


// passed in tagBuf/tagBufLen must be at least EBL_MIN_TAG_SIZE, need not be
//  larger than EBL_MAX_TAG_SIZE
void eblProcessInit(EblConfigType *config,
                    void *dataState,
                    int8u *tagBuf,
                    int16u tagBufLen,
                    boolean returnBetweenBlocks);

BL_Status eblProcess(const EblDataFuncType *dataFuncs, 
                     EblConfigType *config,
                     const EblFlashFuncType *flashFuncs);

#endif //__EBL_H__
