#define STRSAFE_NO_DEPRECATE //allow use of strcpy and strcat

#define GLEW_STATIC
#include "libraries/GLEW_2.1.0/include/glew.h"
#include "libraries/GLFW_3.2.1/include/glfw3.h"
#include "libraries/LINMATH/include/linmath.h"

//Functions to correct for perspective
#include "map_camera_plane.h"
//Functions to get raw data from camera
#include "camera_dshow.h"
//Functions for analyzing the data in the raw frams
#include "image_analyse.h"
//Functions for simulating the wave function
#include "simulation.h"
//Functions for file handling
#include "filereader.h"


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <dirent.h>

#include <math.h>
#ifndef M_PI
  #define M_PI 3.14159265358979323846
#endif
#include <limits.h>


#ifdef _WIN32
 #define filepath_potentials ".\\res\\potentials"
 #define filepath_gui_bmp ".\\res\\textures\\GUI2.bmp"  //CHANGED!!!
 #define filepath_potential_bmp ".\\res\\potentials\\double_slit512.bmp"
 #define filepath_shader_vertex_gui ".\\res\\shaders\\vertex_gui.glsl"
 #define filepath_shader_fragment_gui ".\\res\\shaders\\fragment_gui.glsl"
 #define filepath_shader_vertex_graph ".\\res\\shaders\\vertex_graph.glsl"
 #define filepath_shader_fragment_graph ".\\res\\shaders\\fragment_graph.glsl"
 #define filepath_shader_vertex_target ".\\res\\shaders\\vertex_target.glsl"
 #define filepath_shader_fragment_target ".\\res\\shaders\\fragment_target.glsl"
#elif __linux__
 #define filepath_potentials "./res/potentials"
 #define filepath_gui_bmp "./res/textures/GUI2.bmp"
 //#define filepath_potential_bmp "./res/potentials/double_slit512.bmp"
 #define filepath_shader_vertex_gui "./res/shaders/vertex_gui.glsl"
 #define filepath_shader_fragment_gui "./res/shaders/fragment_gui.glsl"
 #define filepath_shader_vertex_graph "./res/shaders/vertex_graph.glsl"
 #define filepath_shader_fragment_graph "./res/shaders/fragment_graph.glsl"
 #define filepath_shader_vertex_target "./res/shaders/vertex_target.glsl"
 #define filepath_shader_fragment_target "./res/shaders/fragment_target.glsl"
