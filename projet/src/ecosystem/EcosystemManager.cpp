#include "EcosystemManager.h"
#include "../utils/utils.h" 
#include <raylib.h>   
#include <raymath.h> 
#include <cmath>
#include <iostream>
#include <algorithm>

void LoadModels(Model& model_bouleau1, Model& model_bouleau2, Model& model_erable,
               Model& model_mort_bouleau1, Model& model_mort_bouleau2, Model& model_mort_erable,
               Model& emptyModel) {
    
    // Charger les modèles vivants
    model_bouleau1 = LoadModel("models/foret_tempere/arb_bouleau/bouleau_feuilles1.glb");
    model_bouleau2 = LoadModel("models/foret_tempere/arb_bouleau/bouleau_feuilles2.glb");
    model_erable = LoadModel("models/foret_tempere/arb_erable/erable_feuilles.glb");
    
    // Charger les modèles morts
    model_mort_bouleau1 = LoadModel("models/foret_tempere/arb_bouleau/bouleau_mort1.glb");
    model_mort_bouleau2 = LoadModel("models/foret_tempere/arb_bouleau/bouleau_mort2.glb");
    model_mort_erable = LoadModel("models/foret_tempere/arb_erable/erable_mort.glb");
    
    // Créer un modèle vide
    Mesh emptyMesh = GenMeshCube(0.0f, 0.0f, 0.0f);
    emptyModel = LoadModelFromMesh(emptyMesh);
    emptyModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = BLANK;
}

