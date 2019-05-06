//internal includes
#include "common.h"
#include "ShaderProgram.h"

//External dependencies
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include <random>
#include <SOIL/SOIL.h>
#include "glm/ext.hpp"

glm::vec3 cameraPos   = glm::vec3(0.0f, 1.0f,  3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);


GLfloat deltaTime = 0.0f;   // Время, прошедшее между последним и текущим кадром
GLfloat lastFrame = 0.0f;   // Время вывода последнего кадра

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

    std::unordered_map<GLenum, std::string> planeshaders;
    shaders[GL_VERTEX_SHADER]   = "planevertex.glsl";
    shaders[GL_FRAGMENT_SHADER] = "planefragment.glsl";
    ShaderProgram planeprogram(shaders); GL_CHECK_ERRORS;


    
    std::unordered_map<GLenum, std::string> tetrahedronshaders;
    shaders[GL_VERTEX_SHADER]   = "tetrahedronvertex.glsl";
    shaders[GL_FRAGMENT_SHADER] = "tetrahedronfragment.glsl";
    ShaderProgram tetrahedronprogram(shaders); GL_CHECK_ERRORS
    

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
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
            0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
        };  

        float plane[] = {
            -0.5f, -0.5f, 0.0f,      0.0f, 0.0f,
            -0.5f, 0.5f, 0.0f,      0.0f, 1.0f,
            0.5f, -0.5f, 0.0f,      1.0f, 0.0f,
            -0.5f, 0.5f, 0.0f,       0.0f, 1.0f,
            0.5f, -0.5f, 0.0f,      1.0f, 0.0f,
            0.5f, 0.5f, 0.0f,      1.0f, 1.0f

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
            -1.0 / 2.0, 0.0,                        cube_root / 2.0, sqrt_2 / 3.0, 1.0 / 3.0, sqrt_2 / cube_root,           0.0, 0.0,
            -1.0 / 2.0, 0.0,                        -cube_root / 2.0, sqrt_2 / 3.0, 1.0 / 3.0, -sqrt_2 / cube_root,         0.0, 1.0,
            0.0, sqrt_2, 0.0,                       sqrt_2 / 3.0, 1.0 / 3.0, -sqrt_2 / cube_root,                           1.0, 0.5,
            1.0, 0.0, 0.0,                          sqrt_2 / 3.0, 1.0 / 3.0, -sqrt_2 / cube_root,                           0.0, 0.0,

        };

        

        glGenVertexArrays(1, &g_vertexArrayObject);                                                    GL_CHECK_ERRORS;
        glBindVertexArray(g_vertexArrayObject);                                                        GL_CHECK_ERRORS;
        

        glGenBuffers(1, &g_vertexBufferObject);                                                        GL_CHECK_ERRORS;
        glBindBuffer(GL_ARRAY_BUFFER, g_vertexBufferObject);                                           GL_CHECK_ERRORS;
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);  GL_CHECK_ERRORS;

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);
        // Атрибут с цветом
        //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3* sizeof(GLfloat)));
        //glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT,GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(2);  
        glBindVertexArray(0);


        glGenVertexArrays(1, &planeVAO);                                                    GL_CHECK_ERRORS;
        glBindVertexArray(planeVAO);                                                        GL_CHECK_ERRORS;
        

        glGenBuffers(1, &planeVBO);                                                        GL_CHECK_ERRORS;
        glBindBuffer(GL_ARRAY_BUFFER, planeVBO);                                           GL_CHECK_ERRORS;
        glBufferData(GL_ARRAY_BUFFER, sizeof(plane), plane, GL_STATIC_DRAW);  GL_CHECK_ERRORS;

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);
        // Атрибут с цветом
        //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3* sizeof(GLfloat)));
        //glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT,GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
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

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)3);
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);

    }


    glm::vec3 cubePositions[] = {
        glm::vec3( -2.0f,  0.5f,  0.0f), 
        glm::vec3( 2.0f,  0.5f, 0.0f), 

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
    unsigned char* image = SOIL_load_image("container.png", &width, &height, &channel, SOIL_LOAD_RGBA);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);

    SOIL_free_image_data(image);
    glBindTexture(GL_TEXTURE_2D, 0);


    GLuint texture3;
    glGenTextures(1, &texture3);
    glBindTexture(GL_TEXTURE_2D, texture3);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Set texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  // NOTE the GL_NEAREST Here! 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  // NOTE the GL_NEAREST Here! 
    
    image = SOIL_load_image("plane.jpg", &width, &height, 0, SOIL_LOAD_RGBA);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);

    SOIL_free_image_data(image);
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



            //очищаем экран каждый кадр
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);              GL_CHECK_ERRORS;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); GL_CHECK_ERRORS;

        program.StartUseShader();                           GL_CHECK_ERRORS;




        // очистка и заполнение экрана цветом
        //
        glViewport  (0, 0, WIDTH, HEIGHT);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear     (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // draw call
        //
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glBindVertexArray(g_vertexArrayObject); GL_CHECK_ERRORS;

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        program.SetUniform("ourTexture1", 0);

        GLfloat timeValue = glfwGetTime();

        glm::mat4 model(1);
        //model = glm::rotate(model, (GLfloat)glfwGetTime(), glm::vec3(0.5f, 1.0f, 0.0f)); // glm::radians
        //program.SetUniform("model", model);

        GLfloat radius = 2.0f;
        GLfloat camX = 0;
        GLfloat camZ = radius + 1;
        GLfloat camY = radius;
        glm::mat4 view;
        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        program.SetUniform("view", view);

        glm::mat4 projection(1);
        projection = glm::perspective(glm::radians(45.0f), (float) WIDTH / HEIGHT, 0.1f, 100.0f);
        program.SetUniform("projection", projection);



        //1 cube
        model = glm::scale(glm::mat4(1), glm::vec3(0.4f, 0.4f, 0.4f));
        model = glm::translate(model, cubePositions[0]);


        program.SetUniform("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        //2 cube
        model = glm::scale(glm::mat4(1), glm::vec3(0.4f, 0.7f, 1.4f));
        model = glm::translate(model, cubePositions[1]);


        program.SetUniform("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        

        glBindVertexArray(0);
        program.StopUseShader();

        //plane
        planeprogram.StartUseShader(); GL_CHECK_ERRORS;

        glBindVertexArray(planeVAO);

        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture3);
        planeprogram.SetUniform("ourTexture3", 0);
        

        model = glm::scale(glm::mat4(1), glm::vec3(6.0f, 20.0f, 4.0f));
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        planeprogram.SetUniform("model", model);

        planeprogram.SetUniform("view", view);

        projection = glm::perspective(glm::radians(45.0f), (float) WIDTH / HEIGHT, 0.1f, 100.0f);
        planeprogram.SetUniform("projection", projection);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        planeprogram.StopUseShader();


        //tetrahedron
        tetrahedronprogram.StartUseShader();

        glBindVertexArray(tetrahedronVAO);
        GLfloat curtime = glfwGetTime();
        model = glm::scale(glm::mat4(1), glm::vec3(0.3));
        model = glm::translate(model, glm::vec3(2 * sin(5 * curtime), 0.f, 2 * cos(5 * curtime)));
        tetrahedronprogram.SetUniform("model", model);


        tetrahedronprogram.SetUniform("view", view);

        projection = glm::perspective(glm::radians(45.0f), (float) WIDTH / HEIGHT, 0.1f, 100.0f);
        tetrahedronprogram.SetUniform("projection", projection);
        glDrawArrays(GL_TRIANGLES, 0, 12);

        tetrahedronprogram.StopUseShader();


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
