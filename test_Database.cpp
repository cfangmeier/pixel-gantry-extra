//
// Created by Caleb on 2/15/2022.
//
#include <iostream>
#include "database.h"

using namespace std;

int main(int argc, const char** argv) {
    std::cout << "Program Started" << std::endl;
    int session_id = connect("mysqlx://root:carriker@localhost:33060/cmsfpix_phase2");

    char schemas[128][128];
    int n_schemas;

    get_schemas(session_id, &n_schemas, (char*) schemas);

    int n_parts;
    char parts[128][128];
    int versions[128];
    query_parts(session_id, "cmsfpix_phase2", &n_parts, (char*) parts, (int*) versions);
    cout << n_parts << endl;
    for(int i=0; i<n_parts; i++) {
        cout << (char*) parts[i] << ", " << versions[i] << endl;
    }

    disconnect(session_id);
    return 0;
}