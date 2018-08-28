CPP  = g++ 
#CPP = icpc
CC   = gcc
BIN  = CG2018_DiVincenzo

OBJ  = main.o
LINKOBJ  = main.o

STD = c++11

# Library linking
OS := $(shell uname)
ifeq ($(OS),Darwin)
## caso Mac OS
$(info Mac OS detected)
FRMPATH=-F /Library/Frameworks
LIBS =  -framework OpenGL -framework GLUT -lm
$(info GLUT libraries must be in: $(FRMPATH))
else
ifeq ($(OS),MINGW32_NT-6.1)
## caso Windows MinGW
$(info Windows MinGW detected)
FRMPATH = -IC:\MinGW\freeglut\include
LDFLAGS = -LC:\MinGW\freeglut\lib -lfreeglut -LC:\MinGW\lib -lmingw32 -lopengl32 -lglu32 -lm
else
##caso Linux
$(info Linux detected)
FRMPATH=
LDFLAGS = -lGL -lGLU -lglut -lm
endif
endif

LDFLAGS=$(LDFLAGS) "-L./Geometry"  "-L./Glew"
CFLAGS=$(CFLAGS) "-I./Geometry" "-I./Glew"

FLAG =-Wno-deprecated -O3
RM = rm -f

all: clean $(BIN) 

clean:
	${RM} $(OBJ) $(BIN)

$(BIN): $(OBJ)
	$(CPP) -std=$(STD) $(FLAG) $(LINKOBJ) -o $(BIN) $(FRMPATH) $(LIBS)

main.o: main.cpp
	$(CPP) -std=$(STD) $(FLAG) -c $(FRMPATH) main.cpp -o main.o
