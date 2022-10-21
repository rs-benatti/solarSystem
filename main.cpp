// ----------------------------------------------------------------------------
// main.cpp
//
//  Created on: 24 Jul 2020
//      Author: Kiwon Um
//        Mail: kiwon.um@telecom-paris.fr
//
// Description: IGR201 Practical; OpenGL and Shaders (DO NOT distribute!)
//
// Copyright 2020-2022 Kiwon Um
//
// The copyright to the computer program(s) herein is the property of Kiwon Um,
// Telecom Paris, France. The program(s) may be used and/or copied only with
// the written permission of Kiwon Um or in accordance with the terms and
// conditions stipulated in the agreement/contract under which the program(s)
// have been supplied.
// ----------------------------------------------------------------------------

#define _USE_MATH_DEFINES
#define PI 3.14159265

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <glm/gtx/transform.hpp>
#include "glm/gtc/matrix_transform.hpp"

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <memory>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// constants
const static float kSizeSun = 1;
const static float kSizeEarth = 0.5;
const static float kSizeMoon = 0.25;
const static float kRadOrbitEarth = 10;
const static float kRadOrbitMoon = 2;

// Window parameters
GLFWwindow* g_window = nullptr;

// GPU objects
GLuint g_program = 0; // A GPU program contains at least a vertex shader and a fragment shader

// OpenGL identifiers
GLuint g_vao = 0;
GLuint g_posVbo = 0;
GLuint g_ibo = 0;

GLuint g_colvao = 0;
GLuint g_colVbo = 0;


// All vertex positions packed in one array [x0, y0, z0, x1, y1, z1, ...]
std::vector<float> g_vertexPositions;
// All triangle indices packed in one array [v00, v01, v02, v10, v11, v12, ...] with vij the index of j-th vertex of the i-th triangle
std::vector<unsigned int> g_triangleIndices;
std::vector<float> g_colors;

glm::mat4 viewMatrix;
glm::mat4 projMatrix;
glm::mat4 M;

void loadShader(GLuint program, GLenum type, const std::string& shaderFilename);
GLuint loadTextureFromFileToGPU(const std::string& filename);
GLuint g_earthTexID = 0;
GLuint g_sunTexID = 0;
GLuint g_moonTexID = 0;

// Basic camera model
class Camera {
public:
    inline float getFov() const { return m_fov; }
    inline void setFoV(const float f) { m_fov = f; }
    inline float getAspectRatio() const { return m_aspectRatio; }
    inline void setAspectRatio(const float a) { m_aspectRatio = a; }
    inline float getNear() const { return m_near; }
    inline void setNear(const float n) { m_near = n; }
    inline float getFar() const { return m_far; }
    inline void setFar(const float n) { m_far = n; }
    inline void setPosition(const glm::vec3& p) { m_pos = p; }
    inline glm::vec3 getPosition() { return m_pos; }

    inline glm::mat4 computeViewMatrix() const {
        return glm::lookAt(m_pos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    }

    // Returns the projection matrix stemming from the camera intrinsic parameter.
    inline glm::mat4 computeProjectionMatrix() const {
        return glm::perspective(glm::radians(m_fov), m_aspectRatio, m_near, m_far);
        //return glm::perspective(60.0f, m_aspectRatio, m_near, m_far);
    }

private:
    glm::vec3 m_pos = glm::vec3(0, 0, 0);
    float m_fov = 45.f;        // Field of view, in degrees
    float m_aspectRatio = 1.f; // Ratio between the width and the height of the image
    float m_near = 0.1f; // Distance before which geometry is excluded from the rasterization process
    float m_far = 100.f; // Distance after which the geometry is excluded from the rasterization process
};
Camera g_camera;
void initCamera();
void initGLFW();
void initOpenGL();
void initGPUprogram();
class Mesh {
public:
    GLuint g_earthTexID = 0;
    GLuint m_texCoordVbo = 0;
    std::vector<unsigned int> indices;
    void init(float radius) {

        //this->genCube();
        this->genSphere(radius);
        this->initGPUgeometry();
        //this->initGPUprogram();
    } // should properly set up the geometry buffer

