#pragma once
#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <memory>

#include "Hull.h"
#include "AI.h"
#include "Shape.h"

#include "constants.h"

/*
Effects:
- gravity / motion. Different types: force relative to point, relative to horizontal/vertical, and probably more..
- friction (air resistance)
- damage (later: heat, piercing, explosion, etc)

*/

//enum ForceType { eGravity, eMagnetic, eFriction };

using namespace std;

#include "structs.h"

class AI;
class Shape;


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

class evilTriangleHull : public IHull {
public:
	evilTriangleHull(shared_ptr<Shape> owner_) { maxArmor = 100.0f; currentArmor = 100.0f; owner = owner_; }
	void damage(float amount) override {
		currentArmor -= amount;
		if (currentArmor < 0.0f) {
			owner->setKillFlag();
			cout << "Enemy killed" << endl;
		}
	}

};




// replace with something later.
struct sEdge {
	float sx, sy; // start
	float ex, ey; // end
};


class World : public olc::PixelGameEngine, std::enable_shared_from_this<World> {

private:
	vector<shared_ptr<Triangle>> sharedPtrTriangles;
	vector<shared_ptr<Rect>> sharedPtrRectangles;
	vector<shared_ptr<Line>> sharedPtrLines;
	vector<shared_ptr<Circle>> sharedPtrCircles;
	
	shared_ptr<Shape> user_controlled_shape;
	shared_ptr<Line> user_controlled_laser; // kind of a hack..
	shared_ptr<Line> user_controlled_aim; // kind of a hack..
	shared_ptr<Circle> user_controlled_ninjarope; // kind of a hack..

	float magballForce = 0.0f;
	bool magballCharging = false;

	vector<shared_ptr<AI>> sharedPtrAI;
	vector<shared_ptr<IHull>> sharedPtrHulls;

	vector<sEdge> vecEdges;
	vector<tuple<float, float, float>> vecVisibilityPolygonPoints;

public:
	World() { sAppName = "World"; }

	void createVecEdges() {
		vecEdges.clear();
		for (auto& rectangle : sharedPtrRectangles) {
			vecEdges.push_back({ rectangle->getX(), rectangle->getY(), rectangle->getX() + rectangle->w, rectangle->getY() });
			vecEdges.push_back({ rectangle->getX() + rectangle->w, rectangle->getY(), rectangle->getX() + rectangle->w, rectangle->getY() + rectangle->h });
			vecEdges.push_back({ rectangle->getX() + rectangle->w, rectangle->getY() + rectangle->h, rectangle->getX(), rectangle->getY() + rectangle->h });
			vecEdges.push_back({ rectangle->getX(), rectangle->getY() + rectangle->h, rectangle->getX(), rectangle->getY() });
		}
	}

	void CalculateVisibilityPolygon(float ox, float oy, float radius) {
		vecVisibilityPolygonPoints.clear();

		// for each edge in PolyMap
		for (auto& e1 : vecEdges) {
			for (int i = 0; i < 2; i++) {
				float rdx, rdy;
				rdx = (i == 0 ? e1.sx : e1.ex) - ox;
				rdy = (i == 0 ? e1.sy : e1.ey) - oy;

				float base_ang = atan2f(rdy, rdx);

				float ang = 0;

				// for each point, cast 3 rays, 1 directly at point, and 1 a little bit on either side
				for (int j = 0; j < 3; j++) {
					if (j == 0) ang = base_ang - 0.0001f;
					if (j == 1) ang = base_ang;
					if (j == 2) ang = base_ang + 0.0001f;

					// create ray along angle for required distance
					rdx = radius * cosf(ang);
					rdy = radius * sinf(ang);

					float min_t1 = INFINITY;
					float min_px = 0, min_py = 0, min_ang = 0;
					bool bValid = false;

					// Check for ray intersections with all edges
					for (auto& e2 : vecEdges) {
						// create line segment vector
						float sdx = e2.ex - e2.sx;
						float sdy = e2.ey - e2.sy;

						if (fabs(sdx - rdx) > 0.0f && fabs(sdy - rdy) > 0.0f) {
							// t2 is normalised distance from line segment start to line segment end of intersect po
							float t2 = (rdx * (e2.sy - oy) + (rdy * (ox - e2.sx))) / (sdx * rdy - sdy * rdx); // missing something here... 
							// t1 is normalised distance from source along ray to ray length of intersect point
							float t1 = (e2.sx + sdx * t2 - ox) / rdx;

							// if intersect point exists along ray, and along line
							// segment then intersect point is valid
							if (t1 > 0 && t2 >= 0 && t2 <= 1.0f) {
								// Check if this intersect point is closest to source, and if it is, then store this point and reject others
								if (t1 < min_t1) {
									min_t1 = t1;
									min_px = ox + rdx * t1;
									min_py = oy + rdy * t1;
									min_ang = atan2f(min_py - oy, min_px - ox);
									bValid = true;
								}
							}
						}
					}

					// add intersection point to visibility polygon perimeter
					if (bValid)
						vecVisibilityPolygonPoints.push_back({ min_ang, min_px, min_py });
				}
			}
		}

		// Sort perimeter points by angle from source. This will allow us to draw a triangle fan
		sort(
			vecVisibilityPolygonPoints.begin(),
			vecVisibilityPolygonPoints.end(),
			[&](const tuple<float, float, float>& t1, const tuple<float, float, float>& t2)
			{
				return get<0>(t1) < get<0>(t2);
			});
	}

