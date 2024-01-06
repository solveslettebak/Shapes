#pragma once

#include "structs.h"

using namespace std;

class Shape;
class World;

#include <memory>

/*
* A shape can have an AI. Often another shape is involved also, which has its own AI too. Example: Player has AI, creates laser shape, which has a laser AI
*/

class AI {
public:
	virtual void update(float tElapsedTime) = 0;
	virtual bool force(float& px, float& py, float& magnitude, float& radius_of_influence, ForceType& ForceType) { return false; }
	virtual void destroy() { destroyFlag = true; }
	virtual void setup() {} // called once at the beginning
	virtual void trigger(shared_ptr<Shape> other_object, float fElapsedTime) {} // called on collision

	bool getDestroyFlag() { return destroyFlag; }
	void setDestroyFlag(bool destroyFlag_) { destroyFlag = destroyFlag_; }
	shared_ptr<Shape> getExternal() { return external; }
	shared_ptr<Shape> getSelf() { return self; }

	//void setWorld(shared_ptr<World> world_) { world = world_; }
	void setWorld(World* world_) { world = shared_ptr<World>(world_); }
	shared_ptr<World> getWorld() { return world; }

	shared_ptr<Shape> getOwner() { return external; }
	shared_ptr<Shape> getShape() { return self; }
protected:
	shared_ptr<Shape> self, external; // self is the object that has this AI. External is the owner of the shape that has this AI (external may vary...)
	bool destroyFlag = false;

	//void* world; // Hack. Points to world object.
	shared_ptr<World> world;
};

class AI_laser : public AI {
public:
	AI_laser(shared_ptr<Shape> self_, shared_ptr<Shape> external_);
	void update(float tElapsedTime) override;
	void trigger(shared_ptr<Shape> other_object, float fElapsedTime) override;
};

class AI_aim : public AI {
protected:
	bool locked = false;
	shared_ptr<Shape> locked_on_object;
public:
	AI_aim(shared_ptr<Shape> self_, shared_ptr<Shape> external_);
	void key_released();
	virtual void trigger(shared_ptr<Shape> other_object, float fElapsedTime);
	virtual void update(float fElapsedTime);
};

class AI_ninjarope : public AI {
protected:
	bool locked = false;
	bool magnetic = false;
	shared_ptr<Shape> locked_on_object;
	float timePassed = 0.0f;
public:
	AI_ninjarope(shared_ptr<Shape> self_, shared_ptr<Shape> external_);
	void setup() override;
	void update(float tElapsedTime) override;
	void trigger(shared_ptr<Shape> other_object, float fElapsedTime) override;
	bool force(float& px, float& py, float& magnitude, float& radius_of_influence, ForceType& ForceType) override;
};



class AI_magbomb : public AI {
public:
	AI_magbomb(shared_ptr<Shape> self_, shared_ptr<Shape> external_);
	// should explode when it collides with something
	// pushes all objects around it away when it hits something (apply small damage to them as well, depending on distance)
	void setup() override;
	void update(float tElapsedTime) override;
	bool force(float& px, float& py, float& magnitude, float& radius_of_influence, ForceType& ftype) override;
protected:
	float timePassed = 0.0f;
	bool magnetic = false;
	bool hasExploded = false;
	bool explode = false;

	//enum State { eCantCollide, eArmed, eExplode } state = eInitialDelay; // consider doing this as a state machine
};


// Simple AI that just rotates "self" to point towards the external object, and fires laser sometimes.
class AI_follow_user : public AI {
protected:
	float timePassed = 0.0f;
	bool shooting = false;
	shared_ptr<Shape> AI_controlled_laser;
public:
	AI_follow_user(shared_ptr<Shape> self_, shared_ptr<Shape> external_);
	void update(float tElapsedTime) override;
};