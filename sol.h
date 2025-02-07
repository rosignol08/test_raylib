// monobjet.h
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
