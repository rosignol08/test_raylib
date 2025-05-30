// src/rendering/lightning.h
#ifndef LIGHTNING_H
#define LIGHTNING_H

#include <raylib.h>
#include <rlgl.h>

// Fonctions pour le shadow mapping (basées sur le code de @TheManTheMythTheGameDev)
RenderTexture2D LoadShadowmapRenderTexture(int width, int height);
void UnloadShadowmapRenderTexture(RenderTexture2D target);

// Fonctions de gestion de la caméra
void HandleCameraInput(Camera& camera);
void InitializeCamera(Camera& camera);

// Fonctions utilitaires d'éclairage
Color GetSunColorFromTime(float timeOfDay);
Vector3 CalculateSunDirection(float timeOfDay);

#endif