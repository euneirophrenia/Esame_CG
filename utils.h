#pragma once
#include "common.h"
#include "Geometry/geometry.h"

// Funzione che prepara tutto per usare un env map
void SetupEnvmapTexture()
{
  // facciamo binding con la texture 1
  glBindTexture(GL_TEXTURE_2D,1);
   
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_TEXTURE_GEN_S); // abilito la generazione automatica delle coord texture S e T
  glEnable(GL_TEXTURE_GEN_T);
  glTexGeni(GL_S, GL_TEXTURE_GEN_MODE , GL_SPHERE_MAP); // Env map
  glTexGeni(GL_T, GL_TEXTURE_GEN_MODE , GL_SPHERE_MAP);
  glColor3f(1,1,1); // metto il colore neutro (viene moltiplicato col colore texture, componente per componente)
  glDisable(GL_LIGHTING); // disabilito il lighting OpenGL standard (lo faccio con la texture)
}

// funzione che prepara tutto per creare le coordinate texture (s,t) da (x,y,z)
// Mappo l'intervallo [ minY , maxY ] nell'intervallo delle T [0..1]
//     e l'intervallo [ minZ , maxZ ] nell'intervallo delle S [0..1]
void SetupTexture(int texture, Point3 min, Point3 max){
  glBindTexture(GL_TEXTURE_2D, texture);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_TEXTURE_GEN_S);
  glEnable(GL_TEXTURE_GEN_T);
  
  // ulilizzo le coordinate OGGETTO
  // cioe' le coordnate originali, PRIMA della moltiplicazione per la ModelView
  // in modo che la texture sia "attaccata" all'oggetto, e non "proiettata" su esso
  glTexGeni(GL_S, GL_TEXTURE_GEN_MODE , GL_OBJECT_LINEAR);
  glTexGeni(GL_T, GL_TEXTURE_GEN_MODE , GL_OBJECT_LINEAR);
  float sz=1.0/(max.Z() - min.Z());
  float ty=1.0/(max.Y() - min.Y());
  float s[4]={0,0,sz,  - min.Z()*sz };
  float t[4]={0,ty,0,  - min.Y()*ty };
  glTexGenfv(GL_S, GL_OBJECT_PLANE, s); 
  glTexGenfv(GL_T, GL_OBJECT_PLANE, t); 
}