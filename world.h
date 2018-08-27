#pragma once

#include <vector> // la classe vector di STL 
#include <unordered_map>

#include "Geometry/sMesh.h"
#include "tiles.h"

#include <algorithm>

/*
*
*  This holds the environment! It handles the floor, the sky and the tiles.
*
*/

extern bool useHeadlight; // var globale esterna: per usare i fari
extern bool useShadow; // var globale esterna: per generare l'ombra
extern bool useTransparency;
extern bool useWireframe;

extern Point3 player_position;
extern std::string floor_texture;
const std::vector<std::string> wall_textures = {"Resources/wall3.ppm", "Resources/wall4.ppm", "Resources/wall3.ppm", "Resources/wall3.ppm", "Resources/walldoor.ppm", "Resources/wall1.ppm"};

//sort by most distant from player
bool tileCompare(Tile* a, Tile* b) {
    return (a->center - player_position).l1norm() > (b->center - player_position).l1norm();
}

class World {

    private:

        Tile* last_tile = nullptr; //to optimize subsequent calls to find the tile the bike is on

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

            const float S=ROOM_SIZE; // size
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
                // SetupEnvmapTexture(texProvider->indexOf(floor_texture), GL_OBJECT_LINEAR);
                texProvider->SetupAutoTexture2D(floor_texture, Point3(S*0.5/K, 0, S*0.5/K), Point3(-S*0.5/K, 0, -S*0.5/K), GL_OBJECT_LINEAR);
                glEnable(GL_LIGHTING);
                //glColor3f(0.7, 0.7, 0.7);

            }

            glBegin(GL_QUADS);
                glColor3f(1, 1, 1);
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
            int H = ROOM_SIZE;

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

            AtanTile* obstacle3 = new AtanTile((char*) "./Resources/atantile.obj");
            obstacle3->Scale(3, 2, 3);
            obstacle3->Translate(48, 0.1, 0);
            

            SphereTile* sphere = new SphereTile((char*) "./Resources/sphere.obj", "Resources/universe.ppm", 0.5);
            sphere->Scale(2, 2, 2);
            sphere->Rotate(90);
            sphere->Translate(-19, 2, 30);

            SphereTile* sphere2 = new SphereTile((char*) "./Resources/sphere.obj", "Resources/ball2.ppm");
            sphere2->Scale(2, 2, 2);
            sphere2->Rotate(90);
            sphere2->Translate(19, 2, -19);

            SphereTile* sphere3 = new SphereTile((char*) "./Resources/sphere.obj", "Resources/ball2.ppm", -0.5);
            sphere3->Scale(2, 2, 2);
            sphere3->Rotate(90);
            sphere3->Translate(-39, 2, -35);


            CubeTile* cube = new CubeTile((char*) "./Resources/cube.obj");
            cube->Rotate(45);
            cube->Scale(1.5,1.5,1.5);
            cube->Translate(30, 1.5, 20);

            CubeTile* cube2 = new CubeTile((char*) "./Resources/cube.obj");
            cube2->Rotate(45);
            cube2->Scale(1.5,1.5,1.5);
            cube2->Translate(-30, 1.5, 20);

            WhirligigTile* whirl = new WhirligigTile((char*) "./Resources/whirligig.obj");
            whirl->Scale(1.5, 1.5, 1.5);
            whirl->Translate(-30, 0, -45);

            FlatTile* carpet = new FlatTile((char*) "./Resources/flattile.obj", "Resources/carpet.ppm");
            carpet->Scale(27, 1, 16.16);
            carpet->Translate(0, 0.1, -75);

            tiles.push_back(obstacle);
            tiles.push_back(obstacle2);
            tiles.push_back(obstacle3);
            tiles.push_back(cube);
            tiles.push_back(cube2);
            tiles.push_back(whirl);
            tiles.push_back(carpet);
            tiles.push_back(sphere);
            tiles.push_back(sphere2);
            tiles.push_back(sphere3);

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

            if (!useTransparency || useWireframe) {
                for (auto tile: tiles)
                    tile->Draw();
            }
            else {
                std::sort(tiles.begin(), tiles.end(), tileCompare);
                for (auto tile: tiles)
                    tile->Draw();
                
            }
            
        }

        void BindBuffers() {
            for (auto tile: tiles) {
                tile->BindBuffers();
            }
        }


        inline float height_at(float x, float z) {
            return height_at(Point3(x,0,z));
        }

        inline Vector3 friction_at(float x, float z) {
            return friction_at(Point3(x, 0, z));
        }

        inline Vector3 friction_at(Point3 point){
            if (last_tile != nullptr && last_tile->hasInside(point))
                return last_tile->friction_at(point);

            for (auto tile : tiles) {
                if (tile != last_tile && tile->hasInside(point)) {
                    last_tile = tile;
                    return tile->friction_at(point);
                }
            }

            return base_friction;
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

        inline std::vector<Tile*> getTiles() {
            return tiles;
        }




};