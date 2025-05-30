// src/ecosystem/Biome.cpp
#include "biome.h"

Biome ajoute_biome(std::vector<Biome>& liste_biome, Biome biome) {
    liste_biome.push_back(biome);
    return biome;
}

Biome cherche_le_biome_actuelle(std::vector<Biome>& liste_biome) {
    for (auto& biome : liste_biome) {
        if (biome.biome_actuelle) {
            return biome;
        }
    }
    // Retourner le premier biome par défaut si aucun n'est actuel
    return liste_biome.empty() ? Biome("", 0, 0, 0, 0, 0, 0, WHITE, 0.0f, false) : liste_biome[0];
}

Biome change_le_biome_actuelle(std::vector<Biome>& liste_biome, Biome biome) {
    for (auto& b : liste_biome) {
        b.biome_actuelle = (b.nom == biome.nom);
    }
    return biome;
}

int get_biome_temperature_min(Biome biome) {
    return biome.temperature_min;
}

int get_biome_temperature_max(Biome biome) {
    return biome.temperature_max;
}

int get_biome_humidite_min(Biome biome) {
    return biome.humidite_min;
}

int get_biome_humidite_max(Biome biome) {
    return biome.humidite_max;
}

int get_biome_pluviometrie_min(Biome biome) {
    return biome.pluviometrie_min;
}

int get_biome_pluviometrie_max(Biome biome) {
    return biome.pluviometrie_max;
}

Color get_biome_couleur(Biome biome) {
    return biome.couleur;
}

float get_biome_densite_nuage(Biome biome) {
    return biome.densite_nuage;
}