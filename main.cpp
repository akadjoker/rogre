#include <exception>
#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>                 // Required for: srand(), rand(), atexit()
#include <stdio.h>                  // Required for: sprintf() [Used in OpenURL()]
#include <string.h>                 // Required for: strrchr(), strcmp(), strlen()
#include <time.h>                   // Required for: time() [Used in InitTimer()]
#include <math.h>                   // Required for: tan() [Used in BeginMode3D()]
#include <sys/stat.h>               // Required for: stat() [Used in GetFileModTime()]

#if defined(PLATFORM_DESKTOP) && defined(_WIN32) && (defined(_MSC_VER) || defined(__TINYC__))
    #define DIRENT_MALLOC SMALLOC
    #define DIRENT_FREE SFREE
    #include "external/dirent.h"    // Required for: DIR, opendir(), closedir() [Used in GetDirectoryFiles()]
#else
    #include <dirent.h>             // Required for: DIR, opendir(), closedir() [Used in GetDirectoryFiles()]
#endif




#if (defined(__linux__) || defined(PLATFORM_WEB)) && _POSIX_C_SOURCE < 199309L
    #undef _POSIX_C_SOURCE
    #define _POSIX_C_SOURCE 199309L // Required for CLOCK_MONOTONIC if compiled with c99 without gnu ext.
#endif



#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>  // Emscripten library - LLVM to JavaScript compiler
    #include <emscripten/html5.h>       // Emscripten HTML5 library
#endif




#if defined(_WIN32)
    #include <direct.h>             // Required for: _getch(), _chdir()
    #define GETCWD _getcwd          // NOTE: MSDN recommends not to use getcwd(), chdir()
    #define CHDIR _chdir
    #include <io.h>                 // Required for _access() [Used in FileExists()]
#else
    #include <unistd.h>             // Required for: getch(), chdir() (POSIX), access()
    #define GETCWD getcwd
    #define CHDIR chdir
#endif

#if defined(__linux__)
    #define MAX_FILEPATH_LENGTH     4096        // Maximum length for filepaths (Linux PATH_MAX default value)
#else
    #define MAX_FILEPATH_LENGTH      512        // Maximum length supported for filepaths
#endif



#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL.h>

#include <SDL2/SDL_syswm.h>



#include <OgreCamera.h>
#include <OgreEntity.h>
#include <OgreSceneNode.h>
#include <OgreFrameListener.h>
#include <OgreLogManager.h>
#include <OgreRoot.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>
#include <OgreConfigFile.h>
#include <OgreMaterialManager.h>
#include <OgreTextureManager.h>
//#include <OgreOverlaySystem.h>
#include <OgreMeshManager.h>
#include <OgreRTShaderSystem.h>
#include <OgreMovablePlane.h>
#include <OgreFrameListener.h>

#include <OgrePageManager.h>
#include <OgreTerrain.h>
#include <OgreTerrainGroup.h>
#include <OgreTerrainQuadTreeNode.h>
#include <OgreTerrainMaterialGeneratorA.h>
#include <OgreTerrainPaging.h>
using namespace std;
//using namespace Ogre;

// TraceLog message types
typedef enum {
    LOG_ALL,
    LOG_TRACE,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_FATAL,
    LOG_NONE
} TraceLogLevel;

#define TRACELOG(level, ...) SDL_LogInfo(0, __VA_ARGS__)
#define TRACELOGD(...) SDL_LogInfo(0, __VA_ARGS__)


// Allow custom memory allocators
#define RL_MALLOC(sz)       malloc(sz)
#define RL_CALLOC(n,sz)     calloc(n,sz)
#define RL_REALLOC(n,sz)    realloc(n,sz)
#define RL_FREE(p)          free(p)
#define SMALLOC(sz)       malloc(sz)
#define SCALLOC(n,sz)     calloc(n,sz)
#define SREALLOC(ptr,sz)  realloc(ptr,sz)
#define SFREE(ptr)        free(ptr)

#define RAYMATH_STANDALONE
#define RAYMATH_HEADER_ONLY
#include "raymath.h"            // Required for: Vector3, Matrix


#define RLGL_STANDALONE
#define RLGL_SUPPORT_TRACELOG
#define GRAPHICS_API_OPENGL_33
#include "rogre.h"
#include "raygui.h"
#define MSF_GIF_IMPL
#include "msf_gif.h"   // Support GIF recording




typedef struct InputData {



   Size size;
        struct {
            int exitKey;                    // Default exit key
            char currentKeyState[512];      // Registers current frame key state
            char previousKeyState[512];     // Registers previous frame key state
            int keyPressedQueue[MAX_KEY_PRESSED_QUEUE];     // Input keys queue
            int keyPressedQueueCount;       // Input keys queue count
            int charPressedQueue[MAX_CHAR_PRESSED_QUEUE];   // Input characters queue
            int charPressedQueueCount;      // Input characters queue count


        } Keyboard;
        struct {
            Vector2 position;               // Mouse position on screen
            Vector2 offset;                 // Mouse offset
            Vector2 scale;                  // Mouse scaling

            char currentButtonState[3];     // Registers current mouse button state
            char previousButtonState[3];    // Registers previous mouse button state
            float currentWheelMove;         // Registers current mouse wheel variation
            float previousWheelMove;        // Registers previous mouse wheel variation

        } Mouse;
        struct {
            Vector2 position[MAX_TOUCH_POINTS];         // Touch position on screen
            char currentTouchState[MAX_TOUCH_POINTS];   // Registers current touch state
            char previousTouchState[MAX_TOUCH_POINTS];  // Registers previous touch state
        } Touch;



} InputData;




static InputData Input = { 0 };



//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define FORCE_TO_SWIPE      0.0005f     // Swipe force, measured in normalized screen units/time
#define MINIMUM_DRAG        0.015f      // Drag minimum force, measured in normalized screen units (0.0f to 1.0f)
#define MINIMUM_PINCH       0.005f      // Pinch minimum force, measured in normalized screen units (0.0f to 1.0f)
#define TAP_TIMEOUT         300         // Tap minimum time, measured in milliseconds
#define PINCH_TIMEOUT       300         // Pinch minimum time, measured in milliseconds
#define DOUBLETAP_RANGE     0.03f       // DoubleTap range, measured in normalized screen units (0.0f to 1.0f)

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------

// Gestures module state context [136 bytes]
typedef struct {
    unsigned int current;               // Current detected gesture
    unsigned int enabledFlags;          // Enabled gestures flags
    struct {
        int firstId;                    // Touch id for first touch point
        int pointCount;                 // Touch points counter
        double eventTime;               // Time stamp when an event happened
        Vector2 upPosition;             // Touch up position
        Vector2 downPositionA;          // First touch down position
        Vector2 downPositionB;          // Second touch down position
        Vector2 downDragPosition;       // Touch drag position
        Vector2 moveDownPositionA;      // First touch down position on move
        Vector2 moveDownPositionB;      // Second touch down position on move
        int tapCounter;                 // TAP counter (one tap implies TOUCH_DOWN and TOUCH_UP actions)
    } Touch;
    struct {
        bool resetRequired;             // HOLD reset to get first touch point again
        double timeDuration;            // HOLD duration in milliseconds
    } Hold;
    struct {
        Vector2 vector;                 // DRAG vector (between initial and current position)
        float angle;                    // DRAG angle (relative to x-axis)
        float distance;                 // DRAG distance (from initial touch point to final) (normalized [0..1])
        float intensity;                // DRAG intensity, how far why did the DRAG (pixels per frame)
    } Drag;
    struct {
        bool start;                     // SWIPE used to define when start measuring GESTURES.Swipe.timeDuration
        double timeDuration;            // SWIPE time to calculate drag intensity
    } Swipe;
    struct {
        Vector2 vector;                 // PINCH vector (between first and second touch points)
        float angle;                    // PINCH angle (relative to x-axis)
        float distance;                 // PINCH displacement distance (normalized [0..1])
    } Pinch;
} GesturesData;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static GesturesData GESTURES ;

static float VecAngle(Vector2 v1, Vector2 v2)
{
    float angle = atan2f(v2.y - v1.y, v2.x - v1.x)*(180.0f/3.14159265358979323846);

    if (angle < 0) angle += 360.0f;

    return angle;
}

// Calculate distance between two Vector2
static float VecDistance(Vector2 v1, Vector2 v2)
{
    float result;

    float dx = v2.x - v1.x;
    float dy = v2.y - v1.y;

    result = (float)sqrt(dx*dx + dy*dy);

    return result;
}


