#ifndef PLANTE_H
#define PLANTE_H

#include <raylib.h>
#include <string>
#include <vector>

class GridCell; // Forward declaration

class Plante {
public:
    std::string nom;
    int id;
    int sante;
    int humidite_min;
    int humidite_max;
    int temperature_min;
    int temperature_max;
    int pluviometrie_min;
    int pluviometrie_max;
    int influence_humidite;
    int influence_temperature;
    float taille;
    float taille_max;
    float pente_min;
    float pente_max;
    float age;
    bool morte;
    int age_max;
    Model model;
    Color couleur;

    // Constructeur
    Plante(std::string n, int id, int sante, int h_min, int h_max, int t_min, int t_max, 
           int pluviometrie_min, int pluviometrie_max, int inf_h, int inf_temp, 
           float taille, float taille_max, float pente_min, float pente_max, 
           float age, bool morte, int age_max, Model mod, Color couleur);

    // Méthodes
    void influencerVoisins(std::vector<std::vector<GridCell>>& grille, int x, int y);
};

#endif