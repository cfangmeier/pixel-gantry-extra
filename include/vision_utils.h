#include <vector>
#include <string>
#include <sstream>
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#define GVISION_LOG_FILENAME "gVisionlog.txt"

extern "C" __declspec(dllexport) void __cdecl set_debug(bool new_debug);

extern "C" __declspec(dllexport) void __cdecl show(cv::Mat img);

void set_log_filedir(const std::string& new_log_filedir);

void log(const std::string &data);
void log(std::stringstream &data);