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
#include "odbc.h"
#include "Parser.h"
#include "cwee_math.h"
#include "FileSystemH.h"

class cweeOdbcLocal : public cweeODBC {
public:
    /*! Connect to a remote or locally hosted SQL server (requires username and password) or local SQLite database (with full filepath in server slot). */
    virtual nanodbcConnection CreateConnection(cweeStr Server, cweeStr UserID, cweeStr Password, cweeStr Driver) { return ODBC::CreateConnection(Server, UserID, Password, Driver); };

    virtual bool IsConnected(nanodbcConnection con) { return ODBC::IsConnected(con); };

    virtual void EndConnection(nanodbcConnection con) { return ODBC::EndConnection(con); };

    virtual nanodbcResult Query(nanodbcConnection con, const cweeStr& query, int batchSize) { return ODBC::Query(con, query, batchSize); };

    virtual cweeThreadedList<cweeStr> GetNextRow(nanodbcResult& results) { return ODBC::GetNextRow(results); };

    virtual bool GetNextRow(nanodbcResult& results, cweeThreadedList<cweeStr>& out) { return ODBC::GetNextRow(results, out); };

    virtual cweeThreadedList < cweeThreadedList<cweeStr> > GetResults(nanodbcResult& results) { return ODBC::GetResults(results); };

    virtual cweeThreadedList < cweeThreadedList<cweeStr> > GetResults(nanodbcConnection con, const cweeStr& query, int batchSize) { return ODBC::GetResults(con, query, batchSize); };

    virtual void GetResults(nanodbcConnection con, const cweeStr& query, cweeThreadedList < cweeThreadedList<cweeStr> >& out, int batchSize) { return ODBC::GetResults(con, query, out, batchSize); };

    virtual cweeThreadedList<cweeStr> GetDatabaseNames(nanodbcConnection con) { return ODBC::GetDatabaseNames(con); };

    virtual cweeThreadedList<cweeStr> GetTableNames(nanodbcConnection con, const cweeStr& databaseName = "") { return ODBC::GetTableNames(con, databaseName); };

    virtual cweeThreadedList<cweeStr> GetColumnNames(nanodbcConnection con, cweeStr tableName, const cweeStr& databaseName = "") { return ODBC::GetColumnNames(con, tableName, databaseName); };

    virtual cweeThreadedList<cweeStr> FirstColumnOnly(const cweeThreadedList < cweeThreadedList<cweeStr> >& results) { return ODBC::FirstColumnOnly(results); };

    virtual cweeStr SafeString(const cweeStr& in, nanodbcConnection* optionalConnectionForTableSchemaCheck = nullptr) { return ODBC::SafeString(in, optionalConnectionForTableSchemaCheck); };

    virtual cweeStr PartialTableNameToFullTableName(nanodbcConnection con, const cweeStr& in) { return ODBC::PartialTableNameToFullTableName(con, in); };

    virtual cweeThreadedList < cweeStr > GetTableSchema(nanodbcConnection con, const cweeStr& tableName, const cweeStr& databaseName = "") { return ODBC::GetTableSchema(con, tableName, databaseName); };

    virtual bool CreateTable(nanodbcConnection con, cweeStr tableName, const cweeThreadedList<cweeStr>& columnNames, cweeStr databaseName = "") { return ODBC::CreateTable(con, tableName, columnNames, databaseName); };

    virtual cweeStr GetDatabaseName(nanodbcConnection con, cweeStr tableName) { return ODBC::GetDatabaseName(con, tableName); };

    virtual bool TableExists(nanodbcConnection con, cweeStr tableName) { return ODBC::TableExists(con, tableName); };

    virtual void InsertRow(nanodbcConnection con, cweeStr tableFullPath, const cweeThreadedList<cweeStr>& values) { return ODBC::InsertRow(con, tableFullPath, values); };
};
cweeSharedPtr<cweeODBC> odbc = make_cwee_shared<cweeOdbcLocal>(new cweeOdbcLocal()).CastReference<cweeODBC>();