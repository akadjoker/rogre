

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
      #define PLATFORM_DESKTOP
     #include <dirent.h>             // Required for: DIR, opendir(), closedir() [Used in GetDirectoryFiles()]
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

#define MOUSE_LEFT_BUTTON     1

#define KEY_RIGHT           79
#define KEY_LEFT            80
#define KEY_DOWN            81
#define KEY_UP              82
#define KEY_BACKSPACE       42
#define KEY_ENTER           40

#define KEY_DELETE          76
#define KEY_HOME           74
#define KEY_END             77

#define KEY_LEFT_CONTROL  224
#define KEY_LEFT_SHIFT  225
#define KEYLALT  226
#define KEYLGUI  227
#define KEY_RIGHT_CONTROL  228
#define KEY_RIGHT_SHIFT  229
#define KEYRALT  230

#define KEY_A  4
#define KEY_B  5
#define KEY_C  6
#define KEY_D  7
#define KEY_E  8
#define KEY_F  9
#define KEY_G  10
#define KEY_H  11
#define KEY_I  12
#define KEY_J  13
#define KEY_K  14
#define KEY_L  15
#define KEY_M  16
#define KEY_N  17
#define KEY_O  18
#define KEY_P  19
#define KEY_Q  20
#define KEY_R  21
#define KEY_S  22
#define KEY_T  23
#define KEY_U  24
#define KEY_V  25
#define KEY_W  26
#define KEY_X  27
#define KEY_Y  28
#define KEY_Z  29

#define KEY_1  30
#define KEY_2  31
#define KEY_3  32
#define KEY_4  33
#define KEY_5  34
#define KEY_6  35
#define KEY_7  36
#define KEY_8  37
#define KEY_9  38
#define KEY_0  39

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




#if defined(__cplusplus)
extern "C" {            // Prevents name mangling of functions
#endif



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

#if defined(__cplusplus)
}
#endif

#endif

#if defined(UTILS_IMPLEMENTATION)

#if defined(__linux__)
    #define MAX_FILEPATH_LENGTH     4096        // Maximum length for filepaths (Linux PATH_MAX default value)
#else
    #define MAX_FILEPATH_LENGTH      512        // Maximum length supported for filepaths
#endif

    #define MAX_TEXT_BUFFER_LENGTH              1024        // Size of internal static buffers used on some functions:
                                                            // TextFormat(), TextSubtext(), TextToUpper(), TextToLower(), TextToPascal(), TextSplit()
    #define MAX_TEXT_UNICODE_CHARS               512        // Maximum number of unicode codepoints: GetCodepoints()
    #define MAX_TEXTSPLIT_COUNT                  128        // Maximum number of substrings to split: TextSplit()



// Check if the file exists
bool FileExists(const char *fileName)
{
    bool result = false;

#if defined(_WIN32)
    if (_access(fileName, 0) != -1) result = true;
#else
    if (access(fileName, F_OK) != -1) result = true;
#endif

    return result;
}

// Check file extension
// NOTE: Extensions checking is not case-sensitive
bool IsFileExtension(const char *fileName, const char *ext)
{
    bool result = false;
    const char *fileExt = GetFileExtension(fileName);

    if (fileExt != NULL)
    {

        int extCount = 0;
        const char **checkExts = TextSplit(ext, ';', &extCount);

        char fileExtLower[16] = { 0 };
        strcpy(fileExtLower, TextToLower(fileExt));

        for (int i = 0; i < extCount; i++)
        {
            if (TextIsEqual(fileExtLower, TextToLower(checkExts[i] + 1)))
            {
                result = true;
                break;
            }
        }
    }

    return result;
}

// Check if a directory path exists
bool DirectoryExists(const char *dirPath)
{
    bool result = false;
    DIR *dir = opendir(dirPath);

    if (dir != NULL)
    {
        result = true;
        closedir(dir);
    }

    return result;
}

// Get pointer to extension for a filename string
const char *GetFileExtension(const char *fileName)
{
    const char *dot = strrchr(fileName, '.');

    if (!dot || dot == fileName) return NULL;

    return (dot + 1);
}

// String pointer reverse break: returns right-most occurrence of charset in s
static const char *strprbrk(const char *s, const char *charset)
{
    const char *latestMatch = NULL;
    for (; s = strpbrk(s, charset), s != NULL; latestMatch = s++) { }
    return latestMatch;
}

