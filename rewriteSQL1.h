#ifndef _REWRITESQL1_H
#define _REWRITESQL1_H

#include <boost/dynamic_bitset.hpp>
#include <boost/bimap.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/unordered_map.hpp>
#include "node.h"
#include "parse.h"
#include "sysCatalog.h"
#include <vector>
#include <string.h>
#include "stdafx.h"


using namespace std;
using namespace boost;
//typedef unordered_map <resulttuple, int64>::iterator hash_iterator;


typedef struct ddtacontext {
	//if the type got from database is string, then we
	//change it from bimap, (using string<-->int bimapping)supported by c++ boost library
	vector<int> predCustomer; 
	vector<int> predSupplier;
	vector<int> predPart;
	vector<int> predDwdate;

	bimap<string, int> mapForCustomer;		
	bimap<string, int> mapForSupplier;
    bimap<string, int> mapForPart;
	bimap<string, int> mapForDwdate; 

	dynamic_bitset<> predbitCust, predbitSupp, predbitPart, predbitDate;

    char  rewrtSQLStat[5][200];
	bool  FKeyIndicator[4];		

	bool  isTranslated[4];

	long index_part;
	long index_supplier;
	long index_dwdate;
	long index_customer;

    //vector <hash_iterator> IteratorLO;
    //boost::dynamic_bitset<> PredbitLO;
}ddtacontext;


extern void rewrtSql (vector<Node*>&targetList,
			   vector<DimCondition> &allDimCondition,
			   ddtacontext& context,
			   vector<sysCatalogCell*>& sysCatalog
			   );

#endif
