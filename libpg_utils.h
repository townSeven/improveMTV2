#ifndef _UTILITIES_H
#define _UTILITIES_H
#include "parse.h" 
#include "rewriteSQL1.h"
#include "sysCatalog.h"
#include <vector>
#include <boost/dynamic_bitset.hpp>
#include <nlohmann/json.hpp>

using namespace std;
using namespace boost;
using json = nlohmann::json;

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

int getTargetList(const json& parsed_tree, 
                   vector<Node*>& tlist,
                   vector<sysCatalogCell*>& sysCatalog);

int getRangeTbl(const json& parsed_tree, 
				bool *rTblAffected,
				char tblName[][15]);

int getWhereClause(const json& parsed_tree,
			       vector<DimCondition>& allDimCond, 
			       vector<sysCatalogCell*>& sysCatalog,
			       ddtacontext& context);

int getGroupClause(const json& parsed_tree,
				vector<GroupInfo> &groupby,
				vector<sysCatalogCell*> &sysCatalog
				);

void getOrderClause(const json& parsed_tree,
				    vector<SortInfo>&orderby,
				    vector<GroupInfo> &groupby,
				    vector<sysCatalogCell*> &sysCatalog
				    );

void getAttrsInFactTble(ddtacontext&context, 
						vector<sysCatalogCell*> &sysCatalog,
		        		short* attrs);

void getFilters(ddtacontext &context, short* filters, short* nFilter);

int  str2digit(char* strDigit);


const int N_TUPLE     = 6500000;
const int n_LO_attrs  = 9;

typedef struct TableType {

    int *pLOTable[9];
	long size;
}TableType;	

void loadFactTable(TableType& LOTable, runtimeInfoType&runtimeinfo);

#endif //_UTILITIES_H