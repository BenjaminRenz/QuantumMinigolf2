/*#define GLEW_STATIC
#include "libraries/GLEW_2.1.0/include/glew.h"
#include "libraries/GLFW_3.2.1/include/glfw3.h"
#include "libraries/FFTW_3.3.5/include/fftw3.h"
#include "libraries/LINMATH/include/linmath.h"
#include <time.h>
GLFWwindow* MainWindow;
*/
#include "camera_dshow.h"
#include "utf_convert.h"

#define camera_big_grid_points_x 200 //The y gridpoints will be calculated based on the aspect ratio
#define camera_testspots 1
/*
GLuint CompileShaderFromFile(char FilePath[], GLuint shaderType) {
    //read from file into heap memory
    FILE* filepointer = fopen(FilePath, "rb");               //open specified file in read only mode
    if(filepointer == NULL) {
        printf("Error: Filepointer to shaderfile at %s of type %d could not be loaded.", FilePath, shaderType);
        //return;
    }
    fseek(filepointer, 0, SEEK_END);                        //shift filePointer to EndOfFile Position to get filelength
    long filelength = ftell(filepointer);                   //get filePointer position
    fseek(filepointer, 0, SEEK_SET);                        //move file Pointer back to first line of file
    char* filestring = (char*)malloc(filelength + 1);       //
    if(fread(filestring, sizeof(char), filelength, filepointer) != filelength) {
        printf("Error: Missing characters in input string");
        //return;
    }
    if(filestring[0] == 0xEF && filestring[1] == 0xBB && filestring[2] == 0xBF) {   //Detect if file is utf8 with bom
        printf("Error: Remove the bom from your utf8 shader file");
    }
    filestring[filelength] = 0;                           //Set end of string
    fclose(filepointer);                                  //Close File
    const char* ConstFilePointer = filestring;            //opengl wants const pointer
    //compile shader with opengl
    GLuint ShaderId = glCreateShader(shaderType);
    glShaderSource(ShaderId, 1, &ConstFilePointer, NULL);
    glCompileShader(ShaderId);
    GLint compStatus = 0;
    glGetShaderiv(ShaderId, GL_COMPILE_STATUS, &compStatus);
    if(compStatus != GL_TRUE) {
        printf("Error: Compilation of shader %d failed!\n", ShaderId);
        //TODO free resources
        //return;
    }
    printf("Info: Shader %d sucessfully compiled.\n", ShaderId);
    free(filestring);                                   //Delete Shader string from heap
    fclose(filepointer);
    return ShaderId;
}

void glfw_error_callback(int error, const char* description) {
    printf("Error: GLFW-Error type: %d occurred.\nDescription: %s\n", error, description);
}

void APIENTRY openglCallbackFunction(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
    printf("Error in opengl occured!\n");
    printf("Message: %s\n", message);
    printf("type or error: ");
    switch(type) {
    case GL_DEBUG_TYPE_ERROR:
        printf("ERROR");
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        printf("DEPRECATED_BEHAVIOR");
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        printf("UNDEFINED_BEHAVIOR");
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        printf("PORTABILITY");
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        printf("PERFORMANCE");
        break;
    case GL_DEBUG_TYPE_OTHER:
        printf("OTHER");
        break;
    }
    printf("\nId:%d \n", id);
    printf("Severity:");
    switch(severity) {
    case GL_DEBUG_SEVERITY_LOW:
        printf("LOW");
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        printf("MEDIUM");
        break;
    case GL_DEBUG_SEVERITY_HIGH:
        printf("HIGH");
        break;
    }
    printf("\nGLerror end\n");
}


int startGUI(){
    glfwSetErrorCallback(glfw_error_callback);
    if(!glfwInit()) {
        return -1;
    }
    //Set window creation hints
    //glfwWindowHint(GLFW_RESIZABLE,GL_FALSE); //Uncomment to disable resizing
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    //window creation
    const GLFWvidmode* VideoMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    MainWindow = glfwCreateWindow(1280, 720, "Quantum Minigolf 2.0", NULL, NULL);   //Windowed hd ready
    //MainWindow = glfwCreateWindow(VideoMode->width, VideoMode->height, "Quantum Minigolf 2.0", glfwGetPrimaryMonitor(), NULL); //Fullscreen
    if(!MainWindow) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(MainWindow);
    //GLEW init
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if(GLEW_OK != err) {
        printf("Error: glewInit() failed.");
    }
    printf("QuantumMinigolf2\n");
    printf("Info: Using OpenGl Version: %s\n", glGetString(GL_VERSION));
    //enable v-sync
    //glfwSwapInterval(1);
    //Refister Callback for errors (debugging)
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(openglCallbackFunction, 0);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);   //Dont filter messages
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, NULL, GL_FALSE);   //Dont filter messages
    glEnable(GL_DEPTH_TEST);
}*/

