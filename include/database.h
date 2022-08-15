#ifndef CLIONLVTEST_DATABASE_H
#define CLIONLVTEST_DATABASE_H

#define DLLExport __declspec(dllexport)

#define DB_STR_LEN 128
#define DB_ARR_LEN 1024

typedef char str_table_t[DB_ARR_LEN][DB_STR_LEN] ;
typedef int int_table_t[DB_ARR_LEN];
typedef float float_table_t [DB_ARR_LEN];

extern "C" {

    DLLExport int __cdecl connect(const char* username, const char* password, const char* url, int port);
    DLLExport int __cdecl disconnect(int session_id);
    DLLExport int __cdecl start_transaction(int session_id);
    DLLExport int __cdecl rollback_transaction(int session_id);
    DLLExport int __cdecl finish_transaction(int session_id);
    DLLExport int __cdecl max_string_length() {return DB_STR_LEN;};
    DLLExport int __cdecl max_array_length() {return DB_ARR_LEN;};
    DLLExport int __cdecl get_schemas(int session_id, int* n_schemas, char* schemas);
    DLLExport int __cdecl query_parts(int session_id, const char* schema, int* n_parts, char* part, int* version,
                                      char* description, char* prefix, float* dim_x, float* dim_y, float* dim_z, char* type);
    DLLExport int __cdecl query_complaint(int session_id, const char* schema, int* n_complaint, char* who,
                                          char* description, int* id);
    DLLExport int __cdecl query_people(int session_id, const char* schema, int* n_people, char* username , char* name,
                                       char* full_name, char* email, char* institute, char* timezone);
    DLLExport int __cdecl check_login(int session_id, const char* schema, const char* username, const char* password);
    DLLExport int __cdecl query_components(int session_id, const char* schema, const char* part, int* n_component,
                                           int* id, char* status, char* description, char* serial_number,
                                           char* location);
    DLLExport int __cdecl insert_component(int session_id, const char* schema, const char* part, int version,
                                           const char* status, const char* description, const char* who,
                                           const char* serial_number, const char* location);
    DLLExport int __cdecl update_component(int session_id, const char* schema, int id, const char* status, int parent);
    DLLExport int __cdecl remove_component(int session_id, const char* schema, int component_id);
    DLLExport int __cdecl insert_log(int session_id, const char* schema, const char* userid, const char* remote_ip,
                                     const char* type);
    DLLExport int __cdecl insert_test(int session_id, const char* schema, const char* description, const char* data,
                                      int component_id, const char* type);
}

#endif //CLIONLVTEST_DATABASE_H