#ifndef CAMERA_DSHOW_INCLUDED
#define CAMERA_DSHOW_INCLUDED


#include <dshow.h>
#include <guiddef.h>
#include <stdio.h>
#include <strmif.h>
#include <uchar.h>
#include <stddef.h>

struct inputForBrightspotfinder
{
    float* cam_current_xpos;
    float* cam_current_ypos;
    int xres;
    int yres;
};

struct CameraListItem
{
    IMoniker* MonikerPointer;
    char32_t friendlyName[30];
    char32_t devicePath[200];
};

struct CameraStorageObject
{
    IMoniker* MonikerPointer;
    char32_t friendlyName[30];
    char32_t devicePath[200];
    IGraphBuilder* _CameraGraph;
    IMediaControl* _MediaControl;
    IAMCameraControl* _CameraControl;
    unsigned int numberOfSupportedResolutions;
    unsigned long** resolutionsXYPointer;  //treat as if it would be a 2d array e.g.: resolutionsXYPointer[resolutionNum][0] for width ... [width=0,height=1]
    AM_MEDIA_TYPE** _amMediaPointerArray;
    IAMStreamConfig* _StreamCfg;
    IPin* _outputpinPointer;
};

/* This function shall be called with a NULL pointer to initialize and return all available cameras as structs. The user then should pick one camera and
deallocate other cameras witch have not been selected*/
void closeCameras(struct CameraStorageObject* Camera);
HRESULT callbackForGraphview(void* inst, IMediaSample* smp);
HRESULT (*callbackForGraphviewFPointer)(void* inst, IMediaSample* smp); //create a function pointer which we will to inject our custom function into the RenderPinObject

/** \brief call this before any other function, gets available cameras to the system
 *
 * \param pointer to an int in which the number of connected cameras is stored
 * \return pointer to a list of <numberOfCameras> CameraListItems
 *
 */
struct CameraListItem* getCameras(unsigned int* numberOfCameras);


struct CameraStorageObject* getAvailableCameraResolutions(struct CameraListItem* CameraInList);
int registerCameraCallback(struct CameraStorageObject* CameraIn,int selectedResolution,INT_PTR* callbackForGraphviewFPointer);  //selected resolution is position in array
#endif
