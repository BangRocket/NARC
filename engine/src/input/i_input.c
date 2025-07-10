#include "../../include/i_input.h"
#include "../../include/i_system.h"
#include "raylib.h"
#include <string.h>

static void (*inputCallback)(int key, int action) = NULL;
static int lastMouseX = 0;
static int lastMouseY = 0;
static int mouseDeltaX = 0;
static int mouseDeltaY = 0;

void I_InitInput(void) {
    lastMouseX = GetMouseX();
    lastMouseY = GetMouseY();
    mouseDeltaX = 0;
    mouseDeltaY = 0;
    
    I_Log("Input system initialized");
}

void I_ShutdownInput(void) {
    inputCallback = NULL;
    I_Log("Input system shutdown");
}

void I_UpdateInput(void) {
    int currentMouseX = GetMouseX();
    int currentMouseY = GetMouseY();
    
    mouseDeltaX = currentMouseX - lastMouseX;
    mouseDeltaY = currentMouseY - lastMouseY;
    
    lastMouseX = currentMouseX;
    lastMouseY = currentMouseY;
    
    if (inputCallback) {
        for (int key = 0; key < 512; key++) {
            if (IsKeyPressed(key)) {
                inputCallback(key, 1);
            } else if (IsKeyReleased(key)) {
                inputCallback(key, 0);
            }
        }
    }
}

bool I_IsKeyPressed(keycode_t key) {
    return IsKeyPressed((int)key);
}

bool I_IsKeyDown(keycode_t key) {
    return IsKeyDown((int)key);
}

bool I_IsKeyReleased(keycode_t key) {
    return IsKeyReleased((int)key);
}

bool I_IsKeyUp(keycode_t key) {
    return IsKeyUp((int)key);
}

bool I_IsMouseButtonPressed(mouse_button_t button) {
    return IsMouseButtonPressed((int)button);
}

bool I_IsMouseButtonDown(mouse_button_t button) {
    return IsMouseButtonDown((int)button);
}

bool I_IsMouseButtonReleased(mouse_button_t button) {
    return IsMouseButtonReleased((int)button);
}

bool I_IsMouseButtonUp(mouse_button_t button) {
    return IsMouseButtonUp((int)button);
}

void I_GetMousePosition(int* x, int* y) {
    if (x) *x = GetMouseX();
    if (y) *y = GetMouseY();
}

void I_GetMouseDelta(int* dx, int* dy) {
    if (dx) *dx = mouseDeltaX;
    if (dy) *dy = mouseDeltaY;
}

void I_SetMousePosition(int x, int y) {
    SetMousePosition(x, y);
    lastMouseX = x;
    lastMouseY = y;
}

float I_GetMouseWheelMove(void) {
    return GetMouseWheelMove();
}

void I_ShowCursor(void) {
    ShowCursor();
}

void I_HideCursor(void) {
    HideCursor();
}

void I_DisableCursor(void) {
    DisableCursor();
}

void I_EnableCursor(void) {
    EnableCursor();
}

bool I_IsCursorHidden(void) {
    return IsCursorHidden();
}

bool I_IsGamepadAvailable(int gamepad) {
    return IsGamepadAvailable(gamepad);
}

bool I_IsGamepadButtonPressed(int gamepad, gamepad_button_t button) {
    return IsGamepadButtonPressed(gamepad, (int)button);
}

bool I_IsGamepadButtonDown(int gamepad, gamepad_button_t button) {
    return IsGamepadButtonDown(gamepad, (int)button);
}

bool I_IsGamepadButtonReleased(int gamepad, gamepad_button_t button) {
    return IsGamepadButtonReleased(gamepad, (int)button);
}

bool I_IsGamepadButtonUp(int gamepad, gamepad_button_t button) {
    return IsGamepadButtonUp(gamepad, (int)button);
}

float I_GetGamepadAxisMovement(int gamepad, gamepad_axis_t axis) {
    return GetGamepadAxisMovement(gamepad, (int)axis);
}

const char* I_GetGamepadName(int gamepad) {
    return GetGamepadName(gamepad);
}

void I_SetInputCallback(void (*callback)(int key, int action)) {
    inputCallback = callback;
}