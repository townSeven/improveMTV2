#ifndef _SYSCATALOG_H
#define _SYSCATALOG_H

#include <string.h>

///////////////////////////////////////////////////////////////////////


typedef struct sysCatalogCell {
	char attriName[21];
/*
**  greater than 0, represent string; the number represent the length of the string. 
**  0, represent integer;
*/
	short type;
    sysCatalogCell (const char* name, int t)   //@town,Feb.2nd: change "char" to "const char"
	{  
	   strcpy (attriName, name); 
	   type = t;
	}
	
}sysCatalogCell;


static sysCatalogCell part[ ] = {
	sysCatalogCell("p_partkey", 0),
	sysCatalogCell("p_name", 22)   ,
    sysCatalogCell("p_mfgr", 6),
	sysCatalogCell("p_category", 7),
	sysCatalogCell("p_brand",9)  ,	//@town: change p_brandl -> p_brand
	sysCatalogCell("p_color", 11)  ,
	sysCatalogCell("p_type", 25)   ,
	sysCatalogCell("p_size", 0)   ,
	sysCatalogCell("p_container", 10)
};

static sysCatalogCell customer[ ] = {
	sysCatalogCell("c_custkey", 0),
	sysCatalogCell("c_name", 25)  ,
	sysCatalogCell("c_address", 25)   ,
	sysCatalogCell("c_city", 10),
	sysCatalogCell("c_nation",15)  ,
	sysCatalogCell("c_region", 12)  ,
	sysCatalogCell("c_phone", 15)   ,
	sysCatalogCell("c_mktsegment", 10)
};

static sysCatalogCell supplier[ ] = {
	sysCatalogCell("s_suppkey", 0),
	sysCatalogCell("s_name", 25)   ,
	sysCatalogCell("s_address", 25)   ,
	sysCatalogCell("s_city",10)  ,
	sysCatalogCell("s_nation", 15)  ,
	sysCatalogCell("s_region", 12)   ,
	sysCatalogCell("s_phone", 15)
};

static sysCatalogCell dwdate[ ] = {
	sysCatalogCell("d_datekey", 0),
	sysCatalogCell("d_date", 19)   ,
	sysCatalogCell("d_dayofweek", 10),
	sysCatalogCell("d_month", 0),
	sysCatalogCell("d_year", 0),
	sysCatalogCell("d_yearmonthnum",0)  ,
	sysCatalogCell("d_yearmonth", 8) ,
	sysCatalogCell("d_daynuminweek", 0)   ,
	sysCatalogCell("d_daynuminmonth", 0)   ,
	sysCatalogCell("d_daynuminyear", 0)   ,
    sysCatalogCell("d_monthnuminyear", 0),
	sysCatalogCell("d_weeknuminyear", 0)   ,
	sysCatalogCell("d_sellingseason", 13)   ,
	sysCatalogCell("d_lastdayinweekfl", 0),
	sysCatalogCell("d_lastdayinmonthfl",0)  ,
	sysCatalogCell("d_holidayfl", 0)  ,
	sysCatalogCell("d_weekdayfl", 0)
};

static sysCatalogCell lineorder[ ] = {
	sysCatalogCell("lo_orderkey", 0),
	sysCatalogCell("lo_linenumber", 0)   ,
	sysCatalogCell("lo_custkey", 0)   ,
	sysCatalogCell("lo_partkey", 0),
	sysCatalogCell("lo_suppkey",0)  ,
	sysCatalogCell("lo_orderdate", 0) ,
	sysCatalogCell("lo_orderpriority", 15)   ,
	sysCatalogCell("lo_shippriority", 1)   ,
    sysCatalogCell("lo_quantity", 0),
	sysCatalogCell("lo_extendedprice", 0)   ,
	sysCatalogCell("lo_ordertotalprice", 0)  ,
	sysCatalogCell("lo_discount", 0),
	sysCatalogCell("lo_revenue", 0) ,
	sysCatalogCell("lo_supplycost", 0),
	sysCatalogCell("lo_tax",0) ,
	sysCatalogCell("lo_commitdate",0)  ,
	sysCatalogCell("lo_shipmode", 10)
};

#endif