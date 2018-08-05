#pragma once

#include "./Geometry/sMesh.h"
#include "controller.h"
#include "world.h"

extern bool useHeadlight;
extern bool useShadow;
extern bool useEnvmap;

class Vehicle {

    protected:
        /// disegna tutte le parti della macchina
        /// invocato due volte: per la car e la sua ombra
        virtual void RenderAllParts(bool usecolor) = 0;
        virtual void Init() = 0;

    public:
        // Metodi
        Controller controller;

        inline virtual void Render() {
            // sono nello spazio mondo
            glPushMatrix();
                
            glTranslatef(px,py,pz);
            glRotatef(facing, 0,1,0);

            // sono nello spazio MACCHINA

            RenderAllParts(true); 
            
            // ombra!
            if(useShadow)
            {
                glColor3f(0.4,0.4,0.4); // colore fisso
                glTranslatef(0,0.01,0); // alzo l'ombra di un epsilon per evitare z-fighting con il pavimento
                glScalef(1.01,0,1.01);  // appiattisco sulla Y, ingrandisco dell'1% sulla Z e sulla X 
                glDisable(GL_LIGHTING); // niente lighing per l'ombra
                RenderAllParts(false);  // disegno la macchina appiattita

                glEnable(GL_LIGHTING);
            } 
            glPopMatrix(); 
            
            glPopMatrix();
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
};


class MotorBike : public Vehicle {

    private:
        float vxm, vym, vzm; // velocità nello spazio veicolo
        World* world_reference;

    protected:
        sMesh* carlinga = new sMesh((char*)"./Resources/Bike/carlinga.obj");
        sMesh* back_wheel = new sMesh((char*) "./Resources/Bike/backwheel.obj");
        sMesh* front_wheel = new sMesh((char*) "./Resources/Bike/frontwheel.obj");

        void RenderAllParts(bool usecolor) {
            // disegna la carliga con una mesh
            glPushMatrix();
            glScalef(4, 4, 4);
            glRotatef(90, 0, 1, 0);

            glRotatef( sterzo * vzm / 0.12 , 1, 0, 0); // inclinare la moto durante le curve
        
            // ------- Wheels ----------
            glPushMatrix();

            glTranslate( front_wheel->Center());  //rotazione ruota
            glRotatef(mozzoA, 0, 0, 1); 
            glTranslate( -front_wheel->Center());
            front_wheel->RenderNxV();
            glPopMatrix();

            glPushMatrix();
            glTranslate( back_wheel->Center());  //rotazione ruota
            glRotatef(mozzoP, 0, 0, 1); 
            glTranslate( -back_wheel->Center());
            back_wheel->RenderNxV();
            glPopMatrix();



            if (usecolor) glEnable(GL_LIGHTING);
            
            // glPushMatrix();
            // glTranslate( front_wheel->Center() );
            // glScalef(4,4,4);
            // glRotatef( sterzo, 0, 1, 0);
            // glRotatef(-mozzoA, 1, 0, 0);
            // glTranslate( -front_wheel->Center());
            
            // if (usecolor) glColor3f(.6,.6,.6);
            // //if (usecolor) SetupWheelTexture(wheelFR1.bbmin,wheelFR1.bbmax);
            // front_wheel->RenderNxF(); // la ruota viene meglio FLAT SHADED - normali per faccia
            //                         // provare x credere
            // glDisable(GL_TEXTURE_2D);
            // if (usecolor) glColor3f(0.9,0.9,0.9);
            // front_wheel->RenderNxV();
            // glPopMatrix();
        
            // glPushMatrix();
            // glScalef(4,4,4);
            // glTranslate(  back_wheel->Center() );
            // glRotatef(-mozzoA,1,0,0);
            // glTranslate( -back_wheel->Center() );

            // if (usecolor) glColor3f(.6,.6,.6);
            // // if (usecolor) SetupWheelTexture(wheelBR1.bbmin,wheelBR1.bbmax);
            // back_wheel->RenderNxF();
            // glDisable(GL_TEXTURE_2D);
            // if (usecolor) glColor3f(0.9,0.9,0.9);
            // back_wheel->RenderNxV();
            // glPopMatrix();
            
            carlinga->RenderNxV(); // rendering delle mesh carlinga usando normali per vertice

            glPopMatrix();
        }

    public:

        MotorBike(World* w) { 
            Init(); 
            world_reference = w;
        }

        void Init() {
            // inizializzo lo stato della macchina
            px =  pz = facing = 0; // posizione e orientamento
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
            py=h;
        
        } 

};
