#ifndef __SQLITE3_WRAP_H__
#define __SQLITE3_WRAP_H__

#pragma hdrstop
#include "sqlite3.h"
#include "Precompiled.h"

INLINE static int callbackMIN(void* minCol, int argc, char** argv, char** azColName) {
	time_t* c = (time_t*)minCol;
	*c = atof(argv[0]);
	return 0;
};
INLINE static int callback1(void* count, int argc, char** argv, char** azColName) {
	int* c = (int*)count;
	*c = atoi(argv[0]);
	return 0;
};
INLINE static int callback2(void* columns, int argc, char** argv, char** azColName) {
	int* c = (int*)columns;
	*c = argc;
	return 0;
};
INLINE static int callback3(void* out, int argc, char** argv, char** azColName) {
	char** c = (char**)out;
	c = argv;
	return 0;
};
INLINE static decltype(auto) SQLite_Open(const char* value) noexcept {
	sqlite3* DB;
	if (sqlite3_open(value, &DB)) sqlite3_close(DB);
	return DB;
};
INLINE static decltype(auto) SQLite_Close(sqlite3* value) noexcept {
	sqlite3_close(value);
};
INLINE static decltype(auto) SQLite_Exec(sqlite3* value, const char* sql, int (*callback)(void*, int, char**, char**), void* returnArg, char** errmsg) noexcept {
	sqlite3_exec(value, sql, callback, returnArg, errmsg);
};
INLINE static decltype(auto) SQLite_Prepare(sqlite3* value, const char* zSql, int nBytes, sqlite3_stmt** ppStmt, const char** pzTail) noexcept {
	sqlite3_prepare(value, zSql, nBytes, ppStmt, pzTail);
};
INLINE static decltype(auto) SQLite_Step(sqlite3_stmt* pStmt) noexcept {
	return sqlite3_step(pStmt);
};
INLINE static const unsigned char* SQLite_Column_Text(sqlite3_stmt* pStmt, int i) noexcept {
	return sqlite3_column_text(pStmt, i);
};
INLINE static decltype(auto) SQLite_Column_Count(sqlite3_stmt* pStmt) noexcept {
	return sqlite3_column_count(pStmt);
};
INLINE static decltype(auto) SQLite_Finalize(sqlite3_stmt* pStmt) noexcept {
	sqlite3_finalize(pStmt);
};
INLINE static decltype(auto) SQLite_GetTable(sqlite3* value, const char* sql, char*** pazResult, int* pnRow, int* pnColumn, char** pzErrMsg) noexcept {
	sqlite3_get_table(value, sql, pazResult, pnRow, pnColumn, pzErrMsg);
}
INLINE static decltype(auto) SQLite_GetColumnName(sqlite3_stmt* pStmt, int N) noexcept {
	return sqlite3_column_name(pStmt, N);
};

class SQLite {
public:
	static cweeSysInterlockedInteger			SQLite_numSQLiteAccess;
	static std::map<int, sqlite3*>				SQLite_databaseMap;
	static std::map<int, sqlite3_stmt*>			SQLite_statementMap;

	static int openDbDirect(cweeStr fullPath) noexcept {
		sqlite3* DB;
		DB = SQLite_Open(fullPath.c_str());
		auto temp = SQLite_numSQLiteAccess.Increment();
		SQLite_databaseMap.insert(std::make_pair(temp, DB));
		return temp;
	};

	static int openDB(cweeStr folderPath, cweeStr fileName) noexcept {
		cweeStr filePath = folderPath + "\\" + fileName + ".db";
		sqlite3* DB;
		DB = SQLite_Open(filePath.c_str());
		auto temp = SQLite_numSQLiteAccess.Increment();
		SQLite_databaseMap.insert(std::make_pair(temp, DB));
		return temp;
	};