void InitializeEcosystem(std::vector<std::vector<GridCell>>& grille, 
                        std::vector<Plante>& plantes, 
                        std::vector<Plante>& plantes_mortes,
                        std::vector<Biome>& les_biomes,
                        Model& model_bouleau1, Model& model_bouleau2, Model& model_erable,
                        Model& model_mort_bouleau1, Model& model_mort_bouleau2, Model& model_mort_erable,
                        Model& emptyModel, Image& image_sol, Vector3& taille_terrain) {
    
    // Créer les plantes
    Color couleur = WHITE;
    int influence_temperature = GetRandomValue(-5, 5);
    int influence_humidite = GetRandomValue(-5, 5);

    Plante bouleau1("Bouleau1", 0, 100, 70, 80, 5, 10, 500, 1000, influence_temperature, influence_humidite, 
                   0.005f, 0.01f, 0.0f, 0.2f, 0, false, 250, model_bouleau1, couleur);
    
    influence_temperature = GetRandomValue(-5, 5);
    influence_humidite = GetRandomValue(-5, 5);
    Plante bouleau2("Bouleau2", 1, 100, 75, 80, 5, 10, 500, 1000, influence_temperature, influence_humidite, 
                   0.005f, 0.01f, 0.0f, 0.2f, 0, false, 250, model_bouleau2, couleur);
    
    influence_temperature = GetRandomValue(-5, 5);
    influence_humidite = GetRandomValue(-5, 5);
    Plante erable("Erable", 2, 100, 70, 80, 10, 25, 500, 1500, 0, 0, 
                 0.01f, 0.04f, 0.0f, 1.0f, 0, false, 350, model_erable, couleur);

    // Plantes mortes
    Plante bouleau_mort1("Bouleau_mort1", 0, 100, 0, 100, -50, 200, 0, 5000, 0, 0, 
                        0.005f, 0.01f, 0.0f, 0.2f, 0, true, 50, model_mort_bouleau1, couleur);
    Plante bouleau_mort2("Bouleau_mort2", 1, 100, 0, 100, -50, 200, 0, 5000, 0, 0, 
                        0.005f, 0.01f, 0.0f, 0.2f, 0, true, 50, model_mort_bouleau2, couleur);
    Plante erable_mort("Erable_mort", 2, 100, 0, 100, -50, 200, 0, 5000, 0, 0, 
                      0.01f, 0.04f, 0.0f, 0.2f, 0, true, 50, model_mort_erable, couleur);
    Plante vide("Vide", 10, 100, 0, 0, 0, 0, 0, 0, 0, 0, 
               0.0f, 0.0f, 0.0f, 0.0f, 0, false, 100, emptyModel, couleur);

    plantes = {bouleau1, bouleau2, erable};
    plantes_mortes = {bouleau_mort1, bouleau_mort2, erable_mort};
    
    // Créer les biomes
    Biome biome_tempere("Tempere", 5, 25, 60, 80, 500, 1500, Color{255, 141, 34, 255}, 1.0f, true);
    Biome biome_tropic_humide("Tropic humide", 20, 35, 75, 95, 2000, 5000, Color{0, 161, 231, 255}, 1.0f, false);
    Biome biome_tropic_sec("Tropic sec", 25, 35, 40, 70, 1000, 2000, Color{255, 255, 255, 255}, 1.01f, false);
    
    les_biomes = {biome_tempere, biome_tropic_humide, biome_tropic_sec};
    
    // Initialiser la grille
    grille.resize(GRID_SIZE, std::vector<GridCell>(GRID_SIZE, GridCell(0, {0,0,0}, vide.model, true, false, 20, 50, 500, 0.0f, vide)));
    
    for (int x = 0; x < GRID_SIZE; x++) {
        for (int z = 0; z < GRID_SIZE; z++) {
            float posX = x * (taille_terrain.x / GRID_SIZE) - (taille_terrain.x / 2.0f);
            float posZ = z * (taille_terrain.z / GRID_SIZE) - (taille_terrain.z / 2.0f);
            
            float offsetX = random_flottant(-0.1f, 0.1f);
            float offsetZ = random_flottant(-0.1f, 0.1f);
            posX += offsetX;
            posZ += offsetZ;
            
            float height = GetHeightFromTerrain((Vector3){ posX, 0.0f, posZ }, image_sol, taille_terrain);
            
            int temperature = (int) random_flottant(0, 20);
            int humidite = (int) random_flottant(0, 30);
            int pluviometrie = (int) random_flottant(0, 100);
            
            // Calcul de la pente
            float heightLeft = GetHeightFromTerrain((Vector3){ posX - 0.3f, 0.0f, posZ }, image_sol, taille_terrain);
            float heightRight = GetHeightFromTerrain((Vector3){ posX + 0.3f, 0.0f, posZ }, image_sol, taille_terrain);
            float heightUp = GetHeightFromTerrain((Vector3){ posX, 0.0f, posZ - 0.3f }, image_sol, taille_terrain);
            float heightDown = GetHeightFromTerrain((Vector3){ posX, 0.0f, posZ + 0.3f }, image_sol, taille_terrain);
            
            float deltaLeft = fabs(height - heightLeft);
            float deltaRight = fabs(height - heightRight);
            float deltaUp = fabs(height - heightUp);
            float deltaDown = fabs(height - heightDown);
            
            float penteX = (deltaLeft + deltaRight) / 2.0f;
            float penteZ = (deltaUp + deltaDown) / 2.0f;
            float tauxPente = sqrt(penteX * penteX + penteZ * penteZ);
            
            grille[x][z].position = (Vector3){ posX, height, posZ };
            grille[x][z].active = true;
            grille[x][z].occupee = false;
            grille[x][z].humidite = humidite;
            grille[x][z].temperature = temperature;
            grille[x][z].pluviometrie = pluviometrie;
            grille[x][z].pente = tauxPente;
            grille[x][z].plante = vide;
            grille[x][z].plante.age = rand() % vide.age_max;
            grille[x][z].identifiant = x * GRID_SIZE + z;
            
            float taille = grille[x][z].plante.taille;
            Matrix transform = MatrixScale(taille, taille, taille);
            grille[x][z].model.transform = transform;
        }
    }
}

void UpdateEcosystem(std::vector<std::vector<GridCell>>& grille,
                    std::vector<Plante>& plantes,
                    std::vector<Plante>& plantes_mortes,
                    int minTemp, int maxTemp, int minHum, int maxHum) {
    
    Color couleur_sante = WHITE;
    Plante vide("Vide", 10, 100, 0, 0, 0, 0, 0, 0, 0, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0, false, 100, Model{}, WHITE);
    
    for (int x = 0; x < GRID_SIZE; x++) {
        for (int z = 0; z < GRID_SIZE; z++) {
            verifier_plante(grille, &grille[x][z], plantes, plantes_mortes, vide, 
                          minTemp, maxTemp, minHum, maxHum, couleur_sante);
        }
    }
}

