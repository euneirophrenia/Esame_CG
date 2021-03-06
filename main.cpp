#include <math.h>
#include <unistd.h>
#include <time.h>
#include <cstdlib>

#include "vehicles.h"
#include "world.h"


float viewAlpha=20, viewBeta=40;  // angoli che definiscono la vista
float eyeDist=5.0;                // distanza dell'occhio dall'origine
int scrH=750, scrW=750;           // altezza e larghezza viewport (in pixels)

// several swtiches for several functions ----
bool useWireframe=false;      
bool useTransparency=false;
bool useHeadlight=false;
bool useShadow=true;
bool showMinimap = false;
bool showMenu = false;
bool useBadWireFrame = false;
bool stopTime = false;
bool dead = false;
int cameraType=0;
//--------

// for minimap, vertices of minitriangle, in quad coords, fine tuned by hand 
const Point2 A(0.5, 0.56); 
const Point2 B(0.48, 0.47);
const Point2 C(0.52, 0.47);
const Point3 center = (A+B+C)/3.0;
//------

std::string floor_texture = "Resources/parquet.ppm";

int previous_mouse_position[2] = {0, 0};

static int keymap[Controller::NKEYS] = {'A', 'D', 'W', 'S'};

World world;
MotorBike bike(&world); // la nostra moto

Point3 player_position; // #hacks, if I was smarter about organizing my code i would not need these
float player_facing; // #hacks, ""  """ "" "" "" "" "" "" "" "" "" "" "" "" "" ""


int nstep=0; // numero di passi di FISICA fatti fin'ora
const int PHYS_SAMPLING_STEP = 10; // numero di millisec che un passo di fisica simula

// Frames Per Seconds
const int fpsSampling = 3000; // lunghezza intervallo di calcolo fps
float fps=0; // valore di fps dell'intervallo precedente
int fpsNow=0; // quanti fotogrammi ho disegnato fin'ora nell'intervallo attuale
unsigned int timeLastInterval=0; // quando e' cominciato l'ultimo intervallo



