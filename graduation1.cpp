#include "stdafx.h"
#include "sysCatalog.h"
#include "rewriteSQL1.h"
#include "table.h"
#include "node.h"
#include "parse.h"
#include "utilities.h"
#include "exec_ddta.h"
#include "resultSet.h"		//@town: to solve the error: ‘result’ {aka ‘struct result’} has no member named ‘hashgroup’
#include "connection.h"		//@town Feb.2nd: [add]
#include <vector>
#include <iostream>
#include <algorithm>
#include <bitset>
#include <string.h>
#include <stdio.h>
#include  "odbcinst.h"
#include "timer.h"
#include <pthread.h>
#include <ctype.h>
#include <stdlib.h>
#include "SQLClauses.h"


extern "C" {
#include "socketConn.h"
}

extern "C" {
#include "connection.h"
}



using namespace boost;
using namespace std;


#ifndef DEBUG 
#define DEBUG true
#endif

int  sockfd, newfd;

char recv_buf[1024], zSql[1024];

int query = -1;		//@town: which query(global)

vector<sysCatalogCell*> sysCatalog;
TableType LOTable;
SQLHDBC dbc;

char TblName[][15] = { "customer", "supplier",
		               "dwdate", "part",
		               "lineorder"
                      };


void freeMem(Select& selectVar, ddtacontext& context)
{
    vector<GroupInfo>( ).swap(selectVar.groupby);
    selectVar.groupby.clear( );

    vector<SortInfo>( ).swap(selectVar.orderby);
    selectVar.orderby.clear( );

    vector<DimCondition>( ).swap(selectVar.whereCondition);
    selectVar.whereCondition.clear( );

    vector<Node*>( ).swap(selectVar.targetList);
    selectVar.targetList.clear( );

    vector<int>( ).swap(context.predSupplier);
    context.predSupplier.clear( );

    vector<int>( ).swap(context.predCustomer);
    context.predCustomer.clear( );

    vector<int>( ).swap(context.predPart);
    context.predPart.clear( );

    vector<int>( ).swap(context.predDwdate);
    context.predDwdate.clear( );

}


void parse (char* zSql, SelectType& selectVar, ddtacontext& context)
{
	char* pToken;
	int nextPos = 0;
	AttriNode aNode;
	int start = 0;

	fillTargetList(zSql, selectVar.targetList, &start, sysCatalog);	//��ȡĿ���б�

	if (DEBUG) {
		vector<Node*> &targetList = selectVar.targetList;
		for (size_t i = 0; i < targetList.size(); i++)
			if (targetList[i]->type == T_AGG) {
				printf("Aggregation node: ");
				AggNode* p = (AggNode*) (targetList[i]);
				printf("(rel-%d, attri--%d) ", p->lNode.whichRel,
						p->lNode.whichAttri);
				if (p->op != NONE) {
					printf("(rel--%d, attri--%d) ", p->rNode.whichRel,
							p->rNode.whichAttri);
					switch (p->op) {
					case MULTIPLY:
						printf("*");
						break;
					case MINUS:
						printf("-");
						break;
					case PLUS:
						printf("+");
						break;
					}
				}
				printf("\n");
			} else {
				printf("attribute node: ");
				AttriNode* p = (AttriNode*) targetList[i];
				printf("(rel--%d, attri--%d) ", p->whichRel, p->whichAttri);
				printf("\n");
			}
	}

	fillRangeTbl(zSql, selectVar.fromList, &start, TblName);	

	selectVar.hasAgg = fillWhere(zSql, &start, selectVar.whereCondition,
			             sysCatalog, context);

	vector<DimCondition> &allDimCondition = selectVar.whereCondition;

	for (size_t i = 0; i < allDimCondition.size(); i++) {

		int tblPos = allDimCondition[i].aNode.whichRel;

		switch (tblPos) {
		case 0:
			printf("customer: ");
			printf("%s\n", allDimCondition[i].condtion);
			break;
		case 1:
			printf("supplier: ");
			printf("%s\n", allDimCondition[i].condtion);
			break;
		case 2:
			printf("dwdate: ");
			printf("%s\n", allDimCondition[i].condtion);
			break;

		case 3:
			printf("part: ");
			printf("%s\n", allDimCondition[i].condtion);
			break;
		case 4:
			printf("lineorder: ");
			printf("%s\n", allDimCondition[i].condtion);
			break;
		}
	}

	rewrtSql(selectVar.targetList, selectVar.whereCondition, context,
			sysCatalog);

	if (selectVar.hasAgg)
		selectVar.hasOrderBy = fillGroupby(zSql, &start, selectVar.groupby,
				sysCatalog);

	if (selectVar.hasOrderBy)
		fillOrderby(zSql, &start, selectVar.orderby, selectVar.groupby,
				sysCatalog);


}

