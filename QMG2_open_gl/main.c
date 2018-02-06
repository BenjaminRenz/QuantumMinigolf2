/*TODO list
-compile GLEW as static library
-implement webcam brightest spot detection https://www.pyimagesearch.com/2014/09/29/finding-brightest-spot-image-using-python-opencv/
*/
#define GLEW_STATIC
#include "libraries/GLEW_2.1.0/include/glew.h"
#include "libraries/GLFW_3.2.1/include/glfw3.h"
#include "libraries/FFTW_3.3.5/include/fftw3.h"
#include "libraries/LINMATH/include/linmath.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <math.h>
#define PI 3.14159265358979323846
#include <limits.h>
#ifdef _WIN32
#define filepath_gui_bmp ".//GUI.bmp"
#define filepath_potential_bmp ".//512template.bmp"
#elif __linux__
#define filepath_gui_bmp "./GUI.bmp"
#define filepath_potential_bmp "./double_slit512.bmp"
#endif



#define G_OBJECT_INIT 0
#define G_OBJECT_DRAW 1
#define G_OBJECT_UPDATE 2

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow* window, int button,int action, int mods);
void drop_file_callback(GLFWwindow* window, int count, const char** paths);
void mouse_scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void windows_size_callback(GLFWwindow* window, int width, int height);
void glfw_error_callback(int error, const char* description);
unsigned char* read_bmp(char* filepath);
void write_bmp(char* filepath, unsigned int width, unsigned int height);
float update_delta_time();
void APIENTRY openglCallbackFunction(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar* message,const void* userParam);
void drawGui(int G_OBJECT_STATE,float aspectRatio);
void drawPlaneAndGrid(int G_OBJECT_STATE,unsigned int PlaneResolution,unsigned int GridResolution, mat4x4 mvp4x4);
GLuint CompileShaderFromFile(char FilePath[],GLuint shaderType);
//global variables section
float FOV=0.7f;
GLFWwindow* MainWindow;
GLuint psiTexture;
float delta_time;
int numberOfGuiEelements;
struct GUI_render* guiElementsStorage;
#define Resolution 512  //should be power of 2
#define PlaneRes 1024    //must be power of 2
#define GridRes 256        //must be power of 2

//GUI texture atlas
//Slider
#define GUI_TYPE_UNUSED 0
#define GUI_TYPE_SLIDER 1

#define UV_SLIDER_BAR_TOP_LEFT_X 1.0f/16.0f
#define UV_SLIDER_BAR_TOP_LEFT_Y 63.0f/64.0f
#define UV_SLIDER_BAR_DOWN_RIGHT_X 9.0f/16.0f
#define UV_SLIDER_BAR_DOWN_RIGHT_Y 1.0f

#define UV_SLIDER_BUTTON_TOP_LEFT_X 0.0f
#define UV_SLIDER_BUTTON_TOP_LEFT_Y 15.0f/16.0f
#define UV_SLIDER_BUTTON_DOWN_RIGHT_X 1.0f/16.0f
#define UV_SLIDER_BUTTON_DOWN_RIGHT_Y 1.0f

//Button
#define GUI_TYPE_BUTTON 2


struct GUI_render {
    float top_left_x;       //between [0,(9/16 or 3/4)]
    float top_left_y;       //between [0,1]
    float percentOfWidth;   //between [0,1.0f]
    int GUI_TYPE;
    float position;
};

struct GUI {
    int Left_up_x;
    int Left_up_y;
    int Width;
    int Height;
    int Position;
};

struct GUI Button_new;
struct GUI Button_measure;
struct GUI Button_esc;
struct GUI Slider_speed;
struct GUI Slider_size;

//Manipulation parameters
#define Size_start 50-1
#define Diameter_change 10
double diameter = Size_start*2.5f;
#define norm Resolution*Resolution
float Movement_angle = PI/2.0f;
#define Offset_change 10
#define offset_x_start Resolution/2
int offset_x = offset_x_start;
#define offset_y_start 40
int offset_y = offset_y_start;
int measurement = 2;
int particle = 0;
int pos = 0;
int draw=1;
int momentum_prop=1;
#define Speed_start 10
#define Speed_change 5
float dt = (Speed_start+1)*0.0000005f;

