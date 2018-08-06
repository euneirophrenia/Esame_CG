#include <math.h>
// #include "Glew/glew.h"

#ifdef __APPLE__
  #include <SDL2_image/SDL_image.h>
#else
  #include <SDL2/SDL_image.h>
#endif

#include "vehicles.h"
#include "world.h"

#define CAMERA_BACK_CAR 0
#define CAMERA_TOP_FIXED 1
#define CAMERA_TOP_CAR 2
#define CAMERA_PILOT 3
#define CAMERA_MOUSE 4
#define CAMERA_TYPE_MAX 5

float viewAlpha=20, viewBeta=40; // angoli che definiscono la vista
float eyeDist=5.0; // distanza dell'occhio dall'origine
int scrH=750, scrW=750; // altezza e larghezza viewport (in pixels)
bool useWireframe=false;
bool useEnvmap=true;
bool useHeadlight=false;
bool useShadow=true;
int cameraType=0;

static int keymap[Controller::NKEYS] = {'a', 'd', 'w', 's'};

World world;
MotorBike bike(&world); // la nostra macchina


int nstep=0; // numero di passi di FISICA fatti fin'ora
const int PHYS_SAMPLING_STEP=10; // numero di millisec che un passo di fisica simula

// Frames Per Seconds
const int fpsSampling = 3000; // lunghezza intervallo di calcolo fps
float fps=0; // valore di fps dell'intervallo precedente
int fpsNow=0; // quanti fotogrammi ho disegnato fin'ora nell'intervallo attuale
Uint32 timeLastInterval=0; // quando e' cominciato l'ultimo intervallo



// setta le matrici di trasformazione in modo
// che le coordinate in spazio oggetto siano le coord 
// del pixel sullo schemo
void  SetCoordToPixel(){
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(-1,-1,0);
  glScalef(2.0/scrW, 2.0/scrH, 1);
}

bool LoadTexture(int textbind,char *filename){
GLenum texture_format;

  SDL_Surface *s = IMG_Load(filename);
  if (!s) return false;

  if (s->format->BytesPerPixel == 4){     // contiene canale alpha
    if (s->format->Rmask == 0x000000ff){
      texture_format = GL_RGBA;
    }
    else{
      texture_format = GL_BGRA;
    }
  } else if (s->format->BytesPerPixel == 3){     // non contiene canale alpha
     if (s->format->Rmask == 0x000000ff)
       texture_format = GL_RGB;
     else
       texture_format = GL_BGR;
    } else {
        printf("[ERROR] the image is not truecolor\n");
        exit(1);
      }

  glBindTexture(GL_TEXTURE_2D, textbind);
  gluBuild2DMipmaps(
  //glGenerateMipmap(
      GL_TEXTURE_2D, 
      3,
      s->w, s->h, 
      texture_format,
      GL_UNSIGNED_BYTE,
      s->pixels
  );
  glTexParameteri(
  GL_TEXTURE_2D, 
  GL_TEXTURE_MAG_FILTER,
  GL_LINEAR ); 
  glTexParameteri(
  GL_TEXTURE_2D, 
  GL_TEXTURE_MIN_FILTER,
  GL_LINEAR_MIPMAP_LINEAR ); 
  return true;
}

// disegna gli assi nel sist. di riferimento
void drawAxis(){
  const float K=0.10;
  glColor3f(0,0,1);
  glBegin(GL_LINES);
    glVertex3f( -1,0,0 );
    glVertex3f( +1,0,0 );

    glVertex3f( 0,-1,0 );
    glVertex3f( 0,+1,0 );

    glVertex3f( 0,0,-1 );
    glVertex3f( 0,0,+1 );
  glEnd();
  
  glBegin(GL_TRIANGLES);
    glVertex3f( 0,+1  ,0 );
    glVertex3f( K,+1-K,0 );
    glVertex3f(-K,+1-K,0 );

    glVertex3f( +1,   0, 0 );
    glVertex3f( +1-K,+K, 0 );
    glVertex3f( +1-K,-K, 0 );

    glVertex3f( 0, 0,+1 );
    glVertex3f( 0,+K,+1-K );
    glVertex3f( 0,-K,+1-K );
  glEnd();

}


