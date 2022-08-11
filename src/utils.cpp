#include "../include/utils.h"
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


std::string log_filedir;
void set_log_filedir(const std::string& new_log_filedir) {
    log_filedir = new_log_filedir;
}


void log(const std::string& data) {
    if (!log_filedir.empty()) {
        std::ofstream myfile;
        std::string filename;
        filename.append(log_filedir);
        filename.append("\\");
        filename.append(GVISION_LOG_FILENAME);
        myfile.open(filename, std::ios::out | std::ios::app);

        myfile << data << std::endl;
        myfile.close();
    }
}
void log(std::stringstream& data) {
    log(data.str());
    data.str("");
}

