#pragma once

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <map>
#include <string>
#include "mediamode.h"

void CleanupDevice(int aDevice);
HRESULT InitDevice(int aDevice, int mode_id);
int CountCaptureDevices();
std::string GetCaptureDeviceName(int aDevice);
void CheckForFail(int device_id);
int GetErrorCode(int aDevice);
int GetErrorLine(int aDevice);
float GetProperty(int aDevice, int aProp);
int GetPropertyAuto(int aDevice, int aProp);
int SetProperty(int aDevice, int aProp, float aValue, int aAutoval);
void GetAvailableModes(int aDevice, std::map<int,MediaMode> &modes);

void DoCapture(int device_id);
int IsCaptureDone(int device_id);

HRESULT GetFrame(int device_id, int** buffer, int* width, int* height);

int setupESCAPI();

