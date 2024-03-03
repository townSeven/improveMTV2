#ifndef _EXEC_DDTA_H
#define _EXEC_DDTA_H

#include "parse.h"
#include "rewriteSQL1.h"
#include "resultSet.h"
#include "utilities.h"
#include <vector>
#include "stdafx.h"
#include <stdlib.h>
#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>
#include <stdlib.h>
#include <odbcinst.h>
#include "node.h"

extern "C"{
#include "connection.h"
}


using namespace std;

const int m[13] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

#define OLAP_START_DATE 19920101
#define OLAP_END_DATE   19981230
#define LEAP_DAY1       19920229
#define LEAP_DAY2       19960229

#define CLUSTER_NUM  3

//inline void getDimData(ddtacontext& context,
//	vector<GroupInfo>& grpInfo,
//	resultTuple2& restup,
//	int* dimIndex);

inline void getDimData(ddtacontext& context,
	vector<GroupInfo>& grpInfo,
	vector<int>& data,
	int* dimIndex);


inline void 
LoDimKeyToIndex(ddtacontext &context, int* dimIndex, int* LOFKeyIndex);

void        
execGroup(ddtacontext &context, result &res, SelectType &selectVar, int* dimIndex, long aggValue);


inline bool 
predictionJudge(ddtacontext &context, int* dimIndex, bool* flags);

void
exec_cddta(ddtacontext &context, SelectType &selectVar, TableType&LOTable,
		   result& res, runtimeInfoType& runtimeInfo);


void
producePrediction(ddtacontext &context, SQLHDBC& dbc);

void 
exec_cddta_thread(void * arg, ddtacontext&context, TableType&LOTable, SelectType&selectVar);

void exec_cddta_mt(ddtacontext& context, SelectType& selectVar,
		          TableType& factTable, result& res, runtimeInfoType& runtimeInfo);

inline bool
getNeededData(runtimeInfoType& runtimeInfo, SelectType& selectVar,
		       TableType& LOTable, int ntup, int* fKey, int& factValue, Op opType);

//void
//printResult(ddtacontext &context, result &res,
//		    SelectType &selectVar, char* res2Client
//		   );

inline void printResult(ddtacontext& context, result& res, SelectType& selectVar);

inline bool predJudge(ddtacontext& context, int fKey, int tblPos );


#endif
