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
// - Time for code cleanup and separate into files and classes.
// - Move shape color into Shape class, not struct.
// - Collisions: Line-circle, point-triangle, point-rect, point-circle, circle-circle, circle-rect, rect-rect
// - Gyro
// - Ninja rope
// 
// 
// 
// - AI: flee, follow, patrol, attack, defend, random, etc



#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <memory>

/*
Effects:
- gravity / motion. Different types: force relative to point, relative to horizontal/vertical, and probably more..
- friction (air resistance)
- damage (later: heat, piercing, explosion, etc)
- how to fit in gyro/aim here? in player: Laser l; if l.collide .. ?? yeah must hard code that anyway. Can put a variable for that specifically, and a test.

*/


using namespace std;

struct sTriangle {
	float x1, y1;
	float x2, y2;
	float x3, y3;
	olc::Pixel color;
};

struct sCircle { float x, y, r; };
struct sRect { float x, y, w, h; };
struct sLine { float x1, y1, x2, y2; };

class AI;
class Shape;

// Referred to by a shape
class ITrigger {
public:
	ITrigger();
	virtual void trigger(shared_ptr<Shape> owner, shared_ptr<Shape> victim) = 0; // should be enough to give 1 object, and also the px,py of the collision point.
};

class IHull {
public:
	virtual void damage(float amount) = 0;
	//virtual void repair(float amount) = 0;
protected:
	float maxArmor;
	float currentArmor;
	// shield later..
	shared_ptr<Shape> owner;
};

class Shape {
public:
	virtual float centroidX() = 0;
	virtual float centroidY() = 0;
	virtual float area() = 0;
	virtual void rotate(float angle_) = 0; // rotate around centroid
	float getAngle() { return angle; }
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

	void updatePosition() { x += vx; y += vy; }

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
	shared_ptr<AI> ai;
	shared_ptr<ITrigger> varTrigger;
	shared_ptr<IHull> hull;
	// bool isColliding; // could do this, but keep a vector in World class instead. Only properties goes in objects.
};


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

class Triangle : public Shape {
public: 

	Triangle(sTriangle t, float x_, float y_) {
		original_shape = t;
		x = x_; y = y_;
		angle = 0.0f;

		float cx = centroidX();
		float cy = centroidY();
		cout << "cx:" << cx << "  cy:" << cy << endl;
		cout << "x" << x << "  y:" << y << endl;
		original_shape.x1 -= cx;
		original_shape.y1 -= cy;
		original_shape.x2 -= cx;
		original_shape.y2 -= cy;
		original_shape.x3 -= cx;
		original_shape.y3 -= cy;

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
		result.color = original_shape.color;
		return result;
	}
	float area() { return 0.5f * abs((original_shape.x1 - original_shape.x3) * (original_shape.y2 - original_shape.y1) - (original_shape.x1 - original_shape.x2) * (original_shape.y3 - original_shape.y1)); }
	float centroidX() override { return (original_shape.x1 + original_shape.x2 + original_shape.x3) / 3.0f; }
	float centroidY() override { return (original_shape.y1 + original_shape.y2 + original_shape.y3) / 3.0f; }
	void rotate(float angle) override;
	void applyRotation();
public:
	//shared_ptr<Obj> owner;
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

void Triangle::rotate(float angle_) {
	angle += angle_;
	applyRotation();
}

class Circle : public Shape {
public:
	float r;
	olc::Pixel color;
	Circle(float x_, float y_, float r_, olc::Pixel color_) {
		x = x_; y = y_; r = r_; color = color_;
	}
	float centroidX() override { return 0.0f; }
	float centroidY() override { return 0.0f; }
	float area() override { return 3.14f * r * r; }
	void rotate(float angle_) override {
		angle += angle_;
	}

};

// axes aligned with x and y, no rotation
class Rect: public Shape {
public:
	Rect(float x_, float y_, float w_, float h_, olc::Pixel color_) {
		x = x_; y = y_; w = w_; h = h_; color = color_;
	}
	float x, y, w, h;
	olc::Pixel color;
	float centroidX() override { return (x + w) / 2.0f; } // TODO: Rect isn't updated to have world coordinates and local yet. has only world.
	float centroidY() override { return (y + h) / 2.0f; }
	float area() override { return w * h; }
	void rotate(float angle_) override {
		angle += angle_; // no rotation, but can be used for direction, if it's a moving object.
	}
	sRect getStruct() { return sRect{ x, y, w, h }; }
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














ITrigger::ITrigger() {};
/*
class TriggerBomb : public ITrigger {
	void trigger(Obj owner, vector<Obj*> victims) {
		// victims will be all objects within a certain radius of the bomb.
		// loop through them, and apply damage and force.
	};
};
class TriggerLaser : public ITrigger {
	void trigger(Obj owner, vector<Obj*> victims) {};
};
*/




class AI {
public:
	virtual void update(float tElapsedTime) = 0;

