#pragma once

// World constants
const int WORLD_WIDTH = 1800;
const int WORLD_HEIGHT = 1600;
const int VIEWPORT_WIDTH = 800;
const int VIEWPORT_HEIGHT = 600;
const int GRIDSIZE = 100;

const bool SHADOWS = true;

// Magbomb constants
const float MB_TIME_TO_ARM =  1.0f;
const float MB_LIFETIME    = 3.0f;
//const float MB_RADIUS       =  5.0f;
//const float MB_MAGNETISM    =  0.5f;
//const float MB_DAMAGE       =  0.5f;
const float MB_FIRING_FORCE_MULTIPLIER = 1500.0f;
const float MB_FIRING_FORCE_MINIMUM = 750.0f;
const float MB_MASS		     =  1.0f;

// Standard ship constants
const float SHIP_ROTATE_SPEED_FAST =  3.0f;
const float SHIP_ROTATE_SPEED_SLOW =  0.5f;
const float SHIP_THRUST            = 20000.0f;
const float SHIP_MASS = 100.0f;

// Laser constants
const float LASER_DAMAGE = 100.0f;

// Ninja rope constants 
//const float NR_FORCE = 100.0f;
//const float NR_ANCHOR_MASS = 1.0f;
//const float NR_LENGTH_MIN = 100.0f;
//const float NR_LENGTH_MAX = 200.0f;
//const float NR_LIFETIME = 1.0f; // if it doesn't hit anything, it will disappear after this time.
//const float NR_SPRING_CONSTANT = 1.0f;
const float NR_FIRING_FORCE = 5000.0f;