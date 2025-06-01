#include "biome.h"
#include <raylib.h>
#include <iostream>
#include <vector>

Biome ajoute_biome(vector<Biome>& liste_biome, Biome biome) {
    liste_biome.push_back(biome);
    return biome;
}

Biome cherche_le_biome_actuelle(vector<Biome>& liste_biome){
    for (size_t i = 0; i < liste_biome.size(); i++) {
        if (liste_biome[i].biome_actuelle) {
            return liste_biome[i];
        }
    }
    return Biome("", 0, 0, 0, 0, 0, 0, WHITE, 0.0f, false); // Retourne une météo vide si aucune n'est trouvée
}

Biome change_le_biome_actuelle(vector<Biome>& liste_biome, Biome biome){
    for (size_t i = 0; i < liste_biome.size(); i++) {
        if (liste_biome[i].biome_actuelle) {
            liste_biome[i].biome_actuelle = false; // Désactive la météo actuelle
        }
    }
    for (size_t i = 0; i < liste_biome.size(); i++) {
        if (liste_biome[i].nom == biome.nom) {
            liste_biome[i].biome_actuelle = true; // Active la nouvelle météo
            return liste_biome[i];
        }
    }
    return Biome("", 0, 0, 0, 0, 0, 0, WHITE, 0.0f, false); // Retourne une météo vide si aucune n'est trouvée
}

int get_biome_temperature_min(Biome biome){
    return biome.temperature_min;
}

int get_biome_temperature_max(Biome biome){
    return biome.temperature_max;
}

int get_biome_humidite_min(Biome biome){
    return biome.humidite_min;
}

int get_biome_humidite_max(Biome biome){
    return biome.humidite_max;
}

int get_biome_pluviometrie_min(Biome biome){
    return biome.pluviometrie_min;
}

int get_biome_pluviometrie_max(Biome biome){
    return biome.pluviometrie_max;
}

Color get_biome_couleur(Biome biome){
    return biome.couleur;
}

//TODO voir si on affecte cloudThreshold autant que noiseScale
float get_biome_densite_nuage(Biome biome){
    return biome.densite_nuage;
}

float get_biome_frequence_pluie(Biome biome) {
    return biome.frequence_pluie;
}