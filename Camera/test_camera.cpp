////
//// Created by Caleb on 11/10/2022.
////
//#include <iostream>
//#include "interface.h"
//#include "opencv2/opencv.hpp"
//#include "opencv2/imgproc.hpp"
//#include "opencv2/highgui/highgui.hpp"
//
//void list_modes(int camera_id) {
//
//    int ret = InitDevice(camera_id, 0);
//    if (FAILED(ret)) return;
//
//    std::map<int, MediaMode> modes;
//    GetAvailableModes(camera_id, modes);
//    LStrHandle lsh;
//    GetCaptureDeviceName(camera_id, lsh);
//    std::cout << *lsh << std::endl;
//    for (auto [key, val]: modes) {
//        std::cout << key << ") " << val.width << "x" << val.height << " @ " << val.framerate << "fps\n";
//    }
//    CleanupDevice(camera_id);
//}
//
//void capture_image(int camera_id, int mode_id) {
//
//    int ret = InitDevice(camera_id, mode_id);
//    if (FAILED(ret)) return;
//
//    cv::namedWindow("MyWindow", cv::WINDOW_AUTOSIZE);
//    while (true) {
//        DoCapture(camera_id);
//        while (!IsCaptureDone(camera_id));
//
//        int *buffer;
//        int width, height;
//        ret = GetFrame(camera_id, &buffer, &width, &height);
//        cv::Mat img(height, width, CV_8UC4, (void *) buffer, width * sizeof(int));
//        cv::Mat gs;
//        cv::cvtColor(img, gs, cv::COLOR_BGR2GRAY);
//
//        cv::imshow("MyWindow", gs);
//        if (cv::waitKey(5)>0) {
//            break;
//        }
//    }
//    cv::destroyWindow("MyWindow");
//
//}
//
//int main(int argc, const char *argv[]) {
//    std::cout << "Hello World!\n";
//    int count;
//    CountCaptureDevices(&count);
//    std::cout << "count=" << count << std::endl;
////    LStrHandle lsh;
////    GetCaptureDeviceName(0, lsh);
////    std::cout << "Name=" << *lsh << std::endl;
//    list_modes(0);
//
//    capture_image(0, 0);
//
//    return 0;
//}




#include <windows.h>
#include <ioapiset.h>
#include <iostream>
#include <string>

BOOL GetSerialNumberUSB(LPCWSTR sDriveName, std::string &strSerialNumber) {
    // https://social.msdn.microsoft.com/Forums/en-US/faa5119a-7fc1-4e0f-95de-e8bbde7cbdcc/get-serial-number-of-usb-drive-correctly?forum=vbgeneral
    // handle to physical drive
    HANDLE hVolume = CreateFileW(
        sDriveName,                            // lpFileName
        GENERIC_READ,                          // dwDesiredAccess
        FILE_SHARE_READ || FILE_SHARE_WRITE,   // dwShareMode
        NULL,                                  // lpSecurityAttributes
        OPEN_EXISTING,                         // dwCreationDisposition
        FILE_ATTRIBUTE_NORMAL,                 // dwFlagsAndAttributes
        NULL                                   // hTemplateFile
    );
    if (hVolume == INVALID_HANDLE_VALUE) {
        // print GetLastError()
        return FALSE;
    }

    // set the STORAGE_PROPERTY_QUERY input
    STORAGE_PROPERTY_QUERY PropertyQuery;
    ZeroMemory(&PropertyQuery, sizeof(STORAGE_PROPERTY_QUERY));
    PropertyQuery.PropertyId = StorageDeviceProperty;
    PropertyQuery.QueryType = PropertyStandardQuery;

    // first call to DeviceIocontrol to get the size of the output
    STORAGE_DEVICE_DESCRIPTOR  DeviceDescriptor = { 0 };
    DWORD nBytesDeviceDescriptor = 0;
    if ( !DeviceIoControl(
        hVolume,                              // hDevice
        IOCTL_STORAGE_QUERY_PROPERTY,         // dwIoControlCode
        &PropertyQuery,                       // lpInBuffer
        sizeof(STORAGE_PROPERTY_QUERY),       // nInBufferSize
        &DeviceDescriptor,                    // lpOutBuffer
        sizeof(STORAGE_DESCRIPTOR_HEADER),    // nOutBufferSize
        &nBytesDeviceDescriptor,              // lpBytesReturned
        NULL                                  // lpOverlapped
    )) {
        // print GetLastError()
        CloseHandle(hVolume);
        return FALSE;
    }

    // allocate the output
    const DWORD dwOutBufferSize = DeviceDescriptor.Size;
    char* pOutBuffer = new char[dwOutBufferSize];
    ZeroMemory(pOutBuffer, dwOutBufferSize);
    STORAGE_DEVICE_DESCRIPTOR* pDeviceDescriptor = reinterpret_cast<STORAGE_DEVICE_DESCRIPTOR*>(pOutBuffer);

    // second call to DeviceIocontrol to get the actual output STORAGE_DEVICE_DESCRIPTOR
    if (!DeviceIoControl(
        hVolume,
        IOCTL_STORAGE_QUERY_PROPERTY,
        &PropertyQuery,
        sizeof(PropertyQuery),
        pDeviceDescriptor,
        dwOutBufferSize,
        &nBytesDeviceDescriptor,
        NULL
    )) {
        // print GetLastError()
        delete[] pOutBuffer;
        CloseHandle(hVolume);
        return FALSE;
    }

    const DWORD nSerialNumberOffset = pDeviceDescriptor->SerialNumberOffset;
    if (nSerialNumberOffset == 0) {
        // print GetLastError()
        delete[] pOutBuffer;
        CloseHandle(hVolume);
        return FALSE;
    }
    strSerialNumber = static_cast<std::string>(pOutBuffer + nSerialNumberOffset);

    delete[] pOutBuffer;
    CloseHandle(hVolume);
    return (strSerialNumber.empty()) ? FALSE : TRUE;
}

int main(int argc, const char** argv) {
    std::string s;
//    GetSerialNumberUSB(L"\\\\.\\C:", s);  // for USB drive E:
    GetSerialNumberUSB(L"\\?\\usb#vid_046d&pid_085b&mi_00#a&1a72a30b&0&0000#{e5323777-f976-4f5b-9b55-b94699c46e44}\\global", s);  // for USB drive E:
    std::cout << s << std::endl;
    return 0;
}