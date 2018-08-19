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
extern std::string floor_texture;
const std::vector<std::string> wall_textures = {"Resources/wall1.jpg", "Resources/wall2.jpg", "Resources/wall2.jpg", "Resources/wall2.jpg", "Resources/wall2.jpg", "Resources/wall2.jpg"};

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
            const int K=100; //disegna K x K quads
            
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
            if (!useWireframe) {
                SetupEnvmapTexture(5, GL_OBJECT_LINEAR); //use floor_texture
                glEnable(GL_LIGHTING);
                //glColor3f(0.7, 0.7, 0.7);

            }

            glBegin(GL_QUADS);
                glColor3f(1, 1, 1); // colore uguale x tutti i quads
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
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
        }

        void drawSky() {
            int H = 100;

            if (useWireframe) {
                glDisable(GL_TEXTURE_2D);
                glColor3f(0,0,0);
                glDisable(GL_LIGHTING);
                glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
                //drawSphere(100.0, 20, 20);
                glColor3f(1,1,1);
                glPushMatrix();
                    glTranslatef(0, H * 0.99, 0);
                    glScalef(H,H,H);
                    drawCubeWire();
                    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
                glPopMatrix();
                glColor3f(1,1,1);
                glEnable(GL_LIGHTING);
            } 
            else {
                glDisable(GL_LIGHTING);
                //glEnable(GL_LIGHTING);
                glColor3f(1,1,1);
                glPushMatrix();
                    glTranslatef(0, H * 0.99, 0);
                    glScalef(H,H,H);
                    DrawCube(wall_textures);
                glPopMatrix();
            }
        }

    public:

        World() {
            //create all the tiles needed
            ExponentialSlope* obstacle = new ExponentialSlope( (char*) "./Resources/exptile.obj");
            ExponentialSlope* obstacle2 = new ExponentialSlope( (char*) "./Resources/exptile.obj");
            obstacle2->Translate(19, 0, 0);

            SphereTile* sphere = new SphereTile((char*) "./Resources/sphere.obj");
            sphere->Scale(2, 2, 2);
            sphere->Translate(-19, 2, 4);

            CubeTile* cube = new CubeTile((char*) "./Resources/cube.obj");
            cube->Rotate(45);
            cube->Scale(1.5,1.5,1.5);
            cube->Translate(30, 1.5, 20);

            tiles.push_back(obstacle);
            tiles.push_back(obstacle2);
            tiles.push_back(cube);
            tiles.push_back(sphere);

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
            
           
            drawSky();   
            drawFloor();   

            for (auto tile: tiles)
                tile->Draw();      
            
        }

        void BindBuffers() {
            for (auto tile: tiles) {
                tile->BindBuffers();
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

        inline Vector3 normal_at(Point3 point) {
            if (last_tile != nullptr && last_tile->hasInside(point))
                return last_tile->normal_at(point);

            for (auto tile : tiles) {
                if (tile != last_tile && tile->hasInside(point)) {
                    last_tile = tile;
                    return tile->normal_at(point);
                }
            }
            return Vector3(0,1,0);
        }




};