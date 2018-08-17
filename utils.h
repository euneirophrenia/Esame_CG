#pragma once
#include "common.h"
#include "Geometry/geometry.h"


extern int scrH, scrW;

const Vector3 RED(1, 0, 0);
const Vector3 LIGHT_GREEN(0.1, 1, 0.1);
const Vector3 BLUE(0, 0, 1);

// Funzione che prepara tutto per usare un env map
void SetupEnvmapTexture(int texture, GLuint mode = GL_SPHERE_MAP)
{
  // facciamo binding con la texture
  glBindTexture(GL_TEXTURE_2D, texture);
   
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_TEXTURE_GEN_S); // abilito la generazione automatica delle coord texture S e T
  glEnable(GL_TEXTURE_GEN_T);
  glTexGeni(GL_S, GL_TEXTURE_GEN_MODE , mode); // Env map
  glTexGeni(GL_T, GL_TEXTURE_GEN_MODE , mode);
  glColor3f(1,1,1); // metto il colore neutro (viene moltiplicato col colore texture, componente per componente)
  glDisable(GL_LIGHTING); // disabilito il lighting OpenGL standard (lo faccio con la texture)
}

// funzione che prepara tutto per creare le coordinate texture (s,t) da (x,y,z)
// Mappo l'intervallo [ minY , maxY ] nell'intervallo delle T [0..1]
//     e l'intervallo [ minZ , maxZ ] nell'intervallo delle S [0..1]
void SetupTexture(int texture, Point3 min, Point3 max, GLuint mode = GL_OBJECT_LINEAR){
  glBindTexture(GL_TEXTURE_2D, texture);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_TEXTURE_GEN_S);
  glEnable(GL_TEXTURE_GEN_T);
  
  // ulilizzo le coordinate OGGETTO
  // cioe' le coordnate originali, PRIMA della moltiplicazione per la ModelView
  // in modo che la texture sia "attaccata" all'oggetto, e non "proiettata" su esso
  glTexGeni(GL_S, GL_TEXTURE_GEN_MODE , mode);
  glTexGeni(GL_T, GL_TEXTURE_GEN_MODE , mode);

  float sz=1.0/(max.Z() - min.Z());
  float ty=1.0/(max.Y() - min.Y());
  float s[4]={0,0,sz,  - min.Z()*sz };
  float t[4]={0,ty,0,  - min.Y()*ty };
  glTexGenfv(GL_S, GL_OBJECT_PLANE, s); 
  glTexGenfv(GL_T, GL_OBJECT_PLANE, t); 
}

void DrawText(int width, int height, std::string text, Vector3 color = LIGHT_GREEN) {
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0.0, scrW, 0.0, scrH);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  glColor3f(color.coord[0], color.coord[1], color.coord[2]); //font color
  glRasterPos2i(width, height);

  void* font = GLUT_BITMAP_9_BY_15; //font
  for (auto c : text){
    glutBitmapCharacter(font, c);
  }

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();


}