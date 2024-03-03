/*
 * connection.h
 *
 *  Created on: May 7, 2012
 *      Author: onon
 */

#ifndef CONNECTION_H_
#define CONNECTION_H_

#include <stdio.h>
#include <string.h>
#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>
#include <stdlib.h>
#include <odbcinst.h>

//@town: 解决 connToDB 未定义标识符的问题
SQLRETURN connToDB(SQLHDBC *dbc);

void GetTupleFromDB (SQLHSTMT stmt, long* LOTuple, int startFieldNum, int endFieldNum );

void GetStringFromDB(SQLHSTMT stmt, SQLCHAR* attriValue, int nFields);

static void extract_error(char *fn, SQLHANDLE handle, SQLSMALLINT type);

#endif /* CONNECTION_H_ */
