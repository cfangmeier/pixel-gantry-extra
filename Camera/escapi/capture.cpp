#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <Mferror.h>

#include <Shlwapi.h> // QITAB and friends
#include <objbase.h> // IID_PPV_ARGS and friends
#include <dshow.h> // IAMVideoProcAmp and friends

#include <cmath>
#include <iostream>
#include <vector>

#define ESCAPI_DEFINITIONS_ONLY

#include "conversion.h"
#include "capture.h"
#include "scopedrelease.h"
#include "videobufferlock.h"
#include "choosedeviceparam.h"
#include "captureproperties.h"

extern std::map<int,int> gDoCapture;
extern std::map<int,int> gOptions;
extern std::map<int,int*> gBuffer;


#define DO_OR_DIE { if (mErrorLine) return hr; if (!SUCCEEDED(hr)) { mErrorLine = __LINE__; mErrorCode = hr; return hr; } }
#define DO_OR_DIE_CRITSECTION { if (mErrorLine) { LeaveCriticalSection(&mCritsec); return hr;} if (!SUCCEEDED(hr)) { LeaveCriticalSection(&mCritsec); mErrorLine = __LINE__; mErrorCode = hr; return hr; } }

CaptureClass::CaptureClass() {
	mRefCount = 1;
	mReader = nullptr;
	InitializeCriticalSection(&mCritsec);
	mCaptureBuffer = nullptr;
	mCaptureBufferWidth = 0;
	mCaptureBufferHeight = 0;
	mErrorLine = 0;
	mErrorCode = 0;
	mBadIndices = 0;
	mMaxBadIndices = 16;
	mBadIndex = new int[mMaxBadIndices];
	mRedoFromStart = 0;
}

CaptureClass::~CaptureClass() {
	DeleteCriticalSection(&mCritsec);
	delete[] mBadIndex;
}

// IUnknown methods
STDMETHODIMP CaptureClass::QueryInterface(REFIID aRiid, void** aPpv) {
	static const QITAB qit[] = {QITABENT(CaptureClass, IMFSourceReaderCallback), { nullptr },};
	return QISearch(this, qit, aRiid, aPpv);
}

STDMETHODIMP_(ULONG) CaptureClass::AddRef() {
	return InterlockedIncrement(&mRefCount);
}

STDMETHODIMP_(ULONG) CaptureClass::Release() {
	ULONG count = InterlockedDecrement(&mRefCount);
	if (count == 0) {
		delete this;
	}
	// For thread safety, return a temporary variable.
	return count;
}