// setta le matrici di trasformazione in modo
// che le coordinate in spazio oggetto siano le coord 
// del pixel sullo schemo
inline void  SetCoordToPixel(){
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(-1,-1,0);
  glScalef(2.0/scrW, 2.0/scrH, 1);
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
inline void setCamera(){

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
                camd = 2.0;
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
                camd = 0.6;
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
                camh = 1.5;
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
        case CAMERA_FRONT:
                camd = -0.5;
                camh = 1.0;
                ex = px + camd*sinf;
                ey = py + camh;
                ez = pz + camd*cosf;
                cx = px - camd*sinf;
                cy = py + camh;
                cz = pz - camd*cosf;
                gluLookAt(ex,ey,ez,cx,cy,cz, 0.0, 1.0, 0.0);
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

/** --- UI  section ---- */
void setInputState(bool active); //hack per poter usare funzioni dichiarate dopo

void DrawMiniMap() {

  glViewport(scrW*0.5, 0, scrW*0.5, scrW*0.5 );
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glPushMatrix();
    glColor3f(0, 1, 0);

    glTranslated(-player_position.X()/ROOM_SIZE, player_position.Z()/ROOM_SIZE, 0);  //-X, +Z to account for an OpenGL peculiarty

    for (auto tile: world.getTiles()){
      glPushMatrix();
  
        glTranslated(tile->center.X()/ROOM_SIZE, -tile->center.Z()/ROOM_SIZE, 0); // same with the +X, -Z, here
        auto s = tile->getScale();
        glScaled(s.X(), s.Z(), 1);
        glRotated(tile->rotation, 0, 0, 1);
      
        tile->DrawMiniMarker();
        
      glPopMatrix();
    }
  glPopMatrix();

  glPushMatrix();
    glColor3f(1,0,0);
    glTranslatef(-center.X(), -center.Y(), 0);
    glBegin(GL_TRIANGLES);
        auto tmp = rotateAround(A, center, +bike.horizontal_orientation());
        glVertex2f(tmp.coord[0] , tmp.coord[1]);

        tmp = rotateAround(B, center, +bike.horizontal_orientation());
        glVertex2f(tmp.coord[0] , tmp.coord[1]);

        tmp = rotateAround(C, center, +bike.horizontal_orientation());
        glVertex2f(tmp.coord[0] , tmp.coord[1]);

    glEnd();
  glPopMatrix();
  glViewport(0, 0, scrW, scrH);
}

// Draw text and other infos
inline void DrawUI(){

  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  glDisable(GL_CULL_FACE); // it's actually funnier with culling, but wrong
 
  DrawText(0, scrH-20, "FPS " + std::to_string(fps));
  float v=bike.Velocity();
  DrawText(20, 20, "Speed: " + std::to_string(int(v*500)) + " km/h");

  if (bike.crashed && !dead){
    DrawText(scrW/2, scrH/2, "C R A S H E D", RED);
    DrawText(scrW/2 - 5, scrH/2 - 25, "press Q to exit", RED);
    setInputState(false);
  }

  if (dead) {
    DrawText(scrW/2 - 150, scrH/2, "You got eaten by the Monster Under the Bed (TM)", RED);
    DrawText(scrW/2 - 5, scrH/2 - 25, "press Q to exit", RED);
    DrawText(scrW/2 - 150, scrH/2 - 50, "..did I mention there's a monster under the bed?", RED);
    setInputState(false);
    float col0[4]= {40/255.0, 13/255.0, 100.0/255,  1};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, col0);    
    //float col1[4]= {0.5,0.5,0.0,  1};
    glLightfv(GL_LIGHT0, GL_AMBIENT, col0);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND); //~inverse color effect, for fun
  }

  if (showMinimap)
    DrawMiniMap();
  
  if (!showMenu)
    DrawText(scrW*0.3, scrH-20, "Press 'H' to toggle help screen");
  else {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glColor3f(1,1,1);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texProvider->indexOf("Resources/menu.ppm"));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //this gets deformed and the fix is not straight forwads since on MacOS with Retina display pixel coordinates =/= pixels...
    glBegin(GL_QUADS);  
      glTexCoord2f(20, 0);  glVertex2d(20, 0);
      glTexCoord2f(20, 20); glVertex2d(20, 20);
      glTexCoord2f(0, 20);  glVertex2d(0, 20);
      glTexCoord2f(0, 0);   glVertex2d(0, 0);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glViewport(0, 0, scrW, scrH);
  }
  
  // Draw a bar to represent the velocity
  SetCoordToPixel();
  glBegin(GL_QUADS);
    float y = scrH*v * 2;
    float ramp = v * 2;
    glColor3f(1-ramp,0,ramp);
    glVertex2d(10,0);
    glVertex2d(10,y);
    glVertex2d(0,y);
    glVertex2d(0,0);
  glEnd();

  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  
}
// ---- FINE UI ----


/* Handlers per eventi GLUT */
inline void renderHandle(){
  
  // un frame in piu'!!!
  fpsNow++;
  
  glLineWidth(1);
     
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

  
  // setto la posizione luce
  static float tmpv[4] = {0, 1, 2,  0}; // ultima comp=0 => luce direzionale
  glLightfv(GL_LIGHT0, GL_POSITION, tmpv );

  setCamera();

  static float tmpcol[4] = {0.5, 0.5, 0.5,  1};
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, tmpcol);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 50);
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, FOUR_1);
  
  glEnable(GL_LIGHTING);

  bike.Render(); // disegna la moto
  world.Draw(); // disegna il mondo
  

  DrawUI();
  
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  
  // glFinish(); //this one makes everything wait for every call to finish
  glutSwapBuffers();
  glutPostRedisplay();

}

inline void reshapeHandler(int w, int h){
  scrW = w;
  scrH = h;
  glutPostRedisplay();
}

