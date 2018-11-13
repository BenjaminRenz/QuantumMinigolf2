#include <libavcodec/avcodec.h>
#include <dshow.h>
#include <guiddef.h>
#include <stdio.h>
#include <strmif.h>
int createVideoDevice();
int createVideoDevice(){
    if(CoInitializeEx(NULL,COINIT_MULTITHREADED)==S_OK){
        printf("COM library initialized\n");
    }else{
        printf("Error while initializing COM library!\n");
        return 1;
    }
    ICreateDevEnum* myDeviceEnum=NULL;
    if(S_OK==CoCreateInstance(&CLSID_SystemDeviceEnum,NULL,CLSCTX_INPROC,&IID_ICreateDevEnum,(void **)&myDeviceEnum)){
        printf("Sucessfully created enumerator\n");
    } //Get an systemDeviceEnumerator object
    if(NULL==myDeviceEnum){
        printf("Error\n");
        return 2;
    }
    IEnumMoniker* myCameras=NULL;
    if(S_OK==myDeviceEnum->lpVtbl->CreateClassEnumerator(myDeviceEnum,&CLSID_VideoInputDeviceCategory, &myCameras, 0)){
        printf("Sucessfuly enumerated VideoInputDevices...\n");
    }else{
        printf("Error: No Video Devices found\n");
        return 3;
    }
    IMoniker* myMoniker=NULL;
    unsigned long *numberOfFetchedCameras=0;
    if(S_OK==myCameras->lpVtbl->Next(myCameras,1,&myMoniker,numberOfFetchedCameras)){
        printf("Found %lu Devices\n",*numberOfFetchedCameras);
    }


    return 0;
}
/*int deleteVideoDevice(){
    CoUninitialize();
}*/

int main(void){
   createVideoDevice();
}
