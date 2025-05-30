#ifndef BIOME_H
#define BIOME_H

#include <raylib.h>
#include <vector>
#include <string>

struct Biome {
    std::string nom;
    int temperature_min;
    int temperature_max;
    int humidite_min;
    int humidite_max;
    int pluviometrie_min;
    int pluviometrie_max;
    Color couleur;
    float densite_nuage;
    bool biome_actuelle = false;
    
    Biome(std::string nom, int t_min, int t_max, int h_min, int h_max, 
          int pluv_min, int pluv_max, Color couleur, float densite_nuage, bool biome_actuelle)
        : nom(nom), temperature_min(t_min), temperature_max(t_max), 
          humidite_min(h_min), humidite_max(h_max), 
          pluviometrie_min(pluv_min), pluviometrie_max(pluv_max), 
          couleur(couleur), densite_nuage(densite_nuage), biome_actuelle(biome_actuelle) {}
};

// Fonctions utilitaires pour les biomes
Biome ajoute_biome(std::vector<Biome>& liste_biome, Biome biome);
Biome cherche_le_biome_actuelle(std::vector<Biome>& liste_biome);
Biome change_le_biome_actuelle(std::vector<Biome>& liste_biome, Biome biome);

int get_biome_temperature_min(Biome biome);
int get_biome_temperature_max(Biome biome);
int get_biome_humidite_min(Biome biome);
int get_biome_humidite_max(Biome biome);
int get_biome_pluviometrie_min(Biome biome);
int get_biome_pluviometrie_max(Biome biome);
Color get_biome_couleur(Biome biome);
float get_biome_densite_nuage(Biome biome);

#endif