void Log() {
    printf("\n--------- LOG ----------\n");
    printf("- FPS          :\t%f\n", fps);
    bike.Log();

}

inline void keyboardDownHandler(unsigned char key, int x, int y) {

        if (key >= 97) key-= 32; //use only capitals, otherwise a capslock on would block all the input
        bike.controller.EatKey(key, keymap , true);
        switch (key){
          case '1':
            cameraType=(cameraType+1)%CAMERA_TYPE_MAX;
            break;
          case '2':
            useWireframe=!useWireframe;
            useBadWireFrame = useWireframe? useBadWireFrame : false;
            break;
          case '3': 
            useTransparency=!useTransparency;
            break;
          case '4':  
            useHeadlight=!useHeadlight;
            break;
          case '5':
             useShadow=!useShadow;
             break;

          case 'Q':
            exit(0);
          case 27: //escape
            exit(0);
          case 32:  //spacebar (for debug purposes)
            Log();
            break;
          case '0':
            useBadWireFrame = !useBadWireFrame;
            useWireframe = useBadWireFrame;
            break;
          case 'P':
            stopTime = !stopTime;
            break;
          case 'M':
            showMinimap=!showMinimap;
            break;
          case 'H':
            showMenu = !showMenu;
        }

}

inline void keyboardUpHandler(unsigned char key, int x, int y) {
     if (key >= 97) key-= 32; //use only capital letters, otherwise a capslock on would block all the input
     bike.controller.EatKey(key, keymap , false);
}

inline void joyHandler(unsigned int buttonMask, int x, int y, int z) {
  bike.controller.Joy(Controller::ACC, x>0);
  bike.controller.Joy(Controller::LEFT, y<0);
  bike.controller.Joy(Controller::RIGHT, y>0);
  bike.controller.Joy(Controller::DEC, x<0);
  
}

inline void idleFunction() {

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
        player_position.coord[0] = bike.px;
        player_position.coord[1] = bike.py;
        player_position.coord[2] = bike.pz;
        player_facing =  bike.facing * M_PI / 180; //facing = 0 => // asse Z
        nstep++;
        doneSomething = true;
        timeNow = glutGet(GLUT_ELAPSED_TIME);
        if (guardia++>1000) {  //troppo lenti
          printf("[FATAL] Could not afford to run at the specified frequency. Aborting.. ");
          exit(11); 
        } 
      }
      
      if (doneSomething) {
          glutPostRedisplay();
      }
}

void motionHandler(int x, int y) {
    viewAlpha+= x - previous_mouse_position[0];
    viewBeta += y - previous_mouse_position[1];
    if (viewBeta<-90) viewBeta=-90;
    if (viewBeta<+5) viewBeta=+5; //per non andare sotto la macchina
    if (viewBeta>+90) viewBeta=+90;
    previous_mouse_position[0] = x;
    previous_mouse_position[1] = y;
}

void mouseHandler(int button, int state, int x, int y) {

  if (button == GLUT_LEFT_BUTTON && state==GLUT_DOWN && cameraType==CAMERA_MOUSE) {
      //start listening for motion
      glutMotionFunc(motionHandler);
  }

  if (button == GLUT_LEFT_BUTTON && state == GLUT_UP && cameraType == CAMERA_MOUSE)
      glutMotionFunc(NULL); //unbind motion

  if (button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN) {
      // avvicino il punto di vista (zoom in)
      eyeDist=eyeDist*0.9;
      if (eyeDist<1) eyeDist = 1;
  }
  if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN){
    eyeDist=eyeDist/0.9;
    if (eyeDist > 90) eyeDist = 90;
  }
}

void exitFunc(unsigned char key, int x, int y) {
  if (key=='q' || key == 27) 
    exit(0);
}
/* Fine handling eventi GLUT */


/* Utility functions */
/// To disable everything upon crashing
void setInputState(bool active) {
  glutKeyboardUpFunc(active? keyboardUpHandler : exitFunc);
  glutKeyboardFunc(active? keyboardDownHandler : NULL);
  glutMouseFunc(active? mouseHandler : NULL);
  glutIdleFunc(active? idleFunction : NULL);
}

