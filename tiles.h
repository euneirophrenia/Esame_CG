#pragma once

#include "./Geometry/sMesh.h"
#include "utils.h"
#include <cstdlib>
#include <cmath>

extern bool useShadow;
extern bool useTransparency;
extern bool useHeadlight;
extern bool stopTime;
extern bool dead;
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
        Point3 model_center, max, min;
        
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

        inline void UpdateModel() {
        }

    public:
        Point3 center; //position of the center of the tile in world coordinates (the origin of the tile-coordinates)
        float rotation = 0; //rotation between world and tile-coords
        sMesh model;
        bool becomesTransparent = false;

        inline void Translate(Vector3 vect) {
            center = center + vect;
            max = center + (model.bbmax*scale);
            min = center + (model.bbmin*scale);
        }


        inline Vector3 getScale() {
            return scale;
        }

        inline void Translate(float x, float y, float z) {
            center.coord[0] += x;
            center.coord[1] += y;
            center.coord[2] += z;

            max = center + (model.bbmax*scale);
            min = center + (model.bbmin*scale);

        }

        //assume only rotation on the horizontal plane, can't rotate a tile in 3D, just use a different tile
        inline void Rotate(float angle) {
            rotation+=angle;
        }

        inline void Scale(float sx, float sy, float sz) {
            scale.coord[0] *= sx;
            scale.coord[1] *= sy;
            scale.coord[2] *= sz;

            max = center + (model.bbmax*scale);
            min = center + (model.bbmin*scale);

        }

        inline void Scale(Vector3 s) {
            Scale(s.coord[0], s.coord[1], s.coord[2]);
        }

        void BindBuffers() {
            model.BindBuffers();
        }

        void FreeBuffers() {
            model.FreeBuffers();
        }


        virtual bool hasInside(Point3 point) {
            /* TODO: Handle rotations, this code right here below is bugged, and the one in use doesn't account for rotations*/
            /* Also, it shouldn't in principle be run at every single check, the actual bbox should be update with every transformation.
             * But, alas, here we are
             */
            // float cosine = cos(rotation), sine = sin(rotation);
            // model_center = ((scale*model.bbmax)*0.5) + ((scale*model.bbmin) *0.5);
            // Point3 actual_max = model.bbmax - model_center;
            // Point3 actual_min = model.bbmin - model_center;
            // actual_max.coord[0] = cosine * actual_max.X() + sine * actual_max.Z();
            // actual_max.coord[2] = -sine * actual_max.X() + cosine * actual_max.Z();

            // actual_min.coord[0] = cosine * actual_min.X() + sine * actual_min.Z();
            // actual_min.coord[2] = -sine * actual_min.X() + cosine * actual_min.Z();

            // Point3 max = scale*(actual_max + model_center) + center;
            // Point3 min = scale*(actual_min + model_center) + center;
            
            return point.X() <= max.X() && point.X() >= min.X() &&
                    point.Z() <= max.Z() && point.Z() >= min.Z();
        }

        explicit Tile(char* model_path, std::string texture = "Resources/wood.ppm") {
            model.Init(model_path);
            center.coord[0]=0;
            center.coord[1]=0;
            center.coord[2]=0;
            textureName = texture;

            model_center = (model.bbmax + model.bbmin)/2;
            max = center + (model.bbmax*scale);
            min = center + (model.bbmin*scale);
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
            Translate(velocity, 0, 0);
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

    public:
        explicit CubeTile(char* filename) : Tile(filename) {
            textureName = "Resources/logo.ppm";
            textures.push_back(textureName);
            textures.push_back("Resources/dice1.ppm");
            textures.push_back("Resources/dice2.ppm");
            textures.push_back("Resources/dice2.ppm");
            textures.push_back("Resources/dice3.ppm");
            textures.push_back("Resources/dice4.ppm");
            uselight = true;

        }
        float height_at(Point3 point) {
            return 2*scale.Y();
        }

        Vector3 normal_at(Point3 point) {
            return UP;
        }

        virtual void Draw() {
            
            glPushMatrix();
                glColor3f(0.9, 0.9, 0.9);
                glTranslatef(center.X(), center.Y(), center.Z());
                glRotatef(rotation, 0, 1, 0);
                glScalef(scale.X(), scale.Y(), scale.Z());
                glDisable(GL_CULL_FACE);
                if (!useWireframe) {
                      DrawCube(textures, uselight);
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
            uselight = false;

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


class PlotTwist : public Tile {
    protected:
        float v = 0.1f; //how fast to follow the player
        Vector3 velocity;
        const float max_dist = 15;
        const float safety_margin = 2; // to prevent monster staggering

        inline void DoPhysics() {
            if (stopTime) return;
            
            float dist=(player_position - center).modulo();
            if (dist < max_dist && useHeadlight) {
                velocity = (player_position - center) * (dist - max_dist) / max_dist; //for fun, when the monster is close make it run away super fast from light
                velocity.coord[1] = 0; //to prevent some weird phenomena
            }
            else {
                    if (!useHeadlight)
                        velocity = (player_position - center) / dist * v;
            }
            
            if (v!=0){
                // Rotate to face the player
                glRotatef(180.0*atan2(velocity.X(), velocity.Z())/M_PI, 0, 1, 0); //horizontal orientation
                glRotatef(180.0*atan2(velocity.Y(), sqrt(velocity.coord[0] * velocity.coord[0] + velocity.coord[2]*velocity.coord[2]))/M_PI, 0 , 0, 1); //vertical
            }

            //---- Follow him
            Translate(velocity.X(), velocity.Y(), velocity.Z());

            if ((player_position - center).modulo() < 1) {
                v = 0;
                dead = true;
            }
        }

        inline virtual void DrawShadow() {

            float dist=(player_position - center).modulo();
            if (!useHeadlight || dist > 20)
                glColor4f(0.1, 0.1, 0.1, 0.6);
            else
                glColor4f(0.7 - dist/30, 0.7 - dist/30, 0.7 - dist /30, dist/30); // colore non proprio fisso
            glPushMatrix();
                glTranslatef(0, -center.Y() + 0.01,  0); // alzo l'ombra di un epsilon per evitare z-fighting con il pavimento
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

    public:
        explicit PlotTwist(char* body, std::string texturename): Tile(body) {
            basecolor = Vector3(61, 1, 86) / 255.0f;
            textureName = texturename;
        }

        float height_at(Point3 point) {
            return 1337; //instant death upon touching
        }

        Vector3 normal_at(Point3 point) {
            return UP;
        }

        inline void Draw() {
            glPushMatrix();
                glColor3f(1, 1, 1);

                glTranslatef(center.X(), center.Y(), center.Z());
                glScalef(scale.X(), scale.Y(), scale.Z());      

                DoPhysics();
  
                if (useShadow) {
                    DrawShadow();
                }    
               
                   
                if (!useWireframe) {
                    //glColor4f(basecolor.X(), basecolor.Y(), basecolor.Z(), 0.4);
                    glColor4f(1,1,1, 0.4);
                    glEnable(GL_BLEND);
                    glDisable(GL_DEPTH);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    texProvider->SetupAutoTexture2D(textureName, model.bbmax, model.bbmin);

                }
                else {
                    glColor3f(1,1,1);
                }
                model.RenderArray();
                
            glPopMatrix();
            glDisable(GL_BLEND);
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
            glDisable(GL_TEXTURE_2D);
            glEnable(GL_DEPTH);
        }

        void DrawMiniMarker() {
            drawCircle(0.0005);
        }
            

};