int main(int argc, char* argv[]) {
    //GUI INIT
    numberOfGuiEelements=1;
    guiElementsStorage=malloc(100*sizeof(struct GUI_render));
    //Screen coordinates from x[-1.0f,1.0f] y[-1.0f,1.0f]
    guiElementsStorage[0].top_left_x=0.0f;
    guiElementsStorage[0].top_left_y=0.5f;
    guiElementsStorage[0].position=0.5f;
    guiElementsStorage[0].percentOfWidth=0.2f;
    guiElementsStorage[0].GUI_TYPE=GUI_TYPE_SLIDER;
    //GLFW init
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        return -1;
    }

    //Set window creation hints
    //glfwWindowHint(GLFW_RESIZABLE,GL_FALSE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT,GL_TRUE);

    //window creation
    const GLFWvidmode* VideoMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    MainWindow = glfwCreateWindow(1600, 900, "Quantum Minigolf 2.0", NULL, NULL);
    //MainWindow = glfwCreateWindow(VideoMode->width, VideoMode->height, "Quantum Minigolf 2.0", glfwGetPrimaryMonitor(), NULL);
    if (!MainWindow) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(MainWindow);

    //GLEW init
    glewExperimental=GL_TRUE;
    GLenum err = glewInit();
    if(GLEW_OK != err) {
        printf("Error: glewInit() failed.");
    }
    printf("QuantumMinigolf2\n");
    printf("Info: Using OpenGl Version: %s\n",glGetString(GL_VERSION));
    //enable v-sync
    //glfwSwapInterval(1);
    //Refister Callback for errors (debugging)
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(openglCallbackFunction,0);
    glDebugMessageControl(GL_DONT_CARE,GL_DONT_CARE,GL_DONT_CARE,0,NULL,GL_TRUE); //Dont filter messages

    //Register Callbacks for user input
    glfwSetKeyCallback(MainWindow,key_callback);
    glfwSetMouseButtonCallback(MainWindow, mouse_button_callback);
    glfwSetDropCallback(MainWindow,drop_file_callback);
    glfwSetScrollCallback(MainWindow,mouse_scroll_callback);
    glfwSetWindowSizeCallback(MainWindow,windows_size_callback);
    //Get window height
    {
        int maxIndices,maxVertices,maxTexSize,maxTexBufferSize;
        glGetIntegerv(GL_MAX_ELEMENTS_INDICES,&maxIndices);
        glGetIntegerv(GL_MAX_ELEMENTS_VERTICES,&maxVertices);
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
        glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, &maxTexBufferSize);
        printf("Info: maxIndicesPerBuffer:%d\nInfo: maxVertexPerBuffer:%d\nInfo: maxTextureSize:%d\nInfo: maxBufferSize:%d\n",maxIndices,maxVertices,maxTexSize,maxTexBufferSize);
    }
    //Set background color
    glClearColor(0.4f,0.4f,0.4f,1.0f);
    //Enable z checking
    glEnable(GL_DEPTH_TEST);

    /*//https://www.seas.upenn.edu/%7Epcozzi/OpenGLInsights/OpenGLInsights-AsynchronousBufferTransfers.pdf
    //Generate data memory for psi
    unsigned char* psi=malloc(Resolution*Resolution*4);
    //generate Texture
    glActiveTexture(GL_TEXTURE0);
    GLuint psi_texture=0;
    glGenTextures(1,&psi_texture);
    glBindTexture(GL_TEXTURE_2D,psi_texture);
    //generate pbo
    GLuint PBO1=0;
    GLuint PBO2=0;
    glGenBuffers(1,&PBO1);
    glGenBuffers(1,&PBO2);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER,PBO1);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,GL_UNSIGNED_INT_8_8_8_8,speicher);
    glBufferData(GL_PIXEL_UNPACK_BUFFER,4*Resolution*Resolution,psi,GL_STREAM_DRAW);

    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,Resolution,Resolution,GL_RGBA,GL_UNSIGNED_INT_8_8_8_8_REV,NULL); //NULL pointer let opengl fetch data from bound GL_PIXEL_UNPACK_BUFFER
    */
    //glTexSubImage2D(GL_TEXTURE_2D,,0,0,0,Resolution,Resolution,GL_UNSIGNED_INT_8_8_8_8_REV);//4 upadte every frame

    double rotation_up_down=PI;
    double rotation_left_right=PI;
    mat4x4 mvp4x4;
    mat4x4 persp4x4; //remove?? TODO
    vec3 eye_vec= {1.0f,1.0f,1.0f};
    vec3 cent_vec= {0.0f,0.0f,0.0f};
    vec3 up_vec= {0.0f,0.0f,1.0f};

    fftw_complex *psi;
    fftw_complex *psi_transform;
    fftw_complex *prop;
    psi = (fftw_complex*) fftw_alloc_complex(Resolution*Resolution);
    psi_transform = (fftw_complex*) fftw_alloc_complex(Resolution*Resolution);
    prop = (fftw_complex*) fftw_alloc_complex(Resolution*Resolution);
    fftw_plan fft = fftw_plan_dft_2d (Resolution, Resolution, psi, psi_transform, FFTW_FORWARD, FFTW_MEASURE);
    fftw_plan ifft = fftw_plan_dft_2d (Resolution, Resolution, psi_transform, psi, FFTW_BACKWARD, FFTW_MEASURE);

    //GUI
    Button_new.Left_up_x=0; Button_new.Left_up_y=200; Button_new.Width=200; Button_new.Height=100;

    Button_measure.Left_up_x=0; Button_measure.Left_up_y=300; Button_measure.Width=200; Button_measure.Height=100;

    Button_esc.Left_up_x=1720; Button_esc.Left_up_y=0; Button_esc.Width=200; Button_esc.Height=100;

    Slider_speed.Left_up_x=0; Slider_speed.Left_up_y=100; Slider_speed.Width=200; Slider_speed.Height=100; Slider_speed.Position=Speed_start;

    Slider_size.Left_up_x=0; Slider_size.Left_up_y=0; Slider_size.Width=200; Slider_size.Height=100; Slider_size.Position=Size_start;

    unsigned char* speicher = calloc(Resolution*Resolution*4,1);
    unsigned char* pot=read_bmp(filepath_potential_bmp);
    double* potential=malloc(Resolution*Resolution*sizeof(double));

    for(int i=0; i<Resolution*Resolution; i++) {
        potential[i]=(255-pot[4*i+1])/255.0f;
    }
    //Create wave
    delta_time = update_delta_time();

    unsigned int measure_win_x=Resolution/2;
    unsigned int measure_win_y=Resolution-100;
    //@@Graphics
    //init texture for Psi because its dynamic
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1,&psiTexture);
    glBindTexture(GL_TEXTURE_2D,psiTexture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //Init plane and grid
    drawPlaneAndGrid(G_OBJECT_INIT,PlaneRes,GridRes,NULL); //mvp4x4 useless here
    printf("Info: Generation of plane and grid successfull!\n");

    //Init gui
    drawGui(G_OBJECT_INIT,0);  //Initialize Gui with GL_OBJECT_INIT,aspect ratio
    //Test remove
    drawGui(G_OBJECT_UPDATE,VideoMode->width/(float)VideoMode->height);
    printf("Info: Generation of gui successfull!\n");
    //Graphics@@
    while (!glfwWindowShouldClose(MainWindow)) {
        if(glfwGetKey(MainWindow,GLFW_KEY_W)==GLFW_PRESS) {
            if(rotation_up_down<(3.0)) {
                rotation_up_down=rotation_up_down+delta_time;
            }
        }
        if(glfwGetKey(MainWindow,GLFW_KEY_S)==GLFW_PRESS) {
            if(rotation_up_down>(-0.0)) {
                rotation_up_down=rotation_up_down-delta_time;
            }
            if(rotation_up_down<(-0.0)){
                rotation_up_down=0.0;
            }
        }
        //atan(rotation_up_down);
        if(glfwGetKey(MainWindow,GLFW_KEY_D)==GLFW_PRESS) {
            if(rotation_left_right>(-PI)) {
                rotation_left_right=rotation_left_right-delta_time;
            } else {
                rotation_left_right=PI;
            }
        }
        if(glfwGetKey(MainWindow,GLFW_KEY_A)==GLFW_PRESS) {
            if(rotation_left_right<PI) {
                rotation_left_right=rotation_left_right+delta_time;
            } else {
                rotation_left_right=-PI;
            }
        }

        if(measurement == 0) {
            for(int i=0;i<1;i++){
                fftw_execute(fft);
                //momentum space
                for(int i=0; i<Resolution*Resolution; i++) {
                    double psi_re_temp = psi_transform[i][0];
                    psi_transform[i][0] = psi_re_temp*prop[i][0]-psi_transform[i][1]*prop[i][1];
                    psi_transform[i][1] = psi_re_temp*prop[i][1]+psi_transform[i][1]*prop[i][0];
                }
                fftw_execute(ifft);
                for(int i=0; i<Resolution*Resolution; i++) {
                    psi[i][0]=psi[i][0]/(double)(norm);
                    psi[i][1]=psi[i][1]/(double)(norm);
                }
                for(int i=0; i<Resolution*Resolution; i++) {
                    double psi_re_temp = psi[i][0];
                    psi[i][0] = psi_re_temp*cos(potential[i])-psi[i][1]*sin(potential[i]);
                    psi[i][1] = psi_re_temp*sin(potential[i])+psi[i][1]*cos(potential[i]);
                }

                for(int i=0; i<Resolution; i++) {
                    psi[i][0]=0;
                    psi[i][1]=0;
                    psi[i+(Resolution-1)*Resolution][0]=0;
                    psi[i+(Resolution-1)*Resolution][1]=0;
                }

                for(int i=0; i<Resolution; i++) {
                    psi[1+i*Resolution][0]=0;
                    psi[1+i*Resolution][1]=0;
                    psi[Resolution-1+i*Resolution][0]=0;
                    psi[Resolution-1+i*Resolution][1]=0;
                }
            }
        }

        if(measurement==1) {
            if(particle==0){
                srand((long)10000.0f*glfwGetTime());
                double random=(rand()%1001)/1000.0f;
                double sum=0;
                double norm_sum=0;
                for(int i=0; i<Resolution*Resolution; i++) {
                    norm_sum=norm_sum+(psi[i][0]*psi[i][0]+psi[i][1]*psi[i][1]);
                }
                for(pos=0; pos<Resolution*Resolution; pos++) {
                    sum=sum+((psi[pos][0]*psi[pos][0]+psi[pos][1]*psi[pos][1])/norm_sum);
                    if(sum>random) {
                        //printf("sum%f\n",sum);
                        //printf("rand: %f\n",random);
                        break;
                    }
                }
                for(int i=0;i<Resolution*Resolution;i++){
                    psi[i][0]=sqrt(psi[i][0]*psi[i][0]+psi[i][1]*psi[i][1]);
                    psi[i][1]=0;
                }
            }

            for(int j=0; j<Resolution; j++) {
                double radius_squared;
                for(int k=0; k<Resolution;k++){
                    //psi[k+j*Resolution][0]=psi[k+j*Resolution][0]*exp(-(((pos%Resolution)-k)*((pos%Resolution)-k)+((pos/Resolution)-j)*((pos/Resolution)-j))/1000.0f);
                    radius_squared=((pos%Resolution)-k)*((pos%Resolution)-k)+((pos/Resolution)-j)*((pos/Resolution)-j);
                    if(particle>20){
                        if(radius_squared<3){
                            psi[k+j*Resolution][0]+=exp(-(((pos%Resolution)-k)*((pos%Resolution)-k)+((pos/Resolution)-j)*((pos/Resolution)-j))/100.0f)/*(1.0-psi[k+j*Resolution][0])*(1.0-psi[k+j*Resolution][0])*/*0.01;
                        }
                        else{
                            psi[k+j*Resolution][0]=0.99*psi[k+j*Resolution][0];
                        }
                    }
                    else{
                        psi[k+j*Resolution][0]=0.99*psi[k+j*Resolution][0];
                    }
                }
            }
            particle++;
            if(particle==300){
                /*for(int j=0; j<Resolution; j++) {
                    for(int k=0; k<Resolution;k++){
                        if(sqrt(((pos%Resolution)-k)*((pos%Resolution)-k)+((pos/Resolution)-j)*((pos/Resolution)-j))<3){
                            psi[k+j*Resolution][0]=1;
                            if(sqrt((((measure_win_x+measure_win_y*Resolution)%Resolution)-k)*(((measure_win_x+measure_win_y*Resolution)%Resolution)-k)+(((measure_win_x+measure_win_y*Resolution)/Resolution)-j)*(((measure_win_x+measure_win_y*Resolution)/Resolution)-j))<100){
                                psi[k+j*Resolution][1]=1;
                            }
                            else{
                                psi[k+j*Resolution][1]=0;
                            }
                        }
                        else{
                            psi[k+j*Resolution][0]=0;
                            psi[k+j*Resolution][1]=0;
                        }
                    }
                }*/
                particle = 0;
                measurement = 5;
            }
            //measurement=5;
        }

        int biggest=0;
        biggest=0;

        for(int i=0; i<Resolution*Resolution; i++) {
            if(psi[i][0]*psi[i][0]+psi[i][1]*psi[i][1]>psi[biggest][0]*psi[biggest][0]+psi[biggest][1]*psi[biggest][1])
                biggest=i;
        }

        double norming=sqrt(1.0f/(psi[biggest][0]*psi[biggest][0]+psi[biggest][1]*psi[biggest][1]));

        for(int i=0; i<Resolution*Resolution; i++) {
            speicher[i*4+2]=(unsigned char) (0.5f*255*(psi[i][0]*norming+1.0f));
            speicher[i*4+1]=(unsigned char) (0.5f*255*(psi[i][1]*norming+1.0f));
            speicher[i*4+3]=pot[i*4+1];
        }
        //@@Graphics
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D,psiTexture);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,Resolution,Resolution,0,GL_RGBA,GL_UNSIGNED_INT_8_8_8_8,speicher);
        //Graphics@@

        delta_time = update_delta_time();

        if(draw==1){
            for(int j=0; j<Resolution; j++) {
                for(int i=0; i<Resolution; i++) {
                    psi[i+j*Resolution][0]=exp(-((i-offset_x)*(i-offset_x)+(j-offset_y)*(j-offset_y))/diameter)*cos(((i-Resolution/(float)2)*cos(Movement_angle)+(j-Resolution/(float)2)*sin(Movement_angle))*8.0f);
                    psi[i+j*Resolution][1]=exp(-((i-offset_x)*(i-offset_x)+(j-offset_y)*(j-offset_y))/diameter)*sin(((i-Resolution/(float)2)*cos(Movement_angle)+(j-Resolution/(float)2)*sin(Movement_angle))*8.0f);
                }
            }
            draw=0;
        }

        if(momentum_prop==1){
            for(int x=0; x<Resolution/2; x++) {
                for(int y=0; y<Resolution/2; y++) {
                    prop[x*Resolution+y][0] = cos(dt*(-x*x - y*y));
                    prop[x*Resolution+y][1] = sin(dt*(-x*x - y*y));
                }
                for(int y=Resolution/2; y<Resolution; y++) {
                    prop[x*Resolution+y][0] = cos(dt*(-x*x - (y-Resolution)*(y-Resolution)));
                    prop[x*Resolution+y][1] = sin(dt*(-x*x - (y-Resolution)*(y-Resolution)));
                }
            }
            for(int x=Resolution/2; x<Resolution; x++) {
                for(int y=0; y<Resolution/2; y++) {
                    prop[x*Resolution+y][0] = cos(dt*(-(x-Resolution)*(x-Resolution) - y*y));
                    prop[x*Resolution+y][1] = sin(dt*(-(x-Resolution)*(x-Resolution) - y*y));
                }
                for(int y=Resolution/2; y<Resolution; y++) {
                    prop[x*Resolution+y][0] = cos(dt*(-(x-Resolution)*(x-Resolution) - (y-Resolution)*(y-Resolution)));
                    prop[x*Resolution+y][1] = sin(dt*(-(x-Resolution)*(x-Resolution) - (y-Resolution)*(y-Resolution)));
                }
            }
            momentum_prop=0;
        }

        //camera projection an transformation matrix calculation
        eye_vec[0]=1.5f*sin(rotation_left_right)*cos(atan(rotation_up_down));
        eye_vec[1]=1.5f*cos(rotation_left_right)*cos(atan(rotation_up_down));
        eye_vec[2]=1.5f*sin(atan(rotation_up_down));
        mat4x4_look_at(mvp4x4,eye_vec,cent_vec,up_vec);
        mat4x4_perspective(persp4x4,FOV,16.0f/9.0f,0.5f,10.0f);
        mat4x4_mul(mvp4x4,persp4x4,mvp4x4);


        /*concept for async texture upload
        glBindTexture(GL_TEXTURE_2D, psi_texture);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER,PBO1);
        glTexSubImage2D(GL_TEXTURE_2D,0,0,0,Resolution,Resolution,GL_BGRA,GL_UNSIGNED_INT_8_8_8_8,NULL);
        //Make PBO2 ready to recieve new data
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER,PBO2);
        glBufferData(GL_PIXEL_UNPACK_BUFFER,4*Resolution*Resolution,0,GL_STREAM_DRAW);
        GLuint* data = glMapBuffer(GL_PIXEL_UNPACK_BUFFER,0,4*Resolution*Resolution,GL_MAP_WRITE_BIT);//Map buffer on gpu to client address space : offset 0,data,allow to write to buffer
        //write to data

        glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER); //start upload
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER,0);
        {//swap pixel buffers
            GLuint temp=PBO1;
            PBO1=PBO2;*0.5+0.5
            PBO2=temp;
        }*/

        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        drawPlaneAndGrid(G_OBJECT_DRAW,PlaneRes,GridRes,mvp4x4);
        drawGui(G_OBJECT_DRAW,0);
        //Swap Buffers
        glFinish();
        glfwSwapBuffers(MainWindow);
        //Process Events
        glfwPollEvents();
    }
    glfwTerminate();
    fftw_destroy_plan(fft);
    fftw_free(psi);
    return 0;
}
void drawPlaneAndGrid(int G_OBJECT_STATE,unsigned int PlaneResolution,unsigned int GridResolution, mat4x4 mvp4x4){
    //3d object info
    //uses Texture0 which is constantly updated
    static GLint maxSupportedIndices=0;
    static unsigned long indexBufferCountPlane=0;
    static unsigned long indexBufferCountGrid=0;
    static GLuint VboVerticesId=0;
    static GLuint* iboPlanePointer=0;
    static GLuint* iboGridPointer=0;
    static long indicesInLastPlaneBuffer=0;
    static long indicesInLastGridBuffer=0;
    static GLuint mvpMatrixUniform=0;
    static GLuint renderGridOrPlaneUniform=0;
    static GLuint gridAndPlaneShaderID=0;
    if(G_OBJECT_STATE==G_OBJECT_INIT){
        //Compile Shaders
        gridAndPlaneShaderID = glCreateProgram();              //create program to run on GPU
        glAttachShader(gridAndPlaneShaderID, CompileShaderFromFile(".\\res\\shaders\\vertex_graph.glsl",GL_VERTEX_SHADER));        //attach vertex shader to new program
        glAttachShader(gridAndPlaneShaderID, CompileShaderFromFile(".\\res\\shaders\\fragment_graph.glsl",GL_FRAGMENT_SHADER));       //attach fragment shader to new program
        glLinkProgram(gridAndPlaneShaderID);

        //Get Shader Variables
        glUseProgram(gridAndPlaneShaderID);
        mvpMatrixUniform=glGetUniformLocation(gridAndPlaneShaderID,"MVPmatrix");//only callable after glUseProgramm has been called once
        renderGridOrPlaneUniform=glGetUniformLocation(gridAndPlaneShaderID,"potential_true");

        //Texture initialisation for GL_TEXTURE0 (texture is generated outside because we need its address elsewhere
        glUniform1i(glGetUniformLocation(gridAndPlaneShaderID,"texture0"),0);

        //input checking
        if(((PlaneResolution&(PlaneResolution-1))!=0)||((GridResolution&(GridResolution-1))!=0)) {      //Check if plane resolution/grid resolution is power of 2
            printf("Error: Resolution of plane or grid is not a power of 2.\nError: Initialization failed!");
            return;
        }
        {
            GLint maxSupportedVertices=0;
            glGetIntegerv(GL_MAX_ELEMENTS_VERTICES,&maxSupportedVertices);
            if(maxSupportedVertices<1048576){
                printf("Error: Vertex Resolution of your Graphic Card are to low. (Required Minimum 1024*1024)\nError: Initialization failed!");
                return;
            }
        }


        //update Value of max supported indices
        glGetIntegerv(GL_MAX_ELEMENTS_INDICES,&maxSupportedIndices);

        //Calculate requiered buffercount
        if(((PlaneResolution-1)*(PlaneResolution-1)*6)%((maxSupportedIndices/3)*3)==0) {   //
            indexBufferCountPlane=(((PlaneResolution-1)*(PlaneResolution-1)*6)/((maxSupportedIndices/3)*3));
        } else {
            indexBufferCountPlane=(((PlaneResolution-1)*(PlaneResolution-1)*6)/((maxSupportedIndices/3)*3))+1;
        }
        if(((GridResolution-1)*(GridResolution-1)*8)%((maxSupportedIndices/2)*2)==0) {
            indexBufferCountGrid=(((GridResolution-1)*(GridResolution-1)*8)/((maxSupportedIndices/2)*2));
        } else {
            indexBufferCountGrid=(((GridResolution-1)*(GridResolution-1)*8)/((maxSupportedIndices/2)*2))+1;
        }
        //generate arrays
        iboPlanePointer=malloc(indexBufferCountPlane*sizeof(GLuint));
        iboGridPointer=malloc(indexBufferCountGrid*sizeof(GLuint));
        //calculate vertex count and offset if grid- and plane-resolution are different
        unsigned int finalVertexResolution=0;
        unsigned int gridOffsetMultiplier=0;
        unsigned int planeOffsetMultiplier=0;
        if(PlaneResolution>GridResolution) {
            finalVertexResolution=PlaneResolution;
            gridOffsetMultiplier=PlaneResolution/GridResolution;
            planeOffsetMultiplier=1;
        } else {
            finalVertexResolution=GridResolution;
            planeOffsetMultiplier=GridResolution/PlaneResolution;
            gridOffsetMultiplier=1;
        }
        printf("Info: gridMul %d\nInfo: planeMul %d\n",gridOffsetMultiplier,planeOffsetMultiplier);
        //Generate Vertices for grid and plane
        float* plane_vertex_data=malloc(2*finalVertexResolution*finalVertexResolution*sizeof(float));
        unsigned long tempVertIndex=0;
        for(int y=0; y<finalVertexResolution; y++) {
            for(int x=0; x<finalVertexResolution; x++) {
                //Vector coordinates (x,y,z)
                plane_vertex_data[tempVertIndex++]=(((float)x)/(finalVertexResolution-1))-0.5f;
                plane_vertex_data[tempVertIndex++]=(((float)y)/(finalVertexResolution-1))-0.5f; //Set height (y) to zero
            }
        }
        glGenBuffers(1, &VboVerticesId);                                                          //create buffer
        glBindBuffer(GL_ARRAY_BUFFER, VboVerticesId);                                            //Link buffer
        glBufferData(GL_ARRAY_BUFFER, 2*finalVertexResolution*finalVertexResolution*sizeof(float),plane_vertex_data,GL_STATIC_DRAW);    //Upload data to Buffer, Vertex data is set only once and drawn regularly, hence we use GL_STATIC_DRAW
        free(plane_vertex_data);    //we no longer need plane_vertex_data because it has been uploaded to gpu memory
        printf("Info: vertices for plane/grid successfully generated\n");

        //Generate Vertex Indices for Plane
        GLuint* plane_indices = malloc((finalVertexResolution-1)*(finalVertexResolution-1)*8*sizeof(GLuint)); //6 from the points of two triangles, 8 from 4 lines per gridcell max(6,8)=8
        tempVertIndex=0;
        for(unsigned int y=0; y<(finalVertexResolution-planeOffsetMultiplier); y+=planeOffsetMultiplier) {
            for(unsigned int x=0; x<(finalVertexResolution-planeOffsetMultiplier); x+=planeOffsetMultiplier) {
                //Generate first triangle
                plane_indices[tempVertIndex++]=x+(y*finalVertexResolution);   //Vertex lower left first triangle
                plane_indices[tempVertIndex++]=x+planeOffsetMultiplier+(y*finalVertexResolution);//Vertex upper right first triangle
                plane_indices[tempVertIndex++]=x+((y+planeOffsetMultiplier)*finalVertexResolution); //Vertex upper left first triangle
                //Generate second triangle
                plane_indices[tempVertIndex++]=x+planeOffsetMultiplier+(y*finalVertexResolution);   //Vertex lower left second triangle
                plane_indices[tempVertIndex++]=x+planeOffsetMultiplier+((y+planeOffsetMultiplier)*finalVertexResolution); //Vertex lower right second triangle
                plane_indices[tempVertIndex++]=x+((y+planeOffsetMultiplier)*finalVertexResolution); //Vertex upper right first triangle
                //printf("vert%d,%d,%d,%d,%d,%d\n",plane_indices[vert_index-6],plane_indices[vert_index-5],plane_indices[vert_index-4],plane_indices[vert_index-3],plane_indices[vert_index-2],plane_indices[vert_index-1]);
            }
        }

        glGenBuffers(indexBufferCountPlane,iboPlanePointer);
        //Now upload this data to GPU
        unsigned int bufferNumber=0;
        for(; bufferNumber<(indexBufferCountPlane-1); bufferNumber++) { //Upload all but the last buffer
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,iboPlanePointer[bufferNumber]);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER,(maxSupportedIndices/3)*3*sizeof(GLuint),plane_indices+(maxSupportedIndices/3)*3*bufferNumber,GL_STATIC_DRAW);
        }
        //Upload the last Buffer for plane
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,iboPlanePointer[bufferNumber]);
        if(tempVertIndex%((maxSupportedIndices/3)*3)!=0) {
            glBufferData(GL_ELEMENT_ARRAY_BUFFER,(tempVertIndex%((maxSupportedIndices/3)*3))*sizeof(GLuint),plane_indices+((maxSupportedIndices/3)*3)*bufferNumber,GL_STATIC_DRAW);
        } else {
            glBufferData(GL_ELEMENT_ARRAY_BUFFER,((maxSupportedIndices/3)*3)*sizeof(GLuint),plane_indices+((maxSupportedIndices/3)*3)*bufferNumber,GL_STATIC_DRAW);
        }
        indicesInLastPlaneBuffer=tempVertIndex%((maxSupportedIndices/3)*3);
        printf("Info: indices for plane successfully generated\n");

        //Generate Vertex Indices for Grid
        tempVertIndex=0;
        for(unsigned int y=0; y<(finalVertexResolution-gridOffsetMultiplier); y+=gridOffsetMultiplier) {
            for(unsigned int x=0; x<(finalVertexResolution-gridOffsetMultiplier); x+=gridOffsetMultiplier) {
                //Generate first line
                plane_indices[tempVertIndex++]=x+(y*finalVertexResolution);
                plane_indices[tempVertIndex++]=x+gridOffsetMultiplier+(y*finalVertexResolution);
                //Generate second line
                plane_indices[tempVertIndex++]=x+(y*finalVertexResolution);
                plane_indices[tempVertIndex++]=x+((y+gridOffsetMultiplier)*finalVertexResolution);
                //Generate third line
                plane_indices[tempVertIndex++]=x+((y+gridOffsetMultiplier)*finalVertexResolution);
                plane_indices[tempVertIndex++]=x+gridOffsetMultiplier+((y+gridOffsetMultiplier)*finalVertexResolution);
                //Generate fourth line
                plane_indices[tempVertIndex++]=x+gridOffsetMultiplier+(y*finalVertexResolution);
                plane_indices[tempVertIndex++]=x+gridOffsetMultiplier+((y+gridOffsetMultiplier)*finalVertexResolution);
            }
        }

        glGenBuffers(indexBufferCountGrid,iboGridPointer);   //Skip over 2*int+(2*long==4*int)=6
        bufferNumber=0;
        for(; bufferNumber<(indexBufferCountGrid-1); bufferNumber++) { //Upload all but the last buffer
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,iboGridPointer[bufferNumber]);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER,((maxSupportedIndices/2)*2)*sizeof(GLuint),plane_indices+((maxSupportedIndices/2)*2)*bufferNumber,GL_STATIC_DRAW);
        }
        //Upload the last Buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,iboGridPointer[bufferNumber]);
        if(tempVertIndex%((maxSupportedIndices/2)*2)!=0) {
            glBufferData(GL_ELEMENT_ARRAY_BUFFER,(tempVertIndex%((maxSupportedIndices/2)*2))*sizeof(GLuint),plane_indices+((maxSupportedIndices/2)*2)*bufferNumber,GL_STATIC_DRAW);
        } else {
            glBufferData(GL_ELEMENT_ARRAY_BUFFER,((maxSupportedIndices/2)*2)*sizeof(GLuint),plane_indices+((maxSupportedIndices/2)*2)*bufferNumber,GL_STATIC_DRAW);
        }
        indicesInLastGridBuffer=tempVertIndex%((maxSupportedIndices/2)*2);
        printf("Info: indices for grid successfully generated\n");
        free(plane_indices);                //Cleanup Array for indices
    }else if(G_OBJECT_STATE==G_OBJECT_DRAW){
        //Draw Call for Grid and Plane, this does not use vao because we have multiple GL_ELEMENT_ARRAY_BUFFER to draw only one object
        glBindBuffer(GL_ARRAY_BUFFER,VboVerticesId);
        glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,0);
        glEnableVertexAttribArray(0);//x,y
        //Enabler Shader
        glUseProgram(gridAndPlaneShaderID);
        //Set Shader Uniforms to render Grid
        glUniformMatrix4fv(mvpMatrixUniform,1,GL_FALSE,(GLfloat*)mvp4x4);
        glUniform1f(renderGridOrPlaneUniform,1.0f);
        unsigned int buffernumber;
        //Draw all vertices but those in the last index buffer
        for(buffernumber=0; buffernumber<(indexBufferCountGrid-1); buffernumber++) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,iboGridPointer[buffernumber]);
            glDrawElements(GL_LINES,(maxSupportedIndices/2)*2,GL_UNSIGNED_INT,0);
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,iboGridPointer[buffernumber]);
        glDrawElements(GL_LINES,indicesInLastGridBuffer,GL_UNSIGNED_INT,0);

        //enable Transparency TODO remove?
        //glEnable(GL_BLEND);
        //glBlendFunc(GL_ONE,GL_ONE);
        //glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

        //Render Plane
        glUniform1f(renderGridOrPlaneUniform,0.0f);
        for(buffernumber=0; buffernumber<(indexBufferCountPlane-1); buffernumber++) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,iboPlanePointer[buffernumber]);
            glDrawElements(GL_TRIANGLES,(maxSupportedIndices/3)*3,GL_UNSIGNED_INT,0);
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,iboPlanePointer[buffernumber]);
        glDrawElements(GL_TRIANGLES,indicesInLastPlaneBuffer,GL_UNSIGNED_INT,0);
    }
}

