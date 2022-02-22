#include <iostream>
#include <map>
#include <string>
#include <mysqlx/xdevapi.h>
#include "database.h"

using namespace std;

#define STR_LEN 128
#define ARR_LEN 128

map<int, mysqlx::Session*> sessions;
int session_cntr = 0;

int connect(const char* connection_string) {
    std::cout << "Connecting to db " << connection_string << std::endl;
    mysqlx::Session* session;
    try {
        session = new mysqlx::Session(connection_string);
        sessions[session_cntr] = session;
        session_cntr++;
        return session_cntr - 1;
    } catch(std::exception &e){
        std::cout << e.what() << std::endl;
        std::cout << "Failed to connect" << std::endl;
        return -1;
    }
}

int disconnect(int session_id) {
    mysqlx::Session* session;
    try { session = sessions.at(session_id); } catch (out_of_range &e) { return -1; }
    session->close();
    sessions.erase(session_id);
    return 0;
}

int get_schemas(int session_id, int* n_schemas, char* schemas) {
    mysqlx::Session* session;
    try { session = sessions.at(session_id); } catch (out_of_range &e) { return -1; }

    auto schemas_ = session->getSchemas();
    int idx = 0;
    for (const auto& schema : schemas_) {
        if (idx == ARR_LEN) break;
        string s = schema.getName();
        if (s.size() >= STR_LEN) continue;
        memcpy(schemas+idx*STR_LEN, s.c_str(), s.size()+1);
        idx++;
    }
    *n_schemas = idx;
    return 0;
}


int query_parts(int session_id, const char* schema, int* n_parts, char* part, int* version) {
    mysqlx::Session* session;
    try { session = sessions.at(session_id); } catch (out_of_range &e) { return -1; }

    mysqlx::Schema schema_ = session->getSchema(schema);

    mysqlx::Table part_table = schema_.getTable("parts", true);
    auto rows = part_table.select("*").execute();

    int idx = 0;
    for (const auto row : rows) {
        string row_part = row[0].get<string>();
        int row_version = row[1].get<int>();
        memcpy(part + idx*STR_LEN, row_part.c_str(), row_part.size()+1);
        *(version + idx) = row_version;
        idx++;
        cout << part << endl;
    }
    *n_parts = idx;

    return 0;
}