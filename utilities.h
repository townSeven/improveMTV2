#ifndef _UTILITIES_H
#define _UTILITIES_H
#include "parse.h" 
#include "rewriteSQL1.h"
#include "sysCatalog.h"
#include <vector>
#include <boost/dynamic_bitset.hpp>

using namespace std;
using namespace boost;

 
typedef struct runtimeInfoType {
	  short  filter[5];
	  short  nFilter;

	  short  factTblAttrs[9]; //fact table attributes involved in query
	  short  posMap[17];


	  runtimeInfoType ( ) {
		  posMap[0]  = -1;  posMap[1]  = -1;
		  posMap[2]  = 0;   posMap[3]  = 1;  posMap[4] = 2; posMap[5] = 3;
		  posMap[6]  = -1;  posMap[7]  = -1;
		  posMap[8]  = 4,   posMap[9]  = 5;
		  posMap[10] = -1;
		  posMap[11] = 6;   posMap[12] = 7;  posMap[13] = 8;
		  posMap[14] = -1;  posMap[15] = -1; posMap[16] = -1;

	  }

}runtimeInfoType;

extern runtimeInfoType runtimeInfo;

int fillTargetList(char *zSql, 
					vector<Node*>& tlist, 
					int* start, 
					vector<sysCatalogCell*>&sysCatalog
				   );

int fillRangeTbl(char *zSql, 
				 bool *rTblAffected, 
			     int* start, 
				 char tblName[][15]
                );

bool IsOperand (const char* token);

int lookupSysCatalog( char* pToken, 
					  AttriNode* node,
					  vector<sysCatalogCell*>& sysCatalog
					 );

bool fillWhere(char *zSql, 
			   int* start, 
			   vector<DimCondition>& dimCondition, 
			   vector<sysCatalogCell*>&sysCatalog,
			   ddtacontext&context
			  );

bool fillGroupby(char *zSql,
				 int* start,
				 vector<GroupInfo>&groupby,
				 vector<sysCatalogCell*>&sysCatalog
				 );

void fillOrderby(char *zSql,
				 int* start,
				 vector<SortInfo>&orderby,
				 vector<GroupInfo>  &groupby,
				 vector<sysCatalogCell*> &sysCatalog
				 );

void getAttrsInFactTble(ddtacontext&context, 
			vector<sysCatalogCell*> &sysCatalog,
		        short* attrs);

void getFilters(ddtacontext &context, short* filters, short* nFilter);


int  str2digit(char* strDigit);

const int N_TUPLE     = 6500000;
const int n_LO_attrs  = 9;





// typedef struct TableType;

typedef struct TableType {

    int *pLOTable[9];
	long size;
}TableType;		//@town: modified



void loadFactTable(TableType& LOTable, runtimeInfoType&runtimeinfo);


#endif //_UTILITIES_H