	static void addTable(int proj, const cweeStr& tableName, const cweeStr& column1, const cweeStr& column1Type, const cweeStr& column2, const cweeStr& column2Type, const cweeStr& column3, const cweeStr& column3Type, const cweeStr& column4, const cweeStr& column4Type, const cweeStr& column5, const cweeStr& column5Type, const cweeStr& column6, const cweeStr& column6Type) {

		if (SQLite_databaseMap.find(proj) != SQLite_databaseMap.end()) {
			cweeStr command;
			if (column6 != "") {
				command = cweeStr::printf("CREATE TABLE IF NOT EXISTS \"%s\" (\n\"%s\" %s NOT NULL UNIQUE,\n\"%s\"%s NOT NULL,\n\"%s\"%s NOT NULL,\n\"%s\"%s NOT NULL,\n\"%s\"%s NOT NULL,\n\"%s\"%s NOT NULL,\nPRIMARY KEY(\"%s\")\n) WITHOUT ROWID;",
					tableName.c_str(),				// table name
					column1.c_str(),				// column 1 name
					column1Type.c_str(),			// column 1 type
					column2.c_str(),				// column 2 name
					column2Type.c_str(),			// column 2 type
					column3.c_str(),				// column 3 name
					column3Type.c_str(),			// column 3 type
					column4.c_str(),				// column 4 name
					column4Type.c_str(),			// column 4 type
					column5.c_str(),				// column 5 name
					column5Type.c_str(),			// column 5 type
					column6.c_str(),				// column 6 name
					column6Type.c_str(),			// column 6 type
					column1.c_str()					// column 1 reference
				);
			}
			else if (column5 != "") {
				command = cweeStr::printf("CREATE TABLE IF NOT EXISTS \"%s\" (\n\"%s\" %s NOT NULL UNIQUE,\n\"%s\"%s NOT NULL,\n\"%s\"%s NOT NULL,\n\"%s\"%s NOT NULL,\n\"%s\"%s NOT NULL,\nPRIMARY KEY(\"%s\")\n) WITHOUT ROWID;",
					tableName.c_str(),				// table name
					column1.c_str(),				// column 1 name
					column1Type.c_str(),			// column 1 type
					column2.c_str(),				// column 2 name
					column2Type.c_str(),			// column 2 type
					column3.c_str(),				// column 3 name
					column3Type.c_str(),			// column 3 type
					column4.c_str(),				// column 4 name
					column4Type.c_str(),			// column 4 type
					column5.c_str(),				// column 5 name
					column5Type.c_str(),			// column 5 type
					column1.c_str()					// column 1 reference
				);
			}
			else if (column4 != "") {
				command = cweeStr::printf("CREATE TABLE IF NOT EXISTS \"%s\" (\n\"%s\" %s NOT NULL UNIQUE,\n\"%s\"%s NOT NULL,\n\"%s\"%s NOT NULL,\n\"%s\"%s NOT NULL,\nPRIMARY KEY(\"%s\")\n) WITHOUT ROWID;",
					tableName.c_str(),				// table name
					column1.c_str(),				// column 1 name
					column1Type.c_str(),			// column 1 type
					column2.c_str(),				// column 2 name
					column2Type.c_str(),			// column 2 type
					column3.c_str(),				// column 3 name
					column3Type.c_str(),			// column 3 type
					column4.c_str(),				// column 4 name
					column4Type.c_str(),			// column 4 type
					column1.c_str()					// column 1 reference
				);
			}
			else if (column3 != "") {
				command = cweeStr::printf("CREATE TABLE IF NOT EXISTS \"%s\" (\n\"%s\" %s NOT NULL UNIQUE,\n\"%s\"%s NOT NULL,\n\"%s\"%s NOT NULL,\nPRIMARY KEY(\"%s\")\n) WITHOUT ROWID;",
					tableName.c_str(),				// table name
					column1.c_str(),				// column 1 name
					column1Type.c_str(),			// column 1 type
					column2.c_str(),				// column 2 name
					column2Type.c_str(),			// column 2 type
					column3.c_str(),				// column 3 name
					column3Type.c_str(),			// column 3 type
					column1.c_str()					// column 1 reference
				);
			}
			else if (column2 != "") {
				command = cweeStr::printf("CREATE TABLE IF NOT EXISTS \"%s\" (\n\"%s\" %s NOT NULL UNIQUE,\n\"%s\"%s NOT NULL,\nPRIMARY KEY(\"%s\")\n) WITHOUT ROWID;",
					tableName.c_str(),				// table name
					column1.c_str(),				// column 1 name
					column1Type.c_str(),			// column 1 type
					column2.c_str(),				// column 2 name
					column2Type.c_str(),			// column 2 type
					column1.c_str()					// column 1 reference
				);

			}
			else {
				command = cweeStr::printf("CREATE TABLE IF NOT EXISTS \"%s\" (\n\"%s\" %s NOT NULL UNIQUE,\nPRIMARY KEY(\"%s\")\n) WITHOUT ROWID;",
					tableName.c_str(),				// table name
					column1.c_str(),				// column 1 name
					column1Type.c_str(),			// column 1 type
					column1.c_str()					// column 1 reference
				);
			}

			createStatement(proj, command);
			stepStatement(proj);
			endStatement(proj);

			if (column6 != "") {
				command = cweeStr::printf("INSERT INTO %s (\"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\") VALUES (\"\", \"\", \"\", \"\", \"\", \"\");",
					tableName.c_str(),				// table name
					column1.c_str(),				// column 1 name
					column2.c_str(),				// column 2 name
					column3.c_str(),				// column 3 name
					column4.c_str(),				// column 4 name
					column5.c_str(),				// column 5 name
					column6.c_str()					// column 6 name
				);
			}
			else if (column5 != "") {
				command = cweeStr::printf("INSERT INTO %s (\"%s\", \"%s\", \"%s\", \"%s\", \"%s\") VALUES (\"\", \"\", \"\", \"\", \"\");",
					tableName.c_str(),				// table name
					column1.c_str(),				// column 1 name
					column2.c_str(),				// column 2 name
					column3.c_str(),				// column 3 name
					column4.c_str(),				// column 4 name
					column5.c_str()					// column 5 name
				);
			}
			else if (column4 != "") {
				command = cweeStr::printf("INSERT INTO %s (\"%s\", \"%s\", \"%s\", \"%s\") VALUES (\"\", \"\", \"\", \"\");",
					tableName.c_str(),				// table name
					column1.c_str(),				// column 1 name
					column2.c_str(),				// column 2 name
					column3.c_str(),				// column 3 name
					column4.c_str()
				);
			}
			else if (column3 != "") {
				command = cweeStr::printf("INSERT INTO %s (\"%s\", \"%s\", \"%s\") VALUES (\"\", \"\", \"\");",
					tableName.c_str(),				// table name
					column1.c_str(),				// column 1 name
					column2.c_str(),				// column 2 name
					column3.c_str()
				);
			}
			else if (column2 != "") {
				command = cweeStr::printf("INSERT INTO %s (\"%s\", \"%s\") VALUES (\"\", \"\");",
					tableName.c_str(),				// table name
					column1.c_str(),				// column 1 name
					column2.c_str()
				);
			}
			else {
				command = cweeStr::printf("INSERT INTO %s (\"%s\") VALUES (\"\");",
					tableName.c_str(),				// table name
					column1.c_str()
				);
			}

			createStatement(proj, command);
			stepStatement(proj);
			endStatement(proj);
		}
	};