#endif
enum {C_OBJECT_INIT,C_OBJECT_UPDATE};
enum {G_OBJECT_INIT,G_OBJECT_DRAW,G_OBJECT_UPDATE};

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void drop_file_callback(GLFWwindow* window, int count, const char** paths);
void mouse_scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void windows_size_callback(GLFWwindow* window, int width, int height);
void glfw_error_callback(int error, const char* description);
void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);
unsigned char* read_bmp(char* filepath);
void write_bmp(char* filepath, unsigned int width, unsigned int height);
float update_delta_time();
float timerForBlink(int restart);
void APIENTRY openglCallbackFunction(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
void drawGui(int G_OBJECT_STATE, float aspectRatio);
void drawTrackPoint(int G_OBJECT_STATE,mat4x4 mvp,float xpos, float ypos);
void drawPlaneAndGrid(int G_OBJECT_STATE, unsigned int PlaneResolution, unsigned int GridResolution, mat4x4 mvp4x4);
GLuint CompileShaderFromFile(char FilePath[], GLuint shaderType);
void JoystickControll();
void joystick_reset(int selectedGuiElement);
void set_x_position_slider(int selectedGuiElement, double xpos);
void set_y_position_slider(int selectedGuiElement, double ypos);
void rotate_camera(int selectedGuiElement);
//void move_camera(int selectedGuiElement);
//void move_wave(int selectedGuiElement);
void change_speed();
//void draw_wave();
void set_xy_position_joystick(int selectedGuiElement, double xpos, double ypos);
void drawTargetBox(int G_OBJECT_STATE,mat4x4 mvp4x4,float Intensity);
void update_potential();
void refereshGUI();
void camera_collider(int C_OBJECT_STATE ,vec2 posNewIn);
void standard_draw();

enum {type_wave_gauss_width,type_wave_momentum,type_wave_angle,type_wave_dt};
float mapValue(int type,float input);
/*UV COORDINATES FOR GUI
 Y
 ^
/|\
 |
 |
 +----------> X
*/
#define GUI_TYPE_UNUSED 0
//Slider x
#define GUI_TYPE_SLIDER_SIZE 1

#define UV_SLIDER_BAR_ACTIVE_TOP_LEFT_X   (   0.0f/1024.0f)
#define UV_SLIDER_BAR_ACTIVE_TOP_LEFT_Y   (1024.0f/1024.0f)
#define UV_SLIDER_BAR_ACTIVE_DOWN_RIGHT_X ( 512.0f/1024.0f)
#define UV_SLIDER_BAR_ACTIVE_DOWN_RIGHT_Y (1008.0f/1024.0f)

#define UV_SLIDER_BAR_INACTIVE_TOP_LEFT_X   (   0.0f/1024.0f)
#define UV_SLIDER_BAR_INACTIVE_TOP_LEFT_Y   (1004.0f/1024.0f)
#define UV_SLIDER_BAR_INACTIVE_DOWN_RIGHT_X ( 512.0f/1024.0f)
#define UV_SLIDER_BAR_INACTIVE_DOWN_RIGHT_Y ( 988.0f/1024.0f)

#define UV_SLIDER_SIZE_BUTTON_TOP_LEFT_X   ( 260.0f/1024.0f)
#define UV_SLIDER_SIZE_BUTTON_TOP_LEFT_Y   ( 984.0f/1024.0f)
#define UV_SLIDER_SIZE_BUTTON_DOWN_RIGHT_X ( 516.0f/1024.0f)
#define UV_SLIDER_SIZE_BUTTON_DOWN_RIGHT_Y ( 728.0f/1024.0f)

#define GUI_TYPE_SLIDER_TIME 2
#define UV_SLIDER_TIME_BUTTON_TOP_LEFT_X   ( 0.0f/1024.0f)
#define UV_SLIDER_TIME_BUTTON_TOP_LEFT_Y   ( 984.0f/1024.0f)
#define UV_SLIDER_TIME_BUTTON_DOWN_RIGHT_X ( 256.0f/1024.0f)
#define UV_SLIDER_TIME_BUTTON_DOWN_RIGHT_Y ( 728.0f/1024.0f)

#define GUI_TYPE_SLIDER_ANGLE 3
#define UV_SLIDER_ANGLE_BUTTON_TOP_LEFT_X   ( 520.0f/1024.0f)
#define UV_SLIDER_ANGLE_BUTTON_TOP_LEFT_Y   ( 984.0f/1024.0f)
#define UV_SLIDER_ANGLE_BUTTON_DOWN_RIGHT_X ( 776.0f/1024.0f)
#define UV_SLIDER_ANGLE_BUTTON_DOWN_RIGHT_Y ( 728.0f/1024.0f)

//Slider y

//Joystick
#define GUI_TYPE_JOYSTICK_MOVEMENT 4
#define UV_GUI_JOYSTICK_MOVEMENT_TOP_LEFT_X   (   0.0f/1024.0f)
#define UV_GUI_JOYSTICK_MOVEMENT_TOP_LEFT_Y   ( 328.0f/1024.0f)
#define UV_GUI_JOYSTICK_MOVEMENT_DOWN_RIGHT_X ( 256.0f/1024.0f)
#define UV_GUI_JOYSTICK_MOVEMENT_DOWN_RIGHT_Y (  72.0f/1024.0f)

#define GUI_TYPE_JOYSTICK_ROTATION 5
#define UV_GUI_JOYSTICK_ROTATION_TOP_LEFT_X   ( 260.0f/1024.0f)
#define UV_GUI_JOYSTICK_ROTATION_TOP_LEFT_Y   ( 328.0f/1024.0f)
#define UV_GUI_JOYSTICK_ROTATION_DOWN_RIGHT_X ( 516.0f/1024.0f)
#define UV_GUI_JOYSTICK_ROTATION_DOWN_RIGHT_Y (  72.0f/1024.0f)

#define GUI_TYPE_JOYSTICK_WAVE_MOVE 6
#define UV_GUI_JOYSTICK_WAVE_MOVE_TOP_LEFT_X   ( 520.0f/1024.0f)
#define UV_GUI_JOYSTICK_WAVE_MOVE_TOP_LEFT_Y   ( 328.0f/1024.0f)
#define UV_GUI_JOYSTICK_WAVE_MOVE_DOWN_RIGHT_X ( 776.0f/1024.0f)
#define UV_GUI_JOYSTICK_WAVE_MOVE_DOWN_RIGHT_Y (  72.0f/1024.0f)

//Buttons
#define GUI_TYPE_BUTTON_CONTROL 7
#define UV_GUI_BUTTON_NEW_TOP_LEFT_X    (   0.0f/1024.0f)
#define UV_GUI_BUTTON_NEW_TOP_LEFT_Y    ( 724.0f/1024.0f)
#define UV_GUI_BUTTON_NEW_DOWN_RIGHT_X  ( 512.0f/1024.0f)
#define UV_GUI_BUTTON_NEW_DOWN_RIGHT_Y  ( 596.0f/1024.0f)

#define UV_GUI_BUTTON_START_TOP_LEFT_X    (   0.0f/1024.0f)
#define UV_GUI_BUTTON_START_TOP_LEFT_Y    ( 592.0f/1024.0f)
#define UV_GUI_BUTTON_START_DOWN_RIGHT_X  ( 512.0f/1024.0f)
#define UV_GUI_BUTTON_START_DOWN_RIGHT_Y  ( 464.0f/1024.0f)

#define UV_GUI_BUTTON_MESS_TOP_LEFT_X    (   0.0f/1024.0f)
#define UV_GUI_BUTTON_MESS_TOP_LEFT_Y    ( 460.0f/1024.0f)
#define UV_GUI_BUTTON_MESS_DOWN_RIGHT_X  ( 512.0f/1024.0f)
#define UV_GUI_BUTTON_MESS_DOWN_RIGHT_Y  ( 332.0f/1024.0f)

#define GUI_TYPE_BUTTON_POTENTIAL 8
#define UV_GUI_BUTTON_POTENTIAL_TOP_LEFT_X    ( 512.0f/1024.0f)
#define UV_GUI_BUTTON_POTENTIAL_TOP_LEFT_Y    ( 724.0f/1024.0f)
#define UV_GUI_BUTTON_POTENTIAL_DOWN_RIGHT_X  (1024.0f/1024.0f)
#define UV_GUI_BUTTON_POTENTIAL_DOWN_RIGHT_Y  ( 596.0f/1024.0f)

#define UV_GUI_JOYSTICK_BORDER_TOP_LEFT_X     ( 780.0f/1024.0f)
#define UV_GUI_JOYSTICK_BORDER_TOP_LEFT_Y     ( 984.0f/1024.0f)
#define UV_GUI_JOYSTICK_BORDER_DOWN_RIGHT_X   (1024.0f/1024.0f)
#define UV_GUI_JOYSTICK_BORDER_DOWN_RIGHT_Y   ( 740.0f/1024.0f)



//Instantiated GUI_elements
//Slider x
#define SLIDER_X_NUMBER 3

#define GUI_SLIDER_SIZE 0
#define SLIDER_SIZE_START 0.5f
#define GUI_SLIDER_SPEED 1
#define SLIDER_SPEED_START 0.5f
#define GUI_SLIDER_WAVE_ROTATION 2
//Slider y
#define SLIDER_Y_NUMBER 0

//Joysticks
#define JOYSTICK_NUMBER 3

#define GUI_JOYSTICK_PROPERTY_SCALE 0.3f //how big the joystick is in relation to movement Area
#define GUI_JOYSTICK_MOVEMENT 3
#define GUI_JOYSTICK_ROTATION 4
#define GUI_JOYSTICK_WAVE_MOVE 5
//Buttons
#define BUTTON_NUMBER 2

#define GUI_BUTTON_CONTROL 6
#define GUI_BUTTON_POTENTIAL 7

#define GUI_STATE_BUTTON1_RESET 0.0f //the position_x value is used to store what state the button is in
#define GUI_STATE_BUTTON1_START 0.5f
#define GUI_STATE_BUTTON1_MESS 1.0f






struct GUI_render {
    float top_left_x;       //between [0,(9/16 or 3/4)]
    float top_left_y;       //between [0,1]
    float percentOfWidth;   //between [0,1.0f]
    int GUI_TYPE;
    float position_x;
    float position_y;

};

//Manipulation of UI and System
float FOV = 0.7f;
GLFWwindow* MainWindow;
GLuint psiTexture;
float delta_time;
//@numgui
int numberOfGuiElements = SLIDER_X_NUMBER+SLIDER_Y_NUMBER+JOYSTICK_NUMBER+BUTTON_NUMBER;
int selectedGuiElement = (-1);   //-1 == no element selected
struct GUI_render* guiElementsStorage;
double rotation_up_down = M_PI/3;
double rotation_left_right = M_PI;
double position_x_axis = 0.0;
double position_y_axis = 0.0;
#define MovementBorderCamera 0.5f
#define PlaneResx 2048
#define PlaneResy 2048
#define PlaneRes 2048  //should be power of 2 ,vertex resolution of the rendered plane
#define GridResx 256
#define GridResy 256
#define GridRes 256        //must be power of 2 ,vertex resolution of the rendered grid




#define Diameter_change 0.02f
#define SIZE_MULTI 20.0f
double diameter = SLIDER_SIZE_START * SIZE_MULTI + 1.0f;
#define norm Resolutionx*Resolutiony
int pos = 0;
int draw_new_wave = 1; //render next frame?
#define MovementBorderWave 0.4f
#define Speed_change 0.05f
#define SPEED_MULTI 0.0002f

#define Offset_change 10

//@@ begin Simulation
//Wave Parameters
float dt = SLIDER_SPEED_START * SPEED_MULTI;
#define wave_offset_x_start (512*0.5f)
float wave_offset_x = wave_offset_x_start; //offset for particle
#define wave_offset_y_start (512*0.10f)
float wave_offset_y = wave_offset_y_start; //offset for particle
float wave_angle= M_PI / 2.0f;
float wave_momentum=1.0f; //TODO initialize properly
float wave_gauss_width=10.0f;
//@@ end Simulation



float ColorIntensity=2.9f;
int BlinkStep=0;

char PotentialFilesList[20][256]; //Currently limited to 20 .bmp files
uint8_t CountOfPotentialFiles=0;

//Potential Pointer

uint8_t* pot;

int disable_autocenter = 1;
float jerk_for_autocenter = 0.15f;

int main(int argc, char* argv[]) {
    //@Simulation
    simulation_alloc();
    //@Simulation

    //Index and initialize Potential Files
    {
        DIR* directory;
        directory=opendir(filepath_potentials);
        struct dirent* subdirectory;
        if(directory){
            while((subdirectory=readdir(directory))!=0){
                char* fileEnding=strrchr(subdirectory->d_name,'.');
                if(fileEnding && !strcmp(fileEnding, ".bmp")){
                    strcpy(PotentialFilesList[CountOfPotentialFiles++],(subdirectory->d_name));
                    //StringCbCopy(PotentialFilesList[CountOfPotentialFiles++],256,(subdirectory->d_name));
                    printf("Info: Found Potential File '%s'\n",subdirectory->d_name);
                }
            }
        }else{
            printf("Error: Missing Potential directory (./res/potentials)\n");
            return 1;
        }
        printf("\nInfo: Will use '%s' as Potential\n",&(PotentialFilesList[0][0]));
    }

    //GUI INIT
    guiElementsStorage = malloc(numberOfGuiElements * sizeof(struct GUI_render));
    //Screen coordinates from x[-1.0f,1.0f] y[-1.0f,1.0f]
    //0 for SIZE, 1 for SPEED
    guiElementsStorage[GUI_SLIDER_SIZE].top_left_x = 0.0f;
    guiElementsStorage[GUI_SLIDER_SIZE].top_left_y = 0.0f;
    guiElementsStorage[GUI_SLIDER_SIZE].position_x = 0.5f;
    guiElementsStorage[GUI_SLIDER_SIZE].percentOfWidth = 0.3f;
    guiElementsStorage[GUI_SLIDER_SIZE].GUI_TYPE = GUI_TYPE_SLIDER_SIZE;
    //Slider for speed
    guiElementsStorage[GUI_SLIDER_SPEED].top_left_x = 0.0f;
    guiElementsStorage[GUI_SLIDER_SPEED].top_left_y = 0.05f;
    guiElementsStorage[GUI_SLIDER_SPEED].position_x = 0.5f;
    guiElementsStorage[GUI_SLIDER_SPEED].percentOfWidth = 0.3f;
    guiElementsStorage[GUI_SLIDER_SPEED].GUI_TYPE = GUI_TYPE_SLIDER_TIME;
    //Buttons
    guiElementsStorage[GUI_BUTTON_CONTROL].top_left_x = 0.8f;
    guiElementsStorage[GUI_BUTTON_CONTROL].position_x = GUI_STATE_BUTTON1_START;
    guiElementsStorage[GUI_BUTTON_CONTROL].percentOfWidth = 0.2f;
    guiElementsStorage[GUI_BUTTON_CONTROL].GUI_TYPE = GUI_TYPE_BUTTON_CONTROL;
    //
    guiElementsStorage[GUI_JOYSTICK_MOVEMENT].top_left_x = 0.0f;
    guiElementsStorage[GUI_JOYSTICK_MOVEMENT].position_x = 0.0f;
    guiElementsStorage[GUI_JOYSTICK_MOVEMENT].position_y = 0.0f;
    guiElementsStorage[GUI_JOYSTICK_MOVEMENT].percentOfWidth = 0.1f;
    guiElementsStorage[GUI_JOYSTICK_MOVEMENT].GUI_TYPE = GUI_TYPE_JOYSTICK_MOVEMENT;
    //
    guiElementsStorage[GUI_JOYSTICK_ROTATION].top_left_x = 0.15f;
    guiElementsStorage[GUI_JOYSTICK_ROTATION].position_x = 0.0f;
    guiElementsStorage[GUI_JOYSTICK_ROTATION].position_y = 0.0f;
    guiElementsStorage[GUI_JOYSTICK_ROTATION].percentOfWidth = 0.1f;
    guiElementsStorage[GUI_JOYSTICK_ROTATION].GUI_TYPE = GUI_TYPE_JOYSTICK_ROTATION;
    //
    guiElementsStorage[GUI_SLIDER_WAVE_ROTATION].top_left_x = 0.0f;
    guiElementsStorage[GUI_SLIDER_WAVE_ROTATION].top_left_y = 0.10f;
    guiElementsStorage[GUI_SLIDER_WAVE_ROTATION].position_x = 0.5f;
    guiElementsStorage[GUI_SLIDER_WAVE_ROTATION].percentOfWidth = 0.3f;
    guiElementsStorage[GUI_SLIDER_WAVE_ROTATION].GUI_TYPE = GUI_TYPE_SLIDER_ANGLE;
    //
    guiElementsStorage[GUI_JOYSTICK_WAVE_MOVE].top_left_x = 0.3f;
    guiElementsStorage[GUI_JOYSTICK_WAVE_MOVE].position_x = 0.0f;
    guiElementsStorage[GUI_JOYSTICK_WAVE_MOVE].position_y = 0.0f;
    guiElementsStorage[GUI_JOYSTICK_WAVE_MOVE].percentOfWidth = 0.1f;
    guiElementsStorage[GUI_JOYSTICK_WAVE_MOVE].GUI_TYPE = GUI_TYPE_JOYSTICK_WAVE_MOVE;
    //
    guiElementsStorage[GUI_BUTTON_POTENTIAL].top_left_x = 0.8f;
    guiElementsStorage[GUI_BUTTON_POTENTIAL].position_x = 0.0f;
    guiElementsStorage[GUI_BUTTON_POTENTIAL].percentOfWidth = 0.2f;
    guiElementsStorage[GUI_BUTTON_POTENTIAL].GUI_TYPE = GUI_TYPE_BUTTON_POTENTIAL;

    //If you add a gui element remember to increase "number of gui elements @numgui

    //GLFW init
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


    //Register Callbacks for user input
    glfwSetKeyCallback(MainWindow, key_callback);
    glfwSetMouseButtonCallback(MainWindow, mouse_button_callback);
    glfwSetDropCallback(MainWindow, drop_file_callback);
    glfwSetScrollCallback(MainWindow, mouse_scroll_callback);
    glfwSetWindowSizeCallback(MainWindow, windows_size_callback);
    glfwSetCursorPosCallback(MainWindow, cursor_pos_callback);
    //Get window height
    {
        int maxIndices, maxVertices, maxTexSize, maxTexBufferSize;
        glGetIntegerv(GL_MAX_ELEMENTS_INDICES, &maxIndices);
        glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &maxVertices);
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
        glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, &maxTexBufferSize);
        printf("Info: maxIndicesPerBuffer:%d\nInfo: maxVertexPerBuffer:%d\nInfo: maxTextureSize:%d\nInfo: maxBufferSize:%d\n", maxIndices, maxVertices, maxTexSize, maxTexBufferSize);
    }
    //Set background color
    glClearColor(0.6f, 0.6f, 0.6f, 0.0f);
    //Enable z checking
    glEnable(GL_DEPTH_TEST);
    //Enable Antialiasing
    /*
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
    */
    /*//https://www.seas.upenn.edu/%7Epcozzi/OpenGLInsights/OpenGLInsights-AsynchronousBufferTransfers.pdf
    //Generate data memory for psi
    unsigned char* psi=malloc(Resolutionx*Resolutiony*4);
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
    glBufferData(GL_PIXEL_UNPACK_BUFFER,4*Resolutionx*Resolutiony,psi,GL_STREAM_DRAW);

    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,Resolutionx,Resolutiony,GL_RGBA,GL_UNSIGNED_INT_8_8_8_8_REV,NULL); //NULL pointer let opengl fetch data from bound GL_PIXEL_UNPACK_BUFFER
    */
    //glTexSubImage2D(GL_TEXTURE_2D,,0,0,0,Resolutionx,Resolutiony,GL_UNSIGNED_INT_8_8_8_8_REV);//4 upadte every frame

    mat4x4 mvp4x4;
    mat4x4 persp4x4; //remove?? TODO
    vec3 eye_vec = {1.0f, 1.0f, 1.0f};
    vec3 cent_vec = {0.0f, 0.0f, 0.0f};
    vec3 up_vec = {0.0f, 0.0f, 1.0f};
    //GUI
    //Init gui
    drawGui(G_OBJECT_INIT, 0);   //Initialize Gui with GL_OBJECT_INIT,aspect ratio
    refereshGUI();
    printf("Info: Generation of gui successfull!\n");
    //Potential loading
    unsigned char* speicher = calloc(sim_res_total * 4, 1);

    update_potential(speicher);
    //Create wave
    delta_time = update_delta_time();
    //@@Graphics
    //init texture for Psi because its dynamic
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &psiTexture);
    glBindTexture(GL_TEXTURE_2D, psiTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //Init plane and grid
    drawPlaneAndGrid(G_OBJECT_INIT, PlaneRes, GridRes, NULL);   //mvp4x4 useless here
    printf("Info: Generation of plane and grid successfull!\n");
    //Init target box
    //drawTargetBox(G_OBJECT_INIT,0,0.0f);
    drawTrackPoint(G_OBJECT_INIT,0,0.0f,0.0f);
    //end Graphics@@

    //begin Camera@@
    volatile int CamXpos=-1;
    volatile int CamYpos=-1;
    IMediaControl* MediaControl=getPositionPointer(&CamXpos,&CamYpos);
    MediaControl->lpVtbl->Run(MediaControl);
    float CalibPoints[8]={0.f,200.f, 1919.f,200.f, 1919.f,1079.f, 0.f,1079.f};
    mat3x3 CalibData;
    vec2 BrighspotMapped;
    camera_perspec_calibrating(CalibData,CalibPoints);
    while((CamXpos==-1)||(CamYpos==-1)){
            //MediaControl->lpVtbl->Run(MediaControl);
            //printf("Wait\n");
    }
    vec2 CurrentPos={(float) CamXpos, (float)CamYpos};
    camera_perspec_map_point(BrighspotMapped,CalibData,CurrentPos);
    camera_collider(C_OBJECT_INIT ,BrighspotMapped);
    CamXpos=-1;
    CamYpos=-1;
    MediaControl->lpVtbl->Run(MediaControl);
    //end Camera@@
    //printf("testtestsetset\n");

    //Initialize Gaus wave packet for simulation in position space
    standard_draw();
    while(!glfwWindowShouldClose(MainWindow)) { //Main Programm loop
        simulation_run(dt);
        delta_time = update_delta_time();
        //Check for Camera frame update
        if((CamXpos!=-1)&&(CamYpos!=-1)){ //Got Frame update
            //printf("Debug: Cam RawXY: %d, %d\n\n",CamXpos,CamYpos);
            vec2 CurrentPos={(float) CamXpos, (float)CamYpos};
            camera_perspec_map_point(BrighspotMapped,CalibData,CurrentPos);
            //printf("Debug: x,%f y%f\n",BrighspotMapped[0],BrighspotMapped[1]);
            drawTrackPoint(G_OBJECT_UPDATE,0,(BrighspotMapped[0]-0.5f),(BrighspotMapped[1]-0.5f));
            CamYpos=CamXpos=-1;
            camera_collider(C_OBJECT_UPDATE ,BrighspotMapped);
            MediaControl->lpVtbl->Run(MediaControl);

        }else{
            //printf("Debug: ++++++++++++++++++++++++++++++++++++++Frameskip of camera\n");
        }
        //Camera


        /*if(timerForBlink(0)>5.0f){
            //Same as Reset Button
            if(simulation_state==simulation_state_wait_for_restart||simulation_state==simulation_state_measurement_animation||simulation_state==simulation_state_create_and_wait_for_start){
                draw_new_wave = 1;
                guiElementsStorage[GUI_BUTTON_CONTROL].position_x = GUI_STATE_BUTTON1_START;
                refereshGUI();
                simulation_state = 2;
                rotation_left_right = PI;
                rotation_up_down = PI/3;
                position_x_axis = 0;
                position_y_axis = 0;
            }
            guiElementsStorage[GUI_SLIDER_SIZE].position_x = 0.5f;
            guiElementsStorage[GUI_SLIDER_SPEED].position_x = 0.5f;
            guiElementsStorage[GUI_SLIDER_WAVE_ROTATION].position_x = 0.5f;
            refereshGUI();

            //Animation Only
            ColorIntensity=0.4f*cos(((BlinkStep++)*PI/50.0f))+2.5f;
            if(BlinkStep>100){
                BlinkStep=1;
            }
            //TODO RESET
        }*/


        if(selectedGuiElement!= (-1)){
            switch(selectedGuiElement) {
            case GUI_SLIDER_SIZE:
            case GUI_SLIDER_WAVE_ROTATION:
                //draw_wave(selectedGuiElement);
                break;
            case GUI_SLIDER_SPEED:
                change_speed();
                break;
            case GUI_JOYSTICK_ROTATION:
                //rotate_camera(selectedGuiElement); Commented because the rotation would be faster if the gui element is moved slightly
                break;
            case GUI_JOYSTICK_MOVEMENT:
                //move_camera(selectedGuiElement);
                break;
            case GUI_JOYSTICK_WAVE_MOVE:
                //move_wave(selectedGuiElement);
                break;
            default:
                printf("Error: cursor pos callback guiElementsStorage[selectedGuiElement].position_x seems to have logged unregistered GUI Element!\n");
                break;
            }
        }


        //this probably takes ages
        /*int biggest = 0;
        biggest = 0;

        for(int i = 0; i < sim_res_total; i++) {
            if(psi[i][0]*psi[i][0] + psi[i][1]*psi[i][1] > psi[biggest][0]*psi[biggest][0] + psi[biggest][1]*psi[biggest][1])
                biggest = i;
        }
        //FOR AUTOCENTERING
        float sum = 0;
        float weighted_sum_x = 0;
        float weighted_sum_y = 0;

        for(int i = 0; i < sim_res_total; i++) {
            if(psi[i][0]*psi[i][0] + psi[i][1]*psi[i][1] > (psi[biggest][0]*psi[biggest][0] + psi[biggest][1]*psi[biggest][1])*0.9f){
                sum = sum + (psi[i][0]*psi[i][0] + psi[i][1]*psi[i][1]);
                weighted_sum_x = weighted_sum_x + (float)(i % Resolutionx) * (psi[i][0]*psi[i][0] + psi[i][1]*psi[i][1]);
                weighted_sum_y = weighted_sum_y + (float)(i / Resolutiony) * (psi[i][0]*psi[i][0] + psi[i][1]*psi[i][1]);
            }
        }

        float weight_x = ((weighted_sum_x/sum)-(((float)Resolutionx)/2.0f))/((float)Resolutionx);
        float weight_y = ((weighted_sum_y/sum)-(((float)Resolutiony)/2.0f))/((float)Resolutiony);

        //printf("%f,%f,%f,%f\n\n\n",position_x_axis,position_x_axis + (weight_x-position_x_axis)*0.5f,position_y_axis,position_y_axis + (weight_y-position_y_axis)*0.5f);
        if(!disable_autocenter){
            position_x_axis = position_x_axis + (weight_x-position_x_axis)*jerk_for_autocenter;
            position_y_axis = position_y_axis + (weight_y-position_y_axis)*jerk_for_autocenter;
        }
        //disable_autocenter = 0;

        double norming = sqrt(1.0f / (psi[biggest][0] * psi[biggest][0] + psi[biggest][1] * psi[biggest][1]));
        */
        double norming = 1.0;
        for(int i = 0; i < sim_res_total; i++) {
            speicher[i * 4 + 2] = (unsigned char)(0.5f * 255 * (psi[i][0] * norming + 1.0f));
            speicher[i * 4 + 1] = (unsigned char)(0.5f * 255 * (psi[i][1] * norming + 1.0f));
            speicher[i * 4 + 3] = pot[i * 4 + 1];
        }

        //@@Graphics
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, psiTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sim_res_x, sim_res_y, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, speicher);
        //Graphics@@
        //camera projection an transformation matrix calculation
        JoystickControll();
        /*eye_vec[0] = 1.5f * sin(rotation_left_right) * cos(atan(rotation_up_down));
        eye_vec[1] = 1.5f * cos(rotation_left_right) * cos(atan(rotation_up_down));
        eye_vec[2] = 1.5f * sin(atan(rotation_up_down));
        cent_vec[0] = position_x_axis;
        cent_vec[1] = position_y_axis;
        cent_vec[2]=0.0f;
        vec3_add(eye_vec,eye_vec,cent_vec); //apply offset to eye to have folloging effect*/
        //TODO GLOBAL ASPECT RATIO
        int width=0;
        int height=0;
        glfwGetWindowSize(MainWindow, &width, &height);
        mat4x4_ortho(mvp4x4, -0.55f*width/height, 0.55f*width/height, -0.55f, 0.55f, -1.0f, 5.0f);
        //mat4x4_look_at(mvp4x4, eye_vec, cent_vec, up_vec);
        //mat4x4_perspective(persp4x4, FOV, 16.0f / 9.0f, 0.15f, 10.0f); //TODO don't hardcode aspect ratio!!
        //mat4x4_mul(mvp4x4, persp4x4, mvp4x4);


        /*concept for async texture upload
        glBindTexture(GL_TEXTURE_2D, psi_texture);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER,PBO1);
        glTexSubImage2D(GL_TEXTURE_2D,0,0,0,Resolutionx,Resolutiony,GL_BGRA,GL_UNSIGNED_INT_8_8_8_8,NULL);
        //Make PBO2 ready to recieve new data
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER,PBO2);
        glBufferData(GL_PIXEL_UNPACK_BUFFER,4*Resolutionx*Resolutiony,0,GL_STREAM_DRAW);
        GLuint* data = glMapBuffer(GL_PIXEL_UNPACK_BUFFER,0,4*Resolutionx*Resolutiony,GL_MAP_WRITE_BIT);//Map buffer on gpu to client address space : offset 0,data,allow to write to buffer
        //write to data

        glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER); //start upload
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER,0);
        {//swap pixel buffers
            GLuint temp=PBO1;
            PBO1=PBO2;*0.5+0.5
            PBO2=temp;
        }*/

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        drawPlaneAndGrid(G_OBJECT_DRAW, PlaneRes, GridRes, mvp4x4);
        //drawTargetBox(G_OBJECT_DRAW,mvp4x4,ColorIntensity);//,sin(glfwGetTime()*3.0));

        drawTrackPoint(G_OBJECT_DRAW,mvp4x4,0,0);
        drawGui(G_OBJECT_DRAW,0);
        //Swap Buffers
        glFinish();
        glfwSwapBuffers(MainWindow);
        //Process Events
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}
typedef vec2 mat2x2[2];
static inline void mat2x2_mul_vec2(vec2 r, mat2x2 M, vec2 v)
{
	int i, j;
	for(j=0; j<2; ++j) {
		r[j] = 0.f;
		for(i=0; i<2; ++i)
			r[j] += M[i][j] * v[i];
	}
}