	void shadowCastDraw() {
		auto it = unique(
			vecVisibilityPolygonPoints.begin(),
			vecVisibilityPolygonPoints.end(),
			[&](const tuple<float, float, float>& t1, const tuple<float, float, float>& t2) {
				return fabs(get<1>(t1) - get<1>(t2)) < 0.1f && fabs(get<2>(t1) - get<2>(t2)) < 0.1f;
			});

		vecVisibilityPolygonPoints.resize(distance(vecVisibilityPolygonPoints.begin(), it));

		//int nRaysCast2 = vecVisibilityPolygonPoints.size();
		//DrawString(4, 4, "Rays Cast: " + to_string(nRaysCast) + "Rays Drawn: " + to_string(nRaysCast2));

		if (vecVisibilityPolygonPoints.size() > 1) {
			// draw triangle fan
			for (int i = 0; i < vecVisibilityPolygonPoints.size() - 1; i++) {
				FillTriangle(
					user_controlled_shape->getX(),
					user_controlled_shape->getY(),
					get<1>(vecVisibilityPolygonPoints[i]),
					get<2>(vecVisibilityPolygonPoints[i]),

					get<1>(vecVisibilityPolygonPoints[i + 1]),
					get<2>(vecVisibilityPolygonPoints[i + 1]), olc::DARK_BLUE);
			}

			FillTriangle(
				user_controlled_shape->getX(),
				user_controlled_shape->getY(),
				get<1>(vecVisibilityPolygonPoints[vecVisibilityPolygonPoints.size() - 1]),
				get<2>(vecVisibilityPolygonPoints[vecVisibilityPolygonPoints.size() - 1]),
				get<1>(vecVisibilityPolygonPoints[0]),
				get<2>(vecVisibilityPolygonPoints[0]), olc::DARK_BLUE);
		}
	}


	void addRandomEnemy() {
		shared_ptr<Triangle> shape = make_shared<Triangle>(sTriangle{ 10, 8, 10, 22, 30, 15 }, rand() % ScreenWidth(), rand() % ScreenHeight(), olc::RED);
		sharedPtrTriangles.push_back(shape);

		shared_ptr<AI> enemy_AI = make_shared<AI_follow_user>(shape, user_controlled_shape);
		sharedPtrAI.push_back(enemy_AI);
		//shape->setAI(enemy_AI); // not used yet

		shared_ptr<IHull> hull = make_shared<evilTriangleHull>(shape);
		sharedPtrHulls.push_back(hull);
		shape->setHull(hull);
		shape->setCanBeDamaged(true);

	}

	std::shared_ptr<Shape> createPlayer() {
		shared_ptr<Triangle> shape = make_shared<Triangle>(sTriangle{ 10, 8, 10, 22, 30, 15 }, 50, 50, olc::RED);
		sharedPtrTriangles.push_back(shape);
		shape->setMass(SHIP_MASS);
		shape->setColor(olc::GREEN);

		shared_ptr<IHull> hull = make_shared<playerHull>(shape);
		sharedPtrHulls.push_back(hull);
		shape->setHull(hull);
		shape->setCanBeDamaged(true);
		hull->setRepairRate(2.0f);
		return shape;
	}