	static void addTable(int proj, const cweeStr& tableName, const cweeThreadedList<cweeStr>& columnNames, const cweeThreadedList < cweeThreadedList<cweeStr> >& data) {
		if (SQLite_databaseMap.find(proj) != SQLite_databaseMap.end()) {
			cweeStr add; cweeStr contraint;
			for (auto& col : columnNames) {
				add.AddToDelimiter(cweeStr::printf("\"%s\" TEXT", col.c_str()), ", ");
				contraint.AddToDelimiter(col, ", ");
			}
			if (!add.IsEmpty()) {
				cweeStr command = cweeStr::printf("CREATE TABLE IF NOT EXISTS \"%s\" (%s, CONSTRAINT uniqueConstraintID UNIQUE(%s));", tableName.c_str(), add.c_str(), contraint.c_str());
				createStatement(proj, command);
				stepStatement(proj);
				endStatement(proj);

				// instantiate the table. 
				{
					command.Clear(); add.Clear();
					cweeStr columns;
					cweeStr values;

					for (auto& col : columnNames) {
						columns.AddToDelimiter(
							cweeStr::printf("\"%s\"", col.c_str()),
							", "
						);
						values.AddToDelimiter(
							"\"\"",
							", "
						);
					}
					cweeStr command = cweeStr::printf("INSERT INTO %s (%s) VALUES (%s);", tableName.c_str(), columns.c_str(), values.c_str());
					createStatement(proj, command);
					stepStatement(proj);
					endStatement(proj);
				}

				// fill the table, if data is available
				if ((data.Num() > 0) && (data[0].Num() == columnNames.Num())) {
					command.Clear(); add.Clear();
					cweeStr columns;
					cweeStr values;

					for (auto& col : columnNames) {
						columns.AddToDelimiter(
							cweeStr::printf("\"%s\"", col.c_str()),
							", "
						);
					}

					execute(proj, "BEGIN TRANSACTION", NULL, NULL, NULL);
					for (auto& row : data) {
						values.Clear();
						for (auto& col : row) {
							values.AddToDelimiter(
								cweeStr::printf("\"%s\"", col.c_str()),
								", "
							);
						}
						cweeStr command = cweeStr::printf("INSERT INTO %s (%s) VALUES (%s);", tableName.c_str(), columns.c_str(), values.c_str());
						createStatement(proj, command);
						stepStatement(proj);
						endStatement(proj);
					}
					execute(proj, "END TRANSACTION", NULL, NULL, NULL);
				}
			}
		}
	};

