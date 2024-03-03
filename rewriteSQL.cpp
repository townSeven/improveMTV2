#include "stdafx.h" 
#include "parse.h"
#include "node.h"
#include "rewriteSQL1.h"
#include "sysCatalog.h"
#include <boost/bimap.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>
#include <iostream>
#include <stdio.h>
#include <vector>

using namespace std;
using namespace boost;


inline char* getRelName(int RelNO)
{
	switch(RelNO) {		//@town:[add] "(char*)"
		 case 0: return (char*)"customer";   break;
         case 1: return (char*)"supplier";   break;
		 case 2: return (char*)"dwdate";     break;
	     case 3: return (char*)"part";       break;
         case 4: return (char*)"lineorder";  break;
 
	}
    return (char*)"NULL";
}

inline void 
constructRewrtSQLOnFactTbl(ddtacontext &context,
                           const char* prediction,  
						   const char* attriName)
{
    char* dst = context.rewrtSQLStat[4], *pTemp;
	memset(dst, 0, sizeof(dst));
	strcat(dst, "SELECT ");
	for(int i = 0; i < 4; i++) {
		
		if(context.FKeyIndicator[i]){
			pTemp = const_cast<char*>((i == 0) ? "lo_custkey, " :
			              (
						     (i == 1) ? "lo_suppkey, " :
							 ( 
							    (i==2) ? "lo_orderdate, " : "lo_partkey, "
						     )
		       		      ));

		    strcat(dst, pTemp);	  
		}

	}
	strcat(dst, attriName);
	strcat(dst, "\n FROM lineorder");

	if(prediction != NULL){
		strcat(dst, "\nWHERE ");
	    strcat(dst, prediction);
	}
		                 
	
}

void constructRewrtSQL( char* dst, 
						const char* prediction,  
						const char* attriName,
						int type, int whichRel
					   ) 
{
	char* relName;

	memset(dst, 0, sizeof(dst));

	if(attriName && prediction) {
       memset(dst, 0, sizeof(dst));
	   strcpy(dst, "SELECT\n");
	   strcat(dst, "CASE WHEN "); 
	   strcat(dst,  prediction);
	   strcat(dst, "\nTHEN ");
	   strcat(dst,  attriName);
	
	   strcat(dst, "\nELSE FALSE END");
	
	}else if(attriName) {
       memset(dst, 0, sizeof(dst));
       strcpy(dst, "SELECT\n");

	   //alway true, because no prediction acts on this attribute
	   strcat(dst, "CASE WHEN 1 = 1"); 
	   strcat(dst, "\nTHEN ");
	   strcat(dst,  attriName);
	 
	   strcat(dst, "\nELSE FALSE END");

	}else {
       memset(dst, 0, sizeof(dst));
	   strcpy(dst, "SELECT\n");
	   strcat(dst, "CASE WHEN "); 
	   strcat(dst,  prediction);
	   strcat(dst, "\nTHEN true");
	   strcat(dst, "\nELSE false END");
	 
	}
	relName = getRelName(whichRel);
	strcat(dst, "\nfrom ");
	strcat(dst,  relName);
    int len = strlen(dst);
	dst[len] = 0;
}