    void initGPUgeometry() {
        // Create a single handle, vertex array object that contains attributes,
        // vertex buffer objects (e.g., vertex's position, normal, and color)
#ifdef _MY_OPENGL_IS_33_
        glGenVertexArrays(1, &this->m_vao); // If your system doesn't support OpenGL 4.5, you should use this instead of glCreateVertexArrays.
#else
        glCreateVertexArrays(1, &this->m_vao);
#endif
        glBindVertexArray(this->m_vao);

        // Generate a GPU buffer to store the positions of the vertices
        size_t vertexBufferSize = sizeof(float) * this->m_vertexPositions.size(); // Gather the size of the buffer from the CPU-side vector
#ifdef _MY_OPENGL_IS_33_
        glGenBuffers(1, &this->m_posVbo);
        glBindBuffer(GL_ARRAY_BUFFER, this->m_posVbo);
        glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, this->m_vertexPositions.data(), GL_DYNAMIC_READ);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
        glEnableVertexAttribArray(0);

        // Normals
        glGenBuffers(1, &this->m_normalVbo);
        glBindBuffer(GL_ARRAY_BUFFER, this->m_normalVbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * this->m_vertexNormals.size(), this->m_vertexNormals.data(), GL_DYNAMIC_READ);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
        glEnableVertexAttribArray(1);

        // UV map
        glGenBuffers(1, &this->m_texCoordVbo);
        glBindBuffer(GL_ARRAY_BUFFER, this->m_texCoordVbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * this->m_vertexTexCoords.size(), this->m_vertexTexCoords.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
        glEnableVertexAttribArray(2);

#else
        glCreateBuffers(1, &this->m_posVbo);
        glBindBuffer(GL_ARRAY_BUFFER, this->m_posVbo);
        glNamedBufferStorage(this->m_posVbo, vertexBufferSize, this->m_vertexPositions.data(), GL_DYNAMIC_STORAGE_BIT); // Create a data storage on the GPU and fill it from a CPU array
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
        glEnableVertexAttribArray(0);
#endif

        // Same for an index buffer object that stores the list of indices of the
        // triangles forming the mesh
        size_t indexBufferSize = sizeof(unsigned int) * this->indices.size();
#ifdef _MY_OPENGL_IS_33_
        glGenBuffers(1, &this->m_ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, this->indices.data(), GL_DYNAMIC_READ);
#else
        glCreateBuffers(1, &g_ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ibo);
        glNamedBufferStorage(g_ibo, indexBufferSize, this->indices.data(), GL_DYNAMIC_STORAGE_BIT);
#endif

        glBindVertexArray(0); // deactivate the VAO for now, will be activated again when rendering
    }

    std::shared_ptr<Mesh> genSphere(const float radius, const size_t resolution = 24, int texFlag = 1) {

        // phiValues that range from 0 up to 360
        // tetaValues that range from 0 up to 180
        float step = 2 * PI / (resolution - 1);
        float x = 0;
        float y = 0;
        float z = 0;
        unsigned int vertexCounter = 0;
        float textureCoefficient = 1.0f / (resolution - 1);
        for (int i = 0; i < resolution; i++) {
            float phi = PI - (i * step);

            for (int j = 0; j < resolution; j++) {

                float theta = PI - (j * step / 2);
                x = (radius * sin(theta) * cos(phi));
                z = radius * sin(phi) * sin(theta);
                y = radius * cos(theta);
                this->m_vertexNormals.push_back(x);
                this->m_vertexNormals.push_back(y);
                this->m_vertexNormals.push_back(z);
                if (vertexCounter + resolution < resolution * resolution) {
                    this->indices.push_back(vertexCounter);
                    this->indices.push_back(vertexCounter + resolution);
                    this->indices.push_back(vertexCounter + resolution + 1);
                    this->indices.push_back(vertexCounter);
                    this->indices.push_back(vertexCounter + resolution + 1);
                    this->indices.push_back(vertexCounter + 1);

                }
                else {
                    this->indices.push_back(vertexCounter);
                    this->indices.push_back(resolution * resolution);
                    this->indices.push_back(resolution * resolution);
                }

                this->m_vertexPositions.push_back(x);
                this->m_vertexPositions.push_back(y);
                this->m_vertexPositions.push_back(z);

                this->m_vertexTexCoords.push_back(i * textureCoefficient);
                this->m_vertexTexCoords.push_back(1 - j * textureCoefficient);
                //std::cout << x << " " << y << " " << z << std::endl;

                vertexCounter = vertexCounter + 1;
            }

        }

        return NULL;
    }

