#include "interface.h"
#include "capture.h"
#include "scopedrelease.h"
#include "choosedeviceparam.h"

#include <vector>
#include <string>
#include <map>
#include <stdexcept>

std::map<int,CaptureClass*> gDevice;
std::map<int,int> gDoCapture;
std::map<int,int> gOptions;
std::map<int,int*> gBuffer;

void CleanupDevice(int device_id) {
    try {
        CaptureClass* cc = gDevice.at(device_id);
        cc->deinitCapture();
        delete cc;
        gDevice.erase(device_id);

        int* buffer = gBuffer.at(device_id);
        delete buffer;
        gBuffer.erase(device_id);
    } catch (std::out_of_range &e) {
        return;
    }
}

HRESULT InitDevice(int device_id, int mode_id) {
    if (device_id < 0 || mode_id < 0)
        return -1;
    gDoCapture[device_id] = 0;
    gOptions[device_id] = 0;
	if (gDevice.find(device_id) != gDevice.end()) {
		CleanupDevice(device_id);
	}
	gDevice[device_id] = new CaptureClass();
	HRESULT hr = gDevice[device_id]->initCapture(device_id, mode_id);
	if (FAILED(hr)) {
		delete gDevice[device_id];
		gDevice.erase(device_id);
	} else {
        gBuffer[device_id] = new int[gDevice[device_id]->mCaptureBufferWidth*gDevice[device_id]->mCaptureBufferHeight];
    }
	return hr;
}


int CountCaptureDevices() {
	HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (FAILED(hr)) return 0;

	hr = MFStartup(MF_VERSION);
	if (FAILED(hr)) return 0;

	// choose device
	IMFAttributes *attributes = nullptr;
	hr = MFCreateAttributes(&attributes, 1);
	ScopedRelease<IMFAttributes> attributes_s(attributes);
	if (FAILED(hr)) return 0;

	hr = attributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
	if (FAILED(hr)) return 0;

	ChooseDeviceParam param = { nullptr };
	hr = MFEnumDeviceSources(attributes, &param.mDevices, &param.mCount);
	if (FAILED(hr)) return 0;

	return (int) param.mCount;
}


std::string GetCaptureDeviceName(int aDevice) {

	HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (FAILED(hr)) return "N/A";

	hr = MFStartup(MF_VERSION);
	if (FAILED(hr)) return "N/A";

	// choose device
	IMFAttributes *attributes = nullptr;
	hr = MFCreateAttributes(&attributes, 1);
	ScopedRelease<IMFAttributes> attributes_s(attributes);
	if (FAILED(hr)) return "N/A";

	hr = attributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
	if (FAILED(hr)) return "N/A";

	ChooseDeviceParam param = { nullptr };
	hr = MFEnumDeviceSources(attributes, &param.mDevices, &param.mCount);
	if (FAILED(hr)) return "N/A";

	if (aDevice < (signed)param.mCount) {
		WCHAR *name = nullptr;
		UINT32 namelen = 255;
		hr = param.mDevices[aDevice]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &name, &namelen);
		if (SUCCEEDED(hr) && name) {
            std::wstring ws(name);
            std::string str(ws.begin(), ws.end());
            return str;
			CoTaskMemFree(name);
		}
	}
    return "N/A";
}


void CheckForFail(int device_id) {
	if (!gDevice[device_id])
		return;

	if (gDevice[device_id]->mRedoFromStart)
	{
		gDevice[device_id]->mRedoFromStart = 0;
		gDevice[device_id]->deinitCapture();
		HRESULT hr = gDevice[device_id]->initCapture(device_id, gDevice[device_id]->mode_id);
		if (FAILED(hr))
		{
			delete gDevice[device_id];
			gDevice[device_id] = nullptr;
		}
	}
}

void GetAvailableModes(int aDevice, std::map<int,MediaMode> &modes) {
    if (gDevice.find(aDevice) == gDevice.end() || gDevice[aDevice] == nullptr)
        return;
    gDevice[aDevice]->getModes(modes);
}


int GetErrorCode(int aDevice) {
	if (!gDevice[aDevice])
		return 0;
	return gDevice[aDevice]->mErrorCode;
}


int GetErrorLine(int aDevice) {
	if (!gDevice[aDevice])
		return 0;
	return gDevice[aDevice]->mErrorLine;
}


float GetProperty(int aDevice, int aProp) {
	CheckForFail(aDevice);
	if (!gDevice[aDevice])
		return 0;
	float val;
	int autoval;
	gDevice[aDevice]->getProperty(aProp, val, autoval);
	return val;
}


int GetPropertyAuto(int aDevice, int aProp) {
	CheckForFail(aDevice);
	if (!gDevice[aDevice])
		return 0;
	float val;
	int autoval;
	gDevice[aDevice]->getProperty(aProp, val, autoval);
	return autoval;
}


int SetProperty(int aDevice, int aProp, float aValue, int aAutoval) {
	CheckForFail(aDevice);
	if (!gDevice[aDevice])
		return 0;
	return gDevice[aDevice]->setProperty(aProp, aValue, aAutoval);
}


int setupESCAPI() {
    /* Initialize COM.. */
    CoInitialize(nullptr);

    /* and return the number of capture devices found. */
    return CountCaptureDevices();
}

void DoCapture(int device_id) {
    CheckForFail(device_id);
    gDoCapture[device_id] = -1;
}

int IsCaptureDone(int device_id) {
    CheckForFail(device_id);
    if (gDoCapture[device_id] == 1)
        return 1;
    return 0;
}

HRESULT GetFrame(int device_id, int** buffer, int* width, int* height) {
    try {
        *buffer = gBuffer.at(device_id);
        *width = gDevice.at(device_id)->mCaptureBufferWidth;
        *height = gDevice.at(device_id)->mCaptureBufferHeight;
    } catch (std::out_of_range &e) {
        return -1;
    }
    return 0;
}