void rewrtSql (vector<Node*> &targetList,
			   vector<DimCondition> &allDimCondition,
			   ddtacontext& context,
			   vector<sysCatalogCell*> &catalog)
{
	
    size_t  i, j;
	bool used[5];
	short type;

	memset(used, false, sizeof(used));
	memset(context.rewrtSQLStat[0], 0, sizeof(context.rewrtSQLStat[0]));
	memset(context.rewrtSQLStat[1], 0, sizeof(context.rewrtSQLStat[1]));
	memset(context.rewrtSQLStat[2], 0, sizeof(context.rewrtSQLStat[2]));
	memset(context.rewrtSQLStat[3], 0, sizeof(context.rewrtSQLStat[3]));
	memset(context.rewrtSQLStat[4], 0, sizeof(context.rewrtSQLStat[4]));

	for(i = 0; i < targetList.size( ); i++) {
          Node* pNode = targetList[i];
		  if(pNode->type == T_ATTRI) {

               int whichRel   = ((AttriNode*)pNode)->whichRel;
			   int whichAttri = ((AttriNode*)pNode)->whichAttri;
			   used[whichRel] = true;

			   for(j = 0; j < allDimCondition.size( ); j++){
                    DimCondition dimCondition = allDimCondition[j];
					if(dimCondition.aNode.whichRel == whichRel) {
					// now we find an target in select part, and there is(are) some 
				    // prediction(s) on this relation
					   sysCatalogCell *p = catalog[whichRel];

			           constructRewrtSQL(context.rewrtSQLStat[whichRel], 
						      dimCondition.condtion, 
						      (p + whichAttri)->attriName,  
							  (p + whichAttri)->type, whichRel);
                       (p + whichAttri)->type == 0 ?
						   context.isTranslated[whichRel] = false :
					       context.isTranslated[whichRel] = true;
					  //printf("\nthe sql after rewriten is:\n%s\n", context.rewrtSQLStat[whichRel]);
					   break;
					}
			   }//for
			   if(j >= allDimCondition.size( )) {//no prediction acts on this attribute in target list
                       sysCatalogCell *p = catalog[whichRel];
			            constructRewrtSQL(context.rewrtSQLStat[whichRel],
							     NULL, 
						         (p + whichAttri)->attriName,  
								 (p + whichAttri)->type, whichRel);
					    (p + whichAttri)->type == 0 ?
						   context.isTranslated[whichRel] = false :
					       context.isTranslated[whichRel] = true;
					   //printf("\nthe sql after rewriten is\n%s\n", context.rewrtSQLStat[whichRel]);
			   }//if
		  }else {
			   char aggTarget[50];
			   memset(aggTarget, 0, 50);
               sysCatalogCell *p = catalog[4];
               bool hasOp = true;
			 
               strcat(aggTarget, 
					   (p +((AggNode*)pNode)->lNode.whichAttri)->attriName);
			  
			    
			   switch(((AggNode*)pNode)->op) {
					   case PLUS      : strcat(aggTarget, "+"); break;
					   case MINUS     : strcat(aggTarget, "-"); break;
					   case DIVIDE    : strcat(aggTarget, "/"); break;
					   case MULTIPLY  : strcat(aggTarget, "*"); break;
					   case NONE      : hasOp = false; break;

			    }

                if (hasOp)
					strcat(aggTarget, 
					   (p +((AggNode*)pNode)->rNode.whichAttri)->attriName);

               for(j = 0; j < allDimCondition.size( ); j++){
                    DimCondition dimCondition = allDimCondition[j];
					if(dimCondition.aNode.whichRel == 4) {
					// now we find an target in select part, and there is(are) some 
				    // prediction(s) on fact table
			           constructRewrtSQLOnFactTbl(context, 
						                          dimCondition.condtion, 
						                          aggTarget);
					   context.isTranslated[4] = false;
					 // printf("the sql after rewriten is:\n%s\n", context.rewrtSQLStat[4]);

					   break;
					}
			   }
			   if(j >= allDimCondition.size( )) {//no prediction acts on this attribute in target list
			             constructRewrtSQLOnFactTbl(context, 
							                        NULL, 
													aggTarget);
					     context.isTranslated[4] = false;
						 printf("the sql after rewriten is\n%s\n", context.rewrtSQLStat[4]);
			   }//if
		  }//end if pNode->type == T_ATTRI
	}//for

    // just prediction acted on dimension table
	// not appear in the targetlist.So, we just
	// produce the "true" or "false" value
	for(j = 0; j < allDimCondition.size( ); j++){

        DimCondition dimCondition = allDimCondition[j];

		int whichRel = dimCondition.aNode.whichRel;

		if(whichRel != 4 && !used[whichRel]) {

	       used[whichRel] = true;

		   sysCatalogCell *p = catalog[whichRel];

           constructRewrtSQL(context.rewrtSQLStat[whichRel], dimCondition.condtion, 
						                  NULL, 0, whichRel);
        
		   context.isTranslated[whichRel] = false;
		  // printf("the sql after rewriten is\n%s\n", context.rewrtSQLStat[whichRel]);
		}
	}
}