#define HalbeSchlaegerbreiteY 0.05f/2.f

void camera_collider(int C_OBJECT_STATE ,vec2 posNewIn){
    static vec2 posOld;
    static vec2 tempNewIn;
    tempNewIn[0]=posNewIn[0];
    tempNewIn[1]=posNewIn[1];
    if(C_OBJECT_STATE==C_OBJECT_INIT){
        posOld[0]=posNewIn[0];
        posOld[1]=posNewIn[1];
    }else{
        vec2 wavePos={wave_offset_x/sim_res_x,wave_offset_y/sim_res_y};
        //printf("wavPosraw: %f, %f\n",wavePos[0],wavePos[1]);
        //printf("oldPosraw: %f, %f\n",posOld[0],posOld[1]);
        //printf("newPosraw: %f, %f\n",posNewIn[0],posNewIn[1]);
        vec2_sub(wavePos,wavePos,posOld);
        vec2_sub(posNewIn,posNewIn,posOld);
        //printf("subwavPosraw: %f, %f\n",wavePos[0],wavePos[1]);
        //printf("subnewPosraw: %f, %f\n",posNewIn[0],posNewIn[1]);

        float angle=atan2(posNewIn[1],posNewIn[0]);

        float c=cosf(-angle);
        float s=sinf(-angle);
        mat2x2 Rot = {
            {   c,   s},
            {  -s,   c}
        };
        vec2 wavePosRot,posNewInRot;
        //printf("Matrix%f,%f\n       %f,%f\n", Rot[0][0],Rot[0][1],Rot[1][0],Rot[1][1]);
        mat2x2_mul_vec2(wavePosRot,Rot,wavePos);
        mat2x2_mul_vec2(posNewInRot,Rot,posNewIn);
       // printf("NewPos: %f, %f\n",posNewInRot[0],posNewInRot[1]);
       // printf("wavPos: %f, %f\n",wavePosRot[0],wavePosRot[1]);
        if(wavePosRot[1]<HalbeSchlaegerbreiteY&&wavePosRot[1]>-HalbeSchlaegerbreiteY){
            if(wavePosRot[0]>0&&wavePosRot[0]<posNewInRot[0]){
                printf("TODO MOMENTUM\n");
                simulation_redraw_wave((int)wave_offset_x,(int)wave_offset_y,angle,wave_momentum,wave_gauss_width);

                /*printf("angle %f\n", angle*180.f/M_PI);
                angle=angle-M_PI/2.0f;
                if(angle<0){
                    angle=2.0f*M_PI-angle;
                }
                guiElementsStorage[GUI_SLIDER_WAVE_ROTATION].position_x=angle/2.0f/M_PI;
                */

            }
        }
        posOld[0]=tempNewIn[0];
        posOld[1]=tempNewIn[1];
    }
}

void drawTrackPoint(int G_OBJECT_STATE,mat4x4 mvp4x4,float xpos, float ypos){
    static GLuint trackShaderID=0;
    static GLuint vertexBufferID=0;
    float data[18];
    static float savedxpos=0;
    static float savedypos=0;
    static GLuint mvpMatrixUniform=0;
    if(G_OBJECT_STATE==G_OBJECT_INIT){
        trackShaderID=glCreateProgram();
        glAttachShader(trackShaderID,CompileShaderFromFile(".\\res\\shaders\\vertex_track.glsl",GL_VERTEX_SHADER));
        glAttachShader(trackShaderID,CompileShaderFromFile(".\\res\\shaders\\fragment_track.glsl",GL_FRAGMENT_SHADER));
        glLinkProgram(trackShaderID);
        glUseProgram(trackShaderID);
        mvpMatrixUniform=glGetUniformLocation(trackShaderID,"MVPmatrix");
        glGenBuffers(1,&vertexBufferID);
    }else if(G_OBJECT_STATE==G_OBJECT_DRAW){
        data[2]=data[5]=data[8]=data[11]=data[14]=data[17]=1.0f; //Z=0
        data[0]=savedxpos-0.01f;
        data[1]=savedypos-0.01f;
        data[3]=data[9]=savedxpos-0.01f;
        data[4]=data[10]=savedypos+0.01f;
        data[6]=data[12]=savedxpos+0.01f;
        data[7]=data[13]=savedypos-0.01f;
        data[15]=savedxpos+0.01f;
        data[16]=savedypos+0.01f;

        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(data),data,GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(0);   //x,y
        //Enabler Shader
        glUseProgram(trackShaderID);
        glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, (GLfloat*)mvp4x4);
        //Set Shader Uniforms to render Grid
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glDrawArrays(GL_TRIANGLES,0,6);
        glDisableVertexAttribArray(0);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
    }else if(G_OBJECT_STATE==G_OBJECT_UPDATE){
        savedxpos=xpos;
        savedypos=ypos;
    }
}

