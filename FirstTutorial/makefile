CC = clang
CXX = clang++
IDIR = ./include
LDFLAGS = -Wall
LDLIBS = -lglfw -lboost_filesystem -lboost_system -lassimp
CFLAGS = 
CXXFLAGS = -std=c++11
CPPFLAGS = -Wall -c -I$(IDIR)
OBJ = main.o glad.o Camera.o Structs.o Mesh.o Shader.o Model.o
EXECUTABLE = gltutorial
FRAMEWORKS = -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo

all: opengltutorial

opengltutorial: $(OBJ)
	$(CXX) $(LDFLAGS) $(LDLIBS) $(FRAMEWORKS) $(OBJ) -o $(EXECUTABLE)

main.o: main.cpp Shader.h Camera.h Lights.h Structs.h Model.h Mesh.h Object.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) main.cpp

Camera.o: Camera.cpp Camera.h Object.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) Camera.cpp

glad.o: glad.c
	$(CC) $(CFLAGS) $(CPPFLAGS) glad.c

Structs.o: Structs.cpp Structs_Inline.h Structs.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) Structs.cpp

Mesh.o: Mesh.cpp Mesh.h Structs.h Shader.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) Mesh.cpp

Shader.o: Shader.cpp Shader.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) Shader.cpp

Model.o: Model.cpp Model.h Mesh.h Shader.h Structs.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) Model.cpp

clean:
	$(RM) *.o $(EXECUTABLE)

#clang++ -I ./include glad.c main.cpp -lglfw -lboost_system -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo -o Spins
