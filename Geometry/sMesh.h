#pragma once

#include <string.h>
#include <vector>

#include "../common.h"
#include "geometry.h"

/*
    My own improvement to the Mesh class provided by the teacher. 
    It "should" be more efficient
*/

extern bool useWireframe;
extern bool useBadWireFrame;
extern bool useTransparency;

class sMesh {

    protected:
        std::vector<Vertex> v; // vettore di vertici
        std::vector<Face> f;   // vettore di facce
        std::vector<Edge> e;   // vettore di edge
        std::vector<Point3> points;
        std::vector<Vector3> normals; 
        GLuint vertexBuffer, normalBuffer;
                           
    public:  
        // centro del axis aligned bounding box
        Point3 Center(){ return (bbmin+bbmax)/2.0; };
        Point3 bbmin,bbmax; // bounding box 

        inline int number_of_vertices() {
            return v.size();
        }
        inline int number_of_faces() {
            return f.size();
        }

        inline std::vector<Vertex> vertices() {
            return v;
        }


        sMesh() {
            
        }

        // costruttore con caricamento
        sMesh(char* filename) {
            Init(filename);
        }

        void Init(char* filename) {
            LoadFromObj(filename);
            ComputeNormalsPerFace();
            ComputeNormalsPerVertex();
            ComputeBoundingBox();
            populateBuffers();

        }

