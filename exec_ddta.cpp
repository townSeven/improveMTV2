#include  "exec_ddta.h"
#include  "resultSet.h"
#include  "rewriteSQL1.h"
#include  "stdafx.h"
#include  "parse.h"
#include  "connection.h"	//@town Feb.2nd: [add]
#include  <boost/smart_ptr/shared_ptr.hpp>
#include  <boost/typeof/typeof.hpp>
#include  <boost/foreach.hpp>
#include  "boost/bimap.hpp"
#include  <iostream>
#include  <algorithm>
#include  <vector>
#include  <iostream>
#include  <stdlib.h>
#include  <ctype.h>
#include  <pthread.h>
#include  <unistd.h>
#include  "timer.h"
#include  <ctype.h>
#include  <boost/dynamic_bitset.hpp>
#include  <x86gprintrin.h>		//@town Feb.2nd: [add]
//#include  <windows.h>

extern "C" {
#include "socketConn.h"
}


extern "C" {
#include "connection.h"
}

extern int query;

using namespace std;
using namespace boost;


typedef struct thread_args{

	int fact_start;
	int fact_end;
	TableType  *pFactTable;
	SelectType *pSelect;
	ddtacontext * contextptr;
	runtimeInfoType *pRTInfoPtr;
	result  res;

    thread_args( ) {

		fact_start = 0;
		fact_end = 0;
		pSelect = NULL;
		contextptr = NULL;
		pRTInfoPtr = NULL;
	}
}thread_args;

#if defined(__i386__)

static __inline__ unsigned long long rdtsc(void)
{
  unsigned long long int x;
     __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x)); //.byte 0x0f,0x31等价于rdtsc，是另一种原始取机器码的方式
     return x;                                                               //改成__asm__ volatile ("rdtsc" : "=A" (x)); 效果一样
}                                                                                //关于操作码可以参考文献[2]
#elif defined(__x86_64__)
static __inline__ unsigned long long rdtsc(void)
{
  unsigned hi, lo;
  __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
  return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}

#endif


/*
void
printResult(ddtacontext &context, result &res, SelectType &selectVar, char* res2Client)
{


    char tmpStr[30];

    memset(res2Client, 0, sizeof(res2Client));

    if(selectVar.targetList[0]->type == T_AGG){


		  for(size_t i = 0; i < res.retval.size( ); i++) {

				//printf("|%10lld|", res.retval[i].aggval);

                sprintf(tmpStr, "%lld", res.retval[i].aggval);

                strcat(res2Client, "|");

				strcat(res2Client, tmpStr);

				for(size_t j = 0;
					j < res.retval[i].groupData.size( );
					j ++) {
						  int whichRel = selectVar.groupby[j].RelInfo.whichRel;
						  if(context.isTranslated[whichRel]) {
							 bimap<string, int> &map =
								 whichRel == 0 ? context.mapForCustomer :
								 (
									  whichRel == 1 ? context.mapForSupplier :
									  (
										  whichRel == 2 ?
											  context.mapForDwdate : context.mapForPart
									  )
								 );

							  BOOST_AUTO(iter, map.right.find(res.retval[i].groupData[j]));

							  if(iter != map.right.end ( )) {
								 //printf("%20s|", iter->second.c_str( ));
								 strcat(res2Client, "|");
								 strcat(res2Client, iter->second.c_str( ));
							  }

						  }else {//if(context.isTranslated[whichRel])

							  sprintf(tmpStr, "%d", res.retval[i].groupData[j]);

							  strcat(res2Client, "|");

							  strcat(res2Client, tmpStr);
						  }
				}//for

			    strcat(res2Client, "\n");

		  }//for

		  printf("%s\n", res2Client);

   }else {
         for(size_t i = 0; i < res.retval.size( ); i++) {

				for(size_t j = 0;
					j < res.retval[i].groupData.size( );
					j ++) {
						  int whichRel = selectVar.groupby[j].RelInfo.whichRel;
						  if(context.isTranslated[whichRel]) {
							 bimap<string, int> &map =
								 whichRel == 0 ? context.mapForCustomer :
								 (
									  whichRel == 1 ? context.mapForSupplier :
									  (
										  whichRel == 2 ?
											  context.mapForDwdate : context.mapForPart
									  )
								 );

							  BOOST_AUTO(iter, map.right.find(res.retval[i].groupData[j]));

							  if(iter != map.right.end ( )) {

								 //printf("|%15s", iter->second.c_str( ));

								 strcat(res2Client, "|");

								 strcat(res2Client, iter->second.c_str( ));

							  }
						  }else {

							    //printf("|%10d", res.retval[i].groupData[j]);

							    sprintf(tmpStr, "%d", res.retval[i].groupData[j]);

							    strcat (res2Client, "|");

							    strcat (res2Client, tmpStr);

						  }
				}//for
				//printf("|%10lld|\n", res.retval[i].aggval);

				sprintf(tmpStr, "%lld", res.retval[i].aggval);

				strcat(res2Client, "|");

				strcat(res2Client, tmpStr);

				strcat(res2Client, "\n");

         }//for
         printf("%s\n", res2Client);

   }//if

    char tmp[20];

    sprintf(tmp, "%d", res.retval.size( ));

    strcat(tmp, "\n");

    printf("%d\n",res.retval.size( ));

    strcat(res2Client, tmp);

}*/

