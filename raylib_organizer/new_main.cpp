// main.cpp
#include "core/simulation.h"
#include "rendering/renderer.h"

int main(void) {
    Renderer renderer(1280, 720, "raylib - Projet tutore");
    Simulation simulation;
    
    // Choix du terrain - vous pouvez ajouter un menu ici ou hardcoder
    simulation.loadTerrain("ressources/heightmap.png");
    simulation.initialize();
    
    // Initialiser l'herbe après que le terrain soit chargé
    // renderer.initializeGrass(simulation); // À appeler quelque part
    
    while (!WindowShouldClose()) {
        simulation.update(GetFrameTime());
        
        renderer.beginFrame();
        renderer.renderSimulation(simulation);
        renderer.endFrame();
    }
    
    return 0;
}