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

