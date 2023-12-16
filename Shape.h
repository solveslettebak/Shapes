#pragma once
#include "olcPixelGameEngine.h"

class ITrigger;
class IHull;

#include "AI.h"
#include "Hull.h"
#include "Trigger.h"

#include <memory>
#include <vector>

using namespace std;

class Shape {
public:
	virtual float centroidX() = 0;
	virtual float centroidY() = 0;
	virtual float area() = 0;
	virtual void rotate(float angle_) = 0; // rotate around centroid
	float getAngle() {
		return angle;
	}
	void setAngle(float angle_) { angle = angle_; }
	float getX() {
		return x;
	}
	float getY() { return y; }
	void setX(float x_) { x = x_; }
	void setY(float y_) { y = y_; }

	void setAI(shared_ptr<AI> ai_) { ai = ai_; }
	void setTrigger(shared_ptr<ITrigger> trigger_) { varTrigger = trigger_; }
	void trigger(shared_ptr<Shape> owner, shared_ptr<Shape> victim) { varTrigger->trigger(owner, victim); };
	void damage(float amount) { if (canBeDamaged) hull->damage(amount); }
	void setKillFlag() { killFlag = true; }
	bool getKillFlag() { return killFlag; }
	void setCanBeDamaged(bool canBeDamaged_) { canBeDamaged = canBeDamaged_; }
	void setHull(shared_ptr<IHull> hull_) { hull = hull_; }

	void clearForce() { forceX = 0.0f; forceY = 0.0f; }
	void addForce(float force, float angle_) { forceX += force * cos(angle_); forceY += force * sin(angle_); }
	//void applyForce() { vx += forceX / mass; vy += forceY / mass; x += vx; y += vy; clearForce(); };
	void applyForce() { ax = forceX / mass; ay = forceY / mass; vx += ax; vy += ay; clearForce(); };

	void setColor(olc::Pixel color_) { color = color_; }
	olc::Pixel getColor() { return color; }
	shared_ptr<AI> getAI() { return ai; }

	void updatePosition() { x += vx; y += vy; }
	float getMotionAngle() {
		float speed = sqrt(vx * vx + vy * vy);
		if (speed < 0.01f) return angle;
		return atan2(vy, vx);
	}


	//void setColor(olc::Pixel color_) { color = color_; }

	// worldCoord, screenCoord. minimapCoord også?
protected:
	float vx = 0, vy = 0, ax = 0, ay = 0; // velocity, acceleration

	float mass = 100; // kg
	float angle; // radians
	float forceX = 0.0f;
	float forceY = 0.0f;
	bool isStatic;
	bool canCollide;
	bool killFlag = false;
	bool canBeDamaged = false;
	float x, y; // world coordinates

	shared_ptr<ITrigger> varTrigger;
	shared_ptr<IHull> hull;
	shared_ptr<AI> ai;
	olc::Pixel color;
	// bool isColliding; // could do this, but keep a vector in World class instead. Only properties goes in objects.
};