// setto la posizione della camera
void setCamera(){

        double px = bike.px;
        double py = bike.py;
        double pz = bike.pz;
        double angle = bike.facing;
        double cosf = cos(angle*M_PI/180.0);
        double sinf = sin(angle*M_PI/180.0);
        double camd, camh, ex, ey, ez, cx, cy, cz;
        double cosff, sinff;

// controllo la posizione della camera a seconda dell'opzione selezionata
        switch (cameraType) {
        case CAMERA_BACK_CAR:
                camd = 2.5;
                camh = 1.0;
                ex = px + camd*sinf;
                ey = py + camh;
                ez = pz + camd*cosf;
                cx = px - camd*sinf;
                cy = py + camh;
                cz = pz - camd*cosf;
                gluLookAt(ex,ey,ez,cx,cy,cz,0.0,1.0,0.0);
                break;
        case CAMERA_TOP_FIXED:
                camd = 0.5;
                camh = 0.55;
                angle = bike.facing + 40.0;
                cosff = cos(angle*M_PI/180.0);
                sinff = sin(angle*M_PI/180.0);
                ex = px + camd*sinff;
                ey = py + camh;
                ez = pz + camd*cosff;
                cx = px - camd*sinf;
                cy = py + camh;
                cz = pz - camd*cosf;
                gluLookAt(ex,ey,ez,cx,cy,cz,0.0,1.0,0.0);
                break;
        case CAMERA_TOP_CAR:
                camd = 2.5;
                camh = 1.0;
                ex = px + camd*sinf;
                ey = py + camh;
                ez = pz + camd*cosf;
                cx = px - camd*sinf;
                cy = py + camh;
                cz = pz - camd*cosf;
                gluLookAt(ex,ey+5,ez,cx,cy,cz,0.0,1.0,0.0);
                break;
        case CAMERA_PILOT:
                camd = 0.2;
                camh = 0.85;
                ex = px + camd*sinf;
                ey = py + camh;
                ez = pz + camd*cosf;
                cx = px - camd*sinf;
                cy = py + camh;
                cz = pz - camd*cosf;
                gluLookAt(ex,ey,ez,cx,cy,cz,0.0,1.0,0.0);
                break;
        case CAMERA_MOUSE:
                glTranslatef(0,0,-eyeDist);
                glRotatef(viewBeta,  1,0,0);
                glRotatef(viewAlpha, 0,1,0);
/*
printf("%f %f %f\n",viewAlpha,viewBeta,eyeDist);
                ex=eyeDist*cos(viewAlpha)*sin(viewBeta);
                ey=eyeDist*sin(viewAlpha)*sin(viewBeta);
                ez=eyeDist*cos(viewBeta);
                cx = px - camd*sinf;
                cy = py + camh;
                cz = pz - camd*cosf;
                gluLookAt(ex,ey,ez,cx,cy,cz,0.0,1.0,0.0);
*/
                break;
        }
}

/* Esegue il Rendering della scena */
void rendering(){
  
  // un frame in piu'!!!
  fpsNow++;
  
  glLineWidth(2); // linee larghe
     
  // settiamo il viewport
  glViewport(0,0, scrW, scrH);
  
  // colore sfondo = bianco
  glClearColor(1,1,1,1);


  // settiamo la matrice di proiezione
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  gluPerspective( 70, //fovy,
		((float)scrW) / scrH,//aspect Y/X,
		0.2,//distanza del NEAR CLIPPING PLANE in coordinate vista
		1000  //distanza del FAR CLIPPING PLANE in coordinate vista
  );

  glMatrixMode( GL_MODELVIEW ); 
  glLoadIdentity();
  
  // riempe tutto lo screen buffer di pixel color sfondo
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  //drawAxis(); // disegna assi frame VISTA
  
  // setto la posizione luce
  float tmpv[4] = {0,1,2,  0}; // ultima comp=0 => luce direzionale
  glLightfv(GL_LIGHT0, GL_POSITION, tmpv );

  
  // settiamo matrice di vista
  //glTranslatef(0,0,-eyeDist);
  //glRotatef(viewBeta,  1,0,0);
  //glRotatef(viewAlpha, 0,1,0);
  setCamera();

  
  //drawAxis(); // disegna assi frame MONDO

  static float tmpcol[4] = {1,1,1,  1};
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, tmpcol);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 127);
  
  glEnable(GL_LIGHTING);
 
  // settiamo matrice di modellazione
  //drawAxis(); // disegna assi frame OGGETTO
  //drawCubeWire();
  
  world.draw(); // disegna il mondo
  
  bike.Render(); // disegna la moto
	
  // attendiamo la fine della rasterizzazione di 
  // tutte le primitive mandate 
  
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
 
