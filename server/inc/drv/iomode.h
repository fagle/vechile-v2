
#ifndef __IOMODE_H__
#define __IOMODE_H__

#include "stm32f10x.h"

/************************************************
* ������      : u8 Char_To_Num( u8 ch )
* ����        : ���ַ�ת���ɶ�Ӧ������
* �������    : ��
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
u8 Char_To_Num( u8 ch );

/************************************************
* ������      : u8 Num_To_Char( u8 ch )
* ����        : ���ַ�ת���ɶ�Ӧ������
* �������    : ��
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
u8 Num_To_Char( u8 ch );

/***********************************************
* ������      : void Hex_To_TwoChar( u8 high , u8 low)
* ����        : ����iomode�����ʮ�����ƺ��ַ�����ת����Э����ͨ��EM310�����
                ���ϱ�����ʱ���뽫ʮ�������������ַ���ʾ�����磬����ָ�68 42 
                04 00 46 43������ָ����at%ipsend="684204004643"��ÿ�����ֶ���һ
                ���ַ���Ҫ����0x68����Ҫ�á�6���͡�8�������ַ���ʾ
* �������    : ����λ�͵���λ
* �������    : ʮ����������
* �޸ļ�¼    : ��
* ��ע        : ��
************************************************/
void Hex_To_TwoChar( u8 ch , u8 * high , u8 * low);

/************************************************
* ������      : void Num_To_String( u16 size, u8 * str )
* ����        : ������ת����һ���ַ�
* �������    : u16 size, u8 * str
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
void Num_To_String ( u16 size, u8 * str );

/************************************************
* ������      : u8 GPRS_IpsendByte( u8 ch )
* ����        : ʹ��ipsend����һ�ֽ�����
* �������    : �ֽ�
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
u8 GPRS_IpsendByte( u8 ch );

/************************************************
* ������      : u8 u8 GPRS_Ipsend2Byte ( u8 ch )
* ����        : ʹ��ipsend����2�ֽ�����
* �������    : �ֽ�
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
u8 GPRS_Ipsend2Byte ( u8 ch );

/************************************************
* ������      : u8 GPRS_IpsendNumber ( u16 size )
* ����        : ʹ��ipsend����һ�ֽ�����
* �������    : u16 size
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
u8 GPRS_IpsendNumber ( u16 size );

/************************************************
* ������      : u8 GPRS_IpsendString ( u8 * pointer )
* ����        : ʹ��ipsend��������
* �������    : u8 * pointer
* �������    : ture / flase
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
u8 GPRS_IpsendString ( u8 * pointer );

/************************************************
* ������      : u8 GPRS_Ipsend( u8 * pointer )
* ����        : ����һ������
* �������    : �ַ���
* �������    : ��
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
u8 GPRS_Ipsend( u8 * pointer );

/************************************************
* ������      : u8 GPRS_IpwaitAnswer ( void )
* ����        : waiting for response
* �������    : ��
* �������    : true / flase
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
u8 GPRS_IpwaitAnswer ( void );

/************************************************
* ������      : u8 GPRS_IpsendOpen ( void )
* ����        : send open to web
* �������    : ��
* �������    : ture / flase
* �޸ļ�¼    : ��
* ��ע        : ��
*************************************************/
u8 GPRS_IpsendOpen ( void );

#endif   // __IOMODE_H__

