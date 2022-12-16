/**
* Autor: Nedeljko Tesanovic
* Namjena: Demonstracija upotrebe sablonskog projekta za ucitavanje i prikaz modela, 3D transformacije, perspektivne projekcije i klase za bafere
* Original file info
 * @file main.cpp
 * @author Jovan Ivosevic
 * @brief Base project for Computer Graphics course
 * @version 0.1
 * @date 2022-10-09
 *
 * @copyright Copyright (c) 2022
 *
 */
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <chrono>
#include <thread>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include "shader.hpp"
#include "model.hpp" //Klasa za ucitavanje modela
#include "renderable.hpp" //Klasa za bafere


const int WindowWidth = 1280;
const int WindowHeight = 720;
const std::string WindowTitle = "GIM Flight Sim";
const float TargetFPS = 60.0f;
const float TargetFrameTime = 1.0f / TargetFPS;

void FramebufferSizeCallback(GLFWwindow* window, int width, int height) //Aktivira se pri mijenjanju velicine prozora
{
    glViewport(0, 0, width, height);
    /*glViewport(X, Y, w, h) mapira prikaz scene na dio prozora. Vrijednosti parametara su pozitivni cijeli brojevi
    * (radi sa rezolucijom ekrana, ne sa NDC sistemom)
    * X, Y = koordinate donjeg lijevog coska (0,0 je donji lijevi cosak prozora)
    * width, height = duzina i visina prikaza
     */
}

static void
KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    bool IsDown = action == GLFW_PRESS || action == GLFW_REPEAT;
    switch (key) {
        case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GLFW_TRUE); break;
    }
}

static void
ErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error: " << description << std::endl;
}

