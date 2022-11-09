//
// Created by Caleb on 11/8/2022.
//
#include <string>
#include <map>
#include <windows.h>
#include "camera.h"
#include "escapi.h"
#include "opencv2/opencv.hpp"

bool escapi_initialized = false;
std::map<int,SimpleCapParams> cam_params;


int init_escapi() {
    if (!escapi_initialized) {
        int ret = setupESCAPI();
        if (ret < 0) return ret;
        escapi_initialized = true;
    }
    return 0;
}

std::vector<std::string> get_camera_names() {

    int n_cameras = countCaptureDevices();
    std::vector<std::string> names;
    std::map<std::string,int> dev_name_count;

    for(int i=0; i<n_cameras; i++) {
        char buffer[256];
        getCaptureDeviceName(i, (char*) buffer, 256);
        std::string name(buffer);
        std::stringstream ss(name);
        try {
            dev_name_count[name] = dev_name_count[name] + 1;
            ss << name << " (" << dev_name_count[name] << ")";
        } catch (std::out_of_range &e) {
            dev_name_count[name] = 1;
        }
        names.push_back(name);
    }
    return names;
}

int get_camera_id(const char* camera_name) {
    std::vector<std::string> names_ = get_camera_names();
    std::string camera_name_(camera_name);

    auto res = std::find(names_.begin(), names_.end(), camera_name_);
    if (res == names_.end()) {
        return -1;
    } else {
        return (int) std::distance(names_.begin(), res);
    }
}

DLLExport int __cdecl escapi_open_camera(const char* camera_name, int width, int height, float* props, int* prop_autos) {
    if (int ret = init_escapi()) return ret;
    int camera_id = get_camera_id(camera_name);
    if (camera_id < 0) {
        return -1;
    }

    SimpleCapParams p = {
            new int[width * height], // target buffer
            width, // width in pixels
            height  // height in pixels
    };

    // Returns 1 in case of success, 0 for failure
    int ret = initCapture(camera_id, &p);
    if (ret == 0) {
        return -1;
    }
    else {
        cam_params[camera_id] = p;

#define SETPROP(p, idx) \
                        if (prop_autos[idx] >= 0) { \
                            setCaptureProperty(camera_id, p, props[idx], prop_autos[idx]);\
                        }
        SETPROP(CAPTURE_BRIGHTNESS, 0);
        SETPROP(CAPTURE_CONTRAST, 1);
        SETPROP(CAPTURE_HUE, 2);
        SETPROP(CAPTURE_SATURATION, 3);
        SETPROP(CAPTURE_SHARPNESS, 4);
        SETPROP(CAPTURE_GAMMA, 5);
        SETPROP(CAPTURE_COLORENABLE, 6);
        SETPROP(CAPTURE_WHITEBALANCE, 7);
        SETPROP(CAPTURE_BACKLIGHTCOMPENSATION, 8);
        SETPROP(CAPTURE_GAIN, 9);
        SETPROP(CAPTURE_PAN, 10);
        SETPROP(CAPTURE_TILT, 11);
        SETPROP(CAPTURE_ROLL, 12);
        SETPROP(CAPTURE_ZOOM, 13);
        SETPROP(CAPTURE_EXPOSURE, 14);
        SETPROP(CAPTURE_IRIS, 15);
        SETPROP(CAPTURE_FOCUS, 16);
        return 0;
    }
}

DLLExport int __cdecl escapi_close_camera(const char* camera_name) {
    if (int ret = init_escapi()) return ret;
    int camera_id = get_camera_id(camera_name);
    if (camera_id < 0) {
        return -1;
    }
    deinitCapture(camera_id);
    SimpleCapParams p = cam_params.at(camera_id);
    cam_params.erase(camera_id);
    delete p.mTargetBuf;
    return 0;
}

DLLExport int __cdecl escapi_list_cameras(int *n_cameras, char* names) {
    if (int ret = init_escapi()) return ret;

    std::vector<std::string> names_ = get_camera_names();
    *n_cameras = (int) names_.size();

    for(int i=0; i<*n_cameras && i < CAM_ARR_LEN; i++) {
        memcpy((char*) (names + CAM_STR_LEN*i), names_[i].c_str(), names_[i].size());
    }
    return 0;
}

DLLExport int __cdecl escapi_capture_image(const char* camera_name,
                                           char *dstPtr, int dstLineWidth,
                                           int dstWidth, int dstHeight) {
    if (int ret = init_escapi()) return ret;
    cv::Mat imgIn(dstHeight, dstWidth, CV_8U, (void*)dstPtr, dstLineWidth);
    int camera_id = get_camera_id(camera_name);
    if (camera_id < 0) {
        return -1;
    }

    doCapture(camera_id);
    while (isCaptureDone(camera_id) == 0) {
        // wait for this to finish
    }
    const SimpleCapParams& p= cam_params[camera_id];
    for (int i=0; i<p.mHeight && i<dstHeight; i++) {
        for (int j=0; j<p.mWidth && i<dstWidth; j++){
            int pixel = *(p.mTargetBuf + i*p.mWidth + j);
            auto r = (float) ((pixel >> 16) & 0xFF);
            auto g = (float) ((pixel >> 8 & 0xFF));
            auto b = (float) ((pixel >> 0 & 0xFF));
            float pixel_gray = 0.2126f*r + 0.7152f*g + 0.0722f*b;
            imgIn.at<char>(i, j) = (char) pixel_gray;
        }
    }
    return 0;
}