//@town：copy from MemDBproj4

inline void printResult(ddtacontext& context, result& res, SelectType& selectVar)
{

	for (size_t i = 0; i <= res.retval[0].groupData.size(); i++) {
		printf("==============");
	}
	printf("\n");

	if (selectVar.targetList[0]->type == T_AGG) {
		for (size_t i = 0; i < res.retval.size(); i++) {

			printf("|%10lld|", res.retval[i].aggval);
			for (size_t j = 0;
				j < res.retval[i].groupData.size();
				j++) {
				int whichRel = selectVar.groupby[j].RelInfo.whichRel;
				if (context.isTranslated[whichRel]) {
					bimap<string, int>& map =
						whichRel == 0 ? context.mapForCustomer :
						(
							whichRel == 1 ? context.mapForSupplier :
							(
								whichRel == 2 ?
								context.mapForDwdate : context.mapForPart
								)
							);

					BOOST_AUTO(iter, map.right.find(res.retval[i].groupData[j]));

					if (iter != map.right.end())
						printf("%20s|", iter->second.c_str());
				}
				else
					printf("%10d|", res.retval[i].groupData[j]);
			}//for
			printf("\n");
		}//for
	}
	else {
		for (size_t i = 0; i < res.retval.size(); i++) {

			for (size_t j = 0;
				j < res.retval[i].groupData.size();
				j++) {
				int whichRel = selectVar.groupby[j].RelInfo.whichRel;
				if (context.isTranslated[whichRel]) {
					bimap<string, int>& map =
						whichRel == 0 ? context.mapForCustomer :
						(
							whichRel == 1 ? context.mapForSupplier :
							(
								whichRel == 2 ?
								context.mapForDwdate : context.mapForPart
								)
							);

					BOOST_AUTO(iter, map.right.find(res.retval[i].groupData[j]));

					if (iter != map.right.end())
						printf("|%15s", iter->second.c_str());
				}
				else
					printf("|%10d", res.retval[i].groupData[j]);
			}//for
			printf("|%10lld|\n", res.retval[i].aggval);
		}//for

	}//if
	for (size_t i = 0; i <= res.retval[0].groupData.size(); i++) {
		printf("==============");
	}
	printf("\n");
	printf("%ld tuple(s) ", res.retval.size());
}


inline void LoDimKeyToIndex(ddtacontext &context, int* dimIndex, int* LOFKeyIndex)
{


	dimIndex[0] = -1;
	dimIndex[1] = -1;
	dimIndex[2] = -1;
	dimIndex[3] = -1;


	int pos = 0;

	if(context.FKeyIndicator[0]) {
		  //if the lo_custkey is selected, it must be in
		  //pos = 0, else we skip it just by making the pos
		  //keep the same value
		  dimIndex[0] = LOFKeyIndex[pos] - 1;
          pos++;
	}

	if(context.FKeyIndicator[1]){
		  dimIndex[1] = LOFKeyIndex[pos] - 1;
		  pos++;
	}

	if(context.FKeyIndicator[2]){
		  dimIndex[2]  = LOFKeyIndex[pos];
		  pos++;
	}

    if(context.FKeyIndicator[3]){
    	  dimIndex[3]   = LOFKeyIndex[pos] - 1;
		  pos++;
	}

	if(context.FKeyIndicator[2]) {
		long datekey = dimIndex[2];
		long index = 0;

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

		dimIndex[2] = index - 1;
	}

}


//inline void getDimData(ddtacontext       &context,
//					   vector<GroupInfo> &grpInfo,
//					   resultTuple2&     restup,
//					   int*              dimIndex)
//{
//	int index;
//	vector<int> &c_pred = context.predCustomer;
//    vector<int> &s_pred = context.predSupplier;
//    vector<int> &p_pred = context.predPart;
//    vector<int> &d_pred = context.predDwdate;
//
//	for(size_t i = 0; i < grpInfo.size( ); i++) {
//		switch(grpInfo[i].RelInfo.whichRel) {
//
//			case 0:
//				index = dimIndex[0];
//				//@town
//				/*
//				int i1 = context.predCustomer[index];
//				BOOST_AUTO(iter, context.mapForCustomer.right.find(i1));
//				if (iter != context.mapForCustomer.right.end()) {
//					restup.groupData2.c_data.assign(iter->)
//				}*/
//
//				break;
//
//			case 1: {
//				index = dimIndex[1];
//				/*
//				int i1 = context.predSupplier[index];      //c, s, d, p
//			    BOOST_AUTO(iter, context.mapForPart.right.find(i3));
//				if(iter != context.mapForPart.right.end( )) {
//						restup.groupData2.p_brand.assign(iter->second.c_str( ));
//								       //cout << iter->second << " ";
//				}else
//						printf("wrong\n");*/
//
//				break;
//		    }
//
//		    case 2: {
//				index = dimIndex[2];
//				restup.groupData2.d_year = context.predDwdate[index];
//				break;
//			}
//
//			case 3:
//				index = dimIndex[3];
//				int i3 = context.predPart[index];      //c, s, d, p
//			    BOOST_AUTO(iter, context.mapForPart.right.find(i3));
//				if(iter != context.mapForPart.right.end( )) {
//					  restup.groupData2.p_brand.assign(iter->second.c_str( ));
//				      //cout << iter->second << " ";
//				}else
//					  printf("wrong\n");
//
//		}//switch
//
//	}//end for
//}

