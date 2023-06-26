
#ifndef __SQLITE3_WRAP_H__
#define __SQLITE3_WRAP_H__

#pragma hdrstop
#include "sqlite3.h"
#include "precompiled.h"

static int callbackMIN(void* minCol, int argc, char** argv, char** azColName);
__forceinline int callbackMIN(void* minCol, int argc, char** argv, char** azColName) {
	time_t* c = (time_t*)minCol;
	*c = atof(argv[0]);
	return 0;
}

static int callback1(void* count, int argc, char** argv, char** azColName);
__forceinline int callback1(void* count, int argc, char** argv, char** azColName) {
	int* c = (int*)count;
	*c = atoi(argv[0]);
	return 0;
}

static int callback2(void* columns, int argc, char** argv, char** azColName);
__forceinline int callback2(void* columns, int argc, char** argv, char** azColName) {
	int* c = (int*)columns;
	*c = argc;
	return 0;
}

static int callback3(void* out, int argc, char** argv, char** azColName);
__forceinline int callback3(void* out, int argc, char** argv, char** azColName) {
	char** c = (char**)out;
	c = argv;
	return 0;
}

static auto SQLite_Open(const char* value) noexcept
{
	sqlite3* DB;
	if (sqlite3_open(value, &DB)) sqlite3_close(DB);
	return DB;
}

static auto SQLite_Close(sqlite3* value) noexcept
{
	sqlite3_close(value);
}

static auto SQLite_Exec(sqlite3* value, const char* sql, int (*callback)(void*, int, char**, char**), void* returnArg, char** errmsg) noexcept
{
	sqlite3_exec(value, sql, callback, returnArg, errmsg);
}

static auto SQLite_Prepare(sqlite3* value, const char* zSql, int nBytes, sqlite3_stmt** ppStmt, const char** pzTail) noexcept
{
	//sqlite3_prepare_v2(value, zSql, nBytes, ppStmt, pzTail);
	sqlite3_prepare(value, zSql, nBytes, ppStmt, pzTail);
}

static auto SQLite_Step(sqlite3_stmt* pStmt) noexcept
{
	return sqlite3_step(pStmt);
}

static const unsigned char* SQLite_Column_Text(sqlite3_stmt* pStmt, int i) noexcept
{
	return sqlite3_column_text(pStmt, i);
}

static auto SQLite_Column_Count(sqlite3_stmt* pStmt) noexcept
{
	return sqlite3_column_count(pStmt);
}

static auto SQLite_Finalize(sqlite3_stmt* pStmt) noexcept
{
	sqlite3_finalize(pStmt);
}

static auto SQLite_GetTable(sqlite3* value, const char* sql, char*** pazResult, int* pnRow, int* pnColumn, char** pzErrMsg) noexcept
{
	sqlite3_get_table(value, sql, pazResult, pnRow, pnColumn, pzErrMsg);
}

static auto SQLite_GetColumnName(sqlite3_stmt* pStmt, int N) noexcept
{
	return sqlite3_column_name(pStmt, N);
}

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

#endif