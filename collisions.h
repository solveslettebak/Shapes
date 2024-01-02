#pragma once
#include "structs.h"
#include <memory>
#include <vector>
#include <iostream>

// ----- Helper functions ----- //

float minimum(float a, float b, float c) {
	if (a <= b && a <= c) return a;
	if (b <= a && b <= c) return b;
	return c;
}

float maximum(float a, float b, float c) {
	if (a >= b && a >= c) return a;
	if (b >= a && b >= c) return b;
	return c;
}

sRect TriangleToRectangle(sTriangle tr) {
	float x = minimum(tr.x1, tr.x2, tr.x3);
	float y = minimum(tr.y1, tr.y2, tr.y3);
	float w = maximum(tr.x1, tr.x2, tr.x3) - x;
	float h = maximum(tr.y1, tr.y2, tr.y3) - y;
	return sRect{ x, y, w, h };
}

sRect CircleToRectangle(sCircle c) {
	return sRect{ c.x - c.r, c.y - c.r, c.r * 2, c.r * 2 };
}



// ----- Collision functions ----- //




bool LineLineCollision(sLine A, sLine B, float& px, float& py) {
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

struct Point {
	double x, y;
};

//bool doesIntersect(Point p1, Point p2, Point center, double radius) { 
bool LineCircleCollision(sLine ln, sCircle circle, float & px, float & py) {
	Point p1 = Point{ ln.x1, ln.y1 };
	Point p2 = Point{ ln.x2, ln.y2 };
	Point center = Point{ circle.x, circle.y };
	// Calculate the squared distance between the circle center and the line segment
	float lineLengthSq = (p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y);
	float dotProduct = ((center.x - p1.x) * (p2.x - p1.x)) + ((center.y - p1.y) * (p2.y - p1.y));
	float t = dotProduct / lineLengthSq;

	// Find the closest point on the line segment to the circle center
	Point closestPoint;
	if (t < 0) {
		closestPoint = p1;
	}
	else if (t > 1) {
		closestPoint = p2;
	}
	else {
		closestPoint.x = p1.x + t * (p2.x - p1.x);
		closestPoint.y = p1.y + t * (p2.y - p1.y);
	}

	// Check if the closest point is inside the circle
	float distanceSq = (closestPoint.x - center.x) * (closestPoint.x - center.x) +
		(closestPoint.y - center.y) * (closestPoint.y - center.y);

	if (distanceSq <= (circle.r * circle.r)) {
		// Calculate the intersection point on the circle
		//float d = sqrt(circle.r * circle.r - distanceSq);
		//float factor = sqrt(lineLengthSq) / (2 * sqrt(lineLengthSq));
		px = closestPoint.x;// +factor * (p2.y - p1.y) * d;
		py = closestPoint.y;// -factor * (p2.x - p1.x) * d;
		return true;
	}
	return false;
}

bool _LineCircleCollision(sLine ln, sCircle circle, float& px, float& py) {
	float cx = circle.x;
	float cy = circle.y;
	float r = circle.r;
	float x1 = ln.x1;
	float y1 = ln.y1;
	float x2 = ln.x2;
	float y2 = ln.y2;

	float dx = x2 - x1;
	float dy = y2 - y1;

	float A = dx * dx + dy * dy;
	float B = 2 * (dx * (x1 - cx) + dy * (y1 - cy));
	float C = (x1 - cx) * (x1 - cx) + (y1 - cy) * (y1 - cy) - r * r;

	float det = B * B - 4 * A * C;
	if ((A <= 0.0000001) || (det < 0)) {
		// No real solutions.
		return false;
	}
	else if (det == 0) {
		// One solution.
		float t = -B / (2 * A);
		if (t < 0) return false;
		px = x1 + t * dx;
		py = y1 + t * dy;
		return true;
	}
	else {
		// Two solutions.
		float t = (-B + sqrt(det)) / (2 * A);
		if (t < 0) return false;
		px = x1 + t * dx;
		py = y1 + t * dy;
		return true;
	}
}

bool CircleCircleCollision(sCircle c1, sCircle c2) { // , float& px, float& py
	float x1 = c1.x;
	float y1 = c1.y;
	float r1 = c1.r;
	float x2 = c2.x;
	float y2 = c2.y;
	float r2 = c2.r;

	float dx = x2 - x1;
	float dy = y2 - y1;
	float d = sqrt(dx * dx + dy * dy);
	if (d > r1 + r2) {
		return false;
	}
/*	if (d < abs(r1 - r2)) {
		return false;
	}
	if (d == 0 && r1 == r2) {
		return false;
	}

	float a = (r1 * r1 - r2 * r2 + d * d) / (2 * d);
	float h = sqrt(r1 * r1 - a * a);
	float x3 = x1 + a * (x2 - x1) / d;
	float y3 = y1 + a * (y2 - y1) / d;
	px = x3 + h * (y2 - y1) / d;
	py = y3 - h * (x2 - x1) / d;*/
	return true;
}

bool CircleTriangleCollision(sCircle c, sTriangle r) {
	sLine trA = sLine{ r.x1, r.y1, r.x2, r.y2 };
	sLine trB = sLine{ r.x2, r.y2, r.x3, r.y3 };
	sLine trC = sLine{ r.x3, r.y3, r.x1, r.y1 };

	std::vector<sLine> trLines = { trA, trB, trC };

	int count = 0;
	for (auto& trLine : trLines) {
		count++;
		float px, py;
		if (LineCircleCollision(trLine, c, px, py)) {
			//std::cout << "collision on line " << count << std::endl;
			return true;
		}
	}
	return false;
}



bool CircleRectangleCollision(sCircle c, sRect r) {
	sLine rectA = sLine{ r.x, r.y, r.x + r.w, r.y };
	sLine rectB = sLine{ r.x + r.w, r.y, r.x + r.w, r.y + r.h };
	sLine rectC = sLine{ r.x + r.w, r.y + r.h, r.x, r.y + r.h };
	sLine rectD = sLine{ r.x, r.y + r.h, r.x, r.y };

	std::vector<sLine> rectLines = { rectA, rectB, rectC, rectD };

	for (auto& rectLine : rectLines) {
		float px, py;
		if (LineCircleCollision(rectLine, c, px, py)) {
			return true;
		}
	}
	return false;
}

bool LineTriangleCollision(sLine ln, sTriangle tr, float& px, float& py) {
	sLine trA = sLine{ tr.x1, tr.y1, tr.x2, tr.y2 };
	sLine trB = sLine{ tr.x2, tr.y2, tr.x3, tr.y3 };
	sLine trC = sLine{ tr.x3, tr.y3, tr.x1, tr.y1 };
	bool collision = false;
	float px1, py1, px2, py2, px3, py3;
	float distance = 1000000.0f;
	if (LineLineCollision(ln, trA, px1, py1)) { collision = true; distance = sqrt((ln.x1 - px1) * (ln.x1 - px1) + (ln.y1 - py1) * (ln.y1 - py1)); px = px1; py = py1; }
	if (LineLineCollision(ln, trB, px2, py2)) {
		collision = true;
		float dist2 = sqrt((ln.x1 - px2) * (ln.x1 - px2) + (ln.y1 - py2) * (ln.y1 - py2));
		if (dist2 < distance) { px = px2; py = py2; distance = dist2; }
	}
	if (LineLineCollision(ln, trC, px3, py3)) {
		collision = true;
		float dist3 = sqrt((ln.x1 - px3) * (ln.x1 - px3) + (ln.y1 - py3) * (ln.y1 - py3));
		if (dist3 < distance) { px = px3; py = py3; distance = dist3; }
	}
	return collision;
}

bool LineRectCollision(sLine ln, sRect rect, float& px, float& py) {
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

bool RectangleRectangleCollision(sRect r1, sRect r2) {
	return (r1.x < r2.x + r2.w && r1.x + r1.w > r2.x && r1.y < r2.y + r2.h && r1.y + r1.h > r2.y);
}



bool TriangleRectCollision(sTriangle tr, sRect rect, float& px, float& py) {
	 // early out if the rectangles don't intersect. Doesn't seem to help.
	sRect trRect = TriangleToRectangle(tr);
	if (!RectangleRectangleCollision(rect, trRect)) {
		return false;
	}	

	sLine trA = sLine{ tr.x1, tr.y1, tr.x2, tr.y2 };
	sLine trB = sLine{ tr.x2, tr.y2, tr.x3, tr.y3 };
	sLine trC = sLine{ tr.x3, tr.y3, tr.x1, tr.y1 };

	std::vector<sLine> trLines = { trA, trB, trC };

	sLine rectA = sLine{ rect.x, rect.y, rect.x + rect.w, rect.y };
	sLine rectB = sLine{ rect.x + rect.w, rect.y, rect.x + rect.w, rect.y + rect.h };
	sLine rectC = sLine{ rect.x + rect.w, rect.y + rect.h, rect.x, rect.y + rect.h };
	sLine rectD = sLine{ rect.x, rect.y + rect.h, rect.x, rect.y };

	std::vector<sLine> rectLines = { rectA, rectB, rectC, rectD };

	for (auto trLine : trLines) {
		for (auto rectLine : rectLines) {
			if (LineLineCollision(trLine, rectLine, px, py)) {
				return true;
			}
		}
	}

	return false;
}

bool TriangleTriangleCollision(sTriangle t1, sTriangle t2) {
	sLine t1A = sLine{ t1.x1, t1.y1, t1.x2, t1.y2 };
	sLine t1B = sLine{ t1.x2, t1.y2, t1.x3, t1.y3 };
	sLine t1C = sLine{ t1.x3, t1.y3, t1.x1, t1.y1 };

	std::vector<sLine> t1Lines = { t1A, t1B, t1C };

	sLine t2A = sLine{ t2.x1, t2.y1, t2.x2, t2.y2 };
	sLine t2B = sLine{ t2.x2, t2.y2, t2.x3, t2.y3 };
	sLine t2C = sLine{ t2.x3, t2.y3, t2.x1, t2.y1 };

	std::vector<sLine> t2Lines = { t2A, t2B, t2C };

	for (auto t1Line : t1Lines) {
		for (auto t2Line : t2Lines) {
			float px, py;
			if (LineLineCollision(t1Line, t2Line, px, py)) {
				return true;
			}
		}
	}
	return false;
}


float sign(Point p1, Point p2, Point p3)
{
	return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

bool PointInTriangle(Point pt, Point v1, Point v2, Point v3)
{
	float d1, d2, d3;
	bool has_neg, has_pos;

	d1 = sign(pt, v1, v2);
	d2 = sign(pt, v2, v3);
	d3 = sign(pt, v3, v1);

	has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
	has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

	return !(has_neg && has_pos);
}

bool PointTriangleCollision(float x, float y, sTriangle tr) {
	// use PointInTriangle:
	return PointInTriangle(Point{ x, y }, Point{ tr.x1, tr.y1 }, Point{ tr.x2, tr.y2 }, Point{ tr.x3, tr.y3 });
}

bool PointRectCollision(float x, float y, sRect rect) {
	return (x > rect.x && x < rect.x + rect.w && y > rect.y && y < rect.y + rect.h);
}

