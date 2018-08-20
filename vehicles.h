#pragma once

#include "./Geometry/sMesh.h"
#include "controller.h"
#include "world.h"
#include "utils.h"

extern bool useHeadlight;
extern bool useShadow;
extern bool useEnvmap;
extern int cameraType;

const Vector3 SOLDIER_GREEN(31.0/255, 112.0/255, 73.0/255);
const Vector3 HEAD_HACK(38/255.0f, 25/255.0f, 16/255.0f);

class Vehicle {

    protected:
        /// disegna tutte le parti della macchina
        /// invocato due volte: per la car e la sua ombra
        virtual void RenderAllParts(bool usecolor) = 0;
        virtual void Init() = 0;
        float current_ground_level =0;

        void DrawHeadlight(float x, float y, float z, int lightN, bool useHeadlight) {
            int usedLight=GL_LIGHT1 + lightN;
            
            if(useHeadlight)
            {
                glEnable(usedLight);
                
                //float col0[4]= {0.8, 0.8,0.0,  1};
                glLightfv(usedLight, GL_DIFFUSE, FOUR_1);
                
                float col1[4]= {0.5,0.5,0.0,  1};
                glLightfv(usedLight, GL_AMBIENT, col1);
                
                float tmpPos[4] = {x,y,z,  1}; // ultima comp=1 => luce posizionale
                glLightfv(usedLight, GL_POSITION, tmpPos );
                
                float tmpDir[4] = {0,0,-1,  0}; // ultima comp=1 => luce posizionale
                glLightfv(usedLight, GL_SPOT_DIRECTION, tmpDir );
                
                glLightf (usedLight, GL_SPOT_CUTOFF, 30);
                glLightf (usedLight, GL_SPOT_EXPONENT, 5);
                
                glLightf(usedLight,GL_CONSTANT_ATTENUATION, 0);
                glLightf(usedLight,GL_LINEAR_ATTENUATION, 0.6);
            }
            else
                glDisable(usedLight);
        }

    public:
        // Metodi
        Controller controller;

        

        inline virtual void Render() {
            // sono nello spazio mondo
            glPushMatrix();
                
            glTranslatef(px,py,pz);
            glRotatef(facing, 0, 1, 0);

            // sono nello spazio MACCHINA

            DrawHeadlight(-0.3, 0, -1, 0, useHeadlight); // accendi faro sinistro
            DrawHeadlight(+0.3, 0, -1, 1, useHeadlight); // accendi faro destro

            RenderAllParts(true); 
            
            // ombra!
            if(useShadow)
            {
                glColor3f(0.1, 0.1, 0.1); // colore fisso
                glTranslatef(0, -py+current_ground_level + 0.01,0); // alzo l'ombra di un epsilon per evitare z-fighting con il pavimento
                glScalef(1.01,0,1.01);  // appiattisco sulla Y, ingrandisco dell'1% sulla Z e sulla X 
                glDisable(GL_LIGHTING); // niente lighting per l'ombra
                RenderAllParts(false);  // disegno la macchina appiattita

                glEnable(GL_LIGHTING);
            } 
            glPopMatrix(); 
            
        }

        float Velocity() {
            return Vector3(vx, vy, vz).modulo();
        }

        

        virtual void DoStep() = 0; // computa un passo del motore fisico
        
        // STATO DEL VEICOLO
        // (DoStep fa evolvere queste variabili nel tempo)
        float px,py,pz,facing; // posizione e orientamento
        float vx,vy,vz; // velocita' attuale
        float velSterzo, velRitornoSterzo;
        float accMax, attritoX, attritoY, attritoZ, grip;
        float raggioRuotaA, raggioRuotaP;
        float mozzoA, mozzoP, sterzo;
        float step = 0.07;
};


class MotorBike : public Vehicle {

    private:
        float vxm, vym, vzm; // velocità nello spazio veicolo
        Vector3 normaldirection;
        float inclination;
        World* world_reference;
        float length;
        float MAX_STEP;

    protected:
        sMesh* carlinga = new sMesh((char*)"./Resources/Bike/carlinga.obj");
        sMesh* back_wheel = new sMesh((char*) "./Resources/Bike/backwheel.obj");
        sMesh* front_wheel = new sMesh((char*) "./Resources/Bike/frontwheel.obj");
        sMesh* plate = new sMesh((char*) "./Resources/Bike/plate.obj");

