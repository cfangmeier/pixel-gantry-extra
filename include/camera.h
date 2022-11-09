//
// Created by Caleb on 11/8/2022.
//

#ifndef GANTRYEXT_CAMERA_H
#define GANTRYEXT_CAMERA_H

#define DLLExport __declspec(dllexport)

#define CAM_STR_LEN 256
#define CAM_ARR_LEN 1024

extern "C" {
DLLExport int __cdecl max_string_length() {return CAM_STR_LEN;};
DLLExport int __cdecl max_array_length() {return CAM_ARR_LEN;};
DLLExport int __cdecl escapi_open_camera(const char* camera_name, int width, int height, float* config_opts, int* config_auto);
DLLExport int __cdecl escapi_close_camera(const char* camera_name);
DLLExport int __cdecl escapi_list_cameras(int *n_cameras, char* names);
DLLExport int __cdecl escapi_capture_image(const char* camera_name,
                                           char *dstPtr, int dstLineWidth,
                                           int dstWidth, int dstHeight);
}

#endif //GANTRYEXT_CAMERA_H
