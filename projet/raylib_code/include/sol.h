#ifndef SOL_H
#define SOL_H

#include <raylib.h>
#include <vector>
#include <string>
#include <iostream>

using namespace std;
// Charger un modèle 3D
//Model Load3DModel(const char* fileName) {
//    Model model_mort = LoadModel(fileName);
//    return model;
//}
extern Model model_mort;
//extern Plante plante_morte;

class Plante {
public:
    string nom;
    int sante;
    int humidite_min;
    int humidite_max;
    int temperature_min;
    int temperature_max;
    int influence_humidite;
    int influence_temperature;
    float taille;
    float taille_max;
    float pente_min;
    float pente_max;
    int age;
    bool morte;
    int age_max;
    Model model;
    Color couleur;

    //Plante cree_plante(string n, int h_min, int t_min, int inf_h, int inf_temp, float taille, float pen, int age, Model mod);
    // Constructeur
    Plante(string n,int sante, int h_min, int h_max, int t_min, int t_max, int inf_h, int inf_temp, float taille, float taille_max, float pente_min, float pente_max, int age, bool morte, int age_max, Model mod, Color couleur);

    void influencerVoisins(vector<vector<class GridCell>>& grille, int x, int y);
    //void vieux();
    //void verifierConditionsEtMourir(vector<vector<GridCell>>& grille, int x, int y);
    //void meurt();
    
};

class GridCell {
public:
    int identifiant;
    Vector3 position;
    Model model;
    bool active;
    bool occupee;
    int temperature;
    int humidite;
    float pente;
    Plante plante;

    GridCell(int id, Vector3 pos, Model mod, bool act, bool occupee, int temp, int hum, float pen, Plante plante);
    //void update(vector<vector<GridCell>>& grille, int x, int y);
};

class billboard {
public:
    Vector3 position;
    Texture2D texture;
    Shader shader; // Add shader to the billboard class
    bool active;
    bool occupee;
    int temperature;
    int humidite;
    float pente;
    Vector3 billPositionStatic;
    Vector3 billPositionRotating;
    Rectangle source;
    Vector3 billUp;
    Vector2 size;

    billboard(Vector3 pos, Texture2D tex, Shader shd, bool act, bool occupee, int temp, int hum, float pen, Rectangle source, Vector3 billUp, Vector2 size);
};

class SolHerbe {
public:
    Vector3 position;
    billboard model;
    bool active;
    bool occupee;
    int temperature;
    int humidite;
    float pente;

    SolHerbe(Vector3 pos, billboard mod, bool act, bool occupee, int temp, int hum, float pen);
};

#endif