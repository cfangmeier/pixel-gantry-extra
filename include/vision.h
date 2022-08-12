#ifndef CLIONLVTEST_VISION_H
#define CLIONLVTEST_VISION_H

#define DLLExport __declspec(dllexport)

extern "C" {
DLLExport int __cdecl calc_focus(
        char *img,
        int imgLineWidth,
        int imgWidth,
        int imgHeight,
        float *focus,
        char *log_filedir);


DLLExport int __cdecl find_patches(
        char *imgPtr,
        int imgLineWidth,
        int imgWidth,
        int imgHeight,
        int shrinkFactor, // increase to speed up routine
        float fieldOfViewX,  // mm
        float fieldOfViewY,  // mm
        int dilateSize,
        float sizeMin,  // mm^2
        float sizeMax,  // mm^2
        float aspectRatioMin,
        float aspectRatioMax,
        int colorGroups,
        bool debug,
        const char *logFileDir,
        int *numPatches,
        float *patchXCoordinates,
        float *patchYCoordinates,
        float *patchAspectRatios,
        float *patchSizes);


DLLExport int __cdecl find_circles(
        char *imgPtr,
        int imgLineWidth,
        int imgWidth,
        int imgHeight,
        int shrinkFactor, // increase to speed up routine
        float fieldOfViewX,  // mm
        float fieldOfViewY,  // mm
        float minRadius,  // mm
        float maxRadius,  // mm
        int houghGradientParam1,
        int houghGradientParam2,
        bool debug,
        const char *logFileDir,
        int *numCircles,
        float *circleXCenters,
        float *circleYCenters,
        float *circleRadii);


DLLExport int __cdecl find_rects(
        char *imgPtr,
        int imgLineWidth,
        int imgWidth,
        int imgHeight,
        int shrinkFactor,
        float fieldOfViewX,
        float fieldOfViewY,
        float nominalWidth,
        float nominalHeight,
        float tolerance,
        bool allowRotation,
        bool debug,
        const char *logFileDir,
        int *Nrects,
        float *rectXCenters,
        float *rectYCenters,
        float *rectXBLCorners,
        float *rectYBLCorners,
        float *rectXTLCorners,
        float *rectYTLCorners,
        float *rectXTRCorners,
        float *rectYTRCorners,
        float *rectXBRCorners,
        float *rectYBRCorners,
        float *rectWidths,
        float *rectHeights,
        float *rectAngles);

};  // end extern "C"
#endif //CLIONLVTEST_VISION_H
