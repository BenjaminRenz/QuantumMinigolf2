#define COBJMACROS //for using dshow and lpVtbl
#include <libavcodec/avcodec.h>
#include <dshow.h>
#include <guiddef.h>
#include <stdio.h>
#include <strmif.h>
int createVideoDevice();
int createVideoDevice(){
    //Used code from https://www.codeproject.com/Articles/12869/Real-time-video-image-processing-frame-grabber-usi
    //from Ladislav Nevery under "The Code Project Open License"
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
        myBindContext->lpVtbl->Release(myBindContext);

        //TODO only do this after we selected a camera
        //Create Graph
        IGraphBuilder* graph=NULL;
        CoCreateInstance(&CLSID_FilterGraph,NULL,CLSCTX_INPROC,&IID_IGraphBuilder,(void **)&graph);
        //Create Filter
        IBaseFilter* myCameraGraphBaseObj=NULL;
        CreateBindCtx(0,&myBindContext);
        myCamera->lpVtbl->BindToObject(myCamera,myBindContext,NULL,&IID_IBaseFilter,(void **)&myCameraGraphBaseObj);
        graph->lpVtbl->AddFilter(graph,myCameraGraphBaseObj, L"Capture Source");
        IEnumPins* myOutputpins=0;
        myCameraGraphBaseObj->lpVtbl->EnumPins(myCameraGraphBaseObj,&myOutputpins);
        IPin* myOutputpin=0;
        unsigned long numberOfFetchedOutputPins=0;
        myOutputpins->lpVtbl->Next(myOutputpins,1,&myOutputpin,&numberOfFetchedOutputPins); //get one output pin
        graph->lpVtbl->Render(graph,myOutputpin); //Render this output

        IEnumFilters* myFilter=NULL;
        graph->lpVtbl->EnumFilters(graph,&myFilter);
        IBaseFilter* rnd=NULL;
        myFilter->lpVtbl->Next(myFilter,1,&rnd,0);
        IEnumPins* myRenderPins=0;
        rnd->lpVtbl->EnumPins(rnd,&myRenderPins);
        IPin* myRenderPin=0;
        myRenderPins->lpVtbl->Next(myRenderPins,1,&myRenderPin, 0);
        IMemInputPin* myMemoryInputPin= 0;
        myRenderPin->lpVtbl->QueryInterface(myRenderPin,&IID_IMemInputPin,(void**)&myMemoryInputPin);

    }



    return 0;
}
/*int deleteVideoDevice(){
    CoUninitialize();
}*/

int main(void){
   createVideoDevice();
}
