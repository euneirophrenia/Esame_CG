#pragma once

#include "./Geometry/sMesh.h"
#include <cmath>
#include "utils.h"
#include <cstdlib>

extern bool useShadow;
extern bool useTransparency;
extern bool useHeadlight;
extern bool stopTime;
extern Point3 player_position;
extern std::string floor_texture;

const Vector3 UP(0,1,0);
TextureProvider* texProvider = TextureProvider::getInstance();

class Tile {

    protected:
        Vector3 scale = Vector3(1,1,1);
        std::string textureName ;
        bool cullface = true;
        bool uselight = true;
        Vector3 basecolor = Vector3(0.8, 0.8, 0.8);
        Vector3 friction = Vector3(base_friction);
        
        virtual void DrawShadow() {
            glColor3f(0, 0, 0); // colore fisso
            glTranslatef(0, -center.Y() + 0.01,  0); // alzo l'ombra di un epsilon per evitare z-fighting con il pavimento
            glScalef(1.01, 0, 1.01);  // appiattisco sulla Y, ingrandisco dell'1% sulla Z e sulla X 
            glDisable(GL_LIGHTING); // niente lighting per l'ombra

            glDisable(GL_TEXTURE_2D);
            model.RenderArray(); 
            glEnable(GL_LIGHTING);
        }

        virtual void DoPhysics() {};

    public:
        Point3 center; //position of the center of the tile in world coordinates (the origin of the tile-coordinates)
        float rotation = 0; //rotation between world and tile-coords
        sMesh model;
        bool becomesTransparent = false;
        inline void Translate(Vector3 vect) {
            center = center + vect;
        }

