#pragma once
#include "common.h"
#include "Geometry/geometry.h"
#include <map>
#include <string.h>


extern int scrH, scrW;

const Vector3 RED(1, 0, 0);
const Vector3 LIGHT_GREEN(0.1, 1, 0.1);
const Vector3 BLUE(0, 0, 1);
const std::vector<Point2> quad_texcoord = { Point2(1,0), Point2(1, 1), Point2(0,1), Point2(0,0)};
const std::vector<std::vector<Point3>> cube_faces {
  std::vector<Point3>{ Point3(-1,1,1), Point3(-1,-1,1), Point3(1, -1, 1), Point3(1, 1, 1)},
  std::vector<Point3>{ Point3(1,-1,-1), Point3(-1,-1,-1), Point3(-1, +1, -1), Point3(1, 1, -1)},
  std::vector<Point3>{ Point3(1,1,1), Point3(-1,1,1), Point3(-1, 1, -1), Point3(1, 1, -1)},
  std::vector<Point3>{ Point3(1,-1,-1), Point3(-1,-1,-1), Point3(-1, -1, 1), Point3(1, -1, 1)},
  std::vector<Point3>{ Point3(1,1,1), Point3(1,-1,1), Point3(1, -1, -1), Point3(1, 1, -1)},
  std::vector<Point3>{ Point3(-1,1,-1), Point3(-1,-1,-1), Point3(-1, -1, 1), Point3(-1, 1, 1)}
};

const std::vector<Vector3> cube_normals { Vector3(0,0,1), Vector3(0,0,-1), Vector3(0,1,0), Vector3(0,-1,0), Vector3(1,0,0), Vector3(-1,0,0)};

inline Point2 rotateAround(Point2 src, Point3 center, float angle) {
  float cosine = cos(angle);
  float sine = sin(angle);

  Point3 diff = src - center;

  return Point2(center.coord[0] + diff.coord[0]*cosine + diff.coord[1]*sine,
                center.coord[1] - diff.coord[0]*sine + diff.coord[1]*cosine);

}

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
  //glDisable(GL_LIGHTING); // disabilito il lighting OpenGL standard (lo faccio con la texture)
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

class TextureProvider {
  private:
    std::map<std::string, int> map;
    int count=0;
    static TextureProvider* instance;

    static bool LoadTexture(int textbind, char *filename){
        GLenum texture_format;

      SDL_Surface *s = IMG_Load(filename);

      if (!s) return false;

      if (s->format->BytesPerPixel == 4){     // contiene canale alpha
        if (s->format->Rmask == 0x000000ff){
          texture_format = GL_RGBA;
        }
        else{
          texture_format = GL_BGRA;
        }
      } else if (s->format->BytesPerPixel == 3){     // non contiene canale alpha
        if (s->format->Rmask == 0x000000ff)
          texture_format = GL_RGB;
        else
          texture_format = GL_BGR;
        } else {
            printf("[ERROR] the image is not truecolor\n");
            exit(1);
          }

      glBindTexture(GL_TEXTURE_2D, textbind);
      gluBuild2DMipmaps(
          GL_TEXTURE_2D, 
          3,
          s->w, s->h, 
          texture_format,
          GL_UNSIGNED_BYTE,
          s->pixels
      );
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ); 
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR ); 
      // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);

      return true;
    }

    TextureProvider() {}


  public:
    void LoadTexture(std::string name) {
      if (!LoadTexture(count, (char*) name.data())){
        printf("[FATAL] Could not load texture \"%s\", exiting\n", name.data());
        exit(-1);
      }
      map[name] = count;
      count++;
    }

    inline int indexOf(std::string texname) {
      return map[texname];
    }

    static TextureProvider* getInstance() {
      if (instance == nullptr)
        instance = new TextureProvider();
      return instance;
    }

    inline void BindTexture(GLenum target, std::string name) {
      glEnable(target);
      glBindTexture(target, map[name]);
      
    }

    inline void SetupAutoTexture2D(std::string texture, Point3 max, Point3 min, GLuint mode = GL_OBJECT_LINEAR) {
      glBindTexture(GL_TEXTURE_2D, map[texture]);
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

};

TextureProvider* TextureProvider::instance = nullptr;

inline void DrawQuadTexture(std::string texture, std::vector<Point3> face, std::vector<Point2> texcoord, std::vector<Vector3> normals) {
    glColor3f(1,1,1);
    glEnable(GL_TEXTURE_2D);
    TextureProvider::getInstance()->BindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glBegin(GL_QUADS);  
      for (int i=0; i<face.size(); i++) {
        glNormal3f(normals[i].coord[0], normals[i].coord[1], normals[i].coord[2]);
        glTexCoord2f(texcoord[i].coord[0], texcoord[i].coord[1]);
        glVertex3f(face[i].coord[0], face[i].coord[1], face[i].coord[2]);
      }
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

inline void DrawCube(std::vector<std::string> textures, bool withLighting = false){

  bool useTextures = textures.size() > 0;
  if (useTextures)
    glEnable(GL_TEXTURE_2D);

  if (withLighting)
    glEnable(GL_LIGHTING);
  else
    glDisable(GL_LIGHTING);
  TextureProvider* provider = TextureProvider::getInstance();
  glColor3f(1,1,1);

  for (int i=0; i<6; i++){
    DrawQuadTexture(textures[i % textures.size()], cube_faces[i], quad_texcoord, cube_normals);
    glDisable(GL_TEXTURE_2D);
  }
}

// disegna un cubo in wireframe
void drawCubeWire()
{
  glBegin(GL_LINE_LOOP); // faccia z=+1
    glVertex3f( +1,+1,+1 );
    glVertex3f( -1,+1,+1 );
    glVertex3f( -1,-1,+1 );
    glVertex3f( +1,-1,+1 );
  glEnd();

  glBegin(GL_LINE_LOOP); // faccia z=-1
    glVertex3f( +1,-1,-1 );
    glVertex3f( -1,-1,-1 );
    glVertex3f( -1,+1,-1 );
    glVertex3f( +1,+1,-1 );
  glEnd();

  glBegin(GL_LINES);  // 4 segmenti da -z a +z
    glVertex3f( -1,-1,-1 );
    glVertex3f( -1,-1,+1 );

    glVertex3f( +1,-1,-1 );
    glVertex3f( +1,-1,+1 );

    glVertex3f( +1,+1,-1 );
    glVertex3f( +1,+1,+1 );

    glVertex3f( -1,+1,-1 );
    glVertex3f( -1,+1,+1 );  
  glEnd();
}


// Just to be sure that everything is as we expect afterwards
void HardResetState() {  
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  glEnable(GL_NORMALIZE); // opengl, per favore, rinormalizza le normali prima di usarle
  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CW); // consideriamo Front Facing le facce ClockWise
  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  glEnable(GL_POLYGON_OFFSET_FILL); // caro openGL sposta i frammenti generati dalla rasterizzazione poligoni indietro di 1
  glPolygonOffset(1,1); 

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_TEXTURE_GEN_S);
  glDisable(GL_TEXTURE_GEN_T);
  glDisable(GL_BLEND);
}

