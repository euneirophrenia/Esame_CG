#pragma once

#include "./Geometry/sMesh.h"
#include <cmath>
#include "utils.h"

extern bool useShadow;
extern bool useTransparency;
extern bool useHeadlight;
extern Point3 player_position;
extern std::string floor_texture;

const Vector3 UP(0,1,0);
TextureProvider* texProvider = TextureProvider::getInstance();

class Tile {

    protected:
        Point3 center; //position of the center of the tile in world coordinates (the origin of the tile-coordinates)
        float rotation = 0; //rotation between world and tile-coords
        Vector3 scale = Vector3(1,1,1);
        std::string textureName = "Resources/wood.jpg";
        
        virtual void DrawShadow() {
            glColor3f(0, 0, 0); // colore fisso
            glTranslatef(0, -center.Y() + 0.01,  0); // alzo l'ombra di un epsilon per evitare z-fighting con il pavimento
            glScalef(1.01, 0, 1.01);  // appiattisco sulla Y, ingrandisco dell'1% sulla Z e sulla X 
            glDisable(GL_LIGHTING); // niente lighting per l'ombra

            glDisable(GL_TEXTURE_2D);
            model.RenderArray(); 
            glEnable(GL_LIGHTING);
        }

    public:
        sMesh model;
        inline void Translate(Vector3 vect) {
            center = center + vect;
        }

        inline void Translate(float x, float y, float z) {
            center.coord[0] += x;
            center.coord[1] += y;
            center.coord[2] += z;
        }

        //assume only rotation on the horizontal plane, can't rotate a tile in 3D, just use a different tile
        inline void Rotate(float angle) {
            rotation+=angle;
        }

        inline void Scale(float sx, float sy, float sz) {
            scale.coord[0] *= sx;
            scale.coord[1] *= sy;
            scale.coord[2] *= sz;
        }

        inline void Scale(Vector3 s) {
            Scale(s.coord[0], s.coord[1], s.coord[2]);
        }

        inline void BindBuffers() {
            model.BindBuffers();
        }

        //todo:: also handle scaling!!

        virtual bool hasInside(Point3 point){
            Point3 max = center + (model.bbmax*scale);
            Point3 min = center + (model.bbmin*scale);
            return point.X() <= max.X() && point.X() >= min.X() &&
                    point.Z() <= max.Z() && point.Z() >= min.Z();
        }

        explicit Tile(char* model_path) {
            model.Init(model_path);
            center.coord[0]=0;
            center.coord[1]=0;
            center.coord[2]=0;
        }

        virtual void Draw() {
            glPushMatrix();
                glColor3f(0.5, 0.5, 0.5);
                glTranslatef(center.X(), center.Y(), center.Z());
                glRotatef(rotation, 0, 1, 0);
                glScalef(scale.X(), scale.Y(), scale.Z());
                if (!useWireframe) {
                    texProvider->SetupAutoTexture2D(textureName, model.bbmin, model.bbmax);
                }
                model.RenderArray();
                if (useShadow) {
                    DrawShadow();
                }
            glPopMatrix();
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
            glDisable(GL_TEXTURE_2D);
        }

        virtual float height_at(Point3 point) = 0; 
        virtual Vector3 normal_at(Point3 point) =0;
};


class FlatTile : public Tile {
    
    public:
        explicit FlatTile(char* filename) : Tile(filename) {}

        float height_at(Point3 point) {
            return model.bbmax.Y() * scale.Y();
        }

        Vector3 normal_at(Point3 point) {
            return UP;
        }

};

class ExponentialSlope : public Tile {

    public:

        explicit ExponentialSlope(char* filename) : Tile(filename) {}

        float height_at(Point3 point) {

            float sine = -sin(rotation);
            float cosine = cos(rotation);

            //undo any translation
            float tile_x = point.X() - center.X();
            float tile_z = point.Z() - center.Z();

            // undo any rotation between tile and world
            tile_x = cosine * tile_x + sine * tile_z;  // i don't really need the other coordinate, it is not used in this case
            tile_x /= scale.X();
        
            //compute the actual height, according to the custom shape i made
            return scale.Y()*(center.Y() + pow(1.5, -abs(tile_x) + 5));
            // the truth is: i first created the mesh, making it "look good", then I spent a good hour trying to find its damn equation
            // and it turns out that can be written like that
            // when handling also scaling, it might get a bit hard do adjust, but I don't think i will ever need that
        }

        Vector3 normal_at(Point3 point){
            float sine = -sin(rotation);
            float cosine = cos(rotation);

            //undo any translation
            float tile_x = point.X() - center.X();
            float tile_z = point.Z() - center.Z();

            // undo any rotation between tile and world
            tile_x = cosine * tile_x + sine * tile_z;  // i don't really need the other coordinate, it is not used in this case
            tile_x /= scale.X();

            if (tile_x >=0) 
                return Vector3(-pow(1.5, -tile_x + 5)*log(1.5), -1, 0);

            return Vector3(pow(1.5, tile_x + 5)*log(1.5), -1, 0);
        
        }

};