int main() {
    GLFWwindow* Window = 0;
    if (!glfwInit()) {
        std::cerr << "Failed to init glfw" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwSetErrorCallback(ErrorCallback);

    Window = glfwCreateWindow(WindowWidth, WindowHeight, WindowTitle.c_str(), 0, 0);
    if (!Window) {
        std::cerr << "Failed to create window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(Window);
    glfwSetKeyCallback(Window, KeyCallback);
    glfwSetFramebufferSizeCallback(Window, FramebufferSizeCallback);

    GLenum GlewError = glewInit();
    if (GlewError != GLEW_OK) {
        std::cerr << "Failed to init glew: " << glewGetErrorString(GlewError) << std::endl;
        glfwTerminate();
        return -1;
    }

    Shader Basic("shaders/basic.vert", "shaders/basic.frag");



    Model IronMan("IronMan/IronMan.obj");
    if (!IronMan.Load())
    {
        std::cout << "Failed to load model!\n";
        glfwTerminate();
        return -1;
    }



    float cubeVertices[] = //Bijela kocka. Boju cemo mijenjati sa uCol
    {
      -0.2, -0.2,  0.2,   0, 0, 0,
        0.2, -0.2,  0.2,    0, 0, 0,
        0.2,  0.2,  0.2,    0, 0, 0,
        -0.2,  0.2,  0.2,   0, 0, 0,
        // back
        -0.2, -0.2, -0.2,   0, 0, 0,
        0.2, -0.2, -0.2,    0, 0, 0,
        0.2,  0.2, -0.2,    0, 0, 0,
        -0.2,  0.2, -0.2,   0, 0, 0
    };

    float funkyCubeVertices[] = //Sarena kocka
    {
        -0.2, -0.2, -0.2,       0.0, 0.0, 1.0,
        +0.2, -0.2, -0.2,       0.0, 1.0, 0.0,
        -0.2, -0.2, +0.2,       0.0, 1.0, 1.0,
        +0.2, -0.2, +0.2,       1.0, 0.0, 0.0,

        -0.2, +0.2, -0.2,       1.0, 0.0, 1.0,
        +0.2, +0.2, -0.2,       1.0, 1.0, 0.0,
        -0.2, +0.2, +0.2,       1.0, 1.0, 1.0,
        +0.2, +0.2, +0.2,       0.0, 0.0, 0.0,
    };

    float triangleVertices[] = {
    -0.5f, -0.5f, 0.0f, 0.0, 0.0, 0.0,
     0.5f, -0.5f, 0.0f, 0.0, 0.0, 0.0,
     0.0f,  0.5f, 0.0f, 0.0, 0.0, 0.0,
    };

    unsigned int triangleIndices[] =
    {
        0, 1, 2, //Prvi trougao sacinjen od tjemena 0, tjemena 1 i tjemena 2 (Prednja strana mu je naprijed)
        1, 2, 3, //Drugi trougao (Zadnja strana mu je naprijed)
    };


    unsigned int cubeIndices[] = {
        0, 1, 2,
        2, 3, 0,
        // right
        1, 5, 6,
        6, 2, 1,
        // back
        7, 6, 5,
        5, 4, 7,
        // left
        4, 0, 3,
        3, 7, 4,
        // bottom
        4, 5, 1,
        1, 0, 4,
        // top
        3, 2, 6,
        6, 7, 3
    }; //Indeksi za formiranje kocke

    //Pravljenje Renderable objekta (Generise VAO i VBO pri konstrukciji)
    Renderable cube(cubeVertices, sizeof(cubeVertices), cubeIndices, sizeof(cubeIndices));
    Renderable funkyCube(funkyCubeVertices, sizeof(funkyCubeVertices), cubeIndices, sizeof(cubeIndices));
    Renderable triangle(triangleVertices, sizeof(triangleVertices), triangleIndices, sizeof(triangleIndices));


    //Matrica modela (transformacije)
    glm::mat4 m(1.0f);
    //mat4(A) generise matricu 4x4 sa A na glavnoj dijagonali

    //Matrica pogleda (kamere)
    glm::mat4 v = glm::lookAt(glm::vec3(0.0, 1.0, -2.2), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
    //lookAt(pozicija kamere, tacka u koju kamera gleda, kako je kamera rotirana po osi definisanoj sa prethodne dvije tacke)
    
    //Matrica projekcije
    glm::mat4 p = glm::perspective(glm::radians(90.0f), (float)WindowWidth / WindowHeight, 0.1f, 100.0f);
    //perspective(FOV ugao, aspect ratio prozora, prednja odsjecna ravan i zadnja odsjecna ravan)
    
    float angle = 0;//Ugao za rotiranje aviona
   

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.04, 1.00, 1.00, 1.0);


    float FrameStartTime = glfwGetTime();
    float FrameEndTime = glfwGetTime();
    float dt = FrameEndTime - FrameStartTime;
    int oblaci_vidljivi = 1;
    int culling = 1;

    while (!glfwWindowShouldClose(Window)) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (glfwGetKey(Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(Window, GL_TRUE);
        }

        if (glfwGetKey(Window, GLFW_KEY_P) == GLFW_PRESS)
        {
            oblaci_vidljivi = 0;
        }

        if (glfwGetKey(Window, GLFW_KEY_O) == GLFW_PRESS)
        {
            oblaci_vidljivi = 1;
        }

        if (glfwGetKey(Window, GLFW_KEY_K) == GLFW_PRESS)
        {
            culling = 1;
        }
        if (glfwGetKey(Window, GLFW_KEY_L) == GLFW_PRESS)
        {
            culling = 0;
        }



        FrameStartTime = glfwGetTime();
        glUseProgram(Basic.GetId());
        Basic.SetProjection(p);
        Basic.SetView(v);

        if(culling==1)
        {
            glEnable(GL_CULL_FACE); //Ukljuci eliminisanje lica
            glCullFace(GL_BACK); //Koja lica eliminisemo Za prednja: GL_FRONT, Za zadnja: GL_BACK, za prednja i zadnja: GL_FRONT_AND_BACK
        }


        Basic.SetColor(1, 0.5+ abs(sin(glfwGetTime()*2)), 0);
        m = glm::translate(glm::mat4(1.0f), glm::vec3(+1.9, 1.2, -0.4));
        m = glm::rotate(m, glm::radians((float)glfwGetTime() * 30), glm::vec3(1.0, 0, 1));
        Basic.SetModel(m);
        cube.Render();

        Basic.SetColor(1, 0.5 + abs(sin(glfwGetTime()*3)), 0.1);
        m = glm::translate(glm::mat4(1.0f), glm::vec3(+1.9, 1.2, -0.4));
        m = glm::rotate(m, glm::radians((float)glfwGetTime() * 20), glm::vec3(-1.0, 1.0, 0));
        Basic.SetModel(m);
        cube.Render();

        //Crtanje oblaka
        if(oblaci_vidljivi == 1)
        {
            //Levi oblak
            Basic.SetColor(1, 1, 1);
            m = glm::translate(glm::mat4(1.0f), glm::vec3(1.0, 1.3, 0.1));
            m = glm::rotate(m, glm::radians(135.0f), glm::vec3(10, 0, 1.0));
            m = glm::scale(m, glm::vec3(1.0, 1.0, 2.0));
            Basic.SetModel(m);
            cube.Render();

            //Desni oblak
            Basic.SetColor(0.9, 0.9, 0.9);
            m = glm::translate(glm::mat4(1.0f), glm::vec3(-2.0, 1.5, 0.1));
            m = glm::rotate(m, glm::radians(15.0f), glm::vec3(1.0, 1.0, 1.0));
            m = glm::scale(m, glm::vec3(1.50, 0.5, 2.0));
            Basic.SetModel(m);
            cube.Render();
        }
      

        //More
        float menjanje_nijansi_plave = (abs(sin(glfwGetTime()*2)));
        Basic.SetColor(menjanje_nijansi_plave/10, menjanje_nijansi_plave/4 , 1);
        m = glm::translate(glm::mat4(1.0f), glm::vec3(0, (abs(sin(glfwGetTime() / 2)) / 5) - 1.5, 0.0));
        m = glm::scale(m, glm::vec3(55.0, 5.0, 15.0));
        Basic.SetModel(m);
        cube.Render();

    	

        // Desno Ostrvo

        Basic.SetColor(0.43, 0.17, 0.00);
        m = glm::rotate(m, glm::radians(115.0f), glm::vec3(0, 10, 0));
        m = glm::translate(glm::mat4(1.0f), glm::vec3(-3, -0.2, 0));
        m = glm::scale(m, glm::vec3(1.5));
        Basic.SetModel(m);
        cube.Render();



        // Levo Ostrvo
        Basic.SetColor(0.43, 0.17, 0.00);
        m = glm::translate(glm::mat4(1.0f), glm::vec3(3, -0.4, 0));
        m = glm::rotate(m, glm::radians((4.0f*10)), glm::vec3(0, 1, 0));
        m = glm::scale(m, glm::vec3(2.5,2.5,2.5));
        Basic.SetModel(m);
        cube.Render();


        // Centralno ostrvo
        Basic.SetColor(0.43, 0.17, 0.00);
        m = glm::rotate(m, glm::radians(30.0f), glm::vec3(0, 1, 0));
        m = glm::translate(glm::mat4(1.0f), glm::vec3(0, -0.9, -0.5));
        m = glm::scale(m, glm::vec3(5));
        Basic.SetModel(m);
        cube.Render();


        // Stablo palme

        Basic.SetColor(0.75, 0.43, 0.12);
        m = glm::translate(glm::mat4(1.0f), glm::vec3(0, -0.2, -0.5));
        m = glm::rotate(m, glm::radians(30.0f), glm::vec3(0, 1, 0));
        m = glm::scale(m, glm::vec3(0.5,7,0.5));
        Basic.SetModel(m);
        cube.Render();



        /////////////////////////////////////// IronMan
  
        Basic.SetColor(1, 0,0 );
        m = glm::translate(glm::mat4(1.0f), glm::vec3(0.2, 0.20, -1.0));
        m = glm::scale(m, glm::vec3(0.002));
        Basic.SetModel(m);
        IronMan.Render();



        // Vrh palme 
        Basic.SetColor(0.00, 0.31, 0.04);
        m = glm::translate(glm::mat4(1.0f), glm::vec3(0, 1.2, -0.5));
        m = glm::rotate(m, glm::radians(30.0f), glm::vec3(0, 1, 0));
        m = glm::scale(m, glm::vec3(0.6, 0.4, 0.6));
        Basic.SetModel(m);
        cube.Render();


        //Levi list palme
        Basic.SetColor(0.00, 0.31, 0.04);
        m = glm::translate(glm::mat4(1.0f), glm::vec3(0.2, 1.05, -0.5));
        m = glm::scale(m, glm::vec3(0.5, 0.7, 1));
        m = glm::rotate(m, glm::radians(120.0f), glm::vec3(0, 1, 0));
        m = glm::rotate(m, glm::radians(-40.0f), glm::vec3(1, 0, 0));
        Basic.SetModel(m);
        triangle.Render();


        //Desni list palme
        Basic.SetColor(0.00, 0.31, 0.04);
        m = glm::translate(glm::mat4(1.0f), glm::vec3(-0.2, 1.06, -0.5));
        m = glm::scale(m, glm::vec3(0.5, 0.7, 1));
        m = glm::rotate(m, glm::radians(230.0f), glm::vec3(0, 1, 0));
        m = glm::rotate(m, glm::radians(-40.0f), glm::vec3(1, 0, 0));
        Basic.SetModel(m);
        triangle.Render();

        //Centralni list palme
        Basic.SetColor(0.00, 0.31, 0.04);
        m = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 1.06, -0.7));
        m = glm::scale(m, glm::vec3(0.2, 0.7, 1));
        m = glm::rotate(m, glm::radians(190.0f), glm::vec3(0, 1, 0));
        m = glm::rotate(m, glm::radians(-40.0f), glm::vec3(1, 0, 0));
        Basic.SetModel(m);
        triangle.Render();




        if (culling == 1)
        {
            glDisable(GL_CULL_FACE); //Iskljuci eliminisanje lica - ovako mozete da ukljucujete/iskljucujete pri iscrtavanju samo odredjenih objekata koji su trenutno bind-ovani
        }

    

        glUseProgram(0);
        glfwSwapBuffers(Window);

        FrameEndTime = glfwGetTime();
        dt = FrameEndTime - FrameStartTime;
        if (dt < TargetFPS) {
            int DeltaMS = (int)((TargetFrameTime - dt) * 1e3f);
            std::this_thread::sleep_for(std::chrono::milliseconds(DeltaMS));
            FrameEndTime = glfwGetTime();
        }
        dt = FrameEndTime - FrameStartTime;
    }

    glfwTerminate();
    return 0;
}



