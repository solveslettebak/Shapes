#pragma once

using namespace std;

class Shape;
#include <memory>

// Referred to by a shape
class ITrigger {
public:
	ITrigger();
	virtual void trigger(shared_ptr<Shape> owner, shared_ptr<Shape> victim) = 0; // should be enough to give 1 object, and also the px,py of the collision point.
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



