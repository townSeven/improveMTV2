#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include "parsenodes.h"

using namespace std;
using json = nlohmann::json;

// 获取targetList中的字段名
void getTargetList(const json& parsed_json) {
    for (const auto& target : parsed_json[0]["SelectStmt"]["targetList"]) {
        if (target["ResTarget"]["val"].contains("FuncCall"))
        {
            std::string agg = target["ResTarget"]["val"]["FuncCall"]["funcname"][0]["String"]["str"];
            cout << "aggNode: " << agg;
            std::string op = target["ResTarget"]["val"]["FuncCall"]["args"][0]["A_Expr"]["name"][0]["String"]["str"];
            /* 左右表达式 */
            std::string lexpr = target["ResTarget"]["val"]["FuncCall"]["args"][0]["A_Expr"]["lexpr"]["ColumnRef"]["fields"][0]["String"]["str"];
            std::string rexpr = target["ResTarget"]["val"]["FuncCall"]["args"][0]["A_Expr"]["rexpr"]["ColumnRef"]["fields"][0]["String"]["str"];
            cout << "(" << lexpr + op + rexpr << ")" << endl;
        }
        else 
        {
            std::string field_name = target["ResTarget"]["val"]["ColumnRef"]["fields"][0]["String"]["str"];
            std::cout << field_name << std::endl;
        }
    }
}

// 遍历fromClause并提取表名
void getFromClause(const json& parsed_json) {
    for (const auto& table : parsed_json[0]["SelectStmt"]["fromClause"]) {
        std::string table_name = table["RangeVar"]["relname"];
        std::cout << table_name << std::endl;
    }
}

void getWhereClause(const json& parsed_json) {
    for (const auto& condition : parsed_json[0]["SelectStmt"]["whereClause"]["BoolExpr"]["args"]) {
        int expr_kind = condition["A_Expr"]["kind"];
        std::string predicate;
        std::string lexpr;      // left expression
        std::string rexpr;      // right expression
        switch (expr_kind)
        {  
            case AEXPR_OP:
                predicate = condition["A_Expr"]["name"][0]["String"]["str"];    // now,the predicate = "op"
                /* if condition */
                // now,the predicate = "lexpr op"
                lexpr = condition["A_Expr"]["lexpr"]["ColumnRef"]["fields"][0]["String"]["str"];
                predicate = lexpr + predicate;

                if (condition["A_Expr"]["rexpr"].contains("ColumnRef"))
                    // now,the predicate = "lexpr op rexpr"
                    predicate += condition["A_Expr"]["rexpr"]["ColumnRef"]["fields"][0]["String"]["str"];
                else if (condition["A_Expr"]["rexpr"].contains("A_Const"))
                {
                    // string const
                    if (condition["A_Expr"]["rexpr"]["A_Const"]["val"].contains("String"))  
                    {
                        rexpr = condition["A_Expr"]["rexpr"]["A_Const"]["val"]["String"]["str"];
                        predicate += "\'" + rexpr + "\'";
                    }
                    // integer const
                    else if (condition["A_Expr"]["rexpr"]["A_Const"]["val"].contains("Integer"))    
                    {
                        predicate += to_string(condition["A_Expr"]["rexpr"]["A_Const"]["val"]["Integer"]["ival"]);
                    }
                    else
                        predicate = "error@town: A_Const error";
                }
                    
                /* 还有子查询的情况 */
                else if (condition["A_Expr"]["rexpr"].contains("SubLink"))
                    cout << "###sublink" << endl;
                else
                    predicate = "error@town: unknown";

                break;

            case AEXPR_LIKE:
                predicate = " LIKE ";
                /* if condition */
                // now,the predicate = "lexpr op"
                lexpr = condition["A_Expr"]["lexpr"]["ColumnRef"]["fields"][0]["String"]["str"];
                predicate = lexpr + predicate;
                /* ... */

                break;
            
            case AEXPR_BETWEEN:
                predicate = " BETWEEN ";
                lexpr = condition["A_Expr"]["lexpr"]["ColumnRef"]["fields"][0]["String"]["str"];
                predicate = lexpr + predicate;
                if (condition["A_Expr"]["rexpr"][0].contains("A_Const"))
                {
                    // string const
                    if (condition["A_Expr"]["rexpr"][0]["A_Const"]["val"].contains("String"))  
                    {
                        lexpr = condition["A_Expr"]["rexpr"][0]["A_Const"]["val"]["String"]["str"];
                        rexpr = condition["A_Expr"]["rexpr"][1]["A_Const"]["val"]["String"]["str"];
                        predicate += lexpr + " and " + rexpr;
                    }
                    // integer const
                    else if (condition["A_Expr"]["rexpr"][0]["A_Const"]["val"].contains("Integer"))    
                    {
                        lexpr = to_string(condition["A_Expr"]["rexpr"][0]["A_Const"]["val"]["Integer"]["ival"]);
                        rexpr = to_string(condition["A_Expr"]["rexpr"][1]["A_Const"]["val"]["Integer"]["ival"]);
                        predicate += lexpr + " and " + rexpr;
                    }
                    else
                        predicate = "error@town: A_Const error";
                }
                else
                    predicate = "error@town: between error";

            default:
                break;
        }
        std::cout << predicate << std::endl;
    }
}

int main() {
    std::string inputString; // 用于存储输入的字符串

    // 使用循环从标准输入读取每一行并追加到字符串中
    std::string line;
    while (std::getline(std::cin, line)) {
        inputString += line; // 将每行内容追加到inputString中
    }

    // 在这里可以对inputString进行后续处理，如解析、分析等操作
    // 解析JSON字符串
    auto parsed_json = json::parse(inputString);

    cout << "---target list: ---" << endl;
    getTargetList(parsed_json);     // 遍历targetList并提取字段名
    
    cout << "---table: ---" << endl;
    getFromClause(parsed_json);     // 遍历fromClause并提取表名

    cout << "---whereClause: ---" << endl;
    getWhereClause(parsed_json);    // 遍历whereClause并提取谓词条件

    return 0;
}