////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
	//   pre. use SSB to generate data(lineorder....)
	//1. load data dimension table into mysql
	//2. check the result of the parse sql
	/*
	** build up the system catalog
	** position information: (0: customer); (1:supplier);
	** (2:dwdate);(3:part); (4:lineorder)
	*/

	sysCatalog.push_back(customer);
	sysCatalog.push_back(supplier);
    sysCatalog.push_back(dwdate);
	sysCatalog.push_back(part);
	sysCatalog.push_back(lineorder);

    bool dataLoaded = false;

	float whichQuery[13] = {
		1.1, 1.2, 1.3, 2.1, 2.2, 2.3, 3.1, 3.2, 3.3, 3.4, 4.1, 4.2, 4.3
	  //0  , 1  , 2  , 3  , 4  , 5  , 6  , 7  , 8  , 9  , 10 , 11 , 12
	};

	connToDB(&dbc);

	for(int i = 0; i < 13 ; i++) {
		{
			ddtacontext context;
			result res;
			SelectType selectVar;
			runtimeInfoType runtimeInfo;
			timer startTime, endTime;

			char res2Client[1024 * 1000];
			query = i;

			if (!dataLoaded) {
				loadFactTable(LOTable, runtimeInfo);
				dataLoaded = true;
			}

			//��չ�Բ���
			//if (i != 0 && i != 3 && i != 6 && i != 10) continue;

			printf("\n============================== Q %.1f ==============================\n", whichQuery[i]);

			//Q1.1 0
			//"select sum(lo_extendedprice*lo_discount) as \
		// revenue \
		// from lineorder, dwdate \
		// where lo_orderdate = d_datekey \
		// and d_year = 1993 \
		// and lo_discount between 1 and 3 \
		// and lo_quantity < 25",

			startTime = timerStart( );
			parse(pSQL[i], selectVar, context);
			endTime = timerEnd( );
			cout << endl << elapsedTime(startTime, endTime) << " ms. * phase: parse *" << endl << endl;

			getAttrsInFactTble(context, sysCatalog, runtimeInfo.factTblAttrs);

			getFilters(context, runtimeInfo.filter, &runtimeInfo.nFilter);	//�õ���ʵ���ϵĹ�������


			startTime = timerStart( );

			producePrediction(context, dbc);

			endTime = timerEnd( );

			cout << elapsedTime(startTime, endTime) << " ms. phase 1" << std::endl;

			startTime = timerStart( );

			exec_cddta_mt(context, selectVar, LOTable, res, runtimeInfo);

			// cout << "the group size is: " << res.hashgroup.size( ) << endl;

			endTime = timerEnd( );

			cout << elapsedTime(startTime, endTime) << " ms." << std::endl;

			freeMem(selectVar, context);
		}//block
	}//for

	for (int i = 0; i < 9; i++) {
		memset(LOTable.pLOTable[i], 0, N_TUPLE * sizeof(int));
		free(LOTable.pLOTable[i]);
	}

    sysCatalog.clear( );

    SQLDisconnect(dbc);

	//system("pause");	//@town

	return 0;
}

