
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

typedef struct anode_t          // 头节点
{
    u8     adjvex;              // 该边所指向的顶点的位置
    struct anode_t *next;       // 指向下一条边
} anode_t, *panode_t;

typedef struct vnode            // 表节点
{
    u8        data;             // 顶点信息
    anode_t * first;            // 指向第一条依附该节点的边的指针
} vnode_t, *pvnode_t;           // adjlist[MAXLISTS];

typedef struct graph_t
{
    u8      vexnum;             // 节点的个数
    u8      arcnum;             // 边的条数
    u8      arcidx;             // 
    u8      visited[MAXVERS];   // 
    vnode_t verlist[MAXVERS];   // 表节点
    anode_t arclist[MAXARCS];   // 边节点
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
//* 函数名      : void sea_initraffic ( void )
//* 功能        : initialze traffic_info structure body
//* 输入参数    : 无
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_initraffic ( void );

/////////////////////////////////////////////////////////////////////////////////////////////
//
//* 函数名      : void sea_initgraph ( graph_t * G )
//* 功能        : initialize graph structure body
//* 输入参数    : graph_t * G
//* 输出参数    : 无
//* 修改记录    : 无
//* 备注        : 无
//*------------------------------------------------*/
void sea_initgraph ( graph_t * G );

/////////////////////////////////////////////////////////////////////////////////////
//
extern traffic_info_t traffic_info;

/////////////////////////////////////////////////////////////////////////////////////////
//
#endif  // __TRAFFIC_H__