// disegnamo i fps (frame x sec) come una barra a sinistra.
// (vuota = 0 fps, piena = 100 fps)
  printf("\r[DEBUG] FPS %f", fps);
  SetCoordToPixel();
  glBegin(GL_QUADS);
    float y=scrH*fps/100;
    float ramp=fps/100;
    glColor3f(1-ramp,0,ramp);
    glVertex2d(10,0);
    glVertex2d(10,y);
    glVertex2d(0,y);
    glVertex2d(0,0);
  glEnd();
  
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  
  glFinish();
  // ho finito: buffer di lavoro diventa visibile
  glutSwapBuffers();
  glutPostRedisplay();

}

// void redraw(){
//   // ci automandiamo un messaggio che (s.o. permettendo)
//   // ci fara' ridisegnare la finestra
//   SDL_Event e;
//   e.type=SDL_WINDOWEVENT;
//   e.window.event=SDL_WINDOWEVENT_EXPOSED;
//   SDL_PushEvent(&e);
// }

//These functions right here, instead, control the UI (and kinda the controller)

void keyboardDownHandler(unsigned char key, int x, int y) {
        bike.controller.EatKey(key, keymap , true);
        if (key=='1') cameraType=(cameraType+1)%CAMERA_TYPE_MAX;
        if (key=='2') useWireframe=!useWireframe;
        if (key=='3') useEnvmap=!useEnvmap;
        if (key=='4') useHeadlight=!useHeadlight;
        if (key=='5') useShadow=!useShadow;
        if (key=='q') exit(0);

}

void keyboardUpHandler(unsigned char key, int x, int y) {
     bike.controller.EatKey(key, keymap , false);
}

void idleFunction() {
      // nessun evento: siamo IDLE
      
      int timeNow = glutGet(GLUT_ELAPSED_TIME); // che ore sono?
      
      if (timeLastInterval+fpsSampling<timeNow) {
        fps = 1000.0*((float)fpsNow) /(timeNow-timeLastInterval);
        fpsNow=0;
        timeLastInterval = timeNow;
      }
      
      bool doneSomething=false;
      int guardia=0; // sicurezza da loop infinito
      
      // finche' il tempo simulato e' rimasto indietro rispetto
      // al tempo reale...
      while (nstep*PHYS_SAMPLING_STEP < timeNow ) {
        bike.DoStep();
        nstep++;
        doneSomething = true;
        timeNow = glutGet(GLUT_ELAPSED_TIME);
        if (guardia++>1000) { exit(11); } // siamo troppo lenti!
      }
      
      if (doneSomething) {
          rendering();
      }
      //redraw();

      else {
        // tempo libero!!!
      }
}

void mouseHandler(int button, int state, int x, int y) {
  //   if (e.motion.state & button == 1 & cameraType==CAMERA_MOUSE) {
  //           viewAlpha+=e.motion.xrel;
  //           viewBeta +=e.motion.yrel;
  // //          if (viewBeta<-90) viewBeta=-90;
  //           if (viewBeta<+5) viewBeta=+5; //per non andare sotto la macchina
  //           if (viewBeta>+90) viewBeta=+90;
  //           // redraw(); // richiedi un ridisegno (non c'e' bisongo: si ridisegna gia' 
  //                 // al ritmo delle computazioni fisiche)

  /*if (e.wheel.y < 0 ) {
         // avvicino il punto di vista (zoom in)
         eyeDist=eyeDist*0.9;
         if (eyeDist<1) eyeDist = 1;
       };
       if (e.wheel.y > 0 ) {
         // allontano il punto di vista (zoom out)
         eyeDist=eyeDist/0.9;
       };*/

}


int main(int argc, char* argv[])
{
  glutInit( &argc, argv );

  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowSize( scrH, scrW );

  glutCreateWindow( "CG2018 Di Vincenzo" );
  world.BindVAOs(); // setup the VAOs for later efficiency

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_NORMALIZE); // opengl, per favore, rinormalizza le normali 
                          // prima di usarle
  //glEnable(GL_CULL_FACE);
  glFrontFace(GL_CW); // consideriamo Front Facing le facce ClockWise

  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  glEnable(GL_POLYGON_OFFSET_FILL); // caro openGL sposta i 
                                    // frammenti generati dalla
                                    // rasterizzazione poligoni
  glPolygonOffset(1,1);             // indietro di 1
  
  if (!LoadTexture(0,(char *)"Resources/logo.jpg")) return 0;
  if (!LoadTexture(1,(char *)"Resources/envmap_flipped.jpg")) return 0;
  if (!LoadTexture(2,(char *)"Resources/sky_ok.jpg")) return -1;

  glutDisplayFunc(rendering);

  glutKeyboardUpFunc(keyboardUpHandler);
  glutKeyboardFunc(keyboardDownHandler);
  glutMouseFunc(mouseHandler);
  glutIdleFunc(idleFunction);

  glutMainLoop();
 
    
return (0);
}