	static void addData(int proj, const cweeStr& tableName, const cweeThreadedList<cweeStr>& columnNames, const cweeThreadedList < cweeThreadedList<cweeStr> >& data, const bool useTransaction) {
		if (SQLite_databaseMap.find(proj) != SQLite_databaseMap.end()) {
			// fill the table, if data is available
			if ((data.Num() > 0) && (data[0].Num() == columnNames.Num())) {
				cweeStr columns;
				cweeStr values;

				for (auto& col : columnNames) {
					columns.AddToDelimiter(
						cweeStr::printf("\"%s\"", col.c_str()),
						", "
					);
				}

				if (useTransaction) execute(proj, "BEGIN TRANSACTION", NULL, NULL, NULL);
				for (auto& row : data) {
					values.Clear();
					for (auto& col : row) {
						values.AddToDelimiter(
							cweeStr::printf("\"%s\"", col.c_str()),
							", "
						);
					}
					cweeStr command = cweeStr::printf("INSERT INTO %s (%s) VALUES (%s);", tableName.c_str(), columns.c_str(), values.c_str());
					createStatement(proj, command);
					stepStatement(proj);
					endStatement(proj);
				}
				if (useTransaction) execute(proj, "END TRANSACTION", NULL, NULL, NULL);
			}
		}
	};

	static void addData(int proj, const cweeStr& tableName, const cweeThreadedList<cweeStr>& columnNames, const cweeThreadedList<cweeStr>& data, const bool useTransaction) {
		if (SQLite_databaseMap.find(proj) != SQLite_databaseMap.end()) {
			// fill the table, if data is available
			if ((data.Num() > 0) && (data.Num() == columnNames.Num())) {
				cweeStr columns;
				cweeStr values;

				for (auto& col : columnNames) {
					columns.AddToDelimiter(
						cweeStr::printf("\"%s\"", col.c_str()),
						", "
					);
				}

				if (useTransaction) execute(proj, "BEGIN TRANSACTION", NULL, NULL, NULL);
				{
					for (auto& col : data) {
						values.AddToDelimiter(
							cweeStr::printf("\"%s\"", col.c_str()),
							", "
						);
					}
					cweeStr command = cweeStr::printf("INSERT INTO %s (%s) VALUES (%s);", tableName.c_str(), columns.c_str(), values.c_str());
					createStatement(proj, command);
					stepStatement(proj);
					endStatement(proj);
				}
				if (useTransaction) execute(proj, "END TRANSACTION", NULL, NULL, NULL);
			}
		}
	};

