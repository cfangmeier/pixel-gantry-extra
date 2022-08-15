//
// Created by Caleb on 2/15/2022.
//
#include <iostream>
#include "database.h"
#include "sha512.h"

using namespace std;

#define TEST(test_name, ...) {\
        std::cout << "Running Test: " << #test_name << "...  " << std::flush;\
        int code=test_name(__VA_ARGS__);                                     \
        if (code == 0) std::cout << "PASSED!" << std::endl;                  \
        else cout << "FAILURE! Exit-Code=" << code << std::endl;             \
    }

bool VERBOSE = false;

int test_update_component(int session_id, const char* schema) {

    int comp_id = insert_component(session_id, schema, "test_part", 1,
                                   "test", "This is a test component", "Mr. Test",
                                   "TEST-001", "testsville");

    str_table_t status;
    str_table_t location;
    str_table_t description;
    str_table_t serial_number;
    int_table_t id;
    int n_component;

    update_component(session_id, schema, comp_id, "updated", 2);
    query_components(session_id, schema, "test_part", &n_component, (int *) id,
                     (char *) status, (char *) description, (char *) serial_number, (char *) location);
    remove_component(session_id, schema, comp_id);

    bool success = false;
    for(int i=0; i<n_component; i++) {
        string comp_status = status[i];
        if (VERBOSE) {
            bool match = (comp_status == "updated");
            cout << "\"" << comp_status << R"(" == "updated")" << "? " << match << endl;
        }
        if (id[i] == comp_id && comp_status == "updated") {
            success = true;
            break;
        }
    }
    return success ? 0 : 1;
}

int test_query_complaint(int session_id, const char* schema) {
    int n_complaint;
    str_table_t who;
    str_table_t description;
    int_table_t id;

    query_complaint(session_id, schema, &n_complaint, (char*) who, (char*) description, (int*) id);
    if (VERBOSE) {
        cout << endl;
        for(int i=0; i<n_complaint; i++) {
            cout << who[i] << ", " << description[i] <<  ", " << id[i] << endl;
        }
    }
    return n_complaint > 0 ? 0 : 1;
}

int test_query_parts(int session_id, const char* schema) {
    int n_parts;
    str_table_t part;
    int_table_t version;
    str_table_t description;
    str_table_t prefix;
    float_table_t dim_x;
    float_table_t dim_y;
    float_table_t dim_z;
    str_table_t type;
    query_parts(session_id, schema, &n_parts, (char*) part, (int*) version, (char*) description, (char*) prefix,
                (float*) dim_x, (float*) dim_y,(float*) dim_z, (char*) type);
    if (VERBOSE) {
        for(int i=0; i<n_parts; i++) {
            cout << (char*) part[i] << ", " << version[i] << ", " << description[i]<< ", " << prefix[i] << endl;
        }
    }
    return n_parts > 0 ? 0 : 1;
}

int test_query_people(int session_id, const char* schema) {
    int n_people;
    str_table_t username;
    str_table_t name;
    str_table_t full_name;
    str_table_t email;
    str_table_t institute;
    str_table_t timezone;

    query_people(session_id, schema, &n_people, (char*) username , (char*) name,
                 (char*) full_name, (char*) email, (char*) institute, (char*) timezone);
    if (VERBOSE) {
        cout << endl;
        for(int i=0; i<n_people; i++) {
            cout << username[i]
            << ", " << name[i]
            << ", " << full_name[i]
            << ", " << email[i]
            << ", " << institute[i]
            << ", " << timezone[i] << endl;
        }
    }
    return n_people > 0 ? 0 : 1;
}

int test_check_login(int session_id, const char* schema, const char* username, const char* password) {
    int password_check_result = check_login(session_id, schema, username, password);
    return password_check_result;
}

int test_query_components(int session_id, const char* schema, const char* part) {
    str_table_t status;
    str_table_t location;
    str_table_t description;
    str_table_t serial_number;
    int_table_t id;
    int n_component;

    query_components(session_id, schema, part, &n_component, (int*) id ,
                    (char*) status, (char*) description, (char*) serial_number,(char*) location);
    if (VERBOSE) {
        for(int i = 0; i < n_component; i++) {
            cout << id[i]
            << ", " << status[i]
            << ", " <<  description[i]
            << ", " <<  serial_number[i]
            << ", " <<  location[i]
            << endl;
        }
    }
    return n_component > 0 ? 0 : 1;
}

