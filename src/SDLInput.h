

#ifndef SDLUTILSLIB_H
#define SDLUTILSLIB_H

#include <stdlib.h>                 // Required for: srand(), rand(), atexit()
#include <stdio.h>                  // Required for: sprintf() [Used in OpenURL()]
#include <string.h>                 // Required for: strrchr(), strcmp(), strlen()


#include <sys/stat.h>               // Required for: stat() [Used in GetFileModTime()]


#define SMALLOC(sz)       SDL_malloc(sz)
#define SCALLOC(n,sz)     SDL_calloc(n,sz)
#define SREALLOC(ptr,sz)  SDL_realloc(ptr,sz)
#define SFREE(ptr)        SDL_free(ptr)



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


#if (defined(__linux__) || defined(PLATFORM_WEB)) && _POSIX_C_SOURCE < 199309L
    #undef _POSIX_C_SOURCE
    #define _POSIX_C_SOURCE 199309L // Required for CLOCK_MONOTONIC if compiled with c99 without gnu ext.
#endif


#if defined(__STDC__) && __STDC_VERSION__ >= 199901L
    #include <stdbool.h>
#elif !defined(__cplusplus) && !defined(bool)
    typedef enum { false, true } bool;
#endif


#define MAX_TOUCH_POINTS              10
#define MAX_KEY_PRESSED_QUEUE         16
#define MAX_CHAR_PRESSED_QUEUE        16

typedef struct { int x; int y; } Point;
typedef struct { float x; float y; } Pointf;
typedef struct { int  width; int height; } Size;


typedef enum { TOUCH_UP, TOUCH_DOWN, TOUCH_MOVE } TouchAction;


    typedef enum {
        GESTURE_NONE        = 0,
        GESTURE_TAP         = 1,
        GESTURE_DOUBLETAP   = 2,
        GESTURE_HOLD        = 4,
        GESTURE_DRAG        = 8,
        GESTURE_SWIPE_RIGHT = 16,
        GESTURE_SWIPE_LEFT  = 32,
        GESTURE_SWIPE_UP    = 64,
        GESTURE_SWIPE_DOWN  = 128,
        GESTURE_PINCH_IN    = 256,
        GESTURE_PINCH_OUT   = 512
    } Gestures;

// Gesture events
// NOTE: MAX_TOUCH_POINTS fixed to 4
typedef struct {
    int touchAction;
    int pointCount;
    int pointerId[4];
    Pointf position[4];
} GestureEvent;




typedef struct ListNode
{
	int		id;		// id of node
	void*	data;	// data in node
	int type;
	 char* name;

	struct ListNode*	next;	// pointer to next link
	struct ListNode*	prev;	// pointer to previous link
}ListNode;




 typedef struct cList
{
    	struct ListNode*	m_current;	// current link
		struct ListNode*	m_start;	// pointer to start
	    int		m_count;	// number of nodes

}cList;

#define cLISTFOREACH(L, S, M, V) ListNode *_node = NULL;\
    ListNode *V = NULL;\
    for(V = _node = L->S; _node != NULL; V = _node = _node->M)
#define cListCount(A) ((A)->m_count)
#define cListFirst(A) ((A)->m_start != NULL ? (A)->m_start->data : NULL)
#define cListLast(A) ((A)->m_current != NULL ? (A)->m_current->data : NULL)



