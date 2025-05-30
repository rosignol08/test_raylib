// src/rendering/lightning.cpp
#include "lightning.h"
#include <raymath.h>

// Variables globales pour la gestion de la caméra
static float angleX = 0.0f;
static float angleY = 0.0f;
static float distance_cam = 5.0f;
static bool isRotating = false;

// Fonction pour charger une render texture pour le shadow mapping
RenderTexture2D LoadShadowmapRenderTexture(int width, int height) {
    RenderTexture2D target = { 0 };

    target.id = rlLoadFramebuffer(); // Load an empty framebuffer
    target.texture.width = width;
    target.texture.height = height;

    if (target.id > 0) {
        rlEnableFramebuffer(target.id);

        // Create depth texture
        // We don't need a color texture for the shadowmap
        target.depth.id = rlLoadTextureDepth(width, height, false);
        target.depth.width = width;
        target.depth.height = height;
        target.depth.format = 19;       //DEPTH_COMPONENT_24BIT?
        target.depth.mipmaps = 1;

        // Attach depth texture to FBO
        rlFramebufferAttach(target.id, target.depth.id, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_TEXTURE2D, 0);

        // Check if fbo is complete with attachments (valid)
        if (rlFramebufferComplete(target.id)) {
            TRACELOG(LOG_INFO, "FBO: [ID %i] Framebuffer object created successfully", target.id);
        }

        rlDisableFramebuffer();
    } else {
        TRACELOG(LOG_WARNING, "FBO: Framebuffer object can not be created");
    }

    return target;
}

// Unload shadowmap render texture from GPU memory (VRAM)
void UnloadShadowmapRenderTexture(RenderTexture2D target) {
    if (target.id > 0) {
        // NOTE: Depth texture/renderbuffer is automatically
        // queried and deleted before deleting framebuffer
        rlUnloadFramebuffer(target.id);
    }
}

void InitializeCamera(Camera& camera) {
    camera.position = (Vector3){ -5.0f, 0.0f, -5.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 85.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    
    // Reset des variables de caméra
    angleX = 0.0f;
    angleY = 0.0f;
    distance_cam = 5.0f;
    isRotating = false;
}

void HandleCameraInput(Camera& camera) {
    // Activer/désactiver la rotation avec le clic droit
    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) isRotating = true;
    if (IsMouseButtonReleased(MOUSE_RIGHT_BUTTON)) isRotating = false;

    // Capture des mouvements de la souris
    if (isRotating) {
        Vector2 mouseDelta = GetMouseDelta();
        angleX -= mouseDelta.y * 0.2f; // Sensibilité verticale
        angleY -= mouseDelta.x * 0.2f; // Sensibilité horizontale
    }
    
    // Gestion du zoom avec la molette de la souris
    distance_cam -= GetMouseWheelMove() * 0.5f;
    if (distance_cam < 2.0f) distance_cam = 2.0f;   // Distance minimale
    if (distance_cam > 200.0f) distance_cam = 200.0f; // Distance maximale

    // Limiter les angles X pour éviter une rotation complète
    if (angleX > 89.0f) angleX = 89.0f;
    if (angleX < -89.0f) angleX = -89.0f;

    // Calcul de la position de la caméra en coordonnées sphériques
    float radAngleX = DEG2RAD * angleX;
    float radAngleY = DEG2RAD * angleY;

    camera.position.x = distance_cam * cosf(radAngleX) * sinf(radAngleY);
    camera.position.y = distance_cam * sinf(radAngleX);
    camera.position.z = distance_cam * cosf(radAngleX) * cosf(radAngleY);
}

Color GetSunColorFromTime(float timeOfDay) {
    if (timeOfDay < 6.0f || timeOfDay >= 20.0f) {
        return (Color){ 10, 10, 30, 255 }; // Nuit profonde - bleu très foncé
    } else if (timeOfDay < 7.0f) {
        float t = (timeOfDay - 6.0f); // Transition de 6h à 7h
        return (Color){
            (unsigned char)(255 * t),        // Rouge augmente
            (unsigned char)(165 * t),        // Vert augmente
            (unsigned char)(10 * t),         // Bleu diminue
            255
        }; // Lever du soleil - transition du bleu foncé au rose
    } else if (timeOfDay < 8.0f) {
        float t = (timeOfDay - 7.0f); // Transition de 7h à 8h
        return (Color){
            255,                     // Rouge constant
            (unsigned char)(200 + t * 55),   // Vert augmente légèrement
            (unsigned char)(100 - t * 50),   // Bleu diminue
            255
        }; // Lever du soleil - transition du rose au doré
    } else if (timeOfDay < 17.0f) {
        return (Color){ 255, 255, 255, 255 }; // Journée - blanc éclatant
    } else if (timeOfDay < 18.0f) {
        float t = (timeOfDay - 17.0f); // Transition de 17h à 18h
        return (Color){
            (unsigned char)(255 - t * 5),  // Rouge diminue
            (unsigned char)(255 - t * 55),   // Vert diminue légèrement
            (unsigned char)(100 + t * 50),   // Bleu augmente
            255
        }; // Coucher du soleil - transition du blanc au doré
    } else if (timeOfDay < 19.0f) {
        float t = (timeOfDay - 18.0f); // Transition de 18h à 19h
        return (Color){
            (unsigned char)(255 - t * 155),  // Rouge diminue
            (unsigned char)(150 - t * 100),  // Vert diminue
            (unsigned char)(100 + t * 50),   // Bleu augmente
            255
        }; // Coucher du soleil - transition du doré au bleu foncé
    } else {
        float t = (timeOfDay - 19.0f); // Transition de 19h à 20h
        return (Color){
            (unsigned char)(100 - t * 90),   // Rouge diminue
            (unsigned char)(50 - t * 40),    // Vert diminue
            (unsigned char)(150 + t * 10),   // Bleu augmente légèrement
            255
        }; // Fin du coucher du soleil - transition vers la nuit
    }
}

Vector3 CalculateSunDirection(float timeOfDay) {
    // Calcul de la direction du soleil en fonction de l'heure
    float sunAngle = ((timeOfDay - 6.0f) / 12.0f) * PI; // -PI/2 à PI/2 (6h à 18h)

    Vector3 lightDir = {
        cosf(sunAngle),           // X: Est-Ouest
        -sinf(sunAngle),          // Y: Hauteur du soleil
        0.0f                      // Z: Nord-Sud
    };
    
    return Vector3Normalize(lightDir);
}