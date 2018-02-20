EXE:=main
CFILE:= OutilsPGM.c noise.c

all: clean $(EXE)

clean:
	rm -f $(EXE) && rm -f *.o

$(EXE): *.c
	gcc -Wall -o $(EXE) $(EXE).c $(CFILE) -lglut -lGLU -lGL -lm 

run: all
	./$(EXE)