// IMFSourceReaderCallback methods
STDMETHODIMP CaptureClass::OnReadSample(HRESULT aStatus, DWORD aStreamIndex,
                                        DWORD aStreamFlags, LONGLONG aTimestamp, IMFSample *aSample) {
	HRESULT hr = S_OK;
	IMFMediaBuffer *mediabuffer = nullptr;

	if (FAILED(aStatus)) {
		// Bug workaround: some resolutions may just return error.
		// http://stackoverflow.com/questions/22788043/imfsourcereader-giving-error-0x80070491-for-some-resolutions
		// we fix by marking the resolution bad and retrying, which should use the next best match.
		mRedoFromStart = 1;
		if (mBadIndices == mMaxBadIndices) {
			auto *t = new int[mMaxBadIndices * 2];
			memcpy(t, mBadIndex, mMaxBadIndices * sizeof(int));
			delete[] mBadIndex;
			mBadIndex = t;
			mMaxBadIndices *= 2;
		}
		mBadIndex[mBadIndices] = mode_id;
		mBadIndices++;
		return aStatus;
	}

	EnterCriticalSection(&mCritsec);

	if (SUCCEEDED(aStatus)) {
		if (gDoCapture[device_id] == -1) {
			if (aSample) {
				// Get the video frame buffer from the sample.

				hr = aSample->GetBufferByIndex(0, &mediabuffer);
				ScopedRelease<IMFMediaBuffer> mediabuffer_s(mediabuffer);

				DO_OR_DIE_CRITSECTION;

				// Draw the frame.

				if (mConvertFn) {
					VideoBufferLock buffer(mediabuffer);    // Helper object to lock the video buffer.

					BYTE *scanline0 = nullptr;
					LONG stride = 0;
					hr = buffer.LockBuffer(mDefaultStride, mCaptureBufferHeight, &scanline0, &stride);

					DO_OR_DIE_CRITSECTION;

					mConvertFn((BYTE *)mCaptureBuffer, mCaptureBufferWidth * 4, scanline0,
                               stride, mCaptureBufferWidth, mCaptureBufferHeight);
				}
				else {
					// No convert function?
					if (gOptions[device_id] & CAPTURE_OPTION_RAWDATA) {
						// Ah ok, raw data was requested, so let's copy it then.

						VideoBufferLock buffer(mediabuffer);    // Helper object to lock the video buffer.
						unsigned char *scanline0 = nullptr;
						long stride = 0;
						hr = buffer.LockBuffer(mDefaultStride, mCaptureBufferHeight, &scanline0, &stride);
						if (stride < 0) {
							scanline0 += stride * mCaptureBufferHeight;
							stride = -stride;
						}
						long bytes = stride * mCaptureBufferHeight;
						CopyMemory(mCaptureBuffer, scanline0, bytes);
					}
				}

				int i, j;
				int *dst = gBuffer[device_id];
				int *src = (int*)mCaptureBuffer;
				for (i = 0; i < mCaptureBufferHeight; i++) {
					for (j = 0; j < mCaptureBufferWidth; j++, dst++) {
						*dst = src[i * mCaptureBufferWidth + j];
					}
				}
				gDoCapture[device_id] = 1;
			}
		}
	}

	// Request the next frame.
	hr = mReader->ReadSample(
		(DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
		0,
		nullptr,   // actual
		nullptr,   // flags
		nullptr,   // timestamp
		nullptr    // sample
		);

	DO_OR_DIE_CRITSECTION;

	LeaveCriticalSection(&mCritsec);

	return hr;
}

STDMETHODIMP CaptureClass::OnEvent(DWORD, IMFMediaEvent *) {
	return S_OK;
}

STDMETHODIMP CaptureClass::OnFlush(DWORD) {
	return S_OK;
}

int CaptureClass::escapiPropToMFProp(int aProperty) {
	int prop = 0;
	switch (aProperty) {
		//case CAPTURE_BRIGHTNESS:
	default:
		prop = VideoProcAmp_Brightness;
		break;
	case CAPTURE_CONTRAST:
		prop = VideoProcAmp_Contrast;
		break;
	case CAPTURE_HUE:
		prop = VideoProcAmp_Hue;
		break;
	case CAPTURE_SATURATION:
		prop = VideoProcAmp_Saturation;
		break;
	case CAPTURE_SHARPNESS:
		prop = VideoProcAmp_Sharpness;
		break;
	case CAPTURE_GAMMA:
		prop = VideoProcAmp_Gamma;
		break;
	case CAPTURE_COLORENABLE:
		prop = VideoProcAmp_ColorEnable;
		break;
	case CAPTURE_WHITEBALANCE:
		prop = VideoProcAmp_WhiteBalance;
		break;
	case CAPTURE_BACKLIGHTCOMPENSATION:
		prop = VideoProcAmp_BacklightCompensation;
		break;
	case CAPTURE_GAIN:
		prop = VideoProcAmp_Gain;
		break;
	case CAPTURE_PAN:
		prop = CameraControl_Pan;
		break;
	case CAPTURE_TILT:
		prop = CameraControl_Tilt;
		break;
	case CAPTURE_ROLL:
		prop = CameraControl_Roll;
		break;
	case CAPTURE_ZOOM:
		prop = CameraControl_Zoom;
		break;
	case CAPTURE_EXPOSURE:
		prop = CameraControl_Exposure;
		break;
	case CAPTURE_IRIS:
		prop = CameraControl_Iris;
		break;
	case CAPTURE_FOCUS:
		prop = CameraControl_Focus;
		break;
	}
	return prop;
}

int CaptureClass::setProperty(int aProperty, float aValue, int aAuto) {
	HRESULT hr;
	IAMVideoProcAmp *procAmp = nullptr;
	IAMCameraControl *control = nullptr;

	int prop = escapiPropToMFProp(aProperty);

	if (aProperty < CAPTURE_PAN) {
		hr = mSource->QueryInterface(IID_PPV_ARGS(&procAmp));
		if (SUCCEEDED(hr)) {
			long min, max, step, def, caps;
			hr = procAmp->GetRange(prop, &min, &max, &step, &def, &caps);

			if (SUCCEEDED(hr)) {
				LONG val = (long)floor(min + (max - min) * aValue);
				if (aAuto)
					val = def;
				hr = procAmp->Set(prop, val, aAuto ? VideoProcAmp_Flags_Auto : VideoProcAmp_Flags_Manual);
			}
			procAmp->Release();
			return SUCCEEDED(hr);
		}
	}
	else {
		hr = mSource->QueryInterface(IID_PPV_ARGS(&control));
		if (SUCCEEDED(hr)) {
			long min, max, step, def, caps;
			hr = control->GetRange(prop, &min, &max, &step, &def, &caps);

			if (SUCCEEDED(hr)) {
				LONG val = (long)floor(min + (max - min) * aValue);
				if (aAuto)
					val = def;
				hr = control->Set(prop, val, aAuto ? VideoProcAmp_Flags_Auto : VideoProcAmp_Flags_Manual);
			}
			control->Release();
			return SUCCEEDED(hr);
		}
	}

	return 1;
}

int CaptureClass::getProperty(int aProperty, float &aValue, int &aAuto) {
	HRESULT hr;
	IAMVideoProcAmp *procAmp = nullptr;
	IAMCameraControl *control = nullptr;

	aAuto = 0;
	aValue = -1;

	int prop = escapiPropToMFProp(aProperty);

	if (aProperty < CAPTURE_PAN) {
		hr = mSource->QueryInterface(IID_PPV_ARGS(&procAmp));
		if (SUCCEEDED(hr)) {
			long min, max, step, def, caps;
			hr = procAmp->GetRange(prop, &min, &max, &step, &def, &caps);

			if (SUCCEEDED(hr)) {
				long v = 0, f = 0;
				hr = procAmp->Get(prop, &v, &f);
				if (SUCCEEDED(hr)) {
					aValue = (v - min) / (float)(max - min);
					aAuto = !!(f & VideoProcAmp_Flags_Auto);
				}
			}
			procAmp->Release();
			return 0;
		}
	}
	else {
		hr = mSource->QueryInterface(IID_PPV_ARGS(&control));
		if (SUCCEEDED(hr)) {
			long min, max, step, def, caps;
			hr = control->GetRange(prop, &min, &max, &step, &def, &caps);

			if (SUCCEEDED(hr)) {
				long v = 0, f = 0;
				hr = control->Get(prop, &v, &f);
				if (SUCCEEDED(hr)) {
					aValue = (v - min) / (float)(max - min);
					aAuto = !!(f & VideoProcAmp_Flags_Auto);
				}
			}
			control->Release();
			return 0;
		}
	}

	return 1;
}

bool CaptureClass::isFormatSupported(REFGUID aSubtype) {
	int i;
	for (i = 0; i < (signed)gConversionFormats; i++) {
		if (aSubtype == gFormatConversions[i].mSubtype) {
			return true;
		}
	}
	return false;
}

HRESULT CaptureClass::getFormat(DWORD aIndex, GUID *aSubtype) const {
	if (aIndex < gConversionFormats) {
		*aSubtype = gFormatConversions[aIndex].mSubtype;
		return S_OK;
	}
	return MF_E_NO_MORE_TYPES;
}

HRESULT CaptureClass::setConversionFunction(REFGUID aSubtype) {
	mConvertFn = nullptr;

	// If raw data is desired, skip conversion
	if (gOptions[device_id] & CAPTURE_OPTION_RAWDATA)
		return S_OK; 

	for (DWORD i = 0; i < gConversionFormats; i++) {
		if (gFormatConversions[i].mSubtype == aSubtype) {
			mConvertFn = gFormatConversions[i].mXForm;
			return S_OK;
		}
	}
	return MF_E_INVALIDMEDIATYPE;
}

HRESULT CaptureClass::setVideoType(IMFMediaType *aType) {
	HRESULT hr = S_OK;
	GUID subtype = { 0 };

	// Find the video subtype.
	hr = aType->GetGUID(MF_MT_SUBTYPE, &subtype);

	DO_OR_DIE;

	// Choose a conversion function.
	// (This also validates the format type.)

	hr = setConversionFunction(subtype);

	DO_OR_DIE;

	//
	// Get some video attributes.
	//

	subtype = GUID_NULL;

	UINT32 width = 0;
	UINT32 height = 0;

	// Get the subtype and the image size.
	hr = aType->GetGUID(MF_MT_SUBTYPE, &subtype);

	DO_OR_DIE;

	hr = MFGetAttributeSize(aType, MF_MT_FRAME_SIZE, &width, &height);

	DO_OR_DIE;

	hr = MFGetStrideForBitmapInfoHeader(subtype.Data1, width, &mDefaultStride);

	mCaptureBuffer = new unsigned int[width * height];
	mCaptureBufferWidth = (int) width;
	mCaptureBufferHeight = (int) height;

	DO_OR_DIE;

	return hr;
}

int CaptureClass::isMediaOk(IMFMediaType *aType, int aIndex) {
	HRESULT hr = S_OK;

	for (int i = 0; i < (signed)mBadIndices; i++) {
        if (mBadIndex[i] == aIndex) {
            return false;
        }
    }

	bool found = false;
	GUID subtype = { 0 };

	hr = aType->GetGUID(MF_MT_SUBTYPE, &subtype);
	DO_OR_DIE;

	// Do we support this type directly?
	if (isFormatSupported(subtype)) {
		found = true;
	}
	else { // Can we decode this media type to one of our supported output formats?
		for (int i = 0;; i++) {
			// Get the i'th format.
			hr = getFormat(i, &subtype);
			if (FAILED(hr)) { break; }

			hr = aType->SetGUID(MF_MT_SUBTYPE, subtype);
			if (FAILED(hr)) { break; }

			// Try to set this type on the source reader.
			hr = mReader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, aType);
			if (SUCCEEDED(hr)) {
				found = true;
				break;
			}
		}
	}
	return found;
}

