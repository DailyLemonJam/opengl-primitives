#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>

#define _USE_MATH_DEFINES
#include <math.h>

#include "shader_s.h"
#include "camera.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void process_input(GLFWwindow* window);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGL())
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    Shader shaderProgram("shader.vert", "shader.frag");

    // Vertices & indices
    float sphereRadius = 1.0f;
    unsigned int levels = 100; // amount of circles (min 1)
    unsigned int detailing = 500; // amount of vertices for each circle (min 3)
    unsigned int vertCount = (levels * detailing * 3) + 2 * 3; // vertCount = real vertices * 3 (x, y, z)
    unsigned int indCount = 2 * (detailing * 3) + (levels - 1) * (detailing * 2 * 3);
    float levelStep = (2 * sphereRadius / (levels + 1));
    std::cout << "vertCount: " << vertCount << std::endl;
    std::cout << "indCount: " << indCount << std::endl;

    float* vertices = new float[vertCount];
    unsigned int* indices = new unsigned int[indCount];

    // top vertex
    int counter = 0;
    vertices[counter++] = 0.0f;
    vertices[counter++] = 0.0f;
    vertices[counter++] = 0.0f;

    float distFromTopToCurLevel = levelStep;
    for (int l = 0; l < levels; l++)
    {
        float levelRadius = sqrt(pow(sphereRadius, 2) - pow(sphereRadius - distFromTopToCurLevel, 2));
        // create circle
        float angle = 0.0f;
        for (int v = 0; v < detailing; v++)
        {
            vertices[counter++] = levelRadius * cos(M_PI * 2 * angle / 360.0f); // x
            vertices[counter++] = vertices[1] - distFromTopToCurLevel; // y
            vertices[counter++] = levelRadius * sin(M_PI * 2 * angle / 360.0f); // z
            angle += 360.0f / detailing;
        }
        distFromTopToCurLevel += levelStep;
    }

    // bottom vertex
    vertices[vertCount - 3] = vertices[0];
    vertices[vertCount - 2] = vertices[1] - (2 * sphereRadius);
    vertices[vertCount - 1] = vertices[2];

    // filling indices
    // top vertex & top circle
    counter = 0;
    for (int i = 0; i < detailing; i++)
    {
        indices[counter++] = i + 1;
        if (i == detailing - 1)
        {
            indices[counter++] = 1;
        }
        else
        {
            indices[counter++] = i + 2;
        }
        indices[counter++] = 0;
    }
    // middle circles
    for (int l = 0; l < levels - 1; l++)
    {
        for (int i = 1; i < detailing + 1; i++)
        {
            // Tr 1
            indices[counter++] = l * detailing + i;
            indices[counter++] = (l + 1) * detailing + i;
            if (i == detailing)
            {
                indices[counter++] = (l + 1) * detailing + 1;
            }
            else
            {
                indices[counter++] = (l + 1) * detailing + i + 1;
            }
            // Tr 2
            indices[counter++] = l * detailing + i;
            if (i == detailing)
            {
                indices[counter++] = (l + 1) * detailing + 1;
                indices[counter++] = l * detailing + 1;
            }
            else
            {
                indices[counter++] = (l + 1) * detailing + i + 1;
                indices[counter++] = l * detailing + i + 1;
            }
        }
    }

    // bottom vertex & bottom circle
    for (int i = 0; i < detailing; i++)
    {
        indices[counter++] = (levels - 1) * detailing + i + 1;
        indices[counter++] = levels * detailing + 1;
        if (i == detailing - 1)
        {
            indices[counter++] = (levels - 1) * detailing + 2;
        }
        else
        {
            indices[counter++] = (levels - 1) * detailing + i + 2;
        }
    }

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertCount, vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indCount, indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0); // VBO
    glBindVertexArray(0); // VAO
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // EBO AFTER VAO

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        process_input(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shaderProgram.use();
        shaderProgram.setVec3("viewPos", camera.Position);
        shaderProgram.setFloat("material.shininess", 32.0f);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);
        shaderProgram.setMat4("projection", projection);
        shaderProgram.setMat4("view", view);
        shaderProgram.setMat4("model", model);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indCount, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    delete[] vertices;
    delete[] indices;

    glfwTerminate();
    return 0;
}

void process_input(GLFWwindow* window)
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
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

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
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