inline void getDimData(ddtacontext& context,
	vector<GroupInfo>& grpInfo,
	vector<int>& data,
	int* dimIndex)
{
	int index;
	vector<int>& c_pred = context.predCustomer;
	vector<int>& s_pred = context.predSupplier;
	vector<int>& p_pred = context.predPart;
	vector<int>& d_pred = context.predDwdate;

	for (size_t i = 0; i < grpInfo.size(); i++) {
		switch (grpInfo[i].RelInfo.whichRel) {

		case 0:
			index = dimIndex[0];
			data.push_back(c_pred[index]);
			break;

		case 1: {
			index = dimIndex[1];
			data.push_back(s_pred[index]);
			break;
		}

		case 2: {
			index = dimIndex[2];
			data.push_back(d_pred[index]);
			break;
		}

		case 3:
			index = dimIndex[3];
			data.push_back(p_pred[index]);
			break;

		}//switch

	}//end for
}

inline void
execGroup(ddtacontext &context, result &res, SelectType &selectVar, int* dimIndex, long aggValue){

	vector<GroupInfo> &groupby = selectVar.groupby;
    vector<int> dimData;

	resultTuple2 restup;

    //load dimension data for grouping
	getDimData(context, groupby, restup.groupData, dimIndex);

	/*int i1 = restup.groupData[0];
	int i2 = restup.groupData[1];
	printf("%d ", i1);
		

	BOOST_AUTO(iter, context.mapForPart.right.find(i2));
	if(iter != context.mapForPart.right.end( ))
				cout << iter->second << " ";
	printf("%ld\n", aggValue);*/

	/*int i1 = restup.groupData2.d_year;
	string i2 = restup.groupData2.p_brand;
	if (i1 == 1992 && i2 == "MFGR#2225") {
		cout << i1 << " " << i2 << " " << aggValue << endl;
	}*/
	
	boost::unordered_map<resultTuple2, int64> & group      = res.hashgroup;
	boost::unordered_map<resultTuple2, int64>::iterator it = group.find(restup);

	if(it == group.end( )){

		group[restup] = aggValue;
		//cout << group[restup] << endl;
	}
	else{
		
		it->second += aggValue;
	}
}

inline bool predictionJudge(ddtacontext &context, int* dimIndex, bool* flags)
{

	memset(flags, true, sizeof(flags));

	int c_size  = context.predCustomer.size( ),
		s_size  = context.predSupplier.size( ),
		p_size  = context.predPart.size( ),
		d_size  = context.predDwdate.size( );

	if((dimIndex[0] >= c_size && c_size != 0) ||
	   (dimIndex[1] >= s_size && s_size != 0) ||
	   (dimIndex[2] >= d_size && d_size != 0) ||
	   (dimIndex[3] >= p_size && p_size != 0)
	   ) return false;



	 if(dimIndex[0] != -1 && c_size != 0) {
		 flags[0] = context.predCustomer[dimIndex[0]];
		 if(!flags[0]) return 0;
	 }

	 if(dimIndex[1] != -1 && s_size != 0){
         flags[1] = context.predSupplier[dimIndex[1]];
         if(!flags[1]) return 0;
	 }

    if(dimIndex[2]  != -1 && d_size != 0){
		flags[2] = context.predDwdate[dimIndex[2]];
		if(!flags[2]) return 0;
    }

    if(dimIndex[3] != -1 && p_size != 0){
		flags[3] = context.predPart[dimIndex[3]];
		if(!flags[3]) return 0;
    }

	return (flags[0] && flags[1] && flags[2] && flags[3]);

}



