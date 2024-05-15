#include <iostream>
#include <fstream>
#include <string>
#include <assert.h>
#include <ctype.h>
#include <nlohmann/json.hpp>
//#include "parsenodes.h"
#include "libpg_utils.h"
#include "exec_ddta.h"
#include "parse.h"

using namespace std;
using json = nlohmann::json;

static short numberOfAttri[] = {8, //number of attributes in Customer
                         7,        //number of attributes in Supplier
						 17,       //number of attributes in dwdate
						 9,        //number of attributes in part
						 17};      //                     in lineorder

Op getOperator(string token)
{
    if (token == "\"*\"") 
        return MULTIPLY;
    else if (token == "\"+\"")
        return PLUS;
    else if (token == "\"-\"")
        return MINUS;
    else 
        return DIVIDE;
}

int lookupSysCatalog(string token, 
					 AttriNode* node,
					 vector<sysCatalogCell*>& sysCatalog)
{
        for(int i = 0; i < 5; i++){
		    sysCatalogCell *p = sysCatalog[i];
		    for(int j = 0; j < numberOfAttri[i]; j++) {
				if (token == (p + j)->attriName) {
					node->whichAttri = j;
					node->whichRel = i;
                    strcpy(node->attriName, (p + j)->attriName);
					return  ((p + j)->type);
				}
		    }
	   }
	   return -1;
}

int getTargetList(const json& parsed_tree, 
                  vector<Node*>& tlist,
                  vector<sysCatalogCell*>& sysCatalog)
{
    string token;
    AttriNode attriNode;
	AggNode aggNode;
	State state = ATTRISTATE;
    // bool opFlag = false;

    for (const auto& target : parsed_tree[0]["SelectStmt"]["targetList"]) {
        if (target["ResTarget"]["val"].contains("FuncCall"))    // if isAgg, e.g. sum, count...
        {
            // now, we know the state is switching to aggregation state
            state = AGGSTATE;
			aggNode.aggFlag = SUM;
			aggNode.nodeType.type = T_AGG;

            if (target["ResTarget"]["val"]["FuncCall"]["args"][0].contains("A_Expr"))   //if isExpr, e.g. sum(lo_extendedprice*lo_discount)
            {
                token = to_string(target["ResTarget"]["val"]["FuncCall"]["args"][0]["A_Expr"]["name"][0]["String"]["str"]);
                aggNode.op = getOperator(token);

                // left node
                token = target["ResTarget"]["val"]["FuncCall"]["args"][0]["A_Expr"]["lexpr"]["ColumnRef"]["fields"][0]["String"]["str"];
                lookupSysCatalog(token, &attriNode, sysCatalog);
                aggNode.lNode = attriNode;
                
                // right node
                token = target["ResTarget"]["val"]["FuncCall"]["args"][0]["A_Expr"]["rexpr"]["ColumnRef"]["fields"][0]["String"]["str"];
                lookupSysCatalog(token, &attriNode, sysCatalog);
                aggNode.rNode = attriNode;

                state = ATTRISTATE;
                // AggNode *ptr = (AggNode*)malloc(sizeof(AggNode));
                // memcpy(ptr, &aggNode, sizeof(aggNode));
                // tlist.push_back((Node*)ptr);
                
                //continue;
            }
            else    // notExpr, e.g. sum(lo_revenue)
            {
                token = target["ResTarget"]["val"]["FuncCall"]["args"][0]["ColumnRef"]["fields"][0]["String"]["str"];
                lookupSysCatalog(token, &attriNode, sysCatalog);
                aggNode.lNode = attriNode;

                state = ATTRISTATE;
                aggNode.op = NONE;
                // AggNode *ptr = (AggNode*)malloc(sizeof(AggNode));
                // memcpy(ptr, &aggNode, sizeof(aggNode));
                // tlist.push_back((Node*)ptr);

                //continue;
            }

            AggNode *ptr = (AggNode*)malloc(sizeof(AggNode));
            memcpy(ptr, &aggNode, sizeof(aggNode));
            tlist.push_back((Node*)ptr);

        } // end if isAgg
        else // notAgg, is filed name
        {
            attriNode.nodeType.type = T_ATTRI;
            token = target["ResTarget"]["val"]["ColumnRef"]["fields"][0]["String"]["str"];
            lookupSysCatalog(token, &attriNode, sysCatalog);
            
            AttriNode *ptr = (AttriNode*)malloc(sizeof(AttriNode));
            memcpy(ptr, &attriNode, sizeof(attriNode));
            tlist.push_back((Node*)ptr);
        }
    }

    //cout << tlist.size() << endl;
    //cout << tlist[0]->type << endl;
    return 0;
}