HRESULT callbackForGraphview(void* inst, IMediaSample* smp) //when first called will set smp==0 and recieve a inputForBrightspotfinder Sruct to sore camera resolution and pointer for bright spot coordinates
{
    static int xres; //since we don't have controll when this callback will occure it will be initialised before giving this functionpointer to directshow to get some values passed
    static int yres;
    static float* current_xpos;
    static float* current_ypos;
    /*static int xsteps;
    static int ysteps;*/
    static int xstepwidth;
    static int ystepwidth;
    if(smp==NULL)  //Recieve initialisation not normal operation
    {
        xres=((struct inputForBrightspotfinder*)inst)->xres;
        yres=((struct inputForBrightspotfinder*)inst)->yres;
        current_xpos=((struct inputForBrightspotfinder*)inst)->cam_current_xpos;
        current_ypos=((struct inputForBrightspotfinder*)inst)->cam_current_ypos;
        /*xsteps=camera_big_grid_points_x;
        ysteps=(int)(camera_big_grid_points_x/(((float)xres)/yres));
        */
        //get aspect ratio in float and calculate the int value for the numer of grid points in y direction
        xstepwidth=xres/camera_big_grid_points_x;
        ystepwidth=yres/((int)(camera_big_grid_points_x/(((float)xres)/yres)));
        printf("Debug: xres: %d, yres %d\n",xres,yres);
        //printf("Debug: xsteps: %d, ysetps %d\n",xsteps,ysteps);
        printf("Debug: xsteps: %d, ysetps %d\n",xstepwidth,ystepwidth);
        return S_OK;
    }
    else
    {
        int bitperPixel=smp->lpVtbl->GetActualDataLength(smp)/(xres*yres);
        BYTE* pictureBuffer=NULL;
        smp->lpVtbl->GetPointer(smp,&pictureBuffer);
        //printf("DEBUG: %d",pictureBuffer[3]);
        //Rough search for five brightest spots with a big grid
        int* BrightSpots=(int*)malloc(2*sizeof(int)*camera_testspots);
        memset(BrightSpots,0,2*sizeof(int)*camera_testspots);
        for(int y_pos=0;y_pos<yres;y_pos+=ystepwidth){
            for(int x_pos=0;x_pos<xres;x_pos+=xstepwidth){
                int arrayposWithoutRGB=(x_pos*yres+y_pos);
                int birghtness=pictureBuffer[arrayposWithoutRGB*bitperPixel+2];
                if(birghtness>BrightSpots[2*(camera_testspots-1)+1]){ //check if brighter then the least bright spot
                    int i=0; //non local offset for bright spot array
                    for(;i<(camera_testspots-1);i++){ //compare to the other four brighter spots (will from this point on definitively override an existing brightspot)
                        if(birghtness>BrightSpots[2*((camera_testspots-1)-i)+1]){
                            BrightSpots[2*((camera_testspots-1)-i)]=BrightSpots[2*((camera_testspots-2)-i)];//shift less brighter spots down in the array
                            BrightSpots[2*((camera_testspots-1)-i)+1]=BrightSpots[2*((camera_testspots-2)-i)+1];//including its intensity val
                        }else{
                            break;
                        }
                    }
                    BrightSpots[2*((camera_testspots-1)-i)]=arrayposWithoutRGB;
                    BrightSpots[2*((camera_testspots-1)-i)+1]=birghtness;
                }
            }

        }
        /*int* BrightSpots=(int*)malloc(2*sizeof(int)*camera_testspots); //first val is coordinate as x_coord+y_coord*xres, second is brightness val, 0 index is the brightest, 1 index the second brightest and so on
        memset(BrightSpots,0,2*sizeof(int)*camera_testspots);
        for(int y_pos=0,y_counter=0;y_pos<yres;y_pos=((yres*++y_counter)/ysteps)){ //Loop over picture array with larger grid and find 5 brightest spots
            for(int x_pos=0,x_counter=0;x_pos<xres;x_pos=((xres*++x_counter)/xsteps)){
                int arrayposWithoutRGB=(x_pos*yres+y_pos);
                int birghtness=pictureBuffer[arrayposWithoutRGB*bitperPixel]+pictureBuffer[arrayposWithoutRGB*bitperPixel+1]+pictureBuffer[arrayposWithoutRGB*bitperPixel+2];
                if(birghtness>BrightSpots[2*(camera_testspots-1)+1]){ //check if brighter then the least bright spot
                    int i=0; //non local offset for bright spot array
                    for(;i<(camera_testspots-1);i++){ //compare to the other four brighter spots (will from this point on definitively override an existing brightspot)
                        if(birghtness>BrightSpots[2*((camera_testspots-1)-i)+1]){
                            BrightSpots[2*((camera_testspots-1)-i)]=BrightSpots[2*((camera_testspots-2)-i)];//shift less brighter spots down in the array
                            BrightSpots[2*((camera_testspots-1)-i)+1]=BrightSpots[2*((camera_testspots-2)-i)+1];//including its intensity val
                        }else{
                            break;
                        }
                    }
                    BrightSpots[2*((camera_testspots-1)-i)]=arrayposWithoutRGB;
                    BrightSpots[2*((camera_testspots-1)-i)+1]=birghtness;
                }
            }
        }*/
        /*printf("Debug: ");
        for(int i=0;i<camera_testspots;i++){
            printf("p:%d,b:%d,,,",BrightSpots[2*i],BrightSpots[2*i+1]);
        }
        printf("\n");*/

        //TODO dead pixel blocker?

        //Average brightSpot
        int averagebrightspot_x=0;
        int averagebrightspot_y=0;

        for(int brightspot=0;brightspot<camera_testspots;brightspot++){
            averagebrightspot_x+=BrightSpots[2*brightspot]%xres; //NOTE if this value does not fit then the picture is most likely not arranged as rgb or rgba, try a different camera or modify the code
            averagebrightspot_y+=BrightSpots[2*brightspot]/xres;
        }
        averagebrightspot_x/=camera_testspots;
        averagebrightspot_y/=camera_testspots; //Calculate the mean
        printf("%d\t%d\n",averagebrightspot_x,averagebrightspot_y);
        *current_xpos=averagebrightspot_x;
        *current_ypos=averagebrightspot_y;
        //Do a scanline search around this point



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
        free(BrightSpots);
        return S_OK;
    }
}

