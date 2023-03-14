#ifndef _TEXTUALRENDERVISITOR_H_
#define _TEXTUALRENDERVISITOR_H_

#include "SGNodeVisitor.h"
#include "GroupNode.h"
#include "LeafNode.h"
#include "TransformNode.h"
#include "RotateTransform.h"
#include "ScaleTransform.h"
#include "TranslateTransform.h"
#include <ShaderProgram.h>
#include <ShaderLocationsVault.h>
#include "ObjectInstance.h"
#include <stack>
#include <iostream>
using namespace std;

namespace sgraph
{
    /**
     * This visitor implements drawing the scene graph using OpenGL
     *
     */
    class TextualRenderVisitor : public SGNodeVisitor
    {
    public:
        /**
         * @brief Construct a new GLScenegraphRenderer object
         *
         * @param mv a reference to modelview stack that will be used while rendering
         * @param os the map of ObjectInstance objects
         * @param shaderLocations the shader locations for the program used to render
         */
        TextualRenderVisitor()
        {
            level = 0;
        }

        string getOutput()
        {
            return output.str();
        }

        /**
         * @brief Recur to the children for drawing
         *
         * @param groupNode
         */
        void visitGroupNode(GroupNode *groupNode)
        {
            stringstream groupName;
            std::string tabs(2 * level, ' ');
            groupName << tabs << "- " << groupNode->getName() << endl;
            append(groupName.str());
            level++;
            for (int i = 0; i < groupNode->getChildren().size(); i = i + 1)
            {
                groupNode->getChildren()[i]->accept(this);
            }
            level--;
        }

        /**
         * @brief Draw the instance for the leaf, after passing the
         * modelview and color to the shader
         *
         * @param leafNode
         */
        void visitLeafNode(LeafNode *leafNode)
        {
            stringstream leafName;
            std::string tabs(2 * level, ' ');
            leafName << tabs << "- " << leafNode->getName() << endl;
            append(leafName.str());
        }

        /**
         * @brief Multiply the transform to the modelview and recur to child
         *
         * @param transformNode
         */
        void visitTransformNode(TransformNode *transformNode)
        {
            stringstream transformName;
            std::string tabs(2 * level, ' ');
            transformName << tabs << "- " << transformNode->getName() << endl;
            append(transformName.str());
            if (transformNode->getChildren().size() > 0)
            {
                level++;
                transformNode->getChildren()[0]->accept(this);
                level--;
            }
        }

        /**
         * @brief For this visitor, only the transformation matrix is required.
         * Thus there is nothing special to be done for each type of transformation.
         * We delegate to visitTransformNode above
         *
         * @param scaleNode
         */
        void visitScaleTransform(ScaleTransform *scaleNode)
        {
            visitTransformNode(scaleNode);
        }

        /**
         * @brief For this visitor, only the transformation matrix is required.
         * Thus there is nothing special to be done for each type of transformation.
         * We delegate to visitTransformNode above
         *
         * @param translateNode
         */
        void visitTranslateTransform(TranslateTransform *translateNode)
        {
            visitTransformNode(translateNode);
        }

        void visitRotateTransform(RotateTransform *rotateNode)
        {
            visitTransformNode(rotateNode);
        }

    private:
        void append(const string &str)
        {
            output << str;
        }

        int level;
        stringstream output;
    };
}

#endif