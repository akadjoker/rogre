
double GetCurrentTime(void)
{
 return (double)SDL_GetTicks()/1000;
}

void EnableCursor() {SDL_ShowCursor(SDL_ENABLE);}       // Unlock cursor
void DisableCursor() {SDL_ShowCursor( SDL_DISABLE);}      // Lock cursor


// Enable only desired getures to be detected
void SetGesturesEnabled(unsigned int gestureFlags)
{
    GESTURES.enabledFlags = gestureFlags;
}



bool IsGestureDetected(int gesture)
{
    if ((GESTURES.enabledFlags & GESTURES.current) == gesture) return true;
    else return false;
}




// Get number of touch points
int GetTouchPointsCount(void)
{
    return GESTURES.Touch.pointCount;
}

// Get latest detected gesture
int GetGestureDetected(void)
{
    // Get current gesture only if enabled
    return (GESTURES.enabledFlags & GESTURES.current);
}

// Hold time measured in ms
float GetGestureHoldDuration(void)
{
    // NOTE: time is calculated on current gesture HOLD

    double time = 0.0;

    if (GESTURES.current == GESTURE_HOLD) time = GetCurrentTime() - GESTURES.Hold.timeDuration;

    return (float)time;
}

// Get drag vector (between initial touch point to current)
Vector2 GetGestureDragVector(void)
{
    // NOTE: drag vector is calculated on one touch points TOUCH_MOVE

    return GESTURES.Drag.vector;
}

// Get drag angle
// NOTE: Angle in degrees, horizontal-right is 0, counterclock-wise
float GetGestureDragAngle(void)
{
    // NOTE: drag angle is calculated on one touch points TOUCH_UP

    return GESTURES.Drag.angle;
}

// Get distance between two pinch points
Vector2 GetGesturePinchVector(void)
{
    // NOTE: The position values used for GESTURES.Pinch.distance are not modified like the position values of [core.c]-->GetTouchPosition(int index)
    // NOTE: pinch distance is calculated on two touch points TOUCH_MOVE

    return GESTURES.Pinch.vector;
}

// Get angle beween two pinch points
// NOTE: Angle in degrees, horizontal-right is 0, counterclock-wise
float GetGesturePinchAngle(void)
{
    // NOTE: pinch angle is calculated on two touch points TOUCH_MOVE

    return GESTURES.Pinch.angle;
}


//----------------------------------------------------------------------------------
// Module Functions Definition - Input (Keyboard, Mouse, Gamepad) Functions
//----------------------------------------------------------------------------------
// Detect if a key has been pressed once
bool IsKeyPressed(int key)
{
    bool pressed = false;

    if ((Input.Keyboard.previousKeyState[key] == 0) && (Input.Keyboard.currentKeyState[key] == 1)) pressed = true;
    else pressed = false;

    return pressed;
}
bool IsAnyKeyPressed()
{
    bool keyPressed = false;
    int key = GetKeyPressed();

    if ((key >= 32) && (key <= 126)) keyPressed = true;

    return keyPressed;
}

// Detect if a key is being pressed (key held down)
bool IsKeyDown(int key)
{
    if (Input.Keyboard.currentKeyState[key] == 1) return true;
    else return false;
}

// Detect if a key has been released once
bool IsKeyReleased(int key)
{
    bool released = false;

    if ((Input.Keyboard.previousKeyState[key] == 1) && (Input.Keyboard.currentKeyState[key] == 0)) released = true;
    else released = false;

    return released;
}

// Detect if a key is NOT being pressed (key not held down)
bool IsKeyUp(int key)
{
    if (Input.Keyboard.currentKeyState[key] == 0) return true;
    else return false;
}

// Get the last key pressed
int GetKeyPressed(void)
{
    int value = 0;

    if (Input.Keyboard.keyPressedQueueCount > 0)
    {
        // Get character from the queue head
        value = Input.Keyboard.keyPressedQueue[0];

        // Shift elements 1 step toward the head.
        for (int i = 0; i < (Input.Keyboard.keyPressedQueueCount - 1); i++)
            Input.Keyboard.keyPressedQueue[i] = Input.Keyboard.keyPressedQueue[i + 1];

        // Reset last character in the queue
        Input.Keyboard.keyPressedQueue[Input.Keyboard.keyPressedQueueCount] = 0;
        Input.Keyboard.keyPressedQueueCount--;
    }

    return value;
}

// Get the last char pressed
int GetCharPressed(void)
{
    int value = 0;

    if (Input.Keyboard.charPressedQueueCount > 0)
    {
        // Get character from the queue head
        value = Input.Keyboard.charPressedQueue[0];

        // Shift elements 1 step toward the head.
        for (int i = 0; i < (Input.Keyboard.charPressedQueueCount - 1); i++)
            Input.Keyboard.charPressedQueue[i] = Input.Keyboard.charPressedQueue[i + 1];

        // Reset last character in the queue
        Input.Keyboard.charPressedQueue[Input.Keyboard.charPressedQueueCount] = 0;
        Input.Keyboard.charPressedQueueCount--;
    }

    return value;
}




