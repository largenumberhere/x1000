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



void drawHexTiles() {
	// draw bg hexagon
	{;
		float size = 400;
		Vector2 px = {gameRenderTextureSize.width / 2, gameDestinationScreenSize.height / 2};
		px.y += 100;


		drawHexagonR(px, size, clrYellow, clrDarkGreen, 30);
		drawHexagonR(px, size -10, clrDarkGreen, clrDarkGreen, 30);
	}

	Vector2 tilesInset = {40, 200};
	float tileSize = 75;


	// draw plain grid
	for (int r = 0; r < MAX_R; r++) {
		for (int q = 0; q < MAX_Q; q++) {
			AxialHex hexUnit = {q, r};
			bool isUnusedTile = hexTiles.hex[q][r] == TILE_UNUSED;

			Vector2 px = hexToVec2(hexUnit, tileSize);

			// centre the hex of hexes according to the normal display characteristics of a single hexagon.
			// this is not very precise and doesn't take into account the non-horizontal layout but it's nice
			px.x += (hexHorizDiff(tileSize) / 2);
			px.y += (hexVerticalDiff(tileSize) / 2);

			px.x += tilesInset.x; // offset from the top left
			px.y += tilesInset.y;


			if (!isUnusedTile) {
				drawHexagon(px, tileSize, clrLightGreen, BLACK);
			} else {
				if (debugTiles) {
					drawHexagon(px, tileSize, ORANGE, BLACK);
				}
			}
		}
	}





	AxialHex centre = {2, 2};
	{
		// draw centre tile
		Vector2 px = hexToVec2(centre, tileSize);
		px.x += tilesInset.x; // offset from the top left
		px.y += tilesInset.y;

		px.x += (hexHorizDiff(tileSize) / 2);
		px.y += (hexVerticalDiff(tileSize) / 2);
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

			// centre the hex of hexes according to the normal display characteristics of a single hexagon.
			// this is not very precise and doesn't take into account the non-horizontal layout but it's nice
			px.x += (hexHorizDiff(tileSize) / 2);
			px.y += (hexVerticalDiff(tileSize) / 2);

			px.x += tilesInset.x; // offset from the top left
			px.y += tilesInset.y;


			drawHexagon(px, tileSize - 20, clrDarkGreen, clrOrange);
		}
	}

	{
		// draw dots in centres
		for (int r = 0; r < MAX_R; r++) {
			for (int q = 0; q < MAX_Q; q++) {
				AxialHex hexUnit = {q, r};

				Vector2 px = hexToVec2(hexUnit, tileSize);
				px.x += (hexHorizDiff(tileSize) / 2);
				px.y += (hexVerticalDiff(tileSize) / 2);

				px.x += tilesInset.x; // offset from the top left
				px.y += tilesInset.y;


				bool isUnusedTile = hexTiles.hex[q][r] == TILE_UNUSED;

				if (!isUnusedTile || debugTiles) {

					bool isCentreTile = q == centre.q && r == centre.r;
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
			px.x += (hexHorizDiff(tileSize) / 2);
			px.y += (hexVerticalDiff(tileSize) / 2);

			px.x += tilesInset.x; // offset from the top left
			px.y += tilesInset.y;


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



					DrawLineEx(point1, point2, 13, clrOrange);
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

				// centre the hex of hexes according to the normal display characteristics of a single hexagon.
				// this is not very precise and doesn't take into account the non-horizontal layout but it's nice
				px.x += (hexHorizDiff(tileSize) / 2);
				px.y += (hexVerticalDiff(tileSize) / 2);

				px.x += tilesInset.x; // offset from the top left
				px.y += tilesInset.y;


				float value = hexTiles.hex[q][r];

				// float to string
				sprintf(hexHexBuff, "0x%x", (int) value);
				if (value == -1) {
					hexHexBuff[0] = '\0';
					strcat(hexHexBuff, "-0x1");
				}
				//

				if ((!isUnusedTile || debugTiles) && !isTileEmpty) {
					DrawText(hexHexBuff, px.x, px.y, 25, GREEN);
				}
			}
		}


	}

	// draw new tile fade-in
	{
		Vector2 px = hexToVec2(newTile, tileSize);
		px.x += (hexHorizDiff(tileSize) / 2);
		px.y += (hexVerticalDiff(tileSize) / 2);

		px.x += tilesInset.x; // offset from the top left
		px.y += tilesInset.y;
		float animDurationSecs = 1.0;
		float ratio = deltaSinceNewTile / animDurationSecs;
		float alpha = Lerp(1, 0, ratio);

		drawHexagon(px, tileSize-25 ,ColorAlpha(ORANGE ,alpha), clrDarkGreen );
	}


}
