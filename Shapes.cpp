// TODO: 
// DONE - shapes trenger koordinater relativt til cetroid, i tillegg til world coordinates. Tror jeg.
// DONE - AI: follow user
// DONE - Line on triangle collision detection. Gjenstår å finne nærmeste punkt på triangel, og returnere det.
// DONE - Line collision: Må oppdatere line-shape, slik at draw blir til riktig punkt
// - I stedet for å lage laser-linje for hver frame, og slette den igjen, kan jeg lage en AI subclass. .. men får samme problem, med å slette den igjen.
// - User control burde også være en AI subclass. Dermed trenger AI user input, og jeg kan bruke det andre steder også.
// DONE - Collision detect: return hit object
// DONE - Damage. Hvordan?
// - Types of forces. How to implement? force() to return enum, and then apply force in World class? Point source, field (gravity), friction (air resistance), .. more?
// GAVE UP - Time for code cleanup and separate into files and classes.
// DONE - Move shape color into Shape class, not struct.
// - Collisions: Triangle-Rect, Line-circle, point-triangle, point-rect, point-circle, circle-circle, circle-rect, rect-rect
// - Gyro
// - Ninja rope
// - rotational physics. (angular momentum, torque, etc). Currently only setting the values directly. 
// - Really do need to figure out about calling stuff. Otherwise this project is dead.
// - may need to redesign a bit so everything is handled in World class, and references to this are sent everywhere, or even just be global.
// 
// - AI: rename to "controller" maybe?
// - Force-class maybe? instead of the enum solution. an AI can then add multiple forces and types of forces
// - AI: flee, follow, patrol, attack, defend, random, etc


// Notes for next version:
// - Obj should come back, and have a shape. Shape refers back to Obj with an owner field. Not sure why I didn't do that in the end..
// - Need to figure out how objects should be talking to each other. And structure everything neatly from the start.

#pragma once
#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <memory>

#include "Trigger.h"
#include "Hull.h"
#include "AI.h"
#include "Shape.h"

/*
Effects:
- gravity / motion. Different types: force relative to point, relative to horizontal/vertical, and probably more..
- friction (air resistance)
- damage (later: heat, piercing, explosion, etc)
- how to fit in gyro/aim here? in player: Laser l; if l.collide .. ?? yeah must hard code that anyway. Can put a variable for that specifically, and a test.

*/

//enum ForceType { eGravity, eMagnetic, eFriction };

using namespace std;

#include "structs.h"

class AI;
class Shape;


class triggerLaser : public ITrigger {
public:
	void trigger(shared_ptr<Shape> owner, shared_ptr<Shape> victim) override {
		victim->damage(0.1f);
	};
};



class playerHull : public IHull {
public:
	playerHull(shared_ptr<Shape> owner_) { maxArmor = 100.0f; currentArmor = 100.0f; owner = owner_; }
	void damage(float amount) override {
		currentArmor -= amount;
		if (currentArmor < 0.0f) {
			owner->setKillFlag();
			cout << "Player killed" << endl;
		}
	}
};



class AI_magbomb : public AI {
public:
	AI_magbomb(shared_ptr<Shape> self_, shared_ptr<Shape> external_) { self = self_; external = external_; }
	// should wait a second after creation, then start attracting objects.
	// should change color when activated
	// should explode when it collides with something
	// pushes all objects around it away when it hits something (apply small damage to them as well, depending on distance)

	void update(float tElapsedTime) override {
		timePassed += tElapsedTime;
		if ((timePassed > 1.0f) && (!magnetic)) { // called once when initial delay is over
			magnetic = true;
			self->setColor(olc::YELLOW);
		}
		if ((timePassed > 0.5f) && (timePassed < 1.5f)) {
			// slow down
			self->addForce(-0.01f, self->getMotionAngle());
		}
	}

	bool force(float &px, float &py, float &magnitude, float &radius_of_influence, ForceType &ftype) override {
		if (!magnetic) return false;
		px = self->getX();
		py = self->getY();
		magnitude = 0.01f;
		radius_of_influence = 100.0f;
		ftype = eMagnetic;
		return true;
	}
protected:
	float timePassed = 0.0f;
	bool magnetic = false;
};