int getRangeTbl(const json& parsed_tree, 
				bool *rTblAffected,
				char tblName[][15])
{
    string token;
    memset(rTblAffected, 0, sizeof(rTblAffected));
    // 遍历 fromClause
    for (const auto& table : parsed_tree[0]["SelectStmt"]["fromClause"]) {
        string table_name = table["RangeVar"]["relname"];
        for(int i = 0; i < 5; i++) 
        {
            if(strcmp(table_name.c_str(), tblName[i]) == 0)
                rTblAffected[i] = true;
                break;
		}
    }

    return 0;
}

int getWhereClause(const json& parsed_tree,
			       vector<DimCondition>& allDimCond, 
			       vector<sysCatalogCell*>& sysCatalog,
			       ddtacontext& context)
{
    AttriNode aNode, lastAttriNode;
    string token, predicate;
    DimCondition dimCondition;
    int offset, intToken;

    lastAttriNode.whichAttri = -1, lastAttriNode.whichRel = -1;
    memset(&dimCondition, 0, sizeof(dimCondition));
    memset(context.FKeyIndicator, false, sizeof(4));

    for (const auto& condition : parsed_tree[0]["SelectStmt"]["whereClause"]["BoolExpr"]["args"])
    {
        if (condition.contains("A_Expr"))
        {
            int expr_kind = condition["A_Expr"]["kind"];
            
            switch (expr_kind)
            {
                case 0:     // normal op, e.g. =,<,>,...
                    if (condition["A_Expr"]["rexpr"].contains("ColumnRef")) // skip through "fact foreign key = dimension key"
                    {
                        token = condition["A_Expr"]["rexpr"]["ColumnRef"]["fields"][0]["String"]["str"];
                        lookupSysCatalog(token,&aNode,sysCatalog);
                        context.FKeyIndicator[aNode.whichRel] = true;
                    }
                    else    // dimension condition
                    {
                        if(allDimCond.size( ) >= 1) lastAttriNode = aNode;
                        token = condition["A_Expr"]["lexpr"]["ColumnRef"]["fields"][0]["String"]["str"];
                        lookupSysCatalog(token, &aNode, sysCatalog);
                        
                        if (aNode.whichRel == lastAttriNode.whichRel)   // 和上一个过滤条件属于同一个关系
                        {
                            dimCondition = allDimCond[allDimCond.size( )-1];
                            allDimCond.pop_back( );

                            // offset = strlen(dimCondition.condtion);
                            // strcat(dimCondition.condtion + offset, to_string(condition["A_Expr"]["lexpr"]["ColumnRef"]["fields"][0]["String"]["str"]).c_str());
                            
                            // offset = strlen(dimCondition.condtion);
                            // strcat(dimCondition.condtion + offset, to_string(condition["A_Expr"]["name"][0]["String"]["str"]).c_str());
                            
                            if (condition["A_Expr"]["rexpr"]["A_Const"]["val"].contains("String"))
                            {
                                token = condition["A_Expr"]["rexpr"]["A_Const"]["val"]["String"]["str"];
                                
                                predicate = "";
                                predicate.append(condition["A_Expr"]["lexpr"]["ColumnRef"]["fields"][0]["String"]["str"]);
                                predicate.append(" ");
                                predicate.append(condition["A_Expr"]["name"][0]["String"]["str"]);
                                predicate.append(" ");
                                predicate.append("\'");
                                predicate.append(token);
                                predicate.append("\'");

                                // offset = strlen(dimCondition.condtion);
                                // strcat(dimCondition.condtion + offset, "\'");

                                // strcat(dimCondition.condtion + (offset + 1), to_string(condition["A_Expr"]["rexpr"]["A_Const"]["val"]["String"]["str"]).c_str());
                                
                                // offset = strlen(dimCondition.condtion);
                                // strcat(dimCondition.condtion + offset, "\'");
                            }
                            else
                            {
                                intToken = condition["A_Expr"]["rexpr"]["A_Const"]["val"]["Integer"]["ival"];

                                predicate = "";
                                predicate.append(condition["A_Expr"]["lexpr"]["ColumnRef"]["fields"][0]["String"]["str"]);
                                predicate.append(" ");
                                predicate.append(condition["A_Expr"]["name"][0]["String"]["str"]);
                                predicate.append(" ");
                                predicate.append(to_string(intToken));
                                
                                // offset = strlen(dimCondition.condtion);
                                // strcat(dimCondition.condtion + offset, to_string(intToken).c_str());
                            }

                            
                            offset = strlen(dimCondition.condtion);
                            strcat(dimCondition.condtion + offset, " and ");
                            strcat(dimCondition.condtion + (offset + 5), predicate.c_str());
                        }
                        else    // 和上一个过滤条件 不属于同一个关系
                        {
                            memset(&dimCondition, 0, sizeof(dimCondition));

                            if (condition["A_Expr"]["rexpr"]["A_Const"]["val"].contains("String"))  // if rexpr is string
                            {
                                token = condition["A_Expr"]["rexpr"]["A_Const"]["val"]["String"]["str"];
                                
                                predicate = "";
                                predicate.append(condition["A_Expr"]["lexpr"]["ColumnRef"]["fields"][0]["String"]["str"]);
                                predicate.append(" ");
                                predicate.append(condition["A_Expr"]["name"][0]["String"]["str"]);
                                predicate.append(" ");
                                predicate.append("\'");
                                predicate.append(token);
                                predicate.append("\'");
                                
                                offset = strlen(dimCondition.condtion);
                                strcat(dimCondition.condtion + offset, predicate.c_str());
                            }
                            else    // if rexpr is integer
                            {
                                intToken = condition["A_Expr"]["rexpr"]["A_Const"]["val"]["Integer"]["ival"];
                                
                                predicate = "";
                                predicate.append(condition["A_Expr"]["lexpr"]["ColumnRef"]["fields"][0]["String"]["str"]);
                                predicate.append(" ");
                                predicate.append(condition["A_Expr"]["name"][0]["String"]["str"]);
                                predicate.append(" ");
                                predicate.append(to_string(intToken));

                                offset = strlen(dimCondition.condtion);
                                strcat(dimCondition.condtion + offset, predicate.c_str());
                            }
                                
                            dimCondition.aNode = aNode;
                        }

                        // add dimCondition to allDimCond
                        allDimCond.push_back(dimCondition);
                        memset(dimCondition.condtion, 0, 100);
                    }
                    
                    break;
                
                case 10:    // between
                    memset(&dimCondition, 0, sizeof(dimCondition));
                    if(allDimCond.size( ) >= 1) lastAttriNode = aNode;
                    token = condition["A_Expr"]["lexpr"]["ColumnRef"]["fields"][0]["String"]["str"];
                    lookupSysCatalog(token, &aNode, sysCatalog);

                    if (condition["A_Expr"]["rexpr"][0].contains("A_Const"))
                    {
                        // string const
                        if (condition["A_Expr"]["rexpr"][0]["A_Const"]["val"].contains("String"))  
                        {
                            predicate = "";     //intialize predicate
                            predicate.append(condition["A_Expr"]["lexpr"]["ColumnRef"]["fields"][0]["String"]["str"]);
                            predicate.append(" between ");
                            predicate.append("\'");
                            predicate.append(condition["A_Expr"]["rexpr"][0]["A_Const"]["val"]["String"]["str"]);
                            predicate.append("\'");
                            predicate.append(" and ");
                            predicate.append("\'");
                            predicate.append(condition["A_Expr"]["rexpr"][1]["A_Const"]["val"]["String"]["str"]);
                            predicate.append("\'");
                        }
                        // integer const
                        else if (condition["A_Expr"]["rexpr"][0]["A_Const"]["val"].contains("Integer"))    
                        {
                            predicate = "";     //intialize predicate
                            predicate.append(condition["A_Expr"]["lexpr"]["ColumnRef"]["fields"][0]["String"]["str"]);
                            predicate.append(" between ");
                            predicate.append(to_string(condition["A_Expr"]["rexpr"][0]["A_Const"]["val"]["Integer"]["ival"]));
                            predicate.append(" and ");
                            predicate.append(to_string(condition["A_Expr"]["rexpr"][1]["A_Const"]["val"]["Integer"]["ival"]));
                        }
                        else
                            cout << "error in getWhereClause -> 293" << endl;
                    }
                    else
                        cout << "error in getWhereClause -> 296" << endl;

                    if (aNode.whichRel == lastAttriNode.whichRel)   // 和上一个过滤条件属于同一个关系
                    {
                        dimCondition = allDimCond[allDimCond.size( )-1];
                        allDimCond.pop_back( );
                        offset = strlen(dimCondition.condtion);
                        strcat(dimCondition.condtion + offset, " and ");
                        strcat(dimCondition.condtion + (offset + 5), predicate.c_str());
                    }
                    else    // 不属于同一个关系
                    {
                        dimCondition.aNode = aNode;
                        offset = strlen(dimCondition.condtion);
                        strcat(dimCondition.condtion + offset, predicate.c_str());
                    }
                    
                    // add dimCondition to allDimCond
                    allDimCond.push_back(dimCondition);
                    memset(dimCondition.condtion, 0, 100);

                    break;    
                
                default:
                    break;
            }
        }   // end if (condition.contains("A_Expr"))
        else if (condition.contains("BoolExpr"))
        {
            memset(&dimCondition, 0, sizeof(dimCondition));
            predicate = "";
            if(allDimCond.size( ) >= 1) lastAttriNode = aNode;
            token = condition["BoolExpr"]["args"][0]["A_Expr"]["lexpr"]["ColumnRef"]["fields"][0]["String"]["str"];
            lookupSysCatalog(token, &aNode, sysCatalog);

            predicate.append("(");  // 1. predicate is "("
            predicate.append(condition["BoolExpr"]["args"][0]["A_Expr"]["lexpr"]["ColumnRef"]["fields"][0]["String"]["str"]);
            predicate.append(" ");
            predicate.append(condition["BoolExpr"]["args"][0]["A_Expr"]["name"][0]["String"]["str"]);
            predicate.append(" ");
            if (condition["BoolExpr"]["args"][0]["A_Expr"]["rexpr"]["A_Const"]["val"].contains("String"))
            {
                predicate.append("\'");
                predicate.append(condition["BoolExpr"]["args"][0]["A_Expr"]["rexpr"]["A_Const"]["val"]["String"]["str"]);
                predicate.append("\'");
            }
            else
                predicate.append(to_string(condition["BoolExpr"]["args"][0]["A_Expr"]["rexpr"]["A_Const"]["val"]["Integer"]["ival"]));


            predicate.append(" or ");   // 2. predicate is "(left or"

            predicate.append(condition["BoolExpr"]["args"][1]["A_Expr"]["lexpr"]["ColumnRef"]["fields"][0]["String"]["str"]);
            predicate.append(" ");
            predicate.append(condition["BoolExpr"]["args"][1]["A_Expr"]["name"][0]["String"]["str"]);
            predicate.append(" ");
            if (condition["BoolExpr"]["args"][0]["A_Expr"]["rexpr"]["A_Const"]["val"].contains("String"))
            {
                predicate.append("\'");
                predicate.append(condition["BoolExpr"]["args"][1]["A_Expr"]["rexpr"]["A_Const"]["val"]["String"]["str"]);
                predicate.append("\'");
            }
            else
                predicate.append(to_string(condition["BoolExpr"]["args"][1]["A_Expr"]["rexpr"]["A_Const"]["val"]["Integer"]["ival"]));


            predicate.append(")");  // 3. predicate is "(left or right)"

            if (aNode.whichRel == lastAttriNode.whichRel)   // 和上一个过滤条件属于同一个关系
            {
                dimCondition = allDimCond[allDimCond.size( )-1];
                allDimCond.pop_back( );
                offset = strlen(dimCondition.condtion);
                strcat(dimCondition.condtion + offset, " and ");
                strcat(dimCondition.condtion + (offset + 5), predicate.c_str());
            }
            else    // 不属于同一个关系
            {
                dimCondition.aNode = aNode;
                offset = strlen(dimCondition.condtion);
                strcat(dimCondition.condtion + offset, predicate.c_str());
            }

            // add dimCondition to allDimCond
            allDimCond.push_back(dimCondition);
            memset(dimCondition.condtion, 0, 100);
        }
        else 
            cout << "error in getWhereClause -> 329" << endl;
    }
    
    return 0;
}