// Process gesture event and translate it into gestures
void ProcessGestureEvent(GestureEvent event)
{
    // Reset required variables
    GESTURES.Touch.pointCount = event.pointCount;      // Required on UpdateGestures()

    if (GESTURES.Touch.pointCount < 2)
    {
        if (event.touchAction == TOUCH_DOWN)
        {
            GESTURES.Touch.tapCounter++;    // Tap counter

            // Detect GESTURE_DOUBLE_TAP
            if ((GESTURES.current == GESTURE_NONE) && (GESTURES.Touch.tapCounter >= 2) && ((GetCurrentTime() - GESTURES.Touch.eventTime) < TAP_TIMEOUT) && (VecDistance(GESTURES.Touch.downPositionA, event.position[0]) < DOUBLETAP_RANGE))
            {
                GESTURES.current = GESTURE_DOUBLETAP;
                GESTURES.Touch.tapCounter = 0;
            }
            else    // Detect GESTURE_TAP
            {
                GESTURES.Touch.tapCounter = 1;
                GESTURES.current = GESTURE_TAP;
            }

            GESTURES.Touch.downPositionA = event.position[0];
            GESTURES.Touch.downDragPosition = event.position[0];

            GESTURES.Touch.upPosition = GESTURES.Touch.downPositionA;
            GESTURES.Touch.eventTime = GetCurrentTime();

            GESTURES.Touch.firstId = event.pointerId[0];

            GESTURES.Drag.vector = (Vector2){ 0.0f, 0.0f };
        }
        else if (event.touchAction == TOUCH_UP)
        {
            if (GESTURES.current == GESTURE_DRAG) GESTURES.Touch.upPosition = event.position[0];

            // NOTE: GESTURES.Drag.intensity dependend on the resolution of the screen
            GESTURES.Drag.distance = VecDistance(GESTURES.Touch.downPositionA, GESTURES.Touch.upPosition);
            GESTURES.Drag.intensity = GESTURES.Drag.distance/(float)((GetCurrentTime() - GESTURES.Swipe.timeDuration));

            GESTURES.Swipe.start = false;

            // Detect GESTURE_SWIPE
            if ((GESTURES.Drag.intensity > FORCE_TO_SWIPE) && (GESTURES.Touch.firstId == event.pointerId[0]))
            {
                // NOTE: Angle should be inverted in Y
                GESTURES.Drag.angle = 360.0f - VecAngle(GESTURES.Touch.downPositionA, GESTURES.Touch.upPosition);

                if ((GESTURES.Drag.angle < 30) || (GESTURES.Drag.angle > 330)) GESTURES.current = GESTURE_SWIPE_RIGHT;        // Right
                else if ((GESTURES.Drag.angle > 30) && (GESTURES.Drag.angle < 120)) GESTURES.current = GESTURE_SWIPE_UP;      // Up
                else if ((GESTURES.Drag.angle > 120) && (GESTURES.Drag.angle < 210)) GESTURES.current = GESTURE_SWIPE_LEFT;   // Left
                else if ((GESTURES.Drag.angle > 210) && (GESTURES.Drag.angle < 300)) GESTURES.current = GESTURE_SWIPE_DOWN;   // Down
                else GESTURES.current = GESTURE_NONE;
            }
            else
            {
                GESTURES.Drag.distance = 0.0f;
                GESTURES.Drag.intensity = 0.0f;
                GESTURES.Drag.angle = 0.0f;

                GESTURES.current = GESTURE_NONE;
            }

            GESTURES.Touch.downDragPosition = (Vector2){ 0.0f, 0.0f };
            GESTURES.Touch.pointCount = 0;
        }
        else if (event.touchAction == TOUCH_MOVE)
        {
            if (GESTURES.current == GESTURE_DRAG) GESTURES.Touch.eventTime = GetCurrentTime();

            if (!GESTURES.Swipe.start)
            {
                GESTURES.Swipe.timeDuration = GetCurrentTime();
                GESTURES.Swipe.start = true;
            }

            GESTURES.Touch.moveDownPositionA = event.position[0];

            if (GESTURES.current == GESTURE_HOLD)
            {
                if (GESTURES.Hold.resetRequired) GESTURES.Touch.downPositionA = event.position[0];

                GESTURES.Hold.resetRequired = false;

                // Detect GESTURE_DRAG
                if (VecDistance(GESTURES.Touch.downPositionA, GESTURES.Touch.moveDownPositionA) >= MINIMUM_DRAG)
                {
                    GESTURES.Touch.eventTime = GetCurrentTime();
                    GESTURES.current = GESTURE_DRAG;
                }
            }

            GESTURES.Drag.vector.x = GESTURES.Touch.moveDownPositionA.x - GESTURES.Touch.downDragPosition.x;
            GESTURES.Drag.vector.y = GESTURES.Touch.moveDownPositionA.y - GESTURES.Touch.downDragPosition.y;
        }
    }
    else    // Two touch points
    {
        if (event.touchAction == TOUCH_DOWN)
        {
            GESTURES.Touch.downPositionA = event.position[0];
            GESTURES.Touch.downPositionB = event.position[1];

            //GESTURES.Pinch.distance = VecDistance(GESTURES.Touch.downPositionA, GESTURES.Touch.downPositionB);

            GESTURES.Pinch.vector.x = GESTURES.Touch.downPositionB.x - GESTURES.Touch.downPositionA.x;
            GESTURES.Pinch.vector.y = GESTURES.Touch.downPositionB.y - GESTURES.Touch.downPositionA.y;

            GESTURES.current = GESTURE_HOLD;
            GESTURES.Hold.timeDuration = GetCurrentTime();
        }
        else if (event.touchAction == TOUCH_MOVE)
        {
            GESTURES.Pinch.distance = VecDistance(GESTURES.Touch.moveDownPositionA, GESTURES.Touch.moveDownPositionB);

            GESTURES.Touch.downPositionA = GESTURES.Touch.moveDownPositionA;
            GESTURES.Touch.downPositionB = GESTURES.Touch.moveDownPositionB;

            GESTURES.Touch.moveDownPositionA = event.position[0];
            GESTURES.Touch.moveDownPositionB = event.position[1];

            GESTURES.Pinch.vector.x = GESTURES.Touch.moveDownPositionB.x - GESTURES.Touch.moveDownPositionA.x;
            GESTURES.Pinch.vector.y = GESTURES.Touch.moveDownPositionB.y - GESTURES.Touch.moveDownPositionA.y;

            if ((VecDistance(GESTURES.Touch.downPositionA, GESTURES.Touch.moveDownPositionA) >= MINIMUM_PINCH) || (VecDistance(GESTURES.Touch.downPositionB, GESTURES.Touch.moveDownPositionB) >= MINIMUM_PINCH))
            {
                if ((VecDistance(GESTURES.Touch.moveDownPositionA, GESTURES.Touch.moveDownPositionB) - GESTURES.Pinch.distance) < 0) GESTURES.current = GESTURE_PINCH_IN;
                else GESTURES.current = GESTURE_PINCH_OUT;
            }
            else
            {
                GESTURES.current = GESTURE_HOLD;
                GESTURES.Hold.timeDuration = GetCurrentTime();
            }

            // NOTE: Angle should be inverted in Y
            GESTURES.Pinch.angle = 360.0f - VecAngle(GESTURES.Touch.moveDownPositionA, GESTURES.Touch.moveDownPositionB);
        }
        else if (event.touchAction == TOUCH_UP)
        {
            GESTURES.Pinch.distance = 0.0f;
            GESTURES.Pinch.angle = 0.0f;
            GESTURES.Pinch.vector = (Vector2){ 0.0f, 0.0f };
            GESTURES.Touch.pointCount = 0;

            GESTURES.current = GESTURE_NONE;
        }
    }
}


// Update gestures detected (must be called every frame)
void UpdateGestures(void)
{
    // NOTE: Gestures are processed through system callbacks on touch events

    // Detect GESTURE_HOLD
    if (((GESTURES.current == GESTURE_TAP) || (GESTURES.current == GESTURE_DOUBLETAP)) && (GESTURES.Touch.pointCount < 2))
    {
        GESTURES.current = GESTURE_HOLD;
        GESTURES.Hold.timeDuration = GetCurrentTime();
    }

    if (((GetCurrentTime() - GESTURES.Touch.eventTime) > TAP_TIMEOUT) && (GESTURES.current == GESTURE_DRAG) && (GESTURES.Touch.pointCount < 2))
    {
        GESTURES.current = GESTURE_HOLD;
        GESTURES.Hold.timeDuration = GetCurrentTime();
        GESTURES.Hold.resetRequired = true;
    }

    // Detect GESTURE_NONE
    if ((GESTURES.current == GESTURE_SWIPE_RIGHT) || (GESTURES.current == GESTURE_SWIPE_UP) || (GESTURES.current == GESTURE_SWIPE_LEFT) || (GESTURES.current == GESTURE_SWIPE_DOWN))
    {
        GESTURES.current = GESTURE_NONE;
    }
}



 void KeyCallback( int key, int scancode, int action, int mods)
{


  //  SDL_Log( "Key Callback: KEY:%i(%c) SDL KEY:%i(%c) - SCANCODE:%i (STATE:%i)", key, key,index,index, scancode, action);
      // int keycode =get_sdl_to_glfw(event->key.keysym.scancode);
       //if(core.key)  core.key(core.window,keycode,event->key.keysym.sym,GLFW_PRESS,event->key.keysym.mod);


        // WARNING: GLFW could return GLFW_REPEAT, we need to consider it as 1
        // to work properly with our implementation (IsKeyDown/IsKeyUp checks)
        if (action == 0) Input.Keyboard.currentKeyState[key] = 0;
        else Input.Keyboard.currentKeyState[key] = 1;


        // Check if there is space available in the key queue
        if ((Input.Keyboard.keyPressedQueueCount < MAX_KEY_PRESSED_QUEUE) && (action == 0))
        {
            // Add character to the queue
            Input.Keyboard.keyPressedQueue[Input.Keyboard.keyPressedQueueCount] = key;
            Input.Keyboard.keyPressedQueueCount++;
        }


}


 void CharCallback( unsigned int key)
{
   // SDL_Log( "Char Callback: KEY:%i(%c)", key, key);
    if (Input.Keyboard.charPressedQueueCount < MAX_KEY_PRESSED_QUEUE)
    {
        Input.Keyboard.charPressedQueue[Input.Keyboard.charPressedQueueCount] = key;
        Input.Keyboard.charPressedQueueCount++;
    }
}




 void MouseButtonCallback( int button, int action, int mods)
{
    // WARNING: GLFW could only return GLFW_PRESS (1) or GLFW_RELEASE (0) for now,
    // but future releases may add more actions (i.e. GLFW_REPEAT)
    Input.Mouse.currentButtonState[button] = action;


    // Process mouse events as touches to be able to use mouse-gestures
    GestureEvent gestureEvent = { 0 };

    // Register touch actions
    if ((Input.Mouse.currentButtonState[button] == 1) && (Input.Mouse.previousButtonState[button] == 0)) gestureEvent.touchAction = TOUCH_DOWN;
    else if ((Input.Mouse.currentButtonState[button] == 0) && (Input.Mouse.previousButtonState[button] == 1)) gestureEvent.touchAction = TOUCH_UP;

    // NOTE: TOUCH_MOVE event is registered in MouseCursorPosCallback()

    // Assign a pointer ID
    gestureEvent.pointerId[0] = 0;

    // Register touch points count
    gestureEvent.pointCount = 1;

    // Register touch points position, only one point registered
    gestureEvent.position[0].x = GetMousePosition().x;
    gestureEvent.position[0].y = GetMousePosition().y;

     Size s =Input.size;
    // Normalize gestureEvent.position[0] for CORE.Window.screen.width and CORE.Window.screen.height
    gestureEvent.position[0].x /= (float)s.width;
    gestureEvent.position[0].y /= (float)s.height;

    // Gesture data is sent to gestures system for processing
    ProcessGestureEvent(gestureEvent);

}


 void MouseCursorPosCallback( double x, double y)
{
    Input.Mouse.position.x = (float)x;
    Input.Mouse.position.y = (float)y;
    Input.Touch.position[0] = Input.Mouse.position;


    // Process mouse events as touches to be able to use mouse-gestures
    GestureEvent gestureEvent = { 0 };

    gestureEvent.touchAction = TOUCH_MOVE;

    // Assign a pointer ID
    gestureEvent.pointerId[0] = 0;

    // Register touch points count
    gestureEvent.pointCount = 1;

    // Register touch points position, only one point registered
    gestureEvent.position[0].x = Input.Touch.position[0].x;
    gestureEvent.position[0].y = Input.Touch.position[0].y;


     Size s =Input.size;
    // Normalize gestureEvent.position[0] for CORE.Window.screen.width and CORE.Window.screen.height
    gestureEvent.position[0].x /= (float)s.width;
    gestureEvent.position[0].y /= (float)s.height;


    // Gesture data is sent to gestures system for processing
    ProcessGestureEvent(gestureEvent);

}

/*
Ogre::Euler QuaternionToEuler(Ogre::Quaternion quat)
{
   Ogre::Matrix3 mat;
   Ogre::Radian yRad, pRad, rRad;

   quat.ToRotationMatrix(mat);
   mat.ToEulerAnglesYXZ(yRad, pRad, rRad);

   return Euler(yRad, pRad, rRad);
}

Ogre::Quaternion EulerToQuaternion(Ogre::Real yaw, Ogre::Real pitch, Ogre::Real roll)
{
   Ogre::Matrix3 mat;
   Ogre::Quaternion quat;

   mat.FromEulerAnglesYXZ(Ogre::Degree(yaw), Ogre::Degree(pitch), Ogre::Degree(roll));
   quat.FromRotationMatrix(mat);

   return quat;
}
*/

#define TRY try {
#define CATCH(func) } catch(Ogre::Exception& e) { LogError(e.what()); } catch(std::exception& e) { LogError(e.what()); }  catch(...) { LogError("An unknown error has occurred in " + Ogre::String(func) + "!"); }



void DisplayError(Ogre::String error_msg)
{
  // MessageBoxA(NULL, error_msg.c_str(), "rOgre3D", MB_OK | MB_ICONERROR | MB_TASKMODAL);
}

void LogError(Ogre::String error_msg)
{
   Ogre::LogManager::getSingleton().getDefaultLog()->logMessage(error_msg, Ogre::LML_CRITICAL);
}

void LogNormal(Ogre::String error_msg)
{
   Ogre::LogManager::getSingleton().getDefaultLog()->logMessage(error_msg, Ogre::LML_NORMAL);
}

void LogTrivial(Ogre::String error_msg)
{
   Ogre::LogManager::getSingleton().getDefaultLog()->logMessage(error_msg, Ogre::LML_TRIVIAL);
}

