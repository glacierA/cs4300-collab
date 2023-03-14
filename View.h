#ifndef __VIEW_H__
#define __VIEW_H__

#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <ShaderProgram.h>
#include "sgraph/SGNodeVisitor.h"
#include "ObjectInstance.h"
#include "PolygonMesh.h"
#include "VertexAttrib.h"
#include "Callbacks.h"
#include "sgraph/IScenegraph.h"

#include <stack>
using namespace std;

class View
{
public:
    View();
    ~View();
    void init(Callbacks *callbacks, map<string, util::PolygonMesh<VertexAttrib>> &meshes);
    void display(sgraph::IScenegraph *scenegraph);
    void displayFirstPerson(sgraph::IScenegraph *scenegraph, glm::vec3 cameraPos, glm::vec3 cameraFront, glm::vec3 cameraUp);
    bool shouldWindowClose();
    void closeWindow();
    // void updateXrotate();
    double xrotate, yrotate, zrotate;
    GLFWwindow *window;

private:
    util::ShaderProgram program;
    util::ShaderLocationsVault shaderLocations;
    map<string, util::ObjectInstance *> objects;
    glm::mat4 projection;
    stack<glm::mat4> modelview;
    sgraph::SGNodeVisitor *renderer;
    int frames;
    double time;
    double prevXpos, prevYpos;
};

#endif