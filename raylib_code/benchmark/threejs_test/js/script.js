import * as THREE from 'three';
import { OrbitControls } from './OrbitControls.js';
import Stats from 'https://cdnjs.cloudflare.com/ajax/libs/stats.js/17/Stats.js'

// Le canvas
let cnv = document.querySelector('#canvas');
if (!cnv) {
    console.error("Canvas introuvable !");
}

/**
 * Crée un cube avec des dimensions et une couleur.
 *
 * @param {number} x - la coordonnée x du cube.
 * @param {number} y - la coordonnée y du cube.
 * @param {number} z - la coordonnée z du cube.
 * @param {number} w - la largeur du cube.
 * @param {number} h - la hauteur du cube.
 * @param {number} d - la profondeur du cube.
 * @param {number|string} color - la couleur du cube.
 * @returns {THREE.Mesh} - le cube créé.
 */
function cree_cube(x, y, z, w, h, d, color) {
    let geometry = new THREE.BoxGeometry(w, h, d);
    let material = new THREE.MeshBasicMaterial({ color: color });
    let cube = new THREE.Mesh(geometry, material);
    cube.position.set(x, y, z);
    return cube;
}

// Initialisation du renderer
const renderer = new THREE.WebGLRenderer({ canvas: cnv, antialias: true });
renderer.setSize(window.innerWidth, window.innerHeight);
renderer.shadowMap.enabled = true; // Activer les ombres

// Création de la scène et de la caméra
const scene = new THREE.Scene();
const camera = new THREE.PerspectiveCamera(40, window.innerWidth / window.innerHeight, 0.1, 1000000);
camera.position.z = 50; // Éloigner la caméra pour voir tous les cubes

// Contrôles de la caméra
const controls = new OrbitControls(camera, renderer.domElement);

// Ajout de 50 000 cubes à des positions aléatoires
const cubeCount = 50000;
for (let i = 0; i < cubeCount; i++) {
    const x = (Math.random() - 0.5) * 20; // Position x aléatoire entre -10 et 10
    const y = (Math.random() - 0.5) * 20; // Position y aléatoire entre -10 et 10
    const z = (Math.random() - 0.5) * 20; // Position z aléatoire entre -10 et 10
    const color = Math.random() * 0xffffff; // Couleur aléatoire
    const cube = cree_cube(x, y, z, 1, 1, 1, color); // Taille 1x1x1
    scene.add(cube);
}

// Ajout de stats.js pour mesurer les FPS
const stats = new Stats();
stats.showPanel(0); // Affiche uniquement les FPS
document.body.appendChild(stats.dom);

// Fonction d'animation
function animate() {
    stats.begin(); // Début de la mesure des FPS

    // Rendu de la scène
    renderer.render(scene, camera);
    controls.update();

    stats.end(); // Fin de la mesure des FPS

    requestAnimationFrame(animate);
}

// Lancement de l'animation
animate();

// Redimensionnement de la fenêtre
window.addEventListener('resize', () => {
    camera.aspect = window.innerWidth / window.innerHeight;
    camera.updateProjectionMatrix();
    renderer.setSize(window.innerWidth, window.innerHeight);
});