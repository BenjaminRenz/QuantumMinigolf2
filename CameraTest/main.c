//#include <libavcodec/avcodec.h>
#include <dshow.h>
#include <guiddef.h>
#include <stdio.h>
#include <strmif.h>
#include <uchar.h>

int camResolutionX=0; //needed for callback function because we can't specify any parameters
int camResolutionY=0;

struct inputForBrightspotfinder{
    float* cam_current_xpos;
    float* cam_current_ypos;
    int xres;
    int yres;
};

struct CameraListItem{
    IMoniker* MonikerPointer;
    char32_t friendlyName[30];
    char32_t devicePath[200];
};

struct CameraStorageObject{
    IMoniker* MonikerPointer;
    char32_t friendlyName[30];
    char32_t devicePath[200];
    IGraphBuilder* _CameraGraph;
    IMediaControl* _MediaControl;
    unsigned int numberOfSupportedResolutions;
    unsigned long** resolutionsXYPointer;  //treat as if it would be a 2d array e.g.: resolutionsXYPointer[resolutionNum][0] for width ... [width=0,height=1]
    AM_MEDIA_TYPE** _amMediaPointerArray;
    IAMStreamConfig* _StreamCfg;
    IPin* _outputpinPointer;
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

size_t utf16_to_utf32(char16_t* inputString, size_t numberOfChar16s, char32_t* outputString){ //Make sure the supplied output buffer is able to hold at least TODO characters
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
        outputString[output_array_index++]=inputString[input_array_index]&&0x7F;
    }
    return output_array_index;
}

/* This function shall be called with a NULL pointer to initialize and return all available cameras as structs. The user then should pick one camera and
deallocate other cameras witch have not been selected*/
void closeCameras(struct CameraStorageObject* Camera){
    Camera->_MediaControl->lpVtbl->Stop(Camera->_MediaControl);

    // Enumerate the filters in the graph.
    IEnumFilters* FilterEnum = NULL;
    if (S_OK==Camera->_CameraGraph->lpVtbl->EnumFilters(Camera->_CameraGraph,&FilterEnum))
    {
        IBaseFilter* Filter = NULL;
        while (S_OK == FilterEnum->lpVtbl->Next(FilterEnum,1, &Filter, NULL))
        {
            // Remove the filter.
            Camera->_CameraGraph->lpVtbl->RemoveFilter(Camera->_CameraGraph,Filter);
            // Reset the enumerator.
            FilterEnum->lpVtbl->Reset(FilterEnum);
            Filter->lpVtbl->Release(Filter);
        }
        FilterEnum->lpVtbl->Release(FilterEnum);
    }
    CoUninitialize(); //Must be called once for every CoInitialize(Ex) to unload the dll
}

HRESULT callbackForGraphview(void* inst, IMediaSample* smp);
HRESULT (*callbackForGraphviewFPointer)(void* inst, IMediaSample* smp); //create a function pointer which we will to inject our custom function into the RenderPinObject
HRESULT callbackForGraphview(void* inst, IMediaSample* smp){
    static int xres;
    static int yres;
    static float* current_xpos;
    static float* current_ypos;
    if(smp==NULL){ //Recieve initialisation not normal operation
        xres=((struct inputForBrightspotfinder*)inst)->xres;
        yres=((struct inputForBrightspotfinder*)inst)->yres;
        current_xpos=((struct inputForBrightspotfinder*)inst)->cam_current_xpos;
        current_ypos=((struct inputForBrightspotfinder*)inst)->cam_current_ypos;
        return;
    }else{
        BYTE* pictureBuffer=NULL;
        smp->lpVtbl->GetPointer(smp,&pictureBuffer);
        int brightestVal=0;
        int search_current_max_pixel=0;
        for(unsigned int pixel=0;pixel<xres*yres;pixel+=19){ //*3 because of rgb
            int pixel_brightness=pictureBuffer[pixel*3]+pictureBuffer[1+pixel*3]+pictureBuffer[2+pixel*3];
            if(brightestVal<pixel_brightness){
                //printf("brighter one found%d",pixel/3);
                search_current_max_pixel=pixel;
                brightestVal=pixel_brightness;
            }
        }
        *current_xpos=((float)(search_current_max_pixel%xres))/xres;
        *current_ypos=((float)(search_current_max_pixel/xres))/yres;
        printf("x%f,y%f\n",*current_xpos,*current_ypos);
        //printf("%d\n",pictureBuffer[0]);
        return S_OK;
    }
}