        sMesh* pilot = new sMesh((char*) "./Resources/Bike/body3.obj");
        sMesh* face  = new sMesh((char*) "./Resources/Bike/head2.obj");
        sMesh* helmet = new sMesh((char*) "./Resources/Bike/helmet2.obj");
        

        void RenderAllParts(bool usecolor) {
            // disegna la carliga con una mesh
            glPushMatrix();
            glScalef(4, 4, 4);
            glRotatef(90, 0, 1, 0);

            glRotatef(inclination, normaldirection.X(), normaldirection.Y(), normaldirection.Z());

            glRotatef( sterzo * -abs(vzm)  / 0.12 , 1, 0, 0); // tilt when stirring
        
            // ------- Wheels ----------
            glPushMatrix();
                glTranslate( front_wheel->Center());  // wheel rotation
                glRotatef(mozzoA, 0, 0, 1); 
                glTranslate( -front_wheel->Center());
                if (usecolor){
                    glColor3f(.6,.6,.6);
                    SetupTexture(6, front_wheel->bbmin, front_wheel->bbmax);
                    glColor3f(0.9,0.9,0.9);
                }
                front_wheel->RenderNxF();
                glDisable(GL_TEXTURE_2D);
            glPopMatrix();

            glPushMatrix();
                glTranslate( back_wheel->Center());  // wheel rotation
                glRotatef(mozzoP, 0, 0, 1); 
                glTranslate( -back_wheel->Center());

                if (usecolor) {
                    glColor3f(.6,.6,.6);
                    SetupTexture(6, back_wheel->bbmin, back_wheel->bbmax);
                    glColor3f(0.9,0.9,0.9);
                }
                back_wheel->RenderNxV();
                glDisable(GL_TEXTURE_2D);
            glPopMatrix();

            if (usecolor) {
                glEnable(GL_LIGHTING);
                glColor3f(0.5, 0.6, 0.5);
            }

            carlinga->RenderArray(); // rendering delle mesh carlinga usando normali per vertice
            if (usecolor) {
                glColor3f(1,1,1);
                SetupTexture(7, plate->bbmin, plate->bbmax);
            }
            plate->RenderNxF();
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);


