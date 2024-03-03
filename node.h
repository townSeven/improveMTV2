#ifndef _NODE_H
#define _NODE_H

/*
** we have the two types of nodes: pareser node and table node
** 
**<Parse Node> is as following:
**  attribute node: used in the parser, for attribute field
**  aggregation node: used in the parser, for aggregation field.
**
**<Table Node> is as following:
**  T_CUSTOMER table and so on.
*/

typedef enum NodeTag{
   T_ATTRI, 
   T_AGG,
   
   T_CUSTOMER = 10,
   T_SUPPLIER,
   T_DWDATE,
   T_PART,
   T_LINEORDER
}NodeTag;


typedef struct Node{
	NodeTag type;
}Node;

#endif