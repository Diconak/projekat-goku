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

// settings
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(5.0f, 10.0f, 15.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
};



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
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
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

    // build and compile shaders
    // -------------------------
    Shader ourShader("resources/shaders/2.model_lighting.vs", "resources/shaders/2.model_lighting.fs");
    Shader skyboxShader("resources/shaders/skybox.vs", "resources/shaders/skybox.fs");
    Shader travaShader("resources/shaders/trava.vs", "resources/shaders/trava.fs");


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

    // load models
    // -----------
    // u ucitana ostrva
    Model ostrvo1("resources/objects/island/island.obj");
    ostrvo1.SetShaderTextureNamePrefix("material.");

   //ucitana drva
    Model drvo1("resources/objects/Tree/Tree.obj"); // MALO DRVO
    drvo1.SetShaderTextureNamePrefix("material.");
    Model drvo2("resources/objects/Tree2/Tree.obj"); // VELIKO DRVO
    drvo2.SetShaderTextureNamePrefix("material.");

    //ucitavamo cvece i zbunje
    Model zbun1("resources/objects/Round_Box_Hedge/10453_Round_Box_Hedge_v1_Iteration3.obj"); //ostrvo1
    zbun1.SetShaderTextureNamePrefix("material.");
    Model tulip("resources/objects/tulip_flower/12978_tulip_flower_l3.obj"); //ostrvo1
    tulip.SetShaderTextureNamePrefix("material.");

    // ostalo
    Model bench("resources/objects/ConcreteBench/ConcreteBench-L3.obj"); //ostrvo1
    bench.SetShaderTextureNamePrefix("material.");
    Model bird("resources/objects/Bird/12214_Bird_v1max_l3.obj"); //ostrvo1
    bird.SetShaderTextureNamePrefix("material.");


    PointLight pointLight ;
    pointLight.position = glm::vec3(0.0f, 4.0, 0.0);
    pointLight.ambient = glm::vec3(0.6, 0.6, 0.6);
    pointLight.diffuse = glm::vec3(0.6, 0.6, 0.6);
    pointLight.specular = glm::vec3(1.0, 1.0, 1.0);

    pointLight.constant = 1.0f;
    pointLight.linear = 0.09f;
    pointLight.quadratic = 0.032f;

    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

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

        //PRVO SE AKTIVIRA SEJDER
        ourShader.use();
        //TODO Dodaj ostala osvetljenja i sredi ovo direkciono svetlo da bi sredili ambijent slike
        //TODO Smanji difuzno svetlo za objekte jer ne trebaju da se presijavaju

        //directional
        ourShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.0f);
        ourShader.setVec3("dirLight.ambient", 0.2f, 0.2f, 0.2f);
        ourShader.setVec3("dirLight.diffuse", 0.2f, 0.2f, 0.2f);
        ourShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);

        // Pointlight

        pointLight.position = glm::vec3(0.0, 4.0f, 0.0f);
        ourShader.setVec3("pointLight.position", pointLight.position);
        ourShader.setVec3("pointLight.ambient", pointLight.ambient);
        ourShader.setVec3("pointLight.diffuse", pointLight.diffuse);
        ourShader.setVec3("pointLight.specular", pointLight.specular);
        ourShader.setFloat("pointLight.constant", pointLight.constant);
        ourShader.setFloat("pointLight.linear", pointLight.linear);
        ourShader.setFloat("pointLight.quadratic", pointLight.quadratic);
          ourShader.setVec3("viewPosition", camera.Position);
          ourShader.setFloat("material.shininess", 32.0f);
        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
                                                (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        //****************************************************************************************
        // island one CENTAR
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model,glm::vec3(0.0f,-3.0f,0.0f));
        model = glm::scale(model, glm::vec3(0.5f,0.5f,0.5f));
        ourShader.setMat4("model", model);
        ostrvo1.Draw(ourShader);

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
        model = glm::translate(model,glm::vec3(-5.4f,1.5f,8.9f));
        model = glm::scale(model, glm::vec3(0.08f,0.08f,0.08f));
        model = glm::rotate(model, glm::radians((float) -90.0), glm::vec3(1.0f, 0.0f, 0.0f));
        ourShader.setMat4("model", model);
        tulip.Draw(ourShader);

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


    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
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