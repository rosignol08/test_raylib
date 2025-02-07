CC = gcc

# Détection du système d'exploitation
OS := $(shell uname -s 2>/dev/null || echo Windows_NT)

# Options communes
CFLAGS = -Wall -std=c99 -Wno-missing-braces -O1
INCLUDE = -Iinclude/
SRC = main.c

ifeq ($(OS), Windows_NT)
    # Compilation pour Windows (statique)
    LDFLAGS = -Llib/ -lraylib -lopengl32 -lgdi32 -lwinmm
    OUTPUT = main.exe
    RM = del /Q
else
    # Compilation pour Linux (dynamique)
    LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
    OUTPUT = main
    RM = rm -f
endif

# Règle principale
all:
	$(CC) $(SRC) -o $(OUTPUT) $(CFLAGS) $(INCLUDE) $(LDFLAGS)

# Nettoyer les fichiers exécutables
clean:
	$(RM) $(OUTPUT)