void drawGui(int G_OBJECT_STATE,float aspectRatio){
    //create and activate vertexArrayObject

    static GLuint vaoID=0;
    static GLuint vboIndicesID=0;
    static GLuint vboVertexID=0;
    static GLuint guiShaderID=0;
    static GLuint textureId=0;
    static int numberOfQuads=3;
    if(G_OBJECT_STATE==G_OBJECT_INIT){
        //Compile Shader
        guiShaderID = glCreateProgram();
        glAttachShader(guiShaderID, CompileShaderFromFile(".\\res\\shaders\\vertex_gui.glsl",GL_VERTEX_SHADER));         //attach vertex shader to new program
        glAttachShader(guiShaderID, CompileShaderFromFile(".\\res\\shaders\\fragment_gui.glsl",GL_FRAGMENT_SHADER));     //attach fragment shader to new program
        glLinkProgram(guiShaderID);
        //Set texture for gui shader
        glActiveTexture(GL_TEXTURE1);
        glGenTextures(1,&textureId);
        glBindTexture(GL_TEXTURE_2D,textureId);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        void* tempClientGuiTexture=read_bmp(filepath_gui_bmp);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,1024,1024,0,GL_RGBA,GL_UNSIGNED_INT_8_8_8_8,tempClientGuiTexture);
        free(tempClientGuiTexture);

        glUseProgram(guiShaderID);
        glUniform1i(glGetUniformLocation(guiShaderID,"texture1"),1);
        float GUI_positions_and_uv[8*2]=
        {
            -1.0f,0.5f,
            -0.5f,0.5f,
            -1.0f,1.0f,
            -0.5f,1.0f,
            UV_SLIDER_BAR_TOP_LEFT_X,UV_SLIDER_BAR_DOWN_RIGHT_Y,
            UV_SLIDER_BAR_DOWN_RIGHT_X,UV_SLIDER_BAR_DOWN_RIGHT_Y,
            UV_SLIDER_BAR_TOP_LEFT_X,UV_SLIDER_BAR_TOP_LEFT_Y,
            UV_SLIDER_BAR_DOWN_RIGHT_X,UV_SLIDER_BAR_TOP_LEFT_Y
        };
        unsigned int GUI_indices[6]={
            0,1,2,
            2,1,3
        };

        glGenVertexArrays(1,&vaoID);
        glBindVertexArray(vaoID);

        glGenBuffers(1,&vboVertexID);
        glBindBuffer(GL_ARRAY_BUFFER,vboVertexID);
        glBufferData(GL_ARRAY_BUFFER,sizeof(float)*16,GUI_positions_and_uv,GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,(GLvoid*)(sizeof(float)*8));
        glEnableVertexAttribArray(1);

        glGenBuffers(1,&vboIndicesID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,vboIndicesID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,6*sizeof(GLuint),GUI_indices,GL_DYNAMIC_DRAW);
        glBindVertexArray(0);

    }else if(G_OBJECT_STATE==G_OBJECT_DRAW){
        glBindVertexArray(vaoID); //This binds all buffers (vertices, indicesAndUVs)
        glUseProgram(guiShaderID);
        glActiveTexture(GL_TEXTURE1);
        glDisable(GL_DEPTH_TEST);
        //TODO transparency
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glDrawElements(GL_TRIANGLES,6*numberOfQuads,GL_UNSIGNED_INT,0); //TODO dynamic
        glDisable(GL_BLEND);
        glBindVertexArray(0);
        glEnable(GL_DEPTH_TEST);
        glActiveTexture(GL_TEXTURE0);
    }else if(G_OBJECT_STATE==G_OBJECT_UPDATE){
        printf("Info: GUI Updated\n");
        numberOfQuads=0;
        for(int gElmt=0;gElmt<numberOfGuiEelements;gElmt++){
            if(guiElementsStorage[gElmt].GUI_TYPE==GUI_TYPE_SLIDER){
                numberOfQuads+=3;
            }
            if(guiElementsStorage[gElmt].GUI_TYPE==GUI_TYPE_BUTTON){
                numberOfQuads+=1;
            }
        }
        float* GUI_positions_and_uv = (float*)malloc(16*numberOfQuads*sizeof(float)); //(*2 UV and XY Positions) (*4 vertices) (*2 each x,y)
        GLuint* GUI_indices = (GLuint*)malloc(6*numberOfQuads*sizeof(GLuint));
        int offsetInGuiPaUV=0;
        for(int gElmt=0;gElmt<numberOfGuiEelements;gElmt++){
            if(guiElementsStorage[gElmt].GUI_TYPE==GUI_TYPE_SLIDER){
                float glCoordsX=2.0f*(guiElementsStorage[gElmt].top_left_x-0.5f);       //Transform coordinates from [0,1] to [-1,1]
                float glCoordsY=-2.0f*(guiElementsStorage[gElmt].top_left_y*aspectRatio-0.5f);       //Transform coordinates from [0,1] to [-1,1]
                float glCoordsSize=2.0f*guiElementsStorage[gElmt].percentOfWidth;       //Transform coordinates from [0,1] to [-1,1]
                //Positions

                //Part 1 slider
                //Left part slider lower left vertex
                GUI_positions_and_uv[offsetInGuiPaUV++]=glCoordsX;
                GUI_positions_and_uv[offsetInGuiPaUV++]=glCoordsY-aspectRatio*glCoordsSize*(40.0f/512.0f);
                //Left part slider lower right vertex
                GUI_positions_and_uv[offsetInGuiPaUV++]=glCoordsX+((guiElementsStorage[gElmt].position*440.0f+4.0f)/512.0f)*glCoordsSize;
                GUI_positions_and_uv[offsetInGuiPaUV++]=glCoordsY-aspectRatio*glCoordsSize*(40.0f/512.0f);
                //Left part slider upper left vertex
                GUI_positions_and_uv[offsetInGuiPaUV++]=glCoordsX;
                GUI_positions_and_uv[offsetInGuiPaUV++]=glCoordsY-aspectRatio*glCoordsSize*(24.0f/512.0f);
                //Left part slider upper right vertex
                GUI_positions_and_uv[offsetInGuiPaUV++]=glCoordsX+((guiElementsStorage[gElmt].position*440.0f+4.0f)/512.0f)*glCoordsSize;
                GUI_positions_and_uv[offsetInGuiPaUV++]=glCoordsY-aspectRatio*glCoordsSize*(24.0f/512.0f);

                //Part 2 button
                //Left part slider lower left vertex
                GUI_positions_and_uv[offsetInGuiPaUV++]=glCoordsX+((guiElementsStorage[gElmt].position*440.0f+4.0f)/512.0f)*glCoordsSize;
                GUI_positions_and_uv[offsetInGuiPaUV++]=glCoordsY-aspectRatio*glCoordsSize*(64.0f/512.0f);
                //Left part slider lower right vertex
                GUI_positions_and_uv[offsetInGuiPaUV++]=glCoordsX+((guiElementsStorage[gElmt].position*440.0f+68.0f)/512.0f)*glCoordsSize;
                GUI_positions_and_uv[offsetInGuiPaUV++]=glCoordsY-aspectRatio*glCoordsSize*(64.0f/512.0f);
                //Left part slider upper left vertex
                GUI_positions_and_uv[offsetInGuiPaUV++]=glCoordsX+((guiElementsStorage[gElmt].position*440.0f+4.0f)/512.0f)*glCoordsSize;
                GUI_positions_and_uv[offsetInGuiPaUV++]=glCoordsY;
                //Left part slider upper right vertex
                GUI_positions_and_uv[offsetInGuiPaUV++]=glCoordsX+((guiElementsStorage[gElmt].position*440.0f+68.0f)/512.0f)*glCoordsSize;
                GUI_positions_and_uv[offsetInGuiPaUV++]=glCoordsY;

                //Part 3 slider
                //Left part slider lower left vertex
                GUI_positions_and_uv[offsetInGuiPaUV++]=glCoordsX+((guiElementsStorage[gElmt].position*440.0f+68.0f)/512.0f)*glCoordsSize;
                GUI_positions_and_uv[offsetInGuiPaUV++]=glCoordsY-aspectRatio*glCoordsSize*(40.0f/512.0f);
                //Left part slider lower right vertex
                GUI_positions_and_uv[offsetInGuiPaUV++]=glCoordsX+glCoordsSize;
                GUI_positions_and_uv[offsetInGuiPaUV++]=glCoordsY-aspectRatio*glCoordsSize*(40.0f/512.0f);
                //Left part slider upper left vertex
                GUI_positions_and_uv[offsetInGuiPaUV++]=glCoordsX+((guiElementsStorage[gElmt].position*440.0f+68.0f)/512.0f)*glCoordsSize;
                GUI_positions_and_uv[offsetInGuiPaUV++]=glCoordsY-aspectRatio*glCoordsSize*(24.0f/512.0f);
                //Left part slider upper right vertex
                GUI_positions_and_uv[offsetInGuiPaUV++]=glCoordsX+glCoordsSize;
                GUI_positions_and_uv[offsetInGuiPaUV++]=glCoordsY-aspectRatio*glCoordsSize*(24.0f/512.0f);

                //UV
                //Part 1 slider
                //Left part slider lower left vertex
                GUI_positions_and_uv[offsetInGuiPaUV++]=UV_SLIDER_BAR_TOP_LEFT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++]=UV_SLIDER_BAR_DOWN_RIGHT_Y;
                //Left part slider lower right vertex
                GUI_positions_and_uv[offsetInGuiPaUV++]=UV_SLIDER_BAR_TOP_LEFT_X+((guiElementsStorage[gElmt].position*440.0f+4.0f)/1024.0f);
                GUI_positions_and_uv[offsetInGuiPaUV++]=UV_SLIDER_BAR_DOWN_RIGHT_Y;
                //Left part slider upper left vertex
                GUI_positions_and_uv[offsetInGuiPaUV++]=UV_SLIDER_BAR_TOP_LEFT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++]=UV_SLIDER_BAR_TOP_LEFT_Y;
                //Left part slider upper right vertex
                GUI_positions_and_uv[offsetInGuiPaUV++]=UV_SLIDER_BAR_TOP_LEFT_X+((guiElementsStorage[gElmt].position*440.0f+4.0f)/1024.0f);
                GUI_positions_and_uv[offsetInGuiPaUV++]=UV_SLIDER_BAR_TOP_LEFT_Y;

                //Part 2 middle slider
                //Left part slider lower left vertex
                GUI_positions_and_uv[offsetInGuiPaUV++]=UV_SLIDER_BUTTON_TOP_LEFT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++]=UV_SLIDER_BUTTON_DOWN_RIGHT_Y;
                //Left part slider lower right vertex
                GUI_positions_and_uv[offsetInGuiPaUV++]=UV_SLIDER_BUTTON_DOWN_RIGHT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++]=UV_SLIDER_BUTTON_DOWN_RIGHT_Y;
                //Left part slider upper left vertex
                GUI_positions_and_uv[offsetInGuiPaUV++]=UV_SLIDER_BUTTON_TOP_LEFT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++]=UV_SLIDER_BUTTON_TOP_LEFT_Y;
                //Left part slider upper right vertex
                GUI_positions_and_uv[offsetInGuiPaUV++]=UV_SLIDER_BUTTON_DOWN_RIGHT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++]=UV_SLIDER_BUTTON_TOP_LEFT_Y;

                //Part 3 slider
                //Left part slider lower left vertex
                GUI_positions_and_uv[offsetInGuiPaUV++]=UV_SLIDER_BAR_TOP_LEFT_X+((guiElementsStorage[gElmt].position*440.0f+68.0f)/1024.0f);
                GUI_positions_and_uv[offsetInGuiPaUV++]=UV_SLIDER_BAR_DOWN_RIGHT_Y;
                //Left part slider lower right vertex
                GUI_positions_and_uv[offsetInGuiPaUV++]=UV_SLIDER_BAR_DOWN_RIGHT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++]=UV_SLIDER_BAR_DOWN_RIGHT_Y;
                //Left part slider upper left vertex
                GUI_positions_and_uv[offsetInGuiPaUV++]=UV_SLIDER_BAR_TOP_LEFT_X+((guiElementsStorage[gElmt].position*440.0f+68.0f)/1024.0f);
                GUI_positions_and_uv[offsetInGuiPaUV++]=UV_SLIDER_BAR_TOP_LEFT_Y;
                //Left part slider upper right vertex
                GUI_positions_and_uv[offsetInGuiPaUV++]=UV_SLIDER_BAR_DOWN_RIGHT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++]=UV_SLIDER_BAR_TOP_LEFT_Y;
            }
            if(guiElementsStorage[gElmt].GUI_TYPE==GUI_TYPE_BUTTON){
                //TODO
            }
        }
        {//Generate Indices for quads
            unsigned int index=0;
            int positionInIbo=0;
            while(positionInIbo<(numberOfQuads*6)){
                GUI_indices[positionInIbo++]=index++; //0
                GUI_indices[positionInIbo++]=index++; //1
                GUI_indices[positionInIbo++]=index; //2
                GUI_indices[positionInIbo++]=index--; //2
                GUI_indices[positionInIbo++]=index; //1
                index+=2;
                GUI_indices[positionInIbo++]=index++; //3
            }
        }
        glBindBuffer(GL_ARRAY_BUFFER,vboVertexID);
        glBufferData(GL_ARRAY_BUFFER,16*numberOfQuads*sizeof(float),GUI_positions_and_uv,GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,vboIndicesID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,6*numberOfQuads*sizeof(GLuint),GUI_indices,GL_DYNAMIC_DRAW);
        //update vao
        glBindVertexArray(vaoID);
        glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,(GLvoid*)(sizeof(float)*8*numberOfQuads));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);
        free(GUI_positions_and_uv);
        free(GUI_indices);
    }
}

