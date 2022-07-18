#ifndef CLIONLVTEST_DATABASE_H
#define CLIONLVTEST_DATABASE_H

#define DLLExport __declspec(dllexport)

extern "C" {

    DLLExport int __cdecl connect(const char* connection_string);
    DLLExport int __cdecl get_schemas(int session_id, int* n_schemas, char* schemas);
    DLLExport int __cdecl query_parts(int session_id, const char* schema, int *n_complaint, char* part, int* version);
    DLLExport int __cdecl query_complaint(int session_id, const char* schema, int* n_complaint, char* who , char* description, int* id);
    DLLExport int __cdecl query_people(int session_id, const char* schema, int* n_people, char* username , char* name, char* full_name, char* email, char* institute, char* timezone);
    DLLExport int __cdecl query_specific_password(int session_id, const char* schema, const char* username, const char* password);
    DLLExport int __cdecl query_component(int session_id, const char* schema, const char* part, int* id, char* status, char* description, char* location);
    DLLExport int __cdecl insert_component(int session_id, const char* schema, char* part, char* status, char* description, char* location);
    DLLExport int __cdecl disconnect(int session_id);
}

#endif //CLIONLVTEST_DATABASE_H