void UpdateBiomes(std::vector<Biome>& les_biomes,
                 std::vector<std::vector<GridCell>>& grille,
                 int& minTemp, int& maxTemp, int& minHum, int& maxHum,
                 int& minPluv, int& maxPluv) {
    
    Biome biome_actuel = cherche_le_biome_actuelle(les_biomes);
    
    static int prev_temp_min = biome_actuel.temperature_min;
    static int prev_temp_max = biome_actuel.temperature_max;
    static int prev_hum_min = biome_actuel.humidite_min;
    static int prev_hum_max = biome_actuel.humidite_max;
    static int prev_pluv_min = biome_actuel.pluviometrie_min;
    static int prev_pluv_max = biome_actuel.pluviometrie_max;
    
    if (prev_temp_min != biome_actuel.temperature_min || 
        prev_temp_max != biome_actuel.temperature_max || 
        prev_hum_min != biome_actuel.humidite_min || 
        prev_hum_max != biome_actuel.humidite_max ||
        prev_pluv_min != biome_actuel.pluviometrie_min ||
        prev_pluv_max != biome_actuel.pluviometrie_max) {

        minTemp = biome_actuel.temperature_min;
        maxTemp = biome_actuel.temperature_max;
        minHum = biome_actuel.humidite_min;
        maxHum = biome_actuel.humidite_max;
        minPluv = biome_actuel.pluviometrie_min;
        maxPluv = biome_actuel.pluviometrie_max;

        for (int x = 0; x < GRID_SIZE; x++) {
            for (int z = 0; z < GRID_SIZE; z++) {
                grille[x][z].temperature = GetRandomValue(minTemp, maxTemp);
                grille[x][z].humidite = GetRandomValue(minHum, maxHum);
                grille[x][z].pluviometrie = GetRandomValue(minPluv, maxPluv);
            }
        }
        
        prev_temp_min = biome_actuel.temperature_min;
        prev_temp_max = biome_actuel.temperature_max;
        prev_hum_min = biome_actuel.humidite_min;
        prev_hum_max = biome_actuel.humidite_max;
        prev_pluv_min = biome_actuel.pluviometrie_min;
        prev_pluv_max = biome_actuel.pluviometrie_max;
    }
}

void UnloadEcosystemResources(Model& model_bouleau1, Model& model_bouleau2, Model& model_erable,
                             Model& model_mort_bouleau1, Model& model_mort_bouleau2, Model& model_mort_erable,
                             Model& model_sol, Model& emptyModel) {
    UnloadModel(model_bouleau1);
    UnloadModel(model_bouleau2);
    UnloadModel(model_erable);
    UnloadModel(model_mort_bouleau1);
    UnloadModel(model_mort_bouleau2);
    UnloadModel(model_mort_erable);
    UnloadModel(model_sol);
    UnloadModel(emptyModel);
}

// Fonctions utilitaires
bool is_plant_morte(const std::string& nom, const std::vector<Plante>& plantes_mortes) {
    for (const Plante& plante : plantes_mortes) {
        if (plante.nom == nom) {
            return true;
        }
    }
    return false;
}





