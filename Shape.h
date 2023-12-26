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
	void setMass(float mass_) { mass = mass_; }
	float getMass() { return mass; }

	void clearForce() { forceX = 0.0f; forceY = 0.0f; }
	void addForce(float force, float angle_) { forceX += force * cos(angle_); forceY += force * sin(angle_); }
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



class Line : public Shape {
public:
	float x2, y2;
	float getX2() { return x2; }
	float getY2() { return y2; }
	Line(float x_, float y_) {
		x = x_; y = y_; angle = 0.0f;
		x2 = x + 1000.0f * cos(angle);
		y2 = y + 1000.0f * sin(angle);
	}
	float centroidX() override { return x; }
	float centroidY() override { return y; }
	float area() override { return 0.0f; }
	void rotate(float angle_) override {
		angle += angle_;
	}
	sLine getStruct() { return sLine{ x, y, x + 1000.0f * cos(angle), y + 1000.0f * sin(angle) }; }
};


// axes aligned with x and y, no rotation
class Rect : public Shape {
public:
	Rect(float x_, float y_, float w_, float h_, olc::Pixel color_) { x = x_; y = y_; w = w_; h = h_; color = color_; }
	float w, h;
	//olc::Pixel color;
	float centroidX() override { return (x + w) / 2.0f; } // TODO: Rect isn't updated to have world coordinates and local yet. has only world.
	float centroidY() override { return (y + h) / 2.0f; }
	float area() override { return w * h; }
	void rotate(float angle_) override {
		angle += angle_; // no rotation, but can be used for direction, if it's a moving object.
	}
	sRect getStruct() { return sRect{ x, y, w, h }; }
};


class Circle : public Shape {
public:
	float r;
	Circle(float x_, float y_, float r_, olc::Pixel color_) { x = x_; y = y_; r = r_; color = color_; }
	float centroidX() override { return 0.0f; }
	float centroidY() override { return 0.0f; }
	float area() override { return 3.14f * r * r; }
	void rotate(float angle_) override { angle += angle_; }
};


class Triangle : public Shape {
public:
	Triangle(sTriangle t, float x_, float y_, olc::Pixel color_) {
		original_shape = t;
		x = x_; y = y_;
		angle = 0.0f;
		color = color_;

		float cx = centroidX();
		float cy = centroidY();
		original_shape.x1 -= cx; original_shape.y1 -= cy;
		original_shape.x2 -= cx; original_shape.y2 -= cy;
		original_shape.x3 -= cx; original_shape.y3 -= cy;
		rotated_shape = original_shape;
	}

	sTriangle worldCoordinates() {
		sTriangle result;
		applyRotation();
		result.x1 = rotated_shape.x1 + x;
		result.y1 = rotated_shape.y1 + y;
		result.x2 = rotated_shape.x2 + x;
		result.y2 = rotated_shape.y2 + y;
		result.x3 = rotated_shape.x3 + x;
		result.y3 = rotated_shape.y3 + y;
		return result;
	}
	float area() { return 0.5f * abs((original_shape.x1 - original_shape.x3) * (original_shape.y2 - original_shape.y1) - (original_shape.x1 - original_shape.x2) * (original_shape.y3 - original_shape.y1)); }
	float centroidX() override { return (original_shape.x1 + original_shape.x2 + original_shape.x3) / 3.0f; }
	float centroidY() override { return (original_shape.y1 + original_shape.y2 + original_shape.y3) / 3.0f; }
	void rotate(float angle) override;
	void applyRotation();
	sTriangle rotated_shape, original_shape;
};

void Triangle::applyRotation() {
	float cx = centroidX();
	float cy = centroidY();
	rotated_shape.x1 = cx + (original_shape.x1 - cx) * cos(angle) - (original_shape.y1 - cy) * sin(angle);
	rotated_shape.y1 = cy + (original_shape.x1 - cx) * sin(angle) + (original_shape.y1 - cy) * cos(angle);
	rotated_shape.x2 = cx + (original_shape.x2 - cx) * cos(angle) - (original_shape.y2 - cy) * sin(angle);
	rotated_shape.y2 = cy + (original_shape.x2 - cx) * sin(angle) + (original_shape.y2 - cy) * cos(angle);
	rotated_shape.x3 = cx + (original_shape.x3 - cx) * cos(angle) - (original_shape.y3 - cy) * sin(angle);
	rotated_shape.y3 = cy + (original_shape.x3 - cx) * sin(angle) + (original_shape.y3 - cy) * cos(angle);
}

void Triangle::rotate(float angle_) { angle += angle_; applyRotation(); }
