#ifndef RESULTSET_H_
#define RESULTSET_H_

#include <boost/unordered/unordered_map.hpp>
#include "boost/smart_ptr/shared_ptr.hpp"
#include "parse.h"
#include "stdafx.h"
#include <algorithm>
#include <vector>
#include "rewriteSQL1.h"
#include "typedef.h"


using namespace std;
using namespace boost;

//typedef  long long  int64;



typedef struct resultTuple2 {
	vector<int>       groupData; //scan the fact table and get the data
	int64             aggval;

	resultTuple2 &operator= (const resultTuple2 &other) {
		if (this == &other)
			return *this;
		this->groupData   = other.groupData;
		this->aggval      = other.aggval;
		return *this;
	}

	resultTuple2( ) : aggval(0) { }

	// the operator == is for hash map
	friend bool operator ==(const resultTuple2 &left, const resultTuple2 &right) {
		if (&left == &right)
			return true;
		size_t limit = left.groupData.size();
		assert(left.groupData.size() == right.groupData.size());

		for (size_t i = 0; i < limit; i++) {
			if (left.groupData[i] != right.groupData[i])
				return false;
		}

		return true;
	}

	// Calculate hash value
    friend size_t hash_value(const resultTuple2 &p) {

    	return boost::hash_range(p.groupData.begin(), p.groupData.end());
	}

}resultTuple2;

typedef struct result {

	vector<resultTuple2> retval;
	int64               single_result;
	
	boost::unordered_map<resultTuple2, int64> hashgroup;

	result() :single_result(0) { }
}result;




//typedef struct {
//	int d_year;
//	string p_brand;
//
//	//string c_data;	//@town
//	//string s_data;	//@town
//}GroupData2;
//
//typedef struct resultTuple2 {
//	GroupData2   groupData2;
//	int64        aggval;
//
//	resultTuple2 &operator= (const resultTuple2 &other) {
//		if (this == &other)
//			return *this;
//		this->groupData2.d_year = other.groupData2.d_year;
//		this->groupData2.p_brand.assign(other.groupData2.p_brand.c_str( ));
//		this->aggval      = other.aggval;
//		return *this;
//	}
//
//	resultTuple2( ) : aggval(0) { }
//
//	/* the operator == is for hash map */
//	friend bool operator ==(const resultTuple2 &left, const resultTuple2 &right) {
//		if (&left == &right)
//			return true;
//
//	    if(left.groupData2.d_year != right.groupData2.d_year)
//			return false;
//		if( strcmp(left.groupData2.p_brand.c_str( ), right.groupData2.p_brand.c_str( )) != 0)
//			return false;
//
//		return true;
//	}
//
//	/* Calculate hash value */
//    friend std::size_t hash_value(resultTuple2 const& resTuple2)
//    {
//        std::size_t seed = 0;
//        boost::hash_combine(seed, resTuple2.groupData2.d_year);
//		boost::hash_combine(seed, resTuple2.groupData2.p_brand);
//
//        return seed;
//    }
//
//
//}resultTuple2;
//
//typedef struct result {
//	vector<resultTuple2> retval;
//	int64               single_result;
//
//	unordered_map<resultTuple2, int64> hashgroup;
//	result( ) :single_result(0) { }
//}result;


void execsort(result &res,
		      ddtacontext &context,
		      SelectType &selectVar);


#endif 