int getGroupClause(const json& parsed_tree,
				   vector<GroupInfo> &groupby,
				   vector<sysCatalogCell*> &sysCatalog
				   )
{
    string token;
    GroupInfo grpInfo;
	AttriNode aNode;

    for (const auto& group : parsed_tree[0]["SelectStmt"]["groupClause"]){
        token = group["ColumnRef"]["fields"][0]["String"]["str"];
        int res = lookupSysCatalog(token, &aNode, sysCatalog);
        if(res >= 0) 
        {
            grpInfo.isTranslatedByMap = res > 0 ? true : false;
            grpInfo.RelInfo           = aNode;
            groupby.push_back(grpInfo);
        }
    }
    return 0;
}

void getOrderClause(const json& parsed_tree,
				    vector<SortInfo>&orderby,
				    vector<GroupInfo> &groupby,
				    vector<sysCatalogCell*> &sysCatalog
				    )
{
    string token;
    AttriNode aNode;
    for (const auto& order : parsed_tree[0]["SelectStmt"]["sortClause"]){
        token = order["SortBy"]["node"]["ColumnRef"]["fields"][0]["String"]["str"];
        int res = lookupSysCatalog(token, &aNode, sysCatalog);
        
        if(res != -1) {
            for(size_t i = 0; i < groupby.size( ); i++) {
                if(aNode.whichRel == groupby[i].RelInfo.whichRel) {
                    SortInfo sInfo;
                    sInfo.sortPosInGroupby = i;
                    sInfo.isAsc            = true;
                    sInfo.type             = res;
                    orderby.push_back(sInfo);
                    break;
                }
            }//end for

			//not in the group by, so we assume the sort attribute must be aggregation value
		}
        else {
            SortInfo sInfo;
            sInfo.sortPosInGroupby = -1, sInfo.isAsc = true;
            orderby.push_back(sInfo);
        }

        if(order["SortBy"]["sortby_dir"] == 1){
            orderby.back( ).isAsc = true;
        }
        if(order["SortBy"]["sortby_dir"] == 2){
            orderby.back( ).isAsc = false;
        }
    }
}

