#include "View.h"
#include <cstdio>
#include <cstdlib>
#include <vector>
using namespace std;
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "sgraph/GLScenegraphRenderer.h"
#include "VertexAttrib.h"

View::View()
{
}

View::~View()
{
}

void View::init(Callbacks *callbacks, map<string, util::PolygonMesh<VertexAttrib>> &meshes)
{

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    window = glfwCreateWindow(800, 800, "Hello GLFW: Per-vertex coloring", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwSetWindowUserPointer(window, (void *)callbacks);

    // using C++ functions as callbacks to a C-style library
    glfwSetKeyCallback(window,
                       [](GLFWwindow *window, int key, int scancode, int action, int mods)
                       {
                           reinterpret_cast<Callbacks *>(glfwGetWindowUserPointer(window))->onkey(key, scancode, action, mods);
                       });

    glfwSetWindowSizeCallback(window,
                              [](GLFWwindow *window, int width, int height)
                              {
                                  reinterpret_cast<Callbacks *>(glfwGetWindowUserPointer(window))->reshape(width, height);
                              });

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(1);

    // create the shader program
    program.createProgram(string("shaders/default.vert"),
                          string("shaders/default.frag"));
    // assuming it got created, get all the shader variables that it uses
    // so we can initialize them at some point
    // enable the shader program
    program.enable();
    shaderLocations = program.getAllShaderVariables();

    /* In the mesh, we have some attributes for each vertex. In the shader
     * we have variables for each vertex attribute. We have to provide a mapping
     * between attribute name in the mesh and corresponding shader variable
     name.
     *
     * This will allow us to use PolygonMesh with any shader program, without
     * assuming that the attribute names in the mesh and the names of
     * shader variables will be the same.

       We create such a shader variable -> vertex attribute mapping now
     */
    map<string, string> shaderVarsToVertexAttribs;

    shaderVarsToVertexAttribs["vPosition"] = "position";

    for (typename map<string, util::PolygonMesh<VertexAttrib>>::iterator it = meshes.begin();
         it != meshes.end();
         it++)
    {
        util::ObjectInstance *obj = new util::ObjectInstance(it->first);
        obj->initPolygonMesh(shaderLocations, shaderVarsToVertexAttribs, it->second);
        objects[it->first] = obj;
    }

    int window_width, window_height;
    glfwGetFramebufferSize(window, &window_width, &window_height);

    // prepare the projection matrix for perspective projection
    projection = glm::perspective(glm::radians(60.0f), (float)window_width / window_height, 0.1f, 10000.0f);
    glViewport(0, 0, window_width, window_height);
    frames = 0;
    time = glfwGetTime();
    renderer = new sgraph::GLScenegraphRenderer(modelview, objects, shaderLocations);
}

void drawCamera(glm::vec3 cameraPos, glm::vec3 cameraFront, glm::vec3 cameraUp)
{

    float fov = 45.0f;
    float aspectRatio = 800.0f / 600.0f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;
    // Draw the camera
    glm::vec3 cameraRight = glm::normalize(glm::cross(cameraFront, cameraUp));

    // Calculate the width and height of the near plane based on the field of view and aspect ratio
    float nearHeight = 2 * tan(glm::radians(fov / 2)) * nearPlane;
    float nearWidth = nearHeight * aspectRatio;

    // Calculate the width and height of the far plane based on the same formula
    float farHeight = 2 * tan(glm::radians(fov / 2)) * farPlane;
    float farWidth = farHeight * aspectRatio;

    // Calculate the corners of the near and far planes
    glm::vec3 nearCenter = cameraPos + cameraFront * nearPlane;
    glm::vec3 farCenter = cameraPos + cameraFront * farPlane;
    glm::vec3 nearTopLeft = nearCenter + (cameraUp * (nearHeight / 2)) - (cameraRight * (nearWidth / 2));
    glm::vec3 nearTopRight = nearCenter + (cameraUp * (nearHeight / 2)) + (cameraRight * (nearWidth / 2));
    glm::vec3 nearBottomLeft = nearCenter - (cameraUp * (nearHeight / 2)) - (cameraRight * (nearWidth / 2));
    glm::vec3 nearBottomRight = nearCenter - (cameraUp * (nearHeight / 2)) + (cameraRight * (nearWidth / 2));
    glm::vec3 farTopLeft = farCenter + (cameraUp * (farHeight / 2)) - (cameraRight * (farWidth / 2));
    glm::vec3 farTopRight = farCenter + (cameraUp * (farHeight / 2)) + (cameraRight * (farWidth / 2));
    glm::vec3 farBottomLeft = farCenter - (cameraUp * (farHeight / 2)) - (cameraRight * (farWidth / 2));
    glm::vec3 farBottomRight = farCenter - (cameraUp * (farHeight / 2)) + (cameraRight * (farWidth / 2));

    // Define the vertices of the frustum wireframe mesh
    std::vector<glm::vec3> vertices = {
        // Near plane
        nearTopLeft, nearTopRight, nearTopRight, nearBottomRight,
        nearBottomRight, nearBottomLeft, nearBottomLeft, nearTopLeft,
        // Far plane
        farTopLeft, farTopRight, farTopRight, farBottomRight,
        farBottomRight, farBottomLeft, farBottomLeft, farTopLeft,
        // Lines connecting near and far planes
        nearTopLeft, farTopLeft, nearTopRight, farTopRight,
        nearBottomRight, farBottomRight, nearBottomLeft, farBottomLeft};

    // Create a vertex buffer object (VBO) to store the vertices
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    // Create a vertex array object (VAO) and bind it
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Bind the VBO to the VAO and set up the vertex attributes
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);

    // Unbind the VBO and VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Draw the frustum wireframe mesh
    glBindVertexArray(vao);
    glLineWidth(1.0f); // Set the line width to 1 pixel
    glDrawArrays(GL_LINES, 0, vertices.size());

    // Delete the VBO and VAO
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}