//int CaptureClass::scanMediaTypes(unsigned int aWidth, unsigned int aHeight, float aFramerate) {
//	HRESULT hr;
//	HRESULT nativeTypeErrorCode = S_OK;
//	DWORD count = 0;
//	int besterror = 0xfffffff;
//	int bestfit = 0;
//
//	while (nativeTypeErrorCode == S_OK && besterror) {
//		IMFMediaType * nativeType = nullptr;
//		nativeTypeErrorCode = mReader->GetNativeMediaType(
//			(DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
//			count,
//			&nativeType);
//		ScopedRelease<IMFMediaType> nativeType_s(nativeType);
//
//		if (nativeTypeErrorCode != S_OK) continue;
//
//		// get the media type
//		GUID nativeGuid = { 0 };
//		hr = nativeType->GetGUID(MF_MT_SUBTYPE, &nativeGuid);
//
//		if (FAILED(hr)) return bestfit;
//
//		if (isMediaOk(nativeType, count)) {
//   			UINT32 num;
//			UINT32 denum;
//			hr = MFGetAttributeRatio(nativeType, MF_MT_FRAME_RATE, &num, &denum);
//
//			float framerate = ((float) num) / denum;
//			UINT32 width, height;
//			hr = MFGetAttributeSize(nativeType, MF_MT_FRAME_SIZE, &width, &height);
////            std::cout << "resolution: " << width << "x" << height << " @ " << num << "/" << denum << std::endl;
//
//			if (FAILED(hr)) return bestfit;
//
//			int error = 0;
//
//			// prefer (hugely) to get too much than too little data...
//
//			if (aWidth < width) error += (width - aWidth);
//			if (aHeight < height) error += (height - aHeight);
//			if (aWidth > width) error += (aWidth - width) * 2;
//			if (aHeight > height) error += (aHeight - height) * 2;
//
//			if (aWidth == width && aHeight == height) // ...but perfect match is a perfect match
//				error = 0;
//
//			if (besterror > error && framerate >= aFramerate)
//			{
//				besterror = error;
//				bestfit = count;
//			}
////			char temp[1024];
////			sprintf(temp, "%d x %d, %x:%x:%x:%x %d %d %d\n", width, height, nativeGuid.Data1, nativeGuid.Data2,
////                    nativeGuid.Data3, nativeGuid.Data4, bestfit == count, error, besterror);
////            std::cout << temp;
////			OutputDebugStringA(temp);
//		}
//
//		count++;
//	}
//	return bestfit;
//}


