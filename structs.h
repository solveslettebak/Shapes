#pragma once

struct sTriangle { float x1, y1, x2, y2, x3, y3; };
struct sCircle { float x, y, r; };
struct sRect { float x, y, w, h; };
struct sLine { float x1, y1, x2, y2; };

//struct sDecorativeDot { float x, y, elapsedTime, lifetime; olc::Pixel color; };

enum ForceType { eGravity, eMagnetic, eFriction, eExplosion };