	std::shared_ptr<Shape> createMagbomb(float force) {
		shared_ptr<Circle> circle = make_shared<Circle>(user_controlled_shape->getX(), user_controlled_shape->getY(), 5.0f, olc::GREEN);
		sharedPtrCircles.push_back(circle);
		if (force > 1.0f) force = 1.0f;
		circle->addForce(1.5f + force * 5, user_controlled_shape->getAngle());
		circle->setVelocity(user_controlled_shape->getVelocityX(), user_controlled_shape->getVelocityY());

		shared_ptr<IHull> hull = make_shared<playerHull>(circle);
		sharedPtrHulls.push_back(hull);
		circle->setHull(hull);
		circle->getHull()->setArmor(1.0f);

		shared_ptr<AI> ai = make_shared<AI_magbomb>(circle, nullptr);
		sharedPtrAI.push_back(ai);
		circle->setAI(ai);
		ai->setup();
		return circle;
	}

	shared_ptr<Circle> createNinjaRope() {
		shared_ptr<Circle> circle = make_shared<Circle>(user_controlled_shape->getX(), user_controlled_shape->getY(), 5.0f, olc::WHITE);
		sharedPtrCircles.push_back(circle);
		circle->addForce(4.0f, user_controlled_shape->getAngle());
		circle->setVelocity(user_controlled_shape->getVelocityX(), user_controlled_shape->getVelocityY());

		shared_ptr<AI> ai = make_shared<AI_ninjarope>(circle, user_controlled_shape);
		sharedPtrAI.push_back(ai);
		ai->setWorld(this);
		ai->setup();
		circle->setAI(ai);
		return circle;
	}

	std::shared_ptr<Shape> createBall() {
		shared_ptr<Circle> circle = make_shared<Circle>(rand() % ScreenWidth(), rand() % ScreenHeight(), rand() % 10 + 5.0f, olc::GREY);
		sharedPtrCircles.push_back(circle);
		return circle;
	}

	std::shared_ptr<Line> createLaser() {
		shared_ptr<Line> laser;
		laser = make_shared<Line>(user_controlled_shape->getX(), user_controlled_shape->getY());
		laser->setAngle(user_controlled_shape->getAngle());
		laser->setColor(olc::RED);
		sharedPtrLines.push_back(laser);

		shared_ptr<AI_laser> ai = make_shared<AI_laser>(laser, user_controlled_shape);
		sharedPtrAI.push_back(ai);
		laser->setAI(ai);

		return laser;
	}

	std::shared_ptr<Line> createAim() {
		shared_ptr<Line> aim;
		aim = make_shared<Line>(user_controlled_shape->getX(), user_controlled_shape->getY());
		aim->setAngle(user_controlled_shape->getAngle());
		aim->setColor(olc::BLUE);
		sharedPtrLines.push_back(aim);

		shared_ptr<AI_aim> ai = make_shared<AI_aim>(aim, user_controlled_shape);
		//ai->setWorld(std::make_shared<World>(weak_from_this()));
		World *world = this;
		
		//ai->setWorld(shared_from_this()); // TODO: figure out this shit and work with proper shared_ptrs or whatever. For now - this somehow compiles.
		ai->setWorld(world);
		sharedPtrAI.push_back(ai);
		aim->setAI(ai);

		return aim;
	}

	bool OnUserCreate() override {

		user_controlled_shape = createPlayer();

		for (int i = 0; i < 20; i++) addRandomEnemy();
		for (int i = 0; i < 20; i++) createBall();

		shared_ptr<Rect> leftWall = make_shared<Rect>(10,10, 5, ScreenHeight()-20, olc::WHITE);
		shared_ptr<Rect> rightWall = make_shared<Rect>(ScreenWidth()-15, 10, 5, ScreenHeight()-20, olc::WHITE);
		shared_ptr<Rect> topWall = make_shared<Rect>(10, 10, ScreenWidth()-20, 5, olc::WHITE);
		shared_ptr<Rect> bottomWall = make_shared<Rect>(10, ScreenHeight()-15, ScreenWidth()-20, 5, olc::WHITE);
		sharedPtrRectangles.push_back(leftWall);
		sharedPtrRectangles.push_back(rightWall);
		sharedPtrRectangles.push_back(topWall);
		sharedPtrRectangles.push_back(bottomWall);

		shared_ptr<Rect> randomWall = make_shared<Rect>(200, 100, 30, 40, olc::WHITE); sharedPtrRectangles.push_back(randomWall);
		shared_ptr<Rect> randomWall2 = make_shared<Rect>(300, 200, 30, 40, olc::WHITE); sharedPtrRectangles.push_back(randomWall2);
		shared_ptr<Rect> randomWall3 = make_shared<Rect>(400, 300, 30, 40, olc::WHITE); sharedPtrRectangles.push_back(randomWall3);
		shared_ptr<Rect> randomWall4 = make_shared<Rect>(500, 400, 30, 40, olc::WHITE); sharedPtrRectangles.push_back(randomWall4);
		randomWall->setColor(olc::BLUE);
		randomWall2->setColor(olc::BLUE);
		randomWall3->setColor(olc::BLUE);
		randomWall4->setColor(olc::BLUE);


		createVecEdges();

		return true;
	}