// Get pointer to filename for a path string
const char *GetFileName(const char *filePath)
{
    const char *fileName = NULL;
    if (filePath != NULL) fileName = strprbrk(filePath, "\\/");

    if (!fileName) return filePath;

    return fileName + 1;
}

// Get filename string without extension (uses static string)
const char *GetFileNameWithoutExt(const char *filePath)
{
    #define MAX_FILENAMEWITHOUTEXT_LENGTH   128

    static char fileName[MAX_FILENAMEWITHOUTEXT_LENGTH];
    memset(fileName, 0, MAX_FILENAMEWITHOUTEXT_LENGTH);

    if (filePath != NULL) strcpy(fileName, GetFileName(filePath));   // Get filename with extension

    int len = (int)strlen(fileName);

    for (int i = 0; (i < len) && (i < MAX_FILENAMEWITHOUTEXT_LENGTH); i++)
    {
        if (fileName[i] == '.')
        {
            // NOTE: We break on first '.' found
            fileName[i] = '\0';
            break;
        }
    }

    return fileName;
}

// Get directory for a given filePath
const char *GetDirectoryPath(const char *filePath)
{
    // NOTE: Directory separator is different in Windows and other platforms,
    // fortunately, Windows also support the '/' separator, that's the one should be used
    #if defined(_WIN32)
        char separator = '\\';
    #else
        char separator = '/';
    #endif

    const char *lastSlash = NULL;
    static char dirPath[MAX_FILEPATH_LENGTH];
    memset(dirPath, 0, MAX_FILEPATH_LENGTH);

    // In case provided path does not contain a root drive letter (C:\, D:\) nor leading path separator (\, /),
    // we add the current directory path to dirPath
    if (filePath[1] != ':' && filePath[0] != '\\' && filePath[0] != '/')
    {
        // For security, we set starting path to current directory,
        // obtained path will be concated to this
        dirPath[0] = '.';
        dirPath[1] = '/';
    }

    lastSlash = strprbrk(filePath, "\\/");
    if (lastSlash)
    {
        if (lastSlash == filePath)
        {
            // The last and only slash is the leading one: path is in a root directory
            dirPath[0] = filePath[0];
            dirPath[1] = '\0';
        }
        else
        {
            // NOTE: Be careful, strncpy() is not safe, it does not care about '\0'
            memcpy(dirPath + (filePath[1] != ':' && filePath[0] != '\\' && filePath[0] != '/' ? 2 : 0), filePath, strlen(filePath) - (strlen(lastSlash) - 1));
            dirPath[strlen(filePath) - strlen(lastSlash) + (filePath[1] != ':' && filePath[0] != '\\' && filePath[0] != '/' ? 2 : 0)] = '\0';  // Add '\0' manually
        }
    }

    return dirPath;
}

// Get previous directory path for a given path
const char *GetPrevDirectoryPath(const char *dirPath)
{
    static char prevDirPath[MAX_FILEPATH_LENGTH];
    memset(prevDirPath, 0, MAX_FILEPATH_LENGTH);
    int pathLen = (int)strlen(dirPath);

    if (pathLen <= 3) strcpy(prevDirPath, dirPath);

    for (int i = (pathLen - 1); (i >= 0) && (pathLen > 3); i--)
    {
        if ((dirPath[i] == '\\') || (dirPath[i] == '/'))
        {
            // Check for root: "C:\" or "/"
            if (((i == 2) && (dirPath[1] ==':')) || (i == 0)) i++;

            strncpy(prevDirPath, dirPath, i);
            break;
        }
    }

    return prevDirPath;
}

// Get current working directory
const char *GetWorkingDirectory(void)
{
    static char currentDir[MAX_FILEPATH_LENGTH];
    memset(currentDir, 0, MAX_FILEPATH_LENGTH);

    char *ptr = GETCWD(currentDir, MAX_FILEPATH_LENGTH - 1);

    return ptr;
}

static char **dirFilesPath = NULL;          // Store directory files paths as strings
static int dirFilesCount = 0;               // Count directory files strings


