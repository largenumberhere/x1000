#include "ray_wrap/draw_hexagon.h"

#include <math.h>


float hexVerticalDiff(float size) {
	return (float)3/(float)2 * size;
}

float hexHorizDiff(float size) {
	return sqrtf(3) * size;
}


/* For flat hexagons. Computes the outer edge coordinate from the inner edge and a count of degrees.
 * Adapted from https://www.redblobgames.com/grids/hexagons/
 * Supports over 360 degrees somehow
 *
 *	  *****
 *	*      *
 * *		*
 *	*	   *
 *	  *****
 *	( ^ a flat-top hexagon  )
 */
Vector2 hexagonCorner(Vector2 centerPos, float size, float sideNumber, float offsetRotation) {
	float degrees = 60 * sideNumber - 30 + offsetRotation;
	float radians = PI / 180 * degrees;

	Vector2 point = {centerPos.x + size * cos(radians),
		centerPos.y + size * sin(radians)
	};
	return point;
}

void drawHexagonOutline(Vector2 centrePoint, float size, Color color, float rotationOffset) {
	// Draw outline
	// (A big choppy on corners, but shhh it's fine with small line tickness.)
	for (int i = 1; i < 6 + 1; i++) {
		Vector2 leftPoint = hexagonCorner(centrePoint, size, i - 1, rotationOffset);
		Vector2 rightPoint = hexagonCorner(centrePoint, size, i, rotationOffset);
		DrawLineEx(leftPoint, rightPoint, 3, color);
	}
}


void drawHexagon(Vector2 centrePoint, float size, Color color1, Color color2) {
	// Draw 6 triangles to compose the shape, starting from the centre point.
	// Wraps around for the first one to always have 3 valid points for every single valid triangle
	for (int i = 1; i < 6 + 1; i++) {
		Vector2 leftPoint = hexagonCorner(centrePoint, size, i -1, 0);
		Vector2 rightPoint = hexagonCorner(centrePoint, size, i, 0);

		DrawTriangle(centrePoint, rightPoint, leftPoint, color1);
	}

	drawHexagonOutline(centrePoint, size, color2, 0);

}

void drawHexagonR(Vector2 centrePoint, float size, Color color1, Color color2, float rotation) {
	for (int i = 1; i < 6 + 1; i++) {
		Vector2 leftPoint = hexagonCorner(centrePoint, size, i -1, rotation);
		Vector2 rightPoint = hexagonCorner(centrePoint, size, i, rotation);

		DrawTriangle(centrePoint, rightPoint, leftPoint, color1);
	}

	drawHexagonOutline(centrePoint, size, color2, rotation);
}

void drawHexagonB(Vector2 centrePoint, float size, Color color1) {
	for (int i = 1; i < 6 + 1; i++) {
		Vector2 leftPoint = hexagonCorner(centrePoint, size, i -1, 0);
		Vector2 rightPoint = hexagonCorner(centrePoint, size, i, 0);

		DrawTriangle(centrePoint, rightPoint, leftPoint, color1);
	}
}