template<typename T>
T ConvertFromPointer(double ptr)
{
   T obj = reinterpret_cast<T>(static_cast<intptr_t>(ptr));

   if (obj == NULL)
      LogError("Invalid object ID passed!");

   return obj;
}

template<typename T>
double ConvertToPointer(T ptr)
{
   return static_cast<double>(reinterpret_cast<intptr_t>(ptr));
}

SDL_Window* window;

Ogre::Root*                 mRoot;

//Ogre::Camera *camera;
//Ogre::Viewport *mViewport;
//Ogre::SceneManager *scene;
Ogre::RenderWindow*         mWindow;
int mWindowHeight=0;
int mWindowWidth=0;


// Get current screen width
Size GetWindowSize(void)
{
    Size s;
    SDL_GetWindowSize(window,&s.width,&s.height);
    return s;
}


static int screenshotCounter = 0;           // Screenshots counter
static int gifFramesCounter = 0;            // GIF frames counter
static bool gifRecording = false;           // GIF recording state
static MsfGifState gifState = { 0 };        // MSGIF context state

void screenRecod()
{
            if (gifRecording)
            {
                gifRecording = false;
                MsfGifResult result = msf_gif_end(&gifState);
                char path[512] = { 0 };
                strcpy(path, TextFormat("./screenrec%03i.gif", screenshotCounter));                            SaveFileData(path, result.data, (unsigned int)result.dataSize);
                msf_gif_free(result);
                SDL_Log( "SYSTEM: Finish animated GIF recording");
            }
            else
            {
                gifRecording = true;
                gifFramesCounter = 0;
                msf_gif_begin(&gifState, mWindow->getWidth(),mWindow->getHeight());
                screenshotCounter++;
                SDL_Log( "SYSTEM: Start animated GIF recording: %s", TextFormat("screenrec%03i.gif", screenshotCounter));
            }
}


Ogre::Image GetScreenData(int width,int height)
{
    unsigned char *imgData =  rlReadScreenPixels(width, height);
    Ogre::Image image= Ogre::Image(Ogre::PF_A8B8G8R8,width,height,1,imgData);
    return image;
}

void writeRenderToFile(const Ogre::String& filename)
{
    Ogre::Image img(Ogre::PF_A8B8G8R8, mWindow->getWidth(), mWindow->getHeight());
    Ogre::PixelBox pb = img.getPixelBox();
    mWindow->copyContentsToMemory(pb, pb);
    img.save(filename);
}
void TakeScreenshot(const char *fileName)
{
 mWindow->writeContentsToFile(fileName);
SDL_LogInfo(0, "SYSTEM: [%s] Screenshot taken successfully", fileName);
}



void InitInput()
{




    Input.Mouse.scale.x=1;
    Input.Mouse.scale.y=1;

    Input.Mouse.offset.x=1;
    Input.Mouse.offset.y=1;

    Input.Mouse.position.x = 0;
    Input.Mouse.position.y = 0;

    GESTURES.Touch.firstId = -1;
    GESTURES.current = GESTURE_NONE;
    GESTURES.enabledFlags = 0b0000001111111111;

    Size s =GetWindowSize();

    Input.size.width=s.width;
    Input.size.height=s.height;




}




// Poll (store) all input events
void ResetInput()
{
 UpdateGestures();


    // Register previous keys states
    for (int i = 0; i < 512; i++) Input.Keyboard.previousKeyState[i] = Input.Keyboard.currentKeyState[i];

    // Register previous mouse states
    for (int i = 0; i < 3; i++) Input.Mouse.previousButtonState[i] = Input.Mouse.currentButtonState[i];

    // Register previous mouse wheel state
    Input.Mouse.previousWheelMove = Input.Mouse.currentWheelMove;
    Input.Mouse.currentWheelMove = 0.0f;


    // Register previous touch states
    for (int i = 0; i < MAX_TOUCH_POINTS; i++) Input.Touch.previousTouchState[i] = Input.Touch.currentTouchState[i];

}

void PollInputEvents(const SDL_Event* e)
{




                    switch(e->type)
                    {
                            case SDL_WINDOWEVENT:
                            {


                                if (e->window.event == SDL_WINDOWEVENT_RESIZED)
                                {


                                    Input.size.width=e->window.data1;
                                    Input.size.height=e->window.data2;
                                 break;
                                }

                            }
                            break;
                       case SDL_KEYDOWN:
                       {
                            KeyCallback(e->key.keysym.scancode,e->key.keysym.sym,1,e->key.keysym.mod);
                            CharCallback(e->key.keysym.sym);

                       }
                       break;
                       case SDL_KEYUP:
                       {
                       KeyCallback(e->key.keysym.scancode,e->key.keysym.sym,0,e->key.keysym.mod);
                       }
                       break;
                       case SDL_TEXTINPUT:
                       {
                       }
                       break;
                        case SDL_MOUSEWHEEL:
                        {
                            Input.Mouse.previousWheelMove = e->wheel.y;
                        }
                        break;
                        case SDL_MOUSEBUTTONDOWN:
                        {

                         MouseButtonCallback(e->button.button-1,1,0);
                         MouseCursorPosCallback(e->button.x,e->button.y);


                        }
                        break;
                        case SDL_MOUSEBUTTONUP:
                        {
                         MouseButtonCallback(e->button.button-1,0,0);
                         MouseCursorPosCallback(e->button.x,e->button.y);


                        }
                        break;
                        case SDL_MOUSEMOTION:
                        {

                         MouseCursorPosCallback(e->button.x,e->button.y);

                        }
                        break;

                        case SDL_MULTIGESTURE:
                            {
                            float touchLocation_x;
                            float touchLocation_y;


                               Size s =Input.size;


                                 int b=(int) e->tfinger.fingerId;
                                if( fabs( e->mgesture.dTheta ) > 3.14 / 180.0 )
                                {
                                    touchLocation_x = e->mgesture.x * (float)s.width;
                                    touchLocation_y =e->mgesture.y * (float)s.height;


//
                                }
                                 //Pinch detected
                                else if( fabs( e->mgesture.dDist ) > 0.002 )
                                {
                                    touchLocation_x = e->mgesture.x * (float)s.width;
                                    touchLocation_y = e->mgesture.y * (float)s.height;

                                    //Pinch open
                                    if( e->mgesture.dDist > 0 )
                                    {

                                    }
                                    //Pinch close
                                    else
                                    {


                                    }

                                }

                            }
                        break;
                        case SDL_FINGERDOWN:
                            {
                              Size s =Input.size;


                            int b=(int) e->tfinger.fingerId;
                            float x= (float)e->tfinger.x * s.width;
                            float y= (float)e->tfinger.y * s.height;


                            }
                        break;
                        case SDL_FINGERUP:
                            {
                              Size s =Input.size;


                            int b=(int) e->tfinger.fingerId;
                            float x= (float)e->tfinger.x * s.width;
                            float y= (float)e->tfinger.y * s.height;


                            }
                        break;
                        case SDL_FINGERMOTION:
                            {
                         Size s =Input.size;
                            float x= (float)e->tfinger.x *s.width;
                            float y= (float)e->tfinger.y * s.height;

                            }
                        break;
                   }



}


#define TERRAIN_PAGE_MIN_X 0
#define TERRAIN_PAGE_MIN_Y 0
#define TERRAIN_PAGE_MAX_X 0
#define TERRAIN_PAGE_MAX_Y 0
#define TERRAIN_FILE_PREFIX Ogre::String("rOgreTerrain")
#define TERRAIN_FILE_SUFFIX Ogre::String("dat")
#define TERRAIN_WORLD_SIZE 12000.0f
#define TERRAIN_SIZE 513

#define SHADOWS_IN_LOW_LOD_MATERIAL false

class rTerrain
{
   public:
   rTerrain()
        : mTerrainGlobals(0)
        , mTerrainGroup(0)
        , mLayerEdit(1)
        , mBrushSizeTerrainSpace(0.02)
        , mHeightUpdateCountDown(0)
        , mTerrainPos(1000,0,5000)
        , mTerrainsImported(false)

   {};
   virtual ~rTerrain()
   {};

   void shutdown()
    {
        if(mTerrainGroup)
        {
            OGRE_DELETE mTerrainGroup;
            mTerrainGroup = 0;
        }

        if (mTerrainGlobals)
        {
            OGRE_DELETE mTerrainGlobals;
            mTerrainGlobals = 0;
        }


    }



    void setupContent(Ogre::SceneManager* mSceneMgr,Ogre::Light* l)
    {
       using namespace Ogre;

       SDL_LogInfo(0,"[TERRAIN ] Init");

        mTerrainGlobals = new Ogre::TerrainGlobalOptions();
        MaterialManager::getSingleton().setDefaultTextureFiltering(TFO_ANISOTROPIC);
        MaterialManager::getSingleton().setDefaultAnisotropy(7);


        SDL_LogInfo(0,"[TERRAIN ] Create Group");

        //! [terrain_create]
        mTerrainGroup = new Ogre::TerrainGroup(mSceneMgr, Ogre::Terrain::ALIGN_X_Z, TERRAIN_SIZE, TERRAIN_WORLD_SIZE);
        mTerrainGroup->setFilenameConvention(TERRAIN_FILE_PREFIX, TERRAIN_FILE_SUFFIX);
        mTerrainGroup->setOrigin(mTerrainPos);
        //! [terrain_create]


        SDL_LogInfo(0,"[TERRAIN ] Configure");
        configureTerrainDefaults(mSceneMgr,l);


        SDL_LogInfo(0,"[TERRAIN ] Build Montains");
        //! [define_loop]
        for (long x = TERRAIN_PAGE_MIN_X; x <= TERRAIN_PAGE_MAX_X; ++x)
            for (long y = TERRAIN_PAGE_MIN_Y; y <= TERRAIN_PAGE_MAX_Y; ++y)
                defineTerrain(x, y);
        // sync load since we want everything in place when we start
        SDL_LogInfo(0,"[TERRAIN ] Load All");
        mTerrainGroup->loadAllTerrains(false);
        //! [define_loop]


         while  (mTerrainGroup->isDerivedDataUpdateInProgress())
        {
                SDL_LogInfo(0,"Building terrain, please wait...");
                SDL_Delay(1);
        }

        //! [init_blend]
        SDL_LogInfo(0,"[TERRAIN ] Init Blend");
        if (mTerrainsImported)
        {
            for (const auto& ti : mTerrainGroup->getTerrainSlots())
            {
             //   initBlendMaps(ti.second->instance);
            }
        }

        SDL_LogInfo(0,"[TERRAIN ] Free temprorary resources");
       // mTerrainGroup->freeTemporaryResources();





    }


    void getTerrainImage(bool flipX, bool flipY, Ogre::Image& img)
    {
        //! [heightmap]
        SDL_LogInfo(0,"[TERRAIN ] Load heightmap");
        img.load("terrain.png", mTerrainGroup->getResourceGroup());
        if (flipX)
            img.flipAroundY();
        if (flipY)
            img.flipAroundX();
        //! [heightmap]
    }
       void defineTerrain(long x, long y, bool flat = false)
    {
        // if a file is available, use it
        // if not, generate file from import

        // Usually in a real project you'll know whether the compact terrain data is
        // available or not; I'm doing it this way to save distribution size

        if (flat)
        {
            mTerrainGroup->defineTerrain(x, y, 0.0f);
            return;
        }

        //! [define]
        Ogre::String filename = mTerrainGroup->generateFilename(x, y);
        if (Ogre::ResourceGroupManager::getSingleton().resourceExists(mTerrainGroup->getResourceGroup(), filename))
        {
            mTerrainGroup->defineTerrain(x, y);
        }
        else
        {
            Ogre::Image img;
            getTerrainImage(x % 2 != 0, y % 2 != 0, img);
            mTerrainGroup->defineTerrain(x, y, &img);
            mTerrainsImported = true;
        }
        //! [define]
    }


