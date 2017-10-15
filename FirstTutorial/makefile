CC = clang
CXX = clang++
IDIR = ./include
LDFLAGS = -Wall
LDLIBS = -lglfw -lboost_system
CFLAGS = -Wall -c -I$(IDIR)
OBJ = main.o glad.o
EXECUTABLE = gltutorial
FRAMEWORKS = -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo

all: opengltutorial

opengltutorial: $(OBJ)
	$(CXX) $(LDFLAGS) $(LDLIBS) $(FRAMEWORKS) $(OBJ) -o $(EXECUTABLE)

main.o: main.cpp stb_image.h Shader.h
	$(CXX) $(CFLAGS) main.cpp

glad.o: glad.c
	$(CC) $(CFLAGS) glad.c

clean:
		$(RM) *.o $(EXECUTABLE)

#clang++ -I ./include glad.c main.cpp -lglfw -lboost_system -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo -o Spins
