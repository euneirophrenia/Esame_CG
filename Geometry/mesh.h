#pragma once

#include "../common.h"
#include <vector>
#include "sMesh.h"


class Mesh : public sMesh {
  
  protected:
      GLuint program;
      GLuint vao; 

  public:

      Mesh((char*) modelname, (char*) vShader, (char*)fShader) : sMesh(modelname) {
          program = initShader(vShader, fShader);
          glGenVertexArrays(1, &vao);

          //todo: create a list of point4 for the vertices
          //submit that in a buffer
          //link the position in the buffer to the position in the shader
      }



};