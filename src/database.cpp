#include <iostream>
#include <iomanip>
#include <map>
#include <string>
#include <mysqlx/xdevapi.h>
#include "database.h"
#include "sha512.h"

using namespace std;

#define with_default(c, default, type) (c).isNull() ? default : (c).get<type>()
#define cp_to_buffer(buf, str) memcpy((buf) + (idx)*DB_STR_LEN, (str).c_str(), (str).size()+1 > DB_STR_LEN ? DB_STR_LEN : (str).size()+1)


map<int, mysqlx::Session*> sessions;
int session_cntr = 0;

std::string get_current_utc_timestamp() {
    using namespace std;
    using namespace std::chrono;
    std::time_t now = system_clock::to_time_t(system_clock::now());
    stringstream ss;
    ss << put_time(std::gmtime(&now), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

int connect(const char* username, const char* password, const char* url, int port) {
    stringstream conn_str;
    conn_str << "mysqlx://" << username << ":" << password << "@";

    if (url == nullptr) conn_str << "localhost";
    else conn_str << url;

    if (port < 0) conn_str << ":" << 33060;
    else conn_str << ":" << port;

    std::cout << "Connecting to db " << conn_str.str() << std::endl;
    mysqlx::Session* session;
    try {
        session = new mysqlx::Session(conn_str.str().c_str());
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

int start_transaction(int session_id) {
    mysqlx::Session* session;
    try { session = sessions.at(session_id); } catch (out_of_range &e) { return -1; }

    try { session->startTransaction(); } catch (exception &e) { return -1; }
    return 0;
}

int rollback_transaction(int session_id) {
    mysqlx::Session* session;
    try { session = sessions.at(session_id); } catch (out_of_range &e) { return -1; }
    try { session->rollback(); } catch (exception &e) { return -1; }
    return 0;
}

int finish_transaction(int session_id) {
    mysqlx::Session* session;
    try { session = sessions.at(session_id); } catch (out_of_range &e) { return -1; }
    try { session->commit(); } catch (exception &e) { return -1; }
    return 0;
}

int get_schemas(int session_id, int* n_schemas, char* schemas) {
    mysqlx::Session* session;
    try { session = sessions.at(session_id); } catch (out_of_range &e) { return -1; }

    auto schemas_ = session->getSchemas();
    int idx = 0;
    for (const auto& schema : schemas_) {
        if (idx == DB_ARR_LEN) break;
        string s = schema.getName();
        cp_to_buffer(schemas, s);
        idx++;
    }
    *n_schemas = idx;
    return 0;
}

int query_parts(int session_id, const char* schema, int* n_parts, char* part, int* version, char* description,
                char* prefix, float* dim_x, float* dim_y, float* dim_z, char* type) {
    mysqlx::Session* session;
    try { session = sessions.at(session_id); } catch (out_of_range &e) { return -1; }

    mysqlx::Schema schema_ = session->getSchema(schema);

    mysqlx::Table part_table = schema_.getTable("parts", true);
    auto rows = part_table.select("*").execute();

    int idx = 0;
    for (const auto row : rows) {
        if (idx >= DB_ARR_LEN) break;
        string row_part = with_default(row[0], "", string);
        int row_version = with_default(row[1], -1, int);
        string row_description = with_default(row[2], "", string);
        string row_prefix = with_default(row[3], "", string);
        float row_dim_x = with_default(row[4], -1, float);
        float row_dim_y = with_default(row[5], -1, float);
        float row_dim_z = with_default(row[6], -1, float);
        string row_type = with_default(row[7], "", string);

        cp_to_buffer(part, row_part);
        *(version + idx) = row_version;
        cp_to_buffer(description, row_description);
        cp_to_buffer(prefix, row_prefix);
        *(dim_x + idx) = row_dim_x;
        *(dim_y + idx) = row_dim_y;
        *(dim_z + idx) = row_dim_z;
        cp_to_buffer(type, row_type);

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
        if (idx >= DB_ARR_LEN) break;

        int row_id = with_default(row[0], -1, int);
        string row_description = with_default(row[2], "", string);
        string row_who = with_default(row[3], "", string);

        *(id + idx) = row_id;
        cp_to_buffer(description, row_description);
        cp_to_buffer(who, row_who);

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

    int idx = 0;
    for (const auto row: rows) {
        if (idx >= DB_ARR_LEN) break;

        string row_username = with_default(row[0], "", string);
        string row_name = with_default(row[1], "", string);
        string row_full_name = with_default(row[2], "", string);
        string row_email = with_default(row[3], "", string);
        string row_institute = with_default(row[4], "", string);
        string row_timezone = with_default(row[6], "", string);

        cp_to_buffer(username, row_username);
        cp_to_buffer(name, row_name);
        cp_to_buffer(full_name, row_full_name);
        cp_to_buffer(email, row_email);
        cp_to_buffer(institute, row_institute);
        cp_to_buffer(timezone, row_timezone);

        idx++;
    }
    *n_people = idx;
    return 0;
}

int check_login(int session_id, const char* schema, const char* username, const char* password) {
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

int query_components(int session_id, const char* schema, const char* part, int* n_component, int* id, char* status, char* description, char* serial_number,  char* location) {
    mysqlx::Session* session;
    try { session = sessions.at(session_id); } catch (out_of_range &e) { return -1; }

    mysqlx::Schema schema_ = session->getSchema(schema);

    mysqlx::Table part_table = schema_.getTable("component", true);
    auto rows = part_table.select("id", "status", "description", "serial_number", "location").where("part = :part").bind("part", part).execute();

    int idx = 0;
    for(mysqlx::Row row : rows.fetchAll()) {
        if (idx >= DB_ARR_LEN) break;

        int row_id = with_default(row[0], -1, int);
        string row_status = with_default(row[1], "", string);
        string row_description = with_default(row[1], "", string);
        string row_serial_number = with_default(row[1], "", string);
        string row_location = with_default(row[1], "", string);

        *(id + idx) = row_id;
        cp_to_buffer(status, row_status);
        cp_to_buffer(description, row_description);
        cp_to_buffer(serial_number, row_serial_number);
        cp_to_buffer(location, row_location);

        idx++;
    }
    *n_component = idx;
    return 0;
}

int insert_component(int session_id, const char* schema, const char* part, int version, const char* status,
                     const char* description, const char* who, const char* serial_number, const char* location) {
    mysqlx::Session* session;
    try { session = sessions.at(session_id); } catch (out_of_range &e) { return -1; }

    mysqlx::Schema schema_ = session->getSchema(schema);

    mysqlx::Table component_table = schema_.getTable("component", true);

    string creation_time = get_current_utc_timestamp();
    auto result = component_table.insert("part", "version", "status", "description", "who", "serial_number", "creation_time", "location")
            .values(part, version, status, description, who, serial_number, creation_time.c_str(), location).execute();
    return (int) result.getAutoIncrementValue();
}

int remove_component(int session_id, const char* schema, int component_id) {
    mysqlx::Session* session;
    try { session = sessions.at(session_id); } catch (out_of_range &e) { return -1; }

    mysqlx::Schema schema_ = session->getSchema(schema);
    mysqlx::Table component_table = schema_.getTable("component", true);
    component_table.remove().where("id = :id").bind("id", component_id).execute();
    return 0;
}

int insert_log(int session_id, const char* schema, const char* userid, const char* remote_ip, const char* type) {
    mysqlx::Session* session;
    try { session = sessions.at(session_id); } catch (out_of_range &e) { return -1; }

    mysqlx::Schema schema_ = session->getSchema(schema);
    mysqlx::Table part_table = schema_.getTable("logs", true);
    string now = get_current_utc_timestamp();

    auto result = part_table.insert("userid", "remote_ip", "type", "date")
            .values(userid, remote_ip, type, now.c_str()).execute();
    return (int) result.getAutoIncrementValue();
}

int update_component(int session_id, const char* schema, int id, const char* status, int parent) {
    mysqlx::Session* session;
    try { session = sessions.at(session_id); } catch (out_of_range &e) { return -1; }
    mysqlx::Schema schema_ = session->getSchema(schema);
    mysqlx::Table part_table = schema_.getTable("component", true);

    string now = get_current_utc_timestamp();

    auto rows = part_table.update()
            .set("parent", parent)
            .set("status", status)
            .set("creation_time", now.c_str())
            .where("id = :i")
            .bind("i", id)
            .execute();

    return 0;
}

int insert_test(int session_id, const char *schema, const char *description, const char *data, int component_id,
                const char *type) {
    mysqlx::Session *session;
    try { session = sessions.at(session_id); } catch (out_of_range &e) { return -1; }
    mysqlx::Schema schema_ = session->getSchema(schema);
    mysqlx::Table test_table = schema_.getTable("tests", true);

    string now = get_current_utc_timestamp();
    test_table.insert("date", "description", "data", "part_id", "type")
            .values(now, description, data, component_id, type).execute();

    return 0;
}