// Get filenames in a directory path (max 512 files)
// NOTE: Files count is returned by parameters pointer
char **GetDirectoryFiles(const char *dirPath, int *fileCount)
{
    #define MAX_DIRECTORY_FILES     512

    ClearDirectoryFiles();

    // Memory allocation for MAX_DIRECTORY_FILES
    dirFilesPath = (char **)SMALLOC(sizeof(char *)*MAX_DIRECTORY_FILES);

    for (int i = 0; i < MAX_DIRECTORY_FILES; i++) dirFilesPath[i] = (char *)SMALLOC(sizeof(char)*MAX_FILEPATH_LENGTH);

    int counter = 0;
    struct dirent *entity;
    DIR *dir = opendir(dirPath);

    if (dir != NULL)  // It's a directory
    {
        // TODO: Reading could be done in two passes,
        // first one to count files and second one to read names
        // That way we can allocate required memory, instead of a limited pool

        while ((entity = readdir(dir)) != NULL)
        {
            strcpy(dirFilesPath[counter], entity->d_name);
            counter++;
        }

        closedir(dir);
    }
    else SDL_Log( "FILEIO: Failed to open requested directory");  // Maybe it's a file...

    dirFilesCount = counter;
    *fileCount = dirFilesCount;

    return dirFilesPath;
}

// Clear directory files paths buffers
void ClearDirectoryFiles(void)
{
    if (dirFilesCount > 0)
    {
        for (int i = 0; i < MAX_DIRECTORY_FILES; i++) SFREE(dirFilesPath[i]);

        SFREE(dirFilesPath);
    }

    dirFilesCount = 0;
}

// Change working directory, returns true on success
bool ChangeDirectory(const char *dir)
{
    bool result = CHDIR(dir);

    if (result != 0) SDL_Log( "SYSTEM: Failed to change to directory: %s", dir);

    return (result == 0);
}


// Load data from file into a buffer
unsigned char *LoadFileData(const char *fileName, unsigned int *bytesRead)
{
   unsigned char *data = NULL;
    *bytesRead = 0;

    if (fileName != NULL)
    {
        SDL_RWops* file= SDL_RWFromFile(fileName, "rb");








        if (file != NULL)
        {
            int size =(int) SDL_RWsize(file);

            if (size > 0)
            {
                data = (unsigned char *)malloc(size*sizeof(unsigned char));

                unsigned int count = (unsigned int) SDL_RWread(file, data, sizeof(unsigned char), size);
                *bytesRead = count;

                if (count != size) SDL_LogWarn(0, "FILEIO: [%s] File partially loaded", fileName);
                else  SDL_LogInfo(0, "FILEIO: [%s] File loaded successfully", fileName);
            }
            else SDL_LogWarn(0, "FILEIO: [%s] Failed to read file", fileName);
            SDL_RWclose(file);
        }
        else SDL_LogWarn(0, "FILEIO: [%s] Failed to open file", fileName);
    }
    else SDL_LogWarn(0, "FILEIO: File name provided is not valid");
    return data;
}

// Unload file data allocated by LoadFileData()
void UnloadFileData(unsigned char *data)
{
    SFREE(data);
}

// Save data to file from buffer
bool SaveFileData(const char *fileName, void *data, unsigned int bytesToWrite)
{
    bool success = false;

    if (fileName != NULL)
    {
        SDL_RWops* file= SDL_RWFromFile(fileName, "wb");



        if (file != NULL)
        {
            //unsigned int count = (unsigned int)fwrite(data, sizeof(unsigned char), bytesToWrite, file);

            unsigned int count = (unsigned int) SDL_RWwrite(file, data, sizeof(unsigned char), bytesToWrite);


            if (count == 0) SDL_LogWarn(0, "FILEIO: [%s] Failed to write file", fileName);
            else if (count != bytesToWrite) SDL_LogWarn(0, "FILEIO: [%s] File partially written", fileName);
            else  SDL_LogInfo(0, "FILEIO: [%s] File saved successfully", fileName);

            int result = SDL_RWclose(file);
            if (result == 0) success = true;
        }
        else SDL_LogWarn(0, "FILEIO: [%s] Failed to open file", fileName);
    }
    else SDL_LogWarn(0, "FILEIO: File name provided is not valid");

    return success;
}

