#ifndef __HTTP_H__
#define __HTTP_H__

//////////////////////////////////////////////////////////////////////////////////////////
//
#include "stm32f10x.h"
#include "config.h"

/************************************************
* ������      : void sea_sendgprsmessage ( const u8 * msg, const u8 * phone )
* ����        : ���Ͷ��� from EM310
* �������    : const u8 * msg, const u8 * phone
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
void sea_sendgprsmessage ( const u8 * msg, const u8 * phone );

/************************************************
* ������      : void sea_sendopenmessage ( const u8 * phone )
* ����        : ���Ͷ��� from EM310
* �������    : const u8 * phone
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
void sea_sendopenmessage ( const u8 * phone );

/************************************************
* ������      : void sea_sendclosemessage ( const u8 * phone )
* ����        : ���Ͷ��� from EM310
* �������    : const u8 * phone
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
void sea_sendclosemessage ( const u8 * phone );

/************************************************
* ������      : u8 HTTP_IpsendGet ( void )
* ����        : send get to web
* �������    : ��
* �������    : ture / flase
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
u8 HTTP_IpsendGet ( void );

/************************************************
* ������      : u8 HTTP_IpsendPostSize ( void )
* ����        : send post to web
* �������    : ��
* �������    : ture / flase
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
void HTTP_IpsendPost ( void );

/************************************************
* ������      : u8 HTTP_IpsendPost ( u16 len )
* ����        : send post to web
* �������    : u16 len
* �������    : ture / flase
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
u8 HTTP_IpsendPostSize ( u16 len );

/************************************************
* ������      : u8 HTTP_ReportFrame ( u8 id, u8 size, u16 road_id, u8 * body )
* ����        : ��gprs����֡
* �������    : u8 id, u8 size, u16 road_id, u8 * body
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
u8 HTTP_ReportFrame ( u8 id, u8 size, u16 road, u8 * body );

//////////////////////////////////////////////////////////////////////////////////////////
//
#endif