	bool OnUserUpdate(float fElapsedTime) override {
		Clear(olc::BLACK);

		srand(10);

		for (int i = 0; i < 200; i++) {
			int x = int(rand() % ScreenWidth());
			int y = int(rand() % ScreenHeight());
			olc::Pixel p = olc::Pixel(rand() % 255, rand() % 255, rand() % 255);
			Draw(x, y, p);
			Draw(x-1, y, p);
			Draw(x+1, y, p);
			Draw(x, y-1, p);
			Draw(x, y+1, p);
		}

		CalculateVisibilityPolygon(user_controlled_shape->getX(), user_controlled_shape->getY(), 200.0f);

		shadowCastDraw();


		// ----- BEHAVIOUR ----- //

		// TODO: move user controls to AI's

		if (GetKey(olc::Key::ESCAPE).bPressed) return false;

		if (GetKey(olc::Key::B).bPressed) user_controlled_laser = createLaser();
		if (GetKey(olc::Key::B).bReleased) user_controlled_laser->setKillFlag();

		if (GetKey(olc::Key::S).bPressed) {
			if (user_controlled_aim) user_controlled_aim->setKillFlag();
			user_controlled_aim = createAim();
		}
		if (GetKey(olc::Key::S).bReleased) dynamic_cast<AI_aim*>(user_controlled_aim->getAI().get())->key_released();

		if (GetKey(olc::Key::D).bPressed) {
			if (user_controlled_ninjarope != nullptr) {
				user_controlled_ninjarope->setKillFlag();
				user_controlled_ninjarope = nullptr;
			}
			else
				user_controlled_ninjarope = createNinjaRope();
		}

		//dynamic_cast<Circle*>(self.get())->setStatic(true);
		// TODO : decide on auto-release when max force achieved, or hold there until released.
		if (GetKey(olc::Key::A).bPressed) { magballForce = 0.0f; magballCharging = true; }
		if ((GetKey(olc::Key::A).bHeld) && (magballCharging == true)) {
			magballForce += fElapsedTime; 
			FillRect(int(user_controlled_shape->getX()) - 15, int(user_controlled_shape->getY()) + 15, int(30.0f * magballForce), 5, olc::Pixel(255 * (1.0f - magballForce), 255 * magballForce, 0));
		}
		if (((GetKey(olc::Key::A).bReleased) || (magballForce > 1.0f)) && (magballCharging == true)) {
			createMagbomb(magballForce); 
			magballCharging = false; 
			magballForce = 0.0f; 
		}
			
			


		if (GetKey(olc::Key::LEFT).bHeld)  { user_controlled_shape->rotate(- SHIP_ROTATE_SPEED_FAST * fElapsedTime); }
		if (GetKey(olc::Key::PGUP).bHeld)  { user_controlled_shape->rotate(- SHIP_ROTATE_SPEED_SLOW * fElapsedTime); }
		if (GetKey(olc::Key::PGDN).bHeld)  { user_controlled_shape->rotate(  SHIP_ROTATE_SPEED_SLOW * fElapsedTime); }
		if (GetKey(olc::Key::RIGHT).bHeld) { user_controlled_shape->rotate(  SHIP_ROTATE_SPEED_FAST * fElapsedTime); }
		if (GetKey(olc::Key::UP).bHeld)    { user_controlled_shape->addForce(SHIP_THRUST * fElapsedTime, user_controlled_shape->getAngle()); }

		// ----- AI and Hull updates ----- //

		for (auto& each : sharedPtrAI) { 
			if (each->getDestroyFlag()) {
				sharedPtrAI.erase(std::remove(sharedPtrAI.begin(), sharedPtrAI.end(), each), sharedPtrAI.end());
				break; // find a better way. see the other delete stuff thingy
			}
			else { each->update(fElapsedTime); }
		}
		for (auto& each : sharedPtrHulls) {
			if (each->getDestroyFlag()) {
				sharedPtrHulls.erase(std::remove(sharedPtrHulls.begin(), sharedPtrHulls.end(), each), sharedPtrHulls.end());
				break; // find a better way. see the other delete stuff thingy
			}
			else { each->update(fElapsedTime); }
		}

		// ----- COLLISION ----- //

		checkCollisions2(fElapsedTime);


		// TODO: Expand to all shape types (can be generalized to just Shape class).
		// TODO: Later on: remove this method, and implement some sort of double buffering or other magic to display partially hidden items. Just paint over stuff basically.
		// check center point on shapes, if inside visible area
		for (auto& triangle : sharedPtrTriangles) { // all this is in Shape, can concat that, but lets try this first
			float x = triangle->getX();
			float y = triangle->getY();
			bool inShadow = true;
			sTriangle tri;
			if (vecVisibilityPolygonPoints.size() > 1) {
				for (int i = 0; i < vecVisibilityPolygonPoints.size() - 1; i++) {
					 tri = sTriangle{ user_controlled_shape->getX(), user_controlled_shape->getY(),
											get<1>(vecVisibilityPolygonPoints[i]), get<2>(vecVisibilityPolygonPoints[i]),
											get<1>(vecVisibilityPolygonPoints[i + 1]), get<2>(vecVisibilityPolygonPoints[i + 1]) };
					if (PointTriangleCollision(x, y, tri)) {
						inShadow = false;
						break;
					}
				}
				if (inShadow)
					tri = sTriangle{ user_controlled_shape->getX(), user_controlled_shape->getY(),
																get<1>(vecVisibilityPolygonPoints[vecVisibilityPolygonPoints.size() - 1]), get<2>(vecVisibilityPolygonPoints[vecVisibilityPolygonPoints.size() - 1]),
																get<1>(vecVisibilityPolygonPoints[0]), get<2>(vecVisibilityPolygonPoints[0]) };
					if (PointTriangleCollision(x, y, tri))
						inShadow = false;
			}
			triangle->setIsInShadow(inShadow);
		}


		// ---- DRAW ----- //

		for (auto& each : sharedPtrTriangles)  { 
			if (each->getIsInShadow()) continue; 
			if (each->getFilled()) 
				DrawTriangle(each->worldCoordinates(), each->getColor()); 
			else {
				sTriangle shape = each->worldCoordinates();
				DrawLine(int(shape.x1), int(shape.y1), int(shape.x2), int(shape.y2), each->getColor());
				DrawLine(int(shape.x2), int(shape.y2), int(shape.x3), int(shape.y3), each->getColor());
				DrawLine(int(shape.x3), int(shape.y3), int(shape.x1), int(shape.y1), each->getColor());
			}
			if (each->getHull()) { // TODO: add "display health" property to shape.
				float health = each->getHull()->getHealth();
				FillRect(int(each->getX()) - 15, int(each->getY()) - 15, int(30.0f * health), 5, olc::Pixel(255 * (1.0f - health), 255 * health, 0));
			}
		}
		for (auto& each : sharedPtrRectangles) { 
			if (each->getFilled()) 
				FillRect(int(each->getX()), int(each->getY()), int(each->w), int(each->h), each->getColor()); 
			else 
				DrawRect(int(each->getX()), int(each->getY()), int(each->w), int(each->h), each->getColor());
		}
		for (auto& each : sharedPtrLines) { 
			if (each->getVisible())
				DrawLine(int(each->getX()), int(each->getY()), int(each->getX2()), int(each->getY2()), each->getColor()); 
		}
		for (auto& each : sharedPtrCircles) { 
			if (each->getVisible()) {
				if (each->getFilled())
					FillCircle(int(each->getX()), int(each->getY()), int(each->r), each->getColor());
				else
					DrawCircle(int(each->getX()), int(each->getY()), int(each->r), each->getColor());

				//DrawCircle(int(each->getX()), int(each->getY()), 200, olc::Pixel(50, 50, 155));
			}
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

		// ----- CLEANUP ----- //

		// TODO: merge this stuff with the collision detection creation of one vector. Reduces number of times we have to loop through everything. 
		// 
		// Shapes
		for (auto& each : sharedPtrTriangles) {
			if (each->getKillFlag()) {
				if (each->getAI())
					each->getAI()->destroy(); // either check that an AI exists, or make all shapes have a dummy AI by default.
				sharedPtrTriangles.erase(std::remove(sharedPtrTriangles.begin(), sharedPtrTriangles.end(), each), sharedPtrTriangles.end());
				break;}} // bad. should be able to remove many. see https://stackoverflow.com/questions/3487717/erasing-multiple-objects-from-a-stdvector
		for (auto& each : sharedPtrCircles) {
			if (each->getKillFlag()) {
				if (each->getAI())
					each->getAI()->destroy();
				sharedPtrCircles.erase(std::remove(sharedPtrCircles.begin(), sharedPtrCircles.end(), each), sharedPtrCircles.end());
				break;}}
		for (auto& each : sharedPtrLines) {
			if (each->getKillFlag()) {
				if (each->getAI())
					each->getAI()->destroy();
				sharedPtrLines.erase(std::remove(sharedPtrLines.begin(), sharedPtrLines.end(), each), sharedPtrLines.end());
				break;}}

		// AIs
		for (auto& each : sharedPtrAI) {
			if (each->getDestroyFlag()) {
				sharedPtrAI.erase(std::remove(sharedPtrAI.begin(), sharedPtrAI.end(), each), sharedPtrAI.end());
				break;}}

		// ----- MOVEMENT ----- //

		for (auto& shape : sharedPtrCircles) { shape->updatePosition();}
		for (auto& shape : sharedPtrTriangles) { shape->updatePosition(); }

		return true;
	}

	void DrawTriangle(sTriangle t, olc::Pixel color) { FillTriangle(int(t.x1), int(t.y1), int(t.x2), int(t.y2), int(t.x3), int(t.y3), color); }


#include "collisions.h"



	// treat dynamic collisions. Will work perfectly for circles, and super good enough for the rest.
	// does not work for large objects, because the position is too different from point of collision 
	void dc(Shape* s1, Shape* s2) {
		float mass1 = s1->getMass();
		float mass2 = s2->getMass();

		float distance = sqrtf((s1->getX() - s2->getX()) * (s1->getX() - s2->getX()) + (s1->getY() - s2->getY()) * (s1->getY() - s2->getY()));

		// Normal
		float nx = (s2->getX() - s1->getX()) / distance;
		float ny = (s2->getY() - s1->getY()) / distance;

		// Tangent
		float tx = -ny;
		float ty = nx;

		// Dot Product Tangent
		float dpTan1 = s1->getVelocityX() * tx + s1->getVelocityY() * ty;
		float dpTan2 = s2->getVelocityX() * tx + s2->getVelocityY() * ty;

		// Dot Product Normal
		float dpNorm1 = s1->getVelocityX() * nx + s1->getVelocityY() * ny;
		float dpNorm2 = s2->getVelocityX() * nx + s2->getVelocityY() * ny;

		// Conservation of momentum in 1D
		float m1 = (dpNorm1 * (s1->getMass() - s2->getMass()) + 2.0f * s2->getMass() * dpNorm2) / (s1->getMass() + s2->getMass());
		float m2 = (dpNorm2 * (s2->getMass() - s1->getMass()) + 2.0f * s1->getMass() * dpNorm1) / (s1->getMass() + s2->getMass());

		// Update ball velocities
		s1->setVelocity(tx * dpTan1 + nx * m1, ty * dpTan1 + ny * m1);
		s2->setVelocity(tx * dpTan2 + nx * m2, ty * dpTan2 + ny * m2);

	}

	void checkCollisions2(float fElapsedTime) {
		vector<shared_ptr<Shape>> shapes;

		// Collect all shapes into a single vector. Sorted, to simplify collision detection.
		// lines first, because they need to be treated differently
		for (auto& line : sharedPtrLines) { if (line->getCanCollide()) shapes.push_back(line); }
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
						if (TriangleTriangleCollision(triangle->worldCoordinates(), triangle2->worldCoordinates())) {
							triangle->stepBack(fElapsedTime); // move one timestep back
							triangle2->stepBack(fElapsedTime); // move one timestep back

							dc(triangle.get(), triangle2.get());

							if (triangle->getAI()) {
								triangle->getAI()->trigger(triangle2, fElapsedTime);
							}
							if (triangle2->getAI()) {
								triangle2->getAI()->trigger(triangle, fElapsedTime);
							}
						}
					}
					else if (std::shared_ptr<Rect> rectangle = std::dynamic_pointer_cast<Rect>(shapes[j])) {
						// do triangle-rectangle collision
						if (TriangleRectCollision(triangle->worldCoordinates(), rectangle->getStruct(), px, py)) {
							triangle->stepBack(fElapsedTime); // move out of the wall
							triangle->reverse(); // and bounce
						}
					}
					else if (std::shared_ptr<Circle> circle = std::dynamic_pointer_cast<Circle>(shapes[j])) {
						// do triangle-circle collision
						if (CircleTriangleCollision(circle->getStruct(), triangle->worldCoordinates())) {

							circle->stepBack(fElapsedTime); // move one timestep back
							triangle->stepBack(fElapsedTime); 

							dc(circle.get(), triangle.get());

							if (circle->getAI()) {
								circle->getAI()->trigger(triangle, fElapsedTime);
							}
						}
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
						if (CircleRectangleCollision(circle->getStruct(), rectangle->getStruct())) {
							// Need to resolve collision properly, check if objects are static, modify accordingly, and do dynamic effects also.
							circle->stepBack(fElapsedTime); // move out of the wall
							circle->reverse(); // and bounce

							// doesn't work because getX/Y of a rectangle is far away from the point of collision. Probably. Static resolving should detect that point.
							//dc(rectangle.get(), circle.get());
							// TODO: should be simple to get the position from CircleRectangleCollision() and use that instead of getX/Y


						}
					}
				}
				else if (circle) {
					if (std::shared_ptr<Circle> circle2 = std::dynamic_pointer_cast<Circle>(shapes[j])) {
						// do circle-circle collision
						if (CircleCircleCollision(circle->getStruct(), circle2->getStruct())) {
							// Static resolving of collision.
							float distance = sqrtf((circle->getX() - circle2->getX()) * (circle->getX() - circle2->getX()) + (circle->getY() - circle2->getY()) * (circle->getY() - circle2->getY()));
							float overlap = 0.5f * (distance - circle->r - circle2->r);

							circle->setX(circle->getX() - overlap * (circle->getX() - circle2->getX()) / distance);
							circle->setY(circle->getY() - overlap * (circle->getY() - circle2->getY()) / distance);

							circle2->setX(circle2->getX() + overlap * (circle->getX() - circle2->getX()) / distance);
							circle2->setY(circle2->getY() + overlap * (circle->getY() - circle2->getY()) / distance);

							// Dynamic resolving of collision.
							dc(circle.get(), circle2.get());
						}
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
				//DrawCircle(minPx, minPy, 10.0f, olc::YELLOW);
				line->x2 = minPx;
				line->y2 = minPy;
				if (line->getAI())
					line->getAI()->trigger(hitObject, fElapsedTime);
			}
			else if (line) {
				line->x2 = line->getX() + 1000.0f * cos(line->getAngle());
				line->y2 = line->getY() + 1000.0f * sin(line->getAngle());
			}

		}
	}

};


