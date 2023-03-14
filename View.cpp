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

void View::display(sgraph::IScenegraph *scenegraph)
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