            if (usecolor) {
                //glColor3f(1,1,1);
                //SetupTexture(3, pilot->bbmin, pilot->bbmax);
                glColor3f(SOLDIER_GREEN.X(), SOLDIER_GREEN.Y(), SOLDIER_GREEN.Z());
            }
            pilot->RenderArray(); // to be more efficient
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);

            if (usecolor) {
                glColor3f(1,1,1);
                //don't bother using the face texture if nobody can see it
                if (cameraType == CAMERA_FRONT || cameraType == CAMERA_MOUSE)
                    SetupTexture(4, face->bbmin, face->bbmax, GL_OBJECT_LINEAR);
                else 
                    glColor3f(HEAD_HACK.X(), HEAD_HACK.Y(), HEAD_HACK.Z());
            }
            face->RenderArray();
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);

            if(usecolor) {
                glColor3f(SOLDIER_GREEN.X(), SOLDIER_GREEN.Y(), SOLDIER_GREEN.Z());
            }
            helmet->RenderArray();

            glPopMatrix();
        }

    public:
        bool crashed = false;

        MotorBike(World* w) { 
            Init(); 
            world_reference = w;
        }

        void Init() {
            // inizializzo lo stato della macchina
            px = facing = 0; // posizione e orientamento
            pz = 10;
            py = 0.0;
            
            vx=vy=vz=0;      // velocita' attuale

            mozzoA = mozzoP = sterzo =0;

            // inizializzo la struttura di controllo
            controller.Init();
            
            //velSterzo=3.4;         // A
            velSterzo=2.4;         // A
            velRitornoSterzo=0.93; // B, sterzo massimo = A*B / (1-B)
            
            accMax = 0.0011;
            
            // attriti: percentuale di velocita' che viene mantenuta
            // 1 = no attrito
            // <<1 = attrito grande
            attritoZ = 0.991;  // piccolo attrito sulla Z (nel senso di rotolamento delle ruote)
            attritoX = 0.8;  // grande attrito sulla X (per non fare slittare la macchina)
            attritoY = 1.0;  // attrito sulla y nullo
            
            // Nota: vel max = accMax*attritoZ / (1-attritoZ)
            
            raggioRuotaA = 0.25;
            raggioRuotaP = 0.35;
            
            grip = 0.45; // quanto il facing macchina si adegua velocemente allo sterzo

            length = 4*(carlinga->bbmax.X() - carlinga->bbmin.X());
            MAX_STEP = length / 2 ;

        }

        void BindBuffers() {
            back_wheel->BindBuffers(GL_DYNAMIC_DRAW);
            front_wheel->BindBuffers(GL_DYNAMIC_DRAW);
            carlinga->BindBuffers(GL_DYNAMIC_DRAW);
            pilot->BindBuffers(GL_STATIC_DRAW);
            face->BindBuffers(GL_STATIC_DRAW);
            plate->BindBuffers(GL_STATIC_DRAW);
            helmet->BindBuffers(GL_STATIC_DRAW);
        }

        inline void DoStep() {
            // da vel frame mondo a vel frame veicolo
            float cosf = cos(facing*M_PI/180.0);
            float sinf = sin(facing*M_PI/180.0);
            vxm = +cosf*vx - sinf*vz;
            vym = vy;
            vzm = +sinf*vx + cosf*vz;
            
            // gestione dello sterzo
            if (controller.key[Controller::LEFT]) sterzo+=velSterzo;
            if (controller.key[Controller::RIGHT]) sterzo-=velSterzo;
            sterzo *= velRitornoSterzo; // ritorno a volante dritto
            
            if (controller.key[Controller::ACC]) vzm -= accMax; // accelerazione in avanti 

            if (controller.key[Controller::DEC]) vzm += accMax; // accelerazione indietro
            
            // attirti (semplificando)
            vxm*=attritoX;  
            vym*=attritoY;
            vzm*=attritoZ;

            vzm+=0.000001; // #hacks così che la moto non sparisca a causa di NaN later on
            
            // l'orientamento della macchina segue quello dello sterzo
            // (a seconda della velocita' sulla z)
            facing = facing - (vzm*grip)*sterzo;

            //rotazione mozzo ruote (a seconda della velocita' sulla z)
            float da ; //delta angolo
            da=(360.0*vzm)/(2.0*M_PI*raggioRuotaA);
            mozzoA+=da;
            da=(360.0*vzm)/(2.0*M_PI*raggioRuotaP);
            mozzoP+=da;
            
            // ritorno a vel coord mondo
            vx = +cosf*vxm + sinf*vzm;
            vy = vym;
            vz = -sinf*vxm + cosf*vzm;
            
            // posizione = posizione + velocita * delta t (ma delta t e' costante)
            px+=vx;
            py+=vy;
            pz+=vz;

            float h = world_reference->height_at(px, pz);
            current_ground_level = h;
            if (py<=h && h-py <= MAX_STEP) { //se si può salire lo scalino
                py=h; 
            }
            if (h-py > MAX_STEP){ //crash su muro
                vx=0;
                vy=0;
                vz=0;
                crashed = true;
            }
            if (py > h + step) {
                py -= step;
                inclination *= 0.99; //essenzialmente la velocità di ritorno dello sterzo.
            }
            else {
                auto tmp = world_reference->normal_at(Point3(px, py, pz));
                auto velocity = Vector3(vx, vy, vz).Normalize();
                auto dotp = tmp*velocity; 
                dotp = dotp / tmp.modulo();
                inclination = dotp.coord[0] + dotp.coord[1] + dotp.coord[2] + 0.00000000001;
                inclination = 90 - (180 * acos(inclination) / M_PI);
                normaldirection = Vector3(0, 0, vzm >=0 ? -1 : 1);
            }

            if (abs(px) > 98 || abs(pz) > 98)
                crashed=true;
        } 

        void Log() {
            printf("- Position     :\t(%f, %f, %f)\n", px, py, pz);
            printf("- Velocity     :\t(%f, %f, %f)\n", vx, vy, vz);
            printf("- Skew         :\t%f\n", inclination);
        }

};
