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
        if (code == 0) std::cout << "SUCCESS" << std::endl;                  \
        else cout << "FAILURE! Exit-Code=" << code << std::endl;             \
    }

bool VERBOSE = false;

int test_update_component(int session_id, const char* schema) {
    int id;
    string status;
    int parent;

    cout << "Enter id" << endl;
    cin >> id;
    cout << "Enter status" << endl;
    cin.ignore();
    getline(cin, status);
    cout << "Enter parent" << endl;
    cin >> parent;

    update_component(session_id, schema, id, status.c_str(), parent);
    return 0;
}

int test_query_complaint(int session_id, const char* schema) {
    int n_complaint;
    char who[128][128];
    char description[128][128];
    int id[128];

    query_complaint(session_id, schema, &n_complaint, (char*) who, (char*) description, (int*) id);
    cout << n_complaint << endl;
    for(int i=0; i<n_complaint; i++) {
        cout << (char*) who[i] << ", " << (char*) description[i] <<  ", " << id[i] << endl;
    }
    return 0;
}

int test_query_parts(int session_id, const char* schema) {
    int n_parts;
    char parts[128][128];
    int versions[128];
    query_parts(session_id, schema, &n_parts, (char*) parts, (int*) versions);
    cout << n_parts << endl;
    for(int i=0; i<n_parts; i++) {
        cout << (char*) parts[i] << ", " << versions[i] << endl;
    }

    return 0;
}

int test_query_people(int session_id, const char* schema) {
    int n_people;
    char username[128][128];
    char name[128][128];
    char full_name[128][128];
    char email[128][128];
    char institute[128][128];
    char timezone[128][128];

    query_people(session_id, schema, &n_people, (char*) username , (char*) name,
                 (char*) full_name, (char*) email, (char*) institute, (char*) timezone);
    for(int i=0; i<n_people; i++) {
        cout << (char*) username[i] << ", " << (char*) name[i] << ", " << (char*) full_name[i] << ", " << (char*) email[i] << ", " << (char*) institute[i] << ", " << (char*) timezone[i] << endl;
    }
    return 0;
}

int test_check_login(int session_id, const char* schema, const char* username, const char* password) {
    int password_check_result = check_login(session_id, schema, username, password);
    return password_check_result;
}

int test_query_components(int session_id, const char* schema, const char* part) {
    char status[128][128];
    char location[128][128];
    char description[128][128];
    char serial_number[128][128];
    int id[128];
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

    char status[128][128];
    char location[128][128];
    char description[128][128];
    char serial_number[128][128];
    int id[128];
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

    char status[128][128];
    char location[128][128];
    char description[128][128];
    char serial_number[128][128];
    int id[128];
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
    string userid;
    string remote_ip;
    string type;

    cout << "Enter userID" << endl;
    getline(cin, userid);
    cout << "Enter remote IP" << endl;
    getline(cin, remote_ip);
    cout << "Enter type" << endl;
    getline(cin, type);


    insert_log(session_id, schema, userid.c_str(), remote_ip.c_str(), type.c_str());
    return 0;
}

int test_get_schemas(int session_id) {
    char schemas[128][128];
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


int main(int argc, const char** argv) {
    VERBOSE = false;
    std::cout << "Program Started" << std::endl;
    int session_id = connect("root", "carriker", nullptr, -1);
    if (session_id < 0) return 0;

    TEST(test_get_schemas, session_id)
    TEST(test_check_login, session_id, "cmsfpix_phase2", "amironov", "blueberries")
    TEST(test_query_components, session_id, "cmsfpix_phase2", "rd53a_chip");
    TEST(test_insert_component, session_id, "cmsfpix_phase2");
    TEST(test_remove_component, session_id, "cmsfpix_phase2");
//    test_insert_log(session_id, "cmsfpix_phase2");
//    test_query_complaint(session_id, "cmsfpix_phase2");
//    test_query_parts(session_id, "cmsfpix_phase2");
//    test_query_people(session_id, "cmsfpix_phase2");
//    test_update_component(session_id, "cmsfpix_phase2");

    disconnect(session_id);
    return 0;
}