class AI_follow_user : public AI {
public:
	AI_follow_user(shared_ptr<Shape> self_, shared_ptr<Shape> external_) { self = self_;  external = external_; }

	void update(float tElapsedTime) override {
		float dx = external->getX() - self->getX();
		float dy = external->getY() - self->getY();
		float angle = atan2(dy, dx);
		self->setAngle(angle + 3.14f);
	}
};

class AI_laser : public AI {
public:
	AI_laser(shared_ptr<Shape> self_, shared_ptr<Shape> external_) { self = self_; external = external_; }

	void update(float tElapsedTime) override {
		self->setAngle(external->getAngle());
		self->setX(external->getX());
		self->setY(external->getY());

		// can do this if i pass a reference to World maybe.
		//if (GetKey(olc::Key::SPACE).bReleased) {
			// kill self
		//}
	}

};



class World : public olc::PixelGameEngine {

private:
	vector<shared_ptr<Triangle>> sharedPtrTriangles;
	vector<shared_ptr<Rect>> sharedPtrRectangles;
	vector<shared_ptr<Line>> sharedPtrLines;
	vector<shared_ptr<Circle>> sharedPtrCircles;
	
	shared_ptr<Shape> user_controlled_shape;

	vector<shared_ptr<AI>> sharedPtrAI;
	vector<shared_ptr<ITrigger>> sharedPtrTriggers;
	vector<shared_ptr<IHull>> sharedPtrHulls;
	

public:
	World() { sAppName = "World"; }

	void addRandomEnemy() {
		shared_ptr<Triangle> shape2 = make_shared<Triangle>(sTriangle{ 10, 8, 10, 22, 30, 15 }, rand() % ScreenWidth(), rand() % ScreenHeight(), olc::RED);
		sharedPtrTriangles.push_back(shape2);

		shared_ptr<AI> enemy_AI = make_shared<AI_follow_user>(shape2, user_controlled_shape);
		sharedPtrAI.push_back(enemy_AI);
		shape2->setAI(enemy_AI); // not used yet

		shared_ptr<IHull> hull = make_shared<playerHull>(shape2);
		sharedPtrHulls.push_back(hull);
		shape2->setHull(hull);
		shape2->setCanBeDamaged(true);

	}

