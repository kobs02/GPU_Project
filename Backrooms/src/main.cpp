#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>
#include <vector>

// 콜백 함수 선언
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);
void renderScene(const Shader& shader);
void renderWall(const Shader& shader, glm::vec3 position, glm::vec3 rotationAxis, float rotationAngle, glm::vec3 scale);
void renderSquareLight(const Shader& shader, glm::vec3 position, glm::vec3 rotationAxis, float rotationAngle, glm::vec3 scale);
unsigned int loadCubemap(vector<std::string> faces);
void renderCube();
void renderQuad();

// 화면 크기 세팅
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 900;

// bloom
bool bloom = true;
bool bloomKeyPressed = false;
float exposure = 1.0f;

// 카메라
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

// 시간 측정용 변수
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// VAO 선언
unsigned int planeVAO;
unsigned wallVAO;
unsigned wallVBO = 0;
unsigned squareLightVAO;
unsigned squareLightVBO = 0;

// 텍스처
unsigned int floorTexture;
unsigned int wallTexture;
unsigned int ceilingTexture;
unsigned int cubeTexture;
unsigned int squareLightTexture;

unsigned int depthMap;

// 추가: bloom 관련 FBO, 텍스처
unsigned int glowFBO, glowTexture;
unsigned int pingpongFBO[2];
unsigned int pingpongColorbuffers[2];

unsigned int quadVAO = 0;
unsigned int quadVBO;

// 바닥 평면 정점 설정
float planeVertices[] = {
    // positions            // normals         // texcoords
     25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
    -25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
    -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

     25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
    -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
     25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
};

// 스카이박스 정점 설정
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