/*void drawTargetBox(int G_OBJECT_STATE,mat4x4 mvp4x4,float Intensity){
    static GLuint vboTargetBoxID=0;
    static GLuint targetShaderID=0;
    static GLuint IntensityFloatUniform = 0;
    static GLuint mvpMatrixUniform = 0; //How is the Uniform variable called in the compiled shader
    if(G_OBJECT_STATE==G_OBJECT_INIT){
        //Compile Shaders
        targetShaderID = glCreateProgram();              //create program to run on GPU
        glAttachShader(targetShaderID, CompileShaderFromFile(filepath_shader_vertex_target, GL_VERTEX_SHADER));       //attach vertex shader to new program
        glAttachShader(targetShaderID, CompileShaderFromFile(filepath_shader_fragment_target, GL_FRAGMENT_SHADER));      //attach fragment shader to new program
        glLinkProgram(targetShaderID);
        //Get Shader Variables
        glUseProgram(targetShaderID);
        mvpMatrixUniform = glGetUniformLocation(targetShaderID, "MVPmatrix");   //only callable after glUseProgramm has been called once
        IntensityFloatUniform = glGetUniformLocation(targetShaderID, "Intensity");
        glGenBuffers(1,&vboTargetBoxID);
        float VertexData[]={
            VertMinX,VertMinY,VertMinZ,VertMaxX,VertMinY,VertMinZ,VertMinX,VertMinY,VertMaxZ,//face 1
            VertMaxX,VertMinY,VertMinZ,VertMaxX,VertMinY,VertMaxZ,VertMinX,VertMinY,VertMaxZ,
            VertMaxX,VertMinY,VertMinZ,VertMaxX,VertMaxY,VertMinZ,VertMaxX,VertMinY,VertMaxZ,//face 2
            VertMaxX,VertMaxY,VertMinZ,VertMaxX,VertMaxY,VertMaxZ,VertMaxX,VertMinY,VertMaxZ,
            VertMaxX,VertMaxY,VertMinZ,VertMinX,VertMaxY,VertMinZ,VertMaxX,VertMaxY,VertMaxZ,//face 3
            VertMinX,VertMaxY,VertMinZ,VertMinX,VertMaxY,VertMaxZ,VertMaxX,VertMaxY,VertMaxZ,
            VertMinX,VertMaxY,VertMinZ,VertMinX,VertMinY,VertMinZ,VertMinX,VertMaxY,VertMaxZ,//face 4
            VertMinX,VertMinY,VertMinZ,VertMinX,VertMinY,VertMaxZ,VertMinX,VertMaxY,VertMaxZ,
            VertMinX,VertMinY,VertMinZ,VertMaxX,VertMinY,VertMinZ,VertMinX,VertMaxY,VertMinZ,//bottomface inverted (normal inward)
            VertMaxX,VertMinY,VertMinZ,VertMaxX,VertMaxY,VertMinZ,VertMinX,VertMaxY,VertMinZ
            //no bottomface
        };
        printf("DEBUG: sizeof: %d\n",sizeof(VertexData));
        glBindBuffer(GL_ARRAY_BUFFER,vboTargetBoxID);
        glBufferData(GL_ARRAY_BUFFER,sizeof(VertexData),VertexData,GL_STATIC_DRAW);//sizeof
        //
        //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }else if(G_OBJECT_STATE==G_OBJECT_DRAW){
        glBindBuffer(GL_ARRAY_BUFFER, vboTargetBoxID);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(0);   //x,y
        //Enabler Shader
        glUseProgram(targetShaderID);
        //Set Shader Uniforms to render Grid
        glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, (GLfloat*)mvp4x4);
        glUniform1f(IntensityFloatUniform,Intensity);
        glEnable(GL_BLEND);
        //glDisable(GL_DEPTH_TEST);
        glBlendFunc(GL_ONE_MINUS_DST_ALPHA,GL_DST_ALPHA);
        glDrawArrays(GL_TRIANGLES,0,6);
        glDrawArrays(GL_TRIANGLES,6,6);
        glDrawArrays(GL_TRIANGLES,12,6);
        glDrawArrays(GL_TRIANGLES,18,6); //12-2 because bottom face is missing
        //glDrawArrays(GL_TRIANGLES,24,6);
        //glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
    }
}*/
void drawPlaneAndGrid(int G_OBJECT_STATE, unsigned int PlaneResolution, unsigned int GridResolution, mat4x4 mvp4x4) {
    //3d object info
    //uses Texture0 which is constantly updated
    static GLint maxSupportedIndices = 0;
    static unsigned long indexBufferCountPlane = 0;
    static unsigned long indexBufferCountGrid = 0;
    static GLuint VboVerticesId = 0;
    static GLuint* iboPlanePointer = 0;
    static GLuint* iboGridPointer = 0;
    static long indicesInLastPlaneBuffer = 0;
    static long indicesInLastGridBuffer = 0;
    static GLuint mvpMatrixUniform = 0;
    static GLuint renderGridOrPlaneUniform = 0;
    static GLuint gridAndPlaneShaderID = 0;
    if(G_OBJECT_STATE == G_OBJECT_INIT) {
        //Compile Shaders
        gridAndPlaneShaderID = glCreateProgram();              //create program to run on GPU
        glAttachShader(gridAndPlaneShaderID, CompileShaderFromFile(filepath_shader_vertex_graph, GL_VERTEX_SHADER));       //attach vertex shader to new program
        glAttachShader(gridAndPlaneShaderID, CompileShaderFromFile(filepath_shader_fragment_graph, GL_FRAGMENT_SHADER));      //attach fragment shader to new program
        glLinkProgram(gridAndPlaneShaderID);

        //Get Shader Variables
        glUseProgram(gridAndPlaneShaderID);
        mvpMatrixUniform = glGetUniformLocation(gridAndPlaneShaderID, "MVPmatrix");   //only callable after glUseProgramm has been called once
        renderGridOrPlaneUniform = glGetUniformLocation(gridAndPlaneShaderID, "potential_true");

        //Texture initialisation for GL_TEXTURE0 (texture is generated outside because we need its address elsewhere
        glUniform1i(glGetUniformLocation(gridAndPlaneShaderID, "texture0"), 0);

        //input checking
        if(((PlaneResolution & (PlaneResolution - 1)) != 0) || ((GridResolution & (GridResolution - 1)) != 0)) {               //Check if plane resolution/grid resolution is power of 2
            printf("Error: Resolution of plane or grid is not a power of 2.\nError: Initialization failed!");
            return;
        }
        {
            GLint maxSupportedVertices = 0;
            glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &maxSupportedVertices);
            if(maxSupportedVertices < 1048576) {
                printf("Error: Vertex Resolution of your Graphic Card are to low. (Required Minimum 1024*1024)\nError: Initialization failed!");
                return;
            }
        }


        //update Value of max supported indices
        glGetIntegerv(GL_MAX_ELEMENTS_INDICES, &maxSupportedIndices);

        //Calculate requiered buffercount
        if(((PlaneResolution - 1) * (PlaneResolution - 1) * 6) % ((maxSupportedIndices / 3) * 3) == 0) {             //
            indexBufferCountPlane = (((PlaneResolution - 1) * (PlaneResolution - 1) * 6) / ((maxSupportedIndices / 3) * 3));
        } else {
            indexBufferCountPlane = (((PlaneResolution - 1) * (PlaneResolution - 1) * 6) / ((maxSupportedIndices / 3) * 3)) + 1;
        }
        if(((GridResolution - 1) * (GridResolution - 1) * 8) % ((maxSupportedIndices / 2) * 2) == 0) {
            indexBufferCountGrid = (((GridResolution - 1) * (GridResolution - 1) * 8) / ((maxSupportedIndices / 2) * 2));
        } else {
            indexBufferCountGrid = (((GridResolution - 1) * (GridResolution - 1) * 8) / ((maxSupportedIndices / 2) * 2)) + 1;
        }
        //generate arrays
        iboPlanePointer = malloc(indexBufferCountPlane * sizeof(GLuint));
        iboGridPointer = malloc(indexBufferCountGrid * sizeof(GLuint));
        //calculate vertex count and offset if grid- and plane-resolution are different
        unsigned int finalVertexResolution = 0;
        unsigned int gridOffsetMultiplier = 0;
        unsigned int planeOffsetMultiplier = 0;
        if(PlaneResolution > GridResolution) {
            finalVertexResolution = PlaneResolution;
            gridOffsetMultiplier = PlaneResolution / GridResolution;
            planeOffsetMultiplier = 1;
        } else {
            finalVertexResolution = GridResolution;
            planeOffsetMultiplier = GridResolution / PlaneResolution;
            gridOffsetMultiplier = 1;
        }
        printf("Info: gridMul %d\nInfo: planeMul %d\n", gridOffsetMultiplier, planeOffsetMultiplier);
        //Generate Vertices for grid and plane
        float* plane_vertex_data = malloc(2 * finalVertexResolution * finalVertexResolution * sizeof(float));
        unsigned long tempVertIndex = 0;
        for(int y = 0; y < finalVertexResolution; y++) {
            for(int x = 0; x < finalVertexResolution; x++) {
                //Vector coordinates (x,y,z)
                plane_vertex_data[tempVertIndex++] = (((float)x) / (finalVertexResolution - 1)) - 0.5f;
                plane_vertex_data[tempVertIndex++] = (((float)y) / (finalVertexResolution - 1)) - 0.5f;         //Set height (y) to zero
            }
        }
        glGenBuffers(1, &VboVerticesId);                                                          //create buffer
        glBindBuffer(GL_ARRAY_BUFFER, VboVerticesId);                          //Link buffer
        glBufferData(GL_ARRAY_BUFFER, 2 * finalVertexResolution * finalVertexResolution * sizeof(float), plane_vertex_data, GL_STATIC_DRAW);     //Upload data to Buffer, Vertex data is set only once and drawn regularly, hence we use GL_STATIC_DRAW
        free(plane_vertex_data);    //we no longer need plane_vertex_data because it has been uploaded to gpu memory
        printf("Info: vertices for plane/grid successfully generated\n");

        //Generate Vertex Indices for Plane
        GLuint* plane_indices = malloc((finalVertexResolution - 1) * (finalVertexResolution - 1) * 8 * sizeof(GLuint));         //6 from the points of two triangles, 8 from 4 lines per gridcell max(6,8)=8
        tempVertIndex = 0;
        for(unsigned int y = 0; y < (finalVertexResolution - planeOffsetMultiplier); y += planeOffsetMultiplier) {
            for(unsigned int x = 0; x < (finalVertexResolution - planeOffsetMultiplier); x += planeOffsetMultiplier) {
                //Generate first triangle
                plane_indices[tempVertIndex++] = x + (y * finalVertexResolution);   //Vertex lower left first triangle
                plane_indices[tempVertIndex++] = x + planeOffsetMultiplier + (y * finalVertexResolution);   //Vertex upper right first triangle
                plane_indices[tempVertIndex++] = x + ((y + planeOffsetMultiplier) * finalVertexResolution);     //Vertex upper left first triangle
                //Generate second triangle
                plane_indices[tempVertIndex++] = x + planeOffsetMultiplier + (y * finalVertexResolution);   //Vertex lower left second triangle
                plane_indices[tempVertIndex++] = x + planeOffsetMultiplier + ((y + planeOffsetMultiplier) * finalVertexResolution);     //Vertex lower right second triangle
                plane_indices[tempVertIndex++] = x + ((y + planeOffsetMultiplier) * finalVertexResolution);     //Vertex upper right first triangle
                //printf("vert%d,%d,%d,%d,%d,%d\n",plane_indices[vert_index-6],plane_indices[vert_index-5],plane_indices[vert_index-4],plane_indices[vert_index-3],plane_indices[vert_index-2],plane_indices[vert_index-1]);
            }
        }

        glGenBuffers(indexBufferCountPlane, iboPlanePointer);
        //Now upload this data to GPU
        unsigned int bufferNumber = 0;
        for(; bufferNumber < (indexBufferCountPlane - 1); bufferNumber++) {     //Upload all but the last buffer
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboPlanePointer[bufferNumber]);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, (maxSupportedIndices / 3) * 3 * sizeof(GLuint), plane_indices + (maxSupportedIndices / 3) * 3 * bufferNumber, GL_STATIC_DRAW);
        }
        //Upload the last Buffer for plane
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboPlanePointer[bufferNumber]);
        if(tempVertIndex % ((maxSupportedIndices / 3) * 3) != 0) {
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, (tempVertIndex % ((maxSupportedIndices / 3) * 3))*sizeof(GLuint), plane_indices + ((maxSupportedIndices / 3) * 3)*bufferNumber, GL_STATIC_DRAW);
        } else {
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, ((maxSupportedIndices / 3) * 3)*sizeof(GLuint), plane_indices + ((maxSupportedIndices / 3) * 3)*bufferNumber, GL_STATIC_DRAW);
        }
        indicesInLastPlaneBuffer = tempVertIndex % ((maxSupportedIndices / 3) * 3);
        printf("Info: indices for plane successfully generated\n");

        //Generate Vertex Indices for Grid
        tempVertIndex = 0;
        for(unsigned int y = 0; y < (finalVertexResolution - gridOffsetMultiplier); y += gridOffsetMultiplier) {
            for(unsigned int x = 0; x < (finalVertexResolution - gridOffsetMultiplier); x += gridOffsetMultiplier) {
                //Generate first line
                plane_indices[tempVertIndex++] = x + (y * finalVertexResolution);
                plane_indices[tempVertIndex++] = x + gridOffsetMultiplier + (y * finalVertexResolution);
                //Generate second line
                plane_indices[tempVertIndex++] = x + (y * finalVertexResolution);
                plane_indices[tempVertIndex++] = x + ((y + gridOffsetMultiplier) * finalVertexResolution);
                //Generate third line
                plane_indices[tempVertIndex++] = x + ((y + gridOffsetMultiplier) * finalVertexResolution);
                plane_indices[tempVertIndex++] = x + gridOffsetMultiplier + ((y + gridOffsetMultiplier) * finalVertexResolution);
                //Generate fourth line
                plane_indices[tempVertIndex++] = x + gridOffsetMultiplier + (y * finalVertexResolution);
                plane_indices[tempVertIndex++] = x + gridOffsetMultiplier + ((y + gridOffsetMultiplier) * finalVertexResolution);
            }
        }

        glGenBuffers(indexBufferCountGrid, iboGridPointer);   //Skip over 2*int+(2*long==4*int)=6
        bufferNumber = 0;
        for(; bufferNumber < (indexBufferCountGrid - 1); bufferNumber++) {     //Upload all but the last buffer
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboGridPointer[bufferNumber]);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, ((maxSupportedIndices / 2) * 2)*sizeof(GLuint), plane_indices + ((maxSupportedIndices / 2) * 2)*bufferNumber, GL_STATIC_DRAW);
        }
        //Upload the last Buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboGridPointer[bufferNumber]);
        if(tempVertIndex % ((maxSupportedIndices / 2) * 2) != 0) {
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, (tempVertIndex % ((maxSupportedIndices / 2) * 2))*sizeof(GLuint), plane_indices + ((maxSupportedIndices / 2) * 2)*bufferNumber, GL_STATIC_DRAW);
        } else {
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, ((maxSupportedIndices / 2) * 2)*sizeof(GLuint), plane_indices + ((maxSupportedIndices / 2) * 2)*bufferNumber, GL_STATIC_DRAW);
        }
        indicesInLastGridBuffer = tempVertIndex % ((maxSupportedIndices / 2) * 2);
        printf("Info: indices for grid successfully generated\n");
        free(plane_indices);                //Cleanup Array for indices
    } else if(G_OBJECT_STATE == G_OBJECT_DRAW) {
        //Draw Call for Grid and Plane, this does not use vao because we have multiple GL_ELEMENT_ARRAY_BUFFER to draw only one object
        glBindBuffer(GL_ARRAY_BUFFER, VboVerticesId);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);   //x,y
        //Enabler Shader
        glUseProgram(gridAndPlaneShaderID);
        //Set Shader Uniforms to render Grid
        glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, (GLfloat*)mvp4x4);
        glUniform1f(renderGridOrPlaneUniform, 1.0f);
        unsigned int buffernumber;
        //Smooth lines?
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glLineWidth(1.0);
        //Draw all vertices but those in the last index buffer
        for(buffernumber = 0; buffernumber < (indexBufferCountGrid - 1); buffernumber++) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboGridPointer[buffernumber]);
            glDrawElements(GL_LINES, (maxSupportedIndices / 2) * 2, GL_UNSIGNED_INT, 0);
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboGridPointer[buffernumber]);
        glDrawElements(GL_LINES, indicesInLastGridBuffer, GL_UNSIGNED_INT, 0);
        //enable Transparency TODO remove?
        //glEnable(GL_BLEND);
        //glBlendFunc(GL_ONE,GL_ONE);
        //glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

        //Render Plane
        glUniform1f(renderGridOrPlaneUniform, 0.0f);
        for(buffernumber = 0; buffernumber < (indexBufferCountPlane - 1); buffernumber++) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboPlanePointer[buffernumber]);
            glDrawElements(GL_TRIANGLES, (maxSupportedIndices / 3) * 3, GL_UNSIGNED_INT, 0);
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboPlanePointer[buffernumber]);
        glDrawElements(GL_TRIANGLES, indicesInLastPlaneBuffer, GL_UNSIGNED_INT, 0);
    }
}