// Load text data from file, returns a '\0' terminated string
// NOTE: text chars array should be freed manually
char *LoadFileText(const char *fileName)
{
    char *text = NULL;

    if (fileName != NULL)
    {

        SDL_RWops* textFile= SDL_RWFromFile(fileName, "rt");



        if (textFile != NULL)
        {
            // WARNING: When reading a file as 'text' file,
            // text mode causes carriage return-linefeed translation...
            // ...but using fseek() should return correct byte-offset

            int size =(int) SDL_RWsize(textFile);


            if (size > 0)
            {
                text = (char *)SMALLOC((size + 1)*sizeof(char));
            //    unsigned int count = (unsigned int)fread(text, sizeof(char), size, textFile);

                unsigned int count = (unsigned int) SDL_RWread(textFile, text, sizeof( char), size);

                // WARNING: \r\n is converted to \n on reading, so,
                // read bytes count gets reduced by the number of lines
                if (count < size) text = (char*)SREALLOC(text, count + 1);

                // Zero-terminate the string
                text[count] = '\0';

                 SDL_LogInfo(0, "FILEIO: [%s] Text file loaded successfully", fileName);
            }
            else SDL_LogWarn(0, "FILEIO: [%s] Failed to read text file", fileName);

            SDL_RWclose(textFile);
        }
        else SDL_LogWarn(0, "FILEIO: [%s] Failed to open text file", fileName);
    }
    else SDL_LogWarn(0, "FILEIO: File name provided is not valid");

    return text;
}

// Unload file text data allocated by LoadFileText()
void UnloadFileText(unsigned char *text)
{
    SFREE(text);
}

// Save text data to file (write), string must be '\0' terminated
bool SaveFileText(const char *fileName, char *text)
{
    bool success = false;

    if (fileName != NULL)
    {

         SDL_RWops* file= SDL_RWFromFile(fileName, "wt");


        if (file != NULL)
        {
             size_t strLen = SDL_strlen( text );
	         int count = SDL_RWwrite( file, text, 1, strLen );


            if (count < 0) SDL_LogWarn(0, "FILEIO: [%s] Failed to write text file", fileName);
            else SDL_LogInfo(0, "FILEIO: [%s] Text file saved successfully", fileName);

            int result = SDL_RWclose(file);
            if (result == 0) success = true;
        }
        else SDL_LogWarn(0, "FILEIO: [%s] Failed to open text file", fileName);
    }
    else SDL_LogWarn(0, "FILEIO: File name provided is not valid");

    return success;
}

// Get file modification time (last write time)
long GetFileModTime(const char *fileName)
{
    struct stat result = { 0 };

    if (stat(fileName, &result) == 0)
    {
        time_t mod = result.st_mtime;

        return (long)mod;
    }

    return 0;
}



// Save integer value to storage file (to defined position)
// NOTE: Storage positions is directly related to file memory layout (4 bytes each integer)
bool SaveStorageValue(unsigned int position, int value)
{
    bool success = false;

#if defined(SUPPORT_DATA_STORAGE)
    char path[512] = { 0 };
#if defined(PLATFORM_ANDROID)
    strcpy(path, SSDL_AndroidGetInternalStoragePath());
    strcat(path, "/");
    strcat(path, STORAGE_DATA_FILE);
#else
    strcpy(path, STORAGE_DATA_FILE);
#endif

    unsigned int dataSize = 0;
    unsigned int newDataSize = 0;
    unsigned char *fileData = LoadFileData(path, &dataSize);
    unsigned char *newFileData = NULL;

    if (fileData != NULL)
    {
        if (dataSize <= (position*sizeof(int)))
        {
            // Increase data size up to position and store value
            newDataSize = (position + 1)*sizeof(int);
            newFileData = (unsigned char *)RL_REALLOC(fileData, newDataSize);

            if (newFileData != NULL)
            {
                // RL_REALLOC succeded
                int *dataPtr = (int *)newFileData;
                dataPtr[position] = value;
            }
            else
            {
                // RL_REALLOC failed
                TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to realloc data (%u), position in bytes (%u) bigger than actual file size", path, dataSize, position*sizeof(int));

                // We store the old size of the file
                newFileData = fileData;
                newDataSize = dataSize;
            }
        }
        else
        {
            // Store the old size of the file
            newFileData = fileData;
            newDataSize = dataSize;

            // Replace value on selected position
            int *dataPtr = (int *)newFileData;
            dataPtr[position] = value;
        }

        success = SaveFileData(path, newFileData, newDataSize);
        RL_FREE(newFileData);
    }
    else
    {
        TRACELOG(LOG_INFO, "FILEIO: [%s] File not found, creating it", path);

        dataSize = (position + 1)*sizeof(int);
        fileData = (unsigned char *)RL_MALLOC(dataSize);
        int *dataPtr = (int *)fileData;
        dataPtr[position] = value;

        success = SaveFileData(path, fileData, dataSize);
        RL_FREE(fileData);
    }
#endif

    return success;
}