int main()
{
    // glfw 초기화 및 설정
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw 윈도우 생성
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
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

    // GLAD 초기화
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // OpenGL 전역 상태 설정
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // Shaders 로드
 // -------------------------
    Shader shadow_mapping_shader("shader/3.1.2.shadow_mapping.vs", "shader/3.1.2.shadow_mapping.fs");
    Shader simpleDepthShader("shader/3.1.2.shadow_mapping_depth.vs", "shader/3.1.2.shadow_mapping_depth.fs");
    Shader skyboxShader("shader/6.1.skybox.vs", "shader/6.1.skybox.fs");
    Shader lightingShader("shader/light.vs", "shader/light.fs");

    Shader shader("7.bloom.vs", "7.bloom.fs");
    Shader shaderLight("7.bloom.vs", "7.light_box.fs");
    Shader shaderBlur("7.blur.vs", "7.blur.fs");
    Shader shaderBloomFinal("7.bloom_final.vs", "7.bloom_final.fs");

    // 평면 VAO/VBO 설정 
    unsigned int planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);

    // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // 텍스처 로딩
    // -------------

    floorTexture = loadTexture("resources/textures/floor.png");
    wallTexture = loadTexture("resources/textures/wall.png");
    ceilingTexture = loadTexture("resources/textures/ceiling.png");
    cubeTexture = loadTexture("resources/textures/container.jpg");
    squareLightTexture = loadTexture("resources/textures/square_light.png");

    vector<std::string> faces
    {
        "resources/textures/skybox/right.jpg",
        "resources/textures/skybox/left.jpg",
        "resources/textures/skybox/top.jpg",
        "resources/textures/skybox/bottom.jpg",
        "resources/textures/skybox/front.jpg",
        "resources/textures/skybox/back.jpg"
    };
    unsigned int cubemapTexture = loadCubemap(faces);

    // shadow mapping용 깊이 텍스처 생성 및 FBO 설정
    // -----------------------s
    const unsigned int SHADOW_WIDTH = 4096, SHADOW_HEIGHT = 4096;
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);

    // create depth texture
    // unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // shader 설정
    // --------------------
    shadow_mapping_shader.use();
    shadow_mapping_shader.setInt("diffuseTexture", 0);
    shadow_mapping_shader.setInt("shadowMap", 1);
    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    // 광원 위치
    // -------------
    glm::vec3 lightPos(0.0f, 1.999f, 0.0f);

    // bloom code
    // HDR 렌더링용 floating-point FBO 생성
    unsigned int hdrFBO;
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

    // 2개의 floating point 컬러 버퍼 생성
    // - colorBuffers[0]: 일반 장면 렌더링용
    // - colorBuffers[1]: 밝은 픽셀 추출용 (bloom threshold)
    unsigned int colorBuffers[2];
    glGenTextures(2, colorBuffers);

    for (unsigned int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
        // HDR용: GL_RGBA16F (16bit float)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // blur 시 테두리 샘플링 방지 위해 CLAMP_TO_EDGE 사용
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // FBO에 텍스처 부착
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
    }

    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    // 어떤 컬러 버퍼들을 쓸지 명시 (두 개의 컬러 attachment 사용)
    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments); // MRT (Multiple Render Targets) 활성화

    // FBO가 완전하게 구성됐는지 확인
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // FBO 바인딩 해제

    // 블러를 위한 Ping-Pong FBO 및 텍스처 2개 생성
    unsigned int pingpongFBO[2];
    unsigned int pingpongColorbuffers[2];
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongColorbuffers);

    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL); // HDR 포맷 유지
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // blur artifacts 방지
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);

        // pingpong FBO는 깊이 버퍼 필요 없음
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
    }

    // shader configuration
    // --------------------
    shader.use();
    shader.setInt("diffuseTexture", 0);

    shaderBlur.use();
    shaderBlur.setInt("image", 0);
    shaderBloomFinal.use();
    shaderBloomFinal.setInt("scene", 0);
    shaderBloomFinal.setInt("bloomBlur", 1);


    // 렌더링 루프
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.0f, -0.0f, 0.0f, 0.7f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



        // 1. render depth of scene to texture (빛의 시점에서 깊이 맵 생성)
        // --------------------------------------------------------------
        glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrix;
        float near_plane = 1.0f, far_plane = 7.5f;
        lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        lightSpaceMatrix = lightProjection * lightView;

        // render scene from light's point of view
        simpleDepthShader.use();
        simpleDepthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        // glActiveTexture(GL_TEXTURE0);
        // glBindTexture(GL_TEXTURE_2D, floorTexture);

        renderScene(simpleDepthShader);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // reset viewport
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 2. render scene as normal using the generated depth/shadow map (카메라 시점에서 장면 렌더링) 
        // --------------------------------------------------------------

        glm::mat4 model = glm::mat4(1.0f);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        shadow_mapping_shader.use();
        shadow_mapping_shader.setMat4("projection", projection);
        shadow_mapping_shader.setMat4("view", view);

        // set light uniforms
        shadow_mapping_shader.setVec3("viewPos", camera.Position);
        shadow_mapping_shader.setVec3("lightPos", lightPos);
        shadow_mapping_shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);




        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMap);

        renderScene(shadow_mapping_shader);

        
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
        


        /*
        // create one large cube that acts as the floor
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0));
        model = glm::scale(model, glm::vec3(12.5f, 0.5f, 12.5f));
        shader.setMat4("model", model);
        */

        // finally show all the light sources as bright cubes
        shaderLight.use();
        shaderLight.setMat4("projection", projection);
        shaderLight.setMat4("view", view);

        /*
        
        // blur postprocessing
        // 1. render scene into floating point framebuffer
        // -----------------------------------------------
        // 씬을 floating-point FBO (hdrFBO)에 렌더링한다. 이건 HDR 지원 + bloom 처리를 위해서 필요
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        

        // 카메라 뷰/투영 행렬 계산
        projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        view = camera.GetViewMatrix();
        model = glm::mat4(1.0f);

        // 일반 object 렌더링용 쉐이더 사용
        shader.use();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);

        /*
        // 바닥 텍스처 설정
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodTexture);
        */
        /*
        // 광원 위치/색상 유니폼 설정
        for (unsigned int i = 0; i < lightPositions.size(); i++) {
            shader.setVec3("lights[" + std::to_string(i) + "].Position", lightPositions[i]);
            shader.setVec3("lights[" + std::to_string(i) + "].Color", lightColors[i]);
        }
        shader.setVec3("viewPos", camera.Position);

        // 바닥 큐브 렌더링
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0));
        model = glm::scale(model, glm::vec3(12.5f, 0.5f, 12.5f));
        shader.setMat4("model", model);
        renderCube();

        // 광원들을 작은 밝은 큐브로 렌더링 (light box)
        shaderLight.use();
        shaderLight.setMat4("projection", projection);
        shaderLight.setMat4("view", view);

        for (unsigned int i = 0; i < lightPositions.size(); i++) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(lightPositions[i]));
            model = glm::scale(model, glm::vec3(0.25f));
            shaderLight.setMat4("model", model);
            shaderLight.setVec3("lightColor", lightColors[i]);
            renderCube();
        }
        

        // hdrFBO 렌더링 완료 → 기본 프레임버퍼로 전환
        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        // 2. blur bright fragments with two-pass Gaussian Blur 
        // --------------------------------------------------
        // 밝은 부분만 뽑아와서 가우시안 블러 처리. horizontal-vertical 2-pass 방식 사용
        bool horizontal = true, first_iteration = true;
        unsigned int amount = 10; // 블러 반복 횟수
        shaderBlur.use();

        for (unsigned int i = 0; i < amount; i++) {
            // 현재 패스에서 사용할 FBO 선택
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);

            // 블러 방향 설정 (horizontal → vertical 번갈아)
            shaderBlur.setInt("horizontal", horizontal);

            // 첫 프레임이면 HDR FBO에서 추출한 밝기 텍스처 사용, 이후엔 이전 패스 결과 사용
            glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);

            renderQuad(); // fullscreen quad 렌더링으로 blur 적용

            horizontal = !horizontal;
            if (first_iteration)
                first_iteration = false;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // 블러 끝나고 FBO 해제


        // 3. 최종 합성 단계
        // ----------------------------------------------------------------
        // 화면에 출력할 때 bloom 효과와 tone mapping을 적용해서 최종 화면으로 출력
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shaderBloomFinal.use();

        // 0번 텍스처: 원래 장면 (HDR 색상)
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);

        // 1번 텍스처: 블러된 밝은 영역 (bloom 효과용)
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);

        // bloom 활성화 여부 및 노출(밝기) 조정
        shaderBloomFinal.setInt("bloom", bloom);
        shaderBloomFinal.setFloat("exposure", exposure);

        renderQuad(); // bloom 및 tone-mapped 결과를 스크린에 출력
        */


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

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(1, &planeVBO);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);

    glfwTerminate();
    return 0;
}

