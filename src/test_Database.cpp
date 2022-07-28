//
// Created by Caleb on 2/15/2022.
//
#include <iostream>
#include "database.h"
#include "sha512.h"

using namespace std;

int main(int argc, const char** argv) {
    std::cout << "Program Started" << std::endl;
    int session_id = connect("mysqlx://root:something@localhost:33060/cmsfpix_phase2");

    char schemas[128][128];
    int n_schemas;

    get_schemas(session_id, &n_schemas, (char*) schemas);

//    int n_parts;
//    char parts[128][128];
//    int versions[128];
//    query_parts(session_id, "cmsfpix_phase2", &n_parts, (char*) parts, (int*) versions);
//    cout << n_parts << endl;
//    for(int i=0; i<n_parts; i++) {
//        cout << (char*) parts[i] << ", " << versions[i] << endl;
//    }
//
    //int n_complaint;
    //char who[128][128];
//    char description[128][128];
//    int id[128];

//    query_complaint(session_id, "cmsfpix_phase2", &n_complaint, (char*) who, (char*) description, (int*) id);
//    cout << n_complaint << endl;
//    for(int i=0; i<n_complaint; i++) {
//        cout << (char*) who[i] << ", " << (char*) description[i] <<  ", " << id[i] << endl;
//    }

//    int n_people;
//    char username[128][128];
//    char name[128][128];
//    char full_name[128][128];
//    char email[128][128];
//    char institute[128][128];
//    char timezone[128][128];
//
//    query_people(session_id, "cmsfpix_phase2", &n_people, (char*) username , (char*) name, (char*) full_name, (char*) email, (char*) institute, (char*) timezone);
//    for(int i=0; i<n_people; i++) {
//        cout << (char*) username[i] << ", " << (char*) name[i] << ", " << (char*) full_name[i] << ", " << (char*) email[i] << ", " << (char*) institute[i] << ", " << (char*) timezone[i] << endl;
//    }

//    string username;
//    string password;
//
//    cout << "Enter Username" << endl;
//    cin >> username;
//    cout << "Enter Password" << endl;
//    cin >> password;
//
//    int password_check_result = query_specific_password(session_id,"cmsfpix_phase2", username.c_str(), password.c_str());
//    cout << "Password Check Result: " << password_check_result << endl;

//    char status[128][128];
//    char location[128][128];
//    char description[128][128];
//    char serial_number[128][128];
//    int id[128];
//    int n_component;
//    string part;
//    string status;
//    string description;
//    string location;
//    int version;


//    cout << "Enter Part" << endl;
//    cin >> part;
//    query_component(session_id, "cmsfpix_phase2", part.c_str(), &n_component, (int*) id , (char*) status, (char*) description, (char*) serial_number,(char*) location);
//    for(int i = 0; i < n_component; i++) {
//        cout << id[i] << ", " << (char*) status[i] << ", " << (char*) description[i] << ", " << (char*) serial_number << ", " << (char*) location[i] << endl;
//    }

//    cout << "Enter new component data" << endl;
//    cout << "Enter part" << endl;
//    getline(cin, part);
//    cout << "Enter version" << endl;
//    cin >> version;
//    cout << "Enter status" << endl;
//    cin.ignore();
//    getline(cin, status);
//    cout << "Enter description" << endl;
//    getline(cin, description);
//    cout << "Enter location" << endl;
//    getline(cin, location);
//
//    insert_component(session_id, "cmsfpix_phase2", part.c_str(), version, status.c_str(), description.c_str(), location.c_str());
//
//

    string userid;
    string remote_ip;
    string type;
    string date;

    cout << "Enter userID" << endl;
    getline(cin, userid);
    cout << "Enter remote IP" << endl;
    getline(cin, remote_ip);
    cout << "Enter type" << endl;
    getline(cin, type);
    cout << "Enter date and time; Format (YYYY-MM-DD HH:mm:ss)" << endl;
    getline(cin, date);

    insert_log(session_id, "cmsfpix_phase2", userid.c_str(), remote_ip.c_str(), type.c_str(), date.c_str());

//    int id;
//    string status;
//    int parent;
//
//    cout << "Enter id" << endl;
//    cin >> id;
//    cout << "Enter status" << endl;
//    cin.ignore();
//    getline(cin, status);
//    cout << "Enter parent" << endl;
//    cin >> parent;
//
//    update_component(session_id, "cmsfpix_phase2", id, status.c_str(), parent);

disconnect(session_id);
return 0;

}
