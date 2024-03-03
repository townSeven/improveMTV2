#include <iostream>
#include <vector>
#include "resultSet.h"
#include "exec_ddta.h"
#include "parse.h"
#include "boost/unordered/unordered_map.hpp"
#include "boost/smart_ptr/shared_ptr.hpp"
#include <cmath> 
#include <stdlib.h>
#include <algorithm>
#include "stdafx.h"

using namespace std;
using namespace boost;


bool   isPreceding(resultTuple2 &a,
		           resultTuple2 &b,
		           Select      &select,
		           ddtacontext &context
				  ) 
{
	vector<SortInfo>  &sortInfoList = select.orderby;
	vector<GroupInfo> &grpInfoList =  select.groupby;

    size_t limit = sortInfoList.size();
	size_t index;
   
	for (size_t i = 0; i < limit; i++) {
		index = sortInfoList[i].sortPosInGroupby;
		if (index != -1) {
			if(sortInfoList[i].type == 0 ) { //integer type
				if (a.groupData[index] < b.groupData[index]) {

					if (sortInfoList[i].isAsc) return true;
					else return false;

				} else if (a.groupData[index] > b.groupData[index]) {

					if (sortInfoList[i].isAsc) return false;
					else return true;

				}
			}else {//string type
                int whichMap = grpInfoList[index].RelInfo.whichRel;
				
				bimap<string, int> &map = 
						whichMap == 0 ? context.mapForCustomer :
						( whichMap == 1 ? context.mapForSupplier :
						  ( whichMap == 2 ? context.mapForDwdate : context.mapForPart
						  )
						);

				BOOST_AUTO(iter,  map.right.find(a.groupData[index]));  

                BOOST_AUTO(iter1, map.right.find(b.groupData[index]));

				if(strcmp(iter->second.c_str( ), iter1->second.c_str( )) < 0) {
			       
					if (sortInfoList[i].isAsc) return true;
			        else return false;

		       } else if (strcmp(iter->second.c_str( ), 
				                 iter1->second.c_str( ) ) > 0) {

			        if (sortInfoList[i].isAsc) return false;
			        else return true;
		       }
			}//if(sortInfoList[i].type == 0 )
       
		} else {

			 // the sort attribute is aggregation value

			if (a.aggval < b.aggval) {
				if (sortInfoList[i].isAsc)
					return true;
				else
					return false;
			} else if (a.aggval > b.aggval) {
				if (sortInfoList[i].isAsc)
					return false;
				else
					return true;
			}
	   }//if
	}//for
	return false;
}



void execsort(result &res, 
			  ddtacontext &context,
			  SelectType &selectVar)
{
	boost::unordered_map<resultTuple2, int64>::iterator it;
	it = res.hashgroup.begin();
	vector<GroupInfo> &grpInfoList = selectVar.groupby;

	for (; it != res.hashgroup.end(); ++it) {
		res.retval.push_back(it->first);
		res.retval.back().aggval = it->second;
	}
	
	for(size_t i = 0; i < res.retval.size( ) - 1; i++) {
		 int tmp = i;
		 for(size_t j = i + 1; j < res.retval.size( ); j++) {
			 if(!isPreceding(res.retval[tmp], res.retval[j], selectVar, context)) {
                      tmp = j;
			 }
		 }
		 if(tmp != i) {
				 resultTuple2 r = res.retval[i];
				 res.retval[i] = res.retval[tmp];
				 res.retval[tmp] = r;
	     }
	}
	
	
}


