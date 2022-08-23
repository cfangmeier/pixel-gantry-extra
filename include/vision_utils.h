#include <vector>
#include <string>
#include <sstream>
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

extern "C" {
    __declspec(dllexport) void __cdecl set_debug(bool new_debug);
    __declspec(dllexport) void __cdecl show(cv::Mat img);
    __declspec(dllexport) void __cdecl set_log_file_path(const char* new_log_file_path);
};

void log(const std::string &data);
void log(std::stringstream &data);

int color_code(const char* lv_color_code);
