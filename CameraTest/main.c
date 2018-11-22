#define COBJMACROS //for using dshow and lpVtbl
#include <libavcodec/avcodec.h>
#include <dshow.h>
#include <guiddef.h>
#include <stdio.h>
#include <strmif.h>
HRESULT callbackForGraphview(void* inst, IMediaSample *smp);
HRESULT (*callbackForGraphviewFPointer)(void* inst, IMediaSample *smp); //create a function pointer which we will to inject our custom function into the RenderPinObject
HRESULT callbackForGraphview(void* inst, IMediaSample *smp){
    printf("Function has been called\n");
    return S_OK;
}
int createVideoDevice();
int createVideoDevice(){
    DWORD no;
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
    IMediaControl* myMediaControll=NULL;
    INT_PTR* p=NULL;
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
        IGraphBuilder* myGraph=NULL;
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
    //p=(INT_PTR*)&callbackForGraphviewFPointer;
    while(1){
        myMediaControll->lpVtbl->Run(myMediaControll);
    }

    return 0;
}
/*int deleteVideoDevice(){
    CoUninitialize();
}*/

int main(void){
    callbackForGraphviewFPointer=&callbackForGraphview; //Get the address of our function which we wish to inject into the object for callback

   createVideoDevice();
}
