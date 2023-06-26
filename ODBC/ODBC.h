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
#include "../WaterWatchCpp/Precompiled.h"
#include "../WaterWatchCpp/List.h"
#include "../WaterWatchCpp/Strings.h"
#include "../WaterWatchCpp/SharedPtr.h"

class nanodbcConnection {
public:
    nanodbcConnection() : data() {};
    nanodbcConnection(cweeSharedPtr<void> d) : data(d) {};
    cweeSharedPtr<void> data;
};
class nanodbcResult {
public:
    nanodbcResult() : data() {};
    nanodbcResult(cweeSharedPtr<void> d) : data(d) {};
    cweeSharedPtr<void> data;
};

class ODBC {
public:
    /* Connect to ODBC or SQLite database. Server/Username/Password for ODBC, or filepath/""/"" for SQLite. Returns connection handle. */
    static nanodbcConnection CreateConnection(cweeStr Server, cweeStr UserID, cweeStr Password, cweeStr Driver = "") ;
    /* Confirm if connection is valid. */
    static bool IsConnected(nanodbcConnection con);
    /* Optionally ends connection. Forgetting a connection will automatically disconnect. */
    static void EndConnection(nanodbcConnection con);
    /* 
    Sends text query to a database connection. Will need to specialize the query depending on the database engine. 
    Batch size can be an optimization for large multi-row queries. May need to be set to 1 when returning duplicate rows on some database engines. Larger values can make iterating on large queries more efficient.
    Returns a handle to the result.
    */
    static nanodbcResult Query(nanodbcConnection con, const cweeStr& query, int batchSize = 10);
    /* Gets the actual result for the next row in the query. Asking beyond the limit of the result returns an empty vector. */
    static cweeThreadedList<cweeStr> GetNextRow(nanodbcResult& results);
    /* Returns true if successful. Fills the "out" vector with the next row. Highly optimized and suggested path for looping results. Memory stable for even large queries. */
    static bool GetNextRow(nanodbcResult& results, cweeThreadedList<cweeStr>& out);
    /* Get all results from a query, as a row-major matrix. Be careful when calling this on a large query. */
    static cweeThreadedList < cweeThreadedList<cweeStr> > GetResults(nanodbcResult& results);
    /* Get all results from a query, as a row-major matrix. Be careful when calling this on a large query. */
    static cweeThreadedList < cweeThreadedList<cweeStr> > GetResults(nanodbcConnection con, const cweeStr& query, int batchSize = 10);
    /* Get all results from a query placed into the "out" matrix (row-major). Be careful when calling this on a large query. */
    static void GetResults(nanodbcConnection con, const cweeStr& query, cweeThreadedList < cweeThreadedList<cweeStr> >& out, int batchSize = 10);
    /* Get all databases in the current connection. (SQLite has no "databases", but ODBC allows for unlimited number of databases per connection) */
    static cweeThreadedList<cweeStr> GetDatabaseNames(nanodbcConnection con);
    /* Get the list of all tables within a database (leave empty for SQlite or to just get a list of all tables in all databases) */
    static cweeThreadedList<cweeStr> GetTableNames(nanodbcConnection con, const cweeStr& databaseName = "");
    /* Get the list of all columns within a table (may need to specify the database) */
    static cweeThreadedList<cweeStr> GetColumnNames(nanodbcConnection con, cweeStr tableName, const cweeStr& databaseName = "");
    /* Copy the first column from a row-major matrix into a vector. */
    static cweeThreadedList<cweeStr> FirstColumnOnly(const cweeThreadedList < cweeThreadedList<cweeStr> >& results);
    /* Make sure a variable name /table name / etc. is correctly formatted for this connection. Suggested to use this function frequently. For example, adds square brackets where necessary. */
    static cweeStr SafeString(const cweeStr& in, nanodbcConnection* optionalConnectionForTableSchemaCheck = nullptr);
    /* Corrects a tablename to use the full call-out */
    static cweeStr PartialTableNameToFullTableName(nanodbcConnection con, const cweeStr& in);
    /* Gets the schema for a table */
    static cweeThreadedList < cweeStr > GetTableSchema(nanodbcConnection con, const cweeStr& tableName, const cweeStr& databaseName = "");
    /* Create a new table of strings with the given column names. */
    static bool CreateTable(nanodbcConnection con, cweeStr tableName, const cweeThreadedList<cweeStr>& columnNames, cweeStr databaseName = "");
    /* Get the database name for the provided table */
    static cweeStr GetDatabaseName(nanodbcConnection con, cweeStr tableName);
    /* Check if table exists */
    static bool TableExists(nanodbcConnection con, cweeStr tableName);
    /* Insert a row into the existing table */
    static void InsertRow(nanodbcConnection con, cweeStr tableFullPath, const cweeThreadedList<cweeStr>& values);
};