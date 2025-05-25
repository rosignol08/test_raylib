
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

    //Plante cree_plante(string n, int h_min, int t_min, int inf_h, int inf_temp, float taille, float pen, int age, Model mod);
    // Constructeur
    Plante(string n,int id, int sante, int h_min, int h_max, int t_min, int t_max, int pluviometrie_min, int pluviometrie_max, int inf_h, int inf_temp, float taille, float taille_max, float pente_min, float pente_max, float age, bool morte, int age_max, Model mod, Color couleur);

    void influencerVoisins(vector<vector<class GridCell>>& grille, int x, int y);
    //void vieux();
    //void verifierConditionsEtMourir(vector<vector<GridCell>>& grille, int x, int y);
    //void meurt();
    
};
/* Classe représentant une cellule de la grille 
* arguments:
* - id: identifiant de la cellule 
* - pos: position de la cellule dans l'espace 3D
* - mod: modèle 3D associé à la cellule
* - act: booléen indiquant si la cellule est active
* - occupee: booléen indiquant si la cellule est occupée par une plante
* - temp: température de la cellule
* - hum: humidité de la cellule
* - pluviometrie: pluviométrie de la cellule
* - pen: pente de la cellule
* - plante: plante associée à la cellule
*/
class GridCell {
public:
    int identifiant;
    Vector3 position;
    Model model;
    bool active;
    bool occupee;
    int temperature;
    int humidite;
    int pluviometrie;
    float pente;
    Plante plante;

    GridCell(int id, Vector3 pos, Model mod, bool act, bool occupee, int temp, int hum, int pluviometrie, float pen, Plante plante);
    //void update(vector<vector<GridCell>>& grille, int x, int y);
};
