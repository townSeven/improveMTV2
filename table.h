#ifndef _TABLE_H
#define _TABLE_H
#include "node.h"
#include <vector>
using namespace std;

//////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct Part {
   Node nodeType;
   size_t relsize;
   vector<vector<void*> > p_attri;
   Part ( ) {nodeType.type = T_PART;}
}PartTbl;

typedef struct Customer {
   Node nodeType;
   size_t relsize;
   vector<vector<void*> > c_attri;
   Customer ( ) {nodeType.type = T_CUSTOMER;}
}CustomerTbl;

typedef struct Supplier {
   Node nodeType;
   size_t relsize;
   vector<vector<void*> > s_attri;
   Supplier ( ) {nodeType.type = T_SUPPLIER;}
}SupplierTbl;

typedef struct Dwdate {
   Node nodeType;
   size_t relsize;
   vector<vector<void*> > d_attri;
   Dwdate ( ) {nodeType.type = T_DWDATE;}
}DwdateTbl;

typedef struct lineorder {
   Node nodeType;
   size_t relsize;
   vector<vector<void*> > d_attri;
   lineorder ( ) {nodeType.type = T_LINEORDER;}
}LineorderTbl;

 static DwdateTbl dTbl;
 static LineorderTbl loTbl;
 static CustomerTbl cTbl;
 static SupplierTbl sTbl;
 static PartTbl pTbl;

#endif