void drawGui(int G_OBJECT_STATE, float aspectRatio) {
    //create and activate vertexArrayObject

    static GLuint vaoID = 0;
    static GLuint vboIndicesID = 0;
    static GLuint vboVertexID = 0;
    static GLuint guiShaderID = 0;
    static GLuint textureId = 0;
    static int numberOfQuads = 3;
    if(G_OBJECT_STATE == G_OBJECT_INIT) {
        //Compile Shader
        guiShaderID = glCreateProgram();
        glAttachShader(guiShaderID, CompileShaderFromFile(filepath_shader_vertex_gui, GL_VERTEX_SHADER));        //attach vertex shader to new program
        glAttachShader(guiShaderID, CompileShaderFromFile(filepath_shader_fragment_gui, GL_FRAGMENT_SHADER));     //attach fragment shader to new program
        glLinkProgram(guiShaderID);
        //Set texture for gui shader
        glActiveTexture(GL_TEXTURE1);
        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        void* tempClientGuiTexture = read_bmp(filepath_gui_bmp);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, tempClientGuiTexture);
        free(tempClientGuiTexture);

        glUseProgram(guiShaderID);
        glUniform1i(glGetUniformLocation(guiShaderID, "texture1"), 1);
        glGenVertexArrays(1, &vaoID);
        glBindVertexArray(vaoID);
        glGenBuffers(1, &vboVertexID);
        glBindBuffer(GL_ARRAY_BUFFER, vboVertexID);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);
        glGenBuffers(1, &vboIndicesID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndicesID);
        glBindVertexArray(0);

    } else if(G_OBJECT_STATE == G_OBJECT_DRAW) {
        glBindVertexArray(vaoID);   //This binds all buffers (vertices, indicesAndUVs)
        glUseProgram(guiShaderID);
        glActiveTexture(GL_TEXTURE1);
        glDisable(GL_DEPTH_TEST);
        //TODO transparency
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDrawElements(GL_TRIANGLES, 6 * numberOfQuads, GL_UNSIGNED_INT, 0);   //TODO dynamic
        glDisable(GL_BLEND);
        glBindVertexArray(0);
        glEnable(GL_DEPTH_TEST);
        glActiveTexture(GL_TEXTURE0);
    } else if(G_OBJECT_STATE == G_OBJECT_UPDATE) {
        numberOfQuads = 0;
        for(int gElmt = 0; gElmt < numberOfGuiElements; gElmt++) {
            switch(guiElementsStorage[gElmt].GUI_TYPE) {
            case GUI_TYPE_SLIDER_SIZE:
            case GUI_TYPE_SLIDER_TIME:
            case GUI_TYPE_SLIDER_ANGLE:
                numberOfQuads += 3;
                break;
            case GUI_TYPE_BUTTON_CONTROL:
            case GUI_TYPE_BUTTON_POTENTIAL:
                numberOfQuads += 1;
                break;
            case GUI_TYPE_JOYSTICK_ROTATION:
            case GUI_TYPE_JOYSTICK_MOVEMENT:
            case GUI_TYPE_JOYSTICK_WAVE_MOVE:
                numberOfQuads += 2; //one for button and one for surounding box
                break;
            }
        }
        float* GUI_positions_and_uv = (float*)malloc(16 * numberOfQuads * sizeof(float));       //(*2 UV and XY Positions) (*4 vertices) (*2 each x,y)
        GLuint* GUI_indices = (GLuint*)malloc(6 * numberOfQuads * sizeof(GLuint));
        int offsetInGuiPaUV = 0;
        for(int gElmt = 0; gElmt < numberOfGuiElements; gElmt++) {
            if(guiElementsStorage[gElmt].GUI_TYPE == GUI_TYPE_SLIDER_SIZE || guiElementsStorage[gElmt].GUI_TYPE == GUI_TYPE_SLIDER_TIME || guiElementsStorage[gElmt].GUI_TYPE == GUI_TYPE_SLIDER_ANGLE) {
                float glCoordsX = 2.0f * (guiElementsStorage[gElmt].top_left_x - 0.5f);   //Transform coordinates from [0,1] to [-1,1]
                float glCoordsY = -2.0f * (guiElementsStorage[gElmt].top_left_y * aspectRatio - 0.5f);   //Transform coordinates from [0,1] to [-1,1]
                float glCoordsSize = 2.0f * guiElementsStorage[gElmt].percentOfWidth;   //Transform coordinates from [0,1] to [-1,1]
                //Positions
                //Are in openGl coordinate system so from [-1..1] for x and y where x is positive to the right and y is positive to the top
                //Part 1 slider
                //Left part slider lower left vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsX;
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsY - aspectRatio * glCoordsSize * (40.0f / 512.0f);
                //Left part slider lower right vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsX + ((guiElementsStorage[gElmt].position_x * 440.0f + 4.0f) / 512.0f) * glCoordsSize;
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsY - aspectRatio * glCoordsSize * (40.0f / 512.0f);
                //Left part slider upper left vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsX;
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsY - aspectRatio * glCoordsSize * (24.0f / 512.0f);
                //Left part slider upper right vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsX + ((guiElementsStorage[gElmt].position_x * 440.0f + 4.0f) / 512.0f) * glCoordsSize;
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsY - aspectRatio * glCoordsSize * (24.0f / 512.0f);

                //Part 2 button
                //Left part slider lower left vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsX + ((guiElementsStorage[gElmt].position_x * 440.0f + 4.0f) / 512.0f) * glCoordsSize;
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsY - aspectRatio * glCoordsSize * (64.0f / 512.0f);
                //Left part slider lower right vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsX + ((guiElementsStorage[gElmt].position_x * 440.0f + 68.0f) / 512.0f) * glCoordsSize;
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsY - aspectRatio * glCoordsSize * (64.0f / 512.0f);
                //Left part slider upper left vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsX + ((guiElementsStorage[gElmt].position_x * 440.0f + 4.0f) / 512.0f) * glCoordsSize;
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsY;
                //Left part slider upper right vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsX + ((guiElementsStorage[gElmt].position_x * 440.0f + 68.0f) / 512.0f) * glCoordsSize;
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsY;

                //Part 3 slider
                //Left part slider lower left vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsX + ((guiElementsStorage[gElmt].position_x * 440.0f + 68.0f) / 512.0f) * glCoordsSize;
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsY - aspectRatio * glCoordsSize * (40.0f / 512.0f);
                //Left part slider lower right vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsX + glCoordsSize;
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsY - aspectRatio * glCoordsSize * (40.0f / 512.0f);
                //Left part slider upper left vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsX + ((guiElementsStorage[gElmt].position_x * 440.0f + 68.0f) / 512.0f) * glCoordsSize;
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsY - aspectRatio * glCoordsSize * (24.0f / 512.0f);
                //Left part slider upper right vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsX + glCoordsSize;
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsY - aspectRatio * glCoordsSize * (24.0f / 512.0f);
            } else if(guiElementsStorage[gElmt].GUI_TYPE == GUI_TYPE_BUTTON_CONTROL || guiElementsStorage[gElmt].GUI_TYPE == GUI_TYPE_BUTTON_POTENTIAL) {
                float glCoordsX = 2.0f * (guiElementsStorage[gElmt].top_left_x - 0.5f);   //Transform coordinates from [0,1] to [-1,1]
                float glCoordsY = -2.0f * (guiElementsStorage[gElmt].top_left_y * aspectRatio - 0.5f);   //Transform coordinates from [0,1] to [-1,1]
                float glCoordsSize = 2.0f * guiElementsStorage[gElmt].percentOfWidth;   //Transform coordinates from [0,1] to [-1,1]
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsX;
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsY - aspectRatio * glCoordsSize * 0.25f;
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsX + glCoordsSize;
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsY - aspectRatio * glCoordsSize * 0.25f;
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsX;
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsY;
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsX + glCoordsSize;
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsY;
            } else if(guiElementsStorage[gElmt].GUI_TYPE == GUI_TYPE_JOYSTICK_MOVEMENT || guiElementsStorage[gElmt].GUI_TYPE == GUI_TYPE_JOYSTICK_ROTATION ||guiElementsStorage[gElmt].GUI_TYPE == GUI_TYPE_JOYSTICK_WAVE_MOVE) {
                float positionX = guiElementsStorage[gElmt].position_x;
                float positionY = guiElementsStorage[gElmt].position_y;
                float glCoordsX = 2.0f * ((guiElementsStorage[gElmt].top_left_x) - 0.5f);     //Transform coordinates from [0,1] to [-1,1]
                float glCoordsY = -2.0f * (guiElementsStorage[gElmt].top_left_y * aspectRatio - 0.5f);   //Transform coordinates from [0,1] to [-1,1]
                float glCoordsSize = 2.0f * guiElementsStorage[gElmt].percentOfWidth;   //Transform coordinates from [0,1] to [-1,1]
                //for box
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsX; //LOWERLEFT
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsY - aspectRatio * glCoordsSize;
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsX + glCoordsSize; //LOWERRIGHT
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsY - aspectRatio * glCoordsSize;
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsX; //UPPERLEFT
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsY;
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsX + glCoordsSize; //UPPERRIGHT
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsY;
                //for button
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsX + glCoordsSize * (positionX * 0.5f + 0.5f) * (1 - GUI_JOYSTICK_PROPERTY_SCALE); //LOWERLEFT
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsY - aspectRatio * glCoordsSize * (1 - (1 - GUI_JOYSTICK_PROPERTY_SCALE) * (positionY * (-0.5f) + 0.5f));
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsX + glCoordsSize * (1 - (1 - GUI_JOYSTICK_PROPERTY_SCALE) * (positionX * (-0.5f) + 0.5f)); //LOWERRIGHT
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsY - aspectRatio * glCoordsSize * (1 - (1 - GUI_JOYSTICK_PROPERTY_SCALE) * (positionY * (-0.5f) + 0.5f));
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsX + glCoordsSize * (positionX * 0.5f + 0.5f) * (1 - GUI_JOYSTICK_PROPERTY_SCALE); //UPPERLEFT
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsY - aspectRatio * glCoordsSize * ((positionY * 0.5f + 0.5f) * (1 - GUI_JOYSTICK_PROPERTY_SCALE));
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsX + glCoordsSize * (1 - (1 - GUI_JOYSTICK_PROPERTY_SCALE) * (positionX * (-0.5f) + 0.5f)); //UPPERRIGHT
                GUI_positions_and_uv[offsetInGuiPaUV++] = glCoordsY - aspectRatio * glCoordsSize * ((positionY * 0.5f + 0.5f) * (1 - GUI_JOYSTICK_PROPERTY_SCALE));
            }
        }
        for(int gElmt = 0; gElmt < numberOfGuiElements; gElmt++) {
            if(guiElementsStorage[gElmt].GUI_TYPE == GUI_TYPE_JOYSTICK_WAVE_MOVE || guiElementsStorage[gElmt].GUI_TYPE == GUI_TYPE_JOYSTICK_ROTATION || guiElementsStorage[gElmt].GUI_TYPE == GUI_TYPE_JOYSTICK_MOVEMENT){
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_JOYSTICK_BORDER_TOP_LEFT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_JOYSTICK_BORDER_DOWN_RIGHT_Y;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_JOYSTICK_BORDER_DOWN_RIGHT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_JOYSTICK_BORDER_DOWN_RIGHT_Y;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_JOYSTICK_BORDER_TOP_LEFT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_JOYSTICK_BORDER_TOP_LEFT_Y;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_JOYSTICK_BORDER_DOWN_RIGHT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_JOYSTICK_BORDER_TOP_LEFT_Y;
            }
            if(guiElementsStorage[gElmt].GUI_TYPE == GUI_TYPE_SLIDER_SIZE) {
                //UV
                //Part 1 slider
                //Left part slider lower left vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_ACTIVE_TOP_LEFT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_ACTIVE_DOWN_RIGHT_Y;
                //Left part slider lower right vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_ACTIVE_TOP_LEFT_X + ((guiElementsStorage[gElmt].position_x * 440.0f + 4.0f) / 1024.0f);
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_ACTIVE_DOWN_RIGHT_Y;
                //Left part slider upper left vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_ACTIVE_TOP_LEFT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_ACTIVE_TOP_LEFT_Y;
                //Left part slider upper right vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_ACTIVE_TOP_LEFT_X + ((guiElementsStorage[gElmt].position_x * 440.0f + 4.0f) / 1024.0f);
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_ACTIVE_TOP_LEFT_Y;

                //Part 2 middle slider
                //Left part slider lower left vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_SIZE_BUTTON_TOP_LEFT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_SIZE_BUTTON_DOWN_RIGHT_Y;
                //Left part slider lower right vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_SIZE_BUTTON_DOWN_RIGHT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_SIZE_BUTTON_DOWN_RIGHT_Y;
                //Left part slider upper left vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_SIZE_BUTTON_TOP_LEFT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_SIZE_BUTTON_TOP_LEFT_Y;
                //Left part slider upper right vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_SIZE_BUTTON_DOWN_RIGHT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_SIZE_BUTTON_TOP_LEFT_Y;

                //Part 3 slider (inactive)
                //Left part slider lower left vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_INACTIVE_TOP_LEFT_X + ((guiElementsStorage[gElmt].position_x * 440.0f + 68.0f) / 1024.0f);
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_INACTIVE_DOWN_RIGHT_Y;
                //Left part slider lower right vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_INACTIVE_DOWN_RIGHT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_INACTIVE_DOWN_RIGHT_Y;
                //Left part slider upper left vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_INACTIVE_TOP_LEFT_X + ((guiElementsStorage[gElmt].position_x * 440.0f + 68.0f) / 1024.0f);
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_INACTIVE_TOP_LEFT_Y;
                //Left part slider upper right vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_INACTIVE_DOWN_RIGHT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_INACTIVE_TOP_LEFT_Y;
            }else if(guiElementsStorage[gElmt].GUI_TYPE == GUI_TYPE_SLIDER_TIME) {
                //UV
                //Part 1 slider
                //Left part slider lower left vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_ACTIVE_TOP_LEFT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_ACTIVE_DOWN_RIGHT_Y;
                //Left part slider lower right vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_ACTIVE_TOP_LEFT_X + ((guiElementsStorage[gElmt].position_x * 440.0f + 4.0f) / 1024.0f);
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_ACTIVE_DOWN_RIGHT_Y;
                //Left part slider upper left vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_ACTIVE_TOP_LEFT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_ACTIVE_TOP_LEFT_Y;
                //Left part slider upper right vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_ACTIVE_TOP_LEFT_X + ((guiElementsStorage[gElmt].position_x * 440.0f + 4.0f) / 1024.0f);
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_ACTIVE_TOP_LEFT_Y;

                //Part 2 middle slider
                //Left part slider lower left vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_TIME_BUTTON_TOP_LEFT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_TIME_BUTTON_DOWN_RIGHT_Y;
                //Left part slider lower right vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_TIME_BUTTON_DOWN_RIGHT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_TIME_BUTTON_DOWN_RIGHT_Y;
                //Left part slider upper left vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_TIME_BUTTON_TOP_LEFT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_TIME_BUTTON_TOP_LEFT_Y;
                //Left part slider upper right vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_TIME_BUTTON_DOWN_RIGHT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_TIME_BUTTON_TOP_LEFT_Y;

                //Part 3 slider (inactive)
                //Left part slider lower left vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_INACTIVE_TOP_LEFT_X + ((guiElementsStorage[gElmt].position_x * 440.0f + 68.0f) / 1024.0f);
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_INACTIVE_DOWN_RIGHT_Y;
                //Left part slider lower right vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_INACTIVE_DOWN_RIGHT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_INACTIVE_DOWN_RIGHT_Y;
                //Left part slider upper left vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_INACTIVE_TOP_LEFT_X + ((guiElementsStorage[gElmt].position_x * 440.0f + 68.0f) / 1024.0f);
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_INACTIVE_TOP_LEFT_Y;
                //Left part slider upper right vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_INACTIVE_DOWN_RIGHT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_INACTIVE_TOP_LEFT_Y;
            }else if(guiElementsStorage[gElmt].GUI_TYPE == GUI_TYPE_SLIDER_ANGLE) {
                //UV
                //Part 1 slider
                //Left part slider lower left vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_ACTIVE_TOP_LEFT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_ACTIVE_DOWN_RIGHT_Y;
                //Left part slider lower right vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_ACTIVE_TOP_LEFT_X + ((guiElementsStorage[gElmt].position_x * 440.0f + 4.0f) / 1024.0f);
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_ACTIVE_DOWN_RIGHT_Y;
                //Left part slider upper left vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_ACTIVE_TOP_LEFT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_ACTIVE_TOP_LEFT_Y;
                //Left part slider upper right vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_ACTIVE_TOP_LEFT_X + ((guiElementsStorage[gElmt].position_x * 440.0f + 4.0f) / 1024.0f);
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_ACTIVE_TOP_LEFT_Y;

                //Part 2 middle slider
                //Left part slider lower left vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_ANGLE_BUTTON_TOP_LEFT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_ANGLE_BUTTON_DOWN_RIGHT_Y;
                //Left part slider lower right vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_ANGLE_BUTTON_DOWN_RIGHT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_ANGLE_BUTTON_DOWN_RIGHT_Y;
                //Left part slider upper left vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_ANGLE_BUTTON_TOP_LEFT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_ANGLE_BUTTON_TOP_LEFT_Y;
                //Left part slider upper right vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_ANGLE_BUTTON_DOWN_RIGHT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_ANGLE_BUTTON_TOP_LEFT_Y;

                //Part 3 slider (inactive)
                //Left part slider lower left vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_INACTIVE_TOP_LEFT_X + ((guiElementsStorage[gElmt].position_x * 440.0f + 68.0f) / 1024.0f);
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_INACTIVE_DOWN_RIGHT_Y;
                //Left part slider lower right vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_INACTIVE_DOWN_RIGHT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_INACTIVE_DOWN_RIGHT_Y;
                //Left part slider upper left vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_INACTIVE_TOP_LEFT_X + ((guiElementsStorage[gElmt].position_x * 440.0f + 68.0f) / 1024.0f);
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_INACTIVE_TOP_LEFT_Y;
                //Left part slider upper right vertex
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_INACTIVE_DOWN_RIGHT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_SLIDER_BAR_INACTIVE_TOP_LEFT_Y;
            }else if(guiElementsStorage[gElmt].GUI_TYPE == GUI_TYPE_BUTTON_CONTROL) {
                if(guiElementsStorage[gElmt].position_x == GUI_STATE_BUTTON1_RESET) {
                    GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_BUTTON_NEW_TOP_LEFT_X;
                    GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_BUTTON_NEW_DOWN_RIGHT_Y;
                    GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_BUTTON_NEW_DOWN_RIGHT_X;
                    GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_BUTTON_NEW_DOWN_RIGHT_Y;
                    GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_BUTTON_NEW_TOP_LEFT_X;
                    GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_BUTTON_NEW_TOP_LEFT_Y;
                    GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_BUTTON_NEW_DOWN_RIGHT_X;
                    GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_BUTTON_NEW_TOP_LEFT_Y;
                } else if(guiElementsStorage[gElmt].position_x == GUI_STATE_BUTTON1_START) {
                    GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_BUTTON_START_TOP_LEFT_X;
                    GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_BUTTON_START_DOWN_RIGHT_Y;
                    GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_BUTTON_START_DOWN_RIGHT_X;
                    GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_BUTTON_START_DOWN_RIGHT_Y;
                    GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_BUTTON_START_TOP_LEFT_X;
                    GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_BUTTON_START_TOP_LEFT_Y;
                    GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_BUTTON_START_DOWN_RIGHT_X;
                    GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_BUTTON_START_TOP_LEFT_Y;
                } else if(guiElementsStorage[gElmt].position_x == GUI_STATE_BUTTON1_MESS) {
                    GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_BUTTON_MESS_TOP_LEFT_X;
                    GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_BUTTON_MESS_DOWN_RIGHT_Y;
                    GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_BUTTON_MESS_DOWN_RIGHT_X;
                    GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_BUTTON_MESS_DOWN_RIGHT_Y;
                    GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_BUTTON_MESS_TOP_LEFT_X;
                    GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_BUTTON_MESS_TOP_LEFT_Y;
                    GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_BUTTON_MESS_DOWN_RIGHT_X;
                    GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_BUTTON_MESS_TOP_LEFT_Y;
                }
            }else if(guiElementsStorage[gElmt].GUI_TYPE == GUI_TYPE_BUTTON_POTENTIAL) {
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_BUTTON_POTENTIAL_TOP_LEFT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_BUTTON_POTENTIAL_DOWN_RIGHT_Y;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_BUTTON_POTENTIAL_DOWN_RIGHT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_BUTTON_POTENTIAL_DOWN_RIGHT_Y;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_BUTTON_POTENTIAL_TOP_LEFT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_BUTTON_POTENTIAL_TOP_LEFT_Y;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_BUTTON_POTENTIAL_DOWN_RIGHT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_BUTTON_POTENTIAL_TOP_LEFT_Y;
            }
             else if(guiElementsStorage[gElmt].GUI_TYPE == GUI_TYPE_JOYSTICK_MOVEMENT) {
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_JOYSTICK_MOVEMENT_TOP_LEFT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_JOYSTICK_MOVEMENT_DOWN_RIGHT_Y;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_JOYSTICK_MOVEMENT_DOWN_RIGHT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_JOYSTICK_MOVEMENT_DOWN_RIGHT_Y;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_JOYSTICK_MOVEMENT_TOP_LEFT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_JOYSTICK_MOVEMENT_TOP_LEFT_Y;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_JOYSTICK_MOVEMENT_DOWN_RIGHT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_JOYSTICK_MOVEMENT_TOP_LEFT_Y;
            } else if(guiElementsStorage[gElmt].GUI_TYPE == GUI_TYPE_JOYSTICK_ROTATION) {
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_JOYSTICK_ROTATION_TOP_LEFT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_JOYSTICK_ROTATION_DOWN_RIGHT_Y;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_JOYSTICK_ROTATION_DOWN_RIGHT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_JOYSTICK_ROTATION_DOWN_RIGHT_Y;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_JOYSTICK_ROTATION_TOP_LEFT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_JOYSTICK_ROTATION_TOP_LEFT_Y;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_JOYSTICK_ROTATION_DOWN_RIGHT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_JOYSTICK_ROTATION_TOP_LEFT_Y;
            } else if(guiElementsStorage[gElmt].GUI_TYPE == GUI_TYPE_JOYSTICK_WAVE_MOVE) {
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_JOYSTICK_WAVE_MOVE_TOP_LEFT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_JOYSTICK_WAVE_MOVE_DOWN_RIGHT_Y;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_JOYSTICK_WAVE_MOVE_DOWN_RIGHT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_JOYSTICK_WAVE_MOVE_DOWN_RIGHT_Y;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_JOYSTICK_WAVE_MOVE_TOP_LEFT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_JOYSTICK_WAVE_MOVE_TOP_LEFT_Y;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_JOYSTICK_WAVE_MOVE_DOWN_RIGHT_X;
                GUI_positions_and_uv[offsetInGuiPaUV++] = UV_GUI_JOYSTICK_WAVE_MOVE_TOP_LEFT_Y;
            }
        }
        {
            //Generate Indices for quads
            unsigned int index = 0;
            int positionInIbo = 0;
            while(positionInIbo < (numberOfQuads * 6)) {
                GUI_indices[positionInIbo++] = index++; //0
                GUI_indices[positionInIbo++] = index++; //1
                GUI_indices[positionInIbo++] = index; //2
                GUI_indices[positionInIbo++] = index--; //2
                GUI_indices[positionInIbo++] = index; //1
                index += 2;
                GUI_indices[positionInIbo++] = index++; //3
            }
        }
        glBindBuffer(GL_ARRAY_BUFFER, vboVertexID);
        glBufferData(GL_ARRAY_BUFFER, 16 * numberOfQuads * sizeof(float), GUI_positions_and_uv, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndicesID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * numberOfQuads * sizeof(GLuint), GUI_indices, GL_DYNAMIC_DRAW);
        //update vao
        glBindVertexArray(vaoID);//in order to bind vertexAttribPointers to this object
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(sizeof(float) * 8 * numberOfQuads));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);
        free(GUI_positions_and_uv);
        free(GUI_indices);
    }
}

