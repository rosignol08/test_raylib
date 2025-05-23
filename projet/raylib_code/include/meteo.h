#include <raylib.h>
#include <vector>
#include <string>
#include <iostream>

using namespace std;

struct Meteo {
    public:
        string nom;
        int temperature;
        int humidite;
        Color couleur;
        float densite_nuage;
        bool meteo_actuelle = false;
        Meteo(string nom, int temperature, int humidite, Color couleur, float densite_nuage, bool meteo_actuelle)
            : nom(nom), temperature(temperature), humidite(humidite), couleur(couleur), densite_nuage(densite_nuage), meteo_actuelle(meteo_actuelle) {}
};

Meteo ajoute_meteo(vector<Meteo>& liste_meteo, Meteo meteo);
Meteo cherche_la_meteo_actuelle(vector<Meteo>& liste_meteo);
Meteo change_la_meteo_actuelle(vector<Meteo>& liste_meteo, Meteo meteo);
int get_meteo_temperature(Meteo meteo);
int get_meteo_humidite(Meteo meteo);
Color get_meteo_couleur(Meteo meteo);
float get_meteo_densite_nuage(Meteo meteo);
/*
les parametres defini par notre météo : 
windStrength : à voir pour le rendre plus intererssant avec le shader de l'herbe pour qu'il aille vraiment plus vite
cloudThreshold : nuages
hum_modifieur : modifie l'humidité de la case
temperature_modifieur : modifie la température de la case
*/