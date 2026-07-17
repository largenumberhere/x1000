#include "draw_hex_tiles.h"

#include <tgmath.h>

#include "game.h"
#include "hexagons.h"
#include "raylib.h"
#include "raymath.h"
#include "string.h"
#include "stdio.h"

#include "ray_wrap/draw_hexagon.h"

void fmtHex(char* medBuffer, float value) {
	int v = (int) value;
	int rhs = v % 0x100;
	int lhs = (v - rhs) / 0x100;
	sprintf(medBuffer, "x\n");
	sprintf(medBuffer+1, "%02x", rhs);
}

void drawHexTiles() {

	AxialHex centreHexAxial = {2, 2};
	float tileSize = 62;
	Vector2 tilesOffset = {120 + hexHorizDiff(tileSize) / 2, 250 + hexVerticalDiff(tileSize) / 2};

	Font font = GetFontDefault();

	// draw bg hexagon
	{;
		float bgSize = 320;
		Vector2 px = hexToVec2(centreHexAxial, tileSize);
		px = Vector2Add(px, tilesOffset);

		drawHexagonR(px, bgSize, clrYellow, clrDarkGreen, 30);
		drawHexagonR(px, bgSize -10, clrDarkGreen, clrDarkGreen, 30);

	}


	// draw plain grid
	for (int r = 0; r < MAX_R; r++) {
		for (int q = 0; q < MAX_Q; q++) {
			AxialHex hexUnit = {q, r};

			bool isUnusedTile = !hexTileUseable(hexUnit);

			Vector2 px = hexToVec2(hexUnit, tileSize);
			px = Vector2Add(px, tilesOffset);

			if (!isUnusedTile) {
				drawHexagon(px, tileSize, clrLightGreen, BLACK);
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
			if (!hexTileOccupied(hexUnit)) {
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


				if (hexTileUseable(hexUnit) && !hexTileOccupied(hexUnit) || debugTiles) {

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

			bool isUnusedTile = !hexTileUseable(hexUnit);

			if (!isUnusedTile || debugTiles) {
				float tileValue = hexTileGet(hexUnit);

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
		// highlight 0x100 tiles
		// draw dots on the edges to visually indicate tile magnitude
		for (int r = 0; r < MAX_R; r++) {
			for (int q = 0; q < MAX_Q; q++) {
				AxialHex hexUnit = {q, r};

				Vector2 px = hexToVec2(hexUnit, tileSize);
				px = Vector2Add(px, tilesOffset);


				bool isUnusedTile = !hexTileUseable(hexUnit);
				bool isBigValueTile = hexTileGet(hexUnit) >= 0x100;

				if (!isUnusedTile && isBigValueTile) {
					drawHexagon(px, 35, clrYellow, clrLightGreen);
				}
			}
		}
	}

	{
		char hexHexBuff[16] = {0};
		// draw text on hexagons
		for (int r = 0; r < MAX_R; r++) {
			for (int q = 0; q < MAX_Q; q++) {
				AxialHex hexUnit = {q, r};

				bool isTileEmpty = !hexTileOccupied(hexUnit);
				bool isUnusedTile = !hexTileUseable(hexUnit);

				Vector2 px = hexToVec2(hexUnit, tileSize);
				px = Vector2Add(px, tilesOffset);


				float value = hexTileGet(hexUnit);
				fmtHex(hexHexBuff, value);


				if ((!isUnusedTile || debugTiles) && !isTileEmpty) {
					drawTextCentred(px, font,  hexHexBuff, 35, clrOrange);
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

	{
		if (debugTiles) {

			char buff[128] = {0};
			for (int q = 0; q < MAX_Q; q++) {
				for (int r = 0; r < MAX_R; r++) {
					AxialHex hexUnit = {q, r};
					Vector2 px = hexToVec2(hexUnit, tileSize);
					px = Vector2Add(px, tilesOffset);

					buff[0] = '\0';

					sprintf(buff, "%.0f,%.0f", hexUnit.q, hexUnit.r);
					DrawText(buff, px.x, px.y, 40, WHITE);
				}
			}
		}
	}

}
