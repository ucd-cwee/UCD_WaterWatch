#pragma hdrstop
#include "SQLite.h"

cweeSysInterlockedInteger			SQLite_numSQLiteAccess;
std::map<int, sqlite3*>				SQLite_databaseMap;
std::map<int, sqlite3_stmt*>		SQLite_statementMap;

int SQLite::openDbDirect(cweeStr fullPath) noexcept {
	sqlite3* DB;
	DB = SQLite_Open(fullPath.c_str());
	auto temp = SQLite_numSQLiteAccess.Increment();
	SQLite_databaseMap.insert(std::make_pair(temp, DB));
	return temp;
}

int SQLite::openDB(cweeStr folderPath, cweeStr fileName) noexcept {
	cweeStr filePath = folderPath + "\\" + fileName + ".db";
	sqlite3* DB;
	DB = SQLite_Open(filePath.c_str());
	auto temp = SQLite_numSQLiteAccess.Increment();
	SQLite_databaseMap.insert(std::make_pair(temp, DB));
	return temp;
}

void SQLite::addTable(int proj, const cweeStr& tableName, const cweeStr& column1, const cweeStr& column1Type, const cweeStr& column2, const cweeStr& column2Type, const cweeStr& column3, const cweeStr& column3Type, const cweeStr& column4, const cweeStr& column4Type, const cweeStr& column5, const cweeStr& column5Type, const cweeStr& column6, const cweeStr& column6Type) {

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
}

void SQLite::addTable(int proj, const cweeStr& tableName, const cweeThreadedList<cweeStr>& columnNames, const cweeThreadedList < cweeThreadedList<cweeStr> >& data) {
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
			if ((data.Num() > 0) && (data[0].Num() == columnNames.Num())){
				command.Clear(); add.Clear();
				cweeStr columns;
				cweeStr values;

				for (auto& col : columnNames) {
					columns.AddToDelimiter(
						cweeStr::printf("\"%s\"", col.c_str()),
						", "
					);
				}

				SQLite::execute(proj, "BEGIN TRANSACTION", NULL, NULL, NULL);
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
				SQLite::execute(proj, "END TRANSACTION", NULL, NULL, NULL);
			}
		}
	}
}

void SQLite::addData(int proj, const cweeStr& tableName, const cweeThreadedList<cweeStr>& columnNames, const cweeThreadedList < cweeThreadedList<cweeStr> >& data, const bool useTransaction) {
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

			if (useTransaction) SQLite::execute(proj, "BEGIN TRANSACTION", NULL, NULL, NULL);
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
			if (useTransaction) SQLite::execute(proj, "END TRANSACTION", NULL, NULL, NULL);
		}	
	}
}

void SQLite::addData(int proj, const cweeStr& tableName, const cweeThreadedList<cweeStr>& columnNames, const cweeThreadedList<cweeStr>& data, const bool useTransaction) {
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

			if (useTransaction) SQLite::execute(proj, "BEGIN TRANSACTION", NULL, NULL, NULL);
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
			if (useTransaction) SQLite::execute(proj, "END TRANSACTION", NULL, NULL, NULL);
		}
	}
}

void SQLite::addTableNonUnique(int proj, const cweeStr& tableName, const cweeStr& column1, const cweeStr& column1Type, const cweeStr& column2, const cweeStr& column2Type, const cweeStr& column3, const cweeStr& column3Type, const cweeStr& column4, const cweeStr& column4Type, const cweeStr& column5, const cweeStr& column5Type, const cweeStr& column6, const cweeStr& column6Type) {

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
}

void SQLite::addNewRowToTable(int proj, const cweeStr& tableName, const cweeStr& data1, const cweeStr& data2, const cweeStr& data3, const cweeStr& data4, const cweeStr& data5, const cweeStr& data6) {
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
}

void SQLite::updateColumnValue(int proj, cweeStr tableName, int columnNum, cweeStr column0Seek, cweeStr newValueAtRowColumnIntersection) {
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
}

cweeStr SQLite::getTable(int proj, cweeStr command, int* row, int* column) noexcept {

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
}

int SQLite::getNumTables(int proj) noexcept {

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
}

cweeStr SQLite::getTableName(int proj, int whichTable) noexcept {

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
}

int	SQLite::getTableNum(int proj, cweeStr tableName) noexcept {
	if (SQLite_databaseMap.find(proj) != SQLite_databaseMap.end()) {
		for (int i = 0; i < getNumTables(proj); i++) {
			if (tableName == getTableName(proj, i)) {				
				return i;
			}
		}
	}
	return -1;
}

int SQLite::getNumRows(int proj, cweeStr tableName) noexcept {
	if (SQLite_databaseMap.find(proj) != SQLite_databaseMap.end()) {
		int row;
		char* messageError;
		cweeStr command = cweeStr::printf("select count(*) from \"%s\"", tableName.c_str());
		execute(proj, command, callback1, &row, &messageError);
		return row;
	}
	return -1;
}