inline bool meetWhereCond(ddtacontext &context, int* dimIndex)
{

	bool flags[4];

	memset(flags, true, sizeof(flags));

	int c_size  = context.predCustomer.size( ),
		s_size  = context.predSupplier.size( ),
		p_size  = context.predPart.size( ),
		d_size  = context.predDwdate.size( );

	if((dimIndex[0] >= c_size && c_size != 0) ||
	   (dimIndex[1] >= p_size && p_size != 0) ||
	   (dimIndex[2] >= s_size && s_size != 0) ||
	   (dimIndex[3] >= d_size && d_size != 0)
	   ) return false;



	 if(dimIndex[0] != -1 && c_size != 0) {
		 flags[0] = context.predCustomer[dimIndex[0]];
		 if(!flags[0]) return 0;
	 }

	 if(dimIndex[1] != -1 && p_size != 0){
         flags[1] = context.predPart[dimIndex[1]];
         if(!flags[1]) return 0;
	 }

    if(dimIndex[2]  != -1 && s_size != 0){
		flags[2] = context.predSupplier[dimIndex[2]];
		if(!flags[2]) return 0;
    }

    if(dimIndex[3] != -1 && d_size != 0){
		flags[3] = context.predDwdate[dimIndex[3]];
		if(!flags[3]) return 0;
    }

	return (flags[0] && flags[1] && flags[2] && flags[3]);

}

inline bool
isAttrsInWhere(SelectType&selectVar, int posTransformed)
{
	   vector<DimCondition> &allDimCondition = selectVar.whereCondition;
	   bool flags[5], res;

	   memset(flags, false, sizeof(flags));

	   for (size_t i = 0; i < allDimCondition.size(); i++) {

			int tblPos = allDimCondition[i].aNode.whichRel;
            flags[tblPos] = true;
	  }

	 switch (posTransformed) {
	   case 0:
		//printf("customer: ");

		  res = flags[0];
		  break;
	   case 1:

		  //printf("part: ");
		  res = flags[3];
		  break;
	   case 2:
		  //printf("supplier: ");

		  res = flags[1];
		  break;

	   case 3:
		  //printf("date: ");
		  res = flags[2];
		  break;

	   case 4: case 6:

		   res = flags[4];
		   break;


	  }
	  return res;
}

/*
void DimFilter(runtimeInfoType& runtimeInfo, TableType& LOTable ,
		       ddtacontext &context, SelectType& selectVar, int start, int end)
{


	 int  posOriginal,  posTransformed;


	 bool inWhere, isFilter[4], flag;

	 int  filterIndex[4];

	 int  dimIndex;

	 memset(isFilter, false, sizeof(isFilter));


	 dynamic_bitset< > &ftFilter = runtimeInfo.FTFilter;

	 for(int i = 0; runtimeInfo.factTblAttrs[i] != -2; i++) { //get foreign key

	        posOriginal    = runtimeInfo.factTblAttrs[i];

	        //now , we can get index of the value in the tuple
	        posTransformed = runtimeInfo.posMap[posOriginal];

            inWhere =  isAttrsInWhere(selectVar, posTransformed);

            if(!inWhere) continue;

            if(posTransformed <= 3) {
                   vector<int> &pred = (posTransformed == 0 ? context.predCustomer :
            			                 (posTransformed == 1 ? context.predPart :
            			                    (posTransformed == 2 ? context.predSupplier :
            			            		   context.predDwdate
            			                    )
            	                         )
            	                       );

                  for(int j = start; j <= end; j++) {
            		   if(ftFilter.test(j)) {

                            dimIndex = LOTable.pLOTable[posTransformed][j];

                            if(dimIndex >= pred.size( ) || !pred[dimIndex])
                        	       ftFilter.reset(j);
            		   }
                  }

           }else {

        	   switch(runtimeInfo.nFilter) {
        	    case 3:
        	    	 for(int ntup = start; ntup <= end; ntup++) {

        	    		  if(!ftFilter.test(ntup))  continue;

        	              if(!(runtimeInfo.filter[0] <= LOTable.pLOTable[6][ntup] &&
        	                            LOTable.pLOTable[6][ntup] <= runtimeInfo.filter[1]) &&
        	                            (LOTable.pLOTable[4][ntup] < runtimeInfo.filter[2])

        	                           )
                                ftFilter.reset(ntup);


        	         }

        	         break;

        	    case 4:
                     for(int ntup = start; ntup <= end; ntup++) {

                    	    if(!ftFilter.test(ntup)) continue;

        	         	    if( !( ( runtimeInfo.filter[0] <= LOTable.pLOTable[6][ntup] &&
        	         	    			   LOTable.pLOTable[6][ntup] <= runtimeInfo.filter[1]
        	         	    			 )
        	         	    	    &&   (runtimeInfo.filter[2] <= LOTable.pLOTable[4][ntup] &&
        	         	    	    		LOTable.pLOTable[4][ntup] <= runtimeInfo.filter[3]
        	         	    	         )
        	         	    	       )
        	         	    	    )
        	         	    		ftFilter.reset(ntup);
                     }

                     break;
        	  }//switch
         }
	 }
}



inline bool
getNeededData(runtimeInfoType& runtimeInfo, SelectType& selectVar,
		       TableType& LOTable, int ntup, int* fKey, int& factValue, Op opType)
{
	  int posOriginal, posTransformed, i;
	  int f1, f2;


      for(i = 0; runtimeInfo.factTblAttrs[i] <= 5; i++) { //get foreign key

            posOriginal    = runtimeInfo.factTblAttrs[i];

            //now , we can get index of the value in the tuple
            posTransformed = runtimeInfo.posMap[posOriginal];

            fKey[i] = LOTable.pLOTable[posTransformed][ntup];

            // printf("%d ", fKey[i]);

      }

      if(runtimeInfo.nFilter > 0) {

        	   switch(runtimeInfo.nFilter) {
        	      case 3:


                        	    posOriginal    = runtimeInfo.factTblAttrs[i];

                        	    //now , we can get index of the value in the tuple
                        	    posTransformed = runtimeInfo.posMap[posOriginal];

                        	    f1 = LOTable.pLOTable[posTransformed][ntup];

                        	    posOriginal    = runtimeInfo.factTblAttrs[i + 1];

                        	   //now , we can get index of the value in the tuple
                                posTransformed = runtimeInfo.posMap[posOriginal];

                        	    f2 = LOTable.pLOTable[posTransformed][ntup];

                        	    factValue = f1 * f2;

                        	    //printf("%d\n", factValue);

                        	    return true;


                         break;
        	      case 4:



        	    	           posOriginal    = runtimeInfo.factTblAttrs[i];

        	    	           //now , we can get index of the value in the tuple
        	    	           posTransformed = runtimeInfo.posMap[posOriginal];

        	    	           f1 = LOTable.pLOTable[posTransformed][ntup];

        	    	           posOriginal    = runtimeInfo.factTblAttrs[i + 1];

        	    	            //now , we can get index of the value in the tuple
        	    	           posTransformed = runtimeInfo.posMap[posOriginal];

        	    	           f2 = LOTable.pLOTable[posTransformed][ntup];

        	    	           factValue = f1 * f2;

        	    	           //printf("%d\n", factValue);

        	    	           return true;


        	    	  break;
        	   }//switch
      }else {
             if(opType != NONE) {

            	 posOriginal    = runtimeInfo.factTblAttrs[i];

			     //now , we can get index of the value in the tuple
			     posTransformed = runtimeInfo.posMap[posOriginal];

			     f1 = LOTable.pLOTable[posTransformed][ntup];

			     posOriginal = runtimeInfo.factTblAttrs[i + 1];

			     //now , we can get index of the value in the tuple
			     posTransformed = runtimeInfo.posMap[posOriginal];

			     f2 = LOTable.pLOTable[posTransformed][ntup];

			     switch(opType) {
			        case MULTIPLY:
			    	   factValue = f1 * f2;
			    	   break;
			        case MINUS:
			    	   factValue = f1 - f2;
			    	   break;
			     }

			     //printf("%d\n", factValue);


             }else {

            	 posOriginal    = runtimeInfo.factTblAttrs[i];

            	//now , we can get index of the value in the tuple
            	 posTransformed = runtimeInfo.posMap[posOriginal];

                 factValue = LOTable.pLOTable[posTransformed][ntup];

                 //printf("%d\n", factValue);

             }

      }

      return true;
}
*/

