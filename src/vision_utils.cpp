#include "vision_utils.h"
#include <sstream>
#include <fstream>
#include "opencv2/opencv.hpp"


bool debug = false;
__declspec(dllexport) void __cdecl set_debug(bool new_debug) {
    debug = new_debug;
}

__declspec(dllexport) void __cdecl show(cv::Mat img) {
    if (debug) {
        cv::namedWindow("MyWindow", cv::WINDOW_AUTOSIZE);
        cv::imshow("MyWindow", img);
        cv::waitKey(0);
        cv::destroyWindow("MyWindow");
    }
}


std::string log_file_path;
void set_log_file_path(const char* new_log_file_path) {
    log_file_path = new_log_file_path;
}


void log(const std::string& data) {
    if (!log_file_path.empty()) {
        std::ofstream myfile;
        myfile.open(log_file_path, std::ios::out | std::ios::app);

        myfile << data << std::endl;
        myfile.close();
    }
}
void log(std::stringstream& data) {
    log(data.str());
    data.str("");
}

int color_code(const char* lv_color_code) {
    int code = -1;
    if (strcmp(lv_color_code, "Grayscale (U8)") == 0) {
        code = CV_8UC1;
    } else if(strcmp(lv_color_code, "Grayscale (I16)") == 0) {
        code = CV_16SC1;
    } else if(strcmp(lv_color_code, "Grayscale (U16)") == 0) {
        code = CV_16UC1;
    } else if(strcmp(lv_color_code, "Grayscale (SGL)") == 0) {
        code = CV_32FC1;
    } else if(strcmp(lv_color_code, "Complex (CSG)") == 0) {
        code = CV_32FC2;
    } else if(strcmp(lv_color_code, "RGB (U32)") == 0) {
        code = CV_8UC4;
    } else if(strcmp(lv_color_code, "RGB (U64)") == 0) {
        code = CV_16UC4;
    } else if(strcmp(lv_color_code, "HSL (U32)") == 0) {
        code = CV_8UC4;
    }
    return code;
}