    void render(glm::mat4 transformationMatrix, GLuint texID) {
        glActiveTexture(GL_TEXTURE0); // activate texture unit 0
        glBindTexture(GL_TEXTURE_2D, texID);
        glUniform1i(texID, 0);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glUniformMatrix4fv(glGetUniformLocation(g_program, "transformationMatrix"), 1, GL_FALSE, glm::value_ptr(transformationMatrix));
        glUniformMatrix4fv(glGetUniformLocation(g_program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
        glUniformMatrix4fv(glGetUniformLocation(g_program, "M"), 1, GL_FALSE, glm::value_ptr(M));
        glBindVertexArray(this->get_m_vao());     // activate the VAO storing geometry data
        glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0); // Call for rendering: stream the current GPU geometry through the current GPU program
    }

    GLuint get_m_vao() {
        return this->m_vao;
    }

    // ...
private:
    std::vector<float> m_vertexPositions;
    std::vector<float> m_vertexNormals;
    std::vector<float> m_vertexTexCoords;
    GLuint m_vao = 0;
    GLuint m_posVbo = 0;
    GLuint m_normalVbo = 0;
    GLuint m_ibo = 0;

    // ...
};


GLuint loadTextureFromFileToGPU(const std::string& filename) {
    int width, height, numComponents;
    // Loading the image in CPU memory using stb_image
    unsigned char* data = stbi_load(
        filename.c_str(),
        &width, &height,
        &numComponents, // 1 for a 8 bit grey-scale image, 3 for 24bits RGB image, 4 for 32bits RGBA image
        0);

    GLuint texID;
    glGenTextures(1, &texID); // generate an OpenGL texture container
    glBindTexture(GL_TEXTURE_2D, texID); // activate the texture
    // Setup the texture filtering option and repeat mode; check www.opengl.org for details.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Fill the GPU texture with the data stored in the CPU image
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

    // Free useless CPU memory
    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0); // unbind the texture

    return texID;
}

// Executed each time the window is resized. Adjust the aspect ratio and the rendering viewport to the current window.
void windowSizeCallback(GLFWwindow* window, int width, int height) {
    g_camera.setAspectRatio(static_cast<float>(width) / static_cast<float>(height));
    glViewport(0, 0, (GLint)width, (GLint)height); // Dimension of the rendering region in the window
}

// Executed each time a key is entered.
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS && key == GLFW_KEY_W) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else if (action == GLFW_PRESS && key == GLFW_KEY_F) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    else if (action == GLFW_PRESS && (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q)) {
        glfwSetWindowShouldClose(window, true); // Closes the application if the escape key is pressed
    }
}

void errorCallback(int error, const char* desc) {
    std::cout << "Error " << error << ": " << desc << std::endl;
}

void initGLFW() {
    glfwSetErrorCallback(errorCallback);

    // Initialize GLFW, the library responsible for window management
    if (!glfwInit()) {
        std::cerr << "ERROR: Failed to init GLFW" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // Before creating the window, set some option flags
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    // Create the window
    g_window = glfwCreateWindow(
        1024, 768,
        "Interactive 3D Applications (OpenGL) - Simple Solar System",
        nullptr, nullptr);
    if (!g_window) {
        std::cerr << "ERROR: Failed to open window" << std::endl;
        glfwTerminate();
        std::exit(EXIT_FAILURE);
    }

    // Load the OpenGL context in the GLFW window using GLAD OpenGL wrangler
    glfwMakeContextCurrent(g_window);
    glfwSetWindowSizeCallback(g_window, windowSizeCallback);
    glfwSetKeyCallback(g_window, keyCallback);
}

void initOpenGL() {
    // Load extensions for modern OpenGL
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "ERROR: Failed to initialize OpenGL context" << std::endl;
        glfwTerminate();
        std::exit(EXIT_FAILURE);
    }

    glCullFace(GL_BACK); // Specifies the faces to cull (here the ones pointing away from the camera)
    glEnable(GL_CULL_FACE); // Enables face culling (based on the orientation defined by the CW/CCW enumeration).
    glDepthFunc(GL_LESS);   // Specify the depth test for the z-buffer
    glEnable(GL_DEPTH_TEST);      // Enable the z-buffer test in the rasterization
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f); // specify the background color, used any time the framebuffer is cleared
}