#define cameraNum 1

int main(void){
    printf("first test\n");
    unsigned int numberOfAllocatedCams=0;
    struct CameraListItem* AllAvailableCameras=getCameras(&numberOfAllocatedCams); //Get the address of our function which we wish to inject into the object for callback
    if(numberOfAllocatedCams==0){
        return 1;
    }
    struct CameraStorageObject* allRes=getAvailableCameraResolutions(&AllAvailableCameras[cameraNum]);
    printf("----Camera Info----\n");
    printf("FriendlyName: %S\n",AllAvailableCameras[cameraNum].friendlyName);
    printf("Path: %S\n",AllAvailableCameras[cameraNum].devicePath);
    printf("NumSupRes: %d",(allRes->numberOfSupportedResolutions));
    if(S_OK!=registerCameraCallback(allRes, (allRes->numberOfSupportedResolutions)-1, &callbackForGraphview)){
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
    BrightSpotInput->xres=allRes->resolutionsXYPointer[(allRes->numberOfSupportedResolutions)-1][0]; //
    BrightSpotInput->yres=allRes->resolutionsXYPointer[(allRes->numberOfSupportedResolutions)-1][1]; //Hardcoded for first camera [0][xy]
    printf("Test7\n");
    BrightSpotInput->cam_current_xpos=&cam_refreshed_xpos;
    BrightSpotInput->cam_current_ypos=&cam_refreshed_ypos;
    printf("Test8\n");
    callbackForGraphview(BrightSpotInput,NULL); //We want to pass data before first call of this function so that it knows the resolution
    printf("Test9\n");

    //3d

    //3d

    while(1)
    {
        allRes->_MediaControl->lpVtbl->Run(allRes->_MediaControl);
        cam_refreshed_xpos=0;
        cam_refreshed_ypos=0;
        while(cam_refreshed_xpos+cam_refreshed_ypos==0){}
        printf("Frame update");
        /*glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //Swap Buffers
        glFinish();
        glfwSwapBuffers(MainWindow);
        //Process Events
        glfwPollEvents();
        */
        //TODO / WARNING make sure that no other program is accessing the camera at the same time as this will result in an error!!!
    }
}