	virtual bool force(float& px, float& py, float& magnitude, float& radius_of_influence) { return false; }
	shared_ptr<Shape> getExternal() { return external; }
	shared_ptr<Shape> getSelf() { return self; }
protected:
	shared_ptr<Shape> self, external;
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
			cout << "magnetic" << endl;
		}
	}

	bool force(float &px, float &py, float &magnitude, float &radius_of_influence) override {
		if (!magnetic) return false;
		px = self->getX();
		py = self->getY();
		magnitude = 0.01f;
		radius_of_influence = 100.0f;
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
		shared_ptr<Triangle> shape2 = make_shared<Triangle>(sTriangle{ 10, 8, 10, 22, 30, 15, olc::RED }, rand() % ScreenWidth(), rand() % ScreenHeight());
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

		shared_ptr<Triangle> shape = make_shared<Triangle>(sTriangle { 10, 8, 10, 22, 30, 15, olc::RED }, 50, 50);
		sharedPtrTriangles.push_back(shape);
		user_controlled_shape = shape;

		for (int i = 0; i < 20; i++)
			addRandomEnemy();

		for (int i = 0; i < 5; i++) {
			shared_ptr<Circle> circle = make_shared<Circle>(rand() % ScreenWidth(), rand() % ScreenHeight(), 5.0f, olc::GREEN);
			sharedPtrCircles.push_back(circle);

			shared_ptr<AI> ai = make_shared<AI_magbomb>(circle, nullptr);
			sharedPtrAI.push_back(ai);
		}

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

		if (GetKey(olc::Key::LEFT).bHeld)  { user_controlled_shape->rotate(-0.001f); }
		if (GetKey(olc::Key::RIGHT).bHeld) { user_controlled_shape->rotate( 0.001f); }

		if (GetKey(olc::Key::UP).bHeld) {
			user_controlled_shape->addForce(0.008f, user_controlled_shape->getAngle());
		}

		for (auto& each : sharedPtrAI) {
			each->update(fElapsedTime);
		}

		// combine all shapes into one vector
		/*vector<shared_ptr<Shape>> shapes;
		for (auto& each : sharedPtrTriangles) { shapes.push_back(each); }
		for (auto& each : sharedPtrRectangles) { shapes.push_back(each); }
		for (auto& each : sharedPtrCircles) { shapes.push_back(each); }*/

		// ----- COLLISION ----- //

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

		// ---- DRAW ----- //

		for (auto& each : sharedPtrTriangles)  { DrawTriangle(each->worldCoordinates()); }
		for (auto& each : sharedPtrRectangles) { FillRect(int(each->x), int(each->y), int(each->w), int(each->h), each->color); }
		for (auto& each : sharedPtrLines)      { DrawLine(int(each->getX()), int(each->getY()), int(each->getX2()), int(each->getY2()), olc::YELLOW); }
		for (auto& each : sharedPtrCircles)    { 
			FillCircle(int(each->getX()), int(each->getY()), int(each->r), each->color); 
			//DrawCircle(int(each->getX()), int(each->getY()), 40, olc::WHITE);
		}

		// ----- EFFECTS ----- //
		
		// Forces generated from AI, summed up for each shape
		for (auto& each : sharedPtrAI) {
			float px,py,magnitude,radius_of_influence;
			if (each->force(px, py, magnitude, radius_of_influence)) {
				for (auto& triangle : sharedPtrTriangles) {
					float dx = px - (triangle->centroidX() + triangle->getX());
					float dy = py - (triangle->centroidY() + triangle->getY());
					float distance = sqrt(dx * dx + dy * dy);
					if (distance < radius_of_influence) {
						float force = magnitude * (radius_of_influence - distance) / radius_of_influence;
						triangle->addForce(force, atan2(dy, dx));
						each->getSelf()->addForce(-force, atan2(dy, dx));
					}
				}
			}
		}

		// apply summed up force on each shape
		for (auto& triangle : sharedPtrTriangles) { triangle->applyForce(); }
		for (auto& circle : sharedPtrCircles) { circle->applyForce(); }

		// Remove dead objects
		for (auto& each : sharedPtrTriangles) {
			if (each->getKillFlag()) {
				sharedPtrTriangles.erase(std::remove(sharedPtrTriangles.begin(), sharedPtrTriangles.end(), each), sharedPtrTriangles.end());
				break;
			}
		}

		// ----- MOVEMENT ----- //

		for (auto& shape : sharedPtrCircles) { shape->updatePosition();}
		for (auto& shape : sharedPtrTriangles) { shape->updatePosition(); }

		sharedPtrLines.clear();

		return true;
	}

