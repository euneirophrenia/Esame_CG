#pragma once

#include "./Geometry/sMesh.h"
#include <cmath>

class Tile {

    protected:
        Point3 center; //position of the center of the tile in world coordinates (the origin of the tile-coordinates)
        float rotation = 0; //rotation between world and tile-coords
        Vector3 scale = Vector3(1,1,1);

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

        inline bool hasInside(Point3 point){
            Point3 max = center + (model.bbmax*scale);
            Point3 min = center + (model.bbmin*scale);
            return point.X() <= max.X() && point.X() >= min.X() &&
                    point.Z() <= max.Z() && point.Z() >= min.Z();
        }

        explicit Tile(char* model_path) {
            model.Init(model_path);
        }

        inline void Draw() {
            glPushMatrix();
                glTranslatef(center.X(), 0, center.Y());
                glRotatef(rotation, 0, 1, 0);
                glScalef(scale.X(), scale.Y(), scale.Z());
                model.RenderArray();
            glPopMatrix();
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
            return Vector3(0,1,0);
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
        
            //compute the actual height, according to the custom shape i made
            return center.Y() + pow(1.5, -abs(tile_x) + 5); //don't ask why
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

            if (tile_x >=0) 
                return Vector3(-pow(1.5, -tile_x + 5)*log(1.5), -1, 0);

            return Vector3(pow(1.5, tile_x + 5)*log(1.5), -1, 0);
        
        }

};