struct CameraListItem* getCameras(unsigned int* numberOfCameras){ //TODO change to return char list with names
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
    struct CameraListItem* CameraListItem=NULL;        //create a pointer for the CameraIdentifier-structs we wish to alloacate
    while(S_OK==myCameralist->lpVtbl->Next(myCameralist,1,&myCamera,NULL)){
        CameraListItem=(struct CameraListItem*) realloc(CameraListItem,((*numberOfCameras)+1)*sizeof(struct CameraListItem));
        CameraListItem[*numberOfCameras].MonikerPointer=myCamera;//Store the moniker object for later use
        //IBindCtx* myBindContext=NULL;
        //hr=CreateBindCtx(0,&myBindContext);
        IPropertyBag* myPropertyBag=NULL;
        VARIANT VariantField; //Do not set to =0 or we will get access violation
        VariantInit(&VariantField);

        //Get specific data such as name and device path for camera
        hr=myCamera->lpVtbl->BindToStorage(myCamera,0,NULL,&IID_IPropertyBag,(void**)&myPropertyBag); //TODO test if it also works with a nullpointer
        myPropertyBag->lpVtbl->Read(myPropertyBag,L"FriendlyName",&VariantField,0);
        memcpy(CameraListItem[*numberOfCameras].friendlyName, VariantField.bstrVal ,28*sizeof(char32_t ));//Fill our structs with info and leve last character as null string terminator
        CameraListItem[*numberOfCameras].friendlyName[29]=0; //set string termination character
        myPropertyBag->lpVtbl->Read(myPropertyBag,L"DevicePath",&VariantField,0);
        memcpy(CameraListItem[*numberOfCameras].devicePath,VariantField.bstrVal,198*sizeof(char32_t ));
        CameraListItem[*numberOfCameras].friendlyName[199]=0; //make sure we have terminated the string
        //Increment
        (*numberOfCameras)++;
        //Cleanup
        VariantClear(&VariantField);
        myPropertyBag->lpVtbl->Release(myPropertyBag);
        //myBindContext->lpVtbl->Release(myBindContext); //After we got the device path and name we release the Context
    }
    printf("Info: Detected %d Camera(s)\n",*numberOfCameras);
    return CameraListItem;
}

struct CameraStorageObject* getAvailableCameraResolutions(struct CameraListItem* CameraInList){
    struct CameraStorageObject* CameraOut=(struct CameraStorageObject*) malloc(sizeof(struct CameraStorageObject));

