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

//    char schemas[128][128];
//    int n_schemas;
//
//    get_schemas(session_id, &n_schemas, (char*) schemas);
//
//    int n_parts;
//    char parts[128][128];
//    int versions[128];
//    query_parts(session_id, "cmsfpix_phase2", &n_parts, (char*) parts, (int*) versions);
//    cout << n_parts << endl;
//    for(int i=0; i<n_parts; i++) {
//        cout << (char*) parts[i] << ", " << versions[i] << endl;
//    }
//
    int n_complaint;
    char who[128][128];
    int id[128];
    char description[128][128];

    query_complaint(session_id, "cmsfpix_phase2", &n_complaint, (char*) who, (char*) description, (int*) id);
    //cout << n_complaint << endl;
    for(int i=0; i<n_complaint; i++) {
        //cout << (char*) who[i] << ", " << (char*) description[i] <<  ", " << id[i] << endl;
    }

    int n_password;
    char username[128][128];
    char password[128][128];

    query_password(session_id, "cmsfpix_phase2", &n_password, (char*) username , (char*) password);
    //cout << n_password << endl;
    for(int i=0; i<n_password; i++) {
        //cout << (char*) username[i] << ", " << (char*) password[i] << endl;
    }

    query_specific_password(session_id,"cmsfpix_phase2", (char*) username, (char*) password);
    //cout << (char*) username << "," << (char*) password << endl;

disconnect(session_id);
return 0;

}
