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

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow* window, int button,int action, int mods);
void drop_file_callback(GLFWwindow* window, int count, const char** paths);
void mouse_scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void* createPlaneVBO(unsigned int PlaneResolution, unsigned int GridResolution);
void createCube();
unsigned char* read_bmp(char* filepath);
void write_bmp(char* filepath, unsigned int width, unsigned int height);
float update_delta_time();
void APIENTRY openglCallbackFunction(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar* message,const void* userParam);
void glfw_error_callback(int error, const char* description);
GLuint CompileShaderFromFile(char FilePath[],GLuint shaderType);
//global variables section
float FOV=0.7f;
#define Resolution 512
#define PlaneRes 4096       //must be power of 2
#define GridRes 256        //must be power of 2
GLFWwindow* MainWindow;
#define ButtonStart

struct GUI {
    int Left_up_x;
    int Left_up_y;
    int Width;
    int Height;
    int Position;
    int Render_pos;
    int Push;
};

int number_Buttons;

int main(int argc, char* argv[]) {
    //GLFW init
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        return -1;
    }

    //Set window creation hints
    //glfwWindowHint(GLFW_RESIZABLE,GL_FALSE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT,GL_TRUE);

    //window creation
    //MainWindow = glfwCreateWindow(1900, 1000, "Quantum Minigolf 2.0", NULL, NULL);
    MainWindow = glfwCreateWindow(1920, 1080, "Quantum Minigolf 2.0", glfwGetPrimaryMonitor(), NULL);
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
    printf("QuantumMinigolf v2 opengl:\n");
    printf("using OpenGl Version: %s\n",glGetString(GL_VERSION));
    //enable v-sync
    glfwSwapInterval(1);
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
    //Get window height
    int window_width = 0;
    int window_height = 0;
    glfwGetWindowSize(MainWindow,&window_width,&window_height);
    int maxIndices,maxVertices,maxTexSize,maxTexBufferSize;
    glGetIntegerv(GL_MAX_ELEMENTS_INDICES,&maxIndices);
    glGetIntegerv(GL_MAX_ELEMENTS_VERTICES,&maxVertices);
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
    glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, &maxTexBufferSize);
    printf("Ind:%d\nVert:%d\nTex:%d\nBuf:%d\n",maxIndices,maxVertices,maxTexSize,maxTexBufferSize);

    //Initialize shaders
    //TODO filepath for windows, alter for unix like os
    //GLuint computeShaderId = ComlongpileShaderFromFile(".\\res\\shaders\\compute.glsl",GL_COMPUTE_SHADER);

    //Compile graph 3D shader
    GLuint vertexShaderId = CompileShaderFromFile(".\\res\\shaders\\vertex_graph.glsl",GL_VERTEX_SHADER);
    GLuint fragmentShaderId = CompileShaderFromFile(".\\res\\shaders\\fragment_graph.glsl",GL_FRAGMENT_SHADER);
    GLuint graphShaderID = glCreateProgram();              //create program to run on GPU
    glAttachShader(graphShaderID, vertexShaderId);         //attach vertex shader to new program
    glAttachShader(graphShaderID, fragmentShaderId);       //attach fragment shader to new program
    glLinkProgram(graphShaderID);
    //Get Shader Variables
    glUseProgram(graphShaderID);
    GLint MVPmatrix=glGetUniformLocation(graphShaderID,"MVPmatrix");//only callable after glUseProgramm has been called once
    GLint potential_true=glGetUniformLocation(graphShaderID,"potential_true");


    /*Compile Gui Shader
    vertexShaderId = CompileShaderFromFile(".\\res\\shaders\\vertex_gui.glsl",GL_VERTEX_SHADER);
    fragmentShaderId = CompileShaderFromFile(".\\res\\shaders\\fragment_gui.glsl",GL_FRAGMENT_SHADER);
    GLuint guiShaderID = glCreateProgram();
    glAttachShader(guiShaderID, vertexShaderId);         //attach vertex shader to new program
    glAttachShader(guiShaderID, fragmentShaderId);       //attach fragment shader to new program
    glLinkProgram(guiShaderID);
    */
    //GLuint* indexlist=createPlaneVBO(256,32);
    void* index_buffer_array=createPlaneVBO(PlaneRes,GridRes);
    printf("index_buffer_array%d,%d\n",*((unsigned int*)index_buffer_array),*((unsigned int*)index_buffer_array+1));
    glDisable(GL_CULL_FACE);
    glClearColor(0.05f,0.05f,0.1f,0.5f);//Set background color
    //Enable z checking
    glEnable(GL_DEPTH_TEST);
    //Texture test code
    glActiveTexture(GL_TEXTURE0);
    GLuint testTexture=0;
    glGenTextures(1,&testTexture);
    glBindTexture(GL_TEXTURE_2D,testTexture);
    //unsigned char* TextureImageTest=read_bmp(".\\double_slit.bmp");
    glUniform1i(glGetUniformLocation(graphShaderID,"texture0"),0);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //float tempBorderColor[]={0.5f,0.5f,0.0f,0.0f};
    //glTexParameterfv(GL_TEXTURE_2D,GL_TEXTURE_BORDER_COLOR, tempBorderColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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

    double rotation_up_down=PI;//PI/4.0f;
    double rotation_left_right=PI;
    mat4x4 mvp4x4;
    mat4x4 persp4x4;
    vec3 eye_vec= {1.0f,1.0f,1.0f};
    vec3 cent_vec= {0.0f,0.0f,0.0f};
    vec3 up_vec= {0.0f,0.0f,1.0f};

    int width = Resolution;
    int height = Resolution;
    fftw_complex *psi;
    fftw_complex *prop;
    psi = (fftw_complex*) fftw_alloc_complex(width*height);
    prop = (fftw_complex*) fftw_alloc_complex(width*height);
    fftw_plan fft = fftw_plan_dft_2d (width, height, psi, psi, FFTW_FORWARD, FFTW_MEASURE);

    fftw_plan ifft = fftw_plan_dft_2d (width, height, psi, psi, FFTW_BACKWARD, FFTW_MEASURE);
    //GUI
    struct GUI Button_new;
    struct GUI Button_measure;
    struct GUI Button_esc;
    struct GUI Slider_size;

    Button_new.Left_up_x=0;
    Button_new.Left_up_y=100;
    Button_new.Width=200;
    Button_new.Height=100;
    Button_new.Push=0;

    Button_measure.Left_up_x=0;
    Button_measure.Left_up_y=200;
    Button_measure.Width=200;
    Button_measure.Height=100;
    Button_measure.Push=0;

    Button_esc.Left_up_x=1720;
    Button_esc.Left_up_y=0;
    Button_esc.Width=200;
    Button_esc.Height=100;

    Slider_size.Left_up_x=0;
    Slider_size.Left_up_y=0;
    Slider_size.Width=200;
    Slider_size.Height=100;
    Slider_size.Position=50;
    Slider_size.Render_pos=Slider_size.Left_up_y+Slider_size.Height/2;

    //Wave parameter initialisation
    double wavesize_1=Slider_size.Position*2.5f;
    double wavesize_2=500;
    double norm = width*height;
    float angle_mov_1 = PI/2.0f;
    float angle_mov_2 = PI/2.0f;

    int offset_x_1 = 256;
    int offset_y_1 = 40;
    int offset_x_2 = 256;
    int offset_y_2 = 300;

    unsigned char* speicher = calloc(width*height*4,1);
    unsigned char* pot=read_bmp(".//double_slit512.bmp");
    unsigned char* GUI_template=read_bmp(".//GUI.bmp");
    double* potential=malloc(width*height*sizeof(double));

    for(int i=0; i<width*height; i++) {
        potential[i]=(255-pot[4*i+1])/100.0f;
    }
    //Momentum Propagator initialisation
    float dt = 0.00005;
    for(int x=0; x<width/2; x++) {
        for(int y=0; y<height/2; y++) {
            prop[x*height+y][0] = cos(dt*(-x*x - y*y));
            prop[x*height+y][1] = sin(dt*(-x*x - y*y));
        }
        for(int y=height/2; y<height; y++) {
            prop[x*height+y][0] = cos(dt*(-x*x - (y-height)*(y-height)));
            prop[x*height+y][1] = sin(dt*(-x*x - (y-height)*(y-height)));
        }
    }
    for(int x=width/2; x<width; x++) {
        for(int y=0; y<height/2; y++) {
            prop[x*height+y][0] = cos(dt*(-(x-width)*(x-width) - y*y));
            prop[x*height+y][1] = sin(dt*(-(x-width)*(x-width) - y*y));
        }
        for(int y=height/2; y<height; y++) {
            prop[x*height+y][0] = cos(dt*(-(x-width)*(x-width) - (y-height)*(y-height)));
            prop[x*height+y][1] = sin(dt*(-(x-width)*(x-width) - (y-height)*(y-height)));
        }
    }

    //Set program-start
    int measurement = 2;
    //Create wave
    for(int j=0; j<height; j++) {
        for(int i=0; i<width; i++) {        /*sin((i+wavesize_1)/10)/2+0.5;*/
            psi[i+j*width][0]=exp(-((i-offset_x_1)*(i-offset_x_1)+(j-offset_y_1)*(j-offset_y_1))/wavesize_1)*cos(((i-height/(float)2)*cos(angle_mov_1)+(j-height/(float)2)*sin(angle_mov_1))*8.0f)
            +exp(-((i-offset_x_2)*(i-offset_x_2)+(j-offset_y_2)*(j-offset_y_2))/wavesize_2)*cos(((i-height/(float)2)*cos(angle_mov_2)+(j-height/(float)2)*sin(angle_mov_2))*8.0f);
            psi[i+j*width][1]=exp(-((i-offset_x_1)*(i-offset_x_1)+(j-offset_y_1)*(j-offset_y_1))/wavesize_1)*sin(((i-height/(float)2)*cos(angle_mov_1)+(j-height/(float)2)*sin(angle_mov_1))*8.0f)
            +exp(-((i-offset_x_2)*(i-offset_x_2)+(j-offset_y_2)*(j-offset_y_2))/wavesize_2)*sin(((i-height/(float)2)*cos(angle_mov_2)+(j-height/(float)2)*sin(angle_mov_2))*8.0f);
        }
    }
    while (!glfwWindowShouldClose(MainWindow)) {
        if(measurement == 0) {

            fftw_execute(fft);

            //momentum space
            for(int i=0; i<width*height; i++) {
                double psi_re_temp = psi[i][0];
                psi[i][0] = psi_re_temp*prop[i][0]-psi[i][1]*prop[i][1];
                psi[i][1] = psi_re_temp*prop[i][1]+psi[i][1]*prop[i][0];
            }

            fftw_execute(ifft);

            for(int i=0; i<width*height; i++) {
                psi[i][0]=psi[i][0]/norm;
                psi[i][1]=psi[i][1]/norm;
            }

            for(int i=0; i<width*height; i++) {
                double psi_re_temp = psi[i][0];
                psi[i][0] = psi_re_temp*cos(potential[i])-psi[i][1]*sin(potential[i]);
                psi[i][1] = psi_re_temp*sin(potential[i])+psi[i][1]*cos(potential[i]);
            }

            for(int i=0; i<width; i++) {
                psi[i][0]=0;
                psi[i][1]=0;
                psi[i+(width-1)*height][0]=0;
                psi[i+(width-1)*height][1]=0;
            }

            for(int i=0; i<height; i++) {
                psi[1+i*width][0]=0;
                psi[1+i*width][1]=0;
                psi[height-1+i*width][0]=0;
                psi[height-1+i*width][1]=0;
            }
        }

        if(measurement==1) {
            srand((long)10000.0f*glfwGetTime());
            double random=(rand()%1001)/1000.0f;
            double sum=0;
            double norm_sum=0;
            for(int i=0; i<width*height; i++) {
                norm_sum=norm_sum+(psi[i][0]*psi[i][0]+psi[i][1]*psi[i][1]);
            }
            double sqrt_norm_sum=sqrt(norm_sum);
            for(int i=0; i<width*height; i++) {
                psi[i][0]=(psi[i][0]/sqrt_norm_sum);
                psi[i][1]=(psi[i][1]/sqrt_norm_sum);
            }
            for(unsigned int i=0; i<width*height; i++) {
                sum=sum+(psi[i][0]*psi[i][0]+psi[i][1]*psi[i][1]);
                if(sum>random) {
                    //printf("sum%f\n",sum);
                    //printf("rand: %f\n",random);
                    for(int i=0; i<width*height; i++) {
                        psi[i][0]=0;
                        psi[i][1]=0;
                    }
                    psi[i][0]=1;
                    if(i/Resolution<Resolution/2+50&&i/Resolution>Resolution/2-50) {
                        if(i%Resolution>250&&i%Resolution<350)
                            psi[i][1]=1;
                    }
                    break;
                }
            }
        }

        if(glfwGetMouseButton(MainWindow, GLFW_MOUSE_BUTTON_LEFT)==GLFW_PRESS) {
            double xpos, ypos;
            glfwGetCursorPos(MainWindow, &xpos, &ypos);
            //printf("%.0f, %.0f\n",xpos, ypos);
            if(xpos>Button_new.Left_up_x&&xpos<Button_new.Left_up_x+Button_new.Width) {
                if(ypos>Button_new.Left_up_y&&ypos<Button_new.Left_up_y+Button_new.Height) {
                    for(int j=0; j<height; j++) {
                        for(int i=0; i<width; i++) {        /*sin((i+wavesize_1)/10)/2+0.5;*/
                            psi[i+j*width][0]=exp(-((i-offset_x_1)*(i-offset_x_1)+(j-offset_y_1)*(j-offset_y_1))/wavesize_1)*cos(((i-height/(float)2)*cos(angle_mov_1)+(j-height/(float)2)*sin(angle_mov_1))*8.0f);
                            //+exp(-((i-offset_x_2)*(i-offset_x_2)+(j-offset_y_2)*(j-offset_y_2))/wavesize_2)*cos(((i-height/(float)2)*cos(angle_mov_2)+(j-height/(float)2)*sin(angle_mov_2))*8.0f);
                            psi[i+j*width][1]=exp(-((i-offset_x_1)*(i-offset_x_1)+(j-offset_y_1)*(j-offset_y_1))/wavesize_1)*sin(((i-height/(float)2)*cos(angle_mov_1)+(j-height/(float)2)*sin(angle_mov_1))*8.0f);
                            //+exp(-((i-offset_x_2)*(i-offset_x_2)+(j-offset_y_2)*(j-offset_y_2))/wavesize_2)*sin(((i-height/(float)2)*cos(angle_mov_2)+(j-height/(float)2)*sin(angle_mov_2))*8.0f);
                        }
                    }
                    measurement=0;
                    Button_new.Push=1;
                    Button_measure.Push=0;
                }
            }
            if(xpos>Button_measure.Left_up_x&&xpos<Button_measure.Left_up_x+Button_measure.Width) {
                if(ypos>Button_measure.Left_up_y&&ypos<Button_measure.Left_up_y+Button_measure.Height) {
                    measurement = 1;
                    Button_measure.Push=1;
                    Button_new.Push=0;
                }
            }
            if(xpos>Button_esc.Left_up_x&&xpos<Button_esc.Left_up_x+Button_esc.Width) {
                if(ypos>Button_esc.Left_up_y&&ypos<Button_esc.Left_up_y+Button_esc.Height) {
                    glfwSetWindowShouldClose(MainWindow,1);
                }
            }
            if(xpos>Slider_size.Left_up_x&&xpos<Slider_size.Left_up_x+Slider_size.Width) {
                if(ypos>Slider_size.Left_up_y&&ypos<Slider_size.Left_up_y+Slider_size.Height) {
                    Slider_size.Position=xpos-Slider_size.Left_up_x;
                    wavesize_1=Slider_size.Position*5.0f;
                    for(int j=0; j<height; j++) {
                        for(int i=0; i<width; i++) {        /*sin((i+wavesize_1)/10)/2+0.5;*/
                            psi[i+j*width][0]=exp(-((i-offset_x_1)*(i-offset_x_1)+(j-offset_y_1)*(j-offset_y_1))/wavesize_1)*cos(((i-height/(float)2)*cos(angle_mov_1)+(j-height/(float)2)*sin(angle_mov_1))*8.0f);
                            //+exp(-((i-offset_x_2)*(i-offset_x_2)+(j-offset_y_2)*(j-offset_y_2))/wavesize_2)*cos(((i-height/(float)2)*cos(angle_mov_2)+(j-height/(float)2)*sin(angle_mov_2))*8.0f);
                            psi[i+j*width][1]=exp(-((i-offset_x_1)*(i-offset_x_1)+(j-offset_y_1)*(j-offset_y_1))/wavesize_1)*sin(((i-height/(float)2)*cos(angle_mov_1)+(j-height/(float)2)*sin(angle_mov_1))*8.0f);
                            //+exp(-((i-offset_x_2)*(i-offset_x_2)+(j-offset_y_2)*(j-offset_y_2))/wavesize_2)*sin(((i-height/(float)2)*cos(angle_mov_2)+(j-height/(float)2)*sin(angle_mov_2))*8.0f);
                        }
                    }
                    measurement = 2;
                    Button_measure.Push=0;
                    Button_new.Push=0;
                }
            }
            else {
                Button_measure.Push=0;
                Button_new.Push=0;
            }
        }

        if(glfwGetMouseButton(MainWindow, GLFW_MOUSE_BUTTON_LEFT)==GLFW_RELEASE) {
            double xpos, ypos;
            glfwGetCursorPos(MainWindow, &xpos, &ypos);
            if(xpos>Button_new.Left_up_x&&xpos<Button_new.Left_up_x+Button_new.Width) {
                if(ypos>Button_new.Left_up_y&&ypos<Button_new.Left_up_y+Button_new.Height) {
                    Button_new.Push=0;
                }
            }
            if(xpos>Button_measure.Left_up_x&&xpos<Button_measure.Left_up_x+Button_measure.Width) {
                if(ypos>Button_measure.Left_up_y&&ypos<Button_measure.Left_up_y+Button_measure.Height) {
                    Button_measure.Push=0;
                }
            }
        }

        int biggest=0;

        for(int i=0; i<width*height; i++) {
            if(psi[i][0]*psi[i][0]+psi[i][1]*psi[i][1]>psi[biggest][0]*psi[biggest][0]+psi[biggest][1]*psi[biggest][1])
                biggest=i;
        }

        double norming=sqrt(1.0f/(psi[biggest][0]*psi[biggest][0]+psi[biggest][1]*psi[biggest][1]));

        for(int i=0; i<width*height; i++) {
            speicher[i*4+2]=(unsigned char) (0.5f*255*(psi[i][0]*norming+1.0f)/*+psi[i][1]*psi[i][1]*/);
            speicher[i*4+1]=(unsigned char) (0.5f*255*(psi[i][1]*norming+1.0f)/*+psi[i][1]*psi[i][1]*/);
            speicher[i*4+3]=pot[i*4+1];
        }

        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,GL_UNSIGNED_INT_8_8_8_8,speicher);

        float delta_time = update_delta_time();

        //testani_1=testani_1+delta_time*20;
        //Camera Movement calculationunsigned char* speicher = calloc(width*height*4,1);

        if(glfwGetKey(MainWindow,GLFW_KEY_W)==GLFW_PRESS) {
            if(rotation_up_down<(3.0)) {
                rotation_up_down=rotation_up_down+delta_time;
            }
        }
        if(glfwGetKey(MainWindow,GLFW_KEY_S)==GLFW_PRESS) {
            if(rotation_up_down>(-0.0)) {
                rotation_up_down=rotation_up_down-delta_time;
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
        if(glfwGetKey(MainWindow,GLFW_KEY_M)==GLFW_PRESS) {
            if(measurement==0)
                measurement=1;
        }
        if(glfwGetKey(MainWindow,GLFW_KEY_SPACE)==GLFW_PRESS) {
            if(measurement==2) {
                for(int j=0; j<height; j++) {
                    for(int i=0; i<width; i++) {        /*sin((i+wavesize_1)/10)/2+0.5;*/
                        psi[i+j*width][0]=exp(-((i-offset_x_1)*(i-offset_x_1)+(j-offset_y_1)*(j-offset_y_1))/wavesize_1)*cos(((i-height/(float)2)*cos(angle_mov_1)+(j-height/(float)2)*sin(angle_mov_1))*8.0f)
                        +exp(-((i-offset_x_2)*(i-offset_x_2)+(j-offset_y_2)*(j-offset_y_2))/wavesize_2)*cos(((i-height/(float)2)*cos(angle_mov_2)+(j-height/(float)2)*sin(angle_mov_2))*8.0f);
                        psi[i+j*width][1]=exp(-((i-offset_x_1)*(i-offset_x_1)+(j-offset_y_1)*(j-offset_y_1))/wavesize_1)*sin(((i-height/(float)2)*cos(angle_mov_1)+(j-height/(float)2)*sin(angle_mov_1))*8.0f)
                        +exp(-((i-offset_x_2)*(i-offset_x_2)+(j-offset_y_2)*(j-offset_y_2))/wavesize_2)*sin(((i-height/(float)2)*cos(angle_mov_2)+(j-height/(float)2)*sin(angle_mov_2))*8.0f);
                    }
                }
                measurement=0;
            }
        }
        if(glfwGetKey(MainWindow,GLFW_KEY_R)==GLFW_PRESS) {
            if(measurement==1) {
                for(int j=0; j<height; j++) {
                    for(int i=0; i<width; i++) {        /*sin((i+wavesize_1)/10)/2+0.5;*/
                        psi[i+j*width][0]=exp(-((i-offset_x_1)*(i-offset_x_1)+(j-offset_y_1)*(j-offset_y_1))/wavesize_1)*cos(((i-height/(float)2)*cos(angle_mov_1)+(j-height/(float)2)*sin(angle_mov_1))*8.0f);
                        //+exp(-((i-offset_x_2)*(i-offset_x_2)+(j-offset_y_2)*(j-offset_y_2))/wavesize_2)*cos(((i-height/(float)2)*cos(angle_mov_2)+(j-height/(float)2)*sin(angle_mov_2))*8.0f);
                        psi[i+j*width][1]=exp(-((i-offset_x_1)*(i-offset_x_1)+(j-offset_y_1)*(j-offset_y_1))/wavesize_1)*sin(((i-height/(float)2)*cos(angle_mov_1)+(j-height/(float)2)*sin(angle_mov_1))*8.0f);
                        //+exp(-((i-offset_x_2)*(i-offset_x_2)+(j-offset_y_2)*(j-offset_y_2))/wavesize_2)*sin(((i-height/(float)2)*cos(angle_mov_2)+(j-height/(float)2)*sin(angle_mov_2))*8.0f);
                    }
                }
                measurement=2;
            }
        }
        if(glfwGetKey(MainWindow,GLFW_KEY_O)==GLFW_PRESS) {
            if(Slider_size.Position>0) {
                Slider_size.Position=Slider_size.Position-1;
                wavesize_1=Slider_size.Position*5.0f;
                for(int j=0; j<height; j++) {
                    for(int i=0; i<width; i++) {        /*sin((i+wavesize_1)/10)/2+0.5;*/
                        psi[i+j*width][0]=exp(-((i-offset_x_1)*(i-offset_x_1)+(j-offset_y_1)*(j-offset_y_1))/wavesize_1)*cos(((i-height/(float)2)*cos(angle_mov_1)+(j-height/(float)2)*sin(angle_mov_1))*8.0f);
                        //+exp(-((i-offset_x_2)*(i-offset_x_2)+(j-offset_y_2)*(j-offset_y_2))/wavesize_2)*cos(((i-height/(float)2)*cos(angle_mov_2)+(j-height/(float)2)*sin(angle_mov_2))*8.0f);
                        psi[i+j*width][1]=exp(-((i-offset_x_1)*(i-offset_x_1)+(j-offset_y_1)*(j-offset_y_1))/wavesize_1)*sin(((i-height/(float)2)*cos(angle_mov_1)+(j-height/(float)2)*sin(angle_mov_1))*8.0f);
                        //+exp(-((i-offset_x_2)*(i-offset_x_2)+(j-offset_y_2)*(j-offset_y_2))/wavesize_2)*sin(((i-height/(float)2)*cos(angle_mov_2)+(j-height/(float)2)*sin(angle_mov_2))*8.0f);
                    }
                }
                measurement = 2;
            }
        }
        if(glfwGetKey(MainWindow,GLFW_KEY_P)==GLFW_PRESS) {
            if(Slider_size.Position<Slider_size.Width) {
                Slider_size.Position=Slider_size.Position+1;
                wavesize_1=Slider_size.Position*5.0f;
                for(int j=0; j<height; j++) {
                    for(int i=0; i<width; i++) {        /*sin((i+wavesize_1)/10)/2+0.5;*/
                        psi[i+j*width][0]=exp(-((i-offset_x_1)*(i-offset_x_1)+(j-offset_y_1)*(j-offset_y_1))/wavesize_1)*cos(((i-height/(float)2)*cos(angle_mov_1)+(j-height/(float)2)*sin(angle_mov_1))*8.0f);
                        //+exp(-((i-offset_x_2)*(i-offset_x_2)+(j-offset_y_2)*(j-offset_y_2))/wavesize_2)*cos(((i-height/(float)2)*cos(angle_mov_2)+(j-height/(float)2)*sin(angle_mov_2))*8.0f);
                        psi[i+j*width][1]=exp(-((i-offset_x_1)*(i-offset_x_1)+(j-offset_y_1)*(j-offset_y_1))/wavesize_1)*sin(((i-height/(float)2)*cos(angle_mov_1)+(j-height/(float)2)*sin(angle_mov_1))*8.0f);
                        //+exp(-((i-offset_x_2)*(i-offset_x_2)+(j-offset_y_2)*(j-offset_y_2))/wavesize_2)*sin(((i-height/(float)2)*cos(angle_mov_2)+(j-height/(float)2)*sin(angle_mov_2))*8.0f);
                    }
                }
                measurement = 2;
            }
        }
        //camera projection an transformation matrix calculation
        eye_vec[0]=1.5f*sin(rotation_left_right)*cos(atan(rotation_up_down));
        eye_vec[1]=1.5f*cos(rotation_left_right)*cos(atan(rotation_up_down));
        eye_vec[2]=1.5f*sin(atan(rotation_up_down));
        mat4x4_look_at(mvp4x4,eye_vec,cent_vec,up_vec);
        mat4x4_perspective(persp4x4,FOV,16.0f/9.0f,0.5f,10.0f);
        mat4x4_mul(mvp4x4,persp4x4,mvp4x4);
        glUniformMatrix4fv(MVPmatrix,1,GL_FALSE,(GLfloat*)mvp4x4);
        glUniform1f(potential_true,1.0f);

        /*//update textures
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
        }
*/

        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        unsigned int ibufferplane;

        //Render Grid
        glUniform1f(potential_true,1.0f);
        for(ibufferplane=0; ibufferplane<(*((unsigned int*)index_buffer_array+1)-1); ibufferplane++) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,*(((GLuint*)index_buffer_array)+2+(3*sizeof(unsigned long)/sizeof(GLuint))+(*((unsigned int*)index_buffer_array))+ibufferplane));
            glDrawElements(GL_LINES,(*((unsigned long*)index_buffer_array+(2*sizeof(GLuint))/sizeof(unsigned long)+2)/2)*2,GL_UNSIGNED_INT,0);
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,*(((GLuint*)index_buffer_array)+2+(3*sizeof(unsigned long)/sizeof(GLuint))+(*((unsigned int*)index_buffer_array))+ibufferplane));
        glDrawElements(GL_LINES,*((unsigned long*)index_buffer_array+(2*sizeof(GLuint))/sizeof(unsigned long)+1),GL_UNSIGNED_INT,0);

        //enable Transparency
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE,GL_ONE);
        //Render Plane
        glUniform1f(potential_true,0.0f);
        for(ibufferplane=0; ibufferplane<(*((unsigned int*)index_buffer_array)-1); ibufferplane++) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,*(((GLuint*)index_buffer_array)+2+(3*sizeof(unsigned long)/sizeof(GLuint))+ibufferplane));
            glDrawElements(GL_TRIANGLES,(*((unsigned long*)index_buffer_array+(2*sizeof(GLuint))/sizeof(unsigned long)+2)/3)*3,GL_UNSIGNED_INT,0);
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,*(((GLuint*)index_buffer_array)+2+(3*sizeof(unsigned long)/sizeof(GLuint))+ibufferplane));
        glDrawElements(GL_TRIANGLES,*((unsigned long*)index_buffer_array+(2*sizeof(GLuint))/sizeof(unsigned long)),GL_UNSIGNED_INT,0);
        //disable Transparency
        glDisable(GL_BLEND);

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
    //TODO: move key checking routines here.
}
void mouse_button_callback(GLFWwindow* window, int button,int action, int mods) {
    if(button== GLFW_MOUSE_BUTTON_LEFT&&action==GLFW_PRESS) {
        printf("LMB Down");
    }
}