    //Create Graph and Filter
    //Instantiate Graph to get access to FilterObjects
    CameraOut->_CameraGraph=NULL;
    CoCreateInstance(&CLSID_FilterGraph,NULL,CLSCTX_INPROC,&IID_IGraphBuilder,(void **)&CameraOut->_CameraGraph); //tested working
    //Create Control for Graph
    CameraOut->_MediaControl=NULL;
    CameraOut->_CameraGraph->lpVtbl->QueryInterface(CameraOut->_CameraGraph,&IID_IMediaControl,(void**)&CameraOut->_MediaControl);
    //Create Filter
    IBaseFilter* CameraFilter=NULL;
    IBindCtx* myBindContext=NULL;
    CreateBindCtx(0,&myBindContext);//TODO TEST
    CameraInList->MonikerPointer->lpVtbl->BindToObject(CameraInList->MonikerPointer,NULL,NULL,&IID_IBaseFilter,(void **)&CameraFilter); //Do not swap this and the line below, it will not work!
    CameraOut->_CameraGraph->lpVtbl->AddFilter(CameraOut->_CameraGraph, CameraFilter, L"Capture Source");
    IEnumPins* CameraOutputPins=0;
    CameraFilter->lpVtbl->EnumPins(CameraFilter,&CameraOutputPins);
    //get output pin of CameraInputFilter
    IPin* pOutputPin=(IPin*) malloc(sizeof(IPin));
    CameraOutputPins->lpVtbl->Next(CameraOutputPins,1,&pOutputPin,0);
    CameraOut->_outputpinPointer=pOutputPin;
    //Get Resolution
    //get pin configuration-interface
    IAMStreamConfig* StreamCfg=(IAMStreamConfig*)malloc(sizeof(IAMStreamConfig));
    pOutputPin->lpVtbl->QueryInterface(pOutputPin,&IID_IAMStreamConfig, (void**) &StreamCfg);
    CameraOut->_StreamCfg=StreamCfg;
    //Get size of config-structure for pin
    int numberOfPossibleRes=0;
    int sizeOfCFGStructureInByte=0;
    StreamCfg->lpVtbl->GetNumberOfCapabilities(StreamCfg,&numberOfPossibleRes,&sizeOfCFGStructureInByte);
    printf("Info: found %d possible formats\n",numberOfPossibleRes);
    byte* pUnusedSSC=(byte*) malloc(sizeof(byte)*sizeOfCFGStructureInByte);
    //get VideoSteamConfigSturcure
    CameraOut->_amMediaPointerArray=(AM_MEDIA_TYPE**) malloc(sizeof(AM_MEDIA_TYPE)*numberOfPossibleRes);
    long** resolutionPointerArray=(long**) malloc((2*sizeof(long)+sizeof(long*))*numberOfPossibleRes); //we create an array for x,y resolution (sizeof(long)*2) which contains pointers to the first location of every pair to be accessed as regular 2d array (size_t size of general pointer)
    for(int numOfRes=0;numOfRes<numberOfPossibleRes;numOfRes++){
        AM_MEDIA_TYPE* pAmMedia=NULL;
        StreamCfg->lpVtbl->GetStreamCaps(StreamCfg,numOfRes,&pAmMedia,pUnusedSSC);
        CameraOut->_amMediaPointerArray[numOfRes]=pAmMedia;
        printf("\nInfo: Returning Info for item %d of %d\n",numOfRes+1,numberOfPossibleRes);
        if(pAmMedia->formattype.Data1==FORMAT_VideoInfo.Data1 && (pAmMedia->cbFormat >= sizeof(VIDEOINFOHEADER)) && pAmMedia->pbFormat!=NULL){ //Check if right format, If the space at pointer location is valid memory and if the pointer is actually filled with something
            VIDEOINFOHEADER* pVideoInfoHead=(VIDEOINFOHEADER*) pAmMedia->pbFormat; //Get video info header
            resolutionPointerArray[numOfRes]=((long*)(resolutionPointerArray+numberOfPossibleRes))+(numOfRes*2); //point to the first element of the resolution tuples stored after the pointer table
            resolutionPointerArray[numOfRes][0]=pVideoInfoHead->bmiHeader.biWidth;
            resolutionPointerArray[numOfRes][1]=pVideoInfoHead->bmiHeader.biHeight;
            printf("Info width:%d\n",resolutionPointerArray[numOfRes][0]);
            printf("Info height:%d\n",resolutionPointerArray[numOfRes][1]);
        }else{
            printf("Warn: unusable format, resolution won't be extracted!\n");
            resolutionPointerArray[numOfRes]=((long*)(resolutionPointerArray+numberOfPossibleRes))+(numOfRes*2);
            resolutionPointerArray[numOfRes][0]=0;
            resolutionPointerArray[numOfRes][1]=0;
        }
        //if(numberOfPossibleRes)
        //Code checks for format
        /*if(AmMedia->majortype.Data1!=MEDIATYPE_Video.Data1){
            //Free this MEDIATYPE_FORMAT
            if(AmMedia->pbFormat!=0){ //Block with detailed format description
                CoTaskMemFree((void*)AmMedia->pbFormat);
            }
            if(AmMedia->pUnk!=NULL){
                AmMedia->pUnk->lpVtbl->Release(AmMedia->pUnk);
            }
            CoTaskMemFree(AmMedia);
            printf("Info: Free unused Format\n");
        }else{
            printf("1:%d\n",AmMedia->subtype.Data1);
            printf("2:%d\n",AmMedia->subtype.Data2);
            printf("3:%d\n",AmMedia->subtype.Data3);
            printf("4:%d\n\n",AmMedia->subtype.Data4);
            printf("MediaSubtype:%d\n",MEDIASUBTYPE_RGB24.Data1);
        }
        //DeleteMediaType
        printf("test%d\n\n",numberOfPossibleRes);
        */

    }
    CameraOut->resolutionsXYPointer=resolutionPointerArray;
    free(pUnusedSSC);
    printf("returned");
    return CameraOut;
}

