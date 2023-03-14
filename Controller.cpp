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
    glm::vec3 cameraPos = glm::vec3(30.0f, 5.0f, 75.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

    // Define projection variables
    float fov = 45.0f;
    float aspectRatio = 800.0f / 600.0f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;

    // first person variables
    float cameraSpeed = 2.0f;
    float sensitivity = 1.0f;
    float pitch = 0.0f;
    float yaw = -90.0f;

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
        if (key == GLFW_KEY_R){ KeyPress = GLFW_KEY_R;} 
        else if (key == GLFW_KEY_EQUAL){KeyPress = GLFW_KEY_EQUAL;} 
        else if (key == GLFW_KEY_MINUS){KeyPress = GLFW_KEY_MINUS;} 
        else if (key == GLFW_KEY_0) {KeyPress = GLFW_KEY_0;}
        else if (key == GLFW_KEY_1) {KeyPress = GLFW_KEY_1;}
        else if (key == GLFW_KEY_UP){KeyPress = GLFW_KEY_UP;}
        else if (key == GLFW_KEY_DOWN){KeyPress = GLFW_KEY_DOWN;}
        else if (key == GLFW_KEY_LEFT){KeyPress = GLFW_KEY_LEFT;}
        else if (key == GLFW_KEY_RIGHT){KeyPress = GLFW_KEY_RIGHT;} });

        // If R is pressed, reset
        if (KeyPress == GLFW_KEY_R)
        {
            cameraPos = glm::vec3(30.0f, 10.0f, 50.0f);
            pitch = 0.0f;
            yaw = -90.0f;
            KeyPress = -1;
        }
        else if (KeyPress == GLFW_KEY_EQUAL)
        {
            cameraPos += cameraSpeed * cameraFront;
            KeyPress = -1;
        }
        else if (KeyPress == GLFW_KEY_MINUS)
        {
            cameraPos -= cameraSpeed * cameraFront;
            KeyPress = -1;
        }
        else if (KeyPress == GLFW_KEY_0) // change camera to global
        {
            cameraMode = 0;
            KeyPress = -1;
        }
        else if (KeyPress == GLFW_KEY_1) // change camera to first person
        {
            cameraMode = 1;
            KeyPress = -1;
        }
        else if (KeyPress == GLFW_KEY_UP) // change camera to first person
        {
            pitch = pitch + sensitivity;
            KeyPress = -1;
        }
        else if (KeyPress == GLFW_KEY_DOWN) // change camera to first person
        {
            pitch = pitch - sensitivity;
            KeyPress = -1;
        }
        else if (KeyPress == GLFW_KEY_LEFT) // change camera to first person
        {
            yaw = yaw - sensitivity;
            KeyPress = -1;
        }
        else if (KeyPress == GLFW_KEY_RIGHT) // change camera to first person
        {
            yaw = yaw + sensitivity;
            KeyPress = -1;
        }

        cout << cameraFront.x << " " << cameraFront.y << " " << cameraFront.z << endl;
        cout << pitch << " " << yaw << endl;

        cameraFront = glm::normalize(glm::vec3(
            cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
            sin(glm::radians(pitch)),
            sin(glm::radians(yaw)) * cos(glm::radians(pitch))));
        cout << cameraFront.x << " " << cameraFront.y << " " << cameraFront.z << endl;
        cout << pitch << " " << yaw << endl;
        // change camera mode
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