// Load integer value from storage file (from defined position)
// NOTE: If requested position could not be found, value 0 is returned
int LoadStorageValue(unsigned int position)
{
    int value = 0;
#if defined(SUPPORT_DATA_STORAGE)
    char path[512] = { 0 };
#if defined(PLATFORM_ANDROID)
    strcpy(path, CORE.Android.internalDataPath);
    strcat(path, "/");
    strcat(path, STORAGE_DATA_FILE);
#else
    strcpy(path, STORAGE_DATA_FILE);
#endif

    unsigned int dataSize = 0;
    unsigned char *fileData = LoadFileData(path, &dataSize);

    if (fileData != NULL)
    {
        if (dataSize < (position*4)) SDL_Log("SYSTEM: Failed to find storage position");
        else
        {
            int *dataPtr = (int *)fileData;
            value = dataPtr[position];
        }

        RL_FREE(fileData);
    }
#endif
    return value;
}

// Open URL with default system browser (if available)
// NOTE: This function is only safe to use if you control the URL given.
// A user could craft a malicious string performing another action.
// Only call this function yourself not with user input or make sure to check the string yourself.
// Ref: https://github.com/raysan5/raylib/issues/686
void OpenURL(const char *url)
{
    // Small security check trying to avoid (partially) malicious code...
    // sorry for the inconvenience when you hit this point...
    if (strchr(url, '\'') != NULL)
    {
        SDL_Log( "SYSTEM: Provided URL is not valid");
    }
    else
    {
#if defined(PLATFORM_DESKTOP)
        char *cmd = (char *)SCALLOC(strlen(url) + 10, sizeof(char));
    #if defined(_WIN32)
        sprintf(cmd, "explorer %s", url);
    #elif defined(__linux__)
        sprintf(cmd, "xdg-open '%s'", url); // Alternatives: firefox, x-www-browser
    #elif defined(__APPLE__)
        sprintf(cmd, "open '%s'", url);
    #endif
        system(cmd);
        SFREE(cmd);
#endif
#if defined(PLATFORM_WEB)
        emscripten_run_script(TextFormat("window.open('%s', '_blank')", url));
#endif
    }
}


//----------------------------------------------------------------------------------
// Text strings management functions
//----------------------------------------------------------------------------------
// Get text length in bytes, check for \0 character
unsigned int TextLength(const char *text)
{
    unsigned int length = 0; //strlen(text)

    if (text != NULL)
    {
        while (*text++) length++;
    }

    return length;
}

// Formatting of text with variables to 'embed'
// WARNING: String returned will expire after this function is called MAX_TEXTFORMAT_BUFFERS times
const char *TextFormat(const char *text, ...)
{

    #define MAX_TEXTFORMAT_BUFFERS 4        // Maximum number of static buffers for text formatting

    // We create an array of buffers so strings don't expire until MAX_TEXTFORMAT_BUFFERS invocations
    static char buffers[MAX_TEXTFORMAT_BUFFERS][MAX_TEXT_BUFFER_LENGTH] = { 0 };
    static int  index = 0;

    char *currentBuffer = buffers[index];
    memset(currentBuffer, 0, MAX_TEXT_BUFFER_LENGTH);   // Clear buffer before using

    va_list args;
    va_start(args, text);
    vsprintf(currentBuffer, text, args);
    va_end(args);

    index += 1;     // Move to next buffer for next function call
    if (index >= MAX_TEXTFORMAT_BUFFERS) index = 0;

    return currentBuffer;
}

// Get integer value from text
// NOTE: This function replaces atoi() [stdlib.h]
int TextToInteger(const char *text)
{
    int value = 0;
    int sign = 1;

    if ((text[0] == '+') || (text[0] == '-'))
    {
        if (text[0] == '-') sign = -1;
        text++;
    }

    for (int i = 0; ((text[i] >= '0') && (text[i] <= '9')); ++i) value = value*10 + (int)(text[i] - '0');

    return value*sign;
}


// Copy one string to another, returns bytes copied
int TextCopy(char *dst, const char *src)
{
    int bytes = 0;

    if (dst != NULL)
    {
        while (*src != '\0')
        {
            *dst = *src;
            dst++;
            src++;

            bytes++;
        }

        *dst = '\0';
    }

    return bytes;
}

