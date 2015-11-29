////////////////////////////////////////////////////////////////////////////////////////////
//
//    
//    File: 6x8.c
//
//    Copyright (C): 2003-2006 Searen Network Software Ltd.
//
//    [ This source code is the sole property of Searen Network Software Ltd.  ]
//    [ All rights reserved.  No part of this source code may be reproduced in ]
//    [ any form or by any electronic or mechanical means, including informa-  ]
//    [ tion storage and retrieval system, without the prior written permission]
//    [ of Searen Network Software Ltd.                                        ]
//    [                                                                        ]
//    [   For use by authorized Searen Network Software Ltd. employees only.   ]
//
//    Description:   This class can read, write and watch one serial port.
//					 It sends messages to its owner when something happends on the port
//					 The class creates a thread for reading and writing so the main
//					 program is not blocked.
// 
//
//
//  AUTHOR: Ren Yu.
//  DATE: Apr. 14, 2006
//
//
/////////////////////////////////////////////////////////////////////////////////////////////
//
#ifndef __asc6x8_
#define __asc6x8_ 	   1
const unsigned char zh_char[][24]={
//12X12���ֵ������룬����ʽ����λ��ǰ
{0x34,0x2F,0xF4,0xA4,0xE8,0xA4,0xEA,0x09,0xEA,0x04,0xE8,0x00,0x01,0x01,0x07,0x00,0x07,0x02,0x07,0x00,0x01,0x04,0x07,0x00},/*"��",0*/
{0x00,0x00,0x00,0xC1,0x31,0x0E,0x30,0xC0,0x00,0x00,0x00,0x00,0x04,0x02,0x01,0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x04,0x00},/*"��",1*/
{0x38,0x00,0xFF,0x08,0x02,0x02,0x02,0xFE,0x02,0x03,0x02,0x00,0x04,0x03,0x00,0x01,0x02,0x04,0x04,0x07,0x00,0x00,0x00,0x00},/*"��",2*/
{0x20,0x20,0x2F,0xE9,0xA9,0xA9,0xA9,0xA9,0xAF,0x20,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x04,0x04,0x03,0x00,0x00,0x00},/*"��",3*/
{0x00,0x00,0xFF,0x11,0x11,0x11,0x11,0x11,0x11,0xFF,0x00,0x00,0x00,0x00,0x07,0x02,0x02,0x02,0x02,0x02,0x02,0x07,0x00,0x00},/*"��",4*/
{0x82,0xFF,0xAA,0xAA,0xFF,0x82,0x00,0xFF,0x49,0xFF,0x00,0x00,0x04,0x02,0x01,0x00,0x01,0x02,0x04,0x03,0x00,0x07,0x00,0x00},/*"��",5*/
{0x88,0x68,0xFF,0x48,0x04,0xF4,0x15,0x16,0xF4,0x04,0x04,0x00,0x00,0x00,0x07,0x04,0x02,0x01,0x00,0x00,0x07,0x04,0x07,0x00},/*"��",6*/
{0x00,0xFC,0x94,0x94,0x94,0xFF,0x94,0x94,0x94,0xFE,0x04,0x00,0x00,0x01,0x00,0x00,0x00,0x03,0x04,0x04,0x04,0x04,0x06,0x00},/*"��",7*/
{0x00,0x02,0x22,0x22,0x22,0x22,0x22,0x22,0x23,0x02,0x00,0x00,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x06,0x04,0x00},/*"��",8*/
{0xA8,0x6A,0x2F,0xBA,0xEE,0x8B,0x10,0x7F,0x84,0x7C,0x04,0x00,0x00,0x05,0x05,0x07,0x00,0x04,0x04,0x02,0x01,0x02,0x04,0x00},/*"��",9*/
{0xDF,0x09,0xF9,0x4F,0x88,0x44,0xEB,0x52,0x6A,0xC6,0x40,0x00,0x07,0x02,0x03,0x02,0x02,0x00,0x07,0x04,0x04,0x07,0x00,0x00},/*"·",10*/
{0x38,0x00,0xFF,0x08,0x02,0x02,0x02,0xFE,0x02,0x03,0x02,0x00,0x04,0x03,0x00,0x01,0x02,0x04,0x04,0x07,0x00,0x00,0x00,0x00},/*"��",11*/
{0x00,0x00,0x3C,0xA5,0x26,0xA4,0x26,0x25,0xA4,0x3C,0x00,0x00,0x04,0x03,0x00,0x07,0x04,0x04,0x05,0x04,0x06,0x01,0x02,0x00},/*"��",12*/
{0xA5,0x96,0xCC,0xBF,0x96,0xA5,0x18,0x67,0x84,0x7C,0x04,0x00,0x04,0x05,0x02,0x02,0x03,0x04,0x04,0x02,0x01,0x02,0x04,0x00},/*"��",13*/
{0x11,0xF2,0x00,0x04,0xF5,0x56,0x5C,0x54,0x56,0xF5,0x04,0x00,0x06,0x01,0x02,0x04,0x05,0x05,0x05,0x05,0x05,0x05,0x04,0x00},/*"��",14*/
{0xDF,0x09,0xF9,0x4F,0x88,0x44,0xEB,0x52,0x6A,0xC6,0x40,0x00,0x07,0x02,0x03,0x02,0x02,0x00,0x07,0x04,0x04,0x07,0x00,0x00},/*"·",15*/
{0x20,0x20,0x2F,0xE9,0xA9,0xA9,0xA9,0xA9,0xAF,0x20,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x04,0x04,0x03,0x00,0x00,0x00},/*"��",16*/
{0x74,0x85,0x76,0x04,0xD6,0x54,0xF4,0x57,0xD4,0x54,0xD6,0x00,0x03,0x01,0x01,0x01,0x07,0x00,0x03,0x00,0x03,0x04,0x07,0x00},/*"��",17*/
{0x00,0xFE,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0xFE,0x00,0x00,0x00,0x03,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x03,0x00,0x00},/*"��",18*/
{0x20,0x20,0x2F,0xE9,0xA9,0xA9,0xA9,0xA9,0xAF,0x20,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x04,0x04,0x03,0x00,0x00,0x00},/*"��",19*/
{0x22,0x3A,0x27,0xFA,0xA2,0x10,0x8C,0x77,0x84,0x14,0x0C,0x00,0x01,0x01,0x01,0x07,0x04,0x02,0x01,0x00,0x01,0x02,0x04,0x00},/*"��",20*/
{0x10,0xF8,0x07,0x50,0x4E,0x48,0x48,0xFF,0x48,0x4C,0x48,0x00,0x00,0x07,0x00,0x00,0x00,0x00,0x00,0x07,0x00,0x00,0x00,0x00},/*"��",21*/
{0x00,0xFE,0x48,0xCF,0x08,0xFE,0x2A,0x4A,0x89,0x79,0x09,0x00,0x06,0x01,0x00,0x07,0x02,0x05,0x02,0x01,0x01,0x02,0x04,0x00},/*"��",22*/
{0x04,0x84,0x44,0x24,0x14,0xFF,0x14,0x24,0x44,0x86,0x04,0x00,0x01,0x00,0x01,0x01,0x01,0x07,0x01,0x01,0x01,0x00,0x01,0x00},/*"��",23*/
{0x1C,0x0A,0xEB,0xAE,0xAA,0xAC,0xAA,0xEB,0x8E,0x0A,0x1A,0x00,0x00,0x00,0x07,0x04,0x04,0x04,0x04,0x04,0x07,0x00,0x00,0x00},/*"��",24*/
{0x12,0x12,0xFE,0x12,0x3F,0xA5,0xA5,0xFF,0xA5,0xA5,0x3F,0x00,0x02,0x02,0x01,0x05,0x04,0x04,0x04,0x07,0x04,0x04,0x04,0x00},/*"��",25*/
{0x00,0x00,0xF7,0x15,0x15,0xD5,0x15,0x15,0x17,0xF0,0x00,0x00,0x04,0x04,0x05,0x04,0x02,0x01,0x01,0x02,0x04,0x05,0x00,0x00},/*"Ա",26*/
{0x08,0xFF,0x88,0x74,0x54,0x74,0x04,0x7F,0x84,0x77,0x04,0x00,0x01,0x01,0x00,0x02,0x02,0x01,0x05,0x02,0x01,0x02,0x07,0x00},/*"��",27*/
{0x00,0x10,0x88,0x8C,0x53,0x62,0x52,0x4A,0x46,0xC2,0x00,0x00,0x01,0x01,0x00,0x07,0x04,0x04,0x04,0x04,0x04,0x07,0x00,0x00},/*"��",28*/
{0xFE,0x22,0x22,0xFE,0x04,0x14,0x64,0x04,0xFF,0x04,0x04,0x00,0x03,0x01,0x01,0x03,0x00,0x00,0x04,0x04,0x07,0x00,0x00,0x00},/*"ʱ",29*/
{0xFC,0x01,0x02,0xF8,0x29,0x29,0x29,0xF9,0x01,0xFF,0x00,0x00,0x07,0x00,0x00,0x01,0x01,0x01,0x01,0x01,0x04,0x07,0x00,0x00},/*"��",30*/
{0x44,0x44,0xFF,0x24,0x24,0x02,0x02,0x02,0xFE,0x02,0x02,0x00,0x04,0x04,0x07,0x00,0x00,0x00,0x04,0x04,0x07,0x00,0x00,0x00},/*"��",31*/
{0x20,0x22,0x22,0xFE,0x22,0x22,0x22,0xFE,0x22,0x23,0x22,0x00,0x04,0x04,0x02,0x01,0x00,0x00,0x00,0x07,0x00,0x00,0x00,0x00},/*"��",32*/
{0x10,0x10,0x88,0x94,0x92,0xF1,0x92,0x94,0x88,0x10,0x10,0x00,0x04,0x04,0x04,0x04,0x04,0x07,0x04,0x04,0x04,0x04,0x04,0x00},/*"ȫ",33*/
{0x12,0xD6,0x5A,0x53,0x5A,0xD6,0x12,0xFE,0x02,0x3A,0xC6,0x00,0x00,0x07,0x02,0x02,0x02,0x07,0x00,0x07,0x01,0x02,0x01,0x00},/*"��",34*/
{0xDF,0x09,0xF9,0x4F,0x88,0x44,0xEB,0x52,0x6A,0xC6,0x40,0x00,0x07,0x02,0x03,0x02,0x02,0x00,0x07,0x04,0x04,0x07,0x00,0x00},/*"·",35*/
{0x38,0x00,0xFF,0x08,0x02,0x02,0x02,0xFE,0x02,0x03,0x02,0x00,0x04,0x03,0x00,0x01,0x02,0x04,0x04,0x07,0x00,0x00,0x00,0x00},/*"��",36*/
{0x40,0x44,0x44,0x45,0xC6,0x7C,0xC4,0x46,0x45,0x44,0x40,0x00,0x04,0x04,0x02,0x01,0x00,0x00,0x00,0x01,0x02,0x04,0x04,0x00},/*"��",37*/
{0xFC,0x11,0x92,0x50,0x31,0xFD,0x11,0x11,0x01,0xFF,0x00,0x00,0x07,0x01,0x00,0x02,0x02,0x03,0x00,0x04,0x04,0x07,0x00,0x00},/*"��",38*/
{0x62,0x22,0x3A,0xAA,0xAA,0xAB,0xAA,0xAA,0x3A,0x22,0x62,0x00,0x04,0x04,0x02,0x01,0x00,0x00,0x00,0x07,0x04,0x04,0x07,0x00},/*"��",39*/
{0x00,0xFE,0x0A,0x8A,0xBE,0xAA,0xAB,0xAA,0xBE,0x0A,0x0A,0x00,0x06,0x01,0x04,0x04,0x04,0x03,0x02,0x03,0x04,0x04,0x04,0x00},/*"��",40*/
{0x00,0x00,0x7C,0x55,0x56,0xFC,0x56,0x55,0x54,0x7C,0x00,0x00,0x01,0x01,0x01,0x01,0x01,0x07,0x01,0x01,0x01,0x01,0x01,0x00},/*"��",41*/
{0x38,0x00,0xFF,0x08,0x02,0x02,0x02,0xFE,0x02,0x03,0x02,0x00,0x04,0x03,0x00,0x01,0x02,0x04,0x04,0x07,0x00,0x00,0x00,0x00},/*"��",42*/
{0x10,0xF1,0x02,0x80,0xFF,0x11,0xD5,0x5F,0xD5,0x11,0xFF,0x00,0x00,0x03,0x01,0x04,0x03,0x00,0x01,0x01,0x05,0x04,0x07,0x00},/*"��",43*/
{0x02,0x12,0x12,0x1F,0xF2,0x12,0x12,0x1F,0x12,0xF2,0x02,0x00,0x00,0x00,0x00,0x00,0x07,0x00,0x01,0x01,0x01,0x00,0x00,0x00},/*"��",44*/
{0x10,0x11,0xF2,0x20,0x10,0x6F,0xA1,0x21,0xAF,0x68,0x08,0x00,0x00,0x00,0x03,0x05,0x04,0x04,0x02,0x01,0x02,0x04,0x04,0x00},/*"��",45*/
{0x10,0x17,0xF5,0x55,0x57,0x5D,0x57,0x55,0xF5,0x17,0x10,0x00,0x04,0x04,0x07,0x05,0x05,0x05,0x05,0x05,0x07,0x04,0x04,0x00},/*"��",46*/
{0x04,0x05,0xF5,0x55,0x55,0xFF,0x55,0x55,0xF5,0x05,0x04,0x00,0x04,0x05,0x05,0x05,0x05,0x07,0x05,0x05,0x05,0x05,0x04,0x00},/*"��",47*/
{0x00,0x00,0xFE,0xD2,0x52,0x53,0x52,0x52,0x52,0xDE,0x00,0x00,0x04,0x03,0x00,0x07,0x04,0x04,0x04,0x04,0x04,0x07,0x00,0x00},/*"��",48*/
{0x10,0xF8,0x07,0x44,0x44,0x7F,0x44,0x44,0x7F,0x44,0x44,0x00,0x00,0x07,0x00,0x04,0x02,0x01,0x00,0x00,0x01,0x02,0x04,0x00},/*"��",49*/
{0x00,0xFC,0x94,0x94,0x94,0xFF,0x94,0x94,0x94,0xFE,0x04,0x00,0x00,0x01,0x00,0x00,0x00,0x03,0x04,0x04,0x04,0x04,0x06,0x00},/*"��",50*/
{0xFF,0x92,0x54,0xFF,0x54,0x92,0x00,0xFE,0x12,0xF1,0x11,0x00,0x03,0x02,0x02,0x03,0x02,0x04,0x03,0x00,0x00,0x07,0x00,0x00},/*"��",51*/
{0x00,0xFC,0x94,0x94,0x94,0xFF,0x94,0x94,0x94,0xFE,0x04,0x00,0x00,0x01,0x00,0x00,0x00,0x03,0x04,0x04,0x04,0x04,0x06,0x00},/*"��",52*/
{0x00,0x02,0x4A,0xCA,0x6E,0xDA,0x49,0xC9,0x65,0x41,0x00,0x00,0x04,0x02,0x01,0x04,0x04,0x07,0x00,0x00,0x01,0x02,0x04,0x00},/*"ϵ",53*/
{0x48,0x6C,0x5B,0x4C,0x00,0x32,0xEA,0x27,0xE2,0x32,0x22,0x00,0x02,0x02,0x01,0x05,0x04,0x02,0x01,0x00,0x07,0x04,0x07,0x00},/*"ͳ",54*/
{0x20,0xF8,0x07,0x02,0xAA,0xAA,0xAA,0xAB,0xAA,0xAA,0x02,0x00,0x00,0x07,0x00,0x00,0x07,0x02,0x02,0x02,0x02,0x07,0x00,0x00},/*"��",55*/
{0x00,0x00,0xFE,0xAA,0xAB,0xAA,0xAA,0xAA,0xFE,0x00,0x00,0x00,0x04,0x03,0x00,0x07,0x04,0x05,0x06,0x04,0x06,0x01,0x02,0x00},/*"Ϣ",56*/
{0x88,0xF1,0x1A,0xC0,0x1F,0xF5,0x15,0xF5,0x15,0x9F,0x40,0x00,0x00,0x07,0x04,0x04,0x05,0x07,0x04,0x07,0x05,0x04,0x04,0x00},/*"ʪ",57*/
{0x00,0xFE,0x0A,0x8A,0xBE,0xAA,0xAB,0xAA,0xBE,0x0A,0x0A,0x00,0x06,0x01,0x04,0x04,0x04,0x03,0x02,0x03,0x04,0x04,0x04,0x00}, /*"��",58*/
{0x89,0x72,0x00,0xC0,0x5F,0xD5,0x55,0xD5,0x55,0xDF,0x00,0x00,0x07,0x00,0x04,0x07,0x04,0x07,0x04,0x07,0x04,0x07,0x04,0x00},/*"��",59*/
{0x00,0xFE,0x0A,0x8A,0xBE,0xAA,0xAB,0xAA,0xBE,0x0A,0x0A,0x00,0x06,0x01,0x04,0x04,0x04,0x03,0x02,0x03,0x04,0x04,0x04,0x00},/*"��",60*/
{0x00,0xF0,0xF8,0x0C,0x04,0x04,0x04,0x04,0x0C,0xF8,0xF0,0x00,0x00,0x00,0x01,0x03,0x02,0x02,0x02,0x02,0x03,0x01,0x00,0x00},/*"0",61*/
{0x00,0x00,0x08,0x08,0x08,0xF8,0xFC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x02,0x02,0x03,0x03,0x02,0x02,0x02,0x00,0x00},/*"1",62*/
{0x00,0x18,0x18,0x04,0x84,0x84,0x44,0x2C,0x3C,0x18,0x00,0x00,0x00,0x02,0x03,0x03,0x02,0x02,0x02,0x02,0x02,0x03,0x00,0x00},/*"2",63*/
{0x00,0x08,0x0C,0x04,0x24,0x24,0x24,0x5C,0xD8,0x80,0x00,0x00,0x00,0x01,0x03,0x02,0x02,0x02,0x02,0x03,0x01,0x01,0x00,0x00},/*"3",64*/
{0x00,0x40,0x40,0xA0,0x90,0x88,0x88,0xFC,0xFC,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x03,0x03,0x02,0x00,0x00},/*"4",65*/
{0x00,0x00,0x3C,0x44,0x24,0x24,0x24,0x24,0xE4,0xC4,0x00,0x00,0x00,0x01,0x03,0x02,0x02,0x02,0x02,0x03,0x01,0x01,0x00,0x00},/*"5",66*/
{0x00,0xF0,0xF8,0x48,0x24,0x24,0x24,0x24,0x2C,0xE8,0xC0,0x00,0x00,0x00,0x01,0x03,0x02,0x02,0x02,0x02,0x02,0x01,0x01,0x00},/*"6",67*/
{0x00,0x00,0x0C,0x04,0x04,0x04,0xC4,0x24,0x14,0x0C,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x03,0x00,0x00,0x00,0x00,0x00},/*"7",68*/
{0x00,0x98,0xDC,0x64,0x24,0x24,0x24,0x64,0x6C,0xDC,0x98,0x00,0x00,0x01,0x01,0x03,0x02,0x02,0x02,0x02,0x02,0x01,0x01,0x00},/*"8",69*/
{0x00,0x38,0x78,0x6C,0x44,0x44,0x44,0x44,0xAC,0xF8,0xF0,0x00,0x00,0x00,0x01,0x03,0x02,0x02,0x02,0x03,0x01,0x01,0x00,0x00},/*"9",70*/
{0x00,0x00,0x00,0x00,0x00,0xC0,0xC0,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x30,0x00,0x00,0x00,0x00},/*":",71*/
{0x00,0xFC,0x94,0x94,0x94,0xFF,0x94,0x94,0x94,0xFE,0x04,0x00,0x00,0x01,0x00,0x00,0x00,0x03,0x04,0x04,0x04,0x04,0x06,0x00},/*"��",72*/
{0x11,0xA6,0x70,0x02,0xD2,0x1A,0xD7,0x12,0xDA,0x32,0x02,0x00,0x01,0x07,0x04,0x02,0x01,0x00,0x07,0x00,0x07,0x04,0x07,0x00} /*"��",73*/

};
const unsigned char asc6x8[1024]= {
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x1c,0x22,0x24,0x18,0x16,0x00,0x00, /*!:��*/
  0x00,0x00,0x07,0x00,0x07,0x00,0x00,0x00, /*"*/
  0x00,0x14,0x3e,0x14,0x3e,0x14,0x00,0x00, /*#:��*/
  0x00,0x7e,0x15,0x15,0x15,0x0a,0x00,0x00, /*$:��*/
  0x00,0x23,0x13,0x08,0x64,0x62,0x00,0x00, /*%*/
  0x00,0x36,0x49,0x55,0x22,0x50,0x00,0x00, /*&*/
  0x00,0x00,0x05,0x03,0x00,0x00,0x00,0x00, /*'*/
  0x00,0x00,0x1C,0x22,0x41,0x00,0x00,0x00, /*(*/
  0x00,0x00,0x41,0x22,0x1C,0x00,0x00,0x00, /*)*/
  0x00,0x14,0x08,0x3E,0x08,0x14,0x00,0x00, /***/
  0x00,0x08,0x08,0x3E,0x08,0x08,0x00,0x00, /*+*/
  0x00,0x00,0x50,0x30,0x00,0x00,0x00,0x00, /*;*/
  0x00,0x08,0x08,0x08,0x08,0x08,0x00,0x00, /*-*/
  0x00,0x00,0x60,0x60,0x00,0x00,0x00,0x00, /*.*/
  0x00,0x20,0x10,0x08,0x04,0x02,0x00,0x00, /* / */
  0x00,0x3E,0x51,0x49,0x45,0x3E,0x00,0x00, /*0*/
  0x00,0x00,0x42,0x7F,0x40,0x00,0x00,0x00, /*1*/
  0x00,0x42,0x61,0x51,0x49,0x46,0x00,0x00, /*2*/
  0x00,0x21,0x41,0x45,0x4B,0x31,0x00,0x00, /*3*/
  0x00,0x18,0x14,0x12,0x7F,0x10,0x00,0x00, /*4*/
  0x00,0x27,0x45,0x45,0x45,0x39,0x00,0x00, /*5*/
  0x00,0x3C,0x4A,0x49,0x49,0x30,0x00,0x00, /*6*/
  0x00,0x01,0x01,0x79,0x05,0x03,0x00,0x00, /*7*/
  0x00,0x36,0x49,0x49,0x49,0x36,0x00,0x00, /*8*/
  0x00,0x06,0x49,0x49,0x29,0x1E,0x00,0x00, /*9*/
  0x00,0x00,0x36,0x36,0x00,0x00,0x00,0x00, /*:*/
  0x00,0x00,0x56,0x36,0x00,0x00,0x00,0x00, /*;*/
  0x00,0x08,0x14,0x22,0x41,0x00,0x00,0x00, /*<*/
  0x00,0x14,0x14,0x14,0x14,0x14,0x00,0x00, /*=*/
  0x00,0x00,0x41,0x22,0x14,0x08,0x00,0x00, /*>*/
  0x00,0x02,0x01,0x51,0x09,0x06,0x00,0x00, /*?*/
  0x00,0x32,0x49,0x79,0x41,0x3E,0x00,0x00, /*@*/
  0x00,0x7E,0x11,0x11,0x11,0x7E,0x00,0x00, /*A*/
  0x00,0x41,0x7F,0x49,0x49,0x36,0x00,0x00, /*B*/
  0x00,0x3E,0x41,0x41,0x41,0x22,0x00,0x00, /*C*/
  0x00,0x41,0x7F,0x41,0x41,0x3E,0x00,0x00, /*D*/
  0x00,0x7F,0x49,0x49,0x49,0x49,0x00,0x00, /*E*/
  0x00,0x7F,0x09,0x09,0x09,0x01,0x00,0x00, /*F*/
  0x00,0x3E,0x41,0x41,0x49,0x7A,0x00,0x00, /*G*/
  0x00,0x7F,0x08,0x08,0x08,0x7F,0x00,0x00, /*H*/
  0x00,0x00,0x41,0x7F,0x41,0x00,0x00,0x00, /*I*/
  0x00,0x20,0x40,0x41,0x3F,0x01,0x00,0x00, /*J*/
  0x00,0x7F,0x08,0x14,0x22,0x41,0x00,0x00, /*K*/
  0x00,0x7F,0x40,0x40,0x40,0x40,0x00,0x00, /*L*/
  0x00,0x7F,0x02,0x0C,0x02,0x7F,0x00,0x00, /*M*/
  0x00,0x7F,0x06,0x08,0x30,0x7F,0x00,0x00, /*N*/
  0x00,0x3E,0x41,0x41,0x41,0x3E,0x00,0x00, /*O*/
  0x00,0x7F,0x09,0x09,0x09,0x06,0x00,0x00, /*P*/
  0x00,0x3E,0x41,0x51,0x21,0x5E,0x00,0x00, /*Q*/
  0x00,0x7F,0x09,0x19,0x29,0x46,0x00,0x00, /*R*/
  0x00,0x26,0x49,0x49,0x49,0x32,0x00,0x00, /*S*/
  0x00,0x01,0x01,0x7F,0x01,0x01,0x00,0x00, /*T*/
  0x00,0x3F,0x40,0x40,0x40,0x3F,0x00,0x00, /*U*/
  0x00,0x1F,0x20,0x40,0x20,0x1F,0x00,0x00, /*V*/
  0x00,0x7F,0x20,0x18,0x20,0x7F,0x00,0x00, /*W*/
  0x00,0x63,0x14,0x08,0x14,0x63,0x00,0x00, /*X*/
  0x00,0x07,0x08,0x70,0x08,0x07,0x00,0x00, /*Y*/
  0x00,0x61,0x51,0x49,0x45,0x43,0x00,0x00, /*Z*/
  0x00,0x00,0x7F,0x41,0x41,0x00,0x00,0x00, /*[*/
  0x00,0x02,0x04,0x08,0x10,0x20,0x00,0x00, /*\*/
  0x00,0x00,0x41,0x41,0x7F,0x00,0x00,0x00, /*]*/
  0x00,0x04,0x02,0x01,0x02,0x04,0x00,0x00, /*^*/
  0x00,0x40,0x40,0x40,0x40,0x40,0x00,0x00, /*_*/
  0x00,0x00,0x00,0x07,0x05,0x07,0x00,0x00, /*`���� */
  0x00,0x20,0x54,0x54,0x54,0x78,0x00,0x00, /*a*/
  0x00,0x7F,0x48,0x44,0x44,0x38,0x00,0x00, /*b*/
  0x00,0x38,0x44,0x44,0x44,0x28,0x00,0x00, /*c*/
  0x00,0x38,0x44,0x44,0x48,0x7F,0x00,0x00, /*d*/
  0x00,0x38,0x54,0x54,0x54,0x18,0x00,0x00, /*e*/
  0x00,0x00,0x08,0x7E,0x09,0x02,0x00,0x00, /*f*/ 
  0x00,0x0C,0x52,0x52,0x4C,0x3E,0x00,0x00, /*g*/
  0x00,0x7F,0x08,0x04,0x04,0x78,0x00,0x00, /*h*/
  0x00,0x00,0x44,0x7D,0x40,0x00,0x00,0x00, /*i*/
  0x00,0x20,0x40,0x44,0x3D,0x00,0x00,0x00, /*j*/
  0x00,0x00,0x7F,0x10,0x28,0x44,0x00,0x00, /*k*/
  0x00,0x00,0x41,0x7F,0x40,0x00,0x00,0x00, /*l*/
  0x00,0x7C,0x04,0x78,0x04,0x78,0x00,0x00, /*m*/
  0x00,0x7C,0x08,0x04,0x04,0x78,0x00,0x00, /*n*/
  0x00,0x38,0x44,0x44,0x44,0x38,0x00,0x00, /*o*/
  0x00,0x7E,0x0C,0x12,0x12,0x0C,0x00,0x00, /*p*/
  0x00,0x0C,0x12,0x12,0x0C,0x7E,0x00,0x00, /*q*/
  0x00,0x7C,0x08,0x04,0x04,0x08,0x00,0x00, /*r*/
  0x00,0x58,0x54,0x54,0x54,0x64,0x00,0x00, /*s*/
  0x00,0x04,0x3F,0x44,0x40,0x20,0x00,0x00, /*t*/
  0x00,0x3C,0x40,0x40,0x3C,0x40,0x00,0x00, /*u*/
  0x00,0x1C,0x20,0x40,0x20,0x1C,0x00,0x00, /*v*/
  0x00,0x3C,0x40,0x30,0x40,0x3C,0x00,0x00, /*w*/
  0x00,0x44,0x28,0x10,0x28,0x44,0x00,0x00, /*x*/
  0x00,0x1C,0xA0,0xA0,0x90,0x7C,0x00,0x00, /*y*/
  0x00,0x44,0x64,0x54,0x4C,0x44,0x00,0x00, /*z*/
  0x00,0x00,0x08,0x36,0x41,0x00,0x00,0x00, /*{*/
  0x00,0x00,0x00,0x77,0x00,0x00,0x00,0x00, /*|*/
  0x00,0x00,0x41,0x36,0x08,0x00,0x00,0x00, /*}*/
  0x00,0x08,0x04,0x08,0x10,0x08,0x00,0x00, /*~*/
  0x00,0x5c,0x62,0x02,0x62,0x5c,0x00,0x00  /*OM*/
};

#endif  // asc6x8