float update_delta_time() {             //Get the current time with glfwGetTime and subtract last time to return deltatime
    static double last_glfw_time=0.0f;
    static double current_glfw_time;
    current_glfw_time = glfwGetTime();
    float delta = (float) (current_glfw_time-last_glfw_time);
    last_glfw_time = current_glfw_time;
    return delta;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if(key==GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(MainWindow,1);
    }
    if(glfwGetKey(MainWindow,GLFW_KEY_M)==GLFW_PRESS) {
        if(measurement==0)
            measurement=1;
    }
    if(measurement==2) {
        if(glfwGetKey(MainWindow,GLFW_KEY_SPACE)==GLFW_PRESS) {
            measurement=0;
        }
        if(glfwGetKey(MainWindow,GLFW_KEY_RIGHT)==GLFW_PRESS) {
            offset_x=offset_x+Offset_change;
            draw=1;
        }
        if(glfwGetKey(MainWindow,GLFW_KEY_LEFT)==GLFW_PRESS) {
            offset_x=offset_x-Offset_change;
            draw=1;
        }
        if(glfwGetKey(MainWindow,GLFW_KEY_UP)==GLFW_PRESS) {
            offset_y=offset_y+Offset_change;
            draw=1;
        }
        if(glfwGetKey(MainWindow,GLFW_KEY_DOWN)==GLFW_PRESS) {
            offset_y=offset_y-Offset_change;
            draw=1;
        }
        if(glfwGetKey(MainWindow,GLFW_KEY_R)==GLFW_PRESS) {
            offset_x = offset_x_start;
            offset_y = offset_y_start;
            Slider_size.Position=Size_start;
            diameter=Slider_size.Position*2.5f;
            draw=1;
            Slider_speed.Position = Speed_start;
            dt = (Speed_start+1)*0.0000005f;
            momentum_prop=1;
        }
        if(glfwGetKey(MainWindow,GLFW_KEY_O)==GLFW_PRESS) {
            if(Slider_size.Position>Diameter_change) {
                Slider_size.Position=Slider_size.Position-Diameter_change;
                diameter=Slider_size.Position*5.0f;
                draw=1;
            }
        }
        if(glfwGetKey(MainWindow,GLFW_KEY_P)==GLFW_PRESS) {
            if(Slider_size.Position<Slider_size.Width-Diameter_change) {
                Slider_size.Position=Slider_size.Position+Diameter_change;
                diameter=Slider_size.Position*5.0f;
                draw=1;
            }
        }
    }
    if(measurement==5){
        if(glfwGetKey(MainWindow,GLFW_KEY_N)==GLFW_PRESS) {
            measurement = 2;
            diameter=Slider_size.Position*5.0f;
            draw=1;
        }
    }
    if(glfwGetKey(MainWindow,GLFW_KEY_X)==GLFW_PRESS) {
        Slider_speed.Position=Slider_speed.Position+Diameter_change;
        dt = (Slider_speed.Position+1) * 0.0000005f;
        momentum_prop=1;
    }
    if(glfwGetKey(MainWindow,GLFW_KEY_Y)==GLFW_PRESS) {
        Slider_speed.Position=Slider_speed.Position-Diameter_change;
        dt = (Slider_speed.Position+1) * 0.0000005f;
        momentum_prop=1;
    }
}
void mouse_button_callback(GLFWwindow* window, int button,int action, int mods) {
    double xpos, ypos;
    glfwGetCursorPos(MainWindow, &xpos, &ypos);
    if(button== GLFW_MOUSE_BUTTON_LEFT&&action==GLFW_PRESS) {
        printf("LMB Down");
        //printf("%.0f, %.0f\n",xpos, ypos);
        if(measurement==0){
            if(xpos>Button_measure.Left_up_x&&xpos<Button_measure.Left_up_x+Button_measure.Width) {
                if(ypos>Button_measure.Left_up_y&&ypos<Button_measure.Left_up_y+Button_measure.Height) {
                    measurement = 1;
                    Button_measure.Position=1;
                    Button_new.Position=0;
                }
            }
        }
        if(xpos>Button_esc.Left_up_x&&xpos<Button_esc.Left_up_x+Button_esc.Width) {
            if(ypos>Button_esc.Left_up_y&&ypos<Button_esc.Left_up_y+Button_esc.Height) {
                glfwSetWindowShouldClose(MainWindow,1);
            }
        }

        if(measurement==2){
            if(xpos>Button_new.Left_up_x&&xpos<Button_new.Left_up_x+Button_new.Width) {
                if(ypos>Button_new.Left_up_y&&ypos<Button_new.Left_up_y+Button_new.Height) {
                    diameter=Slider_size.Position*2.5f;
                    draw=1;
                    measurement=0;
                    Button_new.Position=1;
                    Button_measure.Position=0;
                }
            }
            if(xpos>Slider_size.Left_up_x&&xpos<Slider_size.Left_up_x+Slider_size.Width) {
                if(ypos>Slider_size.Left_up_y&&ypos<Slider_size.Left_up_y+Slider_size.Height) {
                    Slider_size.Position=xpos-Slider_size.Left_up_x;
                    diameter=Slider_size.Position*5.0f;
                    draw=1;
                    Button_measure.Position=0;
                    Button_new.Position=0;
                }
            }
            if(xpos>Slider_speed.Left_up_x&&xpos<Slider_speed.Left_up_x+Slider_speed.Width) {
                if(ypos>Slider_speed.Left_up_y&&ypos<Slider_speed.Left_up_y+Slider_speed.Height) {
                    Slider_speed.Position=xpos-Slider_speed.Left_up_x;
                    diameter=Slider_speed.Position*5.0f;
                    dt = (Slider_speed.Position+1) * 0.0000005f;
                    momentum_prop=1;
                }
            }
        }
        else {
            Button_measure.Position=0;
            Button_new.Position=0;
        }
    }
    if(button== GLFW_MOUSE_BUTTON_LEFT&&action==GLFW_RELEASE) {
        if(xpos>Button_new.Left_up_x&&xpos<Button_new.Left_up_x+Button_new.Width) {
            if(ypos>Button_new.Left_up_y&&ypos<Button_new.Left_up_y+Button_new.Height) {
                Button_new.Position=0;
            }
        }
        if(xpos>Button_measure.Left_up_x&&xpos<Button_measure.Left_up_x+Button_measure.Width) {
            if(ypos>Button_measure.Left_up_y&&ypos<Button_measure.Left_up_y+Button_measure.Height) {
                Button_measure.Position=0;
            }
        }
    }
}

