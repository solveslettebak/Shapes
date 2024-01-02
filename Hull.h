#pragma once

using namespace std;

class Shape;
#include <memory>

class IHull {
public:
	virtual void damage(float amount) = 0;
	//virtual void damage(float shrapnel, float blast, float heat) = 0; // TODO: implement
	//virtual void repair(float amount) = 0;
	float getArmor() { return currentArmor; }
	float getMaxArmor() { return maxArmor; }
	float getHealth() { return currentArmor / maxArmor; }
	void setArmor(float armor) { currentArmor = armor; }
protected:
	float maxArmor;
	float currentArmor;
	shared_ptr<Shape> owner;

	// to be implemented
	bool isMagnetic = true;
};