// Check if two text string are equal
// REQUIRES: strcmp()
bool TextIsEqual(const char *text1, const char *text2)
{
    bool result = false;

    if (strcmp(text1, text2) == 0) result = true;

    return result;
}

// Get a piece of a text string
const char *TextSubtext(const char *text, int position, int length)
{
    static char buffer[MAX_TEXT_BUFFER_LENGTH] = { 0 };

    int textLength = TextLength(text);

    if (position >= textLength)
    {
        position = textLength - 1;
        length = 0;
    }

    if (length >= textLength) length = textLength;

    for (int c = 0 ; c < length ; c++)
    {
        *(buffer + c) = *(text + position);
        text++;
    }

    *(buffer + length) = '\0';

    return buffer;
}

// Replace text string
// REQUIRES: strstr(), strncpy(), strcpy()
// WARNING: Internally allocated memory must be freed by the user (if return != NULL)
char *TextReplace(char *text, const char *replace, const char *by)
{
    // Sanity checks and initialization
    if (!text || !replace || !by) return NULL;

    char *result;

    char *insertPoint;      // Next insert point
    char *temp;             // Temp pointer
    int replaceLen;         // Replace string length of (the string to remove)
    int byLen;              // Replacement length (the string to replace replace by)
    int lastReplacePos;     // Distance between replace and end of last replace
    int count;              // Number of replacements

    replaceLen = TextLength(replace);
    if (replaceLen == 0) return NULL;  // Empty replace causes infinite loop during count

    byLen = TextLength(by);

    // Count the number of replacements needed
    insertPoint = text;
    for (count = 0; (temp = strstr(insertPoint, replace)); count++) insertPoint = temp + replaceLen;

    // Allocate returning string and point temp to it
    temp = result =(char*) SMALLOC(TextLength(text) + (byLen - replaceLen)*count + 1);

    if (!result) return NULL;   // Memory could not be allocated

    // First time through the loop, all the variable are set correctly from here on,
    //    temp points to the end of the result string
    //    insertPoint points to the next occurrence of replace in text
    //    text points to the remainder of text after "end of replace"
    while (count--)
    {
        insertPoint = strstr(text, replace);
        lastReplacePos = (int)(insertPoint - text);
        temp = strncpy(temp, text, lastReplacePos) + lastReplacePos;
        temp = strcpy(temp, by) + byLen;
        text += lastReplacePos + replaceLen; // Move to next "end of replace"
    }

    // Copy remaind text part after replacement to result (pointed by moving temp)
    strcpy(temp, text);

    return result;
}

// Insert text in a specific position, moves all text forward
// WARNING: Allocated memory should be manually freed
char *TextInsert(const char *text, const char *insert, int position)
{
    int textLen = TextLength(text);
    int insertLen =  TextLength(insert);

    char *result = (char *)SMALLOC(textLen + insertLen + 1);

    for (int i = 0; i < position; i++) result[i] = text[i];
    for (int i = position; i < insertLen + position; i++) result[i] = insert[i];
    for (int i = (insertLen + position); i < (textLen + insertLen); i++) result[i] = text[i];

    result[textLen + insertLen] = '\0';     // Make sure text string is valid!

    return result;
}

// Join text strings with delimiter
// REQUIRES: memset(), memcpy()
const char *TextJoin(const char **textList, int count, const char *delimiter)
{
    static char text[MAX_TEXT_BUFFER_LENGTH] = { 0 };
    memset(text, 0, MAX_TEXT_BUFFER_LENGTH);
    char *textPtr = text;

    int totalLength = 0;
    int delimiterLen = TextLength(delimiter);

    for (int i = 0; i < count; i++)
    {
        int textLength = TextLength(textList[i]);

        // Make sure joined text could fit inside MAX_TEXT_BUFFER_LENGTH
        if ((totalLength + textLength) < MAX_TEXT_BUFFER_LENGTH)
        {
            memcpy(textPtr, textList[i], textLength);
            totalLength += textLength;
            textPtr += textLength;

            if ((delimiterLen > 0) && (i < (count - 1)))
            {
                memcpy(textPtr, delimiter, delimiterLen);
                totalLength += delimiterLen;
                textPtr += delimiterLen;
            }
        }
    }

    return text;
}