void getAttrsInFactTble(ddtacontext&context, 
						vector<sysCatalogCell*> &sysCatalog, 
						short* attrs)
{
     char *sqlOnFactTbl = context.rewrtSQLStat[4];
     int start = 0;
	 char *pToken;
	 int i = 0;
     AttriNode attriNode;

	 memset(attrs, -1, sizeof(attrs));

	 printf("involved attributes in fact table:\n");

	 while(1) {
		 pToken = getToken(sqlOnFactTbl, &start);

		 if(!pToken) return;

		 if(strcmp(pToken, "FROM") == 0 ||
		    strcmp(pToken, "from") == 0
			){
				free(pToken);
				break;
		 }
         int res = lookupSysCatalog(pToken, &attriNode, sysCatalog);
		 free(pToken);
		 if(res >= 0) {
			 attrs[i++] = attriNode.whichAttri;
			 printf("%d ", attriNode.whichAttri);
		 }
	 } 
	 printf("\n");
	 attrs[i] = -2;

}

void getFilters(ddtacontext &context, short* filters, short* nFilter)
{
    char *pToken;
	char *sqlOnFactTbl = context.rewrtSQLStat[4];

	int start = 0;
    *nFilter  = 0;

	while(1) {
      pToken = getToken(sqlOnFactTbl, &start);

	  if (!pToken) return;

	  if(strcmp(pToken, "where")  == 0 ||
		 strcmp(pToken, "WHERE")  == 0 
		 ) {
			  free(pToken); break;

	  }

	  free(pToken);

	}

	while(1) {
		pToken  = getToken(sqlOnFactTbl, &start);
		if(!pToken) break;
		if(isdigit(*pToken)) filters[(*nFilter)++] = str2digit(pToken);
	}

}