	static void addTableNonUnique(int proj, const cweeStr& tableName, const cweeStr& column1, const cweeStr& column1Type, const cweeStr& column2, const cweeStr& column2Type, const cweeStr& column3, const cweeStr& column3Type, const cweeStr& column4, const cweeStr& column4Type, const cweeStr& column5, const cweeStr& column5Type, const cweeStr& column6, const cweeStr& column6Type) {

		if (SQLite_databaseMap.find(proj) != SQLite_databaseMap.end()) {
			cweeStr command;
			if (column6 != "") {
				command = cweeStr::printf("CREATE TABLE IF NOT EXISTS \"%s\" (\n\"%s\" %s,\n\"%s\"%s,\n\"%s\"%s,\n\"%s\"%s,\n\"%s\"%s,\n\"%s\"%s\n);",
					tableName.c_str(),				// table name
					column1.c_str(),				// column 1 name
					column1Type.c_str(),			// column 1 type
					column2.c_str(),				// column 2 name
					column2Type.c_str(),			// column 2 type
					column3.c_str(),				// column 3 name
					column3Type.c_str(),			// column 3 type
					column4.c_str(),				// column 4 name
					column4Type.c_str(),			// column 4 type
					column5.c_str(),				// column 5 name
					column5Type.c_str(),			// column 5 type
					column6.c_str(),				// column 6 name
					column6Type.c_str()				// column 6 type
				);
			}
			else if (column5 != "") {
				command = cweeStr::printf("CREATE TABLE IF NOT EXISTS \"%s\" (\n\"%s\" %s,\n\"%s\"%s,\n\"%s\"%s,\n\"%s\"%s,\n\"%s\"%s\n);",
					tableName.c_str(),				// table name
					column1.c_str(),				// column 1 name
					column1Type.c_str(),			// column 1 type
					column2.c_str(),				// column 2 name
					column2Type.c_str(),			// column 2 type
					column3.c_str(),				// column 3 name
					column3Type.c_str(),			// column 3 type
					column4.c_str(),				// column 4 name
					column4Type.c_str(),			// column 4 type
					column5.c_str(),				// column 5 name
					column5Type.c_str()			// column 5 type
				);
			}
			else if (column4 != "") {
				command = cweeStr::printf("CREATE TABLE IF NOT EXISTS \"%s\" (\n\"%s\" %s,\n\"%s\"%s,\n\"%s\"%s,\n\"%s\"%s\n);",
					tableName.c_str(),				// table name
					column1.c_str(),				// column 1 name
					column1Type.c_str(),			// column 1 type
					column2.c_str(),				// column 2 name
					column2Type.c_str(),			// column 2 type
					column3.c_str(),				// column 3 name
					column3Type.c_str(),			// column 3 type
					column4.c_str(),				// column 4 name
					column4Type.c_str()				// column 4 type
				);
			}
			else if (column3 != "") {
				command = cweeStr::printf("CREATE TABLE IF NOT EXISTS \"%s\" (\n \"%s\" %s, \n \"%s\" %s, \n \"%s\" %s \n);",
					tableName.c_str(),				// table name
					column1.c_str(),				// column 1 name
					column1Type.c_str(),			// column 1 type
					column2.c_str(),				// column 2 name
					column2Type.c_str(),			// column 2 type
					column3.c_str(),				// column 3 name
					column3Type.c_str()				// column 3 type
				);
			}
			else if (column2 != "") {
				command = cweeStr::printf("CREATE TABLE IF NOT EXISTS \"%s\" (\n\"%s\" %s,\n\"%s\"%s\n);",
					tableName.c_str(),				// table name
					column1.c_str(),				// column 1 name
					column1Type.c_str(),			// column 1 type
					column2.c_str(),				// column 2 name
					column2Type.c_str()				// column 2 type
				);

			}
			else {
				command = cweeStr::printf("CREATE TABLE IF NOT EXISTS \"%s\" (\n\"%s\" %s\n);",
					tableName.c_str(),				// table name
					column1.c_str(),				// column 1 name
					column1Type.c_str()				// column 1 type
				);
			}

			createStatement(proj, command);
			stepStatement(proj);
			endStatement(proj);

			if (column6 != "") {
				command = cweeStr::printf("INSERT INTO %s (\"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\") VALUES (\"\", \"\", \"\", \"\", \"\", \"\");",
					tableName.c_str(),				// table name
					column1.c_str(),				// column 1 name
					column2.c_str(),				// column 2 name
					column3.c_str(),				// column 3 name
					column4.c_str(),				// column 4 name
					column5.c_str(),				// column 5 name
					column6.c_str()					// column 6 name
				);
			}
			else if (column5 != "") {
				command = cweeStr::printf("INSERT INTO %s (\"%s\", \"%s\", \"%s\", \"%s\", \"%s\") VALUES (\"\", \"\", \"\", \"\", \"\");",
					tableName.c_str(),				// table name
					column1.c_str(),				// column 1 name
					column2.c_str(),				// column 2 name
					column3.c_str(),				// column 3 name
					column4.c_str(),				// column 4 name
					column5.c_str()					// column 5 name
				);
			}
			else if (column4 != "") {
				command = cweeStr::printf("INSERT INTO %s (\"%s\", \"%s\", \"%s\", \"%s\") VALUES (\"\", \"\", \"\", \"\");",
					tableName.c_str(),				// table name
					column1.c_str(),				// column 1 name
					column2.c_str(),				// column 2 name
					column3.c_str(),				// column 3 name
					column4.c_str()
				);
			}
			else if (column3 != "") {
				command = cweeStr::printf("INSERT INTO %s (\"%s\", \"%s\", \"%s\") VALUES (\"\", \"\", \"\");",
					tableName.c_str(),				// table name
					column1.c_str(),				// column 1 name
					column2.c_str(),				// column 2 name
					column3.c_str()
				);
			}
			else if (column2 != "") {
				command = cweeStr::printf("INSERT INTO %s (\"%s\", \"%s\") VALUES (\"\", \"\");",
					tableName.c_str(),				// table name
					column1.c_str(),				// column 1 name
					column2.c_str()
				);
			}
			else {
				command = cweeStr::printf("INSERT INTO %s (\"%s\") VALUES (\"\");",
					tableName.c_str(),				// table name
					column1.c_str()
				);
			}

			createStatement(proj, command);
			stepStatement(proj);
			endStatement(proj);
		}
	};

