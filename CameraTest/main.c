#define COBJMACROS //for using dshow and lpVtbl
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
    IEnumMoniker* myCameralist=NULL;
    if(S_OK==myDeviceEnum->lpVtbl->CreateClassEnumerator(myDeviceEnum,&CLSID_VideoInputDeviceCategory, &myCameralist, 0)){
        printf("Sucessfuly enumerated VideoInputDevices...\n");
    }else{
        printf("Error: No Video Devices found\n");
        return 3;
    }
    IMoniker* myCamera=NULL;
    unsigned long numberOfFetchedCameras=0;
    while(S_OK==IEnumMoniker_Next(myCameralist,1,&myCamera,&numberOfFetchedCameras)){ //equivalent to myCameralist->lpVtbl->Next(");
        IBindCtx* myBindContext=NULL;
        CreateBindCtx(0,&myBindContext);
        IPropertyBag* myPropertyBag=NULL;
        if(S_OK!=myCamera->lpVtbl->BindToStorage(myCamera,myBindContext,NULL,&IID_IPropertyBag,(void**)&myPropertyBag)){
            return 6;
        }

        VARIANT myFieldForFriendlyName; //Do not set to =0 or we will get access violation
        VariantInit(&myFieldForFriendlyName);
        VARIANT myFieldForDevicePath;   //Do not set to =0 or we will get access violation
        VariantInit(&myFieldForDevicePath);

        myPropertyBag->lpVtbl->Read(myPropertyBag,L"FriendlyName",&myFieldForFriendlyName,0);
        myPropertyBag->lpVtbl->Read(myPropertyBag,L"DevicePath",&myFieldForDevicePath,0);
        printf("%S\n",myFieldForFriendlyName.bstrVal);
        printf("%S\n",myFieldForDevicePath.bstrVal);
    }



    return 0;
}
/*int deleteVideoDevice(){
    CoUninitialize();
}*/

int main(void){
   createVideoDevice();
}
