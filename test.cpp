#include "vendored/libpg_query/pg_query.h"
#include "vendored/libpg_query/pg_list.h"
#include <stdio.h>
#include <nlohmann/json.hpp>
#include "SQLClauses.h"

/*
void print_pg_parse_tree2(List* tree) {
    char* tree_json = pg_query_nodes_to_json(tree);
    //printf("%s\n", tree_json);
    cout << tree_json << endl;
    pfree(tree_json);
}
*/

int main(int argc, char* argv[]) {
    //const char* tpch_query2 = "SELECT s_acctbal, s_name, n_name, p_partkey, p_mfgr, s_address, s_phone, s_comment FROM part, supplier, partsupp, nation, region WHERE p_partkey = ps_partkey AND s_suppkey = ps_suppkey AND p_size = 15 AND p_type LIKE '%BRASS' AND s_nationkey = n_nationkey AND n_regionkey = r_regionkey AND r_name = 'EUROPE' AND ps_supplycost = (SELECT min(ps_supplycost) FROM partsupp, supplier, nation, region WHERE p_partkey = ps_partkey AND s_suppkey = ps_suppkey AND s_nationkey = n_nationkey AND n_regionkey = r_regionkey AND r_name = 'EUROPE') ORDER BY s_acctbal DESC, n_name, s_name, p_partkey;";
    int query = atoi(argv[1]);
    const char* ssb_query = pSQL[query];

    void* ctx = pg_query_parse_init();
    PgQueryInternalParsetreeAndError result = pg_query_parse(ssb_query);

    if (result.error) {
        printf("Error occurred: %s\n", result.error->message);
    } else {
        print_pg_parse_tree(result.tree);
    }

    pg_query_free_parse_result(result);
    pg_query_parse_finish(ctx);

    return 0;
}