	static void addNewRowToTable(int proj, const cweeStr& tableName, const cweeStr& data1, const cweeStr& data2, const cweeStr& data3, const cweeStr& data4, const cweeStr& data5, const cweeStr& data6) {
		if (SQLite_databaseMap.find(proj) != SQLite_databaseMap.end()) {
			int numCol = getNumColumns(proj, tableName);
			cweeThreadedList<cweeStr> list;
			for (int i = 0; i < numCol; i++) {
				cweeStr temp = cweeStr::printf("\"%s\"", getColumnName(proj, i, tableName).c_str());
				if (temp != "") list.Append(temp);
			}

			cweeStr listOut;
			for (auto& x : list) listOut.AddToDelimiter(x, ",");

			cweeStr command = cweeStr::printf("INSERT INTO \"%s\"(%s)\nVALUES\n", tableName.c_str(), listOut.c_str());
			if (data6 != "") {
				command += cweeStr::printf("(%s, %s, %s, %s, %s, %s);", data1.c_str(), data2.c_str(), data3.c_str(), data4.c_str(), data5.c_str(), data6.c_str());
			}
			else if (data5 != "") {
				command += cweeStr::printf("(%s, %s, %s, %s, %s);", data1.c_str(), data2.c_str(), data3.c_str(), data4.c_str(), data5.c_str());
			}
			else if (data4 != "") {
				command += cweeStr::printf("(%s, %s, %s, %s);", data1.c_str(), data2.c_str(), data3.c_str(), data4.c_str());
			}
			else if (data3 != "") {
				command += cweeStr::printf("(%s, %s, %s);", data1.c_str(), data2.c_str(), data3.c_str());
			}
			else if (data2 != "") {
				command += cweeStr::printf("(%s, %s);", data1.c_str(), data2.c_str());
			}
			else {
				command += cweeStr::printf("(%s);", data1.c_str());
			}

			createStatement(proj, command);
			stepStatement(proj);
			endStatement(proj);

		}
	};

	static void updateColumnValue(int proj, cweeStr tableName, int columnNum, cweeStr column0Seek, cweeStr newValueAtRowColumnIntersection) {
		if (SQLite_databaseMap.find(proj) != SQLite_databaseMap.end()) {
			cweeStr command = cweeStr::printf("UPDATE \"%s\"\nSET %s = %s\nWHERE %s = %s;",
				tableName.c_str(),
				cweeStr::printf("\"%s\"", getColumnName(proj, columnNum, tableName).c_str()).c_str(),
				newValueAtRowColumnIntersection.c_str(),
				cweeStr::printf("\"%s\"", getColumnName(proj, 0, tableName).c_str()).c_str(),
				column0Seek.c_str()
			);

			createStatement(proj, command);
			stepStatement(proj);
			endStatement(proj);
		}
	};