void View::display(sgraph::IScenegraph *scenegraph, glm::vec3 cameraPos, glm::vec3 cameraFront, glm::vec3 cameraUp)
{

    program.enable();
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    modelview.push(glm::mat4(1.0));
    modelview.top() = modelview.top() * glm::lookAt(glm::vec3(100.0f, 150.0f, 150.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    // send projection matrix to GPU
    glUniformMatrix4fv(shaderLocations.getLocation("projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // draw scene graph here
    map<string, util::PolygonMesh<VertexAttrib>> meshes = scenegraph->getMeshes();

    // save the current modelview
    modelview.push(modelview.top());

    // Rotate the scene graph determined by the x rotate and y rotate values
    // These values are changed by the controller
    modelview.top() = modelview.top() * glm::rotate(glm::mat4(1.0), (float)xrotate * (3.141592f / 180.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
                      glm::rotate(glm::mat4(1.0), (float)yrotate * (3.141592f / 180.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    // Draw scene graph
    scenegraph->getRoot()->accept(renderer);
    drawCamera(cameraPos, cameraFront, cameraUp);
    modelview.pop();

    modelview.pop();
    glFlush();
    program.disable();

    glfwSwapBuffers(window);
    glfwPollEvents();
    frames++;
    double currenttime = glfwGetTime();
    if ((currenttime - time) > 1.0)
    {
        printf("Framerate: %2.0f\r", frames / (currenttime - time));
        frames = 0;
        time = currenttime;
    }
}

void View::displayFirstPerson(sgraph::IScenegraph *scenegraph, glm::vec3 cameraPos, glm::vec3 cameraFront, glm::vec3 cameraUp)
{
    float fov = 45.0f;
    float aspectRatio = 800.0f / 600.0f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;
    program.enable();
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    modelview.push(glm::mat4(1.0));
    modelview.top() = modelview.top() * glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

    // send projection matrix to GPU
    glUniformMatrix4fv(shaderLocations.getLocation("projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // draw scene graph here
    map<string, util::PolygonMesh<VertexAttrib>> meshes = scenegraph->getMeshes();

    // save the current modelview
    modelview.push(modelview.top());

    // Draw scene graph
    scenegraph->getRoot()->accept(renderer);
    drawCamera(cameraPos, cameraFront, cameraUp);
    modelview.pop();

    modelview.pop();
    glFlush();
    program.disable();

    glfwSwapBuffers(window);
    glfwPollEvents();
    frames++;
    double currenttime = glfwGetTime();
    if ((currenttime - time) > 1.0)
    {
        printf("Framerate: %2.0f\r", frames / (currenttime - time));
        frames = 0;
        time = currenttime;
    }
}

bool View::shouldWindowClose()
{
    return glfwWindowShouldClose(window);
}

void View::closeWindow()
{
    for (map<string, util::ObjectInstance *>::iterator it = objects.begin();
         it != objects.end();
         it++)
    {
        it->second->cleanup();
        delete it->second;
    }
    glfwDestroyWindow(window);

    glfwTerminate();
}