     void initBlendMaps(Ogre::Terrain* terrain)
    {
        //! [blendmap]
        using namespace Ogre;
        SDL_LogInfo(0,"[TERRAIN ] Gnerate blend 0");
        TerrainLayerBlendMap* blendMap0 = terrain->getLayerBlendMap(1);
        SDL_LogInfo(0,"[TERRAIN ] Gnerate blend 1");
        TerrainLayerBlendMap* blendMap1 = terrain->getLayerBlendMap(2);
        float minHeight0 = 20;
        float fadeDist0 = 15;
        float minHeight1 = 70;
        float fadeDist1 = 15;
        float* pBlend0 = blendMap0->getBlendPointer();
        float* pBlend1 = blendMap1->getBlendPointer();
        for (uint16 y = 0; y < terrain->getLayerBlendMapSize(); ++y)
        {
            for (uint16 x = 0; x < terrain->getLayerBlendMapSize(); ++x)
            {
                Real tx, ty;

                blendMap0->convertImageToTerrainSpace(x, y, &tx, &ty);
                float height = terrain->getHeightAtTerrainPosition(tx, ty);

                *pBlend0++ = Math::saturate((height - minHeight0) / fadeDist0);
                *pBlend1++ = Math::saturate((height - minHeight1) / fadeDist1);
            }
        }
        blendMap0->dirty();
        blendMap1->dirty();
        blendMap0->update();
        blendMap1->update();
    }
     void configureTerrainDefaults(Ogre::SceneManager* mSceneMgr,Ogre::Light* l)
    {
        //! [configure_lod]
        mTerrainGlobals->setMaxPixelError(8);
        mTerrainGlobals->setCompositeMapDistance(3000);
        //! [configure_lod]

        //mTerrainGlobals->setUseRayBoxDistanceCalculation(true);
        //mTerrainGlobals->getDefaultMaterialGenerator()->setDebugLevel(1);
        //mTerrainGlobals->setLightMapSize(256);

        // Disable the lightmap for OpenGL ES 2.0. The minimum number of samplers allowed is 8(as opposed to 16 on desktop).
        // Otherwise we will run over the limit by just one. The minimum was raised to 16 in GL ES 3.0.
        if (Ogre::Root::getSingletonPtr()->getRenderSystem()->getCapabilities()->getNumTextureUnits() < 9)
        {
          //  Ogre::TerrainMaterialGeneratorA::SM2Profile* matProfile =static_cast<TerrainMaterialGeneratorA::SM2Profile*>(mTerrainGlobals->getDefaultMaterialGenerator()->getActiveProfile());
         //   matProfile->setLightmapEnabled(false);
        }

        //! [composite_lighting]
        SDL_LogInfo(0,"[TERRAIN ] Generate Lightmap");
        // Important to set these so that the terrain knows what to use for baked (non-realtime) data
        mTerrainGlobals->setLightMapDirection(l->getDerivedDirection());
        mTerrainGlobals->setCompositeMapAmbient(mSceneMgr->getAmbientLight());
        mTerrainGlobals->setCompositeMapDiffuse(l->getDiffuseColour());
        //! [composite_lighting]
        //mTerrainGlobals->setCompositeMapAmbient(ColourValue::Red);

        // Configure default import settings for if we use imported image
        //! [import_settings]
        SDL_LogInfo(0,"[TERRAIN ] Import");
        Ogre::Terrain::ImportData& defaultimp = mTerrainGroup->getDefaultImportSettings();
        defaultimp.inputScale = 600;
        defaultimp.minBatchSize = 33;
        defaultimp.maxBatchSize = 65;
        //! [import_settings]


        SDL_LogInfo(0,"[TERRAIN ] Blend Texture");
        //! [tex_from_src]
        Ogre::Image combined;
        combined.loadTwoImagesAsRGBA("Ground23_col.jpg", "Ground23_spec.png", "General");
        Ogre::TextureManager::getSingleton().loadImage("Ground23_diffspec", "General", combined);
        //! [tex_from_src]

        //! [textures]
        SDL_LogInfo(0,"[TERRAIN ] Paint Texture");
        defaultimp.layerList.resize(3);
        defaultimp.layerList[0].worldSize = 200;
        defaultimp.layerList[0].textureNames.push_back("Ground37_diffspec.dds");
        defaultimp.layerList[0].textureNames.push_back("Ground37_normheight.dds");
        defaultimp.layerList[1].worldSize = 200;
        defaultimp.layerList[1].textureNames.push_back("Ground23_diffspec"); // loaded from memory
        defaultimp.layerList[1].textureNames.push_back("Ground23_normheight.dds");
        defaultimp.layerList[2].worldSize = 400;
        defaultimp.layerList[2].textureNames.push_back("Rock20_diffspec.dds");
        defaultimp.layerList[2].textureNames.push_back("Rock20_normheight.dds");

        //! [textures]
    }


    Ogre::TerrainGlobalOptions* mTerrainGlobals;
    Ogre::TerrainGroup* mTerrainGroup;
    bool mPaging;


    Ogre::uint8 mLayerEdit;
    Ogre::Real mBrushSizeTerrainSpace;




    Ogre::Real mHeightUpdateCountDown;
    Ogre::Real mHeightUpdateRate;
    Ogre::Vector3 mTerrainPos;
    bool mTerrainsImported;
    //TerrainPaging* mTerrainPaging;
    //PageManager* mPageManager;
};

Ogre::RenderWindow* createWindow(const Ogre::String& name, Ogre::uint32 w, Ogre::uint32 h, Ogre::NameValuePairList miscParams)
{
    Ogre::RenderWindow* ret;

mWindowHeight=w;
mWindowWidth=h;


   Ogre::RenderSystemList::const_iterator renderers = (mRoot->getAvailableRenderers().begin());

 while(renderers != mRoot->getAvailableRenderers().end())
  {
      Ogre::String rName = (*renderers)->getName();
   //   SDL_Log("%s",rName.c_str());
        if (rName == "OpenGL Rendering Subsystem")
            break;
        renderers++;
  }

  Ogre::RenderSystem *renderSystem = *renderers;


//  assert( !renderers->empty() ); // we need at least one renderer to do anything useful


  assert( renderSystem ); // user might pass back a null renderer, which would be bad!
  mRoot->setRenderSystem( renderSystem );

   Ogre::String res;
  res  = Ogre::StringConverter::toString(w);
  res += "x";
  res += Ogre::StringConverter::toString(h);

 // renderSystem->setConfigOption( "Video Mode", res);
 // mRoot->getRenderSystem()->setConfigOption( "Full Screen", "No" );
  mRoot->saveConfig();
  mRoot->initialise(false); // don't create a window

  window =        SDL_CreateWindow(name.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w,h, SDL_WINDOW_RESIZABLE );
   //gl_context = SDL_GL_CreateContext(window);
     //   SDL_GL_MakeCurrent(window, gl_context);
       // SDL_GL_SetSwapInterval(0); // Enable vsync


SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(window, &wmInfo);


      Ogre::NameValuePairList params;

    // for tiny rendersystem
params["sdlwin"] = Ogre::StringConverter::toString(size_t(window));
params["externalWindowHandle"] = Ogre::StringConverter::toString(size_t(wmInfo.info.x11.window));



ret = mRoot->createRenderWindow( name,
                      w,h,
                       false,
                       &params );

ret->setActive(true);


    return ret;
}

Ogre::Matrix4 Raymatrix4ToOgre(Matrix m)
{
Ogre::Matrix4 mat=Ogre::Matrix4(
m.m0 , m.m1 ,m.m2 ,m.m3,
m.m4 , m.m5 ,m.m6 ,m.m7,
m.m8 , m.m9 ,m.m10,m.m11,
m.m12, m.m13,m.m14,m.m15);

/*
            mat.m[0][0]=m.m0;
            mat.m[1][0]=m.m1;
            mat.m[2][0]=m.m2;
            mat.m[3][0]=m.m3;

            mat.m[0][1]=m.m4;
            mat.m[1][1]=m.m5;
            mat.m[2][1]=m.m6;
            mat.m[3][1]=m.m7;

            mat.m[0][2]=m.m8;
            mat.m[1][2]=m.m9;
            mat.m[2][2]=m.m10;
            mat.m[3][2]=m.m11;

            mat.m[0][3]=m.m12;
            mat.m[1][3]=m.m13;
            mat.m[2][3]=m.m14;
            mat.m[3][3]=m.m15;
*/




            return mat;
}
Matrix matrix4ToRayMatrix(Ogre::Matrix4 m)
{
Matrix mat;
            mat.m0=m[0][0];
            mat.m1=m[1][0];
            mat.m2=m[2][0];
            mat.m3=m[3][0];

            mat.m4=m[0][1];
            mat.m5=m[1][1];
            mat.m6=m[2][1];
            mat.m7=m[3][1];

            mat.m8=m[0][2];
            mat.m9=m[1][2];
            mat.m10=m[2][2];
            mat.m11=m[3][2];

            mat.m12=m[0][3];
            mat.m13=m[1][3];
            mat.m14=m[2][3];
            mat.m15=m[3][3];

            return mat;
}
void DefaultOnRender(float pre, float post)
{
}

class Application : public Ogre::FrameListener
{
public:
    explicit Application(const Ogre::String& appName = "Ogre3D")
    {
       mRoot = new Ogre::Root("plugins.cfg", "config.cfg", "ogre.log");

        Ogre::LogManager::getSingleton().setLogDetail(Ogre::LL_LOW);

          Ogre::ConfigFile cf;
          cf.load("resources.cfg");

          Ogre::String name, locType;
          Ogre::ConfigFile::SectionIterator secIt = cf.getSectionIterator();

          while (secIt.hasMoreElements())
          {
            Ogre::ConfigFile::SettingsMultiMap* settings = secIt.getNext();
            Ogre::ConfigFile::SettingsMultiMap::iterator it;

            for (it = settings->begin(); it != settings->end(); ++it)
            {
              locType = it->first;
              name = it->second;
             // SDL_Log("%s  %s",name.c_str(),locType.c_str());

              Ogre::ResourceGroupManager::getSingleton().addResourceLocation(name, locType);
            }
          }

    }

    virtual ~Application()
    {
         if (mRoot)
        {
        OGRE_DELETE mRoot;
        mRoot = NULL;
        }
    }



    void initApp(int screenWidth,int screenHeight,const char* title)
    {


        mWindow =createWindow(title, screenWidth ,screenHeight,  Ogre::NameValuePairList());
        Ogre::Root::getSingleton().getRenderSystem()->_initRenderTargets();
        Ogre::Root::getSingleton().clearEventTimes();
        mRoot->addFrameListener(this);

  //      Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
 //       Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

        InitInput();


        OnPreRender=NULL;
        OnRender=NULL;
        OnPostRender=NULL;

/*

        scene = mRoot->createSceneManager( );
        Ogre::SceneNode * root_node = scene->getRootSceneNode();


        camera = scene->createCamera("MainCamera");
        camera->setNearClipDistance(1.0f);
        camera->setAutoAspectRatio(true);
        Ogre::SceneNode * mTarget = root_node->createChildSceneNode();
        mTarget->lookAt(Ogre::Vector3(0, 0, -1), Ogre::Node::TS_PARENT);
        mTarget->attachObject(camera);


        Ogre::Viewport *mViewport = mWindow->addViewport( camera );
        mViewport->setBackgroundColour( Ogre::ColourValue( 0.8,0.8,1 ) );
*/



        rlglInit(screenWidth, screenHeight);

        GuiLoadStyleDefault();
        GuiSetStyle(LISTVIEW, LIST_ITEMS_HEIGHT, 24);
        GuiSetStyle(LISTVIEW, SCROLLBAR_WIDTH, 12);




   //    mRoot->startRendering();

    }


