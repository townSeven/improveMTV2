#ifndef _PARSE_H
#define _PARSE_H
#include "node.h"
#include "stdafx.h"
#include <vector>

using namespace std;

/*
** node tag is designed for identifying whether the node 
** is aggregation node or normal node, i.e., the attribute field.
** for example, the following query:
**
**                  select sum(lo_revenue), d_year, p_brand1
**                  from lineorder, dwdate, part, supplier
**                  where lo_orderdate = d_datekey
**                  and lo_partkey = p_partkey
**                  and lo_suppkey = s_suppkey
**                  and p_brand1= 'MFGR#2239'
**                  and s_region = 'EUROPE'
**                  group by d_year, p_brand1
**                  order by d_year, p_brand1;
**
** the sum(lo_revenue) is aggNode, and d_year, p_brand1 is attriNode
*/


/*
** the following define is for aggregation
** Op: +,-,*, /
** lhs: the left hand side operator
** rhs: the right hand side operator
** aggFlag: sum, Count, and so on...
*/
typedef enum {
   NONE, 
   PLUS, 
   MINUS, 
   MULTIPLY, 
   DIVIDE
}Op;

typedef enum { // sum(...) or count(...)
   SUM, COUNT
}AggFlag;

typedef struct AttriNode {
	Node  nodeType;
	char  attriName[20];
    short whichRel, whichAttri;
}AttriNode;

typedef struct AggNode {
	Node      nodeType;
	char      alias[20];
	Op        op;
    AttriNode lNode, rNode;
	AggFlag   aggFlag;
}AggNode;


////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum State{
     ATTRISTATE, 
	 AGGSTATE
}State;

typedef enum whereState{
	  Begin, 
      LBracket, RBracket,    

	  BA,
	  B1A,
	  BAnd,
	  BA1,

	  OneDim
	  
}whereState;

typedef enum CompOperator{
	 EQ,    //=
	 LT,    //<
	 GT,    //>
	 LE,    //<=
	 GE,     // >=
	 BETWEENAND //between...and...
}CompOperator;

typedef struct condition
{
    AttriNode aNode;
}condition;  // not used.

typedef struct DimCondition{
	char      condtion[100];
	AttriNode aNode;
	int       predConn;  //not used.
}DimCondition;

typedef struct GroupInfo {
	bool      isTranslatedByMap;
    AttriNode RelInfo;
}GroupInfo;


typedef struct SortInfo {
   short type;
   short sortPosInGroupby;
   bool  isAsc;
}SortInfo;

typedef struct Select {
  vector<Node*>        targetList;
  bool                 fromList[5];
  vector<DimCondition> whereCondition;
  bool                 hasAgg;
  bool                 hasOrderBy;
  vector<GroupInfo>    groupby;
  vector<SortInfo>     orderby;
}SelectType;


char* getToken (char* zSql, int* start);

#endif