// AI AIM //

AI_aim::AI_aim(shared_ptr<Shape> self_, shared_ptr<Shape> external_) { self = self_; external = external_; }

void AI_aim::update(float tElapsedTime) {
	self->setAngle(external->getAngle());
	self->setX(external->getX());
	self->setY(external->getY());

	if (locked)
		if (locked_on_object->getKillFlag()) {
			locked = false; // not sure if this is reliable / stable.. 
			self->setKillFlag();
		}

	if (locked) {
		world->DrawRect(int(locked_on_object->getX()) - 15, int(locked_on_object->getY()) - 15, 30, 30, olc::Pixel(255, 255, 255));
		world->DrawRect(int(locked_on_object->getX()) - 14, int(locked_on_object->getY()) - 14, 28, 28, olc::Pixel(255, 255, 255));
	}
}

void AI_aim::trigger(shared_ptr<Shape> other_object, float fElapsedTime) { 
	// TODO: check if other_object is a valid target. If not, return.
	locked_on_object = other_object;
	locked = true;
};

void AI_aim::key_released() {
	self->setVisible(false);
	self->setCanCollide(false);
}


// AI LASER //

AI_laser::AI_laser(shared_ptr<Shape> self_, shared_ptr<Shape> external_) { self = self_; external = external_; }

