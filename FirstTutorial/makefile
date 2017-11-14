CC = clang
CXX = clang++
IDIR = ./include
LDFLAGS = -Wall
LDLIBS = -lglfw -lboost_filesystem -lboost_system -lassimp
CFLAGS = 
CXXFLAGS = -std=c++11
CPPFLAGS = -Wall -c -I$(IDIR)
OBJ = main.o glad.o Camera.o
EXECUTABLE = gltutorial
FRAMEWORKS = -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo

all: opengltutorial

opengltutorial: $(OBJ)
	$(CXX) $(LDFLAGS) $(LDLIBS) $(FRAMEWORKS) $(OBJ) -o $(EXECUTABLE)

main.o: main.cpp stb_image.h Shader.h Lights.h Structs.h Mesh.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) main.cpp

Camera.o: Camera.cpp Camera.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) Camera.cpp

glad.o: glad.c
	$(CC) $(CFLAGS) $(CPPFLAGS) glad.c

clean:
	$(RM) *.o $(EXECUTABLE)

#clang++ -I ./include glad.c main.cpp -lglfw -lboost_system -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo -o Spins
