#pragma once

#include <vector> // la classe vector di STL 
#include <unordered_map>

#include "Geometry/sMesh.h"
#include "tiles.h"

// var globale di tipo mesh
sMesh pista( (char*) "./Resources/pista.obj");

extern bool useEnvmap; // var globale esterna: per usare l'evnrionment mapping
extern bool useHeadlight; // var globale esterna: per usare i fari
extern bool useShadow; // var globale esterna: per generare l'ombra


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
void SetupWheelTexture(Point3 min, Point3 max){
  glBindTexture(GL_TEXTURE_2D,0);
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

class World {

    private:

        Tile* last_tile = nullptr; 

        std::vector<Tile*> tiles;
        std::vector<float> vertices;

        void drawSphere(double r, int lats, int longs) {
            int i, j;
            for(i = 0; i <= lats; i++) {
                double lat0 = M_PI * (-0.5 + (double) (i - 1) / lats);
                double z0  = sin(lat0);
                double zr0 =  cos(lat0);
            
                double lat1 = M_PI * (-0.5 + (double) i / lats);
                double z1 = sin(lat1);
                double zr1 = cos(lat1);
                
                glBegin(GL_QUAD_STRIP);
                for(j = 0; j <= longs; j++) {
                    double lng = 2 * M_PI * (double) (j - 1) / longs;
                    double x = cos(lng);
                    double y = sin(lng);
                
            //le normali servono per l'EnvMap
                    glNormal3f(x * zr0, y * zr0, z0);
                    glVertex3f(r * x * zr0, r * y * zr0, r * z0);
                    glNormal3f(x * zr1, y * zr1, z1);
                    glVertex3f(r * x * zr1, r * y * zr1, r * z1);
                }
                glEnd();
            }
        }

        void drawFloor() {

            const float S=100; // size
            const float H=0;   // altezza
            const int K=150; //disegna K x K quads
            
            /*
            //vecchio codice ora commentato
            // disegna un quad solo 
            glBegin(GL_QUADS);
                glColor3f(0.5, 0.2, 0.0);
                glNormal3f(0,1,0);
                glVertex3d(-S, H, -S);
                glVertex3d(+S, H, -S);
                glVertex3d(+S, H, +S);
                glVertex3d(-S, H, +S);
            glEnd();
            */
            
            // disegna KxK quads
            glBegin(GL_QUADS);
                glColor3f(0.6, 0.6, 0.6); // colore uguale x tutti i quads
                glNormal3f(0,1,0);       // normale verticale uguale x tutti
                for (int x=0; x<K; x++) 
                for (int z=0; z<K; z++) {
                float x0=-S + 2*(x+0)*S/K;
                float x1=-S + 2*(x+1)*S/K;
                float z0=-S + 2*(z+0)*S/K;
                float z1=-S + 2*(z+1)*S/K;
                glVertex3d(x0, H, z0);
                glVertex3d(x1, H, z0);
                glVertex3d(x1, H, z1);
                glVertex3d(x0, H, z1);
                }
            glEnd();
        }

        void drawSky() {
            int H = 100;

            if (useWireframe) {
                glDisable(GL_TEXTURE_2D);
                glColor3f(0,0,0);
                glDisable(GL_LIGHTING);
                glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
                drawSphere(100.0, 20, 20);
                glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
                glColor3f(1,1,1);
                glEnable(GL_LIGHTING);
            }
            else
            {
                    glBindTexture(GL_TEXTURE_2D,2);
                    glEnable(GL_TEXTURE_2D);
                    glEnable(GL_TEXTURE_GEN_S);
                    glEnable(GL_TEXTURE_GEN_T);
                    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE , GL_SPHERE_MAP); // Env map
                    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE , GL_SPHERE_MAP);
                    glColor3f(1,1,1);
                    glDisable(GL_LIGHTING);

                //   drawCubeFill();
                    drawSphere(100.0, 20, 20);

                    glDisable(GL_TEXTURE_GEN_S);
                    glDisable(GL_TEXTURE_GEN_T);
                    glDisable(GL_TEXTURE_2D);
                    glEnable(GL_LIGHTING);
            }

}

    public:

        World() {
            ExponentialSlope* obstacle = new ExponentialSlope( (char*) "./WIP/writings.obj");
            ExponentialSlope* obstacle2 = new ExponentialSlope( (char*) "./WIP/writings.obj");
            FlatTile* flat = new FlatTile((char*) "./WIP/flat.obj");
            obstacle2->Translate(19, 0, 0);
            flat->Translate(-19, 0, 0) ;
            flat->Scale(1, 1, 0.2);
            tiles.push_back(obstacle);
            tiles.push_back(obstacle2);
            tiles.push_back(flat);

            // for (auto tile : tiles) {
            //     for (float f : tile->model.Flat_Vertices()) {
            //         vertices.push_back(f);
            //     }
            // }
            
        }

        void draw() {
            // glPushMatrix();
            // glColor3f(0.4,0.4,.8);
            // glScalef(0.75, 1.0, 0.75);
            // glTranslatef(0,0.01,0);
            // //pista.RenderNxV();
            // pista.RenderNxF();
            // glPopMatrix();
            
            for (auto tile: tiles)
                tile->Draw();

            drawFloor();
            drawSky();
        }

        void BindVAOs() {
            for (auto tile: tiles) {
                tile->BindVAO();
            }
        }


        inline float height_at(float x, float z) {
            return height_at(Point3(x,0,z));
        }

        inline float height_at(Point3 point) {
            if (last_tile != nullptr && last_tile->hasInside(point))
                return last_tile->height_at(point);

            for (auto tile : tiles) {
                if (tile != last_tile && tile->hasInside(point)) {
                    last_tile = tile;
                    return tile->height_at(point);
                }
            }
            return 0;
        }




};