# Makefile simple pour ProjetTutore

# Compilateur et flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude -Isrc

# Détecter la plateforme
UNAME_S := $(shell uname -s)

# Configuration selon la plateforme
ifeq ($(UNAME_S),Linux)
    # Linux - liaison dynamique
    LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
    TARGET = main
    CXXFLAGS += -DPLATFORM_DESKTOP
endif

ifeq ($(UNAME_S),Darwin)
    # macOS (si besoin)
    LIBS = -lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
    TARGET = main
    CXXFLAGS += -DPLATFORM_DESKTOP
endif

ifdef WINDOWS
    # Windows - liaison statique
    LIBS = lib/libraylib.a -lwinmm -lgdi32 -lopengl32 -static -static-libgcc -static-libstdc++
    TARGET = main.exe
    CXXFLAGS += -DPLATFORM_DESKTOP -static
endif

# Trouver tous les fichiers source
SOURCES = $(shell find src -name "*.cpp")
OBJECTS = $(SOURCES:.cpp=.o)

# Règle par défaut
all: $(TARGET)

# Créer l'exécutable
$(TARGET): $(OBJECTS)
	@echo "Création de l'exécutable..."
	$(CXX) $(OBJECTS) -o $(TARGET) $(LIBS)
	@echo "✅ Compilation terminée: $(TARGET)"

# Compiler les fichiers .cpp en .o
%.o: %.cpp
	@echo "Compilation de $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Nettoyage
clean:
	@echo "Nettoyage..."
	find src -name "*.o" -delete
	rm -f $(TARGET)
	@echo "✅ Nettoyage terminé"

# Lancer le programme
run: $(TARGET)
	./$(TARGET)

# Compilation Windows (depuis Linux avec MinGW)
windows:
	@echo "Compilation pour Windows..."
	make WINDOWS=1 CXX=x86_64-w64-mingw32-g++

# Règles qui ne créent pas de fichiers
.PHONY: all clean run windows

# Affichage des informations
info:
	@echo "Plateforme détectée: $(UNAME_S)"
	@echo "Compilateur: $(CXX)"
	@echo "Flags: $(CXXFLAGS)"
	@echo "Librairies: $(LIBS)"
	@echo "Sources trouvées: $(words $(SOURCES)) fichiers"
	@echo "Cible: $(TARGET)"