// Split string into multiple strings
const char **TextSplit(const char *text, char delimiter, int *count)
{
    // NOTE: Current implementation returns a copy of the provided string with '\0' (string end delimiter)
    // inserted between strings defined by "delimiter" parameter. No memory is dynamically allocated,
    // all used memory is static... it has some limitations:
    //      1. Maximum number of possible split strings is set by MAX_TEXTSPLIT_COUNT
    //      2. Maximum size of text to split is MAX_TEXT_BUFFER_LENGTH

    static const char *result[MAX_TEXTSPLIT_COUNT] = { NULL };
    static char buffer[MAX_TEXT_BUFFER_LENGTH] = { 0 };
    memset(buffer, 0, MAX_TEXT_BUFFER_LENGTH);

    result[0] = buffer;
    int counter = 0;

    if (text != NULL)
    {
        counter = 1;

        // Count how many substrings we have on text and point to every one
        for (int i = 0; i < MAX_TEXT_BUFFER_LENGTH; i++)
        {
            buffer[i] = text[i];
            if (buffer[i] == '\0') break;
            else if (buffer[i] == delimiter)
            {
                buffer[i] = '\0';   // Set an end of string at this point
                result[counter] = buffer + i + 1;
                counter++;

                if (counter == MAX_TEXTSPLIT_COUNT) break;
            }
        }
    }

    *count = counter;
    return result;
}

// Append text at specific position and move cursor!
// REQUIRES: strcpy()
void TextAppend(char *text, const char *append, int *position)
{
    strcpy(text + *position, append);
    *position += TextLength(append);
}

// Find first text occurrence within a string
// REQUIRES: strstr()
int TextFindIndex(const char *text, const char *find)
{
    int position = -1;

    char *ptr = (char*)strstr(text, find);

    if (ptr != NULL) position = (int)(ptr - text);

    return position;
}

// Get upper case version of provided string
const char *TextToUpper(const char *text)
{
    static char buffer[MAX_TEXT_BUFFER_LENGTH] = { 0 };

    for (int i = 0; i < MAX_TEXT_BUFFER_LENGTH; i++)
    {
        if (text[i] != '\0')
        {
            buffer[i] = (char)toupper(text[i]);
            //if ((text[i] >= 'a') && (text[i] <= 'z')) buffer[i] = text[i] - 32;

            // TODO: Support Utf8 diacritics!
            //if ((text[i] >= 'à') && (text[i] <= 'ý')) buffer[i] = text[i] - 32;
        }
        else { buffer[i] = '\0'; break; }
    }

    return buffer;
}

// Get lower case version of provided string
const char *TextToLower(const char *text)
{
    static char buffer[MAX_TEXT_BUFFER_LENGTH] = { 0 };

    for (int i = 0; i < MAX_TEXT_BUFFER_LENGTH; i++)
    {
        if (text[i] != '\0')
        {
            buffer[i] = (char)tolower(text[i]);
            //if ((text[i] >= 'A') && (text[i] <= 'Z')) buffer[i] = text[i] + 32;
        }
        else { buffer[i] = '\0'; break; }
    }

    return buffer;
}

// Get Pascal case notation version of provided string
const char *TextToPascal(const char *text)
{
    static char buffer[MAX_TEXT_BUFFER_LENGTH] = { 0 };

    buffer[0] = (char)toupper(text[0]);

    for (int i = 1, j = 1; i < MAX_TEXT_BUFFER_LENGTH; i++, j++)
    {
        if (text[j] != '\0')
        {
            if (text[j] != '_') buffer[i] = text[j];
            else
            {
                j++;
                buffer[i] = (char)toupper(text[j]);
            }
        }
        else { buffer[i] = '\0'; break; }
    }

    return buffer;
}