class PitTile : public Tile {
    protected:
        virtual inline float texture_code() { return 6; }
    public:
        explicit PitTile(char* filename) : Tile(filename) {} 
        float height_at(Point3 point) {
            float sine = -sin(rotation);
            float cosine = cos(rotation);

            //undo any translation
            float tile_x = point.X() - center.X();
            float tile_z = point.Z() - center.Z();

            // undo any rotation between tile and world
            tile_x = cosine * tile_x + sine * tile_z; 
            tile_z = -sine * tile_x + cosine * tile_z;

            tile_x /= scale.X();
            tile_z /= scale.Z();

            return (abs(tile_x) < 0.8 || abs(tile_z) < 0.8) ? 500 : 0;

        }

        Vector3 normal_at(Point3 point) {
            return UP;
        }
};

class SphereTile : public Tile {
    public:
        explicit SphereTile(char* filename) : Tile(filename) {
            textureName = "Resources/universe.jpg";
        }
        float height_at(Point3 point) {
            // float sine = -sin(rotation);
            // float cosine = cos(rotation);

            // //undo any translation
            // float tile_x = point.X() - center.X();
            // float tile_z = point.Z() - center.Z();

            // // undo any rotation between tile and world
            // tile_x = cosine * tile_x + sine * tile_z; 
            // tile_z = -sine * tile_x + cosine * tile_z;

            // tile_x /= scale.X();
            // tile_z /= scale.Z();

            // return scale.Y()*sqrt(abs(1 - tile_x*tile_x - tile_z*tile_z));
            return 1337; //we don't want to be able to be on the sphere, like ever
        }

        Vector3 normal_at(Point3 point) {
            return -point;
        }

        void Draw() {
            glPushMatrix();
                glColor3f(0.5, 0.5, 0.5);
                glTranslatef(center.X(), center.Y(), center.Z());
                glRotatef(rotation, 0, 1, 0);
                glScalef(scale.X(), scale.Y(), scale.Z());
                if (!useWireframe && !useTransparency) {
                    texProvider->SetupAutoTexture2D(textureName, model.bbmin, model.bbmax);
                }
                if (useTransparency && !useWireframe) {
                    glColor4f(0.9, 0.9, 0.9, 0.6);
                    glEnable(GL_BLEND);
                    glDisable(GL_DEPTH);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                }
                model.RenderArray();
                if (useShadow && !useTransparency) {
                    float dist=(player_position - center).modulo();
                    if (!useHeadlight || dist > 20)
                        glColor3f(0, 0, 0);
                    else
                        glColor4f(0.7 - dist/30 , 0.7 - dist/30, 0.7 - dist/30, dist/30); // colore non proprio fisso
                    glTranslatef(0, -center.Y() + 1.01,  0); // alzo l'ombra di un epsilon per evitare z-fighting con il pavimento
                    glScalef(1.01, 0, 1.01);  // appiattisco sulla Y, ingrandisco dell'1% sulla Z e sulla X 
                    glDisable(GL_LIGHTING); // niente lighting per l'ombra
                    glEnable(GL_BLEND);
                    glDisable(GL_DEPTH);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    texProvider->BindTexture(GL_TEXTURE_2D, floor_texture);
                    model.RenderArray();
                    glEnable(GL_LIGHTING);
                    glEnable(GL_DEPTH);
                }
            glPopMatrix();
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_BLEND);
            glEnable(GL_DEPTH);
        }

};

class CubeTile : public Tile {
    protected:
        std::vector<std::string> textures;
        Point3 maximum{1,1,1};
        Point3 minimum{-1,-1,-1};

    public:
        explicit CubeTile(char* filename) : Tile(filename) {
            textureName = "Resources/dice.jpg";
            textures.push_back(textureName);

        }
        float height_at(Point3 point) {
            return 2*scale.Y();
        }

        Vector3 normal_at(Point3 point) {
            return UP;
        }

        virtual bool hasInside(Point3 point){
            Point3 max = center + (maximum*scale);
            Point3 min = center + (minimum*scale);
            return point.X() <= max.X() && point.X() >= min.X() &&
                    point.Z() <= max.Z() && point.Z() >= min.Z();
        }

        virtual void Draw() {
            glPushMatrix();
                glColor3f(0.5, 0.5, 0.5);
                glTranslatef(center.X(), center.Y(), center.Z());
                glRotatef(rotation, 0, 1, 0);
                glScalef(scale.X(), scale.Y(), scale.Z());
                glDisable(GL_CULL_FACE);
                if (!useWireframe) {
                      DrawCube(textures, true);
                }
                else {
                    drawCubeWire();
                }
               
                if (useShadow) {
                    DrawShadow();
                }
            glPopMatrix();
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
            glDisable(GL_TEXTURE_2D);
            glEnable(GL_CULL_FACE);
        }

};

