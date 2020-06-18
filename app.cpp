//TODO: Set up quad rendering to use shaders and crap
//TODO: Add actual doc-style comments

#include "app.h"

#include <iostream>
#include <glew.h>

#include <gl/GL.h>
#include <gl/GLU.h>
#include <SDL_image.h>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl.h>

#include <glm/mat4x4.hpp>
#include <glm/gtx/transform.hpp>

#include <initializer_list>

#include "texture.h"
#include "shader.h"
#include "mesh.h"
#include "camera.h"

App::App() : gTimer(0.0f)
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
    IMG_Init(IMG_INIT_PNG);
    // SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    window = SDL_CreateWindow("Total Editor 3", 
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
        WINDOW_WIDTH, WINDOW_HEIGHT, 
        SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
    context = SDL_GL_CreateContext(window);

    SDL_GL_SetSwapInterval(1);

    if (glewInit() != GLEW_OK)
    {
        fprintf(stderr, "Failed to initialize GLEW!\n");
    }

    assets = std::make_unique<Assets>("assets");
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(window, context);
    ImGui_ImplOpenGL3_Init("#version 330");
}

App::~App()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}

void App::beginLoop()
{
    //For some stupid reason, class constants cannot be passed into std::make_shared...
    auto camera = std::shared_ptr<Camera>(new Camera(75.0f, WINDOW_ASPECT_RATIO, 0.1f, 100.0f));
    
    auto testTexture = assets->getTexture("textures\\spacewall.png");
    auto testTexture2 = assets->getTexture("textures\\spacefloor.png");

    //pos, uv, col, norm
    //-z is in front of default camera
    auto testMesh = std::shared_ptr<Mesh>(
        new Mesh (
            {
                //Back Side          UV            COLOR                       NORMAL
                -1.0f, -1.0f, -1.0f, +0.0f, +0.0f, +1.0f, +1.0f, +1.0f, +1.0f, +0.0f, +0.0f, -1.0f,
                +1.0f, -1.0f, -1.0f, +1.0f, +0.0f, +1.0f, +1.0f, +1.0f, +1.0f, +0.0f, +0.0f, -1.0f,
                +1.0f, +1.0f, -1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +0.0f, +0.0f, -1.0f,
                -1.0f, +1.0f, -1.0f, +0.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +0.0f, +0.0f, -1.0f,
                //Front Side
                -1.0f, -1.0f, +1.0f, +0.0f, +0.0f, +1.0f, +1.0f, +1.0f, +1.0f, +0.0f, +0.0f, +1.0f,
                +1.0f, -1.0f, +1.0f, +1.0f, +0.0f, +1.0f, +1.0f, +1.0f, +1.0f, +0.0f, +0.0f, +1.0f,
                +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +0.0f, +0.0f, +1.0f,
                -1.0f, +1.0f, +1.0f, +0.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +0.0f, +0.0f, +1.0f,
                //Left Side
                -1.0f, -1.0f, -1.0f, +0.0f, +0.0f, +1.0f, +1.0f, +1.0f, +1.0f, -1.0f, +0.0f, +0.0f,
                -1.0f, -1.0f, +1.0f, +1.0f, +0.0f, +1.0f, +1.0f, +1.0f, +1.0f, -1.0f, +0.0f, +0.0f,
                -1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, -1.0f, +0.0f, +0.0f,
                -1.0f, +1.0f, -1.0f, +0.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, -1.0f, +0.0f, +0.0f,
                //Right Side
                +1.0f, -1.0f, -1.0f, +0.0f, +0.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +0.0f, +0.0f,
                +1.0f, -1.0f, +1.0f, +1.0f, +0.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +0.0f, +0.0f,
                +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +0.0f, +0.0f,
                +1.0f, +1.0f, -1.0f, +0.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +0.0f, +0.0f,
            }, 
            {
                2U, 1U, 0U, 0U, 3U, 2U, //0
                4U, 5U, 6U, 6U, 7U, 4U,  //4
                8U, 9U, 10U, 10U, 11U, 8U, //8
                14U, 13U, 12U, 12U, 15U, 14U, //12
            }
    ));

    auto testMesh2 = std::shared_ptr<Mesh>(
        new Mesh(
            {
                //Top Side           UV            COLOR                       NORMAL
                -1.0f, +1.0f, -1.0f, +0.0f, +0.0f, +1.0f, +1.0f, +1.0f, +1.0f, +0.0f, +1.0f, +0.0f,
                +1.0f, +1.0f, -1.0f, +1.0f, +0.0f, +1.0f, +1.0f, +1.0f, +1.0f, +0.0f, +1.0f, +0.0f,
                +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +0.0f, +1.0f, +0.0f,
                -1.0f, +1.0f, +1.0f, +0.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +0.0f, +1.0f, +0.0f,
                //Bottom Side        UV            COLOR                       NORMAL
                -1.0f, -1.0f, -1.0f, +0.0f, +0.0f, +1.0f, +1.0f, +1.0f, +1.0f, +0.0f, -1.0f, +0.0f,
                +1.0f, -1.0f, -1.0f, +1.0f, +0.0f, +1.0f, +1.0f, +1.0f, +1.0f, +0.0f, -1.0f, +0.0f,
                +1.0f, -1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +0.0f, -1.0f, +0.0f,
                -1.0f, -1.0f, +1.0f, +0.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +0.0f, -1.0f, +0.0f,
            },
            {
                2U, 1U, 0U, 0U, 3U, 2U, //0
                4U, 5U, 6U, 6U, 7U, 4U,  //4
            }
        )
    );

    auto testShader = std::make_shared<Shader>("assets\\shaders\\mapShader_vert.glsl", "assets\\shaders\\mapShader_frag.glsl");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glClearColor(0.1, 0.1, 0.3, 1.0);

    Uint32 lastTime = 0U;

    bool showDemoWindow = true;
    bool exit = false;
    SDL_Event event;
    while (!exit)
    {
        while (SDL_PollEvent(&event) != 0)
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            switch (event.type)
            {
                case SDL_QUIT:
                    exit = true;
                    break;
            }
        }

        Uint32 now = SDL_GetTicks();
        float deltaTime = (now - lastTime) / 1000.0f;
        lastTime = now;
        gTimer += deltaTime;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();
        ImGui::ShowDemoWindow(&showDemoWindow);
        ImGui::Render();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        testShader->bind();
        
        glm::mat4x4 modelMat = glm::translate(glm::vec3(0.0f, 0.0f, -4.0f))
             * glm::rotate(gTimer, glm::vec3(1.0f, 1.0f, 0.0f));
        
        glUniformMatrix4fv(testShader->getUniformLoc("uModelMat"), 1, GL_FALSE, &modelMat[0][0]);
        glUniformMatrix4fv(testShader->getUniformLoc("uViewProjMat"), 1, GL_FALSE, &camera->getViewProjectionMatrix()[0][0]);

        if (GLenum ass = glGetError(); ass != GL_NO_ERROR)
        {
            std::cout << "Error! " << ass << std::endl;
        }
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, testTexture->getID());
        glUniform1i(testShader->getUniformLoc("uTexture"), 0);

        testMesh->bind();
        glDrawElements(GL_TRIANGLES, testMesh->getIndexCount(), GL_UNSIGNED_SHORT, 0);
        testMesh2->bind();
        glBindTexture(GL_TEXTURE_2D, testTexture2->getID());
        glDrawElements(GL_TRIANGLES, testMesh2->getIndexCount(), GL_UNSIGNED_SHORT, 0);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(window);
    }
}