void AI_laser::update(float tElapsedTime) {
	self->setAngle(external->getAngle());
	self->setX(external->getX());
	self->setY(external->getY());
}

void AI_laser::trigger(shared_ptr<Shape> other_object, float fElapsedTime) { // TODO: need fElapsedTime here also
	other_object->damage(LASER_DAMAGE * fElapsedTime);
	other_object->addForce(-10.0f * fElapsedTime, self->getAngle());
}

// AI NINJA ROPE //

AI_ninjarope::AI_ninjarope(shared_ptr<Shape> self_, shared_ptr<Shape> external_) { self = self_; external = external_; }

void AI_ninjarope::setup() {
	self->setMass(5.0f);
	self->setCanBeDamaged(true);
	self->setCanCollide(false);
	self->setColor(olc::WHITE);
}

// TODO: can AI lasers, ninja ropes and similar make use of singleton design pattern?
void AI_ninjarope::update(float tElapsedTime) {
	timePassed += tElapsedTime;

	if ((timePassed > 0.2f) && (timePassed < 0.25f)) self->setCanCollide(true); // TODO: need to check a flag instead here. This can get screwed in several edge cases.
	if (!locked)
		if (timePassed > 1.0f) self->setKillFlag(); // kill after 1 second if not locked on anything

	if (locked)
		if (locked_on_object->getKillFlag()) locked = false; // not sure if this is reliable / stable.. 

	if (locked) {
		float distance = sqrtf((locked_on_object->getX() - external->getX()) * (locked_on_object->getX() - external->getX()) + (locked_on_object->getY() - external->getY()) * (locked_on_object->getY() - external->getY()));
		float angle = atan2(locked_on_object->getY() - external->getY(), locked_on_object->getX() - external->getX());
		float force = distance * 0.3f * tElapsedTime;
		locked_on_object->addForce(-force, angle);
		external->addForce(force, angle);
		self->setVisible(false);
		self->setCanCollide(false);
		self->setStatic(true);
		world->DrawLine(int(locked_on_object->getX()), int(locked_on_object->getY()), int(external->getX()), int(external->getY()), olc::Pixel(255, 255, 255));
	}
}

