#ifndef __COMMAND_H__
#define __COMMAND_H__

//////////////////////////////////////////////////////////////////////////////
//
#include "config.h"
#include "stm32f10x_type.h"
#include "frame.h"

/////////////////////////////////////////////////////////////////////////////////////////////
//* ������      : void sea_parseroadframe ( pframe_t ptr )
//* ����        : ������λ��ͨ�����ڷ�����Ϣ
//* �������    : pframe_t ptr
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_parseroadframe ( pframe_t ptr );

/////////////////////////////////////////////////////////////////////////////////////////////
//* ������      : void sea_parselampframe ( pframe_t ptr )
//* ����        : ������λ��ͨ�����ڷ�����Ϣ
//* �������    : pframe_t ptr
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_parselampframe ( pframe_t ptr );

/////////////////////////////////////////////////////////////////////////////////////////////
//* ������      : void sea_lampconfig  ( int argc, char * argv[] )
//* ����        : lamp command interface
//* �������    : int argc, char * argv[]
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_lampconfig  ( int argc, char * argv[] );

/////////////////////////////////////////////////////////////////////////////////////////////
//* ������      : void sea_roadconfig  ( int argc, char * argv[] )
//* ����        : road command interface
//* �������    : int argc, char * argv[]
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_roadconfig  ( int argc, char * argv[] );

/////////////////////////////////////////////////////////////////////////////////////////////
//* ������      : void sea_roadkeyboardinput ( char ch )
//* ����        : road command input from keyboard or uart
//* �������    : char ch
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_roadkeyboardinput ( char ch );

/////////////////////////////////////////////////////////////////////////////////////////////
//* ������      : void sea_lampkeyboardinput ( char ch )
//* ����        : lamp command input from keyboard or uart
//* �������    : char ch
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_lampkeyboardinput ( char ch );

//////////////////////////////////////////////////////////////////////////////
//
extern frame_info_t uartfrm;

//////////////////////////////////////////////////////////////////////////////
//
#endif  // __COMMAND_H__