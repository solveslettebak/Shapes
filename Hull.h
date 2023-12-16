#pragma once

using namespace std;

class Shape;
#include <memory>


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