// Loads the content of an ASCII file in a standard C++ string
std::string file2String(const std::string& filename) {
    std::ifstream t(filename.c_str());
    std::stringstream buffer;
    buffer << t.rdbuf();
    return buffer.str();
}

// Loads and compile a shader, before attaching it to a program
void loadShader(GLuint program, GLenum type, const std::string& shaderFilename) {
    GLuint shader = glCreateShader(type); // Create the shader, e.g., a vertex shader to be applied to every single vertex of a mesh
    std::string shaderSourceString = file2String(shaderFilename); // Loads the shader source from a file to a C++ string
    const GLchar* shaderSource = (const GLchar*)shaderSourceString.c_str(); // Interface the C++ string through a C pointer
    glShaderSource(shader, 1, &shaderSource, NULL); // load the vertex shader code
    glCompileShader(shader);
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR in compiling " << shaderFilename << "\n\t" << infoLog << std::endl;
    }
    glAttachShader(program, shader);
    glDeleteShader(shader);
}

void initGPUprogram() {
    g_program = glCreateProgram(); // Create a GPU program, i.e., two central shaders of the graphics pipeline
    loadShader(g_program, GL_VERTEX_SHADER, "vertexShader.glsl");
    loadShader(g_program, GL_FRAGMENT_SHADER, "fragmentShader.glsl");
    glLinkProgram(g_program); // The main GPU program is ready to be handle streams of polygons

    glUseProgram(g_program);
}

void initCamera() {
    int width, height;
    glfwGetWindowSize(g_window, &width, &height);
    g_camera.setAspectRatio(static_cast<float>(width) / static_cast<float>(height));

    g_camera.setPosition(glm::vec3(0.0, 8.0, 30.0));
    g_camera.setNear(0.1);
    g_camera.setFar(80.1);
}

void init() {
    initGLFW();
    initOpenGL();
    //initCPUgeometry();
    initGPUprogram();
    //initGPUgeometry();
    initCamera();
    // The lines above are commented cause I (R. Benatti) deleted their codes just to clean a bit
}

void clear() {
    glDeleteProgram(g_program);
    glfwDestroyWindow(g_window);
    glfwTerminate();
}

/*
// The main rendering call
void render(GLuint render_vao, size_t indicesSize, glm::mat4 transformationMatrix, GLuint texID) {
  glActiveTexture(GL_TEXTURE0); // activate texture unit 0
  glBindTexture(GL_TEXTURE_2D, texID);
  glUniform1i(g_earthTexID, 0);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
  glUniformMatrix4fv(glGetUniformLocation(g_program, "transformationMatrix"), 1, GL_FALSE, glm::value_ptr(transformationMatrix));
  glBindVertexArray(render_vao);     // activate the VAO storing geometry data
  glDrawElements(GL_TRIANGLES, indicesSize, GL_UNSIGNED_INT, 0); // Call for rendering: stream the current GPU geometry through the current GPU program
}
*/

void update(const float currentTimeInSec) {
}


