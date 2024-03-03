
char *pSQL[13]= {		//@town: remove "extern"
		//Q1.1 0	@town: change 1993 to 1992
		(char*)"select sum(lo_extendedprice*lo_discount) as \
		revenue \
		from lineorder, dwdate \
		where lo_orderdate = d_datekey \
		and d_year = 1993 \
		and lo_discount between 1 and 3 \
		and lo_quantity < 25",

		//Q1.2 1	@town: change 'Jan1994' to 'Jan1992'
		(char*)"select sum(lo_extendedprice*lo_discount) as \
		revenue \
		from lineorder, dwdate \
		where lo_orderdate = d_datekey \
		and d_yearmonth = 'Jan1992' \
		and lo_discount between 4 and 6 \
		and lo_quantity between 26 and 35",

		//Q1.3 2	@town: change 'd_weeknuminyear = 6' to 'd_weeknuminyear = 1' and '1994' to '1992'
		(char*)"select sum(lo_extendedprice*lo_discount) as \
		revenue \
		from lineorder, dwdate \
		where lo_orderdate = d_datekey \
		and d_weeknuminyear = 1 \
		and d_year = 1992 \
		and lo_discount between 5 and 7 \
		and lo_quantity between 26 and 35",

        //Q2.1 3
		(char*)"select sum(lo_revenue), d_year, p_brand \
		from lineorder, dwdate, part, supplier \
		where lo_orderdate = d_datekey \
		and lo_partkey = p_partkey \
		and lo_suppkey = s_suppkey \
		and p_category = 'MFGR#12' \
		and s_region = 'AMERICA' \
		group by d_year, p_brand \
		order by d_year, p_brand",

		//Q2.2 4
		(char*)"select sum(lo_revenue), d_year, p_brand \
		from lineorder, dwdate, part, supplier \
		where lo_orderdate = d_datekey \
		and lo_partkey = p_partkey \
		and lo_suppkey = s_suppkey \
		and p_brand between 'MFGR#2221' \
		and 'MFGR#2228' \
		and s_region = 'ASIA' \
		group by d_year, p_brand \
		order by d_year, p_brand",

		//Q2.3 5
		(char*)"select sum(lo_revenue), d_year, p_brand \
		from lineorder, dwdate, part, supplier \
		where lo_orderdate = d_datekey \
		and lo_partkey = p_partkey \
		and lo_suppkey = s_suppkey \
		and p_brand= 'MFGR#2239' \
		and s_region = 'EUROPE' \
		group by d_year, p_brand \
		order by d_year, p_brand",

		//Q3.1 6
		(char*)"select c_nation, s_nation, d_year, \
		sum(lo_revenue) as revenue \
		from customer, lineorder, supplier, dwdate \
		where lo_custkey = c_custkey \
		and lo_suppkey = s_suppkey \
		and lo_orderdate = d_datekey \
		and c_region = 'ASIA' \
		and s_region = 'ASIA' \
		and d_year >= 1992 and d_year <= 1997 \
		group by c_nation, s_nation, d_year \
		order by d_year asc, revenue desc",

		//Q3.2 7
		(char*)"select c_city, s_city, d_year, sum(lo_revenue) \
		as revenue \
		from customer, lineorder, supplier, dwdate \
		where lo_custkey = c_custkey \
		and lo_suppkey = s_suppkey \
		and lo_orderdate = d_datekey \
		and c_nation = 'UNITED STATES' \
		and s_nation = 'UNITED STATES' \
		and d_year >= 1992 and d_year <= 1997 \
		group by c_city, s_city, d_year \
		order by d_year asc, revenue desc",

		//Q3.3 8
		(char*)"select c_city, s_city, d_year, sum(lo_revenue) \
		as revenue \
		from customer, lineorder, supplier, dwdate \
		where lo_custkey = c_custkey \
		and lo_suppkey = s_suppkey \
		and lo_orderdate = d_datekey \
		and (c_city='UNITED KI1' \
		or c_city='UNITED KI5') \
		and (s_city='UNITED KI1' \
		or s_city='UNITED KI5')  \
		and d_yearmonth = 'Dec1997' \
		group by c_city, s_city, d_year \
		order by d_year asc, revenue desc",

		//Q3.4 9
		(char*)"select c_city, s_city, d_year, sum(lo_revenue) \
		as revenue \
		from customer, lineorder, supplier, dwdate \
		where lo_custkey = c_custkey \
		and lo_suppkey = s_suppkey \
		and lo_orderdate = d_datekey \
		and (c_city='UNITED KI1' \
		or c_city='UNITED KI5') \
		and (s_city='UNITED KI1' \
		or s_city='UNITED KI5') \
		and d_yearmonth <= 'Dec1997' \
		group by c_city, s_city, d_year \
		order by d_year asc, revenue desc",


		//Q4.1 10
		(char*)"select d_year, c_nation, \
		sum(lo_revenue - lo_supplycost) as profit \
		from dwdate, customer, supplier, part, lineorder \
		where lo_custkey = c_custkey \
		and lo_suppkey = s_suppkey \
		and lo_partkey = p_partkey \
		and lo_orderdate = d_datekey \
		and c_region = 'AMERICA' \
		and s_region = 'AMERICA' \
		and (p_mfgr = 'MFGR#1' \
		or p_mfgr = 'MFGR#2') \
		group by d_year, c_nation \
		order by d_year, c_nation",

        //Q4.2 11
		(char*)"select d_year, s_nation, p_category, \
		sum(lo_revenue - lo_supplycost) as profit \
		from dwdate, customer, supplier, part, lineorder \
		where lo_custkey = c_custkey \
		and lo_suppkey = s_suppkey \
		and lo_partkey = p_partkey \
		and lo_orderdate = d_datekey \
		and c_region = 'AMERICA' \
		and s_region = 'AMERICA' \
		and (d_year = 1997 or d_year = 1998) \
		and (p_mfgr = 'MFGR#1' \
		or p_mfgr = 'MFGR#2') \
		group by d_year, s_nation, p_category \
		order by d_year, s_nation, p_category",

        //Q4.3 12
		(char*)"select d_year, s_city, p_brand, \
		sum(lo_revenue - lo_supplycost) as profit \
		from dwdate, customer, supplier, part, lineorder \
		where lo_custkey = c_custkey \
		and lo_suppkey = s_suppkey \
		and lo_partkey = p_partkey \
		and lo_orderdate = d_datekey \
		and c_region = 'AMERICA' \
		and s_nation = 'UNITED STATES' \
		and (d_year = 1997 or d_year = 1998) \
		and p_category = 'MFGR#14' \
		group by d_year, s_city, p_brand \
		order by d_year, s_city, p_brand"

 };




