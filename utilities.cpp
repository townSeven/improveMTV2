#include "utilities.h"
#include "stdafx.h"
#include "parse.h"
#include "sysCatalog.h"
#include "rewriteSQL1.h"
#include "node.h"
#include <string.h>
#include <vector>
#include <ctype.h>
#include <assert.h>
#include <fstream>
#include <iostream>
#include "exec_ddta.h"

using namespace std;

static short numberOfAttri[] = {8, //number of attributes in Customer
                         7,        //number of attributes in Supplier
						 17,       //number of attributes in dwdate
						 9,        //number of attributes in part
						 17};      //                     in lineorder



bool IsOperator(const char* token)
{
    if( (strcmp(token, "+") == 0) ||
		(strcmp(token, "-") == 0) ||
		(strcmp(token, "*") == 0) ||
		(strcmp(token, "/") == 0) 
	  )
		return true;
	return false;

}

Op getOperator(char* pToken)
{
      if (strcmp(pToken, "*") == 0) 
		  return MULTIPLY;
	  else if (strcmp(pToken, "+") == 0)
		  return PLUS;
	  else if (strcmp(pToken, "-") == 0)
		  return MINUS;
	  else 
		  return DIVIDE;
	  

}

int lookupSysCatalog(char* pToken, 
					  AttriNode* node,
					  vector<sysCatalogCell*>& sysCatalog
					  )
{
        for(int i = 0; i < 5; i++){
		    sysCatalogCell *p = sysCatalog[i];
		    for(int j = 0; j < numberOfAttri[i]; j++) {
				if (strcmp(pToken, (p + j)->attriName) == 0) {
					node->whichAttri = j;
					node->whichRel = i;
                    strcpy(node->attriName, (p + j)->attriName);
					return  ((p + j)->type);
				}
		    }
	   }
	   return -1;
}



int fillTargetList (char *zSql, 		
					vector<Node*>& tlist, 
					int* start,
					vector<sysCatalogCell*>& sysCatalog
			        )
{
     char* pToken;
     AttriNode attriNode;
	 AggNode aggNode;
	 State state = ATTRISTATE;
     bool opFlag = false;

	 while(strcmp(pToken = getToken(zSql, start), "from")) {
		
		 if (strcmp(pToken, "select") == 0){free(pToken); continue;}

		 if (strcmp(pToken, "as") == 0){
			    free(pToken);
                pToken = getToken(zSql, start); // as alias,
				free(pToken);
				continue;
		 }

		 if(strcmp(pToken, "sum") == 0) { // now, we know the state is switching to aggregation state
			 state = AGGSTATE;
			 aggNode.aggFlag = SUM;
			 aggNode.nodeType.type = T_AGG;
			 continue;
		 } else if(strcmp (pToken, "(") == 0) {
			 state = 
		         (state == AGGSTATE) ? AGGSTATE : ATTRISTATE;
			 continue;

		 }else if( strcmp(pToken, ")") == 0) {
			 if(state == AGGSTATE && opFlag == true) {
                   state = ATTRISTATE;
				   AggNode *ptr = (AggNode*)malloc(sizeof(AggNode));
				   memcpy(ptr, &aggNode, sizeof(aggNode));
				   tlist.push_back((Node*)ptr);
				   opFlag = false;
				   continue;
			 }else if(state == AGGSTATE && opFlag == false) {
                   state = ATTRISTATE;
				   aggNode.op = NONE;
                   AggNode *ptr = (AggNode*)malloc(sizeof(AggNode));
				   memcpy(ptr, &aggNode, sizeof(aggNode));
				   tlist.push_back((Node*)ptr);
				   continue;
			 }
		 }else if (IsOperator(pToken)) {
			 if (state == AGGSTATE) {
				 aggNode.op = getOperator(pToken);
				 opFlag = true;
			 }

		 }else { // we have a token, we look up the system catalog

			 if (state == ATTRISTATE){

				 attriNode.nodeType.type = T_ATTRI;
				 lookupSysCatalog(pToken, &attriNode, sysCatalog);
				 AttriNode *ptr = (AttriNode*)malloc(sizeof(AttriNode));
				 memcpy(ptr, &attriNode, sizeof(attriNode));
				 tlist.push_back((Node*)ptr);

			 } else if (state == AGGSTATE && opFlag == true){

				 lookupSysCatalog(pToken, &attriNode, sysCatalog);
				 aggNode.rNode = attriNode;

			 } else if (state == AGGSTATE && opFlag == false) {

				 lookupSysCatalog(pToken, &attriNode, sysCatalog);
				 aggNode.lNode = attriNode;

			 }
		 }//end else
         free(pToken);
	 }
	 //printf("%d\n", tlist.size( ));
	 return 0;

}