GLuint CompileShaderFromFile(char FilePath[],GLuint shaderType) {
    //read from file into heap memory
    FILE* filepointer=fopen(FilePath,"rb");                  //open specified file in read only mode
    if(filepointer==NULL) {
        printf("Error: Filepointer to shaderfile at %s of type %d could not be loaded.",FilePath,shaderType);
        //return;
    }
    fseek(filepointer,0,SEEK_END);                          //shift filePointer to EndOfFile Position to get filelength
    long filelength = ftell(filepointer);                   //get filePointer position
    fseek(filepointer,0,SEEK_SET);                          //move file Pointer back to first line of file
    char* filestring = (char*)malloc(filelength+1);         //
    if(fread(filestring,sizeof(char),filelength,filepointer) != filelength) {
        printf("Error: Missing characters in input string");
        //return;
    }
    if(filestring[0]==0xEF&&filestring[1]==0xBB&&filestring[2]==0xBF) {  //Detect if file is utf8 with bom
        printf("Error: Remove the bom from your utf8 shader file");
    }
    filestring[filelength]=0;                             //Set end of string
    fclose(filepointer);                                  //Close File
    const char* ConstFilePointer = filestring;            //opengl wants const pointer
    //compile shader with opengl
    GLuint ShaderId= glCreateShader(shaderType);
    glShaderSource(ShaderId,1,&ConstFilePointer,NULL);
    glCompileShader(ShaderId);
    GLint compStatus = 0;
    glGetShaderiv(ShaderId,GL_COMPILE_STATUS,&compStatus);
    if(compStatus!=GL_TRUE) {
        printf("Error: Compilation of shader %d failed!\n",ShaderId);
        //TODO free resources
        //return;
    }
    printf("Info: Shader %d sucessfully compiled.\n",ShaderId);
    free(filestring);                                   //Delete Shader string from heap
    fclose(filepointer);
    return ShaderId;
}