inline int rearrange(int posInFT)
{
	switch(posInFT) {
	case 0: //cust_key
		return 0;
	case 1: //part_key
		return 3;
	case 2: //supp_key
		return 1;
	case 3: //data_key
		return 2;
	
	default:		//@town: [add] because all the brench should have a return.
		return -1;
	}
}


void
producePrediction(ddtacontext &context, SQLHDBC& dbc)
{


      int counter = 1;
      bool flag = false;
      SQLCHAR SQLStmt[255] = { 0 };
      SQLSMALLINT ret = SQL_SUCCESS;
      char szData[50];
      SQLHSTMT stmt;

	  for(int i = 0; i <= 3; i++) {		//作用在维表上的SQL


		      ret = SQLAllocStmt(dbc, &stmt);

		      char* SqlString =  context.rewrtSQLStat[i];

			  if(strlen(SqlString) == 0) continue;

			  // Define A SELECT SQL Statement
			  strcpy((char *) SQLStmt, SqlString);
			  printf("\nrewrite sql is %s\n", SqlString);

			  // Prepare And Execute The SQL Statement
			  ret = SQLExecDirect(stmt, SQLStmt, SQL_NTS);


			  bimap<string, int> &bimap      =
				           i == 0 ? context.mapForCustomer     :
						   (  i == 1 ? context.mapForSupplier :
						     ( i == 2 ? context.mapForDwdate   :
                                        context.mapForPart
							 )
						   );



			  vector<int> &prediction =
			                 i == 0 ? context.predCustomer  :
						    (   i == 1 ? context.predSupplier :
						        (i == 2 ? context.predDwdate  :
							              context.predPart
							    )
						    );

			  dynamic_bitset<> predbit =
					  i == 0 ? context.predbitCust  :
					 		 (   i == 1 ? context.predbitSupp :
					 			 (i == 2 ? context.predbitDate  :
					 					   context.predbitPart
					 			 )
					 	     );

		      //initialize for "0"
			  if(context.isTranslated[i]) {
			       bimap.left.insert(make_pair("0", 0));

		           // Loop through each record
		           while((ret = SQLFetch(stmt)) == SQL_SUCCESS) {


                        GetStringFromDB(stmt, (SQLCHAR*)szData, 1);


						BOOST_AUTO(iter, bimap.left.find(szData));

						if(iter == bimap.left.end( )) {//not in the map

						     bimap.left.insert(make_pair(szData, counter));

						     prediction.push_back(counter);

						     //printf("%s\n", szData);
						     counter++;
						}else {//already in the map

						     prediction.push_back(iter->second);


						     //cout << iter->first << "<--->" << iter->second << endl;
						}

					 }//end while
				   //printf("###produce <key,value> ok!\n");
			  }
			  else {
				     long predictionData[1] = {1};

                     while((ret = SQLFetch(stmt)) == SQL_SUCCESS) {

                    	  GetTupleFromDB(stmt, predictionData, 1, 1);

						  prediction.push_back(predictionData[0]);

					 }
                     
                     /*int k;
                     for(k = 0; k < prediction.size(); k++) {
                    	 printf("%d ", prediction[k]);
                    	 if(k % 15 == 0) printf("\n");
                     }*/
					 //printf("###produce bitmap ok!\n");
			  }//end if
			  SQLFreeHandle(SQL_HANDLE_STMT, stmt);

     }//end for

	 for(int i = 0; i < 4; i++) {
		  vector<int> &prediction =
					                 i == 0 ? context.predCustomer  :
								    (   i == 1 ? context.predSupplier :
								        (i == 2 ? context.predDwdate  :
									              context.predPart
									    )
								    );

		  dynamic_bitset<> &predbit =
							  i == 0 ? context.predbitCust  :
							 		 (   i == 1 ? context.predbitSupp :
							 			 (i == 2 ? context.predbitDate  :
							 					   context.predbitPart
							 			 )
							 	     );
		  predbit.resize(prediction.size( ), true);

		  for(size_t j = 0; j < prediction.size( ); j++)
			  if(!prediction[j])
				  predbit.reset(j);		//@town：将第 j 位设置为 0


	 }
	  printf("produce ok\n");

}

