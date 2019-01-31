#include "camera_dshow.h"
#include "utf_convert.h"

#define camera_gridpoints_x 100 //The y gridpoints will be calculated based on the aspect ratio
HRESULT callbackForGraphview(void* inst, IMediaSample* smp) //when first called will set smp==0 and recieve a inputForBrightspotfinder Sruct to sore camera resolution and pointer for bright spot coordinates
{
    static int xres; //since we don't have controll when this callback will occure it will be initialised before giving this functionpointer to directshow to get some values passed
    static int yres;
    static float* current_xpos;
    static float* current_ypos;
    static int xstepsize;
    static int ystepsize;
    int fiveBrightSpot[5][2]={{0,0},{0,0},{0,0},{0,0},{0,0}}; //first val is coordinate as x_coord+y_coord*xres, second is brightness val, 0 index is the brightest, 1 index the second brightest and so on
    if(smp==NULL)  //Recieve initialisation not normal operation
    {
        xres=((struct inputForBrightspotfinder*)inst)->xres;
        yres=((struct inputForBrightspotfinder*)inst)->yres;
        current_xpos=((struct inputForBrightspotfinder*)inst)->cam_current_xpos;
        current_ypos=((struct inputForBrightspotfinder*)inst)->cam_current_ypos;
        xstepsize=xres/camera_gridpoints_x;
        ystepsize=yres/(int)(camera_gridpoints_x/(((float)xres)/yres)); //get aspect ratio in float and calculate the int value for the numer of grid points in y direction
        printf("Debug: xres: %d, yres %d\n",xres,yres);
        printf("Debug: xstep: %d, ysetp %d\n",xstepsize,ystepsize);
        return S_OK;
    }
    else
    {
        BYTE* pictureBuffer=NULL;
        smp->lpVtbl->GetPointer(smp,&pictureBuffer);
        //Rough search for five brightest spots
        for(int y_coord=0;y_coord<yres;y_coord+=ystepsize){ //Loop over picture array with larger grid and find 5 brightest spots
            for(int x_coord=0;x_coord<xres;x_coord+=xstepsize){
                int arrayposWithoutRGB=(x_coord+y_coord*xres);
                int birghtness=pictureBuffer[arrayposWithoutRGB*3]+pictureBuffer[arrayposWithoutRGB*3+1]+pictureBuffer[arrayposWithoutRGB*3+2];
                if(birghtness>fiveBrightSpot[4][1]){ //check if brighter then the least bright spot
                    int i=0; //non local offset for bright spot array
                    for(;i<4;i++){ //compare to the other four brighter spots (will from this point on definitively override an existing brightspot)
                        if(birghtness>fiveBrightSpot[3-i][1]){
                            fiveBrightSpot[4-i][0]=fiveBrightSpot[3-i][0];//shift less brighter spots down in the array
                            fiveBrightSpot[4-i][1]=fiveBrightSpot[3-i][1];//including its intensity val
                        }else{
                            break;
                        }
                    }
                    fiveBrightSpot[4-i][0]=arrayposWithoutRGB;
                    fiveBrightSpot[4-i][1]=birghtness;
                }
            }
        }
        printf("Debug: Brightest spots: ");
        for(int i=0;i<5;i++){
            printf("xy: %d, b: %d,-",fiveBrightSpot[i][0],fiveBrightSpot[i][1]);
        }
        printf("\n");
        //Check if only dead pixels

        /*
        int brightestVal=0;
        int search_current_max_pixel=0;
        for(unsigned int pixel=0; pixel<xres*yres; pixel+=19) *3 because of rgb
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
    printf("first test\n");
    unsigned int numberOfAllocatedCams=0;
    struct CameraListItem* AllAvailableCameras=getCameras(&numberOfAllocatedCams); //Get the address of our function which we wish to inject into the object for callback
    if(numberOfAllocatedCams==0){
        return 1;
    }
    struct CameraStorageObject* allRes=getAvailableCameraResolutions(&AllAvailableCameras[0]);
    printf("----Camera Info----\n");
    printf("FriendlyName: %S\n",AllAvailableCameras[0].friendlyName);
    printf("Path: %S\n",AllAvailableCameras[0].devicePath);
    if(S_OK!=registerCameraCallback(allRes, 0, &callbackForGraphview)){
        printf("ERROR: Camera in use abort!\n");
        return -2;
    }
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
    printf("Test9\n");
    while(1)
    {
        allRes->_MediaControl->lpVtbl->Run(allRes->_MediaControl);
        //TODO / WARNING make sure that no other program is accessing the camera at the same time as this will result in an error!!!
    }
}
