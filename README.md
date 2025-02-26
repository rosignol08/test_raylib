# Projet tuteuré - Génération réaliste

## Table des matières
1. [Introduction](#introduction)
2. [Présentation de Raylib](#présentation-de-raylib)
3. [Installation](#installation)
4. [Compilation](#compilation)
5. [Test d'un code Raylib](Test-d'un-code-Raylib)
6. [Développement du projet](#développement-du-projet)
7. [Conclusion](#conclusion)
8. [Références](#références)

## Introduction
Observer et décrire un système existant et les règles et paramètres qui influencent sa formation
(par exemple: formation des nuages ; lien entre relief, eau et installation humaine ; croissance
d'un sous-bois et répartition des espèces végétales ; développement de bois de cerfs de
différentes espèces ; etc.)
- Créer un outil de génération d'un exemplaire du système choisi, avec des options ajustables
par l'utilisateur

## Présentation de Raylib
[Raylib](https://github.com/raysan5/raylib) est une bibliothèque de développement de jeux vidéo et d'applications multimédias, créée par Ramon Santamaria. C'est une bibliothèque open-source, écrite en C pur, qui se distingue par sa simplicité et sa facilité d'utilisation tout en offrant des fonctionnalités puissantes.


## Installation

### Windows
1. Prérequis
   - MinGW-w64 :
     * **ATTENTION** : Vous devez absolument télécharger la version avec **win32-seh-msvcrt** :
       * Fichier exact : `x86_64-14.2.0-release-win32-seh-msvcrt-rt_v12-rev0.7z`
       * Depuis [mingw-builds-binaries](https://github.com/niXman/mingw-builds-binaries/releases)
       * ⚠️ Les autres versions (posix, ucrt, etc.) ne fonctionneront pas
     * Installation de MinGW :
       1. Créer un dossier `mingw` à la racine du disque C (`C:\mingw`)
       2. Extraire TOUT le contenu de l'archive `.7z` directement dans `C:\mingw
       3. Ajouter `C:\mingw\bin` au PATH système/variable d'environnement windows

2. Étapes d'installation
   - Télécharger le [source code ZIP de Raylib](https://github.com/raysan5/raylib/releases)

3. Constuire la librairie statiquement
    ```powershell
   cd raylib/src/
   mingw32-make PLATFORM=PLATFORM_DESKTOP
    ``` 
4. Structure
   - Déplacer le fichier libraylib.a dans le dossier lib.
   - Déplacer le fichier raylib.h dans le dossier include et éventuellement tous les fichiers qui seront nécessaires.



### Linux
1. Prérequis
   ```bash
   sudo apt update
   sudo apt install build-essential git make
   ```

2. Installation via les paquets

- Ubuntu
   ```bash
   sudo apt install libasound2-dev libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev libwayland-dev libxkbcommon-dev
   ```
- Fedora
   ```bash
   sudo dnf install alsa-lib-devel mesa-libGL-devel libX11-devel libXrandr-devel libXi-devel libXcursor-devel libXinerama-devel libatomic
   ```

- Arch
   ```bash
   sudo pacman -S alsa-lib mesa libx11 libxrandr libxi libxcursor libxinerama
   ```

- ##### Intel drivers
   ```bash
   sudo xbps-install mesa-dri mesa-intel-dri
   ```



3. Installation depuis les sources
   ```
    git clone --depth 1 https://github.com/raysan5/raylib.git raylib
    cd raylib/src/
    make PLATFORM=PLATFORM_DESKTOP RAYLIB_LIBTYPE=SHARED
    sudo make install RAYLIB_LIBTYPE=SHARED 
   ```
  

## Compilation
### Structure du Projet Windows ( Statique )
```bash
mon_projet/
├── main.c           # Fichier source principal
├── include/
│   └── raylib.h    # Header de raylib
└── lib/
   └── librarylib.a # Bibliothèque statique compilée

# Note : Cette structure est spécifique à la compilation statique sous Windows.
# Cette approche permettra de transporter le projet facilement sans dépendre
# d'une installation de raylib sur la machine cible.
# ( C'est aussi la méthode la moins compliquée et contraignante que j'ai trouvé 
# pour installer sur Windows )
```
### Compiler sur Windows avec [Mingw-w64](https://www.mingw-w64.org/)
```powershell
gcc main.c -o main.exe -O1 -Wall -std=c99 -Wno-missing-braces -I include/ -L lib/ -lraylib -lopengl32 -lgdi32 -lwinmm
```

### Linux
```bash
gcc main.c -o test -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
```



## Test d'un code Raylib
### Exemple de code minimal
```c
#include "raylib.h"

int main(void)
{
    InitWindow(800, 450, "Raylib - Exemple minimal");

    while (!WindowShouldClose())
    {
        BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawText("Bonjour Raylib!", 190, 200, 20, LIGHTGRAY);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
```
Si vous pouvez compiler et lancer ce code, tout est bon. Vous pouvez lancer notre projet.

## Développement du projet
À compléter 

## Conclusion
À compléter 

## Références
- [Site officiel de Raylib](https://www.raylib.com/)
- [Documentation Raylib](https://www.raylib.com/cheatsheet/cheatsheet.html)
- [GitHub Raylib](https://github.com/raysan5/raylib)
