CC = clang
CXX = clang++
IDIR = ./include
LDFLAGS = -Wall
LDLIBS = -lglfw -lboost_filesystem -lboost_system -lassimp
CFLAGS = 
CXXFLAGS = -std=c++11
CPPFLAGS = -Wall -c -I$(IDIR)
OBJ = main.o glad.o Camera.o Structs.o Mesh.o Shader.o Model.o Framebuffer.o SceneGraph.o Scene.o
EXECUTABLE = gltutorial
FRAMEWORKS = -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo

all: opengltutorial

opengltutorial: $(OBJ)
	$(CXX) $(LDFLAGS) $(LDLIBS) $(FRAMEWORKS) $(OBJ) -o $(EXECUTABLE)

main.o: main.cpp Shader.h Camera.h Lights.h Structs.h Model.h Mesh.h Object.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) main.cpp

Camera.o: Camera.h Camera.cpp Object.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) Camera.cpp

glad.o: glad.c
	$(CC) $(CFLAGS) $(CPPFLAGS) glad.c

Structs.o: Structs.h Structs_Inline.h Structs.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) Structs.cpp

Mesh.o: Mesh.h Mesh.cpp Structs.h Shader.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) Mesh.cpp

Shader.o: Shader.h Shader.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) Shader.cpp

Model.o: Model.h Model.cpp Mesh.h Shader.h Structs.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) Model.cpp

Framebuffer.o: Framebuffer.h Framebuffer.cpp Structs.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) Framebuffer.cpp

SceneGraph.o: SceneGraph.h SceneGraph.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) SceneGraph.cpp

Scene.o: Scene.h Scene.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) Scene.cpp

clean:
	$(RM) *.o $(EXECUTABLE)

#clang++ -I ./include glad.c main.cpp -lglfw -lboost_system -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo -o Spins