inline bool predJudge(ddtacontext& context, int fKey, int tblPos )
{

	switch(tblPos) {
		case 0:
			if(fKey >= context.predCustomer.size( ) || !context.predCustomer[fKey])
				return false;
			return true;
			//return context.predbitCust[fKey];
		case 1:
			if(fKey >= context.predPart.size( ) || !context.predPart[fKey])
					return false;
			return true;
			//return context.predbitPart[fKey];
		case 2:
			if(fKey >= context.predSupplier.size( ) || !context.predSupplier[fKey])
					return false;
			return true;
			//return context.predbitSupp[fKey];
		case 3:
			if(fKey >= context.predDwdate.size( ) || !context.predDwdate[fKey])
					return false;
			return true;
			//return context.predbitDate[fKey];

		default:		//@town: [add]
			return true;
	}
}


void *exec_cddta_thread(void * arg){

	thread_args * mt_arg = (thread_args * )arg;
	bool  flag, flags[4];
    int  fKey[4], factValue;

    vector<Node*> &targetList = (mt_arg->pSelect)->targetList;

    Op opType;
    long nInGrp = 0, startCnt, endCnt, totalCnt = 0;

    for (size_t t = 0; t < targetList.size(); t++){

      	 if (targetList[t]->type == T_AGG) {

      	  	  AggNode* p = (AggNode*) (targetList[t]);
      	  	  opType = p->op;
      	  }
    }

    int  attrsToGet[7];
    memset(attrsToGet, -1, sizeof(attrsToGet));		// 初始化 attrsToGet[] 各元素为-1

    int posOriginal,  posTransformed, f1, f2, k = 0;

	//@town: 用于下面做过滤顺序优化
	int lo_quantity = -1, lo_discount = -1;

    for(int i = 0; (mt_arg->pRTInfoPtr)->factTblAttrs[i] != -2; i++) {

    	 posOriginal    = (mt_arg->pRTInfoPtr)->factTblAttrs[i];

    	 //now , we can get index of the value in the tuple
    	 posTransformed = (mt_arg->pRTInfoPtr)->posMap[posOriginal];

    	 attrsToGet[k++] = posTransformed;

    }

    memset(fKey, -1, sizeof(fKey));

	for (size_t i = mt_arg-> fact_start; i <= mt_arg-> fact_end; i++) {

              flag = true;

			  //@town: 先判断事实表的过滤条件
			  /*
			  if (query < 3) {
					lo_discount = (mt_arg->pFactTable)->pLOTable[6][i];
					lo_quantity = (mt_arg->pFactTable)->pLOTable[4][i];

					switch ((mt_arg->pRTInfoPtr)->nFilter)
					{
					case 3:
							if (lo_discount >= (mt_arg->pRTInfoPtr)->filter[0] &&
								lo_discount <= (mt_arg->pRTInfoPtr)->filter[1] &&
								lo_quantity < (mt_arg->pRTInfoPtr)->filter[2])
								flag = true;
							else continue;

							break;

					case 4:
							if (lo_discount >= (mt_arg->pRTInfoPtr)->filter[0] &&
								lo_discount <= (mt_arg->pRTInfoPtr)->filter[1] &&
								lo_quantity >= (mt_arg->pRTInfoPtr)->filter[2] &&
								lo_quantity <= (mt_arg->pRTInfoPtr)->filter[3])
								flag = true;
							else continue;

							break;
					}
			  }
			*/	
			 
			  //get foreign key in fact table and judge
			  for(k = 0;  attrsToGet[k] < 4; k++) {

				  int myOrder = rearrange(attrsToGet[k]);

				  fKey[myOrder] = (mt_arg->pFactTable)->pLOTable[attrsToGet[k]][i];		//拿到外键，对应维表位图或键值对的下标

				  flag = predJudge(*mt_arg->contextptr, fKey[myOrder], attrsToGet[k]);

				  if(!flag) break;
			  }

			  if(!flag){continue;}

			  //@town: 后判断事实表过滤条件
			  /*if (query < 3) {
				  lo_discount = (mt_arg->pFactTable)->pLOTable[6][i];
				  lo_quantity = (mt_arg->pFactTable)->pLOTable[4][i];

				  switch ((mt_arg->pRTInfoPtr)->nFilter)
				  {
				  case 3:
					  if (lo_discount >= (mt_arg->pRTInfoPtr)->filter[0] &&
						  lo_discount <= (mt_arg->pRTInfoPtr)->filter[1] &&
						  lo_quantity < (mt_arg->pRTInfoPtr)->filter[2])
						  flag = true;
					  else continue;

					  break;

				  case 4:
					  if (lo_discount >= (mt_arg->pRTInfoPtr)->filter[0] &&
						  lo_discount <= (mt_arg->pRTInfoPtr)->filter[1] &&
						  lo_quantity >= (mt_arg->pRTInfoPtr)->filter[2] &&
						  lo_quantity <= (mt_arg->pRTInfoPtr)->filter[3])
						  flag = true;
					  else continue;

					  break;
				  }
			  }*/

			  //now, get fact table value for aggregation
			  if(opType != NONE) {
			 		 f1 = (mt_arg->pFactTable)->pLOTable[ attrsToGet[k]][i];
			 		 f2 = (mt_arg->pFactTable)->pLOTable[ attrsToGet[k+1]][i];

			 		// printf(">>%d >>%d \n", f1, f2);

			 		 switch(opType) {

			 		 case MULTIPLY:
			 				factValue = f1 * f2;
			 				break;
			 		 case MINUS:
			 				factValue = f1 - f2;
			 			    break;
			 		  }

			  }else {
			 		 factValue = (mt_arg->pFactTable)->pLOTable[attrsToGet[k]][i];
			  }
			  nInGrp++;
			  startCnt = __rdtsc( );

		      // add to hash map
		      if(mt_arg->pSelect->hasAgg)
				     execGroup( *(mt_arg->contextptr),
				           mt_arg->res,
				           *(mt_arg->pSelect),
				           fKey,
				           factValue
				         );
		      else
			         mt_arg->res.single_result += factValue;

		      endCnt = __rdtsc( );

		      totalCnt += (endCnt - startCnt);

	}

	//cout << "the tuple enter into GROUP BY is: " << nInGrp << endl;
	//cout << "the time used in group by is: " << totalCnt / (2.4 * 1000 * 1000) << endl;
	return NULL;
}


