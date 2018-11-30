//#include <libavcodec/avcodec.h>
#include <dshow.h>
#include <guiddef.h>
#include <stdio.h>
#include <strmif.h>
#include <uchar.h>

struct myCameraIdentifier{
    IMoniker* MonikerPointer;
    char32_t friendlyName[30];
    char32_t devicePath[200];
    unsigned int* SupportedResolutions;
};

size_t utf8_to_utf32(unsigned char* inputString, size_t numberOfChars, char32_t* outputString){
    size_t input_array_index=0;
    size_t output_array_index=0;
    while(input_array_index<numberOfChars){
        if(inputString[input_array_index]>=0xF0){//
            outputString[output_array_index++]=
                (((char32_t)(inputString[input_array_index]&&0x07))<<18)||
                (((char32_t)(inputString[input_array_index+1]&&0x3F))<<12)||
                (((char32_t)(inputString[input_array_index+2]&&0x3F))<<6)||
                (inputString[input_array_index+3]&&0x3F);
                input_array_index++;
        }else if(inputString[input_array_index]>=0xE0){
            outputString[output_array_index++]=
                ((char32_t)((inputString[input_array_index]&&0x0F))<<12)||
                ((char32_t)((inputString[input_array_index+1]&&0x3F))<<6)||
                (inputString[input_array_index+2]&&0x3F);
                input_array_index++;
        }else if(inputString[input_array_index]>=0xC0){
            outputString[output_array_index++]=
                ((char32_t)((inputString[input_array_index]&&0x1F))<<6)||
                (inputString[input_array_index+1]&&0x3F);
            input_array_index++;
        }else{
            outputString[output_array_index++]=
                (inputString[input_array_index++]&&0x7F);
        }
    }
    printf("Info: Converted %d utf8 to %d utf32 chars\n",(unsigned int)input_array_index,(unsigned int)output_array_index);
    return output_array_index;
}

size_t utf16_to_utf32(char16_t* inputString, size_t numberOfChar16s, char32_t* outputString){ //Make sure the supplied output buffer is able to hold at least
    size_t input_array_index=0;
    size_t output_array_index=0;
    while(input_array_index<numberOfChar16s){
        if(inputString[input_array_index]>=0xD800){//detect double characters
            outputString[output_array_index++]=(((char32_t)(inputString[input_array_index]&&0x3FF))<<10)||(inputString[++input_array_index]);
        }else{
            outputString[output_array_index++]=(char32_t)inputString[input_array_index++];
        }
    }
    printf("Info: Converted %d utf16 to %d utf32 chars\n",(unsigned int)input_array_index,(unsigned int)output_array_index);
    return output_array_index;
}

size_t utf32_cut_ASCII(char32_t* inputString, size_t numberOfChar32s, unsigned char* outputString){
    size_t output_array_index=0;
    for(size_t input_array_index=0;input_array_index<numberOfChar32s;input_array_index++){
        outputString[output_array_index++]=inputString[input_array_index++]&&0x7F;
    }
    return output_array_index;
}

/* This function shall be called with a NULL pointer to initialize and return all available cameras as structs. The user then should pick one camera and
deallocate other cameras witch have not been selected*/
void closeCameras(){
    CoUninitialize(); //Must be called once for every CoInitialize(Ex) to unload the dll

}
void getAvailableCameraResolutions(struct myCameraIdentifier* Camera){
    HRESULT hr=0;
    IPin* pin=NULL;
}

struct myCameraIdentifier* getCameras(unsigned int* numberOfCameras){ //TODO change to return char list with names
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
    myDeviceEnum->lpVtbl->Release(myDeviceEnum);//Release the DeviceEnumerator after we have retrieved all available Video Devices
    if(hr!=S_OK){
        return 0;
    }else{
        printf("Info: Successfully created VideoInputEnumerator\n");
    }
    IMoniker* myCamera=NULL;
    unsigned long numberOfFetchedCamerasPerRun=0;
    struct myCameraIdentifier* camerasIdentPointer=NULL;        //create a pointer for the CameraIdentifier-structs we wish to alloacate
    while(S_OK==myCameralist->lpVtbl->Next(myCameralist,1,&myCamera,&numberOfFetchedCamerasPerRun)){ //TODO replace numverOfFetched with nullptr
        camerasIdentPointer=(struct myCameraIdentifier*) realloc(camerasIdentPointer,((*numberOfCameras)+1)*sizeof(struct myCameraIdentifier));
        camerasIdentPointer[*numberOfCameras].MonikerPointer=myCamera;//Store the moniker object for later use
        IBindCtx* myBindContext=NULL;
        hr=CreateBindCtx(0,&myBindContext);
        IPropertyBag* myPropertyBag=NULL;
        VARIANT VariantField; //Do not set to =0 or we will get access violation
        VariantInit(&VariantField);

        //Get specific data such as name and device path for camera
        myCamera->lpVtbl->BindToStorage(myCamera,myBindContext,NULL,&IID_IPropertyBag,(void**)&myPropertyBag);
        myPropertyBag->lpVtbl->Read(myPropertyBag,L"FriendlyName",&VariantField,0);
        memcpy(camerasIdentPointer[*numberOfCameras].friendlyName, VariantField.bstrVal ,29*sizeof(char32_t ));//Fill our structs with info and leve last character as null string terminator
        camerasIdentPointer[*numberOfCameras].friendlyName[29]=0; //set string termination character
        myPropertyBag->lpVtbl->Read(myPropertyBag,L"DevicePath",&VariantField,0);
        memcpy(camerasIdentPointer[*numberOfCameras].devicePath,VariantField.bstrVal,199*sizeof(char32_t ));
        camerasIdentPointer[*numberOfCameras].friendlyName[199]=0; //make sure we have terminated the string
        //Increment
        (*numberOfCameras)++;
        //Cleanup
        VariantClear(&VariantField);
        myPropertyBag->lpVtbl->Release(myPropertyBag);
        myBindContext->lpVtbl->Release(myBindContext); //After we got the device path and name we release the Context
    }
    printf("Info: Detected %d Cameras\n",*numberOfCameras);
    return camerasIdentPointer;

    /*IGraphBuilder* myGraph=NULL;
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

    return 0;*/
}

/*void registerCameraCallback(){
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

    return;
}*/

HRESULT callbackForGraphview(void* inst, IMediaSample *smp);
HRESULT (*callbackForGraphviewFPointer)(void* inst, IMediaSample *smp); //create a function pointer which we will to inject our custom function into the RenderPinObject
HRESULT callbackForGraphview(void* inst, IMediaSample *smp){
    BYTE* pictureBuffer=NULL;
    smp->lpVtbl->GetPointer(smp,&pictureBuffer);
    printf("%d\n",pictureBuffer[0]);
    return S_OK;
}




//int deleteVideoDevice(){
//    CoUninitialize();
//
int main(void){
    struct myCameraIdentifier* AllAvailableCameras=NULL;
    unsigned int numberOfAllocatedCams=0;
    AllAvailableCameras=getCameras(&numberOfAllocatedCams); //Get the address of our function which we wish to inject into the object for callback
    printf("FriendlyName: %S\n",AllAvailableCameras[0].friendlyName);
    printf("Path: %S\n",AllAvailableCameras[0].devicePath);
}