int fillRangeTbl(char *zSql, 
				 bool *rTblAffected, 
			     int* start, 
				 char tblName[][15])
{
	 char* pToken;
	 memset(rTblAffected, 0, sizeof(rTblAffected));
     while(strcmp(pToken = getToken(zSql, start), "where")) {
		 for(int i = 0; i < 5; i++) {
               if(strncmp(pToken, tblName[i], 15) == 0)
				   rTblAffected[i] = true;
		 }
		 free(pToken);
	 }
	 free(pToken);
     return 0;
}

inline void 
dealWithQuotation(bool& inQuotation, char* pToken, char *condition)
{
     int from = strlen(condition);

     if(pToken[0] == '\'' && inQuotation) {

           inQuotation = false;       //running out of ' '
		   condition[from] = '\'';

	 }else if(pToken[0] == '\''){     //first meet ' of 'content'

		   inQuotation = true;
		   condition[from] = ' ';      //and^'condition' = 
		   condition[from + 1] = '\''; //  

	 }else {
		   if(inQuotation && condition[from - 1] == '\''){

		       strcat(condition + from, pToken);  // '^content'

		   }else if(inQuotation && condition[from - 1] != '\''){

			   condition[from] = ' ';
			   strcat(condition + from + 1, pToken);

		   }else {
               condition[from] = ' ';
		       strcat(condition + from + 1, pToken);
		   }
	 }

}

bool fillWhere( char *zSql, 
			    int* start, 
			    vector<DimCondition>& allDimCond, 
			    vector<sysCatalogCell*>&sysCatalog,
			    ddtacontext& context
			  )
{
   AttriNode aNode, tmpAttriANode,lastAttriNode, *pANode;
   DimCondition dimCondition;
   whereState state = Begin;
   char *pToken;
   int oldStart, from;
   bool inQuotation = false, hasAgg = false;

   lastAttriNode.whichAttri = -1, lastAttriNode.whichRel = -1;
   memset(&dimCondition, 0, sizeof(dimCondition));
   memset(context.FKeyIndicator, false, sizeof(4));

   while(1) { //skip through "fact foreign key = dimension key"
	    oldStart = *start;
	    pToken = getToken(zSql, start); // left attribute
		free(pToken);
        pToken = getToken(zSql, start); //  =
		free(pToken);
		pToken = getToken(zSql, start); // right attribute
		int res2 = lookupSysCatalog(pToken,&aNode,sysCatalog);//look up system catalog
		free(pToken);
		if(res2 == -1) break;
		context.FKeyIndicator[aNode.whichRel] = true;
		pToken = getToken(zSql, start); // and
		free(pToken);
   }
   *start = oldStart;

   while(pToken = getToken(zSql, start)) {
	   if(strcmp(pToken, "group") == 0){hasAgg = true; break;}

	   if(strcmp(pToken, "(") == 0) {
		   switch(state) {
				case Begin:
					state = LBracket;
					from = strlen(dimCondition.condtion);
					strcat(dimCondition.condtion + from, "(");
					break;
				default:
					printf("error in state--(\n");
					break;
		   }
	   }else if (strcmp(pToken, "between") == 0) {
           switch(state) {
				case OneDim:
					state = BA;
					from = strlen(dimCondition.condtion);
					strcat(dimCondition.condtion + from, " between");
					break;
				default:
					printf("error in state--BA\n");
					break;
		   }
	   }else if(strcmp(pToken, ")") == 0){
		   switch(state) {
				case OneDim:
					state = RBracket;
					from = strlen(dimCondition.condtion);
					strcat(dimCondition.condtion + from - 1, ")");
					allDimCond.push_back(dimCondition);
					memset(&dimCondition, 0, sizeof(dimCondition));
					break;
				default:
				    printf("error in state--)\n");
					break;
		   }
	   }else if (strcmp(pToken, "and") == 0) {
		   switch(state) {
				case RBracket:
					state = Begin;
                    break;
				case B1A:
					state = BAnd;
					from = strlen(dimCondition.condtion);
					strcat(dimCondition.condtion + from, " and");
					break;
				case OneDim: case BA1:
					state = Begin;
                    allDimCond.push_back(dimCondition);
                    memset(dimCondition.condtion, 0, 100);
					break;
		   }
	   }else {
		   if(allDimCond.size( ) >= 1) lastAttriNode = aNode;
           int res = lookupSysCatalog(pToken, &aNode, sysCatalog);

		   switch(state) {
			   case Begin: case LBracket:
				  
				   if(res == -1) printf("error in state begin\n");

				   if(aNode.whichRel == lastAttriNode.whichRel) { 

					   dimCondition = allDimCond[allDimCond.size( )-1];
					   allDimCond.pop_back( );
					   from = strlen(dimCondition.condtion);
				       strcat(dimCondition.condtion + from, " and ");
					   strcat(dimCondition.condtion + (from + 5), pToken);

				   }else {

					   if(state == Begin) memset(&dimCondition, 0, sizeof(dimCondition));
					   from = strlen(dimCondition.condtion);
				       strcat(dimCondition.condtion + from, pToken);
					   dimCondition.aNode = aNode;

				   }
				   state = OneDim;
				   break;

			   case BA:

				    state = B1A;
                    dealWithQuotation(inQuotation, pToken, dimCondition.condtion);
					break;

			   case BAnd:

				    state = BA1;
                    dealWithQuotation(inQuotation, pToken, dimCondition.condtion);
					break;

			   default:

				    dealWithQuotation(inQuotation, pToken, dimCondition.condtion);
				
		   }//switch

	   }//if

	   free(pToken);

   }//while

   free(pToken);

   if(dimCondition.condtion[0]) allDimCond.push_back(dimCondition);
   return hasAgg;
}