//void *exec_cddta_thread1(void * arg){
//
//
//	thread_args * mt_arg = (thread_args * )arg;
//	int dimIndex[4];
//	bool flags[4], flag;
//    int fKey[4], factValue;
//
//    vector<Node*> &targetList = (mt_arg->pSelect)->targetList;
//    Op opType;
//
//    for (size_t t = 0; t < targetList.size(); t++){
//          if (targetList[t]->type == T_AGG) {
//
//          	  	  AggNode* p = (AggNode*) (targetList[t]);
//          	  	  opType = p->op;
//          }
//     }
//
//     int  attrsToGet[6];
//     memset(attrsToGet, -1, sizeof(attrsToGet));
//     int posOriginal,  posTransformed, f1, f2, k = 0;
//
//     for(int i = 0; (mt_arg->pRTInfoPtr)->factTblAttrs[i] != -2; i++) {
//
//        	 posOriginal    = (mt_arg->pRTInfoPtr)->factTblAttrs[i];
//
//        	 //now , we can get index of the value in the tuple
//        	 posTransformed = (mt_arg->pRTInfoPtr)->posMap[posOriginal];
//
//        	 attrsToGet[k++] = posTransformed;
//
//     }
//
//     DimFilter( *(mt_arg->pRTInfoPtr), *(mt_arg->pFactTable), *(mt_arg->contextptr), *(mt_arg->pSelect),
//    		    mt_arg->fact_start, mt_arg->fact_end);
//
//	 for (size_t i = mt_arg-> fact_start; i <= mt_arg-> fact_end; i++) {
//
//	    if((mt_arg->pRTInfoPtr)->FTFilter.test(i)){
//
//	    	 for(k = 0;  attrsToGet[k] < 4; k++) {
//	    	     int myOrder = rearrange(attrsToGet[k]);
//	    		 fKey[myOrder] = (mt_arg->pFactTable)->pLOTable[ attrsToGet[k] ][i];
//	    	 }
//
//	    	 //now, get fact table value for aggregation
//	    	  if(opType != NONE) {
//	    		   f1 = (mt_arg->pFactTable)->pLOTable[ attrsToGet[k] ][i];
//	    		   f2 = (mt_arg->pFactTable)->pLOTable[ attrsToGet[k+1] ][i];
//
//	    				 switch(opType) {
//
//	    				 case MULTIPLY:
//	    				 		factValue = f1 * f2;
//	    				 		break;
//	    				 case MINUS:
//	    				 		factValue = f1 - f2;
//	    				 	    break;
//	    				 }
//
//	    	 }else {
//
//	    		   factValue = (mt_arg->pFactTable)->pFTValue[attrsToGet[k]-4][i];
//	    	}
//
//
//
//		   // add to hash map
//		   if(mt_arg->pSelect->hasAgg)
//				  execGroup( *(mt_arg->contextptr),
//				               mt_arg->res,
//				               *(mt_arg->pSelect),
//				               fKey,
//				               factValue
//				             );
//		   else
//			     mt_arg->res.single_result += factValue;
//
//	    }
//	}
//
//}