	static cweeStr getTable(int proj, cweeStr command, int* row, int* column) noexcept {

		if (SQLite_databaseMap.find(proj) != SQLite_databaseMap.end()) {
			cweeStr report;
			char** out;
			char* messageError;
			SQLite_GetTable(SQLite_databaseMap.find(proj)->second, command.c_str(), &out, row, column, &messageError);

			for (int i = 0; i < *row; i++) {
				char* read = out[i + 1]; report += cweeStr((const char*)read); report += '\t';
			}
			return report;
		}
		return cweeStr("");
	};

	static int getNumTables(int proj) noexcept {

		if (SQLite_databaseMap.find(proj) != SQLite_databaseMap.end()) {
			cweeStr report;
			char** out;
			char* messageError;
			int row;
			int column;
			SQLite_GetTable(SQLite_databaseMap.find(proj)->second, "Select name From sqlite_master where type='table';", &out, &row, &column, &messageError);
			return row;
		}
		return -1;
	};

	static cweeStr getTableName(int proj, int whichTable) noexcept {

		if (SQLite_databaseMap.find(proj) != SQLite_databaseMap.end()) {
			cweeStr report;
			char** out;
			char* messageError;
			int row;
			int column;
			SQLite_GetTable(SQLite_databaseMap.find(proj)->second, "Select name From sqlite_master where type='table';", &out, &row, &column, &messageError);
			char* read = out[whichTable + 1]; report = cweeStr((const char*)read);
			return report;
		}
		return cweeStr("");
	};

	static int	getTableNum(int proj, cweeStr tableName) noexcept {
		if (SQLite_databaseMap.find(proj) != SQLite_databaseMap.end()) {
			for (int i = 0; i < getNumTables(proj); i++) {
				if (tableName == getTableName(proj, i)) {
					return i;
				}
			}
		}
		return -1;
	};

	static int getNumRows(int proj, cweeStr tableName) noexcept {
		if (SQLite_databaseMap.find(proj) != SQLite_databaseMap.end()) {
			int row;
			char* messageError;
			cweeStr command = cweeStr::printf("select count(*) from \"%s\"", tableName.c_str());
			execute(proj, command, callback1, &row, &messageError);
			return row;
		}
		return -1;
	};

	static int getNumColumns(int proj, cweeStr tableName) noexcept {
		if (SQLite_databaseMap.find(proj) != SQLite_databaseMap.end()) {
			int col;
			char* messageError;
			cweeStr command = cweeStr::printf("select * from \"%s\" LIMIT 1", tableName.c_str());
			execute(proj, command, callback2, &col, &messageError);
			return col;
		}
		return -1;
	};

	static int getNumValuesInCol(int proj, cweeStr tableName, int tagPath_ID) noexcept {
		if (SQLite_databaseMap.find(proj) != SQLite_databaseMap.end()) {
			int num;
			char* messageError;
			cweeStr command = cweeStr::printf("select count(*) from \"%s\" where TagID = \"%i\";", tableName.c_str(), tagPath_ID);
			execute(proj, command, callback1, &num, &messageError);
			return num;
		}
		return -1;
	};

	static time_t getExtremaColumns(int proj, cweeStr Extrema, cweeStr colName, cweeStr tableName) noexcept {
		if (SQLite_databaseMap.find(proj) != SQLite_databaseMap.end()) {
			time_t val;
			char* messageError;
			cweeStr command = cweeStr::printf("SELECT %s(%s) FROM \"%s\"; ", Extrema.c_str(), colName.c_str(), tableName.c_str());
			execute(proj, command, callbackMIN, &val, &messageError);
			return val;
		};
		return -1;
	};