	bool OnUserCreate() override {

		shared_ptr<Triangle> shape = make_shared<Triangle>(sTriangle { 10, 8, 10, 22, 30, 15}, 50, 50, olc::RED);
		sharedPtrTriangles.push_back(shape);
		user_controlled_shape = shape;

		for (int i = 0; i < 20; i++)
			addRandomEnemy();
		
		//shape = sharedPtrTriangles[4];
		//user_controlled_shape = shape; //  std::dynamic_pointer_cast<Shape>(sharedPtrTriangles[3]);
		/*
		for (int i = 0; i < 5; i++) {
			shared_ptr<Circle> circle = make_shared<Circle>(rand() % ScreenWidth(), rand() % ScreenHeight(), 5.0f, olc::GREEN);
			sharedPtrCircles.push_back(circle);

			shared_ptr<AI> ai = make_shared<AI_magbomb>(circle, nullptr);
			sharedPtrAI.push_back(ai);
		}*/

		shared_ptr<Rect> leftWall = make_shared<Rect>(10,10, 5, ScreenHeight()-20, olc::WHITE);
		shared_ptr<Rect> rightWall = make_shared<Rect>(ScreenWidth()-15, 10, 5, ScreenHeight()-20, olc::WHITE);
		shared_ptr<Rect> topWall = make_shared<Rect>(10, 10, ScreenWidth()-20, 5, olc::WHITE);
		shared_ptr<Rect> bottomWall = make_shared<Rect>(10, ScreenHeight()-15, ScreenWidth()-20, 5, olc::WHITE);
		sharedPtrRectangles.push_back(leftWall);
		sharedPtrRectangles.push_back(rightWall);
		sharedPtrRectangles.push_back(topWall);
		sharedPtrRectangles.push_back(bottomWall);

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override {
		Clear(olc::DARK_BLUE);

		// ----- BEHAVIOUR ----- //

		if (GetKey(olc::Key::SPACE).bHeld) {
			shared_ptr<Line> laser;
			laser = make_shared<Line>(user_controlled_shape->getX(), user_controlled_shape->getY());
			laser->setAngle(user_controlled_shape->getAngle());

			shared_ptr<ITrigger> trigger = make_shared<triggerLaser>();
			sharedPtrTriggers.push_back(trigger);
			laser->setTrigger(trigger);

			sharedPtrLines.push_back(laser);

		}

		if (GetKey(olc::Key::SPACE).bPressed) {
			shared_ptr<Circle> circle = make_shared<Circle>(user_controlled_shape->getX(), user_controlled_shape->getY(), 5.0f, olc::GREEN);
			sharedPtrCircles.push_back(circle);
			circle->addForce(15.0f, user_controlled_shape->getAngle());

			shared_ptr<IHull> hull = make_shared<playerHull>(circle);
			sharedPtrHulls.push_back(hull);
			circle->setHull(hull);
			circle->setCanBeDamaged(true);

			shared_ptr<AI> ai = make_shared<AI_magbomb>(circle, nullptr);
			sharedPtrAI.push_back(ai);
			circle->setAI(ai);
		}

		if (GetKey(olc::Key::LEFT).bHeld)  { user_controlled_shape->rotate(-0.002f); }
		if (GetKey(olc::Key::COMMA).bHeld) { user_controlled_shape->rotate(-0.0005f); }
		if (GetKey(olc::Key::PERIOD).bHeld) { user_controlled_shape->rotate(0.0005f); }
		if (GetKey(olc::Key::RIGHT).bHeld) { user_controlled_shape->rotate( 0.002f); }
		if (GetKey(olc::Key::UP).bHeld) { user_controlled_shape->addForce(0.008f, user_controlled_shape->getAngle()); }

		for (auto& each : sharedPtrAI) { 
			if (each->getDestroyFlag()) {
				sharedPtrAI.erase(std::remove(sharedPtrAI.begin(), sharedPtrAI.end(), each), sharedPtrAI.end());
				break; // find a better way. see the other delete stuff thingy
			}
			else { each->update(fElapsedTime); }
		}

		// ----- COLLISION ----- //

		checkCollisions();

		// ---- DRAW ----- //

		for (auto& each : sharedPtrTriangles)  { DrawTriangle(each->worldCoordinates(), each->getColor()); }
		for (auto& each : sharedPtrRectangles) { FillRect(int(each->getX()), int(each->getY()), int(each->w), int(each->h), each->getColor()); }
		for (auto& each : sharedPtrLines)      { DrawLine(int(each->getX()), int(each->getY()), int(each->getX2()), int(each->getY2()), olc::YELLOW); }
		for (auto& each : sharedPtrCircles)    { 
			FillCircle(int(each->getX()), int(each->getY()), int(each->r), each->getColor()); 
			//DrawCircle(int(each->getX()), int(each->getY()), 40, olc::WHITE);
		}

		// ----- EFFECTS ----- //
		
		// Forces generated from AI, summed up for each shape
		for (auto& each : sharedPtrAI) {
			float px,py,magnitude,radius_of_influence;
			ForceType ftype;
			if (each->force(px, py, magnitude, radius_of_influence, ftype)) {
				for (auto& triangle : sharedPtrTriangles) { // TODO: this may get tricky when i include all objects, not just triangles. Counting same twice etc.
					switch (ftype) {
						case eMagnetic: {
							float dx = px - (triangle->centroidX() + triangle->getX());
							float dy = py - (triangle->centroidY() + triangle->getY());
							float distance = sqrt(dx * dx + dy * dy);
							if (distance < radius_of_influence) {
								float force = magnitude * (radius_of_influence - distance) / radius_of_influence;
								triangle->addForce(force, atan2(dy, dx));
								each->getSelf()->addForce(-force, atan2(dy, dx));
							}
							break;
						}
					}
				}
			}
		}

		// apply summed up force on each shape
		for (auto& triangle : sharedPtrTriangles) { triangle->applyForce(); }
		for (auto& circle : sharedPtrCircles) { circle->applyForce(); }

		// Remove dead things

		// Shapes
		for (auto& each : sharedPtrTriangles) {
			if (each->getKillFlag()) {
				each->getAI()->destroy(); // either check that an AI exists, or make all shapes have a dummy AI by default.
				sharedPtrTriangles.erase(std::remove(sharedPtrTriangles.begin(), sharedPtrTriangles.end(), each), sharedPtrTriangles.end());
				break; // bad. should be able to remove many. see https://stackoverflow.com/questions/3487717/erasing-multiple-objects-from-a-stdvector
			}
		}
		for (auto& each : sharedPtrCircles) {
			if (each->getKillFlag()) {
				each->getAI()->destroy();
				sharedPtrCircles.erase(std::remove(sharedPtrCircles.begin(), sharedPtrCircles.end(), each), sharedPtrCircles.end());
				break;
			}
		}

		// AIs
		for (auto& each : sharedPtrAI) {
			if (each->getDestroyFlag()) {
				sharedPtrAI.erase(std::remove(sharedPtrAI.begin(), sharedPtrAI.end(), each), sharedPtrAI.end());
				break;
			}
		}

		// ----- MOVEMENT ----- //

		for (auto& shape : sharedPtrCircles) { shape->updatePosition();}
		for (auto& shape : sharedPtrTriangles) { shape->updatePosition(); }

		sharedPtrLines.clear();

		return true;
	}

	void DrawTriangle(sTriangle t, olc::Pixel color) { FillTriangle(int(t.x1), int(t.y1), int(t.x2), int(t.y2), int(t.x3), int(t.y3), color); }


#include "collisions.h"



	void checkCollisions() {
		for (auto& line : sharedPtrLines) {
			float minPx, minPy, px, py;
			float minDistance = 1000000.0f;
			bool collision = false;
			shared_ptr<Shape> hitObject;

			for (auto& triangle : sharedPtrTriangles) {
				if (triangle == user_controlled_shape) continue; // don't collide with self
				if (LineTriangleCollision(line->getStruct(), triangle->worldCoordinates(), px, py)) {
					float distance = sqrt((line->getX() - px) * (line->getX() - px) + (line->getY() - py) * (line->getY() - py));
					if (distance < minDistance) {
						collision = true;
						minDistance = distance;
						minPx = px;
						minPy = py;
						hitObject = triangle;
					}
				}
			}

			for (auto& rectangle : sharedPtrRectangles) {
				if (rectangle == user_controlled_shape) continue; // don't collide with self
				if (LineRectCollision(line->getStruct(), rectangle->getStruct(), px, py)) {
					float distance = sqrt((line->getX() - px) * (line->getX() - px) + (line->getY() - py) * (line->getY() - py));
					if (distance < minDistance) {
						collision = true;
						minDistance = distance;
						minPx = px;
						minPy = py;
						hitObject = rectangle;
					}
				}
			}

			for (auto& circle : sharedPtrCircles) {
				if (circle == user_controlled_shape) continue; // don't collide with self
				if (LineCircleCollision(line->getStruct(), sCircle{ circle->getX(), circle->getY(), circle->r }, px, py)) {
					float distance = sqrt((line->getX() - px) * (line->getX() - px) + (line->getY() - py) * (line->getY() - py));
					if (distance < minDistance) {
						collision = true;
						minDistance = distance;
						minPx = px;
						minPy = py;
						hitObject = circle;
					}
				}
			}

			if (collision) {
				DrawCircle(minPx, minPy, 10.0f, olc::YELLOW);
				line->x2 = minPx;
				line->y2 = minPy;
				line->trigger(line, hitObject);
			}
			else {
				line->x2 = line->getX() + 1000.0f * cos(line->getAngle());
				line->y2 = line->getY() + 1000.0f * sin(line->getAngle());
			}
		}
	}






};



int main() {
	World world;
	if (world.Construct(500, 400, 1, 1))
		world.Start();
	return 0;
}