void AI_ninjarope::trigger(shared_ptr<Shape> other_object, float fElapsedTime) { // TODO: need fElapsedTime here also
	locked_on_object = other_object;
	locked = true;
}

bool AI_ninjarope::force(float& px, float& py, float& magnitude, float& radius_of_influence, ForceType& ftype) {
	return false; // maybe this should be handled in update() .. 
}

// AI MAGBOMB //


AI_magbomb::AI_magbomb(shared_ptr<Shape> self_, shared_ptr<Shape> external_) { self = self_; external = external_; }

void AI_magbomb::setup() {
	self->setMass(5.0f);
	self->setCanBeDamaged(true);
	self->setCanCollide(false);
	self->setColor(olc::GREEN);
}

void AI_magbomb::update(float tElapsedTime) {
	timePassed += tElapsedTime;

	if ((timePassed > 0.2f) && (timePassed < 0.3f)) self->setCanCollide(true); // can collide after 0.2 seconds (to avoid colliding with player)
	else if ((timePassed > MB_TIME_TO_ARM) && (!magnetic)) { // called once when initial delay is over
		magnetic = true;
		self->setColor(olc::YELLOW);
	}

	if ((timePassed > 0.5f) && (timePassed < 1.5f)) self->addForce(-0.01f, self->getMotionAngle()); // slow down 
	else if ((timePassed > MB_LIFETIME) && (timePassed < MB_LIFETIME + 0.5f)) {
		explode = true;

		// Change shape to an expanding circle. A hack until i figure out a good way to draw from the AI class.
		self->setFilled(false);
		self->setStatic(true);
		self->setCanCollide(false);
		dynamic_cast<Circle*>(self.get())->setRadius(200.0f * (timePassed - MB_LIFETIME) * 2);
	}
	else if (timePassed > MB_LIFETIME + 0.5f) {
		self->setKillFlag();
		//this->setDestroyFlag(true); // TODO: check if this is necessary. How? Print out number of AI's every frame or something.
	}
}

bool AI_magbomb::force(float& px, float& py, float& magnitude, float& radius_of_influence, ForceType& ftype) {
	if (hasExploded) { return false; }

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

// AI follow user //

AI_follow_user::AI_follow_user(shared_ptr<Shape> self_, shared_ptr<Shape> external_) { self = self_;  external = external_; }
void AI_follow_user::update(float tElapsedTime) {
	float dx = external->getX() - self->getX();
	float dy = external->getY() - self->getY();
	float angle = atan2(dy, dx);
	self->setAngle(angle);
}

// ------------------- // 


int main() {
	World world;

	if (world.Construct(800, 600, 1, 1))
		world.Start();
	return 0;
}