int CaptureClass::getModes(std::map<int,MediaMode> &modes) {
    HRESULT hr;
    unsigned int count = 0;

    while (true) {
        IMFMediaType * nativeType = nullptr;
        hr = mReader->GetNativeMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, count, &nativeType);
        ScopedRelease<IMFMediaType> nativeType_s(nativeType);
        if (hr != S_OK) break;

        // get the media type
        GUID nativeGuid = { 0 };
        hr = nativeType->GetGUID(MF_MT_SUBTYPE, &nativeGuid);

        if (FAILED(hr)) break;

        if (isMediaOk(nativeType, count)) {
            UINT32 num;
            UINT32 denum;
            hr = MFGetAttributeRatio(nativeType, MF_MT_FRAME_RATE, &num, &denum);
            if (FAILED(hr)) return -1;
            float framerate = ((float) num) / denum;

            UINT32 width, height;
            hr = MFGetAttributeSize(nativeType, MF_MT_FRAME_SIZE, &width, &height);
            if (FAILED(hr)) return -1;

            MediaMode m;
            m.id = count;
            m.width = width;
            m.height = height;
            m.framerate = framerate;
            modes[count] = m;

//            std::cout << "resolution: " << width << "x" << height << " @ " << num << "/" << denum << std::endl;
        }
        count++;
    }
    return 0;
}