// 최종 장면 렌더링 함수
// --------------------
void renderScene(const Shader& shader)
{
    // 바닥
    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("model", model);
    glBindVertexArray(planeVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, floorTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // 천장
    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 3.0f, 0.0f));
    shader.setMat4("model", model);
    glBindVertexArray(planeVAO);
    glActiveTexture(GL_TEXTURE0);  
    glBindTexture(GL_TEXTURE_2D, ceilingTexture);  
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // 형광등
    renderSquareLight(shader, glm::vec3(0.0f, 1.999f, 2.5f), glm::vec3(1.0f, 0.0f, 0.0f), 90.0f, glm::vec3(0.5f));
    // renderSquareLight(shader, glm::vec3(0.0f, 1.999f, -10.0f), glm::vec3(1.0f, 0.0f, 0.0f), 90.0f, glm::vec3(0.5f));

    // 스폰지점 십자가 통로

    // 앞 방향 왼쪽벽(세로)
    renderWall(shader, glm::vec3(0.0f, -0.5f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(0.0f, -0.5f, -3.0f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(0.0f, -0.5f, -5.0f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, glm::vec3(1.5f));

    // 앞 방향 오른쪽벽(세로)
    renderWall(shader, glm::vec3(3.0f, -0.5f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(3.0f, -0.5f, -3.0f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(3.0f, -0.5f, -5.0f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, glm::vec3(1.5f));

    // 앞 방향 오른쪽 통로(가로)
    renderWall(shader, glm::vec3(3.0f, -0.5f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -180.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(6.0f, -0.5f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -180.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(9.0f, -0.5f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -180.0f, glm::vec3(1.5f));

    // 앞 방향 왼쪽통로(가로)
    renderWall(shader, glm::vec3(-3.0f, -0.5f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -180.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(-6.0f, -0.5f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -180.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(-9.0f, -0.5f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -180.0f, glm::vec3(1.5f));



    // 뒤 방향 오른쪽벽(세로)
    renderWall(shader, glm::vec3(0.0f, -0.5f, 6.0f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, glm::vec3(1.5f));

    renderWall(shader, glm::vec3(0.0f, -0.5f, 9.0f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(0.0f, -0.5f, 12.0f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, glm::vec3(1.5f));

    // 뒤 방향 왼쪽벽(세로)
    renderWall(shader, glm::vec3(3.0f, -0.5f, 6.0f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(3.0f, -0.5f, 9.0f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(3.0f, -0.5f, 12.0f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, glm::vec3(1.5f));

    // 뒤 방향 오른쪽 통로(가로)
    renderWall(shader, glm::vec3(-3.0f, -0.5f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -180.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(-6.0f, -0.5f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -180.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(-9.0f, -0.5f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -180.0f, glm::vec3(1.5f));

    // 뒤 방향 왼쪽 통로(가로)
    renderWall(shader, glm::vec3(3.0f, -0.5f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -180.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(6.0f, -0.5f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -180.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(9.0f, -0.5f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -180.0f, glm::vec3(1.5f));



    // 앞 방
    renderWall(shader, glm::vec3(3.0f, -0.5f, -5.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(6.0f, -0.5f, -5.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(9.0f, -0.5f, -5.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(12.0f, -0.5f, -5.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(15.0f, -0.5f, -5.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(18.0f, -0.5f, -5.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(21.0f, -0.5f, -5.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(23.5f, -0.5f, -5.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(23.5f, -0.5f, -8.0f), glm::vec3(0.0f, 1.0f, 0.0f), 270.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(23.5f, -0.5f, -11.0f), glm::vec3(0.0f, 1.0f, 0.0f), 270.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(23.5f, -0.5f, -14.0f), glm::vec3(0.0f, 1.0f, 0.0f), 270.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(23.5f, -0.5f, -17.0f), glm::vec3(0.0f, 1.0f, 0.0f), 270.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(23.5f, -0.5f, -20.0f), glm::vec3(0.0f, 1.0f, 0.0f), 270.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(23.5f, -0.5f, -22.0f), glm::vec3(0.0f, 1.0f, 0.0f), 270.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(23.5f, -0.5f, -23.5f), glm::vec3(0.0f, 1.0f, 0.0f), 270.0f, glm::vec3(1.5f));


    renderWall(shader, glm::vec3(-3.0f, -0.5f, -5.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(-6.0f, -0.5f, -5.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(-9.0f, -0.5f, -5.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(-12.0f, -0.5f, -5.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(-15.0f, -0.5f, -5.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(-18.0f, -0.5f, -5.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(-21.0f, -0.5f, -5.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(-23.5f, -0.5f, -5.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(-23.5f, -0.5f, -8.0f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(-23.5f, -0.5f, -11.0f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(-23.5f, -0.5f, -14.0f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(-23.5f, -0.5f, -17.0f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(-23.5f, -0.5f, -20.0f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(-23.5f, -0.5f, -22.0f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(-23.5f, -0.5f, -23.5f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, glm::vec3(1.5f));

    renderWall(shader, glm::vec3(-23.5f, -0.5f, -26.5f), glm::vec3(0.0f, 1.0f, 0.0f), 180.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(-21.0f, -0.5f, -26.5f), glm::vec3(0.0f, 1.0f, 0.0f), 180.0f, glm::vec3(1.5f));
    for(int i = 0; i < 15; i++) {
        renderWall(shader, glm::vec3(-18.0f + i * 3.0f, -0.5f, -26.5f), glm::vec3(0.0f, 1.0f, 0.0f), 180.0f, glm::vec3(1.5f));
	}
   

    //기둥 1
    renderWall(shader, glm::vec3(6.0f, -0.5f, -15.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(6.0f, -0.5f, -15.0f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(6.0f, -0.5f, -15.0f), glm::vec3(0.0f, 1.0f, 0.0f), 180.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(6.0f, -0.5f, -15.0f), glm::vec3(0.0f, 1.0f, 0.0f), 270.0f, glm::vec3(1.5f));

    //기둥 2
    renderWall(shader, glm::vec3(0.0f, -0.5f, -15.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(0.0f, -0.5f, -15.0f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(0.0f, -0.5f, -15.0f), glm::vec3(0.0f, 1.0f, 0.0f), 180.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(0.0f, -0.5f, -15.0f), glm::vec3(0.0f, 1.0f, 0.0f), 270.0f, glm::vec3(1.5f));
    
    //기둥 3
    renderWall(shader, glm::vec3(-6.0f, -0.5f, -15.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(-6.0f, -0.5f, -15.0f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(-6.0f, -0.5f, -15.0f), glm::vec3(0.0f, 1.0f, 0.0f), 180.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(-6.0f, -0.5f, -15.0f), glm::vec3(0.0f, 1.0f, 0.0f), 270.0f, glm::vec3(1.5f));

    //기둥 4
    renderWall(shader, glm::vec3(12.0f, -0.5f, -15.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(12.0f, -0.5f, -15.0f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(12.0f, -0.5f, -15.0f), glm::vec3(0.0f, 1.0f, 0.0f), 180.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(12.0f, -0.5f, -15.0f), glm::vec3(0.0f, 1.0f, 0.0f), 270.0f, glm::vec3(1.5f));

    //기둥 5
    renderWall(shader, glm::vec3(-12.0f, -0.5f, -15.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(-12.0f, -0.5f, -15.0f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(-12.0f, -0.5f, -15.0f), glm::vec3(0.0f, 1.0f, 0.0f), 180.0f, glm::vec3(1.5f));
    renderWall(shader, glm::vec3(-12.0f, -0.5f, -15.0f), glm::vec3(0.0f, 1.0f, 0.0f), 270.0f, glm::vec3(1.5f));
}

// 벽 렌더링 함수
void renderWall(const Shader& shader, glm::vec3 position, glm::vec3 rotationAxis, float rotationAngle, glm::vec3 scale)
{
    if (wallVAO == 0)
    {
        float wallVertices[] = {
            -1.0f, 0.0f, -1.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
             1.0f, 0.0f, -1.0f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f,
             1.0f, 2.0f, -1.0f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f,

            -1.0f, 0.0f, -1.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
             1.0f, 2.0f, -1.0f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f,
            -1.0f, 2.0f, -1.0f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f
        };

        glGenVertexArrays(1, &wallVAO);
        glGenBuffers(1, &wallVBO);
        glBindVertexArray(wallVAO);
        glBindBuffer(GL_ARRAY_BUFFER, wallVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(wallVertices), wallVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindVertexArray(0);
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, wallTexture); // 여기서 wallTexture 바인딩해줘야 함

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(rotationAngle), rotationAxis);
    model = glm::scale(model, scale);
    shader.setMat4("model", model);

    glBindVertexArray(wallVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void renderSquareLight(const Shader& shader, glm::vec3 position, glm::vec3 rotationAxis, float rotationAngle, glm::vec3 scale)
{
    // 형광등 평면의 VAO 초기화
    if (squareLightVAO == 0)
    {
        float squareLightVertices[] = {
            -1.0f, 0.0f, -1.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
             1.0f, 0.0f, -1.0f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f,
             1.0f, 2.0f, -1.0f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f,

            -1.0f, 0.0f, -1.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
             1.0f, 2.0f, -1.0f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f,
            -1.0f, 2.0f, -1.0f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f
        };

        glGenVertexArrays(1, &squareLightVAO);
        glGenBuffers(1, &squareLightVBO);
        glBindVertexArray(squareLightVAO);
        glBindBuffer(GL_ARRAY_BUFFER, squareLightVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(squareLightVertices), squareLightVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindVertexArray(0);
    }
    
    shader.setBool("useEmission", true);
    shader.setFloat("emissionStrength", 3.0f); // 발광 세기(2~8 정도 시각적 확인)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, squareLightTexture);
    shader.setInt("diffuseTexture", 0);
    
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(rotationAngle), rotationAxis);
    model = glm::scale(model, scale);
    shader.setMat4("model", model);

    glBindVertexArray(squareLightVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    shader.setBool("useEmission", false);
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
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

    camera.Position.y = 1.0f;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
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

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
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

unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
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

// renderCube() renders a 1x1 3D cube in NDC.
// -------------------------------------------------
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube()
{
    // initialize (if necessary)
    if (cubeVAO == 0)
    {
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
             // bottom face
             -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
              1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
              1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
              1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
             -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
             -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             // top face
             -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
              1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
              1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
              1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
             -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------

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