	void DrawTriangle(sTriangle t) { FillTriangle(int(t.x1), int(t.y1), int(t.x2), int(t.y2), int(t.x3), int(t.y3), t.color); }

	bool LineTriangleCollision(sLine ln, sTriangle tr, float &px, float &py) {
		sLine trA = sLine{ tr.x1, tr.y1, tr.x2, tr.y2 };
		sLine trB = sLine{ tr.x2, tr.y2, tr.x3, tr.y3 };
		sLine trC = sLine{ tr.x3, tr.y3, tr.x1, tr.y1 };
		bool collision = false;
		float px1, py1, px2, py2, px3, py3;
		float distance = 1000000.0f;
		if (LineLineCollision(ln, trA, px1, py1)) { collision = true; distance = sqrt((ln.x1 - px1) * (ln.x1 - px1) + (ln.y1 - py1) * (ln.y1 - py1)); px = px1; py = py1;}
		if (LineLineCollision(ln, trB, px2, py2)) { 
			collision = true; 
			float dist2 = sqrt((ln.x1 - px2) * (ln.x1 - px2) + (ln.y1 - py2) * (ln.y1 - py2)); 
			if (dist2 < distance) { px = px2; py = py2; distance = dist2;  }
		}
		if (LineLineCollision(ln, trC, px3, py3)) { 
			collision = true; 
			float dist3 = sqrt((ln.x1 - px3) * (ln.x1 - px3) + (ln.y1 - py3) * (ln.y1 - py3)); 
			if (dist3 < distance) { px = px3; py = py3; distance = dist3; }
		}
		return collision;
	}
	bool LineLineCollision(sLine A, sLine B, float &px, float &py) {
		float x1 = A.x1;
		float y1 = A.y1;
		float x2 = A.x2;
		float y2 = A.y2; 
		float x3 = B.x1;
		float y3 = B.y1;
		float x4 = B.x2;
		float y4 = B.y2;

		float uA = ((x4 - x3) * (y1 - y3) - (y4 - y3) * (x1 - x3)) / ((y4 - y3) * (x2 - x1) - (x4 - x3) * (y2 - y1));
		float uB = ((x2 - x1) * (y1 - y3) - (y2 - y1) * (x1 - x3)) / ((y4 - y3) * (x2 - x1) - (x4 - x3) * (y2 - y1));
		if (uA >= 0 && uA <= 1 && uB >= 0 && uB <= 1) {
			px = x1 + (uA * (x2 - x1));
			py = y1 + (uA * (y2 - y1));
			return true;
		}
		return false;
	}
	bool LineRectCollision(sLine ln, sRect rect, float &px, float &py) {
		sLine rectA = sLine{ rect.x, rect.y, rect.x + rect.w, rect.y };
		sLine rectB = sLine{ rect.x + rect.w, rect.y, rect.x + rect.w, rect.y + rect.h };
		sLine rectC = sLine{ rect.x + rect.w, rect.y + rect.h, rect.x, rect.y + rect.h };
		sLine rectD = sLine{ rect.x, rect.y + rect.h, rect.x, rect.y };
		bool collision = false;
		float px1, py1, px2, py2, px3, py3, px4, py4;
		float distance = 1000000.0f;
		if (LineLineCollision(ln, rectA, px1, py1)) { collision = true; distance = sqrt((ln.x1 - px1) * (ln.x1 - px1) + (ln.y1 - py1) * (ln.y1 - py1)); px = px1; py = py1; }
		if (LineLineCollision(ln, rectB, px2, py2)) { collision = true; float dist2 = sqrt((ln.x1 - px2) * (ln.x1 - px2) + (ln.y1 - py2) * (ln.y1 - py2)); if (dist2 < distance) { px = px2; py = py2; distance = dist2; } }
		if (LineLineCollision(ln, rectC, px3, py3)) { collision = true; float dist3 = sqrt((ln.x1 - px3) * (ln.x1 - px3) + (ln.y1 - py3) * (ln.y1 - py3)); if (dist3 < distance) { px = px3; py = py3; distance = dist3; } }
		if (LineLineCollision(ln, rectD, px4, py4)) { collision = true; float dist4 = sqrt((ln.x1 - px4) * (ln.x1 - px4) + (ln.y1 - py4) * (ln.y1 - py4)); if (dist4 < distance) { px = px4; py = py4; distance = dist4; } }
		return collision;
	}
	bool LineCircleCollision(sLine ln, sCircle circle) {}
	bool PointTriangleCollision(float x, float y, sTriangle tr) {}
	bool PointRectCollision(float x, float y, sRect rect) {
		if (x > rect.x && x < rect.x + rect.w && y > rect.y && y < rect.y + rect.h)
			return true;
		else
			return false;
	}
};

int main() {
	World world;
	if (world.Construct(500, 400, 1, 1))
		world.Start();
	return 0;
}