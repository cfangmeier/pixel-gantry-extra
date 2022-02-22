#ifndef CLIONLVTEST_DATABASE_H
#define CLIONLVTEST_DATABASE_H

#define DLLExport __declspec(dllexport)

extern "C" {

    DLLExport int __cdecl connect(const char* connection_string);
    DLLExport int __cdecl get_schemas(int session_id, int* n_schemas, char* schemas);
    DLLExport int __cdecl query_parts(int session_id, const char* schema, int *n_parts, char* part, int* version);
    DLLExport int __cdecl disconnect(int session_id);
}

#endif //CLIONLVTEST_DATABASE_H
