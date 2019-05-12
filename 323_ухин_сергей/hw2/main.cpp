//internal includes
#include "common.h"
#include "ShaderProgram.h"

//External dependencies
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include <random>
#include "glm/ext.hpp"



#define STB_IMAGE_IMPLEMENTATION 1
#include "stb_image.h"

using namespace std;
glm::vec3 cameraPos   = glm::vec3(0.0f, 1.0f,  3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);
glm::vec3 lightPos(3.0f, 3.0f, 0.0f);

GLfloat deltaTime = 0.0f;   // Время, прошедшее между последним и текущим кадром
GLfloat lastFrame = 0.0f;   // Время вывода последнего кадра


GLint depthmap = 2;

static const GLsizei WIDTH = 640, HEIGHT = 480; //размеры окна
bool keys[1024];
GLfloat yaw   = -90.0f; // Yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right (due to how Eular angles work) so we initially rotate a bit to the left.
GLfloat pitch =   0.0f;
GLfloat lastX =  WIDTH  / 2.0;
GLfloat lastY =  HEIGHT / 2.0;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if(action == GLFW_PRESS)
        keys[key] = true;
    else if(action == GLFW_RELEASE)
        keys[key] = false;

    if (key == GLFW_KEY_1 ){
        depthmap = 2;
    }
    if (key == GLFW_KEY_2 ){
        depthmap = 1;
    }

}

void do_movement()
{
  // Camera controls
    GLfloat cameraSpeed = 5.0f * deltaTime;
    if(keys[GLFW_KEY_W])
        cameraPos += cameraSpeed * cameraFront;
    if(keys[GLFW_KEY_S])
        cameraPos -= cameraSpeed * cameraFront;
    if(keys[GLFW_KEY_A])
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if(keys[GLFW_KEY_D])
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}


bool firstMouse = true;
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    GLfloat xoffset = xpos - lastX;
    GLfloat yoffset = lastY - ypos; // Reversed since y-coordinates go from bottom to left
    lastX = xpos;
    lastY = ypos;

    GLfloat sensitivity = 0.05; // Change this value to your liking
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw   += xoffset;
    pitch += yoffset;

    // Make sure that when pitch is out of bounds, screen doesn't get flipped
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

int initGL()
{
    int res = 0;
    //грузим функции opengl через glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        return -1;
    }

    std::cout << "Vendor: "   << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "Version: "  << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL: "     << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    return 0;
}



