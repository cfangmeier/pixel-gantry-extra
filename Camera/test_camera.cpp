//
// Created by Caleb on 11/10/2022.
//
#include <iostream>
#include "Camera/escapi/interface.h"
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

void list_modes(int camera_id) {

    int ret = InitDevice(camera_id, 0);
    if (FAILED(ret)) return;

    std::map<int, MediaMode> modes;
    GetAvailableModes(camera_id, modes);
    std::cout << GetCaptureDeviceName(camera_id) << std::endl;
    for (auto [key, val] : modes) {
        std::cout << key << ") " << val.width << "x" << val.height << " @ " << val.framerate << "fps\n";
    }
    CleanupDevice(camera_id);
}

void capture_image(int camera_id, int mode_id) {

    int ret = InitDevice(camera_id, mode_id);
    if (FAILED(ret)) return;

    cv::namedWindow("MyWindow", cv::WINDOW_AUTOSIZE);
    while (true) {
        DoCapture(camera_id);
        while (!IsCaptureDone(camera_id));

        int* buffer;
        int width, height;
        ret = GetFrame(camera_id, &buffer, &width, &height);
        cv::Mat img(height, width, CV_8UC4, (void*)buffer, width*sizeof(int));

        cv::imshow("MyWindow", img);
        if (! cv::waitKey(5)) {
            break;
        }
    }
    cv::destroyWindow("MyWindow");

}

int main(int argc, const char* argv[]) {
    std::cout << "Hello World!\n";
    int n = setupESCAPI();
    std::cout << "n=" << n << std::endl;
    list_modes(0);

    capture_image(0, 7);

    return 0;
}