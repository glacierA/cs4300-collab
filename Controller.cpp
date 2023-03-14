#include "Controller.h"
#include "sgraph/IScenegraph.h"
#include "sgraph/Scenegraph.h"
#include "sgraph/GroupNode.h"
#include "sgraph/LeafNode.h"
#include "sgraph/ScaleTransform.h"
#include "sgraph/TextualRenderVisitor.h"
#include "ObjImporter.h"
using namespace sgraph;
#include <iostream>
using namespace std;
#include "sgraph/ScenegraphExporter.h"
#include "sgraph/ScenegraphImporter.h"

// Int values to be used for mouse movements and rotation math
int mouseX = 0;
int mouseY = 0;
int oldMouseX = 0;
int oldMouseY = 0;
int KeyPress = -1;
int shouldMove = 0;

Controller::Controller(Model &m, View &v)
{
    model = m;
    view = v;

    initScenegraph();
}

void Controller::initScenegraph()
{

    // read in the file of commands
    ifstream inFile("scenegraphmodels/scene.txt");
    // ifstream inFile("tryout.txt");
    sgraph::ScenegraphImporter importer;

    IScenegraph *scenegraph = importer.parse(inFile);
    // scenegraph->setMeshes(meshes);
    model.setScenegraph(scenegraph);
    cout << "Scenegraph made" << endl;
}

Controller::~Controller()
{
}

void Controller::run()
{
    sgraph::IScenegraph *scenegraph = model.getScenegraph();
    // Setup the textual render visitor
    sgraph::TextualRenderVisitor visitor = sgraph::TextualRenderVisitor();
    scenegraph->getRoot()->accept(&visitor);
    cout << visitor.getOutput() << endl;
    map<string, util::PolygonMesh<VertexAttrib>> meshes = scenegraph->getMeshes();
    view.init(this, meshes);

    // Define camera variables
    glm::vec3 cameraPos = glm::vec3(30.0f, 10.0f, 150.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

    // Define projection variables
    float fov = 45.0f;
    float aspectRatio = 800.0f / 600.0f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;

    // first person variables
    float cameraSpeed = 0.5f;

    // camera mode
    int cameraMode = 0;

    while (!view.shouldWindowClose())
    {

        // Handle cursor inputs
        glfwSetCursorPosCallback(view.window, [](GLFWwindow *window, double xpos, double ypos)
                                 {
        oldMouseX = mouseX; 
        oldMouseY = mouseY;
        mouseX = xpos;
        mouseY = ypos;
        shouldMove = 1; });
        // If a cursor input has been entered, rotate the scene accordingly
        if (shouldMove == 1)
        {
            view.xrotate = view.xrotate - ((oldMouseX - mouseX));
            view.yrotate = view.yrotate - ((oldMouseY - mouseY));
            shouldMove = 0;
        }

        // Key handler
        glfwSetKeyCallback(view.window, [](GLFWwindow *window, int key, int scancode, int action, int mods)
                           {
        if (key == GLFW_KEY_R){ KeyPress = 1;} 
        else if (key == GLFW_KEY_W){KeyPress = 2;} 
        else if (key == GLFW_KEY_0) {KeyPress = 3;}
        else if (key == GLFW_KEY_1) {KeyPress = 4;} });

        // If R is pressed, reset
        if (KeyPress == 1)
        {
            view.xrotate = 0;
            view.yrotate = 0;
            KeyPress = -1;
        }
        else if (KeyPress == 2)
        {
            cameraPos += cameraSpeed * cameraFront;
            KeyPress = -1;
        }
        else if (KeyPress == 3)
        {
            cameraMode = 0;
            KeyPress = -1;
        }
        else if (KeyPress == 4)
        {
            cameraMode = 1;
            KeyPress = -1;
        }

        if (cameraMode == 0)
        {
            view.display(scenegraph);
        }
        else if (cameraMode == 1)
        {
            view.displayFirstPerson(scenegraph, cameraPos, cameraFront, cameraUp);
        }
    }

    view.closeWindow();
    exit(EXIT_SUCCESS);
}

void Controller::onkey(int key, int scancode, int action, int mods)
{
    cout << (char)key << " pressed" << endl;
}

void Controller::reshape(int width, int height)
{
    cout << "Window reshaped to width=" << width << " and height=" << height << endl;
    glViewport(0, 0, width, height);
}

void Controller::dispose()
{
    view.closeWindow();
}

void Controller::error_callback(int error, const char *description)
{
    fprintf(stderr, "Error: %s\n", description);
}