float update_delta_time() {             //Get the current time with glfwGetTime and subtract last time to return deltatime
    static double last_glfw_time = 0.0f;
    static double current_glfw_time;
    current_glfw_time = glfwGetTime();
    float delta = (float)(current_glfw_time - last_glfw_time);
    last_glfw_time = current_glfw_time;
    return delta;
}

float timerForBlink(int restart) {             //Get the current time with glfwGetTime and subtract last time to return deltatime
    static double last_glfw_time = 0.0f;
    static double current_glfw_time;
    current_glfw_time = glfwGetTime();
    float delta = (float)(current_glfw_time - last_glfw_time);
    if(restart){
        if(BlinkStep){
            ColorIntensity=2.9f;
            BlinkStep=0;
        }
        last_glfw_time = current_glfw_time;
    }
    return delta;
}

//VWX
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if(key == GLFW_KEY_W) {
        if(rotation_up_down < (3.0)) {
            rotation_up_down = rotation_up_down + delta_time;
        }
    }
    if(key == GLFW_KEY_S) {
        if(rotation_up_down > (-0.0)) {
            rotation_up_down = rotation_up_down - delta_time;
        }
        if(rotation_up_down < (-0.0)) {
            rotation_up_down = 0.0;
        }
    }
    //atan(rotation_up_down);
    if(key == GLFW_KEY_D) {
        if(rotation_left_right > (-M_PI)) {
            rotation_left_right = rotation_left_right - delta_time;
        } else {
            rotation_left_right = M_PI;
        }
    }
    if(key == GLFW_KEY_A) {
        if(rotation_left_right < M_PI) {
            rotation_left_right = rotation_left_right + delta_time;
        } else {
            rotation_left_right = -M_PI;
        }
    }
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(MainWindow, 1);
    }
    if(key == GLFW_KEY_M && action == GLFW_PRESS) {
        if(42!=simulation_measurement(glfwGetTime())){
            guiElementsStorage[2].position_x = GUI_STATE_BUTTON1_RESET;
        }
        //TODO redraw gui
    }

    if(key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        //TODO set simulation running
        guiElementsStorage[2].position_x = GUI_STATE_BUTTON1_MESS;
    }
    if(key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
        wave_offset_x+= Offset_change;
        standard_draw();
    }
    if(key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
        wave_offset_x-=Offset_change;
        standard_draw();
    }
    if(key == GLFW_KEY_UP && action == GLFW_PRESS) {
        wave_offset_y += Offset_change;
        standard_draw();
    }
    if(key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
        wave_offset_y -=Offset_change;
        standard_draw();
    }
    if(key == GLFW_KEY_R && action == GLFW_PRESS) {
            //Reset
            //TODO redraw wave
        /*wave_offset_x = offset_x_start;
        wave_offset_y = offset_y_start;
        guiElementsStorage[GUI_SLIDER_SIZE].position_x = SLIDER_SIZE_START;
        guiElementsStorage[GUI_SLIDER_SPEED].position_x = SLIDER_SPEED_START;
        dt = guiElementsStorage[GUI_SLIDER_SPEED].position_x * SPEED_MULTI;
        momentum_prop = 1;
        draw_new_wave = 1;*/
    }
    if(key == GLFW_KEY_O && action == GLFW_PRESS) {
        if(guiElementsStorage[GUI_SLIDER_SIZE].position_x > Diameter_change) {
            guiElementsStorage[GUI_SLIDER_SIZE].position_x = guiElementsStorage[GUI_SLIDER_SIZE].position_x - Diameter_change;
            standard_draw();
        }
    }
    if(key == GLFW_KEY_P && action == GLFW_PRESS) {
        if(guiElementsStorage[GUI_SLIDER_SIZE].position_x < 1.0f - Diameter_change) {
            guiElementsStorage[GUI_SLIDER_SIZE].position_x = guiElementsStorage[GUI_SLIDER_SIZE].position_x + Diameter_change;
            //TODO redraw wave
        }
    }
    /*if((simulation_state == simulation_state_wait_for_restart || simulation_state == simulation_state_measurement_animation)&&(!AnimationStep)) {
        if(key == GLFW_KEY_N && action == GLFW_PRESS) {
            ColorIntensity=2.99f;
            draw_new_wave = 1;
            simulation_state = simulation_state_create_and_wait_for_start;
            guiElementsStorage[2].position_x = GUI_STATE_BUTTON1_START;
        }
    }*/
    if(key == GLFW_KEY_X && action == GLFW_PRESS) {
        if(guiElementsStorage[GUI_SLIDER_SPEED].position_x < 1.0f - Speed_change) {
            guiElementsStorage[GUI_SLIDER_SPEED].position_x += Speed_change;
            dt = mapValue(type_wave_dt,guiElementsStorage[GUI_SLIDER_SPEED].position_x);
        }
    }
    if(key == GLFW_KEY_Y && action == GLFW_PRESS) {
        if(guiElementsStorage[GUI_SLIDER_SPEED].position_x > Speed_change) {
            guiElementsStorage[GUI_SLIDER_SPEED].position_x -= Speed_change;
            dt = mapValue(type_wave_dt,guiElementsStorage[GUI_SLIDER_SPEED].position_x);
        }
    }
    if(key == GLFW_KEY_Q && action == GLFW_PRESS) {
        if(disable_autocenter == 0){
            disable_autocenter = 1;
        }else if(disable_autocenter == 1){
            disable_autocenter = 0;
        }
    }
    if(key == GLFW_KEY_PERIOD && action == GLFW_PRESS) {
        if(jerk_for_autocenter<1.0f){
            jerk_for_autocenter=jerk_for_autocenter+0.1f;
        }
    }
    if(key == GLFW_KEY_MINUS && action == GLFW_PRESS) {
        if(jerk_for_autocenter>0.2f){
            jerk_for_autocenter=jerk_for_autocenter-0.1f;
        }
    }
    refereshGUI();
}

