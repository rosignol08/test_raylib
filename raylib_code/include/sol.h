#ifndef SOL_H
#define SOL_H

#include <raylib.h>
#include <vector>
#include <string>
#include <iostream>

using namespace std;

class Plante {
public:
    string nom;
    int humidite_min;
    int humidite_max;
    int temperature_min;
    int temperature_max;
    int influence_humidite;
    int influence_temperature;
    float taille;
    float taille_max;
    float pente_max;
    int age;
    bool morte;
    Model model;

    //Plante cree_plante(string n, int h_min, int t_min, int inf_h, int inf_t, float taille, float pen, int age, Model mod);
    // Constructeur
    Plante(string n, int h_min, int h_max, int t_min, int t_max, int inf_h, int inf_t, float taille, float taille_max, float pen, int age, bool morte,Model mod);

    void influencerVoisins(vector<vector<class GridCell>>& grille, int x, int y);
    void vieux();
    void verifierConditionsEtMourir(vector<vector<GridCell>>& grille, int x, int y);
    void meurt();
};

class GridCell {
public:
    Vector3 position;
    Model model;
    bool active;
    bool occupee;
    int temperature;
    int humidite;
    float pente;
    Plante plante;

    GridCell(Vector3 pos, Model mod, bool act, bool occupee, int temp, int hum, float pen);
    void update(vector<vector<GridCell>>& grille, int x, int y);
};

#endif




// monobjet.h
/*
#ifndef MONOBJET_H
#define MONOBJET_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MonObjet MonObjet;

MonObjet* MonObjet_nouveau(int x);
void MonObjet_afficher(MonObjet* obj);
void MonObjet_supprimer(MonObjet* obj);

#ifdef __cplusplus
}
#endif

#endif // MONOBJET_H
*/