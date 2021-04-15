//TODO: Set up quad rendering to use shaders and crap
//TODO: Add actual doc-style comments

#include "app.h"

#include <iostream>
#include <glew.h>

#include <gl/GL.h>
#include <gl/GLU.h>
#include <SDL_image.h>

#include <entt/entt.hpp>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl.h>

#include <glm/mat4x4.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>

#include <initializer_list>

#include "texture.h"
#include "shader.h"
#include "mesh.h"
#include "input.h"
#include "transform.h"
#include "components.h"

App::App() : gTimer(0.0f)
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
    IMG_Init(IMG_INIT_PNG);
    // SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    Input::init();

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

    auto testTexture = assets->getTexture("textures\\spacewall.png");
    auto testTexture2 = assets->getTexture("textures\\spacefloor.png");

    //pos, uv, col, norm
    //-z is in front of default camera
    auto cubeMesh = std::shared_ptr<Mesh>(
        new TriMesh (
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
                8U, 9U, 10U, 10U, 11U, 8U, //8
                14U, 13U, 12U, 12U, 15U, 14U, //12
                18U, 17U, 16U, 16U, 19U, 18U, //16
                20U, 21U, 22U, 22U, 23U, 20U,  //20
            }
    ));

    

    auto testShader = std::make_shared<Shader>("assets\\shaders\\mapShader_vert.glsl", "assets\\shaders\\mapShader_frag.glsl");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glClearColor(0.1, 0.1, 0.3, 1.0);

    entt::registry reg;

    struct Rotate {
        glm::vec3 axis;
        float angle;
        float delta;
    };

    auto frand = [](float l, float u){
        return (((float)rand() / RAND_MAX) * (u-l)) + l;
    };

    //Spawn random cubes
    for (int i = 0; i < 20; ++i) {
        const auto entity = reg.create();
        const float angle = frand(0.0, glm::pi<float>() * 2.0);
        const auto axis = glm::normalize(glm::vec3(frand(-1.0, 1.0), frand(-1.0, 1.0), frand(-1.0, 1.0)));
        reg.emplace<Transform>(entity, 
            glm::vec3(frand(-10.0, 10.0), frand(-5.0, 5.0), frand(-25.0, -10.0)), 
            glm::angleAxis(angle, axis), 1.0f);
        reg.emplace<Rotate>(entity, axis, angle, frand(-glm::pi<float>(), glm::pi<float>()));
            std::cout << "WAH" << std::endl;
        reg.emplace<MeshRenderComponent>(entity, cubeMesh, testShader, testTexture);
    }

    //Spawn camera
    auto cam_ent = reg.create();
    reg.emplace<CameraComponent>(cam_ent, 70.0f, 0.1f, 100.0f);
    reg.emplace<Transform>(cam_ent, glm::vec3(0.0, 0.0, 0.0), glm::angleAxis(0.0f, glm::vec3(0.0, 1.0, 0.0)));
    reg.emplace<Rotate>(cam_ent, glm::vec3(0.0f, 0.0f, 1.0f), 0.0f, 0.5f);

    Uint32 lastTime = 0U;

    bool showDemoWindow = true;
    while (Input::pollEvents())
    {
        Uint32 now = SDL_GetTicks();
        float deltaTime = (now - lastTime) / 1000.0f;
        lastTime = now;
        gTimer += deltaTime;

        auto rView = reg.view<Transform, Rotate>();
        rView.each([&](Transform &trans, Rotate &ro){
            ro.angle += ro.delta * deltaTime;
            trans.setRot(glm::angleAxis(ro.angle, ro.axis));
        });

        //Render
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();
        ImGui::ShowDemoWindow(&showDemoWindow);

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New")) {}
                if (ImGui::MenuItem("Open")) {}
                if (ImGui::BeginMenu("Open Recent")) {
                    ImGui::MenuItem("E1M1.ti");
                    ImGui::MenuItem("E255M9.ti");
                    ImGui::MenuItem("genlab.ti3");
                    ImGui::EndMenu();
                }
                if (ImGui::MenuItem("Save", "Ctrl+S")) {}
                if (ImGui::MenuItem("Save As", "Ctrl+Shift+S")) {}
                if (ImGui::MenuItem("Info", "Ctrl+I")) {

                }
                ImGui::EndMenu();
            }
            ImGui::Separator();
            if (ImGui::BeginMenu("Debug")) {
                if (ImGui::MenuItem("Mouse Info", "Ctrl+I")) {
                    ImGui::Begin("Mouse Debug");
                    ImGui::Text("Mouse Position: (%f, %f)", Input::getMousePosition().x, Input::getMousePosition().y);
                    ImGui::Text("Mouse Movement: (%f, %f)", Input::getMouseMovement().x, Input::getMouseMovement().y);
                    ImGui::Text("Mouse Button 1: Pressed? %s Down? %s", Input::getMouseButtonPressed(1) ? "Y" : "N", Input::getMouseButtonDown(1) ? "Y" : "N");
                    ImGui::Text("Mouse Button 2: Pressed? %s Down? %s", Input::getMouseButtonPressed(2) ? "Y" : "N", Input::getMouseButtonDown(2) ? "Y" : "N");
                    ImGui::Text("Mouse Button 3: Pressed? %s Down? %s", Input::getMouseButtonPressed(3) ? "Y" : "N", Input::getMouseButtonDown(3) ? "Y" : "N");
                    ImGui::End();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }


        ImGui::Render();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        CameraComponent::Update(reg);
        RenderMeshComponents(reg);
        
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(window);
    }
}