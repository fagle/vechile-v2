
#ifndef __TRAFFIC_H__
#define __TRAFFIC_H__

/////////////////////////////////////////////////////////////////////////////////////////
//
#include "config.h"
#include "lamp.h"
#include "frame.h"

/////////////////////////////////////////////////////////////////////////////////////////
//
#define MAXCARLANE        (0x20)
#define MAXVERS            20
#define MAXARCS            20
#define MAXQUEUE           20

/////////////////////////////////////////////////////////////////////////////////////////
//
typedef s8 GraphStatus;

typedef struct arc_t
{
    u8  vex[0x02];
} arc_t, *parc_t;

typedef struct gring_t
{
    u8   in, out;
    u8   node[MAXQUEUE];
    GraphStatus (*ept)  ( struct gring_t * );
    GraphStatus (*get)  ( struct gring_t *, u8 * );
    GraphStatus (*put)  ( struct gring_t *, u8 );
    GraphStatus (*clr)  ( struct gring_t * );
} gring_t;

typedef struct anode_t          // ͷ�ڵ�
{
    u8     adjvex;              // �ñ���ָ��Ķ����λ��
    struct anode_t *next;       // ָ����һ����
} anode_t, *panode_t;

typedef struct vnode            // ��ڵ�
{
    u8        data;             // ������Ϣ
    anode_t * first;            // ָ���һ�������ýڵ�ıߵ�ָ��
} vnode_t, *pvnode_t;           // adjlist[MAXLISTS];

typedef struct graph_t
{
    u8      vexnum;             // �ڵ�ĸ���
    u8      arcnum;             // �ߵ�����
    u8      arcidx;             // 
    u8      visited[MAXVERS];   // 
    vnode_t verlist[MAXVERS];   // ��ڵ�
    anode_t arclist[MAXARCS];   // �߽ڵ�
    GraphStatus (*build) ( struct graph_t *, u8, u8, parc_t ); 
    GraphStatus (*find)  ( struct graph_t *, u8 ); 
    void        (*print) ( struct graph_t * ); 
    void        (*dfs)   ( struct graph_t * ); 
    void        (*bfs)   ( struct graph_t * ); 
} graph_t, *pgraph_t;

/////////////////////////////////////////////////////////////////////////////////////////
//
typedef struct
{
    u8  card;
    u8  status:1;
    u8  cnt:5;
    u8  in:5;
    u8  out:5;
    u8  car[MAXCARLANE];
} lane_t, *plane_t;

typedef struct
{
    u8      state[(MAXCARDS >> 0x03) + 0x01];
    u8      lanesize;
    lane_t  lane[MAXCARDS];
    
    void    (*put) ( u8 card, u8 car );
    u8      (*get) ( u8 card );
    void    (*del) ( u8 card, u8 car );
    void    (*clr) ( u8 card );
    u8      (*ept) ( u8 card );
    void    (*prt) ( u8 card );
    plane_t (*find)( u8 card );
} traffic_info_t, *ptraffic_info_t;

/////////////////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void sea_initraffic ( void )
//* ����        : initialze traffic_info structure body
//* �������    : ��
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_initraffic ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* ������      : void sea_initgraph ( graph_t * G )
//* ����        : initialize graph structure body
//* �������    : graph_t * G
//* �������    : ��
//* �޸ļ�¼    : ��
//* ��ע        : ��
//*------------------------------------------------*/
void sea_initgraph ( graph_t * G );

/////////////////////////////////////////////////////////////////////////////////////
//
extern traffic_info_t traffic_info;

/////////////////////////////////////////////////////////////////////////////////////////
//
#endif  // __TRAFFIC_H__
