#include "camera_dshow.h"
#include "utf_convert.h"
HRESULT callbackForGraphview(void* inst, IMediaSample* smp) //when first called will set smp==0 and recieve a inputForBrightspotfinder Sruct to sore camera resolution and pointer for bright spot coordinates
{
    static int xres; //since we don't have controll when this callback will occure it will be initialised before giving this functionpointer to directshow to get some values passed
    static int yres;
    static float* current_xpos;
    static float* current_ypos;
    if(smp==NULL)  //Recieve initialisation not normal operation
    {
        xres=((struct inputForBrightspotfinder*)inst)->xres;
        yres=((struct inputForBrightspotfinder*)inst)->yres;
        current_xpos=((struct inputForBrightspotfinder*)inst)->cam_current_xpos;
        current_ypos=((struct inputForBrightspotfinder*)inst)->cam_current_ypos;
        return;
    }
    else
    {
        BYTE* pictureBuffer=NULL;
        smp->lpVtbl->GetPointer(smp,&pictureBuffer);
        //Scan different grid sizes and find 10 brightest spots


        /*
        int brightestVal=0;
        int search_current_max_pixel=0;
        for(unsigned int pixel=0; pixel<xres*yres; pixel+=19) //*3 because of rgb
        {
            int pixel_brightness=pictureBuffer[pixel*3]+pictureBuffer[1+pixel*3]+pictureBuffer[2+pixel*3];
            if(brightestVal<pixel_brightness)
            {
                //printf("brighter one found%d",pixel/3);
                search_current_max_pixel=pixel;
                brightestVal=pixel_brightness;
            }
        }
        *current_xpos=((float)(search_current_max_pixel%xres))/xres;
        *current_ypos=((float)(search_current_max_pixel/xres))/yres;
        printf("x%f,y%f\n",*current_xpos,*current_ypos);
        //printf("%d\n",pictureBuffer[0]);
        */
        return S_OK;
    }
}

int main(void){
    unsigned int numberOfAllocatedCams=0;
    struct CameraListItem* AllAvailableCameras=getCameras(&numberOfAllocatedCams); //Get the address of our function which we wish to inject into the object for callback
    struct CameraStorageObject* allRes=getAvailableCameraResolutions(&AllAvailableCameras[0]);
    printf("----Camera Info----\n");
    printf("FriendlyName: %S\n",AllAvailableCameras[0].friendlyName);
    printf("Path: %S\n",AllAvailableCameras[0].devicePath);
    registerCameraCallback(allRes, 0, &callbackForGraphview);
    //set pointers for positions

    //WARNING! these values should not be read from when callback could happen
    //TODO!! lock these values if they are processed
    //Set values to -1 or something like that and check for that in callback to block values



    float cam_refreshed_xpos=1;
    float cam_refreshed_ypos=2;
    struct inputForBrightspotfinder* BrightSpotInput=(struct inputForBrightspotfinder*)malloc(sizeof(struct inputForBrightspotfinder));
    printf("Test6\n");

    BrightSpotInput->xres=allRes->resolutionsXYPointer[0][0]; //
    BrightSpotInput->yres=allRes->resolutionsXYPointer[0][1]; //Hardcoded for first camera [0][xy]
    printf("Test7\n");
    BrightSpotInput->cam_current_xpos=&cam_refreshed_xpos;
    BrightSpotInput->cam_current_ypos=&cam_refreshed_ypos;
    printf("Test8\n");
    callbackForGraphview(BrightSpotInput,NULL); //We want to pass data before first call of this function so that it knows the resolution
    printf("Test6\n");
    while(1)
    {
        allRes->_MediaControl->lpVtbl->Run(allRes->_MediaControl);
    }
}