//XYZ
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    double xpos = 0;
    double ypos = 0;
    int width = 0;
    int height = 0;
    glfwGetCursorPos(MainWindow, &xpos, &ypos);
    glfwGetWindowSize(MainWindow, &width, &height);
    xpos /= width;
    ypos /= width;
    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        for(int gElmt = 0; gElmt < numberOfGuiElements; gElmt++) {
            if(guiElementsStorage[gElmt].GUI_TYPE <= SLIDER_X_NUMBER && guiElementsStorage[gElmt].GUI_TYPE > 0) {
                float x_offset = (guiElementsStorage[gElmt].top_left_x + guiElementsStorage[gElmt].percentOfWidth * ((36.0f) + guiElementsStorage[gElmt].position_x * 440.0f) / 512.0f) - xpos;
                float y_offset = (guiElementsStorage[gElmt].top_left_y + guiElementsStorage[gElmt].percentOfWidth * (32.0f / 512.0f)) - ypos;
                if((x_offset * x_offset + y_offset * y_offset) < ((guiElementsStorage[gElmt].percentOfWidth * 32.0f / 512.0f) * (guiElementsStorage[gElmt].percentOfWidth * 32.0f / 512.0f))) {
                    selectedGuiElement = gElmt;
                    printf("Info: Grabbed on Slider x %d\n", selectedGuiElement);
                    return;
                }
            }else if(guiElementsStorage[gElmt].GUI_TYPE <= (SLIDER_X_NUMBER+SLIDER_Y_NUMBER) && guiElementsStorage[gElmt].GUI_TYPE > SLIDER_X_NUMBER) {
                float x_offset = (guiElementsStorage[gElmt].top_left_x + guiElementsStorage[gElmt].percentOfWidth * (32.0f / 512.0f)) - xpos;
                float y_offset = (guiElementsStorage[gElmt].top_left_y + guiElementsStorage[gElmt].percentOfWidth * ((36.0f) + guiElementsStorage[gElmt].position_x * 440.0f) / 512.0f) - ypos;
                if((x_offset * x_offset + y_offset * y_offset) < ((guiElementsStorage[gElmt].percentOfWidth * 32.0f / 512.0f) * (guiElementsStorage[gElmt].percentOfWidth * 32.0f / 512.0f))) {
                    selectedGuiElement = gElmt;
                    printf("Info: Grabbed on Slider y %d\n", selectedGuiElement);
                    return;
                }
            }else if(guiElementsStorage[gElmt].GUI_TYPE <= (SLIDER_X_NUMBER+SLIDER_Y_NUMBER+JOYSTICK_NUMBER) && guiElementsStorage[gElmt].GUI_TYPE > (SLIDER_X_NUMBER+SLIDER_Y_NUMBER)) {
                float x_offset = guiElementsStorage[gElmt].top_left_x + guiElementsStorage[gElmt].percentOfWidth * (((1 - GUI_JOYSTICK_PROPERTY_SCALE) * 0.5f * guiElementsStorage[gElmt].position_x) + 0.5f) - xpos; //Calculates the centere of the joystic and then calculates delta to pressed position
                float y_offset = guiElementsStorage[gElmt].top_left_y + guiElementsStorage[gElmt].percentOfWidth * (((1 - GUI_JOYSTICK_PROPERTY_SCALE) * 0.5f * guiElementsStorage[gElmt].position_y) + 0.5f) - ypos;
                if((x_offset * x_offset + y_offset * y_offset) < (0.25f * guiElementsStorage[gElmt].percentOfWidth * GUI_JOYSTICK_PROPERTY_SCALE * guiElementsStorage[gElmt].percentOfWidth * GUI_JOYSTICK_PROPERTY_SCALE)) { //if inside the tappable circle 0.25f because it is GUI_JOYSTICK_PROPERT_SCALE is the diameter->*0.5f = radius
                    selectedGuiElement = gElmt;
                    return;
                }
            }else if(guiElementsStorage[gElmt].GUI_TYPE <= (SLIDER_X_NUMBER+SLIDER_Y_NUMBER+JOYSTICK_NUMBER+BUTTON_NUMBER) && guiElementsStorage[gElmt].GUI_TYPE > (SLIDER_X_NUMBER+SLIDER_Y_NUMBER+JOYSTICK_NUMBER) && (guiElementsStorage[gElmt].top_left_x < xpos && ((guiElementsStorage[gElmt].top_left_x + guiElementsStorage[gElmt].percentOfWidth) > xpos)) && (guiElementsStorage[gElmt].top_left_y < ypos && ((guiElementsStorage[gElmt].top_left_y + 0.25f * guiElementsStorage[gElmt].percentOfWidth) > ypos))) {
                printf("BUTTON PUSHED\n\n\n");
                if(guiElementsStorage[gElmt].GUI_TYPE == GUI_TYPE_BUTTON_CONTROL){
                    if(guiElementsStorage[gElmt].position_x == GUI_STATE_BUTTON1_RESET) {
                        standard_draw();
                        printf("TODO BLINK\n");
                        guiElementsStorage[gElmt].position_x = GUI_STATE_BUTTON1_START;
                        drawGui(G_OBJECT_UPDATE, width / (float)height);
                        return;
                    }else if(guiElementsStorage[gElmt].position_x == GUI_STATE_BUTTON1_START) {
                        simulation_unpause();
                        guiElementsStorage[gElmt].position_x = GUI_STATE_BUTTON1_MESS;
                        drawGui(G_OBJECT_UPDATE, width / (float)height);
                        return;
                    }else if(guiElementsStorage[gElmt].position_x == GUI_STATE_BUTTON1_MESS) {
                        if(42!=simulation_measurement(glfwGetTime())){
                            guiElementsStorage[gElmt].position_x = GUI_STATE_BUTTON1_RESET;
                            drawGui(G_OBJECT_UPDATE, width / (float)height);
                        }
                        return;
                    }
                }else if(guiElementsStorage[gElmt].GUI_TYPE == GUI_TYPE_BUTTON_POTENTIAL){
                    printf("Debug: Should change potential\n");
                    update_potential();
                }
            }
        }
        selectedGuiElement = -1;
    }
    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        if(guiElementsStorage[selectedGuiElement].GUI_TYPE <= (SLIDER_X_NUMBER+SLIDER_Y_NUMBER+JOYSTICK_NUMBER) && guiElementsStorage[selectedGuiElement].GUI_TYPE > (SLIDER_X_NUMBER+SLIDER_Y_NUMBER)) { //recenter joystick after released
            joystick_reset(selectedGuiElement);
            drawGui(G_OBJECT_UPDATE, width / (float)height);
        }
        selectedGuiElement = -1; //Deselect all gui elements (tracking for the dragged object like slider or joystick)
    }
}

void mouse_scroll_callback(GLFWwindow* window, double xOffset, double yOffset) {
    float temp_mouse_scroll = -0.02f * (float)yOffset;
    if((FOV + temp_mouse_scroll < 1.0f) && (FOV + temp_mouse_scroll > 0.1f)) {
        FOV += temp_mouse_scroll;
    }else {
    }
}