    void StartRender()
    {
     mRoot->startRendering();
    }



    void closeApp()
    {
         rlglClose();
        mWindow->removeAllViewports();
//        mRoot->destroySceneManager(scene);
        mRoot->destroyRenderTarget(mWindow);

        SDL_DestroyWindow(window);

    }
    void pollEvents()
        {
          SDL_Event event;
          ResetInput();
            while (SDL_PollEvent(&event))
            {
                switch (event.type)
                {
                case SDL_QUIT:
                    mRoot->queueEndRendering();
                    break;
                case SDL_WINDOWEVENT:
                    if(event.window.event == SDL_WINDOWEVENT_RESIZED)
                     {
                        mWindow->resize(event.window.data1, event.window.data2);
                     }
                    break;
                default:
//                    InputEvent(event);
                    PollInputEvents(&event);
                    break;
                }
            }
/*
            int xpos, ypos;
            Uint32 mouse_buttons = SDL_GetMouseState(&xpos, &ypos);
            const Uint8 *state = SDL_GetKeyboardState(NULL);
            if (state[SDL_SCANCODE_ESCAPE])
            {
            mRoot->queueEndRendering();
            }*/
        }

    // callback interface copied from various listeners to be used by ApplicationContext
    virtual bool frameStarted(const Ogre::FrameEvent& evt)
    {
        pollEvents();
        if (IsKeyDown(SDL_SCANCODE_F12))
        {
            TakeScreenshot(TextFormat("screenshot%03i.png", screenshotCounter));
            screenshotCounter++;
        }


     if(OnPreRender)
     {
     OnPreRender((float)evt.timeSinceLastEvent,(float)evt.timeSinceLastFrame);
     }







        return true;
    }
    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt)
    {

     if(OnRender)
     {
     OnRender((float)evt.timeSinceLastEvent,(float)evt.timeSinceLastFrame);
     }



/*
            rlEnableDepthTest();
            rlSetMatrixModelview(matrix4ToRayMatrix(camera->getViewMatrix()));
            rlSetMatrixProjection(matrix4ToRayMatrix(camera->getProjectionMatrix()));


            DrawGrid(10, 1.0f);
            rlDrawRenderBatchActive();


            rlDisableDepthTest();
            rlSetBlendMode(2);

            rlMatrixMode(RL_PROJECTION);                            // Enable internal projection matrix
            rlLoadIdentity();                                       // Reset internal projection matrix
            rlOrtho(0.0, screenWidth, screenHeight, 0.0, 0.0, 1.0); // Recalculate internal projection matrix
            rlMatrixMode(RL_MODELVIEW);                             // Enable internal modelview matrix
            rlLoadIdentity();                                       // Reset internal modelview matrix


         GuiLock();
         GuiUnlock();

            DrawStats(mWindow,20,40);

            rlDrawRenderBatchActive();


            //-----------------------------------------------

*/


            if (gifRecording)
            {
                gifFramesCounter++;

                // NOTE: We record one gif frame every 10 game frames
                if ((gifFramesCounter%10) == 0)
                {
                     Ogre::Image img(Ogre::PF_A8B8G8R8,mWindow->getWidth(),mWindow->getHeight());
                     Ogre::PixelBox pb = img.getPixelBox();
                     mWindow->copyContentsToMemory(pb, pb);
                     msf_gif_frame(&gifState, pb.data, 10, 16, mWindow->getWidth()*4);
                }

                if (((gifFramesCounter/15)%2) == 1)
                {
                    DrawCircle(30, mWindow->getHeight() - 20, 10, RED);
                    DrawText("RECORDING", 50, mWindow->getHeight() - 25, 10, MAROON);
                // SDL_GetWindowTitle()
                 // SDL_SetWindowTitle("RECORDING")
                 printf("RECORDING \n");
                }
                rlDrawRenderBatchActive();

            }



    return true;
    }
    virtual bool frameEnded(const Ogre::FrameEvent& evt)
    {
    if(OnPostRender)
     {
     OnPostRender((float)evt.timeSinceLastEvent,(float)evt.timeSinceLastFrame);
     }


        if (IsKeyReleased(SDL_SCANCODE_F10))
        {
         screenRecod();
        }

    return true;
    }
public :

FuncUpdate OnPreRender;
FuncUpdate OnRender;
FuncUpdate OnPostRender;




};

