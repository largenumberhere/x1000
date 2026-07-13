#include "clickables.h"
#include "game_error.h"
#include "game.h"
#include "../include/ray_wrap/rectangle_shrink.h"
#include "../include/ray_wrap/rectangle_centre.h"
#include "get_mouse_pos_rel.h"
#include "move_hexagons.h"
#include "ray_wrap/draw_arrow.h"

#include <stdbool.h>

#include "raymath.h"


float lmbCooldownGoal = 0.5f;
float lmbCooldown = 1.0f;





void drawCardinalArrow(ClickableDir clickMoveDirection, Vector2 origin, float offsetX, float offsetY) {
    float angle = 180;	// the origin points down for the arrow's implementation;
    switch (clickMoveDirection) {
        case CLICKABLE_DIR_NW:
            angle += 310;
            break;
        case CLICKABLE_DIR_E:
            angle += 90;
            break;
        case CLICKABLE_DIR_SW:
            angle += 45;
            break;
        case CLICKABLE_DIR_SE:
            angle += 135;
            break;
        case CLICKABLE_DIR_W:
            angle += 270;
            break;
        default:
            GAME_ASSERT(false);
    }

    Vector2 offset = {offsetX, offsetY};
    Vector2 pos = Vector2Add(origin, offset);
    drawArrow(pos, angle, 70);
}



void initClickables() {
    GameClickable defaultClickable = {
        .direction = CLICKABLE_DIR_SE,
        .position = {750, 600, 250, 90},
        .tickCooldownCurrent = 1000,
        .tickCooldownMax = 300,
        .hovered = false
    };

    GameClickable seArrow = defaultClickable;
    GameClickable nwArrow = defaultClickable;
    nwArrow.direction = CLICKABLE_DIR_NW;
    nwArrow.position = (Rectangle) {15, 250, 250, 90};
    GameClickable eArrow = defaultClickable;
    eArrow.direction = CLICKABLE_DIR_E;
    eArrow.position = (Rectangle) {830, 420, 150, 90};

    GameClickable wArrow = defaultClickable;
    wArrow.direction = CLICKABLE_DIR_W;
    wArrow.position = (Rectangle) {20, 420, 150, 90};

    // GameClickable arrow5 = defaultClickable;
    // arrow5.direction = CLICK_MOVE_SW;
    // arrow5.position = (Rectangle) {20, 670, 150, 90};



    clickables[clickableCount++] = seArrow;
    clickables[clickableCount++] = nwArrow;

    clickables[clickableCount++] = eArrow;
    clickables[clickableCount++] = wArrow;
    // clickables[clickableCount++] = arrow5;

}


void drawClickables() {
    float roundedness = 0.8;
    int segments = 1;

    for (int i = 0; i < clickableCount; i++) {
        DrawRectangleRounded(clickables[i].position, roundedness, segments, clrDarkGreen);

        Rectangle r1 = rectangleShrink(clickables[i].position, 4);
        DrawRectangleRounded(r1, roundedness, segments, clrYellow);

        Rectangle r2 = rectangleShrink(clickables[i].position, 8);

        if (clickables[i].tickCooldownCurrent < clickables[i].tickCooldownMax) {
            DrawRectangleRounded(r2, roundedness, segments, WHITE);
            DrawRectangleRounded(r2, roundedness, segments, ColorAlpha(clrDarkGreen, 0.8));

        } else if (clickables[i].hovered) {
            DrawRectangleRounded(r2, roundedness, segments, clrYellow);
        } else {
            DrawRectangleRounded(r2, roundedness, segments, clrDarkGreen);
        }


        int size = 80;
        Vector2 pos = { clickables[i].position.x, clickables[i].position.y};
        Rectangle textSubRec = clickables[i].position;
        textSubRec.width -= 60;

        Color color = clrOrange;
        switch (clickables[i].direction) {
            case CLICKABLE_DIR_SE:
                drawTextCentred(rectangleCentre(textSubRec), GetFontDefault(), "SE", size, color);
                drawCardinalArrow(clickables[i].direction, pos, 200, 70);

                break;
            case CLICKABLE_DIR_NW:
                drawTextCentred(rectangleCentre(textSubRec), GetFontDefault(), "NW", size, color);
                drawCardinalArrow(clickables[i].direction, pos, 160, 20);

                break;
            case CLICKABLE_DIR_E:
                drawTextCentred(rectangleCentre(textSubRec), GetFontDefault(), "E", size, color);
                drawCardinalArrow(clickables[i].direction, pos, 137, 45);

                break;
            case CLICKABLE_DIR_W:
                drawTextCentred(rectangleCentre(textSubRec), GetFontDefault(), "W", size, color);
                drawCardinalArrow(clickables[i].direction, pos, 80, 45);

                break;
            case CLICKABLE_DIR_SW:
                drawTextCentred(rectangleCentre(textSubRec), GetFontDefault(), "SW", size, color);
                drawCardinalArrow(clickables[i].direction, pos, 80, 45);


                break;

            default: GAME_ASSERT(false);
        }
    }

}


void tickClickables () {
    Vector2 mouse = getMousePosRel();

    // other mouse functions don't work on wasm! IsMouseButtonPressed(), IsMouseButtonReleased()
    // hacky button debounce logic
    bool wasLeftclickPressed = false;
    bool buttonDown = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
    lmbCooldown += GetFrameTime();
    if (buttonDown) {
        // assume if the user holds down the button, they want to spam it,
        // otherwise assume 0.5 seconds between valid clicks
        if (lmbCooldown >= lmbCooldownGoal) {
            wasLeftclickPressed = true;
            lmbCooldown = 0;
        }
    }


    for (int i = 0; i < clickableCount; i++) {
        bool isButtonHovered = CheckCollisionPointRec(mouse, clickables[i].position);
        if (isButtonHovered) {
            clickables[i].hovered = true;
        } else {
            clickables[i].hovered = false;
        }

        if (isButtonHovered && wasLeftclickPressed && clickables[i].tickCooldownCurrent) {
            clickables[i].tickCooldownCurrent = 0;
            movedHexagonsRecently = true;
            moveHexagons(clickables[i].direction);
        }
    }

    float frameMilis = ((float)GetFrameTime()) * 1000;

    for (int i = 0; i < clickableCount; i++) {
        if (clickables[i].tickCooldownCurrent < clickables[i].tickCooldownMax) {
            clickables[i].tickCooldownCurrent += frameMilis;
        }
    }

}