#if defined(__cplusplus)
extern "C" {            // Prevents name mangling of functions
#endif



cList* cListCreate();
void* cListGet (cList* list, int iID );
void cListDelete (cList* list, int iID );
void cListAdd (cList* list, int num, char* name, void* data ,int type, int iOverwrite );
void cListClear(cList *list);
void cListDestroy(cList* list);

 void   InitInput(SDL_Window* Window);
 void   PollInputEvents(const SDL_Event* event);

 bool IsAnyKeyPressed();
 bool IsKeyPressed(int key);                             // Detect if a key has been pressed once
 bool IsKeyDown(int key);                                // Detect if a key is being pressed
 bool IsKeyReleased(int key);                            // Detect if a key has been released once
 bool IsKeyUp(int key);                                  // Detect if a key is NOT being pressed
 int GetKeyPressed(void);                                // Get key pressed (keycode), call it multiple times for keys queued
 int GetCharPressed(void);                               // Get char pressed (unicode), call it multiple times for chars queued

 bool IsMouseButtonPressed(int button);                  // Detect if a mouse button has been pressed once
 bool IsMouseButtonDown(int button);                     // Detect if a mouse button is being pressed
 bool IsMouseButtonReleased(int button);                 // Detect if a mouse button has been released once
 bool IsMouseButtonUp(int button);                       // Detect if a mouse button is NOT being pressed
 int GetMouseX(void);                                    // Returns mouse position X
 int GetMouseY(void);                                    // Returns mouse position Y
 Point GetMousePosition(void);                         // Returns mouse position XY
 void SetMousePosition(int x, int y);                    // Set mouse position XY
 void SetMouseOffset(int offsetX, int offsetY);          // Set mouse offset
 void SetMouseScale(float scaleX, float scaleY);         // Set mouse scaling
 float GetMouseWheelMove(void);                          // Returns mouse wheel movement Y
 int GetMouseCursor(void);                               // Returns mouse cursor if (MouseCursor enum)
 void SetMouseCursor(int cursor);                        // Set mouse cursor

// Input-related functions: touch
 int GetTouchX(void);                                    // Returns touch position X for touch point 0 (relative to screen size)
 int GetTouchY(void);                                    // Returns touch position Y for touch point 0 (relative to screen size)
 Point GetTouchPosition(int index);                    // Returns touch position XY for a touch point index (relative to screen size)

//------------------------------------------------------------------------------------
// Gestures and Touch Handling Functions (Module: gestures)
//------------------------------------------------------------------------------------
 void SetGesturesEnabled(unsigned int gestureFlags);     // Enable a set of gestures using flags
 bool IsGestureDetected(int gesture);                    // Check if a gesture have been detected
 int GetGestureDetected(void);                           // Get latest detected gesture
 int GetTouchPointsCount(void);                          // Get touch points count
 float GetGestureHoldDuration(void);                     // Get gesture hold time in milliseconds
 Pointf GetGestureDragVector(void);                     // Get gesture drag vector
 float GetGestureDragAngle(void);                        // Get gesture drag angle
 Pointf GetGesturePinchVector(void);                    // Get gesture pinch delta
 float GetGesturePinchAngle(void);                       // Get gesture pinch angle
//DECLSPEC void SDLCALL  InitInput(SDL_Window* Window);

 void *MemAlloc(int size);                                   // Internal memory allocator
 void MemFree(void *ptr);                                    // Internal memory free

 int GetRandomValue(int min, int max);                       // Returns a random value between min and max (both included)

// Files management functions
 unsigned char *LoadFileData(const char *fileName, unsigned int *bytesRead);     // Load file data as byte array (read)
 void UnloadFileData(unsigned char *data);                   // Unload file data allocated by LoadFileData()
 bool SaveFileData(const char *fileName, void *data, unsigned int bytesToWrite); // Save data to file from byte array (write), returns true on success
 char *LoadFileText(const char *fileName);                   // Load text data from file (read), returns a '\0' terminated string
 void UnloadFileText(unsigned char *text);                   // Unload file text data allocated by LoadFileText()
 bool SaveFileText(const char *fileName, char *text);        // Save text data to file (write), string must be '\0' terminated, returns true on success
 bool FileExists(const char *fileName);                      // Check if file exists
 bool DirectoryExists(const char *dirPath);                  // Check if a directory path exists
 bool IsFileExtension(const char *fileName, const char *ext);// Check file extension (including point: .png, .wav)
 const char *GetFileExtension(const char *fileName);         // Get pointer to extension for a filename string (including point: ".png")
 const char *GetFileName(const char *filePath);              // Get pointer to filename for a path string
 const char *GetFileNameWithoutExt(const char *filePath);    // Get filename string without extension (uses static string)
 const char *GetDirectoryPath(const char *filePath);         // Get full path for a given fileName with path (uses static string)
 const char *GetPrevDirectoryPath(const char *dirPath);      // Get previous directory path for a given path (uses static string)
 const char *GetWorkingDirectory(void);                      // Get current working directory (uses static string)
 char **GetDirectoryFiles(const char *dirPath, int *count);  // Get filenames in a directory path (memory should be freed)
 void ClearDirectoryFiles(void);                             // Clear directory files paths buffers (free memory)
 bool ChangeDirectory(const char *dir);                      // Change working directory, return true on success
 long GetFileModTime(const char *fileName);                  // Get file modification time (last write time)

// Persistent storage management
 bool SaveStorageValue(unsigned int position, int value);    // Save integer value to storage file (to defined position), returns true on success
 int LoadStorageValue(unsigned int position);                // Load integer value from storage file (from defined position)

 void OpenURL(const char *url);                              // Open URL with default system browser (if available)


void TimeUpdate();
void TimeBlock();
void Wait(float ms);
 void InitTimer(void);
 void SetTargetFPS(int fps);                                 // Set target FPS (maximum)
 int GetFPS(void);                                           // Returns current FPS
 float GetFrameTime(void);                                   // Returns time in seconds for last frame drawn
 double GetTime(void);                                       // Returns elapsed time in seconds since InitWindow()


// Text strings management functions (no utf8 strings, only byte chars)
// NOTE: Some strings allocate memory internally for returned strings, just be careful!
 int TextCopy(char *dst, const char *src);                                             // Copy one string to another, returns bytes copied
 bool TextIsEqual(const char *text1, const char *text2);                               // Check if two text string are equal
 unsigned int TextLength(const char *text);                                            // Get text length, checks for '\0' ending
 const char *TextFormat(const char *text, ...);                                        // Text formatting with variables (sprintf style)
 const char *TextSubtext(const char *text, int position, int length);                  // Get a piece of a text string
 char *TextReplace(char *text, const char *replace, const char *by);                   // Replace text string (memory must be freed!)
 char *TextInsert(const char *text, const char *insert, int position);                 // Insert text in a position (memory must be freed!)
 const char *TextJoin(const char **textList, int count, const char *delimiter);        // Join text strings with delimiter
 const char **TextSplit(const char *text, char delimiter, int *count);                 // Split text into multiple strings
 void TextAppend(char *text, const char *append, int *position);                       // Append text at specific position and move cursor!
 int TextFindIndex(const char *text, const char *find);                                // Find first text occurrence within a string
 const char *TextToUpper(const char *text);                      // Get upper case version of provided string
 const char *TextToLower(const char *text);                      // Get lower case version of provided string
 const char *TextToPascal(const char *text);                     // Get Pascal case notation version of provided string
 int TextToInteger(const char *text);                            // Get integer value from text (negative values not supported)
char *TextToUtf8(int *codepoints, int length);                  // Encode text codepoint into utf8 text (memory must be freed!)

// UTF8 text strings management functions
 int *GetCodepoints(const char *text, int *count);               // Get all codepoints in a string, codepoints count returned by parameters
 int GetCodepointsCount(const char *text);                       // Get total number of characters (codepoints) in a UTF8 encoded string
 int GetNextCodepoint(const char *text, int *bytesProcessed);    // Returns next codepoint in a UTF8 encoded string; 0x3f('?') is returned on failure
 const char *CodepointToUtf8(int codepoint, int *byteLength);    // Encode codepoint into utf8 text (char array length returned as parameter)
#if defined(__cplusplus)
}
#endif

#endif


#endif
