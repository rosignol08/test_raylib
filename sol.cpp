// monobjet.cpp
#include <iostream>

class MonObjet {
public:
    MonObjet(int x) : valeur(x) {}
    void afficher() const {
        std::cout << "Valeur: " << valeur << std::endl;
    }
private:
    int valeur;
};

// Déclarations des fonctions C pour accéder à l'objet C++
extern "C" {
    MonObjet* MonObjet_nouveau(int x) {
        return new MonObjet(x);
    }

    void MonObjet_afficher(MonObjet* obj) {
        obj->afficher();
    }

    void MonObjet_supprimer(MonObjet* obj) {
        delete obj;
    }
}