bool fillGroupby(char *zSql,	
				 int* start,
				 vector<GroupInfo> &groupby,
				 vector<sysCatalogCell*> &sysCatalog
				 )
{
     char *pToken;
	 bool hasOrderby = false;
	 GroupInfo grpInfo;
	 AttriNode aNode;

	 while(pToken =  getToken(zSql, start)) {
		 if(strcmp(pToken, "by")    == 0)  {free(pToken); continue;} // skip through group by key word.
		
		 if(strcmp(pToken, "order") == 0)  {free(pToken); hasOrderby = true; break;}
         
		 int res = lookupSysCatalog(pToken, &aNode, sysCatalog);
		 
		 if(res >= 0) {
			  grpInfo.isTranslatedByMap = res > 0 ? true : false;
			  grpInfo.RelInfo           = aNode;
              groupby.push_back(grpInfo);
		  }
          free(pToken);   
	 }
	
     return hasOrderby;
}


void fillOrderby(char* zSql,	
				 int* start,
				 vector<SortInfo>&orderby,
				 vector<GroupInfo> &groupby,
				 vector<sysCatalogCell*> &sysCatalog
				 )
{
     char *pToken;
     AttriNode aNode;
     size_t i;
	 
	 while(pToken = getToken(zSql, start)) {
		  if(strcmp(pToken, "by")   == 0) {free(pToken); continue;} // skip through order by...

		  if(strcmp(pToken, "asc")  == 0) { 
			   free(pToken);
			   orderby.back( ).isAsc = true; 
			   continue;
		  }

		  if(strcmp(pToken, "desc") == 0) {
			   free(pToken);
			   orderby.back( ).isAsc = false;
			   continue;
		  }
		  
          int res = lookupSysCatalog(pToken, &aNode, sysCatalog);
		 
		  if(res != -1 ) {
			  for(i = 0; i < groupby.size( ); i++){
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
			 
		  }else if(isalpha(pToken[0])) {
				SortInfo sInfo;
				sInfo.sortPosInGroupby = -1, sInfo.isAsc = true;
			    orderby.push_back(sInfo);
          }
		  free(pToken);
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

char *int64toa(int64 n)
{
   char* p;

   int a, i;

   p = (char*)malloc(30 * sizeof(char));

   p[29] = 0;

   i = 28;

   while(n) {

      a = n % 10;

      printf("%d ", a);

      p[i--] = a  + '0';

      n = n / 10;

   }
   return  p + i + 1;
}