typedef struct InputData {


   SDL_Window* win;
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
            Point position;               // Mouse position on screen
            Point offset;                 // Mouse offset
            Point scale;                  // Mouse scaling

            char currentButtonState[3];     // Registers current mouse button state
            char previousButtonState[3];    // Registers previous mouse button state
            float currentWheelMove;         // Registers current mouse wheel variation
            float previousWheelMove;        // Registers previous mouse wheel variation

        } Mouse;
        struct {
            Point position[MAX_TOUCH_POINTS];         // Touch position on screen
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
        Pointf upPosition;             // Touch up position
        Pointf downPositionA;          // First touch down position
        Pointf downPositionB;          // Second touch down position
        Pointf downDragPosition;       // Touch drag position
        Pointf moveDownPositionA;      // First touch down position on move
        Pointf moveDownPositionB;      // Second touch down position on move
        int tapCounter;                 // TAP counter (one tap implies TOUCH_DOWN and TOUCH_UP actions)
    } Touch;
    struct {
        bool resetRequired;             // HOLD reset to get first touch point again
        double timeDuration;            // HOLD duration in milliseconds
    } Hold;
    struct {
        Pointf vector;                 // DRAG vector (between initial and current position)
        float angle;                    // DRAG angle (relative to x-axis)
        float distance;                 // DRAG distance (from initial touch point to final) (normalized [0..1])
        float intensity;                // DRAG intensity, how far why did the DRAG (pixels per frame)
    } Drag;
    struct {
        bool start;                     // SWIPE used to define when start measuring GESTURES.Swipe.timeDuration
        double timeDuration;            // SWIPE time to calculate drag intensity
    } Swipe;
    struct {
        Pointf vector;                 // PINCH vector (between first and second touch points)
        float angle;                    // PINCH angle (relative to x-axis)
        float distance;                 // PINCH displacement distance (normalized [0..1])
    } Pinch;
} GesturesData;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static GesturesData GESTURES ;

// Get current screen width
Size GetWindowSize(void)
{
    Size s;
    SDL_GetWindowSize(Input.win,&s.width,&s.height);
    return s;
}


void InitInput(SDL_Window* Window)
{



    Input.win = Window;
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


double GetCurrentTime(void)
{
 return (double)SDL_GetTicks()/1000;
}


// Enable only desired getures to be detected
void SetGesturesEnabled(unsigned int gestureFlags)
{
    GESTURES.enabledFlags = gestureFlags;
}


// Check if a gesture have been detected
bool IsGestureDetected(int gesture)
{
    if ((GESTURES.enabledFlags & GESTURES.current) == gesture) return true;
    else return false;
}

static float VecAngle(Pointf v1, Pointf v2)
{
    float angle = atan2f(v2.y - v1.y, v2.x - v1.x)*(180.0f/3.14159265358979323846);

    if (angle < 0) angle += 360.0f;

    return angle;
}

// Calculate distance between two Vector2
static float VecDistance(Pointf v1, Pointf v2)
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

            GESTURES.Drag.vector = (Pointf){ 0.0f, 0.0f };
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

            GESTURES.Touch.downDragPosition = (Pointf){ 0.0f, 0.0f };
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
            GESTURES.Pinch.vector = (Pointf){ 0.0f, 0.0f };
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

// Get number of touch points
int GetTouchPointsCount(void)
{
    // NOTE: point count is calculated when ProcessGestureEvent(GestureEvent event) is called

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
Pointf GetGestureDragVector(void)
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
Pointf GetGesturePinchVector(void)
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
Point GetMousePosition(void)
{
     Point position;
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

Point GetTouchPosition(int index)
{
    Point position;
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



// Poll (store) all input events
void PollInputEvents(const SDL_Event* e)
{
    UpdateGestures();

    // Reset keys/chars pressed registered
    Input.Keyboard.keyPressedQueueCount = 0;
    Input.Keyboard.charPressedQueueCount = 0;




#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)
    // Keyboard/Mouse input polling (automatically managed by GLFW3 through callback)

    // Register previous keys states
    for (int i = 0; i < 512; i++) Input.Keyboard.previousKeyState[i] = Input.Keyboard.currentKeyState[i];

    // Register previous mouse states
    for (int i = 0; i < 3; i++) Input.Mouse.previousButtonState[i] = Input.Mouse.currentButtonState[i];

    // Register previous mouse wheel state
    Input.Mouse.previousWheelMove = Input.Mouse.currentWheelMove;
    Input.Mouse.currentWheelMove = 0.0f;
#endif

    // Register previous touch states
    for (int i = 0; i < MAX_TOUCH_POINTS; i++) Input.Touch.previousTouchState[i] = Input.Touch.currentTouchState[i];




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

                         MouseButtonCallback(e->button.button,1,0);
                         MouseCursorPosCallback(e->button.x,e->button.y);


                        }
                        break;
                        case SDL_MOUSEBUTTONUP:
                        {
                         MouseButtonCallback(e->button.button,0,0);
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


#endif
