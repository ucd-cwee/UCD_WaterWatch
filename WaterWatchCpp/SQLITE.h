/*
WaterWatch, Energy Demand Management System for Water Systems. For further information on WaterWatch, please see https://cwee.ucdavis.edu/waterwatch.

Copyright (c) 2019 - 2023 by Robert Good, Erin Musabandesu, and David Linz. (Email: rtgood@ucdavis.edu). All rights reserved.

Created: RTG	/	2023
History: RTG	/	2023		1. Initial Release.

Copyright / Usage Details: You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise)
when your product is released in binary form. You are allowed to modify the source code in any way you want
except you cannot modify the copyright details at the top of each module. If you want to distribute source
code with your application, then you are only allowed to distribute versions released by the author. This is
to maintain a single distribution point for the source code.
*/

#pragma once
#include "Precompiled.h"
#include "Strings.h"
#include "List.h"

static int callbackMIN(void* minCol, int argc, char** argv, char** azColName);
static int callback1(void* count, int argc, char** argv, char** azColName);
static int callback2(void* columns, int argc, char** argv, char** azColName);
static int callback3(void* out, int argc, char** argv, char** azColName);
static void* SQLite_Open(const char* value) noexcept;
static void SQLite_Close(void* value) noexcept;
static void SQLite_Exec(void* value, const char* sql, int (*callback)(void*, int, char**, char**), void* returnArg, char** errmsg) noexcept;
static void SQLite_Prepare(void* value, const char* zSql, int nBytes, void* ppStmt, const char** pzTail) noexcept;
static int SQLite_Step(void* pStmt) noexcept;
static const unsigned char* SQLite_Column_Text(void* pStmt, int i) noexcept;
static int SQLite_Column_Count(void* pStmt) noexcept;
static void SQLite_Finalize(void* pStmt) noexcept;
static void SQLite_GetTable(void* value, const char* sql, char*** pazResult, int* pnRow, int* pnColumn, char** pzErrMsg) noexcept;
static const char* SQLite_GetColumnName(void* pStmt, int N) noexcept;

class SQLite {
public:
	static int		openDB(cweeStr fullPath, cweeStr fileName) noexcept;
	static int		openDbDirect(cweeStr fullPath) noexcept;
	static void		addTable(int proj, const cweeStr& tableName, const cweeThreadedList<cweeStr>& columnNames, const cweeThreadedList < cweeThreadedList<cweeStr> >& data = cweeThreadedList<cweeThreadedList<cweeStr>>());
	static void		addData(int proj, const cweeStr& tableName, const cweeThreadedList<cweeStr>& columnNames, const cweeThreadedList < cweeThreadedList<cweeStr> >& data, const bool useTransaction = true);
	static void		addData(int proj, const cweeStr& tableName, const cweeThreadedList<cweeStr>& columnNames, const cweeThreadedList<cweeStr>& data, const bool useTransaction = true);
	static void		addTable(int proj, const cweeStr& tableName, const cweeStr& column1, const cweeStr& column1Type, const cweeStr& column2 = "", const cweeStr& column2Type = "", const cweeStr& column3 = "", const cweeStr& column3Type = "", const cweeStr& column4 = "", const cweeStr& column4Type = "", const cweeStr& column5 = "", const cweeStr& column5Type = "", const cweeStr& column6 = "", const cweeStr& column6Type = "");
	static void		addTableNonUnique(int proj, const cweeStr& tableName, const cweeStr& column1, const cweeStr& column1Type, const cweeStr& column2 = "", const cweeStr& column2Type = "", const cweeStr& column3 = "", const cweeStr& column3Type = "", const cweeStr& column4 = "", const cweeStr& column4Type = "", const cweeStr& column5 = "", const cweeStr& column5Type = "", const cweeStr& column6 = "", const cweeStr& column6Type = "");
	static void		addNewRowToTable(int proj, const cweeStr& tableName, const cweeStr& data1, const cweeStr& data2 = "", const cweeStr& data3 = "", const cweeStr& data4 = "", const cweeStr& data5 = "", const cweeStr& data6 = "");
	static void		updateColumnValue(int proj, cweeStr tableName, int columnNum, cweeStr column0Seek, cweeStr newValueAtRowColumnIntersection);

	static cweeStr	getTable(int proj, cweeStr command, int* row, int* column) noexcept;

	static int		getNumTables(int proj) noexcept;
	static cweeStr	getTableName(int proj, int whichTable) noexcept;
	static int		getTableNum(int proj, cweeStr tableName) noexcept;

	static int		getNumRows(int proj, cweeStr tableName) noexcept;
	static int		getNumColumns(int proj, cweeStr tableName) noexcept;
	static int		getNumValuesInCol(int proj, cweeStr tableName, int tagPath_ID) noexcept;
	static time_t	getExtremaColumns(int proj, cweeStr Extrema, cweeStr colName, cweeStr tableName) noexcept;
	static cweeStr  getColumnName(int proj, int N, cweeStr tableName = "") noexcept;
	static void		execute(int proj, cweeStr command, int (*callback)(void*, int, char**, char**), void* returnObj, char** errmsg) noexcept;
	static void		createStatement(int proj, cweeStr command) noexcept;
	static int		stepStatement(int proj) noexcept;
	static cweeStr  getColumnText(int proj, int whichCol) noexcept;
	static cweeStr  getCurrentRow(int proj, int numColumns = -1) noexcept;
	static cweeThreadedList<cweeStr>  getNextRow(int proj) noexcept;
	static void		endStatement(int proj) noexcept;
	static void		closeDB(int proj) noexcept;
};