int str2digit(char* strDigit)
{
    int len = strlen(strDigit);

	int base = 1;
	int n = 0, a = 0;

	for(int i = len - 1; i >= 0; i--) {
		a = strDigit[i] - '0';
		n += a * base;
		base *= 10;
	}
    return n;
}

inline int dataKey2Index(int dateKey)
{
		int datekey = dateKey;
		int index = 0;
		int year = datekey / 10000;
		int month = (datekey % 10000) / 100;
		int day = datekey % 100;

		assert((datekey >= OLAP_START_DATE && datekey <= OLAP_END_DATE));

		index = (year - 1992) * 365;

		for (month--; month > 0; month--)
			index += m[month];
		index += day;

		if (datekey >= LEAP_DAY1)
			index++;

		if (datekey >= LEAP_DAY2)
			index++;

		return (index - 1);
}

void loadFactTable(TableType& LOTable, runtimeInfoType& runtimeInfo)
{
   ifstream fin;

   short *map = runtimeInfo.posMap;
   int pos;
   long count=0;

   for(int i = 0; i < 9; i++)
	   LOTable.pLOTable[i] = (int*)malloc(N_TUPLE * sizeof(int));

   char line[512];

   fin.open("/home/town/improveMTV2/data/ssb_001/lineorder.tbl");   //loading lineorder table

    if(!fin)
		cout <<"Can't open data file ";

    else {
       printf("loading data...\n");

	   fin.getline(line, 512);

       int attrValue;
	   char *ch, *cp, *ptr;

	   while( !fin.eof( ) ){
		   ptr = ch = line;

		   for(int i = 0; i < 17; i++){
		        while(*ch != '|'){ //  seperating each field value
			         cp = ch;
					 cp++;
					 ch++;
			     }

		         *cp='\0';

		         switch(i) {    //set each field value to structure fields

					  case 2:
						  attrValue =  (atoi(ptr) - 1);
						  LOTable.pLOTable[0][count] = attrValue;

						  break;

					  case 3:
						  attrValue =  (atoi(ptr) - 1);
						  LOTable.pLOTable[1][count] = attrValue;

						  break;

					  case 4:
						  attrValue = (atoi(ptr) - 1);
						  LOTable.pLOTable[2][count] = attrValue;

						  break;

					  case 5:
						  attrValue = dataKey2Index(atoi(ptr));
						  LOTable.pLOTable[3][count] =  attrValue;

						  break;

					  case 8:  case 9:
					  case 11: case 12: case 13:

						  attrValue = atoi(ptr);

						 // printf("==%d ", attrValue);

						  pos = map[i];
				          LOTable.pLOTable[pos][count] = (attrValue); //column store
				          break;
				 }//switch

				 ch++;

		         ptr = ch;

		   }//for

		   //printf("\n");

	       fin.getline(line,512);
	       count++;   //count present structure tuple array [number]

	       //if(count == 10 ) break;

	    }//while

	}//else

    LOTable.size = count;
    printf(" size = %ld, loading data finished!\n", count);
	fin.close();
}