        inline Vector3 getScale() {
            return scale;
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

        inline void FreeBuffers() {
            model.FreeBuffers();
        }

        //todo:: also handle rotations!!

        virtual bool hasInside(Point3 point){
            Point3 max = center + (model.bbmax*scale);
            Point3 min = center + (model.bbmin*scale);
            return point.X() <= max.X() && point.X() >= min.X() &&
                    point.Z() <= max.Z() && point.Z() >= min.Z();
        }

        explicit Tile(char* model_path, std::string texture = "Resources/wood.ppm") {
            model.Init(model_path);
            center.coord[0]=0;
            center.coord[1]=0;
            center.coord[2]=0;
            textureName = texture;
        }

        virtual void Draw() {
            glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 100);
            if (!cullface)
                glDisable(GL_CULL_FACE);
            if (!uselight)
                glDisable(GL_LIGHTING);

            glPushMatrix();
                glColor3f(basecolor.X(), basecolor.Y(), basecolor.Z());
                glTranslatef(center.X(), center.Y(), center.Z());
                DoPhysics();
                glRotatef(rotation, 0, 1, 0);
                glScalef(scale.X(), scale.Y(), scale.Z());
                if (!useWireframe) {
                    texProvider->SetupAutoTexture2D(textureName, model.bbmin, model.bbmax);
                }
                model.RenderArray();
                if (useShadow && useWireframe) {
                    DrawShadow();
                }
            glPopMatrix();
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
            glDisable(GL_TEXTURE_2D);
            glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 50);
            if (!cullface) 
                glEnable(GL_CULL_FACE);
            if (!uselight)
                glEnable(GL_LIGHTING);
        }

        virtual float height_at(Point3 point) = 0; 
        virtual Vector3 normal_at(Point3 point) = 0;

        inline Vector3 friction_at(Point3 point) {
            return friction;
        }
        
        virtual void DrawMiniMarker() {
            glBegin(GL_LINE_LOOP);
                glVertex2f(model.bbmax.X() / ROOM_SIZE, model.bbmax.Z() / ROOM_SIZE);
                glVertex2f(model.bbmin.X() / ROOM_SIZE, model.bbmax.Z() / ROOM_SIZE);
                glVertex2f(model.bbmin.X() / ROOM_SIZE, model.bbmin.Z() / ROOM_SIZE);
                glVertex2f(model.bbmax.X() / ROOM_SIZE, model.bbmin.Z() / ROOM_SIZE);
            glEnd();
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

class AtanTile : public Tile {
    public:
        
        explicit AtanTile(char* filename) : Tile(filename) {
            cullface = false;
            basecolor.coord[0] = 1;
            basecolor.coord[1] = 1;
            basecolor.coord[2] = 1;
            //uselight = false;
        }

        float height_at(Point3 point) {
            float sine = -sin(rotation);
            float cosine = cos(rotation);

            //undo any translation
            float tile_x = point.X() - center.X();
            float tile_z = point.Z() - center.Z();

            // undo any rotation between tile and world
            tile_x = cosine * tile_x + sine * tile_z;  // i don't really need the other coordinate, it is not used in this case
            tile_x /= scale.X();

            return scale.Y() * (atan(tile_x/2) + 1.37);
        }

        Vector3 normal_at(Point3 point) {
            float sine = -sin(rotation);
            float cosine = cos(rotation);

            //undo any translation
            float tile_x = point.X() - center.X();
            float tile_z = point.Z() - center.Z();

            // undo any rotation between tile and world
            tile_x = cosine * tile_x + sine * tile_z;  // i don't really need the other coordinate, it is not used in this case
            tile_x /= scale.X();

            return Vector3(2.0/(tile_x*tile_x + 4.1), -1, 0);
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
    protected:
        float velocity = 0.1;
        float angle = 0;

        inline virtual void DrawShadow() {
            float dist=(player_position - center).modulo();
            if (!useHeadlight || dist > 20)
                glColor3f(0, 0, 0);
            else
                glColor4f(0.7 - dist/30 , 0.7 - dist/30, 0.7 - dist/30, dist/30); // colore non proprio fisso
            glPushMatrix();
                glTranslatef(0, -center.Y() + 1.01,  0); // alzo l'ombra di un epsilon per evitare z-fighting con il pavimento
                glScalef(1.01, 0, 1.01);  // appiattisco sulla Y, ingrandisco dell'1% sulla Z e sulla X 
                glDisable(GL_LIGHTING); // niente lighting per l'ombra
                glEnable(GL_BLEND);
                glDisable(GL_DEPTH);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                texProvider->BindTexture(GL_TEXTURE_2D, floor_texture);
                model.RenderArray();
            glPopMatrix();
            glEnable(GL_LIGHTING);
            glEnable(GL_DEPTH);
            glDisable(GL_BLEND);
            glColor3f(1,1,1);
        }

        inline void DoPhysics(){
            if (stopTime) return;
            center.coord[0] += velocity;
            glRotatef(-angle, 0, 0, 1);
            angle += 180.0 * (velocity / scale.Y()) / M_PI;
            if (abs(center.coord[0])>90) 
                velocity *= -1;
        }

    public:
        explicit SphereTile(char* filename, std::string texture = "Resources/universe.ppm", float v = 0.1) : Tile(filename, texture) {
            velocity = v;
            becomesTransparent = true;
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
                glColor3f(1, 1, 1);
                glTranslatef(center.X(), center.Y(), center.Z());
                glScalef(scale.X(), scale.Y(), scale.Z());

                if (useShadow && !useTransparency) {
                    DrawShadow();
                }
                DoPhysics();  
                glRotatef(rotation, 0, 1, 0);
                
            
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
                
            glPopMatrix();
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_BLEND);
            glEnable(GL_DEPTH);
        }

        void DrawMiniMarker() {
            drawCircle(0.01);
        }

};

class CubeTile : public Tile {
    protected:
        std::vector<std::string> textures;
        Point3 maximum{1,1,1};
        Point3 minimum{-1,-1,-1};
        bool useligthing = true;

    public:
        explicit CubeTile(char* filename) : Tile(filename) {
            textureName = "Resources/logo.ppm";
            textures.push_back(textureName);
            for (int k=1; k<6;k++)
                textures.push_back("Resources/dice" + std::to_string(k) + ".ppm");

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
                glColor3f(0.9, 0.9, 0.9);
                glTranslatef(center.X(), center.Y(), center.Z());
                glRotatef(rotation, 0, 1, 0);
                glScalef(scale.X(), scale.Y(), scale.Z());
                glDisable(GL_CULL_FACE);
                if (!useWireframe) {
                      DrawCube(textures, useligthing);
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

class WhirligigTile : public Tile {
    protected:
        const float omega = 20, tilt_omega = 0.05, tilt_max = 5;
        float tilt = 0;

        virtual void DrawShadow() {
            float dist=(player_position - center).modulo();
            if (!useHeadlight || dist > 20)
                glColor3f(0, 0, 0);
            else
                glColor4f(0.7 - dist/30 , 0.7 - dist/30, 0.7 - dist/30, dist/30); // colore non proprio fisso
            //glPushMatrix();
                glTranslatef(0, 0.01,  0); // alzo l'ombra di un epsilon per evitare z-fighting con il pavimento
                glScalef(1.01, 0, 1.01);  // appiattisco sulla Y, ingrandisco dell'1% sulla Z e sulla X 
                glDisable(GL_LIGHTING); // niente lighting per l'ombra
                glEnable(GL_BLEND);
                glDisable(GL_DEPTH);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                texProvider->BindTexture(GL_TEXTURE_2D, floor_texture);
                model.RenderArray();
            //glPopMatrix();
            glEnable(GL_LIGHTING);
            glEnable(GL_DEPTH);
            glDisable(GL_BLEND);
            glColor3f(1,1,1);
        }

        inline virtual void DoPhysics() {
            if (stopTime) return;
            rotation += omega;
            rotation = rotation > 360? rotation - 360 : rotation;
            glRotatef(rotation, 0, 1, 0);

            float r = (float) rand() / (float) RAND_MAX;
            float sign_x = r > 0.5 ? 1 : -1;
            float sign_z = (float) rand() / (float) RAND_MAX > 0.5 ? 1 : -1;
            glRotatef(tilt, sign_x*r, 0, sign_z*(1-r));
            if (tilt > 0) { 
                tilt = tilt - tilt_omega > 0? tilt - tilt_omega: 0;
            }
            if (tilt==0) {
                r = (float) rand() / (float) RAND_MAX;
                if (r * 100 > 99)
                    tilt = tilt_max;
            }
        }

    public:

        float height_at(Point3 point) {
            return 1337; //don't step here
        }

        Vector3 normal_at(Point3 point) {
            return UP; //doesn't matter
        }

        explicit WhirligigTile(char* filename) : Tile(filename) {
            textureName = "Resources/metal.ppm";
        }

        virtual void Draw() {
            glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 10);
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, FOUR_1);
            glEnable(GL_CULL_FACE);
            glPushMatrix();
                glTranslatef(center.X(), center.Y(), center.Z());
                glScalef(scale.X(), scale.Y(), scale.Z());
                glColor3f(0.5, 0.5, 0.5);

                DoPhysics();

                if (!useWireframe) {
                    //texProvider->SetupAutoTexture2D(textureName, model.bbmin, model.bbmax);
                    glColor3f(0.4, 0.4, 0.4); //actually, nvm the texture
                }
                model.RenderArray();
                if (useShadow) {
                    DrawShadow();
                }
               
                
            glPopMatrix();
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
            glDisable(GL_TEXTURE_2D);
            glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 50);
        }

};

class FlatTile : public CubeTile {
    
    public:
        explicit FlatTile(char* filename, std::string texturename) : CubeTile(filename) {
            textures.clear();
            textureName = texturename;
            for (int k=0; k<6; k++)
                textures.push_back(textureName);
            scale.coord[1] = 0.1;
            useligthing = false;

            friction.coord[0] = 0.8;
            friction.coord[1] = 1;
            friction.coord[2] = 0.98;

        }

        float height_at(Point3 point) {
            return model.bbmax.Y()/scale.Y();
        }

        Vector3 normal_at(Point3 point) {
            return UP;
        }


};