int main(int argc, char** argv)
{
    if(!glfwInit())
        return -1;




    //запрашиваем контекст opengl версии 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); 
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); 
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE); 

    GLFWwindow*  window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL basic sample", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window); 
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetKeyCallback(window, key_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);

    if(initGL() != 0) 
        return -1;

    //Reset any OpenGL errors which could be present for some reason
    GLenum gl_error = glGetError();
    while (gl_error != GL_NO_ERROR)
        gl_error = glGetError();

    //создание шейдерной программы из двух файлов с исходниками шейдеров
    //используется класс-обертка ShaderProgram
    std::unordered_map<GLenum, std::string> shaders;
    shaders[GL_VERTEX_SHADER]   = "vertex.glsl";
    shaders[GL_FRAGMENT_SHADER] = "fragment.glsl";
    ShaderProgram program(shaders); GL_CHECK_ERRORS;

    shaders[GL_VERTEX_SHADER]   = "depthvertex.glsl";
    shaders[GL_FRAGMENT_SHADER] = "depthfragment.glsl";
    ShaderProgram depthprogram(shaders); GL_CHECK_ERRORS;


    shaders[GL_VERTEX_SHADER]   = "showDepthvertex.glsl";
    shaders[GL_FRAGMENT_SHADER] = "showDepthfragment.glsl";
    ShaderProgram showDepthprogram(shaders); GL_CHECK_ERRORS;

    glfwSwapInterval(1); // force 60 frames per second

    //Создаем и загружаем геометрию поверхности
    //
    GLuint g_vertexBufferObject;
    GLuint g_vertexArrayObject;
    GLuint planeVBO;
    GLuint planeVAO;
    GLuint tetrahedronVBO;
    GLuint tetrahedronVAO;

    {
        float vertices[] = {
            1.0, -1.0, -1.0,  0.0,  0.0, -1.0,  0.0,  0.0,
            -1.0, -1.0, -1.0,  0.0,  0.0, -1.0,  0.0,  1.0,
            -1.0,  1.0, -1.0,  0.0,  0.0, -1.0,  1.0,  1.0,
            -1.0,  1.0, -1.0,  0.0,  0.0, -1.0,  1.0,  1.0,
             1.0,  1.0, -1.0,  0.0,  0.0, -1.0,  1.0,  0.0,
             1.0, -1.0, -1.0,  0.0,  0.0, -1.0,  0.0,  0.0,

            -1.0, -1.0, 1.0,  0.0,  0.0,  1.0,  0.0,  0.0,
             1.0, -1.0, 1.0,  0.0,  0.0,  1.0,  0.0,  1.0,
             1.0,  1.0, 1.0,  0.0,  0.0,  1.0,  1.0,  1.0,
             1.0,  1.0, 1.0,  0.0,  0.0,  1.0,  1.0,  1.0,
            -1.0,  1.0, 1.0,  0.0,  0.0,  1.0,  1.0,  0.0,
            -1.0, -1.0, 1.0,  0.0,  0.0,  1.0,  0.0,  0.0,

            -1.0, -1.0, -1.0, -1.0,  0.0,  0.0,  0.0,  0.0,
            -1.0, -1.0,  1.0, -1.0,  0.0,  0.0,  0.0,  1.0,
            -1.0,  1.0,  1.0, -1.0,  0.0,  0.0,  1.0,  1.0,
            -1.0,  1.0,  1.0, -1.0,  0.0,  0.0,  1.0,  1.0,
            -1.0,  1.0, -1.0, -1.0,  0.0,  0.0,  1.0,  0.0,
            -1.0, -1.0, -1.0, -1.0,  0.0,  0.0,  0.0,  0.0,

             1.0, -1.0,  1.0,  1.0,  0.0,  0.0,  0.0,  0.0,
             1.0, -1.0, -1.0,  1.0,  0.0,  0.0,  0.0,  1.0,
             1.0,  1.0, -1.0,  1.0,  0.0,  0.0,  1.0,  1.0,
             1.0,  1.0, -1.0,  1.0,  0.0,  0.0,  1.0,  1.0,
             1.0,  1.0,  1.0,  1.0,  0.0,  0.0,  1.0,  0.0,
             1.0, -1.0,  1.0,  1.0,  0.0,  0.0,  0.0,  0.0,

            -1.0, -1.0, -1.0,  0.0, -1.0,  0.0,  0.0,  0.0,
             1.0, -1.0, -1.0,  0.0, -1.0,  0.0,  0.0,  1.0,
             1.0, -1.0,  1.0,  0.0, -1.0,  0.0,  1.0,  1.0,
             1.0, -1.0,  1.0,  0.0, -1.0,  0.0,  1.0,  1.0,
            -1.0, -1.0,  1.0,  0.0, -1.0,  0.0,  1.0,  0.0,
            -1.0, -1.0, -1.0,  0.0, -1.0,  0.0,  0.0,  0.0,

             1.0,  1.0, -1.0,  0.0,  1.0,  0.0,  0.0,  0.0,
            -1.0,  1.0, -1.0,  0.0,  1.0,  0.0,  0.0,  1.0,
            -1.0,  1.0,  1.0,  0.0,  1.0,  0.0,  1.0,  1.0,
            -1.0,  1.0,  1.0,  0.0,  1.0,  0.0,  1.0,  1.0,
             1.0,  1.0,  1.0,  0.0,  1.0,  0.0,  1.0,  0.0,
             1.0,  1.0, -1.0,  0.0,  1.0,  0.0,  0.0,  0.0
        };  

        float plane[] = {
            -1.0,  0.0,  1.0,  0.0,  1.0,  0.0,  0.0,  0.0,
             1.0,  0.0,  1.0,  0.0,  1.0,  0.0,  0.0,  1.0,
             1.0,  0.0, -1.0,  0.0,  1.0,  0.0,  1.0,  1.0,
             1.0,  0.0, -1.0,  0.0,  1.0,  0.0,  1.0,  1.0,
            -1.0,  0.0, -1.0,  0.0,  1.0,  0.0,  1.0,  0.0,
            -1.0,  0.0,  1.0,  0.0,  1.0,  0.0,  0.0,  0.0

        };
        float cube_root = std::pow(3, 1.0 / 3.0);
        float sqrt_2 = std::sqrt(2);
        float tetrahedron[] = { 
            
            1.0, 0.0,0.0,                           0.0, -1.0, 0.0,                                                         0.0, 1.0,
            -1.0 / 2.0, 0.0, cube_root / 2.0,       0.0, -1.0, 0.0,                                                         1.0, 0.5,
            -1.0 / 2.0, 0.0, -cube_root / 2.0,      0.0, -1.0, 0.0,                                                         0.0, 0.0,
            -1.0 / 2.0, 0.0,  cube_root / 2.0,      -2.0 * sqrt_2 / 3.0, 1.0 / 3.0, 0.0,                                    0.0, 1.0,
            0.0, sqrt_2, 0.0,                       -2.0 * sqrt_2 / 3.0, 1.0 / 3.0, 0.0,                                    1.0, 0.5,
            -1.0 / 2.0,    0.0, -cube_root / 2.0,   -2.0 * sqrt_2 / 3.0, 1.0 / 3.0, 0.0,                                    0.0, 0.0,
            1.0, 0.0, 0.0,                          sqrt_2 / 3.0, 1.0 / 3.0, sqrt_2 / cube_root,                            0.0, 1.0,
            0.0, sqrt_2,0.0,                        sqrt_2 / 3.0, 1.0 / 3.0, sqrt_2 / cube_root,                            1.0, 0.5,
            -1.0 / 2.0, 0.0, cube_root / 2.0,       sqrt_2 / 3.0, 1.0 / 3.0, sqrt_2 / cube_root,                            0.0, 0.0,
            -1.0 / 2.0, 0.0, -cube_root / 2.0,      sqrt_2 / 3.0, 1.0 / 3.0, -sqrt_2 / cube_root,                           0.0, 1.0,
            0.0, sqrt_2, 0.0,                       sqrt_2 / 3.0, 1.0 / 3.0, -sqrt_2 / cube_root,                           1.0, 0.5,
            1.0, 0.0, 0.0,                          sqrt_2 / 3.0, 1.0 / 3.0, -sqrt_2 / cube_root,                           0.0, 0.0,
            
        };

        
        //cube
        glGenVertexArrays(1, &g_vertexArrayObject);                                                    GL_CHECK_ERRORS;
        glBindVertexArray(g_vertexArrayObject);                                                        GL_CHECK_ERRORS;
        

        glGenBuffers(1, &g_vertexBufferObject);                                                        GL_CHECK_ERRORS;
        glBindBuffer(GL_ARRAY_BUFFER, g_vertexBufferObject);                                           GL_CHECK_ERRORS;
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);  GL_CHECK_ERRORS;

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3* sizeof(GLfloat)));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT,GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
        glEnableVertexAttribArray(2);  
        glBindVertexArray(0);

        //plane
        glGenVertexArrays(1, &planeVAO);                                                    GL_CHECK_ERRORS;
        glBindVertexArray(planeVAO);                                                        GL_CHECK_ERRORS;
        

        glGenBuffers(1, &planeVBO);                                                        GL_CHECK_ERRORS;
        glBindBuffer(GL_ARRAY_BUFFER, planeVBO);                                           GL_CHECK_ERRORS;
        glBufferData(GL_ARRAY_BUFFER, sizeof(plane), plane, GL_STATIC_DRAW);  GL_CHECK_ERRORS;

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3* sizeof(GLfloat)));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT,GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
        glEnableVertexAttribArray(2);  
        glBindVertexArray(0);

        //tetrahedron
        glGenVertexArrays(1, &tetrahedronVAO);                                                    GL_CHECK_ERRORS;
        glBindVertexArray(tetrahedronVAO);                                                        GL_CHECK_ERRORS;
        
        glGenBuffers(1, &tetrahedronVBO);                                                        GL_CHECK_ERRORS;
        glBindBuffer(GL_ARRAY_BUFFER, tetrahedronVBO);                                           GL_CHECK_ERRORS;
        glBufferData(GL_ARRAY_BUFFER, sizeof(tetrahedron), tetrahedron, GL_STATIC_DRAW);  GL_CHECK_ERRORS;

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT,GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
        glEnableVertexAttribArray(2); 

        glBindVertexArray(0);




    }

    //test
     float quad_vertices[] = {

        -1.0, -1.0, 0.0, 0.0, 0.0,
         1.0, -1.0, 0.0, 1.0, 0.0,
        -1.0,  1.0, 0.0, 0.0, 1.0,
        -1.0,  1.0, 0.0, 0.0, 1.0,
         1.0, -1.0, 0.0, 1.0, 0.0,
         1.0,  1.0, 0.0, 1.0, 1.0
    };
    unsigned int quadVBO, quadVAO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);
    glBindVertexArray(quadVAO);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    //depthmap
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
       SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    
    




    glm::vec3 cubePositions[] = {
        glm::vec3( -6.0f,  1.0f,  0.0f), 
        glm::vec3( 6.0f,  1.0f, 0.0f), 

    };

    GLuint texture1;
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);

    // Set the texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Set texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  // NOTE the GL_NEAREST Here! 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  // NOTE the GL_NEAREST Here! 
    int channel;
    
    int width, height;
    unsigned char* image = stbi_load("container.jpg", &width, &height, &channel, 0);
    cout << width << endl;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(image);
    glBindTexture(GL_TEXTURE_2D, 0);


    GLuint texture3;
    glGenTextures(1, &texture3);
    glBindTexture(GL_TEXTURE_2D, texture3);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Set texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  // NOTE the GL_NEAREST Here! 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  // NOTE the GL_NEAREST Here! 
    
    image = stbi_load("plane.jpg", &width, &height, 0, 0);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(image);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    
    /*for(int i = 0 ; i < 4 ; i++) {
        for (int j = 0; j < 4; ++j) {
            std::cout << trans[i][j] << ' ';
        }
        std::cout << std::endl;
    }*/
    glEnable(GL_DEPTH_TEST);


    //цикл обработки сообщений и отрисовки сцены каждый кадр
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        do_movement();
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        GLfloat timeValue = glfwGetTime();
        GLfloat curtime = glfwGetTime();


            //очищаем экран каждый кадр
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);              GL_CHECK_ERRORS;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); GL_CHECK_ERRORS;


        //float near_plane = 1.0f, far_plane = 7.5f;
        glm::mat4 lightProjection = glm::ortho(-3.0f, 3.0f, -3.0f, 3.0f, 0.2f, 7.0f);


        glm::mat4 lightView = glm::lookAt(glm::vec3(3.0f, 3.0f, 0.0f),
                                  glm::vec3( 0.0f, 0.0f,  0.0f),
                                  glm::vec3( 0.0f, 1.0f,  0.0f));

        glm::mat4 lightSpaceMatrix = lightProjection * lightView;
        depthprogram.StartUseShader();
            depthprogram.SetUniform("lightSpaceMatrix", lightSpaceMatrix);
            glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
            glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
            glClear(GL_DEPTH_BUFFER_BIT);
            glBindVertexArray(g_vertexArrayObject); GL_CHECK_ERRORS;
            glm::mat4 model(1);
            //1 cube
            model = glm::scale(glm::mat4(1), glm::vec3(0.2f));
            model = glm::translate(model, cubePositions[0]);
            depthprogram.SetUniform("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            //2 cube
            model = glm::scale(glm::mat4(1), glm::vec3(0.2f, 0.35f, 0.7f));
            model = glm::translate(model, cubePositions[1]);
            depthprogram.SetUniform("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);
            //plane
            glBindVertexArray(planeVAO);   
            model = glm::scale(glm::mat4(1), glm::vec3(2.6,2,2));
            model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            depthprogram.SetUniform("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);
            //tetrahedron
            glBindVertexArray(tetrahedronVAO); 
            model = glm::scale(glm::mat4(1), glm::vec3(0.3));
            model = glm::translate(model, glm::vec3(2 * cos(2 * curtime), 1.2f, 2 * sin(2 * curtime)));
            depthprogram.SetUniform("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 12);  
            glBindVertexArray(0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        depthprogram.StopUseShader();

        glViewport(0, 0, WIDTH, HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (depthmap == 1) {
            showDepthprogram.StartUseShader();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, depthMap);
            glBindVertexArray(quadVAO);
                glDrawArrays(GL_TRIANGLES, 0, 6);
            showDepthprogram.SetUniform("depthMap", 0);
            showDepthprogram.StopUseShader();
        } else {
            program.StartUseShader();                           GL_CHECK_ERRORS;
            program.SetUniform("lightSpaceMatrix", lightSpaceMatrix);
            program.SetUniform("isTetrahedron", 0);
            program.SetUniform("lightPos", lightPos);
            program.SetUniform("viewPos", cameraPos);
            glBindVertexArray(g_vertexArrayObject); GL_CHECK_ERRORS;

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, depthMap);
            program.SetUniform("shadowMap", 1);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture1);
            program.SetUniform("ourTexture1", 0);
            glm::mat4 model(1);
            glm::mat4 view;
            view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
            program.SetUniform("view", view);
            glm::mat4 projection(1);
            projection = glm::perspective(glm::radians(45.0f), (float) WIDTH / HEIGHT, 0.1f, 100.0f);
            program.SetUniform("projection", projection);
            //1 cube
            model = glm::scale(glm::mat4(1), glm::vec3(0.2f));
            model = glm::translate(model, cubePositions[0]);
            program.SetUniform("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            //2 cube
            model = glm::scale(glm::mat4(1), glm::vec3(0.2f, 0.35f, 0.7f));
            model = glm::translate(model, cubePositions[1]);
            program.SetUniform("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);
            //plane
            glBindVertexArray(planeVAO);   
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture3);
            program.SetUniform("ourTexture1", 0);
            model = glm::scale(glm::mat4(1), glm::vec3(2.6,2,2));
            model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            program.SetUniform("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);
            //tetrahedron
            glBindVertexArray(tetrahedronVAO); 
            program.SetUniform("isTetrahedron", 1);
            glm::vec3 objectColor = glm::vec3(1.0f, 0.5f, 0.31f);
            program.SetUniform("objectColor", objectColor);
            model = glm::scale(glm::mat4(1), glm::vec3(0.3));
            model = glm::translate(model, glm::vec3(2 * cos(2 * curtime), 1.2f, 2 * sin(2 * curtime)));
            program.SetUniform("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 12);
            glBindVertexArray(0);
            program.StopUseShader();

        }


        glfwSwapBuffers(window); 
    }

    //очищаем vboи vao перед закрытием программы
    //
    glDeleteVertexArrays(1, &g_vertexArrayObject);
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteVertexArrays(1, &tetrahedronVAO);
    glDeleteBuffers(1,      &g_vertexBufferObject);
    glDeleteBuffers(1,      &planeVBO);
    glDeleteBuffers(1,      &tetrahedronVBO);
    

    glfwTerminate();
    return 0;
}
