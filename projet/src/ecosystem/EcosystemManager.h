#ifndef ECOSYSTEM_MANAGER_H
#define ECOSYSTEM_MANAGER_H
#include <raylib.h>
#include <vector>
#include "plante.h"
#include "gridCell.h"
#include "biome.h"

#define GRID_SIZE 30

// Fonctions principales de gestion de l'écosystème
void InitializeEcosystem(std::vector<std::vector<GridCell>>& grille, 
                        std::vector<Plante>& plantes, 
                        std::vector<Plante>& plantes_mortes,
                        std::vector<Biome>& les_biomes,
                        Model& model_bouleau1, Model& model_bouleau2, Model& model_erable,
                        Model& model_mort_bouleau1, Model& model_mort_bouleau2, Model& model_mort_erable,
                        Model& emptyModel, Image& image_sol, Vector3& taille_terrain);

void UpdateEcosystem(std::vector<std::vector<GridCell>>& grille,
                    std::vector<Plante>& plantes,
                    std::vector<Plante>& plantes_mortes,
                    int minTemp, int maxTemp, int minHum, int maxHum);

void UpdateBiomes(std::vector<Biome>& les_biomes,
                 std::vector<std::vector<GridCell>>& grille,
                 int& minTemp, int& maxTemp, int& minHum, int& maxHum,
                 int& minPluv, int& maxPluv);

void LoadModels(Model& model_bouleau1, Model& model_bouleau2, Model& model_erable,
               Model& model_mort_bouleau1, Model& model_mort_bouleau2, Model& model_mort_erable,
               Model& emptyModel);

void UnloadEcosystemResources(Model& model_bouleau1, Model& model_bouleau2, Model& model_erable,
                             Model& model_mort_bouleau1, Model& model_mort_bouleau2, Model& model_mort_erable,
                             Model& model_sol, Model& emptyModel);

// Fonctions utilitaires
bool is_plant_morte(const std::string& nom, const std::vector<Plante>& plantes_mortes);
void verifier_plante(std::vector<std::vector<GridCell>>& grille, GridCell* cellule,
                    std::vector<Plante> plantes, std::vector<Plante> plantes_mortes,
                    Plante vide, int minTemp, int maxTemp, int minHum, int maxHum, Color couleur_sante);
float GetHeightFromTerrain(Vector3 position, Image heightmap, Vector3 terrainSize);
float random_flottant(float min, float max);

#endif