        void BindBuffers(GLenum mode = GL_STATIC_DRAW) {
            glGenBuffers( 1, &vertexBuffer );
            glGenBuffers(1, &normalBuffer);

            // Create and initialize a buffer object
            glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
            glBufferData(GL_ARRAY_BUFFER, sizeof(Point3)*points.size(), points.data(), mode); // trust me

            glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
            glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3)*points.size(), normals.data(), mode);
        }

        void FreeBuffers() {
            glDeleteBuffers(1, &vertexBuffer);
            glDeleteBuffers(1, &normalBuffer);
        }

        void ComputeNormalsPerFace() {

            for (int i=0; i<f.size(); i++){
                f[i].ComputeNormal();
            }
        }

        // Computo normali per vertice
        // (come media rinormalizzata delle normali delle facce adjacenti)
        void ComputeNormalsPerVertex() {

            // uso solo le strutture di navigazione FV (da Faccia a Vertice)!
            // fase uno: ciclo sui vertici, azzero tutte le normali
            for (int i=0; i<v.size(); i++) {
                v[i].n = Point3(0,0,0);
            }
            
            // fase due: ciclo sulle facce: accumulo le normali di F nei 3 V corrispondenti
            for (int i=0; i<f.size(); i++) {
                f[i].v[0]->n=f[i].v[0]->n + f[i].n;
                f[i].v[1]->n=f[i].v[1]->n + f[i].n;
                f[i].v[2]->n=f[i].v[2]->n + f[i].n;
            }
            
            // fase tre: ciclo sui vertici; rinormalizzo
            // la normale media rinormalizzata e' uguale alla somma delle normnali, rinormalizzata
            for (int i=0; i<v.size(); i++) {
                v[i].n = v[i].n.Normalize();
            }
        
        }

        inline void RenderArray() {
            glEnableClientState(GL_NORMAL_ARRAY);
        
            glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
            glNormalPointer(GL_FLOAT, 0, BUFFER_OFFSET(0));

            glEnableClientState(GL_VERTEX_ARRAY);
            glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
            glVertexPointer(3, GL_FLOAT, sizeof(Point3), BUFFER_OFFSET(0));
           
            if (useWireframe || useBadWireFrame) {
                glDisable(GL_TEXTURE_2D);
                glColor3f(0.5, 0.5, 0.5);
                // glDrawArrays(GL_LINES, 0, sizeof(Point3)*v.size());
                // glColor3f(0.8,0.8,0.8);
                glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
                if (useBadWireFrame) {
                    glDrawArrays(GL_TRIANGLES, 0, sizeof(Point3)*v.size());
                    glColor3f(0.8, 0.8, 0.8);
                }
            }
            if (!useWireframe || useBadWireFrame)
                glPolygonMode( GL_FRONT_AND_BACK, GL_FILL);
            glDrawArrays(GL_TRIANGLES, 0, sizeof(Point3)*v.size());
            glDisableClientState(GL_VERTEX_ARRAY);
            glDisableClientState(GL_NORMAL_ARRAY);
            //glColor3f(1,1,1);
        }

        void RenderWire() {
            glLineWidth(0.4);
            glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
            // (nota: ogni edge viene disegnato due volte. 
            // sarebbe meglio avere ed usare la struttura edge)
            for (int i=0; i<f.size(); i++) 
            {
                glBegin(GL_LINE_LOOP);
                f[i].n.SendAsNormal();    
                (f[i].v[0])->p.SendAsVertex();   
                (f[i].v[1])->p.SendAsVertex();   
                (f[i].v[2])->p.SendAsVertex();
                glEnd();
            }
            glPolygonMode( GL_FRONT_AND_BACK, GL_FILL);
        }

        // Render usando la normale per faccia (FLAT SHADING)
        void RenderNxF() {
            if (useWireframe) {
                glDisable(GL_TEXTURE_2D);
                glColor3f(.5,.5,.5);
                RenderWire(); 
                glColor3f(1,1,1);
            }

            if (!useWireframe || useBadWireFrame) {
            
                // mandiamo tutti i triangoli a schermo
                glBegin(GL_TRIANGLES);
                for (int i=0; i<f.size(); i++) 
                {
                    f[i].n.SendAsNormal(); // flat shading
                    
                    (f[i].v[0])->p.SendAsVertex();
                    
                    (f[i].v[1])->p.SendAsVertex();
                    
                    (f[i].v[2])->p.SendAsVertex();
                }
                glEnd();
            }
        }

        // Render usando la normale per vertice (GOURAUD SHADING)
        void RenderNxV() {

            if (useWireframe) {
                glDisable(GL_TEXTURE_2D);
                glColor3f(.5,.5,.5);
                RenderWire(); 
                glColor3f(1,1,1);
            }

            if (!useWireframe || useBadWireFrame) {
                // mandiamo tutti i triangoli a schermo
                glBegin(GL_TRIANGLES);
                for (int i=0; i<f.size(); i++) 
                {    
                    (f[i].v[0])->n.SendAsNormal(); // gouroud shading (o phong?)
                    (f[i].v[0])->p.SendAsVertex();
                    
                    (f[i].v[1])->n.SendAsNormal();
                    (f[i].v[1])->p.SendAsVertex();
                    
                    (f[i].v[2])->n.SendAsNormal();
                    (f[i].v[2])->p.SendAsVertex();
                }
                glEnd();
            }
        }

        // trova l'AXIS ALIGNED BOUNDIG BOX
        void ComputeBoundingBox() {
            // basta trovare la min x, y, e z, e la max x, y, e z di tutti i vertici
            // (nota: non e' necessario usare le facce: perche?) 
            if (!v.size()) return;
            bbmin=bbmax=v[0].p;
            for (int i=0; i<v.size(); i++){
                for (int k=0; k<3; k++) {
                    if (bbmin.coord[k]>v[i].p.coord[k]) bbmin.coord[k]=v[i].p.coord[k];
                    if (bbmax.coord[k]<v[i].p.coord[k]) bbmax.coord[k]=v[i].p.coord[k];
                }
            }
        }

        // carica la mesh da un file in formato Obj
        //   Nota: nel file, possono essere presenti sia quads che tris
        //   ma nella rappresentazione interna (classe Mesh) abbiamo solo tris.
        //
        bool LoadFromObj(char* filename) {
        
            FILE* file = fopen(filename, "rt"); // apriamo il file in lettura
            if (!file) return false;

            //make a first pass through the file to get a count of the number
            //of vertices, normals, texcoords & triangles 
            char buf[128];
            int nv,nf,nt;
            float x,y,z;
            int va,vb,vc,vd;
            int na,nb,nc,nd;
            int ta,tb,tc,td;

            nv=0; nf=0; nt=0;
            while(fscanf(file, "%s", buf) != EOF) {
                    switch(buf[0]) {
                    case '#':               // comment
                        // eat up rest of line
                        fgets(buf, sizeof(buf), file);
                        break;
                    case 'v':               // v, vn, vt
                        switch(buf[1]) {
                        case '\0':          // vertex
                            // eat up rest of line 
                            fgets(buf, sizeof(buf), file);
                            nv++;
                            break;
                        default:
                            break;
                        }
                        break;
                    case 'f':               // face
                            fscanf(file, "%s", buf);
                            // can be one of %d, %d//%d, %d/%d, %d/%d/%d %d//%d
                            if (strstr(buf, "//")) {
                                // v//n
                                sscanf(buf, "%d//%d", &va, &na);
                                fscanf(file, "%d//%d", &vb, &nb);
                                fscanf(file, "%d//%d", &vc, &nc);
                                nf++;
                                nt++;
                                while(fscanf(file, "%d//%d", &vd, &nd) > 0) {
                                    nt++;
                                }
                            } else if (sscanf(buf, "%d/%d/%d", &va, &ta, &na) == 3) {
                                // v/t/n
                                fscanf(file, "%d/%d/%d", &vb, &tb, &nb);
                                fscanf(file, "%d/%d/%d", &vc, &tc, &nc);
                                nf++;
                                nt++;
                                while(fscanf(file, "%d/%d/%d", &vd, &td, &nd) > 0) {
                                    nt++;
                                }
                            } else if (sscanf(buf, "%d/%d", &va, &ta) == 2) {
                                // v/t
                                fscanf(file, "%d/%d", &vb, &tb);
                                fscanf(file, "%d/%d", &vc, &tc);
                                nf++;
                                nt++;
                                while(fscanf(file, "%d/%d", &vd, &td) > 0) {
                                    nt++;
                                }
                            } else {
                                // v
                                fscanf(file, "%d", &va);
                                fscanf(file, "%d", &vb);
                                nf++;
                                nt++;
                                while(fscanf(file, "%d", &vc) > 0) {
                                    nt++;
                                }
                            }
                            break;

                        default:
                            // eat up rest of line
                            fgets(buf, sizeof(buf), file);
                            break;
                    }
            }
            
            //printf("dopo FirstPass nv=%d nf=%d nt=%d\n",nv,nf,nt);
            
            // allochiamo spazio per nv vertici
            v.resize(nv);

            // rewind to beginning of file and read in the data this pass
            rewind(file);
            
            //on the second pass through the file, read all the data into the
            //allocated arrays

            nv = 0;
            nt = 0;
            while(fscanf(file, "%s", buf) != EOF) {
                switch(buf[0]) {
                case '#':               // comment
                        // eat up rest of line
                        fgets(buf, sizeof(buf), file);
                        break;
                case 'v':               // v, vn, vt
                        switch(buf[1]) {
                        case '\0':          // vertex
                            fscanf(file, "%f %f %f", &x, &y, &z);
                            v[nv].p = Point3( x,y,z );
                            nv++;
                            break;
                        default:
                            break;
                        }
                        break;
                case 'f':               // face
                    fscanf(file, "%s", buf);
                    // can be one of %d, %d//%d, %d/%d, %d/%d/%d %d//%d
                    if (strstr(buf, "//")) {
                        // v//n
                        sscanf(buf, "%d//%d", &va, &na);
                        fscanf(file, "%d//%d", &vb, &nb);
                        fscanf(file, "%d//%d", &vc, &nc);
                        va--; vb--; vc--;
                        Face newface( &(v[va]), &(v[vc]), &(v[vb]) ); // invoco il costruttore di faccia
                        f.push_back( newface ); // inserico la nuova faccia in coda al vettore facce
                        nt++;
                        vb=vc;
                        while(fscanf(file, "%d//%d", &vc, &nc) > 0) {
                            vc--;
                            Face newface( &(v[va]), &(v[vc]), &(v[vb]) ); // invoco il costruttore di faccia
                            f.push_back( newface ); // inserico la nuova faccia in coda al vettore facce
                            nt++;
                            vb=vc;
                        }
                    } else if (sscanf(buf, "%d/%d/%d", &va, &ta, &na) == 3) {
                            // v/t/n
                            fscanf(file, "%d/%d/%d", &vb, &tb, &nb);
                            fscanf(file, "%d/%d/%d", &vc, &tc, &nc);
                            va--; vb--; vc--;
                            Face newface( &(v[va]), &(v[vc]), &(v[vb]) ); // invoco il costruttore di faccia
                            f.push_back( newface ); // inserico la nuova faccia in coda al vettore facce
                            nt++;
                            vb=vc;
                            while(fscanf(file, "%d/%d/%d", &vc, &tc, &nc) > 0) {
                            vc--;
                            Face newface( &(v[va]), &(v[vc]), &(v[vb]) ); // invoco il costruttore di faccia
                            f.push_back( newface ); // inserico la nuova faccia in coda al vettore facce
                            nt++;
                            vb=vc;
                            }
                    } else if (sscanf(buf, "%d/%d", &va, &ta) == 2) {
                            // v/t
                            fscanf(file, "%d/%d", &va, &ta);
                            fscanf(file, "%d/%d", &va, &ta);
                            va--; vb--; vc--;
                            Face newface( &(v[va]), &(v[vc]), &(v[vb]) ); // invoco il costruttore di faccia
                            f.push_back( newface ); // inserico la nuova faccia in coda al vettore facce
                            nt++;
                            vb=vc;
                            while(fscanf(file, "%d/%d", &vc, &tc) > 0) {
                            vc--;
                            Face newface( &(v[va]), &(v[vc]), &(v[vb]) ); // invoco il costruttore di faccia
                            f.push_back( newface ); // inserico la nuova faccia in coda al vettore facce
                            nt++;
                            vb=vc;
                            }
                    } else {
                            // v
                            sscanf(buf, "%d", &va);
                            fscanf(file, "%d", &vb);
                            fscanf(file, "%d", &vc);
                            va--; vb--; vc--;
                            Face newface( &(v[va]), &(v[vc]), &(v[vb]) ); // invoco il costruttore di faccia
                            f.push_back( newface ); // inserico la nuova faccia in coda al vettore facce
                            nt++;
                            vb=vc;
                            while(fscanf(file, "%d", &vc) > 0) {
                            vc--;
                            Face newface( &(v[va]), &(v[vc]), &(v[vb]) ); // invoco il costruttore di faccia
                            f.push_back( newface ); // inserico la nuova faccia in coda al vettore facce
                            nt++;
                            vb=vc;
                            }
                        }
                        break;

                        default:
                            // eat up rest of line
                            fgets(buf, sizeof(buf), file);
                            break;
                }
            }

            //printf("dopo SecondPass nv=%d nt=%d\n",nv,nt);
            
            fclose(file);
            return true;
        }





    private: 

        void populateEdges() {
            e.reserve(3*f.size());
            for (auto face : this->f) {
                Edge e1(face.v[0],face.v[1]),
                    e2(face.v[0], face.v[2]),
                    e3(face.v[1], face.v[2]);
                // Edge e1,e2,e3;

                // e1.v[0] = face.v[0]; e1.v[1] = face.v[1];
                // e2.v[0] = face.v[0]; e2.v[1] = face.v[2];
                // e3.v[0] = face.v[1]; e3.v[1] = face.v[2];
                
                //if (std::find(e.begin(), e.end(), e1) != e.end())
                    e.push_back(e1);
                //if (std::find(e.begin(), e.end(), e2) != e.end())
                    e.push_back(e2);
                //if (std::find(e.begin(), e.end(), e3) != e.end())
                    e.push_back(e3);
            }

            e.shrink_to_fit();
        }

        void populateBuffers() {
            for (auto face : f) {
                for (int i=0; i<3; i++) {
                    points.push_back((face.v)[i]->p);
                    normals.push_back((face.v)[i]->n);
                    
                }
            }
            // for (auto vertex : v) {
            //         points.push_back(vertex.p);
            //         normals.push_back(vertex.n);
            // }
        }

};
