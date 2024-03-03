#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "sysCatalog.h"
#include "libpg_utils.h"
#include "rewriteSQL1.h"
#include "resultSet.h"
#include "exec_ddta.h"
#include "connection.h"
#include "timer.h"

#ifndef DEBUG 
#define DEBUG true
#endif

using namespace std;
using json = nlohmann::json;

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

void parse (const json& parsed_tree, SelectType& selectVar, ddtacontext& context) {
	
	getTargetList(parsed_tree, selectVar.targetList, sysCatalog);
	
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

	getRangeTbl(parsed_tree, selectVar.fromList, TblName);

	// if (DEBUG)
	// {
	// 	cout << "[** RangeTbl: " ;
	// 	for (int i = 0; i < 5; i++)
	// 	{
	// 		if (selectVar.fromList[i] == true)
	// 			cout << TblName[i] << " ";
	// 	}
	// 	cout << " **]" << endl;
	// }
	
	getWhereClause(parsed_tree, selectVar.whereCondition, sysCatalog, context);
	selectVar.hasAgg = parsed_tree[0]["SelectStmt"].contains("groupClause");
	selectVar.hasOrderBy = parsed_tree[0]["SelectStmt"].contains("sortClause");

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
	{
		getGroupClause(parsed_tree, selectVar.groupby, sysCatalog);
	}

	if (selectVar.hasOrderBy)
	{
		getOrderClause(parsed_tree, selectVar.orderby, selectVar.groupby, sysCatalog);
	}
}

int main() {
    sysCatalog.push_back(customer);
	sysCatalog.push_back(supplier);
    sysCatalog.push_back(dwdate);
	sysCatalog.push_back(part);
	sysCatalog.push_back(lineorder);

	bool dataLoaded = false;

	connToDB(&dbc);

	ddtacontext context;
    SelectType selectVar;
	runtimeInfoType runtimeInfo;
	result res;
	timer startTime, endTime;

    std::string inputString; // 用于存储输入的字符串

    // 使用循环从标准输入读取每一行并追加到字符串中
    std::string line;
    while (std::getline(std::cin, line)) {
        inputString += line; // 将每行内容追加到inputString中
    }

    // 在这里可以对inputString进行后续处理，如解析、分析等操作
    // 解析JSON字符串
    auto parsed_tree = json::parse(inputString);

	if (!dataLoaded) {
		loadFactTable(LOTable, runtimeInfo);
		dataLoaded = true;
	}

	startTime = timerStart( );
    parse(parsed_tree, selectVar, context);
	endTime = timerEnd( );
	cout << endl << elapsedTime(startTime, endTime) << " ms. * phase: parse *" << endl << endl;

	getAttrsInFactTble(context, sysCatalog, runtimeInfo.factTblAttrs);

	getFilters(context, runtimeInfo.filter, &runtimeInfo.nFilter);

	startTime = timerStart( );
	producePrediction(context, dbc);
	endTime = timerEnd( );

	cout << elapsedTime(startTime, endTime) << " ms. phase 1" << endl;

	startTime = timerStart( );
	exec_cddta_mt(context, selectVar, LOTable, res, runtimeInfo);

	// cout << "the group size is: " << res.hashgroup.size( ) << endl;
	endTime = timerEnd( );

	cout << elapsedTime(startTime, endTime) << " ms." << std::endl;


	freeMem(selectVar, context);

	for (int i = 0; i < 9; i++) {
		memset(LOTable.pLOTable[i], 0, N_TUPLE * sizeof(int));
		free(LOTable.pLOTable[i]);
	}

	sysCatalog.clear( );

    SQLDisconnect(dbc);

    return 0;
}