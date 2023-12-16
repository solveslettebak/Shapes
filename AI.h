#pragma once


enum ForceType { eGravity, eMagnetic, eFriction };

using namespace std;

class Shape;

#include <memory>


class AI {
public:
	virtual void update(float tElapsedTime) = 0;
	virtual bool force(float& px, float& py, float& magnitude, float& radius_of_influence, ForceType& ForceType) { return false; }
	virtual void destroy() { destroyFlag = true; }
	bool getDestroyFlag() { return destroyFlag; }
	shared_ptr<Shape> getExternal() { return external; }
	shared_ptr<Shape> getSelf() { return self; }
protected:
	shared_ptr<Shape> self, external;
	bool destroyFlag = false;
};