	static cweeStr getColumnName(int proj, int N, cweeStr tableName) noexcept {
		if (SQLite_databaseMap.find(proj) != SQLite_databaseMap.end()) {
			if (tableName == "") {
				if (SQLite_statementMap.find(proj) != SQLite_statementMap.end()) return cweeStr(SQLite_GetColumnName(SQLite_statementMap.find(proj)->second, N)); // a valid statement is required for this method. 	
				else return "a valid SELECT statement is required for this method";
			}
			else {
				char* messageError;
				cweeStr command = cweeStr::printf("select * from \"%s\" LIMIT 1", tableName.c_str());
				createStatement(proj, command);
				stepStatement(proj);
				if (SQLite_statementMap.find(proj) != SQLite_statementMap.end()) return cweeStr(SQLite_GetColumnName(SQLite_statementMap.find(proj)->second, N));
				endStatement(proj);
			}
		}
		return "no project found";
	};

	static void execute(int proj, cweeStr command, int (*callback)(void*, int, char**, char**), void* returnObj, char** errmsg) noexcept {
		if (SQLite_databaseMap.find(proj) != SQLite_databaseMap.end()) {
			char* messageError;
			SQLite_Exec(SQLite_databaseMap.find(proj)->second, command.c_str(), callback, returnObj, &messageError);
		}
	};

	static void createStatement(int proj, cweeStr command) noexcept {
		// if a statement for this project already exists, remove it first. 
		endStatement(proj);
		if (SQLite_databaseMap.find(proj) != SQLite_databaseMap.end()) {
			sqlite3_stmt* stmt;
			SQLite_Prepare(SQLite_databaseMap.find(proj)->second, command.c_str(), -1, &stmt, NULL);
			SQLite_statementMap.insert(std::make_pair(proj, stmt));
		}
	};

	static int stepStatement(int proj) noexcept {
		if (SQLite_statementMap.find(proj) != SQLite_statementMap.end()) {
			return SQLite_Step(SQLite_statementMap.find(proj)->second);
		}
		return SQLITE_DONE;
	};

	static cweeStr getColumnText(int proj, int whichCol) noexcept {
		if (SQLite_statementMap.find(proj) != SQLite_statementMap.end()) {
			return cweeStr((char*)SQLite_Column_Text(SQLite_statementMap.find(proj)->second, whichCol));
		}
		return cweeStr("");
	};

	static cweeStr getCurrentRow(int proj, int numColumns) noexcept {
		if (SQLite_statementMap.find(proj) != SQLite_statementMap.end()) {
			if (numColumns > -1) {
				// user specified the num columns 
				cweeStr line;
				for (int i = 0; i < numColumns; i++) {
					cweeStr col = cweeStr((char*)SQLite_Column_Text(SQLite_statementMap.find(proj)->second, i));
					line += col; if (i + 1 < numColumns) line += '\t';
				}
				line.ReduceSpaces();
				return line;
			}
			else {
				// user did not specify the num columns 
				cweeStr line; int i = 0; int j = SQLite_Column_Count(SQLite_statementMap.find(proj)->second);
				for (int i = 0; i < j; i++) {
					cweeStr col = cweeStr((char*)SQLite_Column_Text(SQLite_statementMap.find(proj)->second, i));
					line += col; if (i + 1 < j) line += '\t';
				}
				line.ReduceSpaces();
				return line;
			}
		}
		return cweeStr("");
	};

	static cweeThreadedList<cweeStr>  getNextRow(int proj) noexcept {
		cweeThreadedList<cweeStr> out;
		if (SQLite_statementMap.find(proj) != SQLite_statementMap.end())
			if (stepStatement(proj) != SQLITE_DONE) {
				int j = SQLite_Column_Count(SQLite_statementMap.find(proj)->second);
				out.SetGranularity(cweeMath::max(16, j));
				for (int i = 0; i < j; i++)	out.Append(cweeStr((char*)SQLite_Column_Text(SQLite_statementMap.find(proj)->second, i)));
			}

		return out;
	};

	static void endStatement(int proj) noexcept {
		if (SQLite_statementMap.find(proj) != SQLite_statementMap.end()) {
			SQLite_Finalize(SQLite_statementMap.find(proj)->second);
			SQLite_statementMap.erase(proj);
		}
	};

	static void closeDB(int proj) noexcept {
		if (SQLite_databaseMap.find(proj) != SQLite_databaseMap.end()) {
			endStatement(proj);
			SQLite_Close(SQLite_databaseMap.find(proj)->second);
			SQLite_databaseMap.erase(proj);
		}
	};

};

#endif