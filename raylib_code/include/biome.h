#include <raylib.h>
#include <vector>
#include <string>
#include <iostream>

using namespace std;

//- nom du biome
//- temperature_min: température minimale du biome (en degrés)
//- temperature_max: température maximale du biome (en degrés)
//- humidite_min: humidité minimale du biome (en pourcentage)
//- humidite_max: humidité maximale du biome (en pourcentage)
//- pluviometrie_min: pluviométrie minimale du biome (en mm)
//- pluviometrie_max: pluviométrie maximale du biome (en mm)
//- couleur: couleur associée au biome
//- densite_nuage: densité de nuages dans le biome
//- biome_actuelle: booléen indiquant si le biome est actuellement actif
struct Biome {
    public:
        string nom;
        int temperature_min;
        int temperature_max;
        int humidite_min;
        int humidite_max;
        int pluviometrie_min;
        int pluviometrie_max;
        Color couleur;
        float densite_nuage;
        bool biome_actuelle = false;
        float frequence_pluie;
        Biome(string nom, int t_min, int t_max, int h_min, int h_max, int pluv_min, int pluv_max, Color couleur, float densite_nuage, bool biome_actuelle, float frequence_pluie = 0.0f)
            : nom(nom), temperature_min(t_min), temperature_max(t_max), humidite_min(h_min), humidite_max(h_max), pluviometrie_min(pluv_min),pluviometrie_max(pluv_max), couleur(couleur), densite_nuage(densite_nuage), biome_actuelle(biome_actuelle), frequence_pluie(frequence_pluie) {}
};

Biome ajoute_biome(vector<Biome>& liste_biome, Biome biome);
Biome cherche_le_biome_actuelle(vector<Biome>& liste_biome);
Biome change_le_biome_actuelle(vector<Biome>& liste_biome, Biome biome);

int get_biome_temperature_min(Biome biome);
int get_biome_temperature_max(Biome biome);

int get_biome_humidite_min(Biome biome);
int get_biome_humidite_max(Biome biome);

int get_biome_pluviometrie_min(Biome biome);
int get_biome_pluviometrie_max(Biome biome);

Color get_biome_couleur(Biome biome);
float get_biome_densite_nuage(Biome biome);
float get_biome_frequence_pluie(Biome biome);
/*
les parametres defini par notre météo : 
windStrength : à voir pour le rendre plus intererssant avec le shader de l'herbe pour qu'il aille vraiment plus vite
cloudThreshold : nuages
hum_modifieur : modifie l'humidité de la case
temperature_modifieur : modifie la température de la case
*/