int main(int argc, char** argv) {

    initGLFW();
    initOpenGL();
    initGPUprogram();
    g_earthTexID = loadTextureFromFileToGPU("media/earth.jpg");
    g_moonTexID = loadTextureFromFileToGPU("media/moon.jpg");
    g_sunTexID = loadTextureFromFileToGPU("media/sun.jpg");
    int radius = 1;

    Mesh* terra = new Mesh();
    terra->init(radius * kSizeEarth);

    Mesh* lua = new Mesh();
    lua->init(radius * kSizeMoon);

    Mesh* sol = new Mesh();
    sol->init(radius * kSizeSun);

    initCamera();


    //init(); // Your initialization code (user interface, OpenGL states, scene with geometry, material, lights, etc)
    viewMatrix = g_camera.computeViewMatrix();
    projMatrix = g_camera.computeProjectionMatrix();

    const float orbitPeriodTerra = 10.0f;
    const float spinPeriodTerra = orbitPeriodTerra / 2;
    const float omegaOrbitTerra = 360.0f / orbitPeriodTerra;
    const float omegaSpinTerra = 360.0f / spinPeriodTerra;

    const float orbitPeriodLua = spinPeriodTerra / 2.0f;
    const float spinPeriodLua = orbitPeriodLua;
    const float omegaOrbitLua = 360.0f / orbitPeriodLua;
    const float omegaSpinLua = omegaOrbitLua;

    while (!glfwWindowShouldClose(g_window)) {
        update(static_cast<float>(glfwGetTime()));
        //init(); // Your initialization code (user interface, OpenGL states, scene with geometry, material, lights, etc)
        viewMatrix = g_camera.computeViewMatrix();
        projMatrix = g_camera.computeProjectionMatrix();
        float currentTime = glfwGetTime();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Erase the color and z buffers.
        // theta = wt où w = 2*PI/period

        GLuint m_vao = sol->get_m_vao();

        //sol->render(transformationMatrix, g_earthTexID);
        M = glm::mat4(1.0);
        glm::mat4 transformationMatrix = projMatrix * viewMatrix * M;
        glUniform1i(glGetUniformLocation(g_program, "sunFlag"), 1);
        sol->render(transformationMatrix, g_sunTexID);

        m_vao = terra->get_m_vao();

        float spinAngleTerra = omegaSpinTerra * currentTime;
        float orbitAngleTerra = omegaOrbitTerra * currentTime;
        glm::vec4 center = glm::vec4(10.0f, 0.0f, 0.0f, 1.0f);
        glm::mat4 orbitMatrix = glm::rotate(glm::radians(orbitAngleTerra), glm::vec3(0.0f, 1.0f, 0.0f));
        center = orbitMatrix * center;
        glm::vec3 vec3CenterTerra = glm::vec3(center);
        /*
        transformationMatrix = glm::translate(projMatrix * viewMatrix, vec3CenterTerra);
        */
        M = glm::translate(vec3CenterTerra);
        //M = glm::translate(glm::vec3(10.0f, 0.0f, 0.0f));
        float spinAngleLua = omegaSpinLua * currentTime;
        glm::mat4 rotateMatrix = glm::rotate(glm::radians(spinAngleLua), glm::vec3(0.0f, 1.0f, 0.0f));
        rotateMatrix = rotateMatrix * glm::rotate(glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        M = M * rotateMatrix;

        transformationMatrix = projMatrix * viewMatrix * M;
        //terra->render(transformationMatrix, g_earthTexID);
        glUniform1i(glGetUniformLocation(g_program, "sunFlag"), 0);
        terra->render(transformationMatrix, g_earthTexID);


        float orbitAngleLua = omegaOrbitLua * currentTime;
        center = glm::vec4(2.0f, 0.0f, 0.0f, 1.0f);
        orbitMatrix = glm::rotate(glm::radians(orbitAngleLua), glm::vec3(0.0f, 1.0f, 0.0f));
        center = orbitMatrix * center;
        glm::vec3 vec3CenterLua = glm::vec3(center);
        vec3CenterLua = vec3CenterLua + vec3CenterTerra;
        m_vao = lua->get_m_vao();
        M = glm::translate(vec3CenterLua);

        rotateMatrix = glm::rotate(glm::radians(spinAngleTerra), glm::vec3(0.0f, 1.0f, 0.0f));
        rotateMatrix = rotateMatrix * glm::rotate(glm::radians(23.5f), glm::vec3(0.0f, 0.0f, 1.0f));
        M = M * rotateMatrix;

        transformationMatrix = projMatrix * viewMatrix * M;
        //glm::mat4 rotationMatrix = glm::rotate(1.0f , glm::vec3(0.0f, 1.0f, 0.0f));
        glUniform1i(glGetUniformLocation(g_program, "sunFlag"), 0);
        lua->render(transformationMatrix, g_moonTexID);

        const glm::vec3 camPosition = g_camera.getPosition();
        glUniform3f(glGetUniformLocation(g_program, "camPos"), camPosition[0], camPosition[1], camPosition[2]);

        glfwSwapBuffers(g_window);
        glfwPollEvents();
    }
    clear();
    return EXIT_SUCCESS;
}