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
    virtual bool GetNextRow(nanodbcResult& results, cweeThreadedList<double>& out) { return ODBC::GetNextRow(results, out); };

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

    virtual void InsertRow(nanodbcConnection const& con, cweeStr const& tableFullPath, const cweeThreadedList<cweeStr>& values) { return ODBC::InsertRow(con, tableFullPath, values); };

    virtual void InsertRows(nanodbcConnection const& con, cweeStr const& tableFullPath, const cweeThreadedList<cweeThreadedList<cweeStr>>& values) { return ODBC::InsertRows(con, tableFullPath, values); };
};
cweeSharedPtr<cweeODBC> odbc = make_cwee_shared<cweeOdbcLocal>(new cweeOdbcLocal()).CastReference<cweeODBC>();

namespace chaiscript {
    namespace WaterWatch_Lib {
        [[nodiscard]] ModulePtr ODBC_library() {
            auto lib = chaiscript::make_shared<Module>();

            AddBasicClassTemplate(nanodbcConnection);
            lib->AddFunction(, CsvToTable, , SINGLE_ARG({
                // stream the file
                tableName = odbc->SafeString(tableName);

                std::string get;
                bool started = true;


                bool startedTransaction = false;
                int i = 1; int num = 0;
                fileSystem->LockFile(filePath);
                {
                    std::ifstream file(filePath); // ifstream is read only, ofstream is write only, fstream is read/write.
                    while (file.good()) {
                        getline(file, get);
                        num++;
                    }
                    file.close();
                }
                {
                    std::ifstream file(filePath); // ifstream is read only, ofstream is write only, fstream is read/write.
                    cweeParser p;
                    while (file.good()) {
                        getline(file, get);
                        if (started) {
                            if (!odbc->TableExists(con, tableName)) {
                                cweeList<cweeStr> header = cweeStr(get.c_str()).Split(",");
                                for (auto& x : header) x = odbc->SafeString(x);
                                if (!odbc->CreateTable(con, tableName, header)) { throw(std::exception("Could not create the table.")); }
                            }

                            started = false;
                            startedTransaction = true;
                            {
                                AUTO r = odbc->Query(con, "BEGIN DEFERRED TRANSACTION;");
                                odbc->GetResults(r);
                            }
                        }
                        else {
                            p.Parse(get.c_str(), ",", true);
                            p.Trim(' ');
                            odbc->InsertRow(con, tableName, p.getVars());
                            if (++i % (num / 100) == 0) {
                                if (startedTransaction) {
                                    AUTO r = odbc->Query(con, "COMMIT TRANSACTION;");
                                    odbc->GetResults(r);
                                }
                                {
                                    AUTO r = odbc->Query(con, "BEGIN DEFERRED TRANSACTION;");
                                    odbc->GetResults(r);
                                }
                            }
                        }
                    }
                    file.close();
                }
                fileSystem->UnlockFile(filePath);

                if (startedTransaction) {
                    AUTO r = odbc->Query(con, "COMMIT TRANSACTION;");
                    odbc->GetResults(r);
                }

                return tableName;
                }); , nanodbcConnection& con, cweeStr tableName, cweeStr const& filePath);
            lib->add(chaiscript::fun([](cweeStr const& in) { return odbc->SafeString(in); }), "");
            lib->add(chaiscript::fun([](cweeStr const& in) { return odbc->SafeString(in); }), "SafeString"); // adds/removes quotes/database/table names as needed
            lib->add(chaiscript::fun([](cweeStr const& in, nanodbcConnection& con) { return odbc->SafeString(in, &con); }), "SafeString"); // adds/removes quotes/database/table names as needed
            lib->add(chaiscript::fun([](nanodbcConnection& con, cweeStr const& in) { return odbc->SafeString(in, &con); }), "SafeString"); // adds/removes quotes/database/table names as needed
            lib->add(chaiscript::fun([](cweeStr const& Server, cweeStr const& UserID, cweeStr const& Password) { return odbc->CreateConnection(Server, UserID, Password); }), "CreateConnection");
            lib->add(chaiscript::fun([](cweeStr const& filePath) { return odbc->CreateConnection(filePath, "", ""); }), "CreateConnection");
            lib->add(chaiscript::fun([](nanodbcConnection const& con) { return odbc->IsConnected(con); }), "IsConnected");
            lib->add(chaiscript::fun([](nanodbcConnection const& con) { odbc->EndConnection(con); }), "EndConnection");
            lib->add(chaiscript::fun([](nanodbcConnection const& con, const cweeStr& query) {
                AUTO dd = odbc->GetResults(con, query);
                std::vector< chaiscript::Boxed_Value > out;
                out.reserve(dd.size() + 1);
                for (auto& d : dd) {
                    std::vector<chaiscript::Boxed_Value> row;
                    row.reserve(d.size() + 1);
                    for (auto& x : d) {
                        row.push_back(chaiscript::var(std::string(x.c_str())));
                    }
                    out.push_back(chaiscript::var(row));
                }
                return out;
                }), "GetResults");
            lib->add(chaiscript::fun([](nanodbcConnection const& con, const cweeStr& tableName, const cweeThreadedList<cweeStr>& columnNames) { return odbc->CreateTable(con, tableName, columnNames); }), "CreateTable");
            lib->add(chaiscript::fun([](nanodbcConnection const& con, const cweeStr& tableName, const cweeThreadedList<cweeStr>& columnNames, cweeStr databaseName) { return odbc->CreateTable(con, tableName, columnNames, databaseName); }), "CreateTable");
            lib->add(chaiscript::fun([](nanodbcConnection const& con, const cweeStr& in) { return odbc->PartialTableNameToFullTableName(con, in); }), "PartialTableNameToFullTableName");
            lib->add(chaiscript::fun([](nanodbcConnection const& con, const cweeStr& tableName) { return odbc->GetDatabaseName(con, tableName); }), "GetDatabaseName");
            lib->add(chaiscript::fun([](nanodbcConnection const& con, const cweeStr& tableName) { return odbc->TableExists(con, tableName); }), "TableExists");
            lib->add(chaiscript::fun([](nanodbcConnection const& con, const cweeStr& tableFullPath, const cweeThreadedList<cweeStr>& values) { odbc->InsertRow(con, tableFullPath, values); }), "InsertRow");
            lib->add(chaiscript::fun([](nanodbcConnection const& con, const cweeStr& tableFullPath, const cweeThreadedList<cweeThreadedList<cweeStr>>& values) { odbc->InsertRows(con, tableFullPath, values); }), "InsertRows");
            lib->add(chaiscript::fun([](nanodbcConnection const& con, const cweeStr& tableFullPath, const std::vector<cweeThreadedList<cweeStr>>& values) {
                cweeThreadedList<cweeThreadedList<cweeStr>> target(values.size() + 1); for (auto& x : values) {
                    target.Append(x);
                }
                odbc->InsertRows(con, tableFullPath, target);
                }), "InsertRows");
            lib->add(chaiscript::fun([](nanodbcConnection const& con, const cweeStr& tableFullPath, const std::vector<std::vector<cweeStr>>& values) {
                cweeThreadedList<cweeThreadedList<cweeStr>> target(values.size() + 1); for (auto& x : values) {
                    cweeThreadedList<cweeStr> temp(x.size() + 1);
                    for (auto& y : x) {
                        temp.Append(y.c_str());
                    }
                    target.Append(temp);
                }
                odbc->InsertRows(con, tableFullPath, target);
                }), "InsertRows");
            lib->add(chaiscript::fun([](nanodbcConnection const& con, const cweeStr& tableFullPath, const std::vector<std::vector<std::string>>& values) {
                cweeThreadedList<cweeThreadedList<cweeStr>> target(values.size() + 1); for (auto& x : values) {
                    cweeThreadedList<cweeStr> temp(x.size() + 1);
                    for (auto& y : x) {
                        temp.Append(y.c_str());
                    }
                    target.Append(temp);
                }
                odbc->InsertRows(con, tableFullPath, target);
                }), "InsertRows");
            lib->add(chaiscript::fun([](nanodbcConnection const& con, const cweeStr& tableFullPath, const std::vector<Boxed_Value>& rows) {
                cweeThreadedList<cweeThreadedList<cweeStr>> target(rows.size() + 1); for (auto& row_boxed : rows) {
                    auto* row = chaiscript::boxed_cast<std::vector<Boxed_Value>*>(row_boxed);
                    if (!row) throw exception::bad_boxed_cast(row_boxed.get_type_info(), typeid(std::vector<Boxed_Value>));
                    
                    cweeThreadedList<cweeStr> temp(row->size() + 1);
                    for (auto& value_boxed : *row) {
                        {
                            auto* v = chaiscript::boxed_cast<cweeStr*>(value_boxed);
                            if (v) {
                                temp.Append(*v);
                                continue;
                            }
                        }
                        {
                            auto* v = chaiscript::boxed_cast<std::string*>(value_boxed);
                            if (v) {
                                temp.Append(v->c_str());
                                continue;
                            }
                        }
                        {
                            auto* v = chaiscript::boxed_cast<double*>(value_boxed);
                            if (v) {
                                temp.Append(*v);
                                continue;
                            }
                        }
                        throw exception::bad_boxed_cast(value_boxed.get_type_info(), typeid(cweeStr));
                    }
                    target.Append(temp);
                }
                odbc->InsertRows(con, tableFullPath, target);
            }), "InsertRows");
            lib->add(chaiscript::fun([](nanodbcConnection const& con) { AUTO d = odbc->GetDatabaseNames(con); std::vector<chaiscript::Boxed_Value> out; out.reserve(d.size() + 1); for (auto& x : d) { out.push_back(chaiscript::var(std::string(x.c_str()))); } return out; }), "GetDatabaseNames");
            lib->add(chaiscript::fun([](nanodbcConnection const& con) { AUTO d = odbc->GetTableNames(con); std::vector<chaiscript::Boxed_Value> out; out.reserve(d.size() + 1); for (auto& x : d) { out.push_back(chaiscript::var(std::string(x.c_str()))); } return out; }), "GetTableNames");
            lib->add(chaiscript::fun([](nanodbcConnection const& con, const cweeStr& databaseName) { AUTO d = odbc->GetTableNames(con, databaseName); std::vector<chaiscript::Boxed_Value> out; out.reserve(d.size() + 1); for (auto& x : d) { out.push_back(chaiscript::var(std::string(x.c_str()))); } return out; }), "GetTableNames");
            lib->add(chaiscript::fun([](nanodbcConnection const& con, cweeStr const& tableName) { AUTO d = odbc->GetColumnNames(con, tableName); std::vector<chaiscript::Boxed_Value> out; out.reserve(d.size() + 1); for (auto& x : d) { out.push_back(chaiscript::var(std::string(x.c_str()))); } return out; }), "GetColumnNames");
            lib->add(chaiscript::fun([](nanodbcConnection const& con, cweeStr const& tableName, cweeStr const& databaseName) { AUTO d = odbc->GetColumnNames(con, tableName, databaseName); }), "GetColumnNames");
            lib->add(chaiscript::fun([](nanodbcConnection const& con, const cweeStr& tableName, const cweeStr& databaseName) { AUTO d = odbc->GetTableSchema(con, tableName, databaseName); std::vector<chaiscript::Boxed_Value> out; out.reserve(d.size() + 1); for (auto& x : d) { out.push_back(chaiscript::var(std::string(x.c_str()))); } return out; }), "GetTableSchema");
            lib->add(chaiscript::fun([](nanodbcConnection const& con, const cweeStr& tableName) { AUTO d = odbc->GetTableSchema(con, tableName); std::vector<chaiscript::Boxed_Value> out; out.reserve(d.size() + 1); for (auto& x : d) { out.push_back(chaiscript::var(std::string(x.c_str()))); } return out; }), "GetTableSchema");

            AddBasicClassTemplate(nanodbcResult);
            lib->add(chaiscript::fun([](nanodbcConnection const& con, const cweeStr& query) { return odbc->Query(con, query); }), "Query"); // returns a result object that can be streamed
            lib->add(chaiscript::fun([](nanodbcResult& con) {
                AUTO dd = odbc->GetResults(con);
                std::vector< chaiscript::Boxed_Value > out;
                out.reserve(dd.size() + 1);
                for (auto& d : dd) {
                    std::vector<chaiscript::Boxed_Value> row;
                    row.reserve(d.size() + 1);
                    for (auto& x : d) {
                        row.push_back(chaiscript::var(cweeStr(x)));
                    }
                    out.push_back(chaiscript::var(row));
                }
                return out;
            }), "GetResults");
            lib->add(chaiscript::fun([](nanodbcResult& con) {
                return odbc->GetResults(con);
            }), "GetCweeStrResults");
            lib->add(chaiscript::fun([](nanodbcResult& con) { AUTO d = odbc->GetNextRow(con); std::vector<chaiscript::Boxed_Value> out; out.reserve(d.size() + 1); for (auto& x : d) { out.push_back(chaiscript::var(std::string(x.c_str()))); } return out; }), "GetNextRow");
            lib->add(chaiscript::fun([](nanodbcResult& con, cweeList<cweeStr>& row) { return odbc->GetNextRow(con, row); }), "GetNextRow");
            lib->add(chaiscript::fun([](nanodbcResult& con, cweeList<double>& row) { return odbc->GetNextRow(con, row); }), "GetNextRow");

            return lib;
        };
    };
}; // namespace chaiscript