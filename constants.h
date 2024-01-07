#pragma once

// Game settings
const bool SHADOWS = true;
const int VISIBLE_COLOR_R = 0;
const int VISIBLE_COLOR_G = 0;
const int VISIBLE_COLOR_B = 64;

// World constants
const int WORLD_WIDTH = 1800;
const int WORLD_HEIGHT = 1600;
const int VIEWPORT_WIDTH = 800;
const int VIEWPORT_HEIGHT = 600;
const int GRIDSIZE = 100;

// Magbomb constants
const float MB_TIME_TO_ARM =  1.0f;
const float MB_LIFETIME    = 3.0f;
//const float MB_BALL_SIZE       =  5.0f;
const float MB_MAGNETISM    =  100.0f;
const float MB_DAMAGE       =  10.0f;
const float MB_FIRING_FORCE_MULTIPLIER = 1500.0f;
const float MB_FIRING_FORCE_MINIMUM = 750.0f;
const float MB_MASS		     =  1.0f;
const float MB_RADIUS_OF_INFLUENCE = 200.0f;
const float MB_BOUNCINESS = 1.0f; // not ready for reduced bounciness yet. Causes trouble in my only semi-functional collision detection. Need proper static collision resolution first.

// Standard ship constants
const float SHIP_ROTATE_SPEED_FAST =  3.0f;
const float SHIP_ROTATE_SPEED_SLOW =  0.5f;
const float SHIP_THRUST            = 20000.0f;
const float SHIP_MASS = 100.0f;

// Laser constants
const float LASER_DAMAGE = 100.0f;

// Ninja rope constants 
const float NR_FORCE_MULTIPLIER = 100.0f;
//const float NR_ANCHOR_MASS = 1.0f;
//const float NR_LENGTH_MIN = 100.0f;
//const float NR_LENGTH_MAX = 200.0f;
//const float NR_LIFETIME = 1.0f; // if it doesn't hit anything, it will disappear after this time.
//const float NR_SPRING_CONSTANT = 1.0f;
const float NR_FIRING_FORCE = 5000.0f;