/// To avoid memory leaking and reducing memory page swap upon several consecutive runs
void CleanUpFunc() {
  printf("[DEBUG] Cleaning up...");
  bike.FreeBuffers();
  world.FreeBuffers();
  texProvider -> FreeTextures();
  printf(" done!\n");
  if (dead) {
    printf("..maybe if we had some light the monster would go away?\n");
  }
}

int main(int argc, char* argv[])
{
  glutInit( &argc, argv );

  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH); // colori + double buffer + z-buffer
  glutInitWindowSize( scrH, scrW );

  glutCreateWindow("A Toy Soldier Story" );
  const GLubyte* renderer = glGetString (GL_RENDERER);
  const GLubyte* version =  glGetString (GL_VERSION);

  printf("\n-------- A Toy Soldier Story ----------\n");
  printf("... Tratto  da una storia vera ...\n");
  printf("Author:\tMarco Di Vincenzo - 0000795484\n");
  printf ("[DEBUG]\tRenderer: %s\n", renderer);
  printf ("[DEBUG]\tOpenGL version supported: %s\n", version);


  world.BindBuffers(); // setup the buffers so that we don't need to resend all the geometry to the GPU when drawing the world 
  bike.BindBuffers(); // same, but the efficient way is used only on some meshes, since others have some problems

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_RESCALE_NORMAL); // opengl, per favore, rinormalizza le normali prima di usarle (the fast way)
  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CW); // consideriamo Front Facing le facce ClockWise

  //glEnable(GL_MULTISAMPLE);
  
  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  glEnable(GL_POLYGON_OFFSET_FILL); // caro openGL sposta i 
                                    // frammenti generati dalla
                                    // rasterizzazione poligoni
  glPolygonOffset(1,1);             // indietro di 1

  TextureProvider* texProvider = TextureProvider::getInstance();
  srand(time(NULL));

  std::atexit(CleanUpFunc); //to cleanup after exiting

  texProvider->LoadTexture("Resources/logo.ppm");
  texProvider->LoadTexture("Resources/me.ppm");
  texProvider->LoadTexture("Resources/wood.ppm");
  texProvider->LoadTexture("Resources/parquet.ppm");
  texProvider->LoadTexture("Resources/asphalt2.ppm");
  texProvider->LoadTexture("Resources/text.ppm");
  texProvider->LoadTexture("Resources/menu.ppm");
  texProvider->LoadTexture("Resources/ball3.ppm");
  texProvider->LoadTexture("Resources/wall1.ppm");
  texProvider->LoadTexture("Resources/wall3.ppm");
  texProvider->LoadTexture("Resources/wall4.ppm");
  texProvider->LoadTexture("Resources/walldoor.ppm");
  texProvider->LoadTexture("Resources/dice1.ppm");
  texProvider->LoadTexture("Resources/dice2.ppm");
  texProvider->LoadTexture("Resources/dice3.ppm");
  texProvider->LoadTexture("Resources/dice4.ppm");
  texProvider->LoadTexture("Resources/carpet.ppm");
  texProvider->LoadTexture("Resources/ball2.ppm");
  texProvider->LoadTexture("Resources/fire.ppm");
  
  

  // Hook up handlers
  glutDisplayFunc(renderHandle);          //rendering
  glutKeyboardUpFunc(keyboardUpHandler);  // keyboard input UP
  glutKeyboardFunc(keyboardDownHandler);  // keyboard input DOWN
  glutMouseFunc(mouseHandler);            // mouse click
  glutIdleFunc(idleFunction);             // idle 
  glutMotionFunc(NULL);                   // mouse motion
  glutJoystickFunc(joyHandler, 1);        // joystick, with polling interval (1ms)
  glutReshapeFunc(reshapeHandler);        // reshape window
  //----

  glutMainLoop();
 
    
  return 0;
}

