/*
 * connection.c
 *
 *  Created on: May 7, 2012
 *      Author: onon
 */
#include "connection.h"
#include <stdio.h>
#include <string.h>
#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>
#include <stdlib.h>
#include <odbcinst.h>


SQLRETURN connToDB(SQLHDBC* dbc) {
	SQLHENV env;

	SQLRETURN ret; /* ODBC API return status */
	SQLCHAR outstr[1024];
	SQLSMALLINT outstrlen;

	/* Allocate an environment handle */
	ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);

	/* We want ODBC 3 support */
	if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
		ret = SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void *) SQL_OV_ODBC3,
				0);
		printf("right!\n");
	}

	/* Allocate a connection handle */
	if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
		ret = SQLAllocHandle(SQL_HANDLE_DBC, env, dbc);

	/* Connect to the MySQL */
	ret = SQLDriverConnect(*dbc, 
							NULL, 
							(SQLCHAR*)"DSN=MySQL_ssb_001;Server=localhost;UID=root;PSW=town1277;", 
							SQL_NTS, 
							outstr,
							sizeof(outstr), 
							&outstrlen, 
							SQL_DRIVER_COMPLETE);

	if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
		printf("Connected\n");
		printf("Returned connection string was:\n\t%s\n", outstr);
		if (ret == SQL_SUCCESS_WITH_INFO) {
			printf("Driver reported the following diagnostics\n");
			extract_error((char*)"SQLDriverConnect", *dbc, SQL_HANDLE_DBC);	//@town: add "(char*)"
		}
		// Allocate An SQL Statement Handle
		// ret = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
		return ret;
	}else {
		fprintf(stderr, "Failed to connect\n");
		extract_error((char*)"SQLDriverConnect", *dbc, SQL_HANDLE_DBC);	//@town: add "(char*)"
	}
	return SQL_ERROR;
}

static void extract_error(char *fn, SQLHANDLE handle, SQLSMALLINT type)
{
	SQLINTEGER i = 0;
	SQLINTEGER native;
	SQLCHAR state[7];
	SQLCHAR text[512];
	SQLSMALLINT len;
	SQLRETURN ret;

	fprintf(stderr, "\n"
			"The driver reported the following diagnostics while running "
			"%s\n\n", fn);

	do {
		memset(text, 0, sizeof(text));
		ret = SQLGetDiagRec(type, handle, ++i, state, &native, text,
				sizeof(text), &len);
		printf("textLen = %d\n", len);
		if (SQL_SUCCEEDED(ret))
			printf("%s:%d:%d:%s\n", state, i, native, text);	//@town: change "%ld" to "%d"
	} while (ret == SQL_SUCCESS);
}


void GetTupleFromDB (SQLHSTMT stmt, long* LOTuple, int startFieldNum, int endFieldNum )
{
	// At this point you would want to do something
	// with the resultset, such as display it.

	int i;
	SQLSMALLINT type, nameLen;
	SQLCHAR colName[25];
	SQLSMALLINT isNull;
	unsigned int colSize;
	SQLSMALLINT scale;

	 int sCustID;
	 SQLINTEGER cbLen;
	 SQLCHAR szDigit[20];
	 int index ;


	for (i = startFieldNum; i <= endFieldNum; i++) {
		SQLDescribeCol(stmt, *((SQLSMALLINT*) &i), colName, sizeof(colName),
				&nameLen, &type, (SQLULEN *) &colSize, &scale, &isNull);
		//SQLColAttribute(stmt, *((SQLSMALLINT*)&i), SQL_DESC_TYPE, &type, sizeof(type), NULL, NULL);

		switch (type) {
		    case SQL_CHAR:
		    case SQL_VARCHAR:

			    SQLGetData(stmt, *((SQLUSMALLINT*)&i), SQL_C_DEFAULT, szDigit, 20, (SQLLEN*)&cbLen);

			    LOTuple[i - 1] = atoi((char*)szDigit);
			   // printf("%d ", LOTuple[i - 1]);
			    break;

		    case SQL_INTEGER:
                index =  i;
			    SQLGetData(stmt, *((SQLUSMALLINT*)&i), SQL_C_DEFAULT, &LOTuple[index - 1], 0, (SQLLEN*)&cbLen);

               // printf("%ld ", LOTuple[i - 1]);
			    break;

		    case SQL_SMALLINT:
		    	 index =  i;
		         SQLGetData(stmt, *((SQLUSMALLINT*)&i), SQL_C_DEFAULT, &LOTuple[index - 1], 0, (SQLLEN*)&cbLen);
		    	// printf("tt\n");
		    	 break;

		    default:
		    	//extract_error("SQLDescribeCol", stmt, SQL_HANDLE_STMT);
		    	//index =  i;
		    	//SQLGetData(stmt, *((SQLUSMALLINT*)&i), SQL_C_DEFAULT, &LOTuple[index - 1], 0, (SQLLEN*)&cbLen);
		     //   //printf("%d ", LOTuple[i - 1]);
		    	//break;
				SQLGetData(stmt, *((SQLUSMALLINT*)&i), SQL_C_DEFAULT, szDigit, 20, (SQLLEN*)&cbLen);

				LOTuple[i - 1] = atoi((char*)szDigit);
				// printf("%d ", LOTuple[i - 1]);
				break;
		}

	} //end for

}


void GetStringFromDB (SQLHSTMT stmt, SQLCHAR* szData, int i)
{

	SQLSMALLINT type, nameLen;
	SQLCHAR colName[25];
	SQLSMALLINT isNull;
	unsigned int colSize;
	SQLSMALLINT scale;

	int sCustID;
	SQLINTEGER cbLen;
	int index;

	SQLDescribeCol(stmt, *((SQLSMALLINT*) &i), colName, sizeof(colName),
			&nameLen, &type, (SQLULEN *) &colSize, &scale, &isNull);

	switch (type) {
	case SQL_CHAR:
	case SQL_VARCHAR:

		SQLGetData(stmt, *((SQLUSMALLINT*) &i), SQL_C_DEFAULT, szData, 20,
				(SQLLEN*)&cbLen);

		break;

	case SQL_INTEGER:
	case SQL_SMALLINT:
		printf("wrong data");
		break;
	}


}

