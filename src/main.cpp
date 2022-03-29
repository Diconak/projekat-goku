#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void processInput(GLFWwindow *window);

unsigned int loadCubemap(vector<std::string> faces);

unsigned int loadTexture(char const * path);

void renderQuad();

// settings
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 800;
bool hdr = true;
bool hdrKeyPressed = false;
bool bloom = true;
bool bloomKeyPressed = false;
float exposure = 1.0f;

// camera
Camera camera(glm::vec3(4.0f, 5.0f, 22.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;


int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Heavenly escape", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);


    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // Ucitavamo sejdere
    // -------------------------
    Shader ourShader("resources/shaders/2.model_lighting.vs", "resources/shaders/2.model_lighting.fs");
    Shader skyboxShader("resources/shaders/skybox.vs", "resources/shaders/skybox.fs");
    Shader travaShader("resources/shaders/trava.vs", "resources/shaders/trava.fs");
    Shader hdrShader("resources/shaders/hdr.vs","resources/shaders/hdr.fs");
    Shader bloomShader("resources/shaders/bloom.vs","resources/shaders/bloom.fs");

//***********************************************************************************
    float skyboxVertices[] = {
            // positions
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,  1.0f
    };

    // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    vector<std::string> faces
            {
                    FileSystem::getPath("resources/textures/skybox/front.jpg"),
                    FileSystem::getPath("resources/textures/skybox/back.jpg"),
                    FileSystem::getPath("resources/textures/skybox/top.jpg"),
                    FileSystem::getPath("resources/textures/skybox/bottom.jpg"),
                    FileSystem::getPath("resources/textures/skybox/left.jpg"),
                    FileSystem::getPath("resources/textures/skybox/right.jpg")
            };

    unsigned int cubemapTexture = loadCubemap(faces);
//******************************************************************************************
    // kvadrat na kojem ce da stoji tekstura travke koja ce da se doda na ostrvo
    float transparentVertices[] = {
            // positions         // texture Coords (swapped y coordinates because texture is flipped upside down)
            0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
            0.0f, -0.5f,  0.0f,  0.0f,  1.0f,
            1.0f, -0.5f,  0.0f,  1.0f,  1.0f,

            0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
            1.0f, -0.5f,  0.0f,  1.0f,  1.0f,
            1.0f,  0.5f,  0.0f,  1.0f,  0.0f
    };

    //VAO i VBO za kocku koja ce da ima teksturu trave
    unsigned int transparentVAO, transparentVBO;
    glGenVertexArrays(1, &transparentVAO);
    glGenBuffers(1, &transparentVBO);
    glBindVertexArray(transparentVAO);
    glBindBuffer(GL_ARRAY_BUFFER, transparentVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), transparentVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);

    stbi_set_flip_vertically_on_load(false);
    unsigned int travaTexture = loadTexture(FileSystem::getPath("resources/textures/grass.png").c_str());

    //Malo pozicije za travke
    vector<glm::vec3> vegetation
            {
                    //centralno ostrvo
                    glm::vec3(-2.5f,-0.4f,-1.7f),
                    glm::vec3(-3.2f,-0.4f,-1.6f),
                    glm::vec3(-3.2f,-0.4f,-2.0f),
                    glm::vec3(-2.7f,-0.4f,-2.1f),

                    //ostrvo pozadi
                    glm::vec3(0.1f,-0.4f,-12.5f),
                    glm::vec3(-0.5f,-0.4f,-12.4f),
                    glm::vec3(-0.8f,-0.4f,-12.5f),

                    //napred lepo
                    glm::vec3(-7.1f,2.1f,5.4f),
                    glm::vec3(-7.6f,2.1f,5.5f),
                    glm::vec3(-7.5f,2.1f,5.0f),
                    glm::vec3(-7.98f,2.1f,5.1f),
            };
//**********************************************************************************
    // definisanje svega sto treba za rad sa bloom i HDR
    unsigned int hdrFBO;
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

    unsigned int colorBuffers[2];
    glGenTextures(2, colorBuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // attach texture to framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
    }
    // create and attach depth buffer (renderbuffer)
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);
    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ping-pong-framebuffer for blurring
    unsigned int pingpongFBO[2];
    unsigned int pingpongColorbuffers[2];
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongColorbuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
        // also check if framebuffers are complete (no need for depth buffer)
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
    }


  //*************************************************************************************

    // load models
    // -----------
    // u ucitana ostrva
    Model ostrvo1("resources/objects/island/island.obj", true);
    ostrvo1.SetShaderTextureNamePrefix("material.");

   //ucitana drva
    Model drvo1("resources/objects/Tree/Tree.obj", true); // MALO DRVO
    drvo1.SetShaderTextureNamePrefix("material.");
    Model drvo2("resources/objects/Tree2/Tree.obj", true); // VELIKO DRVO
    drvo2.SetShaderTextureNamePrefix("material.");

    //ucitavamo cvece i zbunje
    Model zbun1("resources/objects/Round_Box_Hedge/10453_Round_Box_Hedge_v1_Iteration3.obj", true);
    zbun1.SetShaderTextureNamePrefix("material.");
    Model tulip("resources/objects/tulip_flower/12978_tulip_flower_l3.obj", true);
    tulip.SetShaderTextureNamePrefix("material.");

    // ostalo
    Model bench("resources/objects/ConcreteBench/ConcreteBench-L3.obj", true); //ostrvo1
    bench.SetShaderTextureNamePrefix("material.");
    Model bird("resources/objects/Bird/12214_Bird_v1max_l3.obj", true);
    bird.SetShaderTextureNamePrefix("material.");
    Model lampion("resources/objects/svetlo1/streetlight.obj", true);
    lampion.SetShaderTextureNamePrefix("material.");

    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    bloomShader.use();
    bloomShader.setInt("image", 0);

    hdrShader.use();
    hdrShader.setInt("hdrBuffer", 0);
    hdrShader.setInt("bloomBlur", 1);

    float lin = 0.14f;
    float kvad = 0.07f;
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);


        // render
        // ------
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //Sejder koji renderuje ostrvo sa osnovnim bojama
        ourShader.use();

        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //directional
        ourShader.setVec3("dirLight.direction", -20.0f, -20.0f, 0.0f);
        ourShader.setVec3("dirLight.ambient", 0.06, 0.06, 0.06);
        ourShader.setVec3("dirLight.diffuse",  0.6f,0.2f,0.2);
        ourShader.setVec3("dirLight.specular", 0.1, 0.1, 0.1);

        // Pointlight's
            //1
        ourShader.setVec3("pointLight[0].position", glm::vec3(-1.05f,2.4f,1.7f));
        ourShader.setVec3("pointLight[0].ambient", glm::vec3(0.15, 0.15, 0.15));
        ourShader.setVec3("pointLight[0].diffuse", glm::vec3(1.5f,1.5f,1.1f));
        ourShader.setVec3("pointLight[0].specular", glm::vec3(0.15, 0.15, 0.15));
        ourShader.setFloat("pointLight[0].constant", 1.0f);
        ourShader.setFloat("pointLight[0].linear", lin);
        ourShader.setFloat("pointLight[0].quadratic", kvad);
            //2
        ourShader.setVec3("pointLight[1].position", glm::vec3(-1.70f,2.4f,-11.1f));
        ourShader.setVec3("pointLight[1].ambient", glm::vec3(0.15, 0.15, 0.15));
        ourShader.setVec3("pointLight[1].diffuse", glm::vec3(1.5f,1.5f,1.1f));
        ourShader.setVec3("pointLight[1].specular", glm::vec3(0.15, 0.15, 0.15));
        ourShader.setFloat("pointLight[1].constant", 1.0f);
        ourShader.setFloat("pointLight[1].linear", lin);
        ourShader.setFloat("pointLight[1].quadratic", kvad);
            //3
        ourShader.setVec3("pointLight[2].position", glm::vec3(-5.75f,4.85f,8.95f));
        ourShader.setVec3("pointLight[2].ambient", glm::vec3(0.15, 0.15, 0.15));
        ourShader.setVec3("pointLight[2].diffuse", glm::vec3(1.5f,1.5f,1.1f));
        ourShader.setVec3("pointLight[2].specular", glm::vec3(0.15, 0.15, 0.15));
        ourShader.setFloat("pointLight[2].constant", 1.0f);
        ourShader.setFloat("pointLight[2].linear", lin);
        ourShader.setFloat("pointLight[2].quadratic", kvad);
            //4
        ourShader.setVec3("pointLight[3].position", glm::vec3(7.7f,-0.4f,8.75f));
        ourShader.setVec3("pointLight[3].ambient", glm::vec3(0.15, 0.15, 0.15));
        ourShader.setVec3("pointLight[3].diffuse", glm::vec3(1.5f,1.5f,1.1f));
        ourShader.setVec3("pointLight[3].specular", glm::vec3(0.15, 0.15, 0.15));
        ourShader.setFloat("pointLight[3].constant", 1.0f);
        ourShader.setFloat("pointLight[3].linear", lin);
        ourShader.setFloat("pointLight[3].quadratic", kvad);



          ourShader.setVec3("viewPosition", camera.Position);
          ourShader.setFloat("material.shininess", 32.0f);
        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
                                                (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        //Enabling back face culling
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        //****************************************************************************************
        // island one CENTAR

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model,glm::vec3(0.0f,-3.0f,0.0f));
        model = glm::scale(model, glm::vec3(0.5f,0.5f,0.5f));
        ourShader.setMat4("model", model);
        ostrvo1.Draw(ourShader);

       // Lampion
        model = glm::mat4(1.0f);
        model = glm::translate(model,glm::vec3(-1.2f,-0.95f,1.4f));
        model = glm::scale(model, glm::vec3(1.2f,1.2f,1.2f));
        ourShader.setMat4("model", model);
        lampion.Draw(ourShader);

        // bench
        model = glm::mat4(1.0f);
        model = glm::translate(model,glm::vec3(1.0f,-1.0f,-2.5f));
        model = glm::scale(model, glm::vec3(0.02f,0.02f,0.02f));
        model = glm::rotate(model, glm::radians((float) -90.0), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians((float) -20.0), glm::vec3(0.0f, 0.0f, 1.0f));
        ourShader.setMat4("model", model);
        bench.Draw(ourShader);

        //zbun
        model = glm::mat4(1.0f);
        model = glm::translate(model,glm::vec3(1.5f,-1.0f,2.2f));
        model = glm::scale(model, glm::vec3(0.01f,0.01f,0.01f));
        model = glm::rotate(model, glm::radians((float) -90.0), glm::vec3(1.0f, 0.0f, 0.0f));
        ourShader.setMat4("model", model);
        zbun1.Draw(ourShader);

        //Veliko drvo
        model = glm::mat4(1.0f);
        model = glm::translate(model,glm::vec3(-2.5f,-1.0f,-1.8f));
        model = glm::scale(model, glm::vec3(1.0f,1.0f,1.0f));
        ourShader.setMat4("model", model);
        drvo2.Draw(ourShader);

        //Bird
        model = glm::mat4(1.0f);
        model = glm::translate(model,glm::vec3(1.9f,-0.35f,-2.0f));
        model = glm::scale(model, glm::vec3(0.05f,0.05f,0.05f));
        model = glm::rotate(model, glm::radians((float) -90.0), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians((float) 50.0), glm::vec3(0.0f, 0.0f, 1.0f));
        ourShader.setMat4("model", model);
        bird.Draw(ourShader);


        //***********************************************************************
        // island two POZADI
        model = glm::mat4(1.0f);
        model = glm::translate(model,glm::vec3(0.0f,-3.0f,-10.0f));
        model = glm::scale(model, glm::vec3(0.4f,0.5f,0.4f));
        ourShader.setMat4("model", model);
        ostrvo1.Draw(ourShader);

        //Veliko drvo
        model = glm::mat4(1.0f);
        model = glm::translate(model,glm::vec3(0.0f,-1.0f,-12.75f));
        model = glm::scale(model, glm::vec3(1.0f,1.0f,1.0f));
        ourShader.setMat4("model", model);
        drvo2.Draw(ourShader);

        //Lampion
        model = glm::mat4(1.0f);
        model = glm::translate(model,glm::vec3(-1.9f,-0.95f,-11.5f));
        model = glm::scale(model, glm::vec3(1.0f,1.2f,1.2f));
        ourShader.setMat4("model", model);
        lampion.Draw(ourShader);

        //ptica desno
        model = glm::mat4(1.0f);
        model = glm::translate(model,glm::vec3(1.75f,-0.93f,-8.5f));
        model = glm::scale(model, glm::vec3(0.05f,0.05f,0.05f));
        model = glm::rotate(model, glm::radians((float) -90.0), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians((float) 50.0), glm::vec3(0.0f, 0.0f, 1.0f));
        ourShader.setMat4("model", model);
        bird.Draw(ourShader);

        //ptica levo
        model = glm::mat4(1.0f);
        model = glm::translate(model,glm::vec3(-1.5f,-0.82f,-10.0f));
        model = glm::scale(model, glm::vec3(0.05f,0.05f,0.05f));
        model = glm::rotate(model, glm::radians((float) -90.0), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians((float) 110.0), glm::vec3(0.0f, 0.0f, 1.0f));
        ourShader.setMat4("model", model);
        bird.Draw(ourShader);

        //tulip 1
        model = glm::mat4(1.0f);
        model = glm::translate(model,glm::vec3(-1.8f,-0.95f,-8.4f));
        model = glm::scale(model, glm::vec3(0.08f,0.08f,0.08f));
        model = glm::rotate(model, glm::radians((float) -90.0), glm::vec3(1.0f, 0.0f, 0.0f));
        ourShader.setMat4("model", model);
        tulip.Draw(ourShader);


        //tulip 2
        model = glm::mat4(1.0f);
        model = glm::translate(model,glm::vec3(0.6f,-1.0f,-7.7f));
        model = glm::scale(model, glm::vec3(0.08f,0.08f,0.08f));
        model = glm::rotate(model, glm::radians((float) -90.0), glm::vec3(1.0f, 0.0f, 0.0f));
        ourShader.setMat4("model", model);
        tulip.Draw(ourShader);


        //tulip 3
        model = glm::mat4(1.0f);
        model = glm::translate(model,glm::vec3(1.6f,-1.0f,-11.8f));
        model = glm::scale(model, glm::vec3(0.08f,0.08f,0.08f));
        model = glm::rotate(model, glm::radians((float) -90.0), glm::vec3(1.0f, 0.0f, 0.0f));
        ourShader.setMat4("model", model);
        tulip.Draw(ourShader);

        //******************************************************************
        // island three OSTRVO NAPRED LEVO
        model = glm::mat4(1.0f);
        model = glm::translate(model,glm::vec3(-7.0f,-0.5f,7.0f));
        model = glm::scale(model, glm::vec3(0.4f,0.5f,0.4f));
        ourShader.setMat4("model", model);
        ostrvo1.Draw(ourShader);

        //donje drvo na ostrvu 3
        model = glm::mat4(1.0f);
        model = glm::translate(model,glm::vec3(-8.3f,1.5f,8.4f));
        model = glm::scale(model, glm::vec3(1.2f,1.2f,1.2f));
        ourShader.setMat4("model", model);
        drvo1.Draw(ourShader);

        //zbun dole
        model = glm::mat4(1.0f);
        model = glm::translate(model,glm::vec3(-7.4f,1.5f,8.9f));
        model = glm::scale(model, glm::vec3(0.01f,0.01f,0.01f));
        model = glm::rotate(model, glm::radians((float) -90.0), glm::vec3(1.0f, 0.0f, 0.0f));
        ourShader.setMat4("model", model);
        zbun1.Draw(ourShader);

        //  tulip dole
        model = glm::mat4(1.0f);
        model = glm::translate(model,glm::vec3(-5.9f,1.5f,8.9f));
        model = glm::scale(model, glm::vec3(0.08f,0.08f,0.08f));
        model = glm::rotate(model, glm::radians((float) -90.0), glm::vec3(1.0f, 0.0f, 0.0f));
        ourShader.setMat4("model", model);
        tulip.Draw(ourShader);

        //lampion
        model = glm::mat4(1.0f);
        model = glm::translate(model,glm::vec3(-5.45f,1.55f,8.7f));
        model = glm::scale(model, glm::vec3(1.2f,1.2f,1.2f));
        ourShader.setMat4("model", model);
        lampion.Draw(ourShader);

        //gornje drvo na ostrvu 3
        model = glm::mat4(1.0f);
        model = glm::translate(model,glm::vec3(-7.0f,1.5f,5.2f));
        model = glm::scale(model, glm::vec3(1.0f,1.0f,1.0));
        ourShader.setMat4("model", model);
        drvo1.Draw(ourShader);

        //zbun gore
        model = glm::mat4(1.0f);
        model = glm::translate(model,glm::vec3(-5.75f,1.5f,4.75f));
        model = glm::scale(model, glm::vec3(0.01f,0.01f,0.01f));
        model = glm::rotate(model, glm::radians((float) -90.0), glm::vec3(1.0f, 0.0f, 0.0f));
        ourShader.setMat4("model", model);
        zbun1.Draw(ourShader);

      //  tulip gore
        model = glm::mat4(1.0f);
        model = glm::translate(model,glm::vec3(-8.0f,1.5f,5.0f));
        model = glm::scale(model, glm::vec3(0.08f,0.08f,0.08f));
        model = glm::rotate(model, glm::radians((float) -90.0), glm::vec3(1.0f, 0.0f, 0.0f));
        ourShader.setMat4("model", model);
        tulip.Draw(ourShader);

        //***********************************************************
        // island four OSTRVO NAPRED DESNO
        model = glm::mat4(1.0f);
        model = glm::translate(model,glm::vec3(7.0f,-5.5f,7.0f));
        model = glm::scale(model, glm::vec3(0.4f,0.5f,0.4f));
        ourShader.setMat4("model", model);
        ostrvo1.Draw(ourShader);

        //Veliko drvo na ostrvu 4
        model = glm::mat4(1.0f);
        model = glm::translate(model,glm::vec3(8.0f,-3.5f,5.0f));
        model = glm::scale(model, glm::vec3(0.8f,0.8f,0.8f));
        ourShader.setMat4("model", model);
        drvo2.Draw(ourShader);

        //zbun gore
        model = glm::mat4(1.0f);
        model = glm::translate(model,glm::vec3(6.0f,-3.5f,5.2f));
        model = glm::scale(model, glm::vec3(0.01f,0.01f,0.01f));
        model = glm::rotate(model, glm::radians((float) -90.0), glm::vec3(1.0f, 0.0f, 0.0f));
        ourShader.setMat4("model", model);
        zbun1.Draw(ourShader);

        //malo drvo na ostrvi 4
        model = glm::mat4(1.0f);
        model = glm::translate(model,glm::vec3(5.5f,-3.5f,8.6f));
        model = glm::scale(model, glm::vec3(1.0f,1.0f,1.0f));
        ourShader.setMat4("model", model);
        drvo1.Draw(ourShader);

        //zbun dole

        model = glm::mat4(1.0f);
        model = glm::translate(model,glm::vec3(6.4f,-3.5f,8.8f));
        model = glm::scale(model, glm::vec3(0.01f,0.01f,0.01f));
        model = glm::rotate(model, glm::radians((float) -90.0), glm::vec3(1.0f, 0.0f, 0.0f));
        ourShader.setMat4("model", model);
        zbun1.Draw(ourShader);

        //lampion
        model = glm::mat4(1.0f);
        model = glm::translate(model,glm::vec3(8.2f,-3.4f,8.8f));
        model = glm::scale(model, glm::vec3(1.2f,1.2f,1.2f));
        ourShader.setMat4("model", model);
        lampion.Draw(ourShader);
        glDisable(GL_CULL_FACE);


        //*************************************************************************

        //**********************************************************************
        //Palimo sejder i postavljamo travi

        travaShader.use();
        travaShader.setMat4("projection", projection);
        travaShader.setMat4("view", view);

        glBindVertexArray(transparentVAO);
        glBindTexture(GL_TEXTURE_2D, travaTexture);
        for (unsigned int i = 0; i < vegetation.size(); i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, vegetation[i]);
            travaShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

       //*************************************************************************
        // draw skybox as last
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        //*********************************************
        //load pingpong
        bool horizontal = true, first_iteration = true;
        unsigned int amount = 10;
        bloomShader.use();
        for (unsigned int i = 0; i < amount; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
            bloomShader.setInt("horizontal", horizontal);
            glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);

            renderQuad();

            horizontal = !horizontal;
            if (first_iteration)
                first_iteration = false;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
       // **********************************************
        // load hdr
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        hdrShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
        hdrShader.setBool("hdr", hdr);
        hdrShader.setBool("bloom", bloom);
        hdrShader.setFloat("exposure", exposure);
        renderQuad();


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    //brisanje array i buffera koje ne koristimo vise
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVAO);
    glDeleteVertexArrays(1, &transparentVAO);
    glDeleteBuffers(1, &transparentVBO);
//    glDeleteVertexArrays(1, &cubeVAO);
//    glDeleteBuffers(1, &cubeVBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
                // positions        // texture Coords
                -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);


    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS && !hdrKeyPressed)
    {
        hdr = !hdr;
        hdrKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_RELEASE)
    {
        hdrKeyPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !bloomKeyPressed)
    {
        bloom = !bloom;
        bloomKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE)
    {
        bloomKeyPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        if (exposure > 0.0f)
            exposure -= 0.005f;
        else
            exposure = 0.0f;
    }
    else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        exposure += 0.005f;
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset*0.02, yoffset*0.02);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
}

unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}