GLuint CompileShaderFromFile(char FilePath[],GLuint shaderType) {
    //read from file into heap memory
    FILE* filepointer=fopen(FilePath,"rb");                  //open specified file in read only mode
    if(filepointer==NULL) {
        printf("Error: Filepointer to shaderfile at %screatePlaneVBO2 could not be loaded.",FilePath);
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
    printf("Error in glfw %d occured\nDescription: %s\n",error,description);
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

void* createPlaneVBO(unsigned int PlaneResolution, unsigned int GridResolution) {
    //Input Parameter check
    if(((PlaneResolution&(PlaneResolution-1))!=0)||((GridResolution&(GridResolution-1))!=0)) {      //Check if plane resolution/grid resolution is power of 2
        printf("Error: Resolution of plane or grid is not a power of 2");
        return 0;
    }
    //Local Variable Def
    int maxIndices=0;
    int maxVertices=0;
    unsigned int IndexBufferCountTriangles=0;
    unsigned int IndexBufferCountLines=0;
    unsigned int finalVertexResolution=0;
    unsigned int planeOffsetMultiplier=1;
    unsigned int gridOffsetMultiplier=1;
    glGetIntegerv(GL_MAX_ELEMENTS_INDICES,&maxIndices);         //get max supported IndexBufferSize of GPU
    glGetIntegerv(GL_MAX_ELEMENTS_VERTICES,&maxVertices);

    if(maxVertices<1048576) {
        printf("Error: Vertex Count of your GPU is %d! But requiered count is 1048576\n",maxVertices);
        return 0;
    }
    if(((PlaneResolution-1)*(PlaneResolution-1)*6)%((maxIndices/3)*3)==0) {   //
        IndexBufferCountTriangles=(((PlaneResolution-1)*(PlaneResolution-1)*6)/((maxIndices/3)*3));
    } else {
        IndexBufferCountTriangles=(((PlaneResolution-1)*(PlaneResolution-1)*6)/((maxIndices/3)*3))+1;
    }
    if(((GridResolution-1)*(GridResolution-1)*8)%((maxIndices/2)*2)==0) {
        IndexBufferCountLines=(((GridResolution-1)*(GridResolution-1)*8)/((maxIndices/2)*2));
    } else {
        IndexBufferCountLines=(((GridResolution-1)*(GridResolution-1)*8)/((maxIndices/2)*2))+1;
    }
    void* return_data_pointer=malloc(2*sizeof(unsigned int)+3*sizeof(unsigned long)+IndexBufferCountTriangles*sizeof(GLuint)+IndexBufferCountLines*sizeof(GLuint));
    *((int*)return_data_pointer)=IndexBufferCountTriangles;
    printf("Info: IndexBufferCountTriangles %d\n",*((int*)return_data_pointer));
    *(((int*)return_data_pointer)+1)=IndexBufferCountLines;
    printf("Info: IndexBufferCountLines %d\n",*(((int*)return_data_pointer)+1));
    //Which Mesh should be bigger
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
    //Generate Vertex Array Object
    GLuint VaoId=0;
    glGenVertexArrays(1,&VaoId);
    glBindVertexArray(VaoId);

    //Generate Vertex Positions

    float* plane_vertex_data=malloc(2*finalVertexResolution*finalVertexResolution*sizeof(float));
    unsigned long vert_index=0;
    for(int y=0; y<finalVertexResolution; y++) {
        for(int x=0; x<finalVertexResolution; x++) {
            //Vector coordinates (x,y,z)
            plane_vertex_data[vert_index++]=(((float)x)/(finalVertexResolution-1))-0.5f;
            plane_vertex_data[vert_index++]=(((float)y)/(finalVertexResolution-1))-0.5f; //Set height (y) to zero
        }
    }
    GLuint VboPositionsId=0;
    glGenBuffers(1, &VboPositionsId);                                                          //create buffer
    glBindBuffer(GL_ARRAY_BUFFER, VboPositionsId);                                            //Link buffer
    glBufferData(GL_ARRAY_BUFFER, 2*finalVertexResolution*finalVertexResolution*sizeof(float),plane_vertex_data,GL_STATIC_DRAW);    //Upload data to Buffer, Vertex data is set only once and drawn regularly, hence we use GL_STATIC_DRAW
    free(plane_vertex_data);    //we no longer need plane_verrtex_data because it has been uploaded to gpu memory
    glEnableVertexAttribArray(0);//x,y
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,0);

    //Generate Vertex Indices for Triangles
    GLuint* plane_indices = malloc((finalVertexResolution-1)*(finalVertexResolution-1)*8*sizeof(GLuint)); //6 from the points of two triangles, 8 from 4 lines per gridcell max(6,8)=8
    vert_index=0;
    for(unsigned int y=0; y<(finalVertexResolution-planeOffsetMultiplier); y+=planeOffsetMultiplier) {
        for(unsigned int x=0; x<(finalVertexResolution-planeOffsetMultiplier); x+=planeOffsetMultiplier) {
            //Generate first triangle
            plane_indices[vert_index++]=x+(y*finalVertexResolution);   //Vertex lower left first triangle
            plane_indices[vert_index++]=x+planeOffsetMultiplier+(y*finalVertexResolution);//Vertex upper right first triangle
            plane_indices[vert_index++]=x+((y+planeOffsetMultiplier)*finalVertexResolution); //Vertex upper left first triangle
            //Generate second triangle
            plane_indices[vert_index++]=x+planeOffsetMultiplier+(y*finalVertexResolution);   //Vertex lower left second triangle
            plane_indices[vert_index++]=x+planeOffsetMultiplier+((y+planeOffsetMultiplier)*finalVertexResolution); //Vertex lower right second triangle
            plane_indices[vert_index++]=x+((y+planeOffsetMultiplier)*finalVertexResolution); //Vertex upper right first triangle
            //printf("vert%d,%d,%d,%d,%d,%d\n",plane_indices[vert_index-6],plane_indices[vert_index-5],plane_indices[vert_index-4],plane_indices[vert_index-3],plane_indices[vert_index-2],plane_indices[vert_index-1]);
            //Check if we need to start a new array
        }
    }
    glGenBuffers(IndexBufferCountTriangles, ((GLuint*)return_data_pointer)+2+3*sizeof(long)/sizeof(GLuint));
    //Now upload this data to GPU
    unsigned int bufferNumber=0;
    for(; bufferNumber<(IndexBufferCountTriangles-1); bufferNumber++) { //Upload all but the last buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,*(((GLuint*)return_data_pointer)+2+3*sizeof(long)/sizeof(GLuint)+bufferNumber));
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,(maxIndices/3)*3*sizeof(GLuint),plane_indices+(maxIndices/3)*3*bufferNumber,GL_STATIC_DRAW);
        printf("Info: Uploaded PlaneIndexBuffer%d with %d bytes.\n",*(((GLuint*)return_data_pointer)+2+3*sizeof(long)/sizeof(GLuint)+bufferNumber),(maxIndices/3)*3*sizeof(GLuint));
    }
    //Upload the last Buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,*(((GLuint*)return_data_pointer)+2+3*sizeof(long)/sizeof(GLuint)+bufferNumber));

    if(vert_index%((maxIndices/3)*3)!=0) {
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,(vert_index%((maxIndices/3)*3))*sizeof(GLuint),plane_indices+((maxIndices/3)*3)*bufferNumber,GL_STATIC_DRAW);
    } else {
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,((maxIndices/3)*3)*sizeof(GLuint),plane_indices+((maxIndices/3)*3)*bufferNumber,GL_STATIC_DRAW);
    }
    *(((unsigned long*)return_data_pointer)+(2*sizeof(unsigned int))/sizeof(unsigned long))=vert_index%((maxIndices/3)*3);
    printf("Info: Indices in last Triangle Buffer %d\n",*(((unsigned long*)return_data_pointer)+(2*sizeof(unsigned int))/sizeof(unsigned long)));
    //Generate Vertex Indices for Grid
    vert_index=0;
    for(unsigned int y=0; y<(finalVertexResolution-gridOffsetMultiplier); y+=gridOffsetMultiplier) {
        for(unsigned int x=0; x<(finalVertexResolution-gridOffsetMultiplier); x+=gridOffsetMultiplier) {
            //Generate first line
            plane_indices[vert_index++]=x+(y*finalVertexResolution);
            plane_indices[vert_index++]=x+gridOffsetMultiplier+(y*finalVertexResolution);
            //Generate second line
            plane_indices[vert_index++]=x+(y*finalVertexResolution);
            plane_indices[vert_index++]=x+((y+gridOffsetMultiplier)*finalVertexResolution);
            //Generate third line
            plane_indices[vert_index++]=x+((y+gridOffsetMultiplier)*finalVertexResolution);
            plane_indices[vert_index++]=x+gridOffsetMultiplier+((y+gridOffsetMultiplier)*finalVertexResolution);
            //Generate fourth line
            plane_indices[vert_index++]=x+gridOffsetMultiplier+(y*finalVertexResolution);
            plane_indices[vert_index++]=x+gridOffsetMultiplier+((y+gridOffsetMultiplier)*finalVertexResolution);
        }
    }
    glGenBuffers(IndexBufferCountLines,((GLuint*)return_data_pointer)+2+3*sizeof(long)/sizeof(GLuint)+IndexBufferCountTriangles);   //Skip over 2*int+(2*long==4*int)=6
    printf("Info: IndexBufferFirstInListTriangles %d\n",*(((GLuint*)return_data_pointer)+2+3*sizeof(long)/sizeof(GLuint)+IndexBufferCountLines));
    bufferNumber=0;
    for(; bufferNumber<(IndexBufferCountLines-1); bufferNumber++) { //Upload all but the last buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,*(((GLuint*)return_data_pointer)+2+3*sizeof(long)/sizeof(GLuint)+IndexBufferCountTriangles+bufferNumber));
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,((maxIndices/2)*2)*sizeof(GLuint),plane_indices+((maxIndices/2)*2)*bufferNumber,GL_STATIC_DRAW);
    }
    //Upload the last Buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,*(((GLuint*)return_data_pointer)+2+3*sizeof(long)/sizeof(GLuint)+IndexBufferCountTriangles+bufferNumber));
    if(vert_index%((maxIndices/2)*2)!=0) {
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,(vert_index%((maxIndices/2)*2))*sizeof(GLuint),plane_indices+((maxIndices/2)*2)*bufferNumber,GL_STATIC_DRAW);
    } else {
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,((maxIndices/2)*2)*sizeof(GLuint),plane_indices+((maxIndices/2)*2)*bufferNumber,GL_STATIC_DRAW);
    }
    *(((unsigned long*)return_data_pointer)+(2*sizeof(unsigned int))/sizeof(unsigned long)+1)=vert_index%((maxIndices/2)*2);
    printf("Info: Indices in last Grid Buffer %d\n",*(((unsigned long*)return_data_pointer)+(2*sizeof(unsigned int))/sizeof(unsigned long)+1));
    *(((unsigned long*)return_data_pointer)+(2*sizeof(unsigned int))/sizeof(unsigned long)+2)=maxIndices;
    printf("Info: Supported Max Indices %d\n",*(((unsigned long*)return_data_pointer)+(2*sizeof(unsigned int))/sizeof(unsigned long)+2));
    free(plane_indices);
    return return_data_pointer;//return_data_pointer;
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
        printf("File :%s could not be found\n",filepath);
        fclose(filepointer);
        return 0;
    }

    fseek(filepointer,0,SEEK_SET);  //Jump to beginning of file
    if(read_short_from_endian_file(filepointer)!=0x4D42) { // (equals BM in ASCII)
        fclose(filepointer);
        printf("File :%s is not an BMP\n",filepath);
        return 0;
    }

    fseek(filepointer,10,SEEK_SET);
    unsigned int BitmapOffset = read_uint_from_endian_file(filepointer);
    printf("data offset:%d\n",BitmapOffset);

    if(read_uint_from_endian_file(filepointer)!=124) {
        printf("BitmapHeader is not BITMAPV5HEADER / 124 \n");
        fclose(filepointer);
        return 0;
    }
    unsigned int BitmapWidth=read_uint_from_endian_file(filepointer);
    printf("BitmapWidth is %d.\n",BitmapWidth);

    unsigned int BitmapHeight=read_uint_from_endian_file(filepointer);
    printf("BitmapHeight is %d.\n",BitmapHeight);

    if(read_short_from_endian_file(filepointer)!=1) {
        printf("Unsupported plane count\n");
        return 0;
    }

    unsigned int BitmapColorDepth=read_short_from_endian_file(filepointer);
    printf("BMP color depth:%d",BitmapColorDepth);
    unsigned int BitmapSizeCalculated=(BitmapColorDepth/8)*(BitmapWidth+(BitmapWidth%4))*BitmapWidth;

    unsigned int BitmapCompression=read_uint_from_endian_file(filepointer);
    switch(BitmapCompression) {
    case 0:
        printf("Compression type: none/BI_RGB\n");
        break;
    case 3:
        printf("Compression type: Bitfields/BI_BITFIELDS\n");
        break;
    default:
        printf("Unsupported compression %d\n",BitmapCompression);
        fclose(filepointer);
        return 0;
        break;
    }
    unsigned int BitmapImageSize=read_uint_from_endian_file(filepointer);
    if(BitmapImageSize!=BitmapSizeCalculated) {
        printf("Error while reading image size: Calculated Image Size: %d.\nRead Image size: %d\n",BitmapSizeCalculated,BitmapImageSize);
        fclose(filepointer);
        return 0;
    }
    printf("Image Size:%d\n",BitmapSizeCalculated);
    /*unsigned int BitmapXPpM=*/read_uint_from_endian_file(filepointer);
    /*unsigned int BitmapYPpM=*/read_uint_from_endian_file(filepointer);
    unsigned int BitmapColorsInPalette=read_uint_from_endian_file(filepointer);
    printf("Colors in palette: %d.\n",BitmapColorsInPalette);
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
                printf("Error while BITMASK read. Value: %x!\n",color_channel_mask);
                fclose(filepointer);
                return 0;
                break;
            }
        }
        //TODO implement swapping routine if brga!=[3,2,1,0]
        printf("Shifting value for R:%d G:%d B:%d A:%d\n",RGBA_mask[0],RGBA_mask[1],RGBA_mask[2],RGBA_mask[3]);
        unsigned char* imageData=malloc(BitmapSizeCalculated);
        printf("BMOFFST:%d\n",BitmapOffset);
        fseek(filepointer,BitmapOffset,SEEK_SET);//jump to pixel data
        printf("Calsize:%d\n",BitmapSizeCalculated);
        if(fread(imageData,BitmapSizeCalculated,1,filepointer)==0) {
            printf("Error while reading!");
        }
        fclose(filepointer);
        return imageData;
    }
    printf("Currently not implemented!");
    fclose(filepointer);
    return 0;
}




void write_bmp(char* filepath, unsigned int width, unsigned int height) {
    FILE* filepointer = fopen(filepath,"wb");
    //bytes_per_line=(3*(width+1)/4)*4;
    const char* String_to_write="BMP";
    fwrite(&String_to_write,sizeof(char),3,filepointer);
    return;
}
