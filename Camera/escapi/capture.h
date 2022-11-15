#pragma once

#include "conversion.h"
#include "mediamode.h"
#include <map>

class CaptureClass : public IMFSourceReaderCallback {
public:

	CaptureClass();
	~CaptureClass();
	STDMETHODIMP QueryInterface(REFIID aRiid, void** aPpv) override;
	STDMETHODIMP_(ULONG) AddRef() override;
	STDMETHODIMP_(ULONG) Release() override;
	STDMETHODIMP OnReadSample(HRESULT aStatus, DWORD aStreamIndex, DWORD aStreamFlags,
                              LONGLONG aTimestamp, IMFSample *aSample) override;
	STDMETHODIMP OnEvent(DWORD, IMFMediaEvent *) override;
	STDMETHODIMP OnFlush(DWORD) override;
	static int escapiPropToMFProp(int aProperty);
	int setProperty(int aProperty, float aValue, int aAuto);
	int getProperty(int aProperty, float &aValue, int &aAuto);
	static bool isFormatSupported(REFGUID aSubtype) ;
	HRESULT getFormat(DWORD aIndex, GUID *aSubtype) const;
	HRESULT setConversionFunction(REFGUID aSubtype);
	HRESULT setVideoType(IMFMediaType *aType);
	int isMediaOk(IMFMediaType *aType, int aIndex);
    int getModes(std::map<int,MediaMode> &modes);
	HRESULT initCapture(int aDevice, int mode_id);
	void deinitCapture();

	long                    mRefCount;        // Reference count.
	CRITICAL_SECTION        mCritsec;

	IMFSourceReader         *mReader;
	IMFMediaSource			*mSource;

	LONG                    mDefaultStride;
	IMAGE_TRANSFORM_FN      mConvertFn;    // Function to convert the video to RGB32

	unsigned int			*mCaptureBuffer;
	int			            mCaptureBufferWidth, mCaptureBufferHeight;
	int						mErrorLine;
	int						mErrorCode;
	int						device_id;
    int			            mode_id;

	int			           *mBadIndex;
	int			            mBadIndices;
	int			            mMaxBadIndices;
	int						mRedoFromStart;
};
