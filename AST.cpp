//===- AST.cpp - transform SQL to AST(Json) -------------------------------===//
//
//  this file implements the transformation that SQL to AST.
//
//===----------------------------------------------------------------------===//
#include "vendored/libpg_query/pg_query.h"
#include "vendored/libpg_query/pg_list.h"
#include <stdio.h>
#include <nlohmann/json.hpp>
#include "SQLClauses.h"
#include <iostream>

/*
void print_pg_parse_tree2(List* tree) {
    char* tree_json = pg_query_nodes_to_json(tree);
    //printf("%s\n", tree_json);
    cout << tree_json << endl;
    pfree(tree_json);
}
*/

int main(int argc, char* argv[]) {
    int arg = atoi(argv[1]);
    //std::cout << "input query is: " << arg << std::endl;
    int query = -1;

    switch (arg)
    {
    case 11:    query = 0; break;
    case 12:    query = 1; break;
    case 13:    query = 2; break;
    case 21:    query = 3; break;
    case 22:    query = 4; break;
    case 23:    query = 5; break;
    case 31:    query = 6; break;
    case 32:    query = 7; break;
    case 33:    query = 8; break;
    case 34:    query = 9; break;
    case 41:    query = 10; break;
    case 42:    query = 11; break;
    case 43:    query = 12; break;
    
    default:    query = -1; break;
    }
 
    if(query < 0)   
        std::cout << "no such query in ssb!" << std::endl;

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