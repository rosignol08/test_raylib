# Nom de l'exécutable
EXEC = test.exe

# Dossier contenant les fichiers d'en-tête (.h)
INCLUDE_DIR = include

# Dossier contenant les bibliothèques (.lib)
LIB_DIR = lib

# Fichiers source
SRC = test.c

# Options de compilation
CFLAGS = -O1 -Wall -std=c99 -Wno-missing-braces -I$(INCLUDE_DIR)
LDFLAGS = -L$(LIB_DIR) -lraylib -lopengl32 -lgdi32 -lwinmm

# Commande par défaut
all: $(EXEC)

# Règle pour générer l'exécutable
$(EXEC): $(SRC)
	gcc $(SRC) -o $(EXEC) $(CFLAGS) $(LDFLAGS)

# Nettoyage des fichiers générés
clean:
	del /F /Q $(EXEC)

# Phony targets
.PHONY: all clean