extern "C"
{



#include "input.cc"

#include "core.cc"

Application *app=nullptr;
void InitSystem()
{
SDL_Init(SDL_INIT_VIDEO ) ;
app  = new Application();

}

void CreateRender(const char* title, int w, int h)
{
if (!app) return;
app->initApp(w,h,title);
}

void CloseRender()
{
if (!app) return;
app->closeApp();
}

void SetPreRenderFun(FuncUpdate func)
{
if (!app) return;
app->OnPreRender=func;
}

void SetPostRenderFun(FuncUpdate func)
{
if (!app) return;
app->OnPostRender=func;
}
void SetRenderFun(FuncUpdate func)
{
if (!app) return;
app->OnRender=func;
}
void StartRender()
{
if (!app) return;
app->StartRender();
}

void CloseSystem()
{

if(app)
  delete app;
SDL_Quit();

}

//*************************************************************************************TEXTURE

void SetTextureDefaultNumMipmaps(int n)
{
if(!app || !mRoot)  return ;
Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(n);
}

//*************************************************************************************RESOURCES

void SetResourcesInitializeAll()
{
   if(!app || !mRoot)  return ;
   Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();


}


//*****************************************************************************************WINDOWS
double CreateViewport()
{
   Ogre::Viewport *view = NULL;

   TRY
      view = mWindow->addViewport(NULL);
      view->setBackgroundColour(Ogre::ColourValue(0.0f, 0.0f, 0.0f));
   CATCH("CreateViewport")

   return ConvertToPointer(view);
}

double CreateViewportEx(float left, float top, float width, float height,int zorder)
{
   Ogre::Viewport *view = NULL;

   TRY
      view = mWindow->addViewport(NULL, (int)zorder, (float)left / mWindowWidth, (float)top / mWindowHeight, (float)width / mWindowWidth, (float)height / mWindowHeight);
      view->setBackgroundColour(Ogre::ColourValue(0.0f, 0.0f, 0.0f));
   CATCH("CreateViewport")

   return ConvertToPointer(view);
}


double addWindowViewPort(double c)
{
   if(!app || !mRoot)  return 0;


   Ogre::Camera *cam = ConvertFromPointer<Ogre::Camera*>(c);

   if (cam == NULL)
      return 0;



    Ogre::Viewport *view = mWindow->addViewport(cam );

    return ConvertToPointer(view);

}


void SetViewportCamera(double viewport_ptr, double camera_ptr)
{
   Ogre::Viewport *view = ConvertFromPointer<Ogre::Viewport*>(viewport_ptr);

   if (view == NULL)
      return ;

   Ogre::Camera *cam = ConvertFromPointer<Ogre::Camera*>(camera_ptr);

   if (cam == NULL)
      return ;

   view->setCamera(cam);


}


void DestroyViewport(double viewport_ptr)
{
   mWindow->removeViewport(ConvertFromPointer<Ogre::Viewport*>(viewport_ptr)->getZOrder());

}


void  DestroyAllViewports()
{
   mWindow->removeAllViewports();
}


Ogre::ColourValue getColor(Color color)
{
 return Ogre::ColourValue(color.r/255.0f,color.g/255.0f,color.b/255.0f,color.a/255.0f );
}


void SetViewportBackgroundColor(double v,Color c)
{
    if(!app || !mRoot)  return ;

    Ogre::Viewport *view = ConvertFromPointer<Ogre::Viewport*>(v);

   if (view == NULL)
      return;


    view->setBackgroundColour(getColor(c) );

}


//******************************************************************************************MESH
double CreatePlaneMesh(const char* name,float width, float height, float xsegments, float ysegments, float utile, float vtile)
{

if(!app || !mRoot)  return 0;


Ogre::MeshPtr mesh;

   TRY
      Ogre::Plane plane;
      plane.d = 0;
      plane.normal = Ogre::Vector3::UNIT_Y;
      mesh = Ogre::MeshManager::getSingleton().createPlane(name, Ogre::RGN_DEFAULT, plane,
      width, height, xsegments,ysegments, true, 1,utile, vtile, Ogre::Vector3::UNIT_Z);




   CATCH("CreatePlaneMesh")

   return ConvertToPointer(mesh.getPointer());
}


//******************************************************************************************SCENE
void SetSceneSkyBox(double scene,bool enable,const char* name,float distance)
{
  if(!app || !mRoot)  return ;

  Ogre::SceneManager *mSceneMgr=ConvertFromPointer<Ogre::SceneManager*>(scene);
    if (mSceneMgr == NULL)
      return ;

mSceneMgr->setSkyBox(enable,name,distance);

}

//curvature = 10,tiling = 8,distance = 4000,
void SetSceneSkyDome(double scene,bool enable,const char* name,float curvature,float tiling ,float distance)
{
  if(!app || !mRoot)  return ;

  Ogre::SceneManager *mSceneMgr=ConvertFromPointer<Ogre::SceneManager*>(scene);
    if (mSceneMgr == NULL)
      return ;

mSceneMgr->setSkyDome(enable,name,curvature,tiling,distance);

}

void SetSceneSkyPlane(double scene,bool enable,const char* name,float plane_distance,float scale,float tiling , float bend, int xsegments, int ysegments)
{
  if(!app || !mRoot)  return ;

  Ogre::SceneManager *mSceneMgr=ConvertFromPointer<Ogre::SceneManager*>(scene);
    if (mSceneMgr == NULL)
      return ;

Ogre::Plane plane;
plane.d = plane_distance;
plane.normal = Ogre::Vector3::NEGATIVE_UNIT_Y;

mSceneMgr->setSkyPlane(enable, plane, name,scale, tiling,true,bend,xsegments,ysegments);


}

//FogMode mode = FOG_NONE, Real expDensity = 0.001f, Real linearStart = 0.0f, Real linearEnd = 1.0f);
void SetSceneFog(double scene,FogMode type,Color color,float expDensity,float linearStart, float linearEnd )
{
  if(!app || !mRoot)  return ;

  Ogre::SceneManager *mSceneMgr=ConvertFromPointer<Ogre::SceneManager*>(scene);
    if (mSceneMgr == NULL)
      return ;

mSceneMgr->setFog((Ogre::FogMode)type, Ogre::ColourValue(color.r/255.0f,color.g/255.0f,color.b/255.0f,color.a/255.0f), expDensity, linearStart, linearEnd);


}

//*************************************************************




//mSceneMgr->setSkyBox(true, "Examples/CloudyNoonSkyBox");

double CreateScene()
{
   if(!app || !mRoot)  return 0;
   Ogre::SceneManager *mSceneMgr = mRoot->createSceneManager( );

//   scene->setShadowTechnique(ShadowTechnique::SHADOWTYPE_STENCIL_ADDITIVE);
     mSceneMgr->setShadowTechnique(Ogre::ShadowTechnique::SHADOWTYPE_STENCIL_ADDITIVE);
    //sm->setShadowTextureCasterMaterial(MaterialManager::getSingleton().getByName("DeferredShading/Shadows/Caster", "General"));
   // mSceneMgr->setShadowTextureCount(1);
  //  mSceneMgr->setShadowFarDistance(150);
  //  mSceneMgr->setShadowTextureConfig( 0, 512, 512, PF_FLOAT16_R, 0, 2 );
 //   mSceneMgr->setShadowDirectionalLightExtrusionDistance(75);
      Ogre::LogManager::getSingleton().setMinLogLevel(Ogre::LML_TRIVIAL);
/*
using namespace Ogre;

    TerrainGlobalOptions* mTerrainGlobals;
    TerrainGroup* mTerrainGroup;
    Real mBrushSizeTerrainSpace;
    Real mHeightUpdateCountDown;
    Real mHeightUpdateRate;
    Ogre::Vector3 mTerrainPos=Ogre::Vector3(1000,0,5000);

    bool mTerrainsImported;


      //! [global_opts]
        mTerrainGlobals = new Ogre::TerrainGlobalOptions();
        //! [global_opts]


        MaterialManager::getSingleton().setDefaultTextureFiltering(TFO_ANISOTROPIC);
        MaterialManager::getSingleton().setDefaultAnisotropy(7);

        ColourValue fadeColour(0.7, 0.7, 0.8);


        LogManager::getSingleton().setMinLogLevel(LML_TRIVIAL);

        //! [light]
        Ogre::Light* l = mSceneMgr->createLight();
        l->setType(Ogre::Light::LT_DIRECTIONAL);
        l->setDiffuseColour(ColourValue::White);
        l->setSpecularColour(ColourValue(0.4, 0.4, 0.4));

        Ogre::SceneNode* ln = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        ln->setDirection(Ogre::Vector3(0.55, -0.3, 0.75).normalisedCopy());
        ln->attachObject(l);
        //! [light]


        //! [terrain_create]
        mTerrainGroup = new Ogre::TerrainGroup(mSceneMgr, Ogre::Terrain::ALIGN_X_Z, TERRAIN_SIZE, TERRAIN_WORLD_SIZE);
        mTerrainGroup->setFilenameConvention(TERRAIN_FILE_PREFIX, TERRAIN_FILE_SUFFIX);
        mTerrainGroup->setOrigin(mTerrainPos);
        //! [terrain_create]

      //  configureTerrainDefaults(l);

        for (long x = TERRAIN_PAGE_MIN_X; x <= TERRAIN_PAGE_MAX_X; ++x)
            for (long y = TERRAIN_PAGE_MIN_Y; y <= TERRAIN_PAGE_MAX_Y; ++y)
                  mTerrainGroup->defineTerrain(x, y, 0.0f);
        // sync load since we want everything in place when we start
        mTerrainGroup->loadAllTerrains(true);
        //! [define_loop]


        //! [init_blend]
        if (mTerrainsImported)
        {
            for (const auto& ti : mTerrainGroup->getTerrainSlots())
            {
               // initBlendMaps(ti.second->instance);
            }
        }

        mTerrainGroup->freeTemporaryResources();
*/
   return ConvertToPointer(mSceneMgr);
}


double CreateSceneByName(const char* name)
{
   if(!app || !mRoot)  return 0;
   Ogre::SceneManager *scene = mRoot->createSceneManager(name );
   return ConvertToPointer(scene);
}

double CreateSceneByMask(const char* name,int mask)
{
   if(!app || !mRoot)  return 0;
   Ogre::SceneManager *scene = mRoot->createSceneManager((Ogre::uint16)mask,name );
   return ConvertToPointer(scene);
}


double AddEntityToScene(double scene,double n)
{
  if(!app || !mRoot)  return 0;

  Ogre::SceneManager *sc=ConvertFromPointer<Ogre::SceneManager*>(scene);
    if (sc == NULL)
      return 0;

  Ogre::SceneNode * node= sc->getRootSceneNode()->createChildSceneNode();
  node->attachObject( ConvertFromPointer<Ogre::MovableObject*>(n));

  return ConvertToPointer(node);

}
double GetSceneRootNode(double scene)
{
  if(!app || !mRoot  )  return 0;


  Ogre::SceneManager *sc=ConvertFromPointer<Ogre::SceneManager*>(scene);
    if (sc == NULL)
      return 0;

  Ogre::SceneNode * n = sc->getRootSceneNode();
  return ConvertToPointer(n);

}

void DestroyScene(double scene)
{
    if(!app || !mRoot )  return ;
    Ogre::SceneManager *sc=ConvertFromPointer<Ogre::SceneManager*>(scene);
    if (sc == NULL)
      return ;
     mRoot->destroySceneManager(sc);

}
void SetSceneAmbientLight(double scene,Color c)
{
     if(!app || !mRoot)  return ;
    Ogre::SceneManager *sc=ConvertFromPointer<Ogre::SceneManager*>(scene);
    if (sc == NULL)
      return ;
     sc->setAmbientLight(getColor(c));
}



double CreateCamera(double scene,const char* name)
{
  if(!app || !mRoot)  return 0;

    Ogre::SceneManager *sc=ConvertFromPointer<Ogre::SceneManager*>(scene);
    if (sc == NULL)
      return 0;

  Ogre::Camera *c = sc->createCamera(name);

  return ConvertToPointer(c);

}
 double CreateCameraEx(double scene,const char* name,double aspect, double znear, double zfar, double fov)
{
if(!app || !mRoot)  return 0;
 Ogre::SceneManager *mSceneMgr=ConvertFromPointer<Ogre::SceneManager*>(scene);
    if (mSceneMgr == NULL)
      return 0;

   Ogre::Camera *cam = NULL;

   TRY
      cam = mSceneMgr->createCamera(name);

      if (cam == NULL)
         return 0;

      cam->setNearClipDistance((Ogre::Real)znear);
      cam->setFarClipDistance((Ogre::Real)zfar);
      cam->setAspectRatio(Ogre::Real(aspect));
      cam->setFOVy(Ogre::Degree((Ogre::Real)fov));

   CATCH("CreateCamera")

   return ConvertToPointer(cam);
}
double CreateEntity(double scene,const char* name)
{
  if(!app || !mRoot)  return 0;

    Ogre::SceneManager *sc=ConvertFromPointer<Ogre::SceneManager*>(scene);
    if (sc == NULL)
      return 0;


  Ogre::Entity* n = sc->createEntity(name);

  return ConvertToPointer(n);

}
double CreateEntityFromFile(double scene,const char* filename)
{
  if(!app || !mRoot )  return 0;

    Ogre::SceneManager *sc=ConvertFromPointer<Ogre::SceneManager*>(scene);
    if (sc == NULL)
      return 0;


  Ogre::Entity* n = sc->createEntity(filename);

  return ConvertToPointer(n);

}
double CreateEntityFromMesh(double scene,const char* name,double mesh)
{
  if(!app || !mRoot )  return 0;

    Ogre::SceneManager *sc=ConvertFromPointer<Ogre::SceneManager*>(scene);
    if (sc == NULL)
      return 0;


  Ogre::MeshPtr* pMesh=ConvertFromPointer<Ogre::MeshPtr*>(mesh);

  Ogre::Entity* n = sc->createEntity(name,*pMesh);

  return ConvertToPointer(n);

}


double CreateLightType(double scene,const char* name,LightType lt)
{
  if(!app || !mRoot  )  return 0;

    Ogre::SceneManager *sc=ConvertFromPointer<Ogre::SceneManager*>(scene);
    if (sc == NULL)
      return 0;


  Ogre::Light* n = sc->createLight(name,(Ogre::Light::LightTypes)lt);


  return ConvertToPointer(n);

}


//******************************************************************************************ĆAMERA
enum CameraStyle   /// enumerator values for different styles of camera movement
    {
        CS_FREELOOK,
        CS_ORBIT,
        CS_MANUAL
    };

Ogre::Node::TransformSpace mYawSpace;
Ogre::SceneNode* mCamera;
Ogre::SceneNode* mTarget;
CameraStyle mStyle=CS_FREELOOK;
bool mOrbiting=false;
bool mMoving=false;
bool mFastMove=false;
Ogre::Real mTopSpeed=150;
Ogre::Vector3 mVelocity=Ogre::Vector3::ZERO;
Ogre::Vector3 mOffset=Ogre::Vector3::ZERO;


void setTarget(Ogre::SceneNode *target)
{
if (target == mTarget)
        return;
    mTarget = target;
}

void SetNodeCameraTarget(double n)
{
   if(!app || !mRoot)  return ;
   Ogre::SceneNode *cam = ConvertFromPointer<Ogre::SceneNode*>(n);

   if (cam == NULL)
      return ;

    setTarget(cam);
}
void SetNodeCameraControl(double n,float maxSpeed)
{
   if(!app || !mRoot)  return ;
   Ogre::SceneNode *cam = ConvertFromPointer<Ogre::SceneNode*>(n);

   if (cam == NULL)
      return ;

    mTopSpeed=maxSpeed;

   mCamera=cam;
}


void manualStop()
{
    if (mStyle == CS_FREELOOK)
    {
        mVelocity = Ogre::Vector3::ZERO;
    }
}
Ogre::Real getDistToTarget()
{
    if (!mCamera || !mTarget) return 0;


    Ogre::Vector3 offset = mCamera->getPosition() - mTarget->_getDerivedPosition() - mOffset;
    return offset.length();
}
void translate(float x, float y, float z)
{
    if (!mCamera ) return ;
    mCamera->translate(Ogre::Vector3(x,y,z), Ogre::Node::TS_LOCAL);
}


void setYawPitchDist(Ogre::Radian yaw, Ogre::Radian pitch, Ogre::Real dist)
{
    OgreAssert(mTarget, "no target set");
    mOffset = Ogre::Vector3::ZERO;
    mCamera->setPosition(mTarget->_getDerivedPosition());
    mCamera->setOrientation(mTarget->_getDerivedOrientation());
    mCamera->yaw(yaw);
    mCamera->pitch(-pitch);
    mCamera->translate(Ogre::Vector3(0, 0, dist), Ogre::Node::TS_LOCAL);
}

void setFixedYaw(bool fixed)
{
    mYawSpace = fixed ? Ogre::Node::TS_PARENT : Ogre::Node::TS_LOCAL;
}

void setPivotOffset(const Ogre::Vector3& pivot)
{
    if (!mCamera ) return ;
    Ogre::Real dist = getDistToTarget();
    mOffset = pivot;
    mCamera->setPosition(mTarget->_getDerivedPosition() + mOffset);
    mCamera->translate(Ogre::Vector3(0, 0, dist), Ogre::Node::TS_LOCAL);
}
void SetCameraOffset(float x,float y, float z)
{
setPivotOffset(Ogre::Vector3(x,y,z));
}
void MoveCameraDepth(float speed)
{
if (!mCamera || !mTarget) return ;
 if (mStyle == CS_ORBIT && speed != 0)
{
    Ogre::Real dist = (mCamera->getPosition() - mTarget->_getDerivedPosition()).length();
    mCamera->translate(Ogre::Vector3(0, 0, -speed * 0.08f * dist), Ogre::Node::TS_LOCAL);
}
}
void CameraMouseLook(float xrel, float yrel)
{

     if (mStyle == CS_ORBIT)
    {
        if (!mCamera || !mTarget) return ;
        Ogre::Real dist = getDistToTarget();

        mOrbiting  = IsMouseButtonDown(0);
        mMoving    = IsMouseButtonDown(2);



        if (mOrbiting)   // yaw around the target, and pitch locally
        {
            mCamera->setPosition(mTarget->_getDerivedPosition() + mOffset);

            mCamera->yaw(Ogre::Degree(-xrel * 0.25f), mYawSpace);
            mCamera->pitch(Ogre::Degree(-yrel * 0.25f));

            mCamera->translate(Ogre::Vector3(0, 0, dist), Ogre::Node::TS_LOCAL);
            // don't let the camera go over the top or around the bottom of the target
        }
        else if (mMoving)  // move the camera along the image plane
        {


            Ogre::Vector3 delta = mCamera->getOrientation() * Ogre::Vector3(-xrel, yrel, 0);
            // the further the camera is, the faster it moves
            delta *= dist / 1000.0f;
            mOffset += delta;
            mCamera->translate(delta);

        }

    }
    else if (mStyle == CS_FREELOOK)
    {
        if (!mCamera) return ;
        mCamera->yaw(Ogre::Degree(-xrel * 0.15f), Ogre::Node::TS_PARENT);
        mCamera->pitch(Ogre::Degree(-yrel * 0.15f));
    }

}
void CameraAdvance(Ogre::Vector3 accel,float timeSinceLastFrame)
{
if (!mCamera) return ;

      Ogre::Real topSpeed = mFastMove ? mTopSpeed * 20 : mTopSpeed;
        if (accel.squaredLength() != 0)
        {
            accel.normalise();
            mVelocity += accel * topSpeed * timeSinceLastFrame * 10;
        }
        // if not accelerating, try to stop in a certain time
        else mVelocity -= mVelocity * timeSinceLastFrame * 10;

        Ogre::Real tooSmall = std::numeric_limits<Ogre::Real>::epsilon();

        // keep camera velocity below top speed and above epsilon
        if (mVelocity.squaredLength() > topSpeed * topSpeed)
        {
            mVelocity.normalise();
            mVelocity *= topSpeed;
        }
        else if (mVelocity.squaredLength() < tooSmall * tooSmall)
            mVelocity = Ogre::Vector3::ZERO;

        if (mVelocity != Ogre::Vector3::ZERO) mCamera->translate(mVelocity * timeSinceLastFrame);
}
void MoveCameraForward(float timeSinceLastFrame)
{
if (!mCamera) return ;
   if (mStyle == CS_FREELOOK)
    {
        Ogre::Vector3 accel = Ogre::Vector3::ZERO;
        Ogre::Matrix3 axes = mCamera->getLocalAxes();
        accel -= axes.GetColumn(2);
        CameraAdvance(accel,timeSinceLastFrame);
    }
}
void MoveCameraBack(float timeSinceLastFrame)
{
if (!mCamera) return ;
   if (mStyle == CS_FREELOOK)
    {
        Ogre::Vector3 accel = Ogre::Vector3::ZERO;
        Ogre::Matrix3 axes = mCamera->getLocalAxes();
        accel += axes.GetColumn(2);
        CameraAdvance(accel,timeSinceLastFrame);
    }
}

void MoveCameraUp(float timeSinceLastFrame)
{
if (!mCamera) return ;
   if (mStyle == CS_FREELOOK)
    {
        Ogre::Vector3 accel = Ogre::Vector3::ZERO;
        Ogre::Matrix3 axes = mCamera->getLocalAxes();
        accel += axes.GetColumn(1);
        CameraAdvance(accel,timeSinceLastFrame);
    }
}
void MoveCameraDown(float timeSinceLastFrame)
{
if (!mCamera) return ;
   if (mStyle == CS_FREELOOK)
    {
        Ogre::Vector3 accel = Ogre::Vector3::ZERO;
        Ogre::Matrix3 axes = mCamera->getLocalAxes();
        accel -= axes.GetColumn(1);
        CameraAdvance(accel,timeSinceLastFrame);
    }
}
void MoveCameraLeft(float timeSinceLastFrame)
{
if (!mCamera) return ;
   if (mStyle == CS_FREELOOK)
    {
        Ogre::Vector3 accel = Ogre::Vector3::ZERO;
        Ogre::Matrix3 axes = mCamera->getLocalAxes();
        accel -= axes.GetColumn(0);
        CameraAdvance(accel,timeSinceLastFrame);
    }
}
void MoveCameraRight(float timeSinceLastFrame)
{
if (!mCamera) return ;
   if (mStyle == CS_FREELOOK)
    {
        Ogre::Vector3 accel = Ogre::Vector3::ZERO;
        Ogre::Matrix3 axes = mCamera->getLocalAxes();
        accel += axes.GetColumn(0);
        CameraAdvance(accel,timeSinceLastFrame);
    }
}


void setStyle(CameraStyle style)
{
  if (mStyle != CS_ORBIT && style == CS_ORBIT)
    {
    if (!mCamera || !mTarget) return ;

        setTarget(mTarget ? mTarget : mCamera->getCreator()->getRootSceneNode());
        // fix the yaw axis if requested
        mCamera->setFixedYawAxis(mYawSpace == Ogre::Node::TS_PARENT);
        manualStop();

        // try to replicate the camera configuration
        Ogre::Real dist = getDistToTarget();
        const Ogre::Quaternion& q = mCamera->getOrientation();
        setYawPitchDist(q.getYaw(), q.getPitch(), dist == 0 ? 150 : dist); // enforce some distance
    }
    else if (mStyle != CS_FREELOOK && style == CS_FREELOOK)
    {
        if (!mCamera ) return ;
        mCamera->setFixedYawAxis(true); // also fix axis with lookAt calls
    }
    else if (mStyle != CS_MANUAL && style == CS_MANUAL)
    {
        manualStop();
    }
    mStyle = style;

    if (!mCamera ) return ;
    mCamera->setAutoTracking(false);

}

void SetCameraOrbit()
{
setStyle(CS_ORBIT);
}
void SetCameraFree()
{
setStyle(CS_FREELOOK);
}

void DestroyCamera(double scene,double camera_ptr)
{
   if(!app || !mRoot)  return ;
    Ogre::SceneManager *sc=ConvertFromPointer<Ogre::SceneManager*>(scene);

    if (sc == NULL)
      return ;


   Ogre::Camera *cam = ConvertFromPointer<Ogre::Camera*>(camera_ptr);

   if (cam == NULL)
      return ;

   sc->destroyCamera(cam);


}

 void SetCameraViewMatrix(double camera_ptr, Matrix mat)
{
   if(!app || !mRoot)  return ;
   Ogre::Camera *cam = ConvertFromPointer<Ogre::Camera*>(camera_ptr);

   if (cam == NULL)
      return ;


  Ogre::Matrix4 rmat=Raymatrix4ToOgre(mat);
  Ogre::Affine3 m =Ogre::Affine3(rmat);

   cam->setCustomViewMatrix(true,m);

}

void SetCameraLookAt(double camera_ptr, Vector3 position, Vector3 direction, Vector3 up)
{
   if(!app || !mRoot)  return ;
   Ogre::Camera *cam = ConvertFromPointer<Ogre::Camera*>(camera_ptr);

   if (cam == NULL)
      return ;


  Ogre::Matrix4 rmat=Raymatrix4ToOgre(MatrixLookAt(position,direction,up));
  Ogre::Affine3 m =Ogre::Affine3(rmat);
  cam->setCustomViewMatrix(true,m);

}

 void SetCameraAspect(double camera_ptr, float aspect)
{
   if(!app || !mRoot)  return ;
   Ogre::Camera *cam = ConvertFromPointer<Ogre::Camera*>(camera_ptr);

   if (cam == NULL)
      return ;

   cam->setAspectRatio(aspect);

}

 void SetCameraAutoAspectRatio(double camera_ptr, bool value)
{
   if(!app || !mRoot)  return ;
   Ogre::Camera *cam = ConvertFromPointer<Ogre::Camera*>(camera_ptr);

   if (cam == NULL)
      return ;


   cam->setAutoAspectRatio(value);

}


 void SetCameraClipDistances(double camera_ptr, float znear, float zfar)
{
   if(!app || !mRoot)  return ;
   Ogre::Camera *cam = ConvertFromPointer<Ogre::Camera*>(camera_ptr);

   if (cam == NULL)
      return ;

   cam->setNearClipDistance(znear);
   cam->setFarClipDistance(zfar);


}


//******************************************************************************************SCENENODE


void MoveNode(double node_ptr,Vector3 direction,float dt)
{
  if(!app || !mRoot)  return ;
Ogre::SceneNode *node = ConvertFromPointer<Ogre::SceneNode*>(node_ptr);
  if (node == NULL)
      return ;



  node->translate (node->getOrientation () * Ogre::Vector3(direction.x,direction.y,direction.z) *dt );
}

void MoveNodeZ(double node_ptr,float speed)
{
  if(!app || !mRoot)  return ;
Ogre::SceneNode *node = ConvertFromPointer<Ogre::SceneNode*>(node_ptr);
  if (node == NULL)
      return ;
  node->translate (node->getOrientation () * Ogre::Vector3 (0,0,speed));
}

void MoveNodeX(double node_ptr,float speed)
{
  if(!app || !mRoot)  return ;
Ogre::SceneNode *node = ConvertFromPointer<Ogre::SceneNode*>(node_ptr);
  if (node == NULL)
      return ;
  node->translate (node->getOrientation () * Ogre::Vector3 (speed,0,0));
}
void MoveNodeY(double node_ptr,float speed)
{
  if(!app || !mRoot)  return ;
Ogre::SceneNode *node = ConvertFromPointer<Ogre::SceneNode*>(node_ptr);
  if (node == NULL)
      return ;
  node->translate (node->getOrientation () * Ogre::Vector3 (0,speed,0));
}


void SetNodeLookAt(double node_ptr, Vector3 position, Vector3 direction, Vector3 up)
{
  if(!app || !mRoot)  return ;
Ogre::SceneNode *node = ConvertFromPointer<Ogre::SceneNode*>(node_ptr);
if (node == NULL)
  return ;




   node->setPosition(position.x,position.y,position.z);
  //// node->setDirection(direction.x,direction.y,direction.z);
  // node->lookAt(Ogre::Vector3(up.x,up.y,up.z),Ogre::SceneNode::TS_LOCAL,Ogre::Vector3(up.x,up.y,up.z));


}




void NodeLookAt(double n,float x, float y, float z, TransformSpace space)
{
if(!app || !mRoot)  return ;
Ogre::SceneNode *node = ConvertFromPointer<Ogre::SceneNode*>(n);
if (node == NULL)
  return ;

node->lookAt(Ogre::Vector3(x,y,z),(Ogre::Node::TransformSpace)space);
}

void NodeLookAtVect(double n,Vector3 target,Vector3 dir, TransformSpace space)
{
if(!app || !mRoot)  return ;
Ogre::SceneNode *node = ConvertFromPointer<Ogre::SceneNode*>(n);
if (node == NULL)
  return ;

node->lookAt(Ogre::Vector3(target.x,target.y,target.z),(Ogre::Node::TransformSpace)space,Ogre::Vector3(dir.x,dir.y,dir.z));
}

void AttachObjectToNode(double n,double c)
{
if(!app || !mRoot)  return ;

Ogre::SceneNode *node = ConvertFromPointer<Ogre::SceneNode*>(n);
if (node == NULL)
  return ;

Ogre::MovableObject *child = ConvertFromPointer<Ogre::MovableObject*>(c);
if (child == NULL)
  return ;


node->attachObject(child);
}

double CreateChildNode(double n)
{
if(!app || !mRoot)  return 0;
Ogre::SceneNode *node = ConvertFromPointer<Ogre::SceneNode*>(n);
if (node == NULL)
  return 0 ;

Ogre::SceneNode *child=node->createChildSceneNode();
return ConvertToPointer(child);
}



//******************************************************************************************NODE


void NodeAddChild(double n,double c)
{
if(!app || !mRoot)  return ;

Ogre::SceneNode *node = ConvertFromPointer<Ogre::SceneNode*>(n);
if (node == NULL)
  return ;

Ogre::SceneNode *child = ConvertFromPointer<Ogre::SceneNode*>(c);
if (child == NULL)
  return ;


node->addChild(child);
}

void SetNodePosition(double n,float x, float y, float z)
{
if(!app || !mRoot)  return ;

Ogre::SceneNode *node = ConvertFromPointer<Ogre::SceneNode*>(n);
if (node == NULL)
  return ;
node->setPosition(x,y,z);
}

void SetNodePositionVec(double n,Vector3 v)
{
if(!app || !mRoot)  return ;

Ogre::SceneNode *node = ConvertFromPointer<Ogre::SceneNode*>(n);
if (node == NULL)
  return ;
node->setPosition(v.x,v.y,v.z);
}


void SetNodeDirection(double n,float x, float y, float z)
{
if(!app || !mRoot)  return ;

Ogre::SceneNode *node = ConvertFromPointer<Ogre::SceneNode*>(n);
if (node == NULL)
  return ;
node->setDirection(x,y,z);
}

void SetNodeDirectionVec(double n,Vector3 v)
{
if(!app || !mRoot)  return ;

Ogre::SceneNode *node = ConvertFromPointer<Ogre::SceneNode*>(n);
if (node == NULL)
  return ;
node->setDirection(v.x,v.y,v.z);


}

void SetNodeScale(double n,float x, float y, float z)
{
if(!app || !mRoot)  return ;

Ogre::SceneNode *node = ConvertFromPointer<Ogre::SceneNode*>(n);
if (node == NULL)
  return ;
node->setScale(x,y,z);
}

void SetNodeScaleVec(double n,Vector3 v)
{
if(!app || !mRoot)  return ;

Ogre::SceneNode *node = ConvertFromPointer<Ogre::SceneNode*>(n);
if (node == NULL)
  return ;
node->setScale(v.x,v.y,v.z);
}

void SetNodeOrientation(double n,float w,float x, float y, float z)
{
if(!app || !mRoot)  return ;

Ogre::SceneNode *node = ConvertFromPointer<Ogre::SceneNode*>(n);
if (node == NULL)
  return ;
node->setOrientation(w,x,y,z);
}

void SetNodeOrientationQuat(double n,Quaternion v)
{
if(!app || !mRoot)  return ;

Ogre::SceneNode *node = ConvertFromPointer<Ogre::SceneNode*>(n);
if (node == NULL)
  return ;
node->setOrientation(v.w,v.x,v.y,v.z);
}

void SetNodeTranslate(double node_ptr,float x,float y,float z , TransformSpace space)
{
  if(!app || !mRoot)  return ;
Ogre::SceneNode *node = ConvertFromPointer<Ogre::SceneNode*>(node_ptr);
  if (node == NULL)
      return ;
  node->translate(Ogre::Vector3(x,y,z),(Ogre::Node::TransformSpace)space);
}
void SetNodeVecTranslate(double node_ptr,Vector3 position, TransformSpace space)
{
  if(!app || !mRoot)  return ;
Ogre::SceneNode *node = ConvertFromPointer<Ogre::SceneNode*>(node_ptr);
  if (node == NULL)
      return ;
  node->translate( Ogre::Vector3(position.x,position.y,position.z),(Ogre::Node::TransformSpace)space);
}

void SetNodeYaw(double n,float angle, TransformSpace space)
{
if(!app || !mRoot)  return ;
Ogre::SceneNode *node = ConvertFromPointer<Ogre::SceneNode*>(n);
if (node == NULL)
  return ;
node->yaw(Ogre::Radian(angle),(Ogre::Node::TransformSpace)space);
}

void SetNodePitch(double n,float angle, TransformSpace space)
{
if(!app || !mRoot)  return ;
Ogre::SceneNode *node = ConvertFromPointer<Ogre::SceneNode*>(n);
if (node == NULL)
  return ;
node->pitch(Ogre::Radian(angle),(Ogre::Node::TransformSpace)space);
}

void SetNodeRoll(double n,float angle, TransformSpace space)
{
if(!app || !mRoot)  return ;
Ogre::SceneNode *node = ConvertFromPointer<Ogre::SceneNode*>(n);
if (node == NULL)
  return ;
node->roll(Ogre::Radian(angle),(Ogre::Node::TransformSpace)space);
}

void SetNodeRotate(double n,Vector3 axis,float angle, TransformSpace space)
{
if(!app || !mRoot)  return ;
Ogre::SceneNode *node = ConvertFromPointer<Ogre::SceneNode*>(n);
if (node == NULL)
  return ;
node->rotate(Ogre::Vector3(axis.x,axis.y,axis.z),Ogre::Radian(angle),(Ogre::Node::TransformSpace)space);
}

void SetNodeQuatRotate(double n,Quaternion q, TransformSpace space)
{
if(!app || !mRoot)  return ;
Ogre::SceneNode *node = ConvertFromPointer<Ogre::SceneNode*>(n);
if (node == NULL)
  return ;
node->rotate(Ogre::Quaternion(q.w,q.x,q.y,q.z),(Ogre::Node::TransformSpace)space);
}

Vector3 GetNodePosition(double n)
{
Vector3 v;
if(!app || !mRoot)  return v;

Ogre::SceneNode *node = ConvertFromPointer<Ogre::SceneNode*>(n);
if (node == NULL)
  return v;
Ogre::Vector3 p= node->getPosition();
v.x=p.x;
v.y=p.y;
v.z=p.z;

return v;
}


Vector3 GetNodeScale(double n)
{
Vector3 v;
if(!app || !mRoot)  return v;

Ogre::SceneNode *node = ConvertFromPointer<Ogre::SceneNode*>(n);
if (node == NULL)
  return v;
Ogre::Vector3 p= node->getScale();
v.x=p.x;
v.y=p.y;
v.z=p.z;

return v;
}

Quaternion GetNodeOrientation(double n)
{
Quaternion quat;

if(!app || !mRoot)  return quat;
Ogre::SceneNode *node = ConvertFromPointer<Ogre::SceneNode*>(n);

if (node == NULL)
  return quat;
Ogre::Quaternion Orientation=node->getOrientation();

quat.x=Orientation.x;
quat.y=Orientation.y;
quat.z=Orientation.z;
quat.w=Orientation.w;
return quat;

}


Quaternion GetNodeDerivedOrientation(double n)
{
Quaternion quat;

if(!app || !mRoot)  return quat;
Ogre::SceneNode *node = ConvertFromPointer<Ogre::SceneNode*>(n);

if (node == NULL)
  return quat;
Ogre::Quaternion Orientation=node->_getDerivedOrientation();

quat.x=Orientation.x;
quat.y=Orientation.y;
quat.z=Orientation.z;
quat.w=Orientation.w;
return quat;

}



Vector3 GetNodeDerivedPosition(double n)
{
Vector3 v;
if(!app || !mRoot)  return v;

Ogre::SceneNode *node = ConvertFromPointer<Ogre::SceneNode*>(n);
if (node == NULL)
  return v;
Ogre::Vector3 p= node->_getDerivedPosition();
v.x=p.x;
v.y=p.y;
v.z=p.z;

return v;
}


Vector3 GetNodeDerivedScale(double n)
{
Vector3 v;
if(!app || !mRoot)  return v;

Ogre::SceneNode *node = ConvertFromPointer<Ogre::SceneNode*>(n);
if (node == NULL)
  return v;
Ogre::Vector3 p= node->_getDerivedScale();
v.x=p.x;
v.y=p.y;
v.z=p.z;

return v;
}

//******************************************************************************************ENTITY
void EnableEntityCastShadows(double e, bool enable)
{
   if(!app || !mRoot)  return ;

    Ogre::Entity *node = ConvertFromPointer<Ogre::Entity*>(e);
    if (node == NULL)
      return ;


   node->setCastShadows(enable);
}

void SetEntityMaterialName(double e,const char* name)
{
   if(!app || !mRoot)  return ;

    Ogre::Entity *node = ConvertFromPointer<Ogre::Entity*>(e);
    if (node == NULL)
      return ;


   node->setMaterialName(name);
}



//******************************************************************************************LIGHT

double CreateLight(double scene,const char* name)
{
  if(!app || !mRoot)  return 0;

    Ogre::SceneManager *sc=ConvertFromPointer<Ogre::SceneManager*>(scene);
    if (sc == NULL)
      return 0;


  Ogre::Light* n = sc->createLight(name);





  return ConvertToPointer(n);

}
void DestroyLight(double scene,double light_ptr)
{
    if(!app || !mRoot)  return ;

    Ogre::SceneManager *mSceneMgr=ConvertFromPointer<Ogre::SceneManager*>(scene);
    if (mSceneMgr == NULL)
      return ;



   Ogre::Light *light = ConvertFromPointer<Ogre::Light*>(light_ptr);

   if (light == NULL)
      OgreAssert(mTarget, "no light set");

   mSceneMgr->destroyLight(light);


}



void SetLightType(double light_ptr, LightType type)
{
if(!app || !mRoot)  return ;
   Ogre::Light *light = ConvertFromPointer<Ogre::Light*>(light_ptr);

   if (light == NULL)
      OgreAssert(mTarget, "no light set");

   light->setType(static_cast<Ogre::Light::LightTypes>(type));


}


void SetLightAttenuation(double light_ptr, float range, float constant, float linear, float quadratic)
{
if(!app || !mRoot)  return ;
   Ogre::Light *light = ConvertFromPointer<Ogre::Light*>(light_ptr);

   if (light == NULL)
      OgreAssert(mTarget, "no light set");

   light->setAttenuation(range,constant, linear,quadratic);

}


void SetLightDiffuseColor(double light_ptr,Color c)
{
   if(!app || !mRoot)  return ;
   Ogre::Light *light = ConvertFromPointer<Ogre::Light*>(light_ptr);

   if (light == NULL)
       OgreAssert(mTarget, "no light set");

   light->setDiffuseColour(getColor(c));


}


void SetLightSpecularColor(double light_ptr, Color c)
{
    if(!app || !mRoot)  return ;
   Ogre::Light *light = ConvertFromPointer<Ogre::Light*>(light_ptr);

   if (light == NULL)
      OgreAssert(mTarget, "no light set");

   light->setSpecularColour(getColor(c));


}


void SetLightSpotlightRange(double light_ptr, float inner_angle, float outer_angle, float falloff)
{
   if(!app || !mRoot)  return ;
   Ogre::Light *light = ConvertFromPointer<Ogre::Light*>(light_ptr);

   if (light == NULL)
      OgreAssert(mTarget, "no light set");

   light->setSpotlightRange(Ogre::Degree(inner_angle), Ogre::Degree(outer_angle), falloff);


}


void EnableLightCastShadows(double light_ptr, bool enable)
{
   if(!app || !mRoot)  return ;
   Ogre::Light *light = ConvertFromPointer<Ogre::Light*>(light_ptr);

   if (light == NULL)
      OgreAssert(mTarget, "no light set");

   light->setCastShadows(enable);
}

//********************************************************************************************************TERRAIN


double CreateTerrain(double scene,double light_ptr)
{
  if(!app || !mRoot)  return 0;

    Ogre::SceneManager *sc=ConvertFromPointer<Ogre::SceneManager*>(scene);
    if (sc == NULL)
      return 0;


   Ogre::Light *light = ConvertFromPointer<Ogre::Light*>(light_ptr);
    if (light == NULL)
      return 0;

  rTerrain* terrain= new rTerrain();
  terrain->setupContent(sc,light);



  return ConvertToPointer(terrain);

}
void DestroyTerrain(double terrain_ptr)
{
    if(!app || !mRoot)  return ;

    rTerrain* terrain =ConvertFromPointer<rTerrain*>(terrain_ptr);
    if (terrain == NULL)
      return ;


      terrain->shutdown();

      delete (terrain);


}

}//c


