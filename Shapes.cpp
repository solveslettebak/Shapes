// TODO: 
// - I stedet for å lage laser-linje for hver frame, og slette den igjen, kan jeg lage en AI subclass. .. men får samme problem, med å slette den igjen.
// - User control burde også være en AI subclass. Dermed trenger AI user input, og jeg kan bruke det andre steder også.
// - Types of forces. How to implement? force() to return enum, and then apply force in World class? Point source, field (gravity), friction (air resistance), .. more?
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
// - Need to figure out how objects should be talking to each other. And structure everything neatly from the start. Probably all objects just talk to World, somehow.
// - Currently lines are created and killed every frame. Line is equivalent of object in my current setup.
// - draw up some oop diagrams, and figure out how to structure everything.

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
class triggerAim : public ITrigger {
public:
	void trigger(shared_ptr<Shape> owner, shared_ptr<Shape> victim) override {
		
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
	// should explode when it collides with something
	// pushes all objects around it away when it hits something (apply small damage to them as well, depending on distance)

	void update(float tElapsedTime) override {
		timePassed += tElapsedTime;
		if ((timePassed > 1.0f) && (!magnetic)) { // called once when initial delay is over
			magnetic = true;
			self->setColor(olc::YELLOW);
		} else if ((timePassed > 0.5f) && (timePassed < 1.5f)) {
			self->addForce(-0.01f, self->getMotionAngle()); // slow down
		}
		else if (timePassed > 2.0f) {
			//self->setKillFlag(); // replace with explosion
			explode = true;
		}
	}

	bool force(float &px, float &py, float &magnitude, float &radius_of_influence, ForceType &ftype) override {
		if (hasExploded) {
			self->setKillFlag();
			return false;
		}

		if (explode && !hasExploded) {
			hasExploded = true;
			self->setColor(olc::RED);
			// apply force on all objects within radius of influence
			px = self->getX();
			py = self->getY();
			magnitude = -10.0f;
			radius_of_influence = 200.0f;
			ftype = eExplosion;
			return true;
		}

		if (!magnetic) return false;
		px = self->getX();
		py = self->getY();
		magnitude = 0.02f;
		radius_of_influence = 200.0f;
		ftype = eMagnetic;
		return true;
	}
protected:
	float timePassed = 0.0f;
	bool magnetic = false;
	bool hasExploded = false;
	bool explode = false;
};


class AI_follow_user : public AI {
public:
	AI_follow_user(shared_ptr<Shape> self_, shared_ptr<Shape> external_) { self = self_;  external = external_; }
	void update(float tElapsedTime) override {
		float dx = external->getX() - self->getX();
		float dy = external->getY() - self->getY();
		float angle = atan2(dy, dx);
		self->setAngle(angle);
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

	std::shared_ptr<Shape> createPlayer() {
		shared_ptr<Triangle> shape = make_shared<Triangle>(sTriangle{ 10, 8, 10, 22, 30, 15 }, 50, 50, olc::RED);
		sharedPtrTriangles.push_back(shape);
		//user_controlled_shape = shape;
		shape->setColor(olc::GREEN);
		return shape;
	}

	std::shared_ptr<Shape> createMagbomb() {
		shared_ptr<Circle> circle = make_shared<Circle>(user_controlled_shape->getX(), user_controlled_shape->getY(), 5.0f, olc::GREEN);
		sharedPtrCircles.push_back(circle);
		circle->addForce(4.0f, user_controlled_shape->getAngle());
		circle->setMass(5.0f);

		shared_ptr<IHull> hull = make_shared<playerHull>(circle);
		sharedPtrHulls.push_back(hull);
		circle->setHull(hull);
		circle->getHull()->setArmor(1.0f);
		//circle->setMass(10.0f);
		circle->setCanBeDamaged(true);

		shared_ptr<AI> ai = make_shared<AI_magbomb>(circle, nullptr);
		sharedPtrAI.push_back(ai);
		circle->setAI(ai);
		return circle;
	}

	bool OnUserCreate() override {

		user_controlled_shape = createPlayer();

		for (int i = 0; i < 20; i++)
			addRandomEnemy();

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

		if (GetKey(olc::Key::B).bHeld) {
			shared_ptr<Line> laser;
			laser = make_shared<Line>(user_controlled_shape->getX(), user_controlled_shape->getY());
			laser->setAngle(user_controlled_shape->getAngle());
			laser->setColor(olc::RED);

			shared_ptr<ITrigger> trigger = make_shared<triggerLaser>();
			sharedPtrTriggers.push_back(trigger);
			laser->setTrigger(trigger);

			sharedPtrLines.push_back(laser);
		}

		if (GetKey(olc::Key::S).bHeld) { // this should be an object, and only have a shape when S is held. otherwise keep track of enemy using the appropriate AI.
			shared_ptr<Line> aim;
			aim = make_shared<Line>(user_controlled_shape->getX(), user_controlled_shape->getY());
			aim->setAngle(user_controlled_shape->getAngle());
			aim->setColor(olc::BLUE);

			shared_ptr<ITrigger> trigger = make_shared<triggerAim>();
			sharedPtrTriggers.push_back(trigger);
			aim->setTrigger(trigger);

			sharedPtrLines.push_back(aim);
		}

		if (GetKey(olc::Key::A).bPressed) createMagbomb();
		if (GetKey(olc::Key::LEFT).bHeld)  { user_controlled_shape->rotate(-0.003f); }
		if (GetKey(olc::Key::PGUP).bHeld) { user_controlled_shape->rotate(-0.0005f); }
		if (GetKey(olc::Key::PGDN).bHeld) { user_controlled_shape->rotate(0.0005f); }
		if (GetKey(olc::Key::RIGHT).bHeld) { user_controlled_shape->rotate( 0.003f); }
		if (GetKey(olc::Key::UP).bHeld) { user_controlled_shape->addForce(10.0f * fElapsedTime, user_controlled_shape->getAngle()); }

		for (auto& each : sharedPtrAI) { 
			if (each->getDestroyFlag()) {
				sharedPtrAI.erase(std::remove(sharedPtrAI.begin(), sharedPtrAI.end(), each), sharedPtrAI.end());
				break; // find a better way. see the other delete stuff thingy
			}
			else { each->update(fElapsedTime); }
		}

		// ----- COLLISION ----- //

		checkCollisions2(fElapsedTime);

		// ---- DRAW ----- //

		for (auto& each : sharedPtrTriangles)  { 
			DrawTriangle(each->worldCoordinates(), each->getColor()); 
			if (each->getHull()) { // TODO: add "display health" property to shape.
				float health = each->getHull()->getHealth();
				FillRect(int(each->getX()) - 15, int(each->getY()) - 15, int(30.0f * health), 5, olc::Pixel(255 * (1.0f - health), 255 * health, 0));
			}
		}
		for (auto& each : sharedPtrRectangles) { FillRect(int(each->getX()), int(each->getY()), int(each->w), int(each->h), each->getColor()); }
		for (auto& each : sharedPtrLines)      { DrawLine(int(each->getX()), int(each->getY()), int(each->getX2()), int(each->getY2()), each->getColor()); }
		for (auto& each : sharedPtrCircles)    { 
			FillCircle(int(each->getX()), int(each->getY()), int(each->r), each->getColor()); 
			DrawCircle(int(each->getX()), int(each->getY()), 200, olc::Pixel(50, 50, 155));
		}

		// ----- EFFECTS ----- //
		
		// Forces generated from AI, summed up for each shape
		for (auto& each : sharedPtrAI) {
			float px,py,magnitude,radius_of_influence;
			ForceType ftype;
			if (each->force(px, py, magnitude, radius_of_influence, ftype)) {
				for (auto& triangle : sharedPtrTriangles) { // TODO: this may get tricky when i include all objects, not just triangles. Counting same twice etc.
					if (each->getSelf() == triangle) continue; // don't apply force on self
					switch (ftype) {
						case eExplosion:
						case eMagnetic: 
							float dx = px - (triangle->centroidX() + triangle->getX());
							float dy = py - (triangle->centroidY() + triangle->getY());
							float distance = sqrt(dx * dx + dy * dy);
							if (distance < radius_of_influence) {
								float force = magnitude * (radius_of_influence - distance) / radius_of_influence;
								triangle->addForce(force, atan2(dy, dx));
								each->getSelf()->addForce(-force, atan2(dy, dx));
								if (ftype == eExplosion) triangle->damage(-force * 10.0f);
							}
							break;	
					}
				}
				for (auto& circle : sharedPtrCircles) {
					if (each->getSelf() == circle) continue;
					switch (ftype) {
					case eExplosion:
					case eMagnetic: {
							float dx = px - circle->getX();
							float dy = py - circle->getY();
							float distance = sqrt(dx * dx + dy * dy);
							if (distance < radius_of_influence) {
								float force = magnitude * (radius_of_influence - distance) / radius_of_influence;
								circle->addForce(force, atan2(dy, dx));
								each->getSelf()->addForce(-force, atan2(dy, dx));
								if (ftype == eExplosion) circle->damage(-force * 100.0f);
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
				break;}} // bad. should be able to remove many. see https://stackoverflow.com/questions/3487717/erasing-multiple-objects-from-a-stdvector
		for (auto& each : sharedPtrCircles) {
			if (each->getKillFlag()) {
				each->getAI()->destroy();
				sharedPtrCircles.erase(std::remove(sharedPtrCircles.begin(), sharedPtrCircles.end(), each), sharedPtrCircles.end());
				break;}}

		// AIs
		for (auto& each : sharedPtrAI) {
			if (each->getDestroyFlag()) {
				sharedPtrAI.erase(std::remove(sharedPtrAI.begin(), sharedPtrAI.end(), each), sharedPtrAI.end());
				break;}}

		// ----- MOVEMENT ----- //

		for (auto& shape : sharedPtrCircles) { shape->updatePosition();}
		for (auto& shape : sharedPtrTriangles) { shape->updatePosition(); }

		sharedPtrLines.clear();

		return true;
	}

	void DrawTriangle(sTriangle t, olc::Pixel color) { FillTriangle(int(t.x1), int(t.y1), int(t.x2), int(t.y2), int(t.x3), int(t.y3), color); }


#include "collisions.h"

	void checkCollisions2(float fElapsedTime) {
		vector<shared_ptr<Shape>> shapes;

		// add a check here to figure out if shape can collide at all / has any effect if it collides

		// lines first, because they need to be treated differently
		for (auto& line : sharedPtrLines) { shapes.push_back(line); }
		// then the rest
		for (auto& triangle : sharedPtrTriangles) { if (triangle->getCanCollide()) shapes.push_back(triangle); }
		for (auto& rectangle : sharedPtrRectangles) { if (rectangle->getCanCollide()) shapes.push_back(rectangle); }
		for (auto& circle : sharedPtrCircles) { if (circle->getCanCollide()) shapes.push_back(circle); }

		for (size_t i = 0; i < shapes.size(); i++) {

			float minPx, minPy, px, py;
			float minDistance = 1000000.0f;
			bool collision = false;
			shared_ptr<Shape> hitObject;
			
			std::shared_ptr<Line> line = std::dynamic_pointer_cast<Line>(shapes[i]);
			std::shared_ptr<Triangle> triangle = std::dynamic_pointer_cast<Triangle>(shapes[i]);
			std::shared_ptr<Rect> rectangle = std::dynamic_pointer_cast<Rect>(shapes[i]);
			std::shared_ptr<Circle> circle = std::dynamic_pointer_cast<Circle>(shapes[i]);

			for (size_t j = i + 1; j < shapes.size(); j++) {
				if (triangle) {
					if (std::shared_ptr<Triangle> triangle2 = std::dynamic_pointer_cast<Triangle>(shapes[j])) {
						// do triangle-triangle collision
					}
					else if (std::shared_ptr<Rect> rectangle = std::dynamic_pointer_cast<Rect>(shapes[j])) {
						// do triangle-rectangle collision
						if (TriangleRectCollision(triangle->worldCoordinates(), rectangle->getStruct(), px, py)) {
							cout << "collision" << endl;
							//triangle->addForce(-10.0f, triangle->getAngle());
							//triangle->setKineticEnergy(triangle->getKineticEnergy(), triangle->getDirection() + 3.14f);
							triangle->stepBack(fElapsedTime); // move out of the wall
							triangle->reverse(); // and bounce
							//triangle->setKillFlag();
							// need to resolve collision
							// then do dynamic collision stuff..
						}
					}
					else if (std::shared_ptr<Circle> circle = std::dynamic_pointer_cast<Circle>(shapes[j])) {
						// do triangle-circle collision
					}
					// skip lines here.
				}
				else if (rectangle) {
					if (std::shared_ptr<Triangle> triangle = std::dynamic_pointer_cast<Triangle>(shapes[j])) {
						// do triangle-rectangle collision
					}
					else if (std::shared_ptr<Rect> rectangle2 = std::dynamic_pointer_cast<Rect>(shapes[j])) {
						// do rectangle-rectangle collision
					}
					else if (std::shared_ptr<Circle> circle = std::dynamic_pointer_cast<Circle>(shapes[j])) {
						// do rectangle-circle collision
					}
				}
				else if (circle) {
					if (std::shared_ptr<Triangle> triangle = std::dynamic_pointer_cast<Triangle>(shapes[j])) {
						// do triangle-circle collision
					}
					else if (std::shared_ptr<Rect> rectangle = std::dynamic_pointer_cast<Rect>(shapes[j])) {
						// do rectangle-circle collision
					}
					else if (std::shared_ptr<Circle> circle2 = std::dynamic_pointer_cast<Circle>(shapes[j])) {
						// do circle-circle collision

					}
				}
				else if (line) {
					if (shapes[j] == user_controlled_shape) continue; // don't collide with user... for now. TODO: Figure out better way.. 
					if (std::shared_ptr<Triangle> triangle = std::dynamic_pointer_cast<Triangle>(shapes[j])) {
						// do triangle-line collision
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
					else if (std::shared_ptr<Rect> rectangle = std::dynamic_pointer_cast<Rect>(shapes[j])) {
						// do rectangle-line collision
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
					else if (std::shared_ptr<Circle> circle = std::dynamic_pointer_cast<Circle>(shapes[j])) {
						// do circle-line collision
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
					else if (std::shared_ptr<Line> line2 = std::dynamic_pointer_cast<Line>(shapes[j])) {
						// do line-line collision .. ? i think lines are never physical objects, so.. can probably ignore this.
					}
				}
			}

			if (collision) {
				DrawCircle(minPx, minPy, 10.0f, olc::YELLOW);
				line->x2 = minPx;
				line->y2 = minPy;
				line->trigger(line, hitObject);
			}
			else if (line) {
				line->x2 = line->getX() + 1000.0f * cos(line->getAngle());
				line->y2 = line->getY() + 1000.0f * sin(line->getAngle());
			}

		}
	}


	// static collision detection. only true/false, does not move objects. yet.
	void checkCollisions() {

		// ----- Circles ----- //
		for (auto& circle : sharedPtrCircles) {

		}

		// ----- Lines ----- //
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
	if (world.Construct(800, 600, 1, 1))
		world.Start();
	return 0;
}