void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
    timerForBlink(1);
    if(selectedGuiElement == -1) { //directly leave this if there is no gui object to monitor (no dragged slider / joystick)
        return;
    }
    int width = 0;
    int height = 0;
    glfwGetWindowSize(MainWindow, &width, &height);
    xpos /= width; //xPos now aligned to local gui coordinates ranging from 0 - 1
    ypos /= width; //ypos now aligned to local gui coordinates ranging from 0 - aspect ration 9/16 eg
    if(guiElementsStorage[selectedGuiElement].GUI_TYPE <= SLIDER_X_NUMBER && guiElementsStorage[selectedGuiElement].GUI_TYPE > 0) {
        set_x_position_slider(selectedGuiElement, xpos);
    }else if(guiElementsStorage[selectedGuiElement].GUI_TYPE <= (SLIDER_X_NUMBER+SLIDER_Y_NUMBER) && guiElementsStorage[selectedGuiElement].GUI_TYPE > SLIDER_X_NUMBER) {
        set_y_position_slider(selectedGuiElement, ypos);
    }else if(guiElementsStorage[selectedGuiElement].GUI_TYPE <= (SLIDER_X_NUMBER+SLIDER_Y_NUMBER+JOYSTICK_NUMBER) && guiElementsStorage[selectedGuiElement].GUI_TYPE > (SLIDER_X_NUMBER+SLIDER_Y_NUMBER)) {
        set_xy_position_joystick(selectedGuiElement, xpos, ypos);
    }
    switch(selectedGuiElement) {
    case GUI_SLIDER_SIZE:
        wave_gauss_width=mapValue(type_wave_gauss_width,guiElementsStorage[selectedGuiElement].position_x);
        standard_draw();
        break;
    case GUI_SLIDER_WAVE_ROTATION:
        wave_angle=mapValue(type_wave_angle,guiElementsStorage[selectedGuiElement].position_x);
        standard_draw();
        break;
    case GUI_SLIDER_SPEED:
        wave_momentum=mapValue(type_wave_momentum,guiElementsStorage[selectedGuiElement].position_x);
        standard_draw();
        break;
    /*case GUI_JOYSTICK_ROTATION:
        //rotate_camera(selectedGuiElement); Commented because the rotation would be faster if the gui element is moved slightly
        break;
    case GUI_JOYSTICK_MOVEMENT:
        move_camera(selectedGuiElement);
        break;
    case GUI_JOYSTICK_WAVE_MOVE:
        move_wave(selectedGuiElement);
        break;*/
    default:
        printf("Error: cursor pos callback guiElementsStorage[selectedGuiElement].position_x seems to have logged unregistered GUI Element!\n");
        break;
    }
    drawGui(G_OBJECT_UPDATE, width / (float)height);
}

void JoystickControll(){   //Benötigt, da sonst keine Rotation bei hover on joystick
    double xpos=0.0;
    double ypos=0.0;
    int width=0;
    glfwGetCursorPos(MainWindow,&xpos,&ypos);
    glfwGetWindowSize(MainWindow,&width,NULL);
    xpos/=width;
    ypos/=width;
    if(guiElementsStorage[selectedGuiElement].GUI_TYPE <= (SLIDER_X_NUMBER+SLIDER_Y_NUMBER+JOYSTICK_NUMBER) && guiElementsStorage[selectedGuiElement].GUI_TYPE > (SLIDER_X_NUMBER+SLIDER_Y_NUMBER)) {
        set_xy_position_joystick(selectedGuiElement, xpos, ypos);
    }
    switch(selectedGuiElement){
    case GUI_JOYSTICK_ROTATION:
        rotate_camera(selectedGuiElement);
        break;
    case GUI_JOYSTICK_MOVEMENT:
        //move_camera(selectedGuiElement);
        break;
    case GUI_JOYSTICK_WAVE_MOVE:
        //move_wave(selectedGuiElement);
        break;
    default:
        //printf("No Joystick there\n");
        return;
    }
}

//Needed functions for mouse and joystick control

void set_x_position_slider(int selectedGuiElement, double xpos){
    guiElementsStorage[selectedGuiElement].position_x = ((((xpos - guiElementsStorage[selectedGuiElement].top_left_x) / guiElementsStorage[selectedGuiElement].percentOfWidth) * 512.0f) - 36.0f) / 440.0f;
    if(guiElementsStorage[selectedGuiElement].position_x < 0.0f) {
        guiElementsStorage[selectedGuiElement].position_x = 0.0f;
    } else if(guiElementsStorage[selectedGuiElement].position_x > 1.0f) {
        guiElementsStorage[selectedGuiElement].position_x = 1.0f;
    }
}

void set_y_position_slider(int selectedGuiElement, double ypos){
    guiElementsStorage[selectedGuiElement].position_y = ((((ypos - guiElementsStorage[selectedGuiElement].top_left_y) / guiElementsStorage[selectedGuiElement].percentOfWidth) * 512.0f) - 36.0f) / 440.0f;
    if(guiElementsStorage[selectedGuiElement].position_y < 0.0f) {
        guiElementsStorage[selectedGuiElement].position_y = 0.0f;
    } else if(guiElementsStorage[selectedGuiElement].position_y > 1.0f) {
        guiElementsStorage[selectedGuiElement].position_y = 1.0f;
    }
}

void set_xy_position_joystick(int selectedGuiElement, double xpos, double ypos){
    guiElementsStorage[selectedGuiElement].position_x = ((xpos - (guiElementsStorage[selectedGuiElement].top_left_x + 0.5f * guiElementsStorage[selectedGuiElement].percentOfWidth)) / guiElementsStorage[selectedGuiElement].percentOfWidth) * (2.0f / (1 - GUI_JOYSTICK_PROPERTY_SCALE));
    guiElementsStorage[selectedGuiElement].position_y = ((ypos - (guiElementsStorage[selectedGuiElement].top_left_y + 0.5f * guiElementsStorage[selectedGuiElement].percentOfWidth)) / guiElementsStorage[selectedGuiElement].percentOfWidth) * (2.0f / (1 - GUI_JOYSTICK_PROPERTY_SCALE));
    if(guiElementsStorage[selectedGuiElement].position_x < (-1.0f)) {
        guiElementsStorage[selectedGuiElement].position_x = (-1.0f);
    } else if(guiElementsStorage[selectedGuiElement].position_x > 1.0f) {
        guiElementsStorage[selectedGuiElement].position_x = 1.0f;
    }
    if(guiElementsStorage[selectedGuiElement].position_y < (-1.0f)) {
        guiElementsStorage[selectedGuiElement].position_y = (-1.0f);
    } else if(guiElementsStorage[selectedGuiElement].position_y > 1.0f) {
        guiElementsStorage[selectedGuiElement].position_y = 1.0f;
    }
}

void rotate_camera(int selectedGuiElement){
    rotation_up_down -= delta_time * guiElementsStorage[selectedGuiElement].position_y;
    rotation_left_right -= delta_time * guiElementsStorage[selectedGuiElement].position_x;
    printf("Rotup: %f and %f\n",delta_time * guiElementsStorage[selectedGuiElement].position_y,delta_time * guiElementsStorage[selectedGuiElement].position_x);
    if(rotation_up_down > 3.0f) {
        rotation_up_down = 3.0f;
    } else if(rotation_up_down < 0.0f) {
        rotation_up_down = 0.0f;
    }
    if(rotation_left_right < (-M_PI)) {
        rotation_left_right = M_PI;
    } else if(rotation_left_right > M_PI) {
        rotation_left_right = (-M_PI);
    }
}

/*void move_camera(int selectedGuiElement){
    position_x_axis += MovementBorderCamera*delta_time*(guiElementsStorage[selectedGuiElement].position_x*-cos(rotation_left_right)+guiElementsStorage[selectedGuiElement].position_y*sin(rotation_left_right));
    position_y_axis += MovementBorderCamera*delta_time*(guiElementsStorage[selectedGuiElement].position_x*sin(rotation_left_right)+guiElementsStorage[selectedGuiElement].position_y*cos(rotation_left_right));
    if(position_x_axis>MovementBorderCamera){
        position_x_axis=MovementBorderCamera;
    }else if(position_x_axis<(-MovementBorderCamera)){
        position_x_axis=(-MovementBorderCamera);
    }
    if(position_y_axis>MovementBorderCamera){
        position_y_axis=MovementBorderCamera;
    }else if(position_y_axis<-MovementBorderCamera){
        position_y_axis=-MovementBorderCamera;
    }
}

void draw_wave(int selectedGuiElement){
    wave_offset_x += (sim_res_x*0.25f*delta_time*(guiElementsStorage[selectedGuiElement].position_x*-cos(rotation_left_right)+guiElementsStorage[selectedGuiElement].position_y*sin(rotation_left_right)));
    wave_offset_y += (sim_res_y*0.25f*delta_time*(guiElementsStorage[selectedGuiElement].position_x*sin(rotation_left_right)+guiElementsStorage[selectedGuiElement].position_y*cos(rotation_left_right)));
    if(wave_offset_x>(sim_res_x*0.9f)){
        wave_offset_x=sim_res_x*0.9f;
    }else if(wave_offset_x<(sim_res_x*0.1f)){
        wave_offset_x=(sim_res_x*0.1f);
    }
    if(wave_offset_y>sim_res_y*0.15f){
        wave_offset_y=sim_res_y*0.15f;
    }else if(wave_offset_y<(sim_res_y*0.05f)){
        wave_offset_y=sim_res_y*0.05f;
    }
    simulation_redraw_wave(wave_offset_x,wave_offset_y,wave_angle,wave_momentum,wave_gauss_width);
}*/

void joystick_reset(int selectedGuiElement){
    guiElementsStorage[selectedGuiElement].position_x = 0.0f;
    guiElementsStorage[selectedGuiElement].position_y = 0.0f;
    printf("Debug: Reset Pos of Joystick %d\n", selectedGuiElement);
}

void change_speed(){
    dt = mapValue(type_wave_dt,guiElementsStorage[selectedGuiElement].position_x);
}

//Shader

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
        printf("Error: Compilation of shader %d from %s failed!\n", ShaderId, FilePath);
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

void drop_file_callback(GLFWwindow* window, int count, const char** paths) {
    for(int i = 0; i < count; i++) {
        printf("Dropped File Path: %s\n", paths[i]);
    }
}


void update_potential(unsigned char* graphic_local_texture){
    static unsigned int SelectedPotential=0;
    char PotentialSourceFile[256];
    PotentialSourceFile[0]=0;
    strcat(PotentialSourceFile,filepath_potentials);
    //StringCbCat(PotentialSourceFile,256,filepath_potentials);
    #ifdef _WIN32
    strcat(PotentialSourceFile,"\\");
    //StringCbCat(PotentialSourceFile,256,"\\");
    #elif __linux__
    strcat(PotentialSourceFile,"/");
    #endif
    strcat(PotentialSourceFile,PotentialFilesList[SelectedPotential]);
    //StringCbCat(PotentialSourceFile,256,PotentialFilesList[SelectedPotential]);
    printf("Loading: %s",PotentialSourceFile);

    /*if(PotentialSourceFile[0]==1){
        pot = read_bmp(PotentialSourceFile+1);
    }else{
        pot = read_bmp(PotentialSourceFile);
    }*/
    pot=read_bmp(PotentialSourceFile);
    simulation_load_potential(pot);
    if((++SelectedPotential)==CountOfPotentialFiles){
        SelectedPotential=0;
    }
    //TODO update buttons
    guiElementsStorage[GUI_BUTTON_CONTROL].position_x=GUI_STATE_BUTTON1_START;
    refereshGUI();
    standard_draw();
    ColorIntensity=2.9f;
}

void standard_draw(){
    simulation_redraw_wave((int)wave_offset_x,(int)wave_offset_y,wave_angle,wave_momentum,wave_gauss_width);
}

//window size
void windows_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    //Refresh lower GUI Border
    guiElementsStorage[GUI_BUTTON_CONTROL].top_left_y = (height / (float)width) - guiElementsStorage[GUI_BUTTON_CONTROL].percentOfWidth * 0.25f;
    guiElementsStorage[GUI_BUTTON_POTENTIAL].top_left_y = ((height / (float)width) - guiElementsStorage[GUI_BUTTON_POTENTIAL].percentOfWidth * 0.25f) -0.06f; //-0.1f because button should be on top of other button
    guiElementsStorage[GUI_JOYSTICK_MOVEMENT].top_left_y = (height / (float)width) - guiElementsStorage[GUI_JOYSTICK_MOVEMENT].percentOfWidth;
    guiElementsStorage[GUI_JOYSTICK_ROTATION].top_left_y = (height / (float)width) - guiElementsStorage[GUI_JOYSTICK_ROTATION].percentOfWidth;
    guiElementsStorage[GUI_JOYSTICK_WAVE_MOVE].top_left_y = (height / (float)width) - guiElementsStorage[GUI_JOYSTICK_WAVE_MOVE].percentOfWidth;
    drawGui(G_OBJECT_UPDATE, width / (float)height);
}

float mapValue(int type,float input){
    float output;
    switch(type){
        case type_wave_momentum:
            output=input+1.0f;
            break;
        case type_wave_gauss_width:
            output=SIZE_MULTI*input+2.0f;
            break;
        case type_wave_dt:
            output=SPEED_MULTI*input+0.00001f;
            break;
        case type_wave_angle:
            output=-M_PI*2*input-M_PI/2;
            break;
        default:
            printf("ERROR: NO SUCH TYPE!!");
            output=-1.0f;
            break;
    }
    return output;


}

//GUI
void refereshGUI(){
    int width=0;
    int height=0;
    glfwGetWindowSize(MainWindow,&width,&height);
    windows_size_callback(MainWindow,width,height);
}