void exec_cddta_mt(ddtacontext& context, SelectType& selectVar,
		          TableType& factTable, result& res, runtimeInfoType& runtimeInfo)
{
	/*@town：以下两条是在Windows下使用的*/
	// SYSTEM_INFO sysInfo;
	// GetSystemInfo(&sysInfo);

    // int THREAD_NUM = sysInfo.dwNumberOfProcessors - 7;//sysconf(_SC_NPROCESSORS_ONLN);
	int THREAD_NUM = sysconf(_SC_NPROCESSORS_ONLN) - 7;

	std::vector<thread_args> threads(THREAD_NUM);	//@town: [add] "(THREAD_NUM)"，用于初始化threads向量

	std::vector<pthread_t> tid(THREAD_NUM);

    int i;

    size_t relSize = factTable.size;

    //runtimeInfo.FTFilter.resize(relSize, true);

	// init threads' arguments
	for ( i = 0; i < THREAD_NUM ; i++){

		threads[i].fact_start = relSize / THREAD_NUM * i;
		threads[i].fact_end   = relSize / THREAD_NUM *(i+1) - 1;
		threads[i].contextptr = &context;
		threads[i].pFactTable = &factTable;
		threads[i].pSelect    = &selectVar;
		threads[i].pRTInfoPtr = &runtimeInfo;

	}


	// execute the ddta in multi-threads
	for( i = 0; i< THREAD_NUM; i++){

		int ret = pthread_create (&tid[i], NULL, exec_cddta_thread, &threads[i]);

		if (ret!=0){

			cout << "thread create error!" << std::endl;
		}
	}

	for( i = 0; i < THREAD_NUM; i++){
		pthread_join(tid[i], NULL);
	}

	boost::unordered_map<resultTuple2, int64>::iterator iter;

	if(!selectVar.hasAgg){

		for(i = 0; i< THREAD_NUM; i++)
			res.single_result += threads[i].res.single_result;

		printf("the sigle result is: %lld\n", res.single_result);

	}else{

	    for( i = 0; i< THREAD_NUM; i++){

		    for( iter = threads[i].res.hashgroup.begin();
		         iter!= threads[i].res.hashgroup.end();
		         iter++) {

				if (res.hashgroup.find(iter->first) == res.hashgroup.end()) {
					 
				     res.hashgroup[iter->first] = iter->second;
						
					 /*cout << iter->first.groupData[0] << "|" << iter->first.groupData[1] << "|" << iter->first.groupData[2] << "|";
					 cout << iter->second << endl;*/
				}	
			    else
				     res.hashgroup[iter->first] += iter->second;
		   }//for

	    }//for

		if (res.hashgroup.size() != 0) {
			execsort(res, context, selectVar);
			printResult(context, res, selectVar);
			
			//printf("\n+==================================+\n");
			/*for (iter = res.hashgroup.begin();
				iter != res.hashgroup.end();
				iter++) {
				
				cout << iter->second << "|";
				cout << iter->first.groupData2.d_year << "|" << iter->first.groupData2.p_brand << endl;
			}*/
			//printf("+==================================+\n");
		}
	    	
			

     }//if(!selectVar.hasAgg)

}