int test_insert_component(int session_id, const char* schema) {

    const char* test_part = "test_part";
    int test_version = 1;
    const char* test_status = "test";
    const char* test_description = "This is a test component";
    const char* test_who = "Mr. Test";
    const char* test_serial_number = "TEST-001";
    const char* test_location = "testsville";

    str_table_t status;
    str_table_t location;
    str_table_t description;
    str_table_t serial_number;
    int_table_t id;
    int n_component_pre_insert, n_component_post_insert;

    query_components(session_id, schema, test_part, &n_component_pre_insert, (int*) id ,
                     (char*) status, (char*) description, (char*) serial_number,(char*) location);

    insert_component(session_id, schema, test_part, test_version,
                     test_status, test_description, test_who,
                     test_serial_number, test_location);

    query_components(session_id, schema, test_part, &n_component_post_insert, (int*) id ,
                     (char*) status, (char*) description, (char*) serial_number,(char*) location);

    if (VERBOSE) {
        cout << endl;
        cout << "Components Pre-Insert: " << n_component_pre_insert << endl;
        cout << "Components Post-Insert: " << n_component_post_insert << endl;
    }
    if (n_component_post_insert - n_component_pre_insert == 1) return 0;
    else return 1;
}

int test_remove_component(int session_id, const char* schema) {

    const char* test_part = "test_part";
    int test_version = 1;
    const char* test_status = "test";
    const char* test_description = "This is a test component";
    const char* test_who = "Mr. Test";
    const char* test_serial_number = "TEST-001";
    const char* test_location = "testsville";

    str_table_t status;
    str_table_t location;
    str_table_t description;
    str_table_t serial_number;
    int_table_t id;
    int n_component_pre_remove, n_component_post_remove;

    query_components(session_id, schema, test_part, &n_component_pre_remove, (int*) id ,
                     (char*) status, (char*) description, (char*) serial_number,(char*) location);

    int component_id;
    if (n_component_pre_remove == 0) {
        component_id = insert_component(session_id, schema, test_part, test_version,
                                        test_status, test_description, test_who,
                                        test_serial_number, test_location);
        n_component_pre_remove = 1;

    } else {
        component_id = id[0];
    }
    remove_component(session_id, schema, component_id);

    query_components(session_id, schema, test_part, &n_component_post_remove, (int*) id ,
                     (char*) status, (char*) description, (char*) serial_number,(char*) location);

    if (VERBOSE) {
        cout << endl;
        cout << "Components Pre-Remove: " << n_component_pre_remove << endl;
        cout << "Components Post-Remove: " << n_component_post_remove << endl;
    }
    if (n_component_post_remove - n_component_pre_remove == -1) return 0;
    else return 1;
}


int test_insert_log(int session_id, const char* schema) {
    string userid = "test user";
    string remote_ip = "1.2.3.4";
    string type = "test log";

    insert_log(session_id, schema, userid.c_str(), remote_ip.c_str(), type.c_str());
    return 0;
}

int test_get_schemas(int session_id) {
    str_table_t schemas;
    int n_schemas;
    get_schemas(session_id, &n_schemas, (char*) schemas);

    if (VERBOSE) {
        cout << endl;
        for(int i=0; i<n_schemas; i++){
            cout << "\t" << schemas[i] << endl;
        }
    }
    return n_schemas > 0 ? 0 : 1;
}

int test_insert_test(int session_id, const char* schema) {
    string userid = "test user";
    string remote_ip = "1.2.3.4";
    string type = "test log";

    insert_test(session_id, schema, "This is a test test", "dx=5,dy=12,dz=92,t=1h23m,etc,etc", 68, "test");
    return 0;
}

int main(int argc, const char** argv) {
    VERBOSE = true;
    std::cout << "Program Started" << std::endl;
//    int session_id = connect("root", "carriker", nullptr, -1);
    const char* username = getenv("DB_USERNAME");
    const char* password = getenv("DB_PASSWORD");
    const char* url = getenv("DB_URL");
    const char* port = getenv("DB_PORT");
    if (username == nullptr) username = "root";
    if (password == nullptr) password = "password";
    if (url == nullptr) url = "localhost";
    if (port == nullptr) port = "33060";
    int port_ = atoi(port);


    int session_id = connect(username, password, url, port_);
    if (session_id < 0) return 0;

    TEST(test_get_schemas, session_id)
//    TEST(test_check_login, session_id, "cmsfpix_phase2", "amironov", "blueberries")
//    TEST(test_query_components, session_id, "cmsfpix_phase2", "rd53a_chip");
//    TEST(test_insert_component, session_id, "cmsfpix_phase2");
//    TEST(test_remove_component, session_id, "cmsfpix_phase2");
//    TEST(test_insert_log, session_id, "cmsfpix_phase2");
//    TEST(test_query_complaint, session_id, "cmsfpix_phase2");
//    TEST(test_query_parts, session_id, "cmsfpix_phase2");
//    TEST(test_query_people, session_id, "cmsfpix_phase2");
//    TEST(test_update_component, session_id, "cmsfpix_phase2");
//    TEST(test_insert_test, session_id, "cmsfpix_phase2");

    disconnect(session_id);
    return 0;
}
