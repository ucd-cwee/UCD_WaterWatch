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
#include "List.h"
#include "Strings.h"
#include "SharedPtr.h"
#include "../ODBC/ODBC.h"

class cweeODBC {
public:
    virtual nanodbcConnection CreateConnection(cweeStr Server, cweeStr UserID, cweeStr Password, cweeStr Driver = "") = 0;
    virtual bool IsConnected(nanodbcConnection con) = 0;
    virtual void EndConnection(nanodbcConnection con) = 0;
    virtual nanodbcResult Query(nanodbcConnection con, const cweeStr& query, int batchSize = 10) = 0;
    virtual cweeThreadedList<cweeStr> GetNextRow(nanodbcResult& results) = 0;
    virtual bool GetNextRow(nanodbcResult& results, cweeThreadedList<cweeStr>& out) = 0;
    virtual cweeThreadedList < cweeThreadedList<cweeStr> > GetResults(nanodbcResult& results) = 0;
    virtual cweeThreadedList < cweeThreadedList<cweeStr> > GetResults(nanodbcConnection con, const cweeStr& query, int batchSize = 10) = 0;
    virtual void GetResults(nanodbcConnection con, const cweeStr& query, cweeThreadedList < cweeThreadedList<cweeStr> >& out, int batchSize = 10) = 0;
    virtual cweeThreadedList<cweeStr> GetDatabaseNames(nanodbcConnection con) = 0;
    virtual cweeThreadedList<cweeStr> GetTableNames(nanodbcConnection con, const cweeStr& databaseName = "") = 0;
    virtual cweeThreadedList<cweeStr> GetColumnNames(nanodbcConnection con, cweeStr tableName, const cweeStr& databaseName = "") = 0;
    virtual cweeThreadedList<cweeStr> FirstColumnOnly(const cweeThreadedList < cweeThreadedList<cweeStr> >& results) = 0;
    virtual cweeStr SafeString(const cweeStr& in, nanodbcConnection* optionalConnectionForTableSchemaCheck = nullptr) = 0;
    virtual cweeStr PartialTableNameToFullTableName(nanodbcConnection con, const cweeStr& in) = 0;
    virtual cweeThreadedList < cweeStr > GetTableSchema(nanodbcConnection con, const cweeStr& tableName, const cweeStr& databaseName = "") = 0;
    virtual bool CreateTable(nanodbcConnection con, cweeStr tableName, const cweeThreadedList<cweeStr>& columnNames, cweeStr databaseName = "") = 0;
    virtual cweeStr GetDatabaseName(nanodbcConnection con, cweeStr tableName) = 0;
    virtual bool TableExists(nanodbcConnection con, cweeStr tableName) = 0;
    virtual void InsertRow(nanodbcConnection con, cweeStr tableFullPath, const cweeThreadedList<cweeStr>& values) = 0;
};
/*!
geocoding and mapping services
*/
extern cweeSharedPtr<cweeODBC> odbc;