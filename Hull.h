#pragma once

using namespace std;

class Shape;
#include <memory>


class IHull {
public:
	virtual void damage(float amount) = 0;
	//virtual void repair(float amount) = 0;
	float getArmor();
	float getMaxArmor();
	float getHealth();
	void setArmor(float armor);
protected:
	float maxArmor;
	float currentArmor;
	// shield later..
	shared_ptr<Shape> owner;
};

float IHull::getArmor() {
	return currentArmor;
}

float IHull::getMaxArmor() {
	return maxArmor;
}

float IHull::getHealth() {
	return currentArmor / maxArmor;
}

void IHull::setArmor(float armor) {
	currentArmor = armor;
}