void registerCameraCallback(struct CameraStorageObject* CameraIn,int selectedResolution,INT_PTR* callbackForGraphviewFPointer){ //selected resolution is position in array
    DWORD no;
    INT_PTR* p=0; //will be pointer to the input function of the render filter
    CameraIn->_StreamCfg->lpVtbl->SetFormat(CameraIn->_StreamCfg,(CameraIn->_amMediaPointerArray[selectedResolution]));
    //Free unused formats
    for(int formatIter=0;formatIter<CameraIn->numberOfSupportedResolutions;formatIter++){
        if(formatIter==selectedResolution){
            continue;
        }
        if((CameraIn->_amMediaPointerArray[selectedResolution])->pbFormat!=0){ //Block with detailed format description
            CoTaskMemFree((void*)(CameraIn->_amMediaPointerArray[selectedResolution])->pbFormat);
        }
        if((CameraIn->_amMediaPointerArray[selectedResolution])->pUnk!=NULL){
            (CameraIn->_amMediaPointerArray[selectedResolution])->pUnk->lpVtbl->Release(CameraIn->_amMediaPointerArray[selectedResolution]->pUnk);
        }
        CoTaskMemFree((CameraIn->_amMediaPointerArray[selectedResolution]));
        printf("Info: Free unused Format\n");
    }
    CameraIn->_CameraGraph->lpVtbl->Render(CameraIn->_CameraGraph,CameraIn->_outputpinPointer); //Render this output
    //get renderPin and hijack the method which recieves the inputdata from the last filter in the graph
    IEnumFilters* myFilter=NULL;
    CameraIn->_CameraGraph->lpVtbl->EnumFilters(CameraIn->_CameraGraph,&myFilter);//OK
    IBaseFilter* rnd=NULL;
    myFilter->lpVtbl->Next(myFilter,1,&rnd,0);//Does not work, no filter recieved
    IEnumPins* myRenderPins=0;
    rnd->lpVtbl->EnumPins(rnd,&myRenderPins);
    IPin* myRenderPin=0;
    myRenderPins->lpVtbl->Next(myRenderPins,1,&myRenderPin, 0);
    IMemInputPin* myMemoryInputPin=NULL;
    myRenderPin->lpVtbl->QueryInterface(myRenderPin,&IID_IMemInputPin,(void**)&myMemoryInputPin);
    p=6+*(INT_PTR**)myMemoryInputPin; //Get the function pointer for Recieve() of myRenderPin which we will use later to "inject" out own function pointer to redirect the output of the previous filter
    VirtualProtect(p,4,PAGE_EXECUTE_READWRITE,&no);//To allow the write from our thread because the graph lives in a seperate thread
    //Hide the (now empty/black) popup window
    IVideoWindow* myVideoWindow=NULL;
    CameraIn->_CameraGraph->lpVtbl->QueryInterface(CameraIn->_CameraGraph,&IID_IVideoWindow,&myVideoWindow);
    CameraIn->_MediaControl->lpVtbl->Run(CameraIn->_MediaControl);
    myVideoWindow->lpVtbl->put_Visible(myVideoWindow,0);
    *p=callbackForGraphviewFPointer;
    return;
}




//TODO
//int deleteVideoDevice(){
//    CoUninitialize();
//
int main(void){
    unsigned int numberOfAllocatedCams=0;
    struct CameraListItem* AllAvailableCameras=getCameras(&numberOfAllocatedCams); //Get the address of our function which we wish to inject into the object for callback
    struct CameraStorageObject* allRes=getAvailableCameraResolutions(&AllAvailableCameras[0]);
    printf("FriendlyName: %S\n",AllAvailableCameras[0].friendlyName);
    printf("Path: %S\n",AllAvailableCameras[0].devicePath);
    registerCameraCallback(allRes, 0, &callbackForGraphview);
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
    while(1){
        allRes->_MediaControl->lpVtbl->Run(allRes->_MediaControl);
    }

}
