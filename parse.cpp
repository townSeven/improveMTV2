#include "parse.h"
#include "stdafx.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>



char* getToken (char* zSql, int* start)	
{
    char *ptr; //walk through the zSql
	char *result;
	int sLen = strlen(zSql);
    int i;
	
    if (*start == sLen) return 0;
	for(i = *start, ptr = (zSql + (*start));
		i < sLen;
		ptr++, i++) //skip through
		
		if( *ptr == '(' ||
			*ptr == ')' ||
			*ptr == '+' ||
			*ptr == '*' ||
			*ptr == '/' ||
			*ptr == '-' ||
			*ptr == '=' ||
			*ptr == '\''){
				result = (char*)malloc(2 * sizeof(char));
				result[0] = *ptr;
				result[1] = 0;
				*start = i + 1;
				return result;
		}
		else if(*ptr == '>' || *ptr == '<') {// a token like ">=" or "<="
			if(*(ptr + 1) == '=') {
				result = (char*)malloc(3 * sizeof(char));
				result[0] = *ptr;
				result[1] = '=';
				result[2] = 0;
				*start = i + 2;
				return result;
			}else {
				result = (char*)malloc(2 * sizeof(char));
				result[0] = *ptr;
				result[1] = 0;
				*start = i + 1;
				return result;
			}
		}
		else if ( (*ptr <= 'z' && *ptr >= 'a') ||
					(*ptr <= 'Z' && *ptr >= 'A') ||
					(isdigit(*ptr))              
				) {
					break;
		}
     
	for(*start = i, ptr = (zSql + i);
		ptr && i < sLen;
		ptr++, i++) {
		if (!(isalpha(*ptr) ||
			isdigit(*ptr) ||
			*ptr == '_' ||
			*ptr == '#')
			) { // now, we get a token
			int len = i - (*start);
			result = (char*)malloc((len + 1)* sizeof(char));
			strncpy(result, zSql + (*start), len);
			result[len] = 0;
			*start = i;
			return result;
		}
	 }//end for

	if (i == sLen) { // the last token
		int len = i - (*start);
		result = (char*)malloc((len + 1)* sizeof(char));
		strncpy(result, zSql + (*start), len);
		result[len] = 0;
		*start = sLen;
		return result;
	}

	//@town: If we reach here and haven't returned yet, it means we've reached the end of the string.
	return 0;
}