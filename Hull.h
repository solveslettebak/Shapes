#pragma once

using namespace std;

class Shape;
#include <memory>

class IHull {
public:
	virtual void damage(float amount) = 0;
	//virtual void damage(float impact, float shrapnel, float blast, float heat) = 0; // TODO: implement
	//virtual void repair(float amount) = 0;

	virtual void update(float fElapsedTime) { currentArmor += repairRate * fElapsedTime; if (currentArmor > maxArmor) currentArmor = maxArmor; };

	float getArmor() { return currentArmor; }
	float getMaxArmor() { return maxArmor; }
	float getHealth() { return currentArmor / maxArmor; }
	void setArmor(float armor) { currentArmor = armor; }
	bool getDestroyFlag() { return destroyFlag; }
	void setDestroyFlag(bool destroyFlag_) { destroyFlag = destroyFlag_; }
	void setRepairRate(float repairRate_) { repairRate = repairRate_; }
protected:
	float maxArmor;
	float currentArmor;
	float repairRate = 0.0f;
	shared_ptr<Shape> owner;

	// to be implemented
	bool isMagnetic = true;
	bool destroyFlag = false;
};