int SQLite::getNumColumns(int proj, cweeStr tableName) noexcept {
	if (SQLite_databaseMap.find(proj) != SQLite_databaseMap.end()) {
		int col;
		char* messageError;
		cweeStr command = cweeStr::printf("select * from \"%s\" LIMIT 1", tableName.c_str());
		execute(proj, command, callback2, &col, &messageError);
		return col;
	}
	return -1;
}

int SQLite::getNumValuesInCol(int proj, cweeStr tableName, int tagPath_ID) noexcept {
	if (SQLite_databaseMap.find(proj) != SQLite_databaseMap.end()) {
		int num;
		char* messageError;
		cweeStr command = cweeStr::printf("select count(*) from \"%s\" where TagID = \"%i\";", tableName.c_str(), tagPath_ID);
		execute(proj, command, callback1, &num, &messageError);
		return num;
	}
	return -1;
}

time_t SQLite::getExtremaColumns(int proj, cweeStr Extrema, cweeStr colName, cweeStr tableName) noexcept {
	if (SQLite_databaseMap.find(proj) != SQLite_databaseMap.end()) {
		time_t val;
		char* messageError;
		cweeStr command = cweeStr::printf("SELECT %s(%s) FROM \"%s\"; ", Extrema.c_str(), colName.c_str(), tableName.c_str());
		execute(proj, command, callbackMIN, &val, &messageError);
		return val;
	};
	return -1;
}

cweeStr SQLite::getColumnName(int proj, int N, cweeStr tableName) noexcept {
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
}

void SQLite::execute(int proj, cweeStr command, int (*callback)(void*, int, char**, char**), void* returnObj, char** errmsg) noexcept {
	if (SQLite_databaseMap.find(proj) != SQLite_databaseMap.end()) {
		char* messageError;
		SQLite_Exec(SQLite_databaseMap.find(proj)->second, command.c_str(), callback, returnObj, &messageError);
	}
}

void SQLite::createStatement(int proj, cweeStr command) noexcept {
	// if a statement for this project already exists, remove it first. 
	endStatement(proj);
	if (SQLite_databaseMap.find(proj) != SQLite_databaseMap.end()) {
		sqlite3_stmt* stmt;
		SQLite_Prepare(SQLite_databaseMap.find(proj)->second, command.c_str(), -1, &stmt, NULL);
		SQLite_statementMap.insert(std::make_pair(proj, stmt));
	}
}

int SQLite::stepStatement(int proj) noexcept {
	if (SQLite_statementMap.find(proj) != SQLite_statementMap.end()) {		
		return SQLite_Step(SQLite_statementMap.find(proj)->second);
	}
	return SQLITE_DONE;
}

cweeStr SQLite::getColumnText(int proj, int whichCol) noexcept {
	if (SQLite_statementMap.find(proj) != SQLite_statementMap.end()) {
		return cweeStr((char*)SQLite_Column_Text(SQLite_statementMap.find(proj)->second, whichCol));
	}
	return cweeStr("");
}

cweeStr SQLite::getCurrentRow(int proj, int numColumns) noexcept {
	if (SQLite_statementMap.find(proj) != SQLite_statementMap.end()) {
		if (numColumns > -1) {
			// user specified the num columns 
			cweeStr line;
			for (int i = 0; i < numColumns; i++) {
				cweeStr col = cweeStr((char*)SQLite_Column_Text(SQLite_statementMap.find(proj)->second, i));
				line += col; if (i+1 < numColumns) line += '\t';
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
}

cweeThreadedList<cweeStr>  SQLite::getNextRow(int proj) noexcept {
	cweeThreadedList<cweeStr> out;
	if (SQLite_statementMap.find(proj) != SQLite_statementMap.end()) 
		if (stepStatement(proj) != SQLITE_DONE) {
			int j = SQLite_Column_Count(SQLite_statementMap.find(proj)->second); 
			out.SetGranularity(cweeMath::max(16, j));
			for (int i = 0; i < j; i++)	out.Append(cweeStr((char*)SQLite_Column_Text(SQLite_statementMap.find(proj)->second, i)));		
		}
	
	return out;
}

void SQLite::endStatement(int proj) noexcept {
	if (SQLite_statementMap.find(proj) != SQLite_statementMap.end()) {
		SQLite_Finalize(SQLite_statementMap.find(proj)->second);
		SQLite_statementMap.erase(proj);
	}
}

void SQLite::closeDB(int proj) noexcept {
	if (SQLite_databaseMap.find(proj) != SQLite_databaseMap.end()) {
		endStatement(proj);
		SQLite_Close(SQLite_databaseMap.find(proj)->second);
		SQLite_databaseMap.erase(proj);
	}
}