void verifier_plante(std::vector<std::vector<GridCell>>& grille, GridCell* cellule,
                    std::vector<Plante> plantes, std::vector<Plante> plantes_mortes,
                    Plante vide, int minTemp, int maxTemp, int minHum, int maxHum, Color couleur_sante) {
    
    if(is_plant_morte(cellule->plante.nom, plantes_mortes) || cellule->plante.nom == "Vide") {
        if(cellule->plante.age >= cellule->plante.age_max) {
            Plante bestPlante = vide;
            float bestScore = 0;
            
            for (Plante plante : plantes) {
                if (cellule->temperature >= plante.temperature_min && cellule->temperature <= plante.temperature_max &&
                    cellule->humidite >= plante.humidite_min && cellule->humidite <= plante.humidite_max &&
                    cellule->pluviometrie >= plante.pluviometrie_min && cellule->pluviometrie <= plante.pluviometrie_max &&
                    cellule->pente >= plante.pente_min && cellule->pente <= plante.pente_max) {
                    
                    float scoreTemperature = fabs(cellule->temperature - ((plante.temperature_max + plante.temperature_min) / 2.0f));
                    float scoreHumidite = fabs(cellule->humidite - ((plante.humidite_max + plante.humidite_min) / 2.0f));
                    float scorePluviometrie = fabs(cellule->pluviometrie - ((plante.pluviometrie_max + plante.pluviometrie_min) / 2.0f));
                    float score = scoreTemperature + scoreHumidite + scorePluviometrie;
                    
                    if (score < bestScore || bestScore == 0) {
                        bestScore = score;
                        bestPlante = plante;
                    }
                }
            }
            
            cellule->plante = bestPlante;
            cellule->plante.age = rand() % (int)(bestPlante.age_max * 0.2f);
            cellule->plante.taille = bestPlante.taille;
            
            // Appliquer les influences sur les cases voisines
            for (int dx = -1; dx <= 1; dx++) {
                for (int dz = -1; dz <= 1; dz++) {
                    if (dx == 0 && dz == 0) continue;

                    int voisinX = (cellule->identifiant / GRID_SIZE) + dx;
                    int voisinZ = (cellule->identifiant % GRID_SIZE) + dz;

                    if (voisinX >= 0 && voisinX < GRID_SIZE && voisinZ >= 0 && voisinZ < GRID_SIZE) {
                        GridCell* voisin = &grille[voisinX][voisinZ];
                        voisin->temperature += cellule->plante.influence_temperature;
                        voisin->humidite += cellule->plante.influence_humidite;
                        voisin->humidite = Clamp(voisin->humidite, 0, 100);
                    }
                }
            }
            return;
        } else {
            cellule->plante.age++;
            return;
        }
    } else {
        cellule->plante.age++;
        if (cellule->plante.age >= cellule->plante.age_max) {
            cellule->plante.age = 0;
            float taille_actuelle = cellule->plante.taille;
            
            for (int i = 0; i < plantes_mortes.size(); i++) {
                if (cellule->plante.id == plantes_mortes[i].id) {
                    Plante planteMorteActuelle = plantes_mortes[i];
                    planteMorteActuelle.taille = taille_actuelle;
                    cellule->plante = planteMorteActuelle;
                    break;
                }
            }
            return;
        } else {
            if(cellule->plante.sante >= 1 || cellule->plante.age >= cellule->plante.age_max) {
                if (cellule->pente >= cellule->plante.pente_min &&
                    cellule->pente <= cellule->plante.pente_max) {
                    
                    Plante bestPlante = cellule->plante;
                    float bestScore = 0;
                    
                    for (Plante plante : plantes) {
                        if (cellule->temperature >= plante.temperature_min &&
                            cellule->temperature <= plante.temperature_max &&
                            cellule->humidite >= plante.humidite_min &&
                            cellule->humidite <= plante.humidite_max && 
                            cellule->pluviometrie >= plante.pluviometrie_min &&
                            cellule->pluviometrie <= plante.pluviometrie_max &&
                            cellule->pente >= plante.pente_min &&
                            cellule->pente <= plante.pente_max) {
                            
                            float scoreTemperature = fabs(cellule->temperature - (plante.temperature_max + plante.temperature_min));
                            float scoreHumidite = fabs(cellule->humidite - (plante.humidite_max + plante.humidite_min));
                            float score = scoreTemperature + scoreHumidite;
                            
                            if (score < bestScore || bestScore == 0) {
                                bestScore = score;
                                bestPlante = plante;
                            }
                        }
                    }
                    
                    if(bestPlante.nom != cellule->plante.nom) {
                        cellule->plante.sante -= 4;
                        couleur_sante = (Color){
                            (unsigned char)Clamp(cellule->plante.couleur.r - 10.0f, 0, 255),
                            (unsigned char)Clamp(cellule->plante.couleur.g - 10.0f, 0, 255),
                            (unsigned char)Clamp(cellule->plante.couleur.b - 10.0f, 0, 255),
                            cellule->plante.couleur.a
                        };
                        cellule->plante.couleur = couleur_sante;
                    }
                    
                    if (cellule->plante.taille < cellule->plante.taille_max) {
                        cellule->plante.taille *= 1.01f;
                    }
                    return;
                }
            }
        }
    }
}