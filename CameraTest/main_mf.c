#define WINVER 0x0601
#include <stdint.h>
#include "mfapi.h"
#include "Strmif.h"
#include "mfidl.h"  //requiered for IMFMediaSource
#include <stdio.h>
//mingw has a bug that dome GUID's are missing see: https://sourceforge.net/p/mingw-w64/bugs/770/
EXTERN_GUID(FIXED_MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, 0xc60ac5fe, 0x252a, 0x478f, 0xa0, 0xef, 0xbc, 0x8f, 0xa5, 0xf7, 0xca, 0xd3);
EXTERN_GUID(FIXED_MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID, 0x8ac3587a, 0x4ae7, 0x42d8, 0x99, 0xe0, 0x0a, 0x60, 0x13, 0xee, 0xf9, 0x0f);
void main(void){
    HRESULT hr;
    hr=CoInitializeEx(NULL,COINIT_MULTITHREADED);
    hr=MFStartup(MF_VERSION,MFSTARTUP_NOSOCKET);
    IMFMediaSource *pSource=NULL;
    IMFAttributes *pAttributes=NULL;
    IMFActivate **ppDevices=NULL;
    hr=MFCreateAttributes(&pAttributes ,1); //Holds Attributes of Interface
    hr=pAttributes->lpVtbl->SetGUID(pAttributes,&FIXED_MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,&FIXED_MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID); //Settings to serach for all available Video capture devices
    uint32_t numberOfDevices=0;
    hr=MFEnumDeviceSources(pAttributes,&ppDevices,&numberOfDevices);
    for(uint32_t dev=0;dev<numberOfDevices;dev++){
        IMFPresentationDescriptor** ppPresentationDescriptor=NULL;
        ppDevices[dev]->lpVtbl->ActivateObject(ppDevices[dev],&FIXED_MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,(void*)&pSource);
        pSource->lpVtbl->CreatePresentationDescriptor(pSource,ppPresentationDescriptor);
        //(*ppPresentationDescriptor)->lpVtbl->GetStreamDescriptorByIndex(,,);
        printf("cam\n");
    }
    //MFCreateSourceReaderFromMediaSource();
    /*
    IMFMediaSource->lpVtbl->Shutdown();
    */
}
