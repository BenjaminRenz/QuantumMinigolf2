#include <libavcodec/avcodec.h>
#include <dshow.h>
#include <guiddef.h>
#include <stdio.h>
#include <strmif.h>
struct myCameraIdentifier{
    char friendlyName[30];
    char devicePath[100];
    unsigned int* SupportedResolutions;
};
/* This function shall be called with a NULL pointer to initialize and return all available cameras as structs. The user then should pick one camera and
deallocate other cameras witch have not been selected*/
struct myCameraIdentifier* getCameras(struct myCameraIdentifier* CameraIdent){ //TODO change to return char list with names
    if((intptr_t) CameraIdent==0){
        HRESULT hr=0;
        hr=CoInitializeEx(NULL,COINIT_MULTITHREADED);
        if(hr!=S_OK){
            return 0;
        }else{
            printf("Info: Successfully initialized COM library\n");
        }
        ICreateDevEnum* myDeviceEnum=NULL;
        hr=CoCreateInstance(&CLSID_SystemDeviceEnum,NULL,CLSCTX_INPROC,&IID_ICreateDevEnum,(void **)&myDeviceEnum);
        if(hr!=S_OK){
            return 0;
        }else{
            printf("Info: Successfully created DeviceEnumerator\n");
        }
        IEnumMoniker* myCameralist=NULL;
        hr=myDeviceEnum->lpVtbl->CreateClassEnumerator(myDeviceEnum,&CLSID_VideoInputDeviceCategory, &myCameralist, 0);
        if(hr!=S_OK){
            return 0;
        }else{
            printf("Info: Successfully created VideoInputEnumerator\n");
        }
        IMoniker* myCamera=NULL;
        unsigned long numberOfFetchedCamerasPerRun=0;
        unsigned int numberOfStructsToAllocate=0;
        while(S_OK!=myCameralist->lpVtbl->Next(myCameralist,1,&myCamera,&numberOfFetchedCamerasPerRun)){
            numberOfStructsToAllocate++;
        }
        struct myCameraIdentifier* cameraIdentPointer=NULL;        //create a pointer for the CameraIdentifier-structs we wish to alloacate
        cameraIdentPointer=(struct myCameraIdentifier*) malloc(numberOfStructsToAllocate*sizeof(struct myCameraIdentifier));
        myCameralist->lpVtbl->Reset(myCameralist);
        while(S_OK!=myCameralist->lpVtbl->Next(myCameralist,1,&myCamera,&numberOfFetchedCamerasPerRun)){
            IBindCtx* myBindContext=NULL;
            hr=CreateBindCtx(0,&myBindContext);
            IPropertyBag* myPropertyBag=NULL;
            VARIANT myFieldForFriendlyName; //Do not set to =0 or we will get access violation
            VariantInit(&myFieldForFriendlyName);
            VARIANT myFieldForDevicePath;   //Do not set to =0 or we will get access violation
            VariantInit(&myFieldForDevicePath);
            //Get specific data such as name and device path for camera
            myCamera->lpVtbl->BindToStorage(myCamera,myBindContext,NULL,&IID_IPropertyBag,(void**)&myPropertyBag);
            myPropertyBag->lpVtbl->Read(myPropertyBag,L"FriendlyName",&myFieldForFriendlyName,0);
            myPropertyBag->lpVtbl->Read(myPropertyBag,L"DevicePath",&myFieldForDevicePath,0);
            memcpy(cameraIdentPointer[--numberOfStructsToAllocate].friendlyName, myFieldForFriendlyName.bstrVal ,29);//Fill our structs with info
            memcpy(cameraIdentPointer[numberOfStructsToAllocate].devicePath,myFieldForDevicePath.bstrVal,99);
            //TODO get supported resolutions
            myBindContext->lpVtbl->Release(myBindContext);
        }

    IGraphBuilder* myGraph=NULL;
    CoCreateInstance(&CLSID_FilterGraph,NULL,CLSCTX_INPROC,&IID_IGraphBuilder,(void **)&myGraph);
    IMediaControl* myMediaControll=NULL;
    INT_PTR* p=0;
    CoCreateInstance(&CLSID_FilterGraph,NULL,CLSCTX_INPROC,&IID_IGraphBuilder,(void **)&myGraph);
    myGraph->lpVtbl->QueryInterface(myGraph,&IID_IMediaControl,(void**)&myMediaControll);

        //Create Filter
        IBaseFilter* myCameraGraphBaseObj=NULL;
        CreateBindCtx(0,&myBindContext);
        myCamera->lpVtbl->BindToObject(myCamera,myBindContext,NULL,&IID_IBaseFilter,(void **)&myCameraGraphBaseObj);
        myGraph->lpVtbl->AddFilter(myGraph,myCameraGraphBaseObj, L"Capture Source");
        IEnumPins* myOutputpins=0;
        myCameraGraphBaseObj->lpVtbl->EnumPins(myCameraGraphBaseObj,&myOutputpins);
        IPin* myOutputpin=0;
        unsigned long numberOfFetchedOutputPins=0;
        myOutputpins->lpVtbl->Next(myOutputpins,1,&myOutputpin,&numberOfFetchedOutputPins); //get one output pin
        myGraph->lpVtbl->Render(myGraph,myOutputpin); //Render this output

        IEnumFilters* myFilter=NULL;
        myGraph->lpVtbl->EnumFilters(myGraph,&myFilter);
        IBaseFilter* rnd=NULL;
        myFilter->lpVtbl->Next(myFilter,1,&rnd,0);
        IEnumPins* myRenderPins=0;
        rnd->lpVtbl->EnumPins(rnd,&myRenderPins);
        IPin* myRenderPin=0;
        myRenderPins->lpVtbl->Next(myRenderPins,1,&myRenderPin, 0);
        IMemInputPin* myMemoryInputPin= 0;
        myRenderPin->lpVtbl->QueryInterface(myRenderPin,&IID_IMemInputPin,(void**)&myMemoryInputPin);
        p=6+*(INT_PTR**)myMemoryInputPin; //Get the function pointer for Recieve() of myRenderPin which we will use later to "inject" out own function pointer to redirect the output of the previous filter
        VirtualProtect(p,4,PAGE_EXECUTE_READWRITE,&no);//To allow the write from our thread because the graph lives in a seperate thread
    }
    *p=(INT_PTR*)callbackForGraphviewFPointer;
    while(1){
        myMediaControll->lpVtbl->Run(myMediaControll);
    }

    return 0;
}



HRESULT callbackForGraphview(void* inst, IMediaSample *smp);
HRESULT (*callbackForGraphviewFPointer)(void* inst, IMediaSample *smp); //create a function pointer which we will to inject our custom function into the RenderPinObject
HRESULT callbackForGraphview(void* inst, IMediaSample *smp){
    BYTE* pictureBuffer=NULL;
    smp->lpVtbl->GetPointer(smp,&pictureBuffer);
    printf("%d\n",pictureBuffer[0]);
    return S_OK;
}




///*int deleteVideoDevice(){
//    CoUninitialize();
//}*/
//
//int main(void){
//    getCameras(getCameras_init_and_return_list); //Get the address of our function which we wish to inject into the object for callback
//
//   createVideoDevice();
//}