HRESULT CaptureClass::initCapture(int device_id_, int mode_id_) {
	device_id = device_id_;
    mode_id = mode_id_;
	HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	DO_OR_DIE;

	hr = MFStartup(MF_VERSION);

	DO_OR_DIE;

	// choose device
	IMFAttributes *attributes = nullptr;
	hr = MFCreateAttributes(&attributes, 1);
	ScopedRelease<IMFAttributes> attributes_s(attributes);

	DO_OR_DIE;

	hr = attributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);

	DO_OR_DIE;

	ChooseDeviceParam param = { nullptr };
	hr = MFEnumDeviceSources(attributes, &param.mDevices, &param.mCount);

	DO_OR_DIE;

	if (param.mCount > device_id) {
		// use param.ppDevices[0]
		IMFAttributes   *dev_attributes = nullptr;
		IMFMediaType    *type = nullptr;
		EnterCriticalSection(&mCritsec);

		hr = param.mDevices[device_id]->ActivateObject(__uuidof(IMFMediaSource), (void**)&mSource);

		DO_OR_DIE_CRITSECTION;

		hr = MFCreateAttributes(&dev_attributes, 3);
		ScopedRelease<IMFAttributes> dev_attributes_s(dev_attributes);

		DO_OR_DIE_CRITSECTION;

		hr = dev_attributes->SetUINT32(MF_READWRITE_DISABLE_CONVERTERS, true);

		DO_OR_DIE_CRITSECTION;

		hr = dev_attributes->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, this);

		DO_OR_DIE_CRITSECTION;

		hr = MFCreateSourceReaderFromMediaSource(mSource, dev_attributes, &mReader);

		DO_OR_DIE_CRITSECTION;


		hr = mReader->GetNativeMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, mode_id, &type);
		ScopedRelease<IMFMediaType> type_s(type);

		DO_OR_DIE_CRITSECTION;

		hr = setVideoType(type);

		DO_OR_DIE_CRITSECTION;

		hr = mReader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, type);

		DO_OR_DIE_CRITSECTION;

		hr = mReader->ReadSample((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, nullptr, nullptr, nullptr, nullptr);

		DO_OR_DIE_CRITSECTION;

		LeaveCriticalSection(&mCritsec);
	}
	else {
		return MF_E_INVALIDINDEX;
	}

	/*
	for (i = 0; i < 16; i++)
	{
	char temp[128];
	float v;
	int f;
	int r = GetProperty(i, v, f);
	sprintf(temp, "%d: %3.3f %d (%d)\n", i, v, f, r);
	OutputDebugStringA(temp);
	}
	*/

	return 0;
}

void CaptureClass::deinitCapture()
{
	EnterCriticalSection(&mCritsec);

	mReader->Release();

	mSource->Shutdown();
	mSource->Release();

	delete[] mCaptureBuffer;

	LeaveCriticalSection(&mCritsec);
}
