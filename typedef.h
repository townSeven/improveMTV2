/*
 * typedef.h
 *
 *  Created on: May 20, 2012
 *      Author: root
 */

#ifndef TYPEDEF_H_
#define TYPEDEF_H_

#ifndef _DDTA_TYPEDEF_H_
#define _DDTA_TYPEDEF_H_

#ifndef WIN32
#include <inttypes.h>

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef long long int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef uint8 bits8;
typedef uint16 bits16;
typedef uint32 bits32;

const char FILE_SEPARATOR = '/';
#else

typedef char int8;
typedef short int16;
typedef int int32;
typedef long long int64;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;

const char FILE_SEPARATOR = '\\';

#endif /*_WIN32*/

typedef uint8 bits8;
typedef uint16 bits16;
typedef uint32 bits32;

typedef char *Pointer;
typedef unsigned int Oid;
typedef uint32 TransactionId;
typedef uint32 CommandId;

///////////////////////////////////////////////////data
typedef unsigned long Datum; /* XXX sizeof(long) >= sizeof(void *) */

#define SIZEOF_DATUM sizeof(unsigned long)

typedef Datum *DatumPtr;

#define GET_1_BYTE(datum)	(((Datum) (datum)) & 0x000000ff)
#define GET_2_BYTES(datum)	(((Datum) (datum)) & 0x0000ffff)
#define GET_4_BYTES(datum)	(((Datum) (datum)) & 0xffffffff)
#define SET_1_BYTE(value)	(((Datum) (value)) & 0x000000ff)
#define SET_2_BYTES(value)	(((Datum) (value)) & 0x0000ffff)
#define SET_4_BYTES(value)	(((Datum) (value)) & 0xffffffff)

/*
 * DatumGetBool
 *		Returns boolean value of a datum.
 *
 * Note: any nonzero value will be considered TRUE, but we ignore bits to
 * the left of the width of bool, per comment above.
 */

#define DatumGetBool(X) ((bool) (((bool) (X)) != 0))

/*
 * BoolGetDatum
 *		Returns datum representation for a boolean.
 *
 * Note: any nonzero value will be considered TRUE.
 */

#define BoolGetDatum(X) ((Datum) ((X) ? 1 : 0))

/*
 * DatumGetChar
 *		Returns character value of a datum.
 */

#define DatumGetChar(X) ((char) GET_1_BYTE(X))

/*
 * CharGetDatum
 *		Returns datum representation for a character.
 */

#define CharGetDatum(X) ((Datum) SET_1_BYTE(X))

/*
 * Int8GetDatum
 *		Returns datum representation for an 8-bit integer.
 */

#define Int8GetDatum(X) ((Datum) SET_1_BYTE(X))

/*
 * DatumGetUInt8
 *		Returns 8-bit unsigned integer value of a datum.
 */

#define DatumGetUInt8(X) ((uint8) GET_1_BYTE(X))

/*
 * UInt8GetDatum
 *		Returns datum representation for an 8-bit unsigned integer.
 */

#define UInt8GetDatum(X) ((Datum) SET_1_BYTE(X))

/*
 * DatumGetInt16
 *		Returns 16-bit integer value of a datum.
 */

#define DatumGetInt16(X) ((int16) GET_2_BYTES(X))

/*
 * Int16GetDatum
 *		Returns datum representation for a 16-bit integer.
 */

#define Int16GetDatum(X) ((Datum) SET_2_BYTES(X))

/*
 * DatumGetUInt16
 *		Returns 16-bit unsigned integer value of a datum.
 */

#define DatumGetUInt16(X) ((uint16) GET_2_BYTES(X))

/*
 * UInt16GetDatum
 *		Returns datum representation for a 16-bit unsigned integer.
 */

#define UInt16GetDatum(X) ((Datum) SET_2_BYTES(X))

/*
 * DatumGetInt32
 *		Returns 32-bit integer value of a datum.
 */

#define DatumGetInt32(X) ((int32) GET_4_BYTES(X))

/*
 * Int32GetDatum
 *		Returns datum representation for a 32-bit integer.
 */

#define Int32GetDatum(X) ((Datum) SET_4_BYTES(X))

/*
 * DatumGetUInt32
 *		Returns 32-bit unsigned integer value of a datum.
 */

#define DatumGetUInt32(X) ((uint32) GET_4_BYTES(X))

/*
 * UInt32GetDatum
 *		Returns datum representation for a 32-bit unsigned integer.
 */

#define UInt32GetDatum(X) ((Datum) SET_4_BYTES(X))

/*
 * DatumGetObjectId
 *		Returns object identifier value of a datum.
 */

#define DatumGetObjectId(X) ((Oid) GET_4_BYTES(X))

/*
 * ObjectIdGetDatum
 *		Returns datum representation for an object identifier.
 */

#define ObjectIdGetDatum(X) ((Datum) SET_4_BYTES(X))

/*
 * DatumGetPointer
 *		Returns pointer value of a datum.
 */

#define DatumGetPointer(X) ((Pointer) (X))

/*
 * PointerGetDatum
 *		Returns datum representation for a pointer.
 */

#define PointerGetDatum(X) ((Datum) (X))

/*
 * DatumGetCString
 *		Returns C string (null-terminated string) value of a datum.
 *
 * Note: C string is not a full-fledged Postgres type at present,
 * but type input functions use this conversion for their inputs.
 */

#define DatumGetCString(X) ((char *) DatumGetPointer(X))

/*
 * CStringGetDatum
 *		Returns datum representation for a C string (null-terminated string).
 *
 * Note: C string is not a full-fledged Postgres type at present,
 * but type output functions use this conversion for their outputs.
 * Note: CString is pass-by-reference; caller must ensure the pointed-to
 * value has adequate lifetime.
 */

#define CStringGetDatum(X) PointerGetDatum(X)

#include <cstdlib>
enum DataType {
	T_INT = 0, T_STRING, T_LONGLONG
};
class Bitset {
public:
	virtual void set(size_t pos) = 0;
	virtual void reset(size_t pos) = 0;
	virtual bool test(size_t pos) = 0;
	virtual void resize(size_t size, bool val = false) = 0;
};

#include <vector>
#include <stdexcept>
class CharBitset: Bitset {
protected:
	std::vector<unsigned char> bitset;
public:
	virtual void set(size_t pos) {
		if (pos >= bitset.size()) {
			bitset.resize(pos + 1, 0);
		}
		bitset[pos] = 1;
	}
	virtual void reset(size_t pos) {
		if (pos >= bitset.size()) {
			bitset.resize(pos + 1, 0);
		}
		bitset[pos] = 0;
	}
	virtual bool test(size_t pos) {
		if (pos >= bitset.size()) {
			throw std::invalid_argument("pos out of range");
		}
		return bitset[pos] == 1;
	}
	virtual void resize(size_t size, bool val = false) {
		if (true == val)
			bitset.resize(size, 1);
		else
			bitset.resize(size, 0);
	}
};

#if _DEBUG
#include <cstdarg>
#include <cstdio>
#include <iostream>
#endif
static inline void DbgPrint(const char * fmt, ...) {
#if _DEBUG
	char buffer[256];
	va_list args;
	va_start (args, fmt);
	vsprintf_s (buffer,fmt, args);
	std::cerr<<buffer;
	std::cerr.flush();
	va_end (args);
#endif
}

#endif /*_DDTA_TYPEDEF_H_*/



#endif /* TYPEDEF_H_ */
