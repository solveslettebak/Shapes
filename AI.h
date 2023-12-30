#pragma once

#include "structs.h"

using namespace std;

class Shape;
class World;

#include <memory>


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
protected:
	shared_ptr<Shape> self, external;
	bool destroyFlag = false;

	//void* world; // Hack. Points to world object.
	shared_ptr<World> world;
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