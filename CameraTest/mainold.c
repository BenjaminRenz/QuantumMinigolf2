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
        printf("Sucessfully created enumerator");
    } //Get an systemDeviceEnumerator object
    if(NULL==myDeviceEnum){
        printf("Error\n");
    }
    IEnumMoniker* myCameras=NULL;
    if(S_OK==ICreateDevEnum_CreateClassEnumerator(myDeviceEnum,CLSID_VideoInputDeviceCategory, &myCameras, 0)){
        printf("Sucessfuly enumerated VideoInputDevices...\n");
    }
    IMoniker* myMoniker=NULL;

    return 0;
}
/*int deleteVideoDevice(){
    CoUninitialize();
}*/

int main(void){
   createVideoDevice();
}