// Detect if a mouse button has been pressed once
bool IsMouseButtonPressed(int button)
{
    bool pressed = false;

    if ((Input.Mouse.currentButtonState[button] == 1) && (Input.Mouse.previousButtonState[button] == 0)) pressed = true;

    // Map touches to mouse buttons checking
    if ((Input.Touch.currentTouchState[button] == 1) && (Input.Touch.previousTouchState[button] == 0)) pressed = true;

    return pressed;
}

// Detect if a mouse button is being pressed
bool IsMouseButtonDown(int button)
{
    bool down = false;

    if (Input.Mouse.currentButtonState[button] == 1) down = true;

    // Map touches to mouse buttons checking
    if (Input.Touch.currentTouchState[button] == 1) down = true;

    return down;
}

// Detect if a mouse button has been released once
bool IsMouseButtonReleased(int button)
{
    bool released = false;

    if ((Input.Mouse.currentButtonState[button] == 0) && (Input.Mouse.previousButtonState[button] == 1)) released = true;

    // Map touches to mouse buttons checking
    if ((Input.Touch.currentTouchState[button] == 0) && (Input.Touch.previousTouchState[button] == 1)) released = true;

    return released;
}

// Detect if a mouse button is NOT being pressed
bool IsMouseButtonUp(int button)
{
    return !IsMouseButtonDown(button);
}

// Returns mouse position X
int GetMouseX(void)
{
#if defined(PLATFORM_ANDROID)
    return (int)Input.Touch.position[0].x;
#else
    return (int)((Input.Mouse.position.x + Input.Mouse.offset.x)*Input.Mouse.scale.x);
#endif
}

// Returns mouse position Y
int GetMouseY(void)
{
#if defined(PLATFORM_ANDROID)
    return (int)Input.Touch.position[0].y;
#else
    return (int)((Input.Mouse.position.y + Input.Mouse.offset.y)*Input.Mouse.scale.y);
#endif
}

// Returns mouse position XY
Vector2 GetMousePosition(void)
{
     Vector2 position;
     position.x=0;
     position.y=0;

#if defined(PLATFORM_ANDROID) || defined(PLATFORM_WEB)
    position = GetTouchPosition(0);
#else
    position.x = (Input.Mouse.position.x + Input.Mouse.offset.x)*Input.Mouse.scale.x;
    position.y = (Input.Mouse.position.y + Input.Mouse.offset.y)*Input.Mouse.scale.y;
#endif

    return position;
}

// Set mouse position XY
void SetMousePosition(int x, int y)
{
    Input.Mouse.position.x =x;
    Input.Mouse.position.y =y;


}

// Set mouse offset
// NOTE: Useful when rendering to different size targets
void SetMouseOffset(int offsetX, int offsetY)
{
    Input.Mouse.offset.x=offsetX;
    Input.Mouse.offset.y=offsetY;

}

// Set mouse scaling
// NOTE: Useful when rendering to different size targets
void SetMouseScale(float scaleX, float scaleY)
{
    Input.Mouse.scale.x =  scaleX;
    Input.Mouse.scale.y =  scaleY;

}

// Returns mouse wheel movement Y
float GetMouseWheelMove(void)
{
#if defined(PLATFORM_ANDROID)
    return 0.0f;
#elif defined(PLATFORM_WEB)
    return Input.Mouse.previousWheelMove/100.0f;
#else
    return Input.Mouse.previousWheelMove;
#endif
}


// Returns touch position X for touch point 0 (relative to screen size)
int GetTouchX(void)
{
#if defined(PLATFORM_ANDROID) || defined(PLATFORM_WEB)
    return (int)Input.Touch.position[0].x;
#else   // PLATFORM_DESKTOP, PLATFORM_RPI, PLATFORM_DRM
    return GetMouseX();
#endif
}

// Returns touch position Y for touch point 0 (relative to screen size)
int GetTouchY(void)
{
#if defined(PLATFORM_ANDROID) || defined(PLATFORM_WEB)
    return (int)Input.Touch.position[0].y;
#else   // PLATFORM_DESKTOP, PLATFORM_RPI, PLATFORM_DRM
    return GetMouseY();
#endif
}

Vector2 GetTouchPosition(int index)
{
    Vector2 position;
    position.x=-1;
    position.y=-1;

#if defined(PLATFORM_ANDROID) || defined(PLATFORM_WEB)
    if (index < MAX_TOUCH_POINTS) position = Input.Touch.position[index];

/*
    #if defined(PLATFORM_ANDROID)
    if ((CORE.Window.screen.width > CORE.Window.display.width) || (CORE.Window.screen.height > CORE.Window.display.height))
    {
        position.x = position.x*((float)CORE.Window.screen.width/(float)(CORE.Window.display.width - CORE.Window.renderOffset.x)) - CORE.Window.renderOffset.x/2;
        position.y = position.y*((float)CORE.Window.screen.height/(float)(CORE.Window.display.height - CORE.Window.renderOffset.y)) - CORE.Window.renderOffset.y/2;
    }
    else
    {
        position.x = position.x*((float)CORE.Window.render.width/(float)CORE.Window.display.width) - CORE.Window.renderOffset.x/2;
        position.y = position.y*((float)CORE.Window.render.height/(float)CORE.Window.display.height) - CORE.Window.renderOffset.y/2;
    }
    #endif
*/
#elif defined(PLATFORM_DESKTOP)
    if (index == 0) position = GetMousePosition();
#endif

    return position;
}
