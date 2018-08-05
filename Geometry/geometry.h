#pragma once

#include <OpenGL/gl3.h>
#include <math.h>

// classe Point3: un punto (o vettore) in 3 dimensioni
class Point3 {
    public: 
  
        float coord[3]; // l'UNICO campo: le coordinate x, y, z
        
        float X() const { return coord[0]; }
        float Y() const { return coord[1]; }
        float Z() const { return coord[2]; }
        
        // costruttore 1
        Point3( float x, float y, float z ){
            coord[0]=x;
            coord[1]=y;
            coord[2]=z;
        }

        // costruttore vuoto
        Point3(){
            coord[0]=coord[1]=coord[2]=0;
        }
        
        
        // restituisce la versione di se stesso normalizzata
        inline Point3 Normalize() const{
            return (*this)/modulo();
        }

        // restituisce il modulo
        inline float modulo() const{
            return sqrt(coord[0]*coord[0] + coord[1]*coord[1] + coord[2]*coord[2]);
        }
        
        // operatore "/" binario: divisione per uno scalare (un float)
        inline Point3 operator / (float f)const{
            return Point3( 
            coord[0]/f,   
            coord[1]/f,   
            coord[2]/f   
            );
        }
        
        // operatore "-" unario: inversione del verso del vettore
        inline Point3 operator - ()const{
            return Point3( 
            -coord[0],   
            -coord[1],   
            -coord[2]   
            );
        }

        // operatore "-" binario: differenza fra punti
        inline Point3 operator - (const Point3 &a) const{
            return Point3( 
            coord[0]-a.coord[0],   
            coord[1]-a.coord[1],   
            coord[2]-a.coord[2]   
            );
        }
            
        // somma fra vettori  
        inline Point3 operator + (const Point3 &a) const{
            return Point3( 
            coord[0]+a.coord[0],   
            coord[1]+a.coord[1],   
            coord[2]+a.coord[2]   
            );
        }
        
        
        // ridefinisco l'operatore binario "%" per fare il CROSS PRODUCT
        inline Point3 operator % (const Point3 &a) const{
            return Point3( 
            coord[1]*a.coord[2]-coord[2]*a.coord[1],   
            -(coord[0]*a.coord[2]-coord[2]*a.coord[0]),   
            coord[0]*a.coord[1]-coord[1]*a.coord[0]   
            );
        }

        inline bool operator == (const Point3& other) const {
            return coord[0]==other.coord[0] && coord[1]==other.coord[1] && coord[2]==other.coord[2];
        }
        
        // mandare il punto come vertice di OpenGl
        inline void SendAsVertex() const{
            glVertex3fv(coord);
        }
        
        // mandare il punto come normale di OpenGl
        inline void SendAsNormal() const{
            glNormal3fv(coord);
        }
};

class Point2 : public Point3 {

    public:
        float Z() const { return 0; }
        Point2(float a, float b) : Point3(a,b,0) {}
        bool operator == (const Point2& other) const {
            return coord[0] == other.coord[0] && coord[1] == other.coord[1];
        }



};


// definiamo Vector3 come SINONIMO di Point3
// (solo per chiarezza, per distinguere nel codice fra punti e vettori)
typedef Point3 Vector3;

inline void glTranslate(Point3 v) {
  glTranslatef(v.X(), v.Y(), v.Z() );
}


// classe Vertex: 
class Vertex { 
    public: 
        Point3 p; // posizione

        // attributi per verice
        Vector3 n; // normale (per vertice)
};

class Edge {
    public: 
        Vertex* v[2]; // due puntatori a Vertice (i due estremi dell'edge)
        
        Edge(Vertex* a, Vertex*b) {
            v[0] = a;
            v[1] = b;
        }

        inline bool operator== (Edge other) {
            return (v[0]==other.v[0] && v[1]==other.v[1]) || (v[0]==other.v[1] && v[1] == other.v[0]);
    }

};

class Face {
    public: 
        Vertex* v[3]; // tre puntatori a Vertice (i tre vertici del triangolo)
        
        // costruttore
        Face(Vertex* a, Vertex* b, Vertex* c){
            v[0]=a; v[1]=b; v[2]=c;
        }
        
        // attributi per faccia
        Vector3 n; // normale (per faccia)
        
        // computa la normale della faccia
        inline void ComputeNormal() {
            n= -( (v[1]->p - v[0]->p) % (v[2]->p - v[0]->p) ).Normalize();
        }
};



