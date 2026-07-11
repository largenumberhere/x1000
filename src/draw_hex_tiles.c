#include "../include/draw_hex_tiles.h"

#include <tgmath.h>

#include "../include/game.h"
#include "../include/hexagons.h"
#include "raylib.h"
#include "raymath.h"
#include "string.h"
#include "stdio.h"


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
Vector2 hexagonCorner(Vector2 centerPos, float size, float sideNumber, float offsetRoation) {
	float degrees = 60 * sideNumber - 30 + offsetRoation;
	float radians = PI / 180 * degrees;

	Vector2 point = {centerPos.x + size * cos(radians),
		centerPos.y + size * sin(radians)
	};
	return point;
}

void drawHexagonOutline(Vector2 centrePoint, float size, Color color, float roationOffset) {
	// Draw outline
	// (A big choppy on corners, but shhh it's fine with small line tickness.)
	for (int i = 1; i < 6 + 1; i++) {
		Vector2 leftPoint = hexagonCorner(centrePoint, size, i - 1, roationOffset);
		Vector2 rightPoint = hexagonCorner(centrePoint, size, i, roationOffset);
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

void drawHexagonR(Vector2 centrePoint, float size, Color color1, Color color2, float roation) {
	for (int i = 1; i < 6 + 1; i++) {
		Vector2 leftPoint = hexagonCorner(centrePoint, size, i -1, roation);
		Vector2 rightPoint = hexagonCorner(centrePoint, size, i, roation);

		DrawTriangle(centrePoint, rightPoint, leftPoint, color1);
	}

	drawHexagonOutline(centrePoint, size, color2, roation);
}



void fmtHex(char* medBuffer, float value) {
	int v = (int) value;
	int rhs = v % 0x100;
	int lhs = (v - rhs) / 0x100;
	sprintf(medBuffer, "x\n");
	sprintf(medBuffer+1, "%02x", rhs);
}

void drawTextCentred(Vector2 centrePoint, Font font, const char* str, float textSize, Color color) {
	float spcaing = 0;

	Vector2 size = MeasureTextEx(font, str, textSize, spcaing);

	Vector2 two = {2,  2};
	Vector2 middle = Vector2Divide(size, two);

	Vector2 position = {centrePoint.x - middle.x, centrePoint.y - middle.y};

	DrawTextPro(GetFontDefault(), str, position, Vector2Zero(), 0, textSize, spcaing, color);
}


// note: vertical and horizontal heights differ
// // flat-topped hexagon
// Vector2 hexCentreToStartPty(Vector2 pos, float size) {
// 	pos.x -= size;
// 	pos.y -= size;
//
// 	return  pos;
// }
//
// // pointy-topped hexagon
// Vector2 hexCentreToStartFlt(Vector2 pos, float size) {
// 	float width = sqrtf(3) * size;
// 	float half = width /2;
//
// 	pos.x -= half;
// 	pos.y -= half;
//
// 	return  pos;
// }


void drawHexTiles() {

	AxialHex centreHexAxial = {2, 2};
	float tileSize = 75;
	Vector2 tilesOffset = {40 + hexHorizDiff(tileSize) / 2, 200 + hexVerticalDiff(tileSize) / 2};
	Font font = GetFontDefault();
	// LoadFont()

	// draw bg hexagon
	{;
		float bgSize = 400;
		Vector2 px = hexToVec2(centreHexAxial, tileSize);
		px = Vector2Add(px, tilesOffset);

		drawHexagonR(px, bgSize, clrYellow, clrDarkGreen, 30);
		drawHexagonR(px, bgSize -10, clrDarkGreen, clrDarkGreen, 30);

	}


	// draw plain grid
	for (int r = 0; r < MAX_R; r++) {
		for (int q = 0; q < MAX_Q; q++) {
			AxialHex hexUnit = {q, r};
			bool isUnusedTile = hexTiles.hex[q][r] == TILE_UNUSED;

			Vector2 px = hexToVec2(hexUnit, tileSize);
			px = Vector2Add(px, tilesOffset);

			if (!isUnusedTile) {
				drawHexagon(px, tileSize, clrLightGreen, BLACK);
			} else {
				if (debugTiles) {
					drawHexagon(px, tileSize, ORANGE, BLACK);
				}
			}
		}
	}



	{
		// draw centre tile
		Vector2 px = hexToVec2(centreHexAxial, tileSize);
		px = Vector2Add(px, tilesOffset);
		drawHexagon(px, tileSize, clrYellow, BLACK);
	}


	// draw tiles with non-zero value
	for (int r = 0; r < MAX_R; r++) {
		for (int q = 0; q < MAX_Q; q++) {
			AxialHex hexUnit = {q, r};
			if (!(hexTiles.hex[q][r] >= 1)) {
				continue;
			}

			Vector2 px = hexToVec2(hexUnit, tileSize);

			px = Vector2Add(px, tilesOffset);
			drawHexagon(px, tileSize - 20, clrDarkGreen, clrOrange);
		}
	}


	{
		// draw dots in centres
		for (int r = 0; r < MAX_R; r++) {
			for (int q = 0; q < MAX_Q; q++) {
				AxialHex hexUnit = {q, r};

				Vector2 px = hexToVec2(hexUnit, tileSize);
				px = Vector2Add(px, tilesOffset);

				bool isUnusedTile = hexTiles.hex[q][r] == TILE_UNUSED;
				bool isEmptyTile = hexTiles.hex[q][r] == TILE_EMPTY;

				if (!isUnusedTile && isEmptyTile || debugTiles) {

					bool isCentreTile = q == centreHexAxial.q && r == centreHexAxial.r;
					Color insideColor = isCentreTile ? clrYellow : clrLightGreen;

					drawHexagon(px, 30, clrDarkGreen, clrDarkGreen);
					drawHexagon(px, 25, insideColor, clrLightGreen);
				}
			}
		}
	}


	// draw dots on the edges to visually indicate tile magnitude
	for (int r = 0; r < MAX_R; r++) {
		for (int q = 0; q < MAX_Q; q++) {
			AxialHex hexUnit = {q, r};

			Vector2 px = hexToVec2(hexUnit, tileSize);
			px = Vector2Add(px, tilesOffset);


			bool isUnusedTile = hexTiles.hex[q][r] == TILE_UNUSED;

			if (!isUnusedTile || debugTiles) {
				float tileValue = hexTiles.hex[q][r];
				int exponent =  (int) sqrtf(tileValue);
				exponent -= 1;
				exponent = Clamp(exponent, 0, 6);

				int max = (int) fminf(exponent, 6);
				for (int i = 0; i < max; i++) {

					Vector2 insetCorner = hexagonCorner(px, tileSize-25, i, 0);

					Vector2 insetCorner2 = hexagonCorner(px, tileSize-25, i+1, 0);

					Vector2 point1 = Vector2Lerp(insetCorner, insetCorner2, 0.25);
					Vector2 point2 = Vector2Lerp(insetCorner, insetCorner2, 0.75);



					DrawLineEx(point1, point2, 12, clrYellow);
				}
			}
		}
	}


	{
		char hexHexBuff[16] = {0};
		// draw text on hexagons
		for (int r = 0; r < MAX_R; r++) {
			for (int q = 0; q < MAX_Q; q++) {
				bool isTileEmpty = hexTiles.hex[q][r] == TILE_EMPTY;
				bool isUnusedTile = hexTiles.hex[q][r] == TILE_UNUSED;

				AxialHex hexUnit = {q, r};
				Vector2 px = hexToVec2(hexUnit, tileSize);
				px = Vector2Add(px, tilesOffset);


				float value = hexTiles.hex[q][r];
				fmtHex(hexHexBuff, value);


				if ((!isUnusedTile || debugTiles) && !isTileEmpty) {
					drawTextCentred(px, font,  hexHexBuff, 50, clrOrange);
				}
			}
		}
	}


	// draw new tile fade-in
	{
		Vector2 px = hexToVec2(newTile, tileSize);
		px = Vector2Add(px, tilesOffset);
		float animDurationSecs = 1.0;
		float ratio = deltaSinceNewTile / animDurationSecs;
		float alpha = Lerp(1, 0, ratio);

		drawHexagon(px, tileSize-25 ,ColorAlpha(clrOrange ,alpha), clrDarkGreen );
	}
}