void glfw_error_callback(int error, const char* description) {
    printf("Error: GLFW-Error type: %d occurred.\nDescription: %s\n",error,description);
}

void APIENTRY openglCallbackFunction(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar* message,const void* userParam) {
    printf("Error in opengl occured!\n");
    printf("Message: %s\n",message);
    printf("type or error: ");
    switch (type) {
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
    printf("\nId:%d \n",id);
    printf("Severity:");
    switch (severity) {
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

void drop_file_callback(GLFWwindow* window, int count, const char** paths) {
    for(int i=0; i<count; i++) {
        printf("Dropped File Path: %s\n",paths[i]);
    }
}

void mouse_scroll_callback(GLFWwindow* window, double xOffset, double yOffset) {
    float temp_mouse_scroll= -0.04f*(float)yOffset;
    if((FOV+temp_mouse_scroll<1.0f)&&(FOV+temp_mouse_scroll>0.1f)) {
        FOV+=temp_mouse_scroll;
    } else {
    }
}

unsigned int read_uint_from_endian_file(FILE* file) {
    unsigned char data[4];
    unsigned int data_return_int;
    if(fread(data,1,4,file)<4) { //total number of read elements is less than 4
        return 0;
    }
    //little endian
    data_return_int=(data[3]<<24)|(data[2]<<16)|(data[1]<<8)|data[0];
    //big endian (comment out and comment little endian if needed)
    //data_return_int=(data[0]<<24)|(data[1]<<16)|(data[2]<<8)|data[3];
    return data_return_int;
}

unsigned short read_short_from_endian_file(FILE* file) {
    //http://cpansearch.perl.org/src/DHUNT/PDL-Planet-0.05/libimage/bmp.c
    unsigned char data[2];
    unsigned short data_return_short;
    if(fread(data,1,2,file)<2) { //total number of read elements is less than 4
        return 0;
    }
    //little endian
    data_return_short=(data[1]<<8)|data[0];
    //big endian (comment out and comment little endian if needed)
    //data_return_short=(data[0]<<8)|data[1];
    return data_return_short;
}

unsigned char* read_bmp(char* filepath) {
    //source https://github.com/ndd314/sjsu_cmpe295_cuda_fft_opengl/blob/master/opengl/plane/readBMPV2.c#L50
    //https://stackoverflow.com/questions/7990273/converting-256-color-bitmap-to-rgba-bitmap
    FILE *filepointer=fopen(filepath,"rb");

    if(filepointer==NULL) {
        printf("Error: File :%s could not be found\n",filepath);
        fclose(filepointer);
        return 0;
    }

    fseek(filepointer,0,SEEK_SET);  //Jump to beginning of file
    if(read_short_from_endian_file(filepointer)!=0x4D42) { // (equals BM in ASCII)
        fclose(filepointer);
        printf("Error: File :%s is not an BMP\n",filepath);
        return 0;
    }

    fseek(filepointer,10,SEEK_SET);
    unsigned int BitmapOffset = read_uint_from_endian_file(filepointer);
    printf("Info: BMP data offset:%d\n",BitmapOffset);

    if(read_uint_from_endian_file(filepointer)!=124) {
        printf("Error: BitmapHeader is not BITMAPV5HEADER / 124 \n");
        fclose(filepointer);
        return 0;
    }
    unsigned int BitmapWidth=read_uint_from_endian_file(filepointer);
    printf("Info: BitmapWidth is %d.\n",BitmapWidth);

    unsigned int BitmapHeight=read_uint_from_endian_file(filepointer);
    printf("Info: BitmapHeight is %d.\n",BitmapHeight);

    if(read_short_from_endian_file(filepointer)!=1) {
        printf("Error: Unsupported plane count\n");
        return 0;
    }

    unsigned int BitmapColorDepth=read_short_from_endian_file(filepointer);
    printf("Info: BMP color depth:%d",BitmapColorDepth);
    unsigned int BitmapSizeCalculated=(BitmapColorDepth/8)*(BitmapWidth+(BitmapWidth%4))*BitmapWidth;

    unsigned int BitmapCompression=read_uint_from_endian_file(filepointer);
    switch(BitmapCompression) {
    case 0:
        printf("Warn: Compression type: none/BI_RGB\n");
        break;
    case 3:
        printf("Info: Compression type: Bitfields/BI_BITFIELDS\n");
        break;
    default:
        printf("Error: Unsupported compression %d\n",BitmapCompression);
        fclose(filepointer);
        return 0;
        break;
    }
    unsigned int BitmapImageSize=read_uint_from_endian_file(filepointer);
    if(BitmapImageSize!=BitmapSizeCalculated) {
        printf("Error: Reading image size: Calculated Image Size: %d.\nRead Image size: %d\n",BitmapSizeCalculated,BitmapImageSize);
        fclose(filepointer);
        return 0;
    }
    printf("Info: Image Size:%d\n",BitmapSizeCalculated);
    /*unsigned int BitmapXPpM=*/read_uint_from_endian_file(filepointer);
    /*unsigned int BitmapYPpM=*/read_uint_from_endian_file(filepointer);
    unsigned int BitmapColorsInPalette=read_uint_from_endian_file(filepointer);
    printf("Info: Colors in palette: %d.\n",BitmapColorsInPalette);
    fseek(filepointer,4,SEEK_CUR);//skip over important color count
    if(BitmapCompression==3) {
        unsigned char RGBA_mask[4];
        for(unsigned int color_channel=0; color_channel<4; color_channel++) {
            unsigned int color_channel_mask=read_uint_from_endian_file(filepointer);
            switch(color_channel_mask) { //read shift value for color_channel
            case 0xFF000000:
                RGBA_mask[color_channel]=3;
                break;
            case 0x00FF0000:
                RGBA_mask[color_channel]=2;
                break;
            case 0x0000FF00:
                RGBA_mask[color_channel]=1;
                break;
            case 0x000000FF:
                RGBA_mask[color_channel]=0;
                break;
            default:
                printf("Error: Invalid BITMASK. Value: %x!\n",color_channel_mask);
                fclose(filepointer);
                return 0;
                break;
            }
        }
        //TODO implement swapping routine if brga!=[3,2,1,0]
        printf("Info: Shifting value for R:%d G:%d B:%d A:%d\n",RGBA_mask[0],RGBA_mask[1],RGBA_mask[2],RGBA_mask[3]);
        unsigned char* imageData=malloc(BitmapSizeCalculated);
        printf("Info: BMOFFST: %d\n",BitmapOffset);
        fseek(filepointer,BitmapOffset,SEEK_SET);//jump to pixel data
        printf("Info: Calsize: %d\n",BitmapSizeCalculated);
        if(fread(imageData,BitmapSizeCalculated,1,filepointer)==0) {
            printf("Error: Reading failed!");
        }
        fclose(filepointer);
        return imageData;
    }
    printf("Error: Currently not implemented!");
    fclose(filepointer);
    return 0;
}

void windows_size_callback(GLFWwindow* window, int width, int height){
    glViewport(0,0,width,height);
    drawGui(G_OBJECT_UPDATE,width/(float)height);
}

void write_bmp(char* filepath, unsigned int width, unsigned int height) {
    FILE* filepointer = fopen(filepath,"wb");
    //bytes_per_line=(3*(width+1)/4)*4;
    const char* String_to_write="BMP";
    fwrite(&String_to_write,sizeof(char),3,filepointer);
    return;
}
