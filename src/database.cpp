#include <iostream>
#include <map>
#include <string>
#include <mysqlx/xdevapi.h>
#include "database.h"
#include "sha512.h"

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
    }
    *n_parts = idx;

    return 0;
}

int query_complaint(int session_id, const char* schema, int* n_complaint, char* who , char* description, int* id) {
    mysqlx::Session* session;
    try { session = sessions.at(session_id); } catch (out_of_range &e) { return -1; }

    mysqlx::Schema schema_ = session->getSchema(schema);

    mysqlx::Table part_table = schema_.getTable("complaint", true);
    auto rows = part_table.select("*").execute();

    int idx = 0;
    for (const auto row : rows) {
        int row_id = row[0].get<int>();
        string row_description = row[2].get<string>();
        string row_who = row[3].get<string>();
        //offset to go to next row, think of 2D array
        memcpy(description + idx*STR_LEN, row_description.c_str(), row_description.size()+1);
        memcpy(who + idx*STR_LEN, row_who.c_str(), row_who.size()+1);
        *(id + idx) = row_id;
        idx++;
    }
    *n_complaint = idx;
    return 0;
}

int query_people(int session_id, const char* schema, int* n_people, char* username , char* name, char* full_name, char* email, char* institute, char* timezone) {
    mysqlx::Session *session;
    try { session = sessions.at(session_id); } catch (out_of_range &e) { return -1; }

    mysqlx::Schema schema_ = session->getSchema(schema);

    mysqlx::Table part_table = schema_.getTable("people", true);
    auto rows = part_table.select("*").execute();

    string row_timezone = "";
    int idx = 0;

    for (const auto row: rows) {
        string row_username = row[0].get<string>();
        string row_name = row[1].get<string>();
        string row_full_name = row[2].get<string>();
        string row_email = row[3].get<string>();
        string row_institute = row[4].get<string>();
        if(!row[6].isNull()) {
        row_timezone = row[6].get<string>();
    }

        memcpy(username + idx*STR_LEN, row_username.c_str(), row_username.size() + 1);
        memcpy(name + idx*STR_LEN, row_name.c_str(), row_name.size() + 1);
        memcpy(full_name + idx*STR_LEN, row_full_name.c_str(), row_full_name.size() + 1);
        memcpy(email + idx*STR_LEN, row_email.c_str(), row_email.size() + 1);
        memcpy(institute + idx*STR_LEN, row_institute.c_str(), row_institute.size() + 1);
        memcpy(timezone + idx*STR_LEN, row_timezone.c_str(), row_timezone.size() + 1);
        idx++;
    }
    *n_people = idx;
    return 0;
}

int query_specific_password(int session_id, const char* schema, const char* username, const char* password) {
    /*
     * Return
     *   0: Password check passed
     *   1: User exists & password failed
     *   2: User does not exist
     */
    mysqlx::Session *session;
    try { session = sessions.at(session_id); } catch (out_of_range &e) { return -1; }

    mysqlx::Schema schema_ = session->getSchema(schema);

    mysqlx::Table part_table = schema_.getTable("people", true);
    auto rows = part_table.select("password").where("username = :name").bind("name", username).execute();
    mysqlx::Row row;
    if ((row = rows.fetchOne())) {
        mysqlx::string s = row[0].get<mysqlx::string>();
        string input = sha512(password);
        return s == input ? 0 : 1;
    }
    return 2;
}

int query_component(int session_id, const char* schema, const char* part, int* n_component, int* id, char* status, char* description, char* serial_number,  char* location) {
    mysqlx::Session* session;
    try { session = sessions.at(session_id); } catch (out_of_range &e) { return -1; }

    mysqlx::Schema schema_ = session->getSchema(schema);

    mysqlx::Table part_table = schema_.getTable("component", true);
    auto rows = part_table.select("id", "status", "description", "serial_number", "location").where("part = :component").bind("component", part).execute();

    string row_status = "";
    string row_description = "";
    string row_serial_number = "";
    string row_location = "";
    int idx = 0;

    for(mysqlx::Row row : rows.fetchAll()) {
        int row_id = row[0].get<int>();
        if(!row[1].isNull()) {
            row_status = row[1].get<string>();
        }
        if(!row[2].isNull()) {
            row_description = row[2].get<string>();
        }
        if(!row[3].isNull()) {
            row_serial_number = row[3].get<string>();
        }
        if(!row[4].isNull()) {
            row_location = row[4].get<string>();
        }

        memcpy(status + idx*STR_LEN, row_status.c_str(), row_status.size() + 1);
        memcpy(description + idx*STR_LEN, row_description.c_str(), row_description.size() + 1);
        memcpy(serial_number + idx*STR_LEN, row_serial_number.c_str(), row_serial_number.size() + 1);
        memcpy(location + idx*STR_LEN, row_location.c_str(), row_location.size() + 1);
        *(id + idx) = row_id;
        idx++;
    }
    *n_component = idx;
    return 0;
}

//int insert_component(int session_id, const char* schema, char* part, char* status, char* description, char* location) {
//    mysqlx::Session* session;
//    try { session = sessions.at(session_id); } catch (out_of_range &e) { return -1; }
//
//    mysqlx::Schema schema_ = session->getSchema(schema);
//
//    mysqlx::Table part_table = schema_.getTable("component", true);
//    auto rows = part_table.insert("part", "status", "description", "location").values(part, status, description, location).execute();
//
//    return 0;
//}

//int update_component(int session_id, const char* schema, char* part, char* status, char* description, char* location) {
//    mysqlx::Session* session;
//    try { session = sessions.at(session_id); } catch (out_of_range &e) { return -1; }
//
//    mysqlx::Schema schema_ = session->getSchema(schema);
//
//    mysqlx::Table part_table = schema_.getTable("component", true);
//    auto rows = part_table.insert("part", "status", "description", "location").values(part, status, description, location).execute();
//
//    return 0;
//}