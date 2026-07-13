
#pragma once
#include "raylib.h"


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

Vector2 hexagonCorner(Vector2 centerPos, float size, float sideNumber, float offsetRotation);
void drawHexagonOutline(Vector2 centrePoint, float size, Color color, float rotationOffset);
void drawHexagon(Vector2 centrePoint, float size, Color color1, Color color2);
void drawHexagonR(Vector2 centrePoint, float size, Color color1, Color color2, float rotation);
void drawHexagonB(Vector2 centrePoint, float size, Color color1);


float hexVerticalDiff(float size);
float hexHorizDiff(float size);