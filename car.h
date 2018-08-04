#include "controller.h"


class Car {

  void RenderAllParts(bool usecolor) const; 
                         // disegna tutte le parti della macchina
                         // invocato due volte: per la car e la sua ombra

public:
  // Metodi
  void Init(); // inizializza variabili
  void Render() const; // disegna a schermo
  void DoStep(); // computa un passo del motore fisico
  Car(){Init();} // costruttore
 
  Controller controller;  
  
  // STATO DELLA MACCHINA
  // (DoStep fa evolvere queste variabili nel tempo)
  float px,py,pz,facing; // posizione e orientamento
  float mozzoA, mozzoP, sterzo; // stato interno
  float vx,vy,vz; // velocita' attuale
  
  // STATS DELLA MACCHINA
  // (di solito rimangono costanti)
  float velSterzo, velRitornoSterzo, accMax, attrito,
        raggioRuotaA, raggioRuotaP, grip,
        attritoX, attritoY, attritoZ; // attriti
  
private:
  void DrawHeadlight(float x, float y, float z, int lightN, bool useHeadlight) const;
};
