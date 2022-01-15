
//*   #define GRAPHICS_API_OPENGL_11
//*   #define GRAPHICS_API_OPENGL_21
//*   #define GRAPHICS_API_OPENGL_33
//*   #define GRAPHICS_API_OPENGL_ES2


#ifndef ROGRE_H
#define ROGRE_H

#define RLAPI   // We are building or using rlgl as a static library (or Linux shared library)

#if defined(_WIN32)
    #if defined(BUILD_LIBTYPE_SHARED)
        #define RLAPI __declspec(dllexport)         // We are building raylib as a Win32 shared library (.dll)
    #elif defined(USE_LIBTYPE_SHARED)
        #define RLAPI __declspec(dllimport)         // We are using raylib as a Win32 shared library (.dll)
    #endif
#endif






typedef void (* FuncUpdate)(float,float);


//#define RAYMATH_STANDALONE
//#define RAYMATH_HEADER_ONLY
#define RAYMATH_IMPLEMENTATION
#include "raymath.h"            // Required for: Vector3, Matrix

// Security check in case no GRAPHICS_API_OPENGL_* defined
#if !defined(GRAPHICS_API_OPENGL_11) && \
    !defined(GRAPHICS_API_OPENGL_21) && \
    !defined(GRAPHICS_API_OPENGL_33) && \
    !defined(GRAPHICS_API_OPENGL_ES2)
        #define GRAPHICS_API_OPENGL_33
#endif


// OpenGL 2.1 uses most of OpenGL 3.3 Core functionality
// WARNING: Specific parts are checked with #if defines
#if defined(GRAPHICS_API_OPENGL_21)
    #define GRAPHICS_API_OPENGL_33
#endif

#define SUPPORT_RENDER_TEXTURES_HINT



//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------







#if defined(__STDC__) && __STDC_VERSION__ >= 199901L
    #include <stdbool.h>
#elif !defined(__cplusplus) && !defined(bool)
    typedef enum { false, true } bool;
#endif


#define MAX_TOUCH_POINTS              10
#define MAX_KEY_PRESSED_QUEUE         16
#define MAX_CHAR_PRESSED_QUEUE        16



#define KEY_RIGHT           79
#define KEY_LEFT            80
#define KEY_DOWN            81
#define KEY_UP              82
#define KEY_BACKSPACE       42
#define KEY_ENTER           40
#define MOUSE_LEFT_BUTTON     1
#define KEY_DELETE          76
#define KEY_HOME           74
#define KEY_END             77

#define KEY_LEFT_CONTROL  224
#define KEY_LEFT_SHIFT  225
#define KEY_LEFT_ALT  226
#define KEY_LEFT_GUI  227
#define KEY_RIGHT_CONTROL  228
#define KEY_RIGHT_SHIFT  229
#define KEY_RIGHT_ALT  230

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
    Vector2 position[4];
} GestureEvent;



// Default internal render batch limits

#if defined(GRAPHICS_API_OPENGL_11) || defined(GRAPHICS_API_OPENGL_33)
// This is the maximum amount of elements (quads) per batch
// NOTE: Be careful with text, every letter maps to a quad
#define DEFAULT_BATCH_BUFFER_ELEMENTS   8192
#endif
#if defined(GRAPHICS_API_OPENGL_ES2)
// We reduce memory sizes for embedded systems (RPI and HTML5)
// NOTE: On HTML5 (emscripten) this is allocated on heap,
// by default it's only 16MB!...just take care...
#define DEFAULT_BATCH_BUFFER_ELEMENTS   2048
#endif
#define DEFAULT_BATCH_BUFFERS            1      // Default number of batch buffers (multi-buffering)
#define DEFAULT_BATCH_DRAWCALLS        256      // Default number of batch draw calls (by state changes: mode, texture)
#define MAX_BATCH_ACTIVE_TEXTURES        4      // Maximum number of additional textures that can be activated on batch drawing (SetShaderValueTexture())
#define MAX_MATRIX_STACK_SIZE           32      // Maximum size of Matrix stack
#define MAX_MESH_VERTEX_BUFFERS          7      // Maximum vertex buffers (VBO) per mesh
#define MAX_SHADER_LOCATIONS            32      // Maximum number of shader locations supported
#define MAX_MATERIAL_MAPS               12      // Maximum number of shader maps supported
#define RL_CULL_DISTANCE_NEAR         0.01      // Default near cull distance
#define RL_CULL_DISTANCE_FAR        1000.0      // Default far cull distance
// Texture parameters (equivalent to OpenGL defines)
#define RL_TEXTURE_WRAP_S               0x2802      // GL_TEXTURE_WRAP_S
#define RL_TEXTURE_WRAP_T               0x2803      // GL_TEXTURE_WRAP_T
#define RL_TEXTURE_MAG_FILTER           0x2800      // GL_TEXTURE_MAG_FILTER
#define RL_TEXTURE_MIN_FILTER           0x2801      // GL_TEXTURE_MIN_FILTER

#define RL_TEXTURE_FILTER_NEAREST               0x2600      // GL_NEAREST
#define RL_TEXTURE_FILTER_LINEAR                0x2601      // GL_LINEAR
#define RL_TEXTURE_FILTER_MIP_NEAREST           0x2700      // GL_NEAREST_MIPMAP_NEAREST
#define RL_TEXTURE_FILTER_NEAREST_MIP_LINEAR    0x2702      // GL_NEAREST_MIPMAP_LINEAR
#define RL_TEXTURE_FILTER_LINEAR_MIP_NEAREST    0x2701      // GL_LINEAR_MIPMAP_NEAREST
#define RL_TEXTURE_FILTER_MIP_LINEAR            0x2703      // GL_LINEAR_MIPMAP_LINEAR
#define RL_TEXTURE_FILTER_ANISOTROPIC           0x3000      // Anisotropic filter (custom identifier)

#define RL_TEXTURE_WRAP_REPEAT                  0x2901      // GL_REPEAT
#define RL_TEXTURE_WRAP_CLAMP                   0x812F      // GL_CLAMP_TO_EDGE
#define RL_TEXTURE_WRAP_MIRROR_REPEAT           0x8370      // GL_MIRRORED_REPEAT
#define RL_TEXTURE_WRAP_MIRROR_CLAMP            0x8742      // GL_MIRROR_CLAMP_EXT

// Matrix modes (equivalent to OpenGL)
#define RL_MODELVIEW                    0x1700      // GL_MODELVIEW
#define RL_PROJECTION                   0x1701      // GL_PROJECTION
#define RL_TEXTURE                      0x1702      // GL_TEXTURE

// Primitive assembly draw modes
#define RL_LINES                        0x0001      // GL_LINES
#define RL_TRIANGLES                    0x0004      // GL_TRIANGLES
#define RL_QUADS                        0x0007      // GL_QUADS

// GL equivalent data types
#define RL_UNSIGNED_BYTE                0x1401      // GL_UNSIGNED_BYTE
#define RL_FLOAT                        0x1406      // GL_FLOAT

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum { OPENGL_11 = 1, OPENGL_21, OPENGL_33, OPENGL_ES_20 } GlVersion;

typedef enum {
    RL_ATTACHMENT_COLOR_CHANNEL0 = 0,
    RL_ATTACHMENT_COLOR_CHANNEL1,
    RL_ATTACHMENT_COLOR_CHANNEL2,
    RL_ATTACHMENT_COLOR_CHANNEL3,
    RL_ATTACHMENT_COLOR_CHANNEL4,
    RL_ATTACHMENT_COLOR_CHANNEL5,
    RL_ATTACHMENT_COLOR_CHANNEL6,
    RL_ATTACHMENT_COLOR_CHANNEL7,
    RL_ATTACHMENT_DEPTH = 100,
    RL_ATTACHMENT_STENCIL = 200,
} FramebufferAttachType;

typedef enum {
    RL_ATTACHMENT_CUBEMAP_POSITIVE_X = 0,
    RL_ATTACHMENT_CUBEMAP_NEGATIVE_X,
    RL_ATTACHMENT_CUBEMAP_POSITIVE_Y,
    RL_ATTACHMENT_CUBEMAP_NEGATIVE_Y,
    RL_ATTACHMENT_CUBEMAP_POSITIVE_Z,
    RL_ATTACHMENT_CUBEMAP_NEGATIVE_Z,
    RL_ATTACHMENT_TEXTURE2D = 100,
    RL_ATTACHMENT_RENDERBUFFER = 200,
} FramebufferAttachTextureType;

// Dynamic vertex buffers (position + texcoords + colors + indices arrays)
typedef struct VertexBuffer {
    int elementsCount;          // Number of elements in the buffer (QUADS)

    int vCounter;               // Vertex position counter to process (and draw) from full buffer
    int tcCounter;              // Vertex texcoord counter to process (and draw) from full buffer
    int cCounter;               // Vertex color counter to process (and draw) from full buffer

    float *vertices;            // Vertex position (XYZ - 3 components per vertex) (shader-location = 0)
    float *texcoords;           // Vertex texture coordinates (UV - 2 components per vertex) (shader-location = 1)
    unsigned char *colors;      // Vertex colors (RGBA - 4 components per vertex) (shader-location = 3)
#if defined(GRAPHICS_API_OPENGL_11) || defined(GRAPHICS_API_OPENGL_33)
    unsigned int *indices;      // Vertex indices (in case vertex data comes indexed) (6 indices per quad)
#endif
#if defined(GRAPHICS_API_OPENGL_ES2)
    unsigned short *indices;    // Vertex indices (in case vertex data comes indexed) (6 indices per quad)
#endif
    unsigned int vaoId;         // OpenGL Vertex Array Object id
    unsigned int vboId[4];      // OpenGL Vertex Buffer Objects id (4 types of vertex data)
} VertexBuffer;

// Draw call type
// NOTE: Only texture changes register a new draw, other state-change-related elements are not
// used at this moment (vaoId, shaderId, matrices), raylib just forces a batch draw call if any
// of those state-change happens (this is done in core module)
typedef struct DrawCall {
    int mode;                   // Drawing mode: LINES, TRIANGLES, QUADS
    int vertexCount;            // Number of vertex of the draw
    int vertexAlignment;        // Number of vertex required for index alignment (LINES, TRIANGLES)
    //unsigned int vaoId;       // Vertex array id to be used on the draw -> Using RLGL.currentBatch->vertexBuffer.vaoId
    //unsigned int shaderId;    // Shader id to be used on the draw -> Using RLGL.currentShader.id
    unsigned int textureId;     // Texture id to be used on the draw -> Use to create new draw call if changes

    //Matrix projection;        // Projection matrix for this draw -> Using RLGL.projection by default
    //Matrix modelview;         // Modelview matrix for this draw -> Using RLGL.modelview by default
} DrawCall;

// RenderBatch type
typedef struct RenderBatch {
    int buffersCount;           // Number of vertex buffers (multi-buffering support)
    int currentBuffer;          // Current buffer tracking in case of multi-buffering
    VertexBuffer *vertexBuffer; // Dynamic buffer(s) for vertex data

    DrawCall *draws;            // Draw calls array, depends on textureId
    int drawsCounter;           // Draw calls counter
    float currentDepth;         // Current depth value for next draw
} RenderBatch;

// Shader attribute data types
typedef enum {
    SHADER_ATTRIB_FLOAT = 0,
    SHADER_ATTRIB_VEC2,
    SHADER_ATTRIB_VEC3,
    SHADER_ATTRIB_VEC4
} ShaderAttributeDataType;

    #ifndef __cplusplus
    // Boolean type
  //  typedef enum { false, true } bool;
    #endif

    typedef struct Rectangle {
    float x;
    float y;
    float width;
    float height;
} Rectangle;

    // Color type, RGBA (32bit)
    typedef struct Color {
        unsigned char r;
        unsigned char g;
        unsigned char b;
        unsigned char a;
    } Color;


    #if defined(__cplusplus)
    #define CLITERAL(type)      type
#else
    #define CLITERAL(type)      (type)
#endif

// Some Basic Colors
// NOTE: Custom raylib color palette for amazing visuals on WHITE background
#define LIGHTGRAY  CLITERAL(Color){ 200, 200, 200, 255 }   // Light Gray
#define GRAY       CLITERAL(Color){ 130, 130, 130, 255 }   // Gray
#define DARKGRAY   CLITERAL(Color){ 80, 80, 80, 255 }      // Dark Gray
#define YELLOW     CLITERAL(Color){ 253, 249, 0, 255 }     // Yellow
#define GOLD       CLITERAL(Color){ 255, 203, 0, 255 }     // Gold
#define ORANGE     CLITERAL(Color){ 255, 161, 0, 255 }     // Orange
#define PINK       CLITERAL(Color){ 255, 109, 194, 255 }   // Pink
#define RED        CLITERAL(Color){ 230, 41, 55, 255 }     // Red
#define MAROON     CLITERAL(Color){ 190, 33, 55, 255 }     // Maroon
#define GREEN      CLITERAL(Color){ 0, 228, 48, 255 }      // Green
#define LIME       CLITERAL(Color){ 0, 158, 47, 255 }      // Lime
#define DARKGREEN  CLITERAL(Color){ 0, 117, 44, 255 }      // Dark Green
#define SKYBLUE    CLITERAL(Color){ 102, 191, 255, 255 }   // Sky Blue
#define BLUE       CLITERAL(Color){ 0, 121, 241, 255 }     // Blue
#define DARKBLUE   CLITERAL(Color){ 0, 82, 172, 255 }      // Dark Blue
#define PURPLE     CLITERAL(Color){ 200, 122, 255, 255 }   // Purple
#define VIOLET     CLITERAL(Color){ 135, 60, 190, 255 }    // Violet
#define DARKPURPLE CLITERAL(Color){ 112, 31, 126, 255 }    // Dark Purple
#define BEIGE      CLITERAL(Color){ 211, 176, 131, 255 }   // Beige
#define BROWN      CLITERAL(Color){ 127, 106, 79, 255 }    // Brown
#define DARKBROWN  CLITERAL(Color){ 76, 63, 47, 255 }      // Dark Brown

#define WHITE      CLITERAL(Color){ 255, 255, 255, 255 }   // White
#define BLACK      CLITERAL(Color){ 0, 0, 0, 255 }         // Black
#define BLANK      CLITERAL(Color){ 0, 0, 0, 0 }           // Blank (Transparent)
#define MAGENTA    CLITERAL(Color){ 255, 0, 255, 255 }     // Magenta
#define RAYWHITE   CLITERAL(Color){ 245, 245, 245, 255 }   // My own White (raylib logo)

    // Texture type
    // NOTE: Data stored in GPU memory
    typedef struct Texture2D {
        unsigned int id;        // OpenGL texture id
        int width;              // Texture base width
        int height;             // Texture base height
        int mipmaps;            // Mipmap levels, 1 by default
        int format;             // Data format (PixelFormat)
    } Texture2D;

    // Font character info
typedef struct CharInfo {
    int value;              // Character value (Unicode)
    int offsetX;            // Character offset X when drawing
    int offsetY;            // Character offset Y when drawing
    int advanceX;           // Character advance position X
  //  Image image;            // Character image data
} CharInfo;

// Font type, includes texture and charSet array data
typedef struct TextFont {
    int baseSize;           // Base size (default chars height)
    int charsCount;         // Number of characters
    int charsPadding;       // Padding around the chars
    Texture2D texture;      // Characters texture atlas
    Rectangle *recs;        // Characters rectangles in texture
    CharInfo *chars;        // Characters info data
} TextFont;

// Font type, defines generation method
typedef enum {
    FONT_DEFAULT = 0,       // Default font generation, anti-aliased
    FONT_BITMAP,            // Bitmap font generation, no anti-aliasing
    FONT_SDF                // SDF font generation, requires external shader
} FontType;

    // Shader type (generic)
    typedef struct Shader {
        unsigned int id;        // Shader program id
        int *locs;              // Shader locations array (MAX_SHADER_LOCATIONS)
    } Shader;


    // Texture formats (support depends on OpenGL version)
    typedef enum {
        PIXELFORMAT_UNCOMPRESSED_GRAYSCALE = 1,     // 8 bit per pixel (no alpha)
        PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA,
        PIXELFORMAT_UNCOMPRESSED_R5G6B5,            // 16 bpp
        PIXELFORMAT_UNCOMPRESSED_R8G8B8,            // 24 bpp
        PIXELFORMAT_UNCOMPRESSED_R5G5B5A1,          // 16 bpp (1 bit alpha)
        PIXELFORMAT_UNCOMPRESSED_R4G4B4A4,          // 16 bpp (4 bit alpha)
        PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,          // 32 bpp
        PIXELFORMAT_UNCOMPRESSED_R32,               // 32 bpp (1 channel - float)
        PIXELFORMAT_UNCOMPRESSED_R32G32B32,         // 32*3 bpp (3 channels - float)
        PIXELFORMAT_UNCOMPRESSED_R32G32B32A32,      // 32*4 bpp (4 channels - float)
        PIXELFORMAT_COMPRESSED_DXT1_RGB,            // 4 bpp (no alpha)
        PIXELFORMAT_COMPRESSED_DXT1_RGBA,           // 4 bpp (1 bit alpha)
        PIXELFORMAT_COMPRESSED_DXT3_RGBA,           // 8 bpp
        PIXELFORMAT_COMPRESSED_DXT5_RGBA,           // 8 bpp
        PIXELFORMAT_COMPRESSED_ETC1_RGB,            // 4 bpp
        PIXELFORMAT_COMPRESSED_ETC2_RGB,            // 4 bpp
        PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA,       // 8 bpp
        PIXELFORMAT_COMPRESSED_PVRT_RGB,            // 4 bpp
        PIXELFORMAT_COMPRESSED_PVRT_RGBA,           // 4 bpp
        PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA,       // 8 bpp
        PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA        // 2 bpp
    } PixelFormat;

    // Texture parameters: filter mode
    // NOTE 1: Filtering considers mipmaps if available in the texture
    // NOTE 2: Filter is accordingly set for minification and magnification
    typedef enum {
        TEXTURE_FILTER_POINT = 0,               // No filter, just pixel aproximation
        TEXTURE_FILTER_BILINEAR,                // Linear filtering
        TEXTURE_FILTER_TRILINEAR,               // Trilinear filtering (linear with mipmaps)
        TEXTURE_FILTER_ANISOTROPIC_4X,          // Anisotropic filtering 4x
        TEXTURE_FILTER_ANISOTROPIC_8X,          // Anisotropic filtering 8x
        TEXTURE_FILTER_ANISOTROPIC_16X,         // Anisotropic filtering 16x
    } TextureFilter;

    // Texture parameters: wrap mode
    typedef enum {
        TEXTURE_WRAP_REPEAT = 0,        // Repeats texture in tiled mode
        TEXTURE_WRAP_CLAMP,             // Clamps texture to edge pixel in tiled mode
        TEXTURE_WRAP_MIRROR_REPEAT,     // Mirrors and repeats the texture in tiled mode
        TEXTURE_WRAP_MIRROR_CLAMP       // Mirrors and clamps to border the texture in tiled mode
    } TextureWrap;

    // Color blending modes (pre-defined)
    typedef enum {
        BLEND_ALPHA = 0,                // Blend textures considering alpha (default)
        BLEND_ADDITIVE,                 // Blend textures adding colors
        BLEND_MULTIPLIED,               // Blend textures multiplying colors
        BLEND_ADD_COLORS,               // Blend textures adding colors (alternative)
        BLEND_SUBTRACT_COLORS,          // Blend textures subtracting colors (alternative)
        BLEND_CUSTOM                    // Belnd textures using custom src/dst factors (use SetBlendModeCustom())
    } BlendMode;

    // Shader location point type
    typedef enum {
        SHADER_LOC_VERTEX_POSITION = 0,
        SHADER_LOC_VERTEX_TEXCOORD01,
        SHADER_LOC_VERTEX_TEXCOORD02,
        SHADER_LOC_VERTEX_NORMAL,
        SHADER_LOC_VERTEX_TANGENT,
        SHADER_LOC_VERTEX_COLOR,
        SHADER_LOC_MATRIX_MVP,
        SHADER_LOC_MATRIX_MODEL,
        SHADER_LOC_MATRIX_VIEW,
        SHADER_LOC_MATRIX_NORMAL,
        SHADER_LOC_MATRIX_PROJECTION,
        SHADER_LOC_VECTOR_VIEW,
        SHADER_LOC_COLOR_DIFFUSE,
        SHADER_LOC_COLOR_SPECULAR,
        SHADER_LOC_COLOR_AMBIENT,
        SHADER_LOC_MAP_ALBEDO,          // SHADER_LOC_MAP_DIFFUSE
        SHADER_LOC_MAP_METALNESS,       // SHADER_LOC_MAP_SPECULAR
        SHADER_LOC_MAP_NORMAL,
        SHADER_LOC_MAP_ROUGHNESS,
        SHADER_LOC_MAP_OCCLUSION,
        SHADER_LOC_MAP_EMISSION,
        SHADER_LOC_MAP_HEIGHT,
        SHADER_LOC_MAP_CUBEMAP,
        SHADER_LOC_MAP_IRRADIANCE,
        SHADER_LOC_MAP_PREFILTER,
        SHADER_LOC_MAP_BRDF
    } ShaderLocationIndex;

    #define SHADER_LOC_MAP_DIFFUSE      SHADER_LOC_MAP_ALBEDO
    #define SHADER_LOC_MAP_SPECULAR     SHADER_LOC_MAP_METALNESS

    // Shader uniform data types
    typedef enum {
        SHADER_UNIFORM_FLOAT = 0,
        SHADER_UNIFORM_VEC2,
        SHADER_UNIFORM_VEC3,
        SHADER_UNIFORM_VEC4,
        SHADER_UNIFORM_INT,
        SHADER_UNIFORM_IVEC2,
        SHADER_UNIFORM_IVEC3,
        SHADER_UNIFORM_IVEC4,
        SHADER_UNIFORM_SAMPLER2D
    } ShaderUniformDataType;




    //*************************OGRE3D

typedef void VOID;



typedef enum
{
    /// Transform is relative to the local space
    LOCAL,
    /// Transform is relative to the space of the parent node
    PARENT,
    /// Transform is relative to world space
    WORLD
}TransformSpace;


typedef enum
{
    /// Point light sources give off light equally in all directions, so require only position not direction
    POINT = 0,
    /// Directional lights simulate parallel light beams from a distant source, hence have direction but no position
    DIRECTIONAL = 1,
    /// Spotlights simulate a cone of light from a source so require position and direction, plus extra values for falloff
    SPOTLIGHT = 2
}LightType;


typedef enum FogMode
{
    /// No fog. Duh.
    FOG_NONE=0,
    /// Fog density increases  exponentially from the camera (fog = 1/e^(distance * density))
    FOG_EXP,
    /// Fog density increases at the square of FOG_EXP, i.e. even quicker (fog = 1/e^(distance * density)^2)
    FOG_EXP2,
    /// Fog density increases linearly between the start and end distances
    FOG_LINEAR
}FogMode;


#if defined(__cplusplus)
extern "C" {            // Prevents name mangling of functions
#endif


RLAPI void InitSystem();
RLAPI void CreateRender(const char* title, int w, int h);
RLAPI void CloseRender();
RLAPI void CloseSystem();
RLAPI void StartRender();

RLAPI void SetPreRenderFun(FuncUpdate func);
RLAPI void SetPostRenderFun(FuncUpdate func);
RLAPI void SetRenderFun(FuncUpdate func);

//------------------------------------------------------------------------------------
// Functions Declaration - TextureManager operations
//------------------------------------------------------------------------------------

RLAPI void SetTextureDefaultNumMipmaps(int n);


//------------------------------------------------------------------------------------
// Functions Declaration - ResourceGroupManager operations
//------------------------------------------------------------------------------------
RLAPI void SetResourcesInitializeAll();

//------------------------------------------------------------------------------------
// Functions Declaration - Window operations
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
// Functions Declaration - ViewPort operations
//------------------------------------------------------------------------------------
RLAPI double CreateViewport();
RLAPI double CreateViewportEx(float left, float top, float width, float height,int zorder);
RLAPI void SetViewportCamera(double viewport, double camera_ptr);
RLAPI void DestroyViewport(double viewport);
RLAPI void DestroyAllViewports();
RLAPI void SetViewportBackgroundColor(double viewport,Color c);

//------------------------------------------------------------------------------------
// Functions Declaration - Node operations
//------------------------------------------------------------------------------------
RLAPI void NodeAddChild(double n,double c);



//------------------------------------------------------------------------------------
// Functions Declaration - SceneNode operations
//------------------------------------------------------------------------------------
RLAPI double CreateChildNode(double n);
RLAPI void AttachObjectToNode(double node,double entity);
RLAPI void NodeLookAt(double n,float x, float y, float z, TransformSpace space);
RLAPI void NodeLookAtVect(double n,Vector3 target,Vector3 dir, TransformSpace space);

RLAPI void SetNodeScale(double n,float x, float y, float z);
RLAPI void SetNodeScaleVec(double n,Vector3 v);
RLAPI void SetNodeOrientation(double n,float w,float x, float y, float z);
RLAPI void SetNodeOrientationQuat(double n,Quaternion v);
RLAPI void SetNodeTranslate(double node_ptr,float x,float y,float z , TransformSpace space);
RLAPI void SetNodeVecTranslate(double node_ptr,Vector3 position, TransformSpace space);
RLAPI void SetNodePosition(double n,float x, float y, float z);
RLAPI void SetNodePositionVec(double n,Vector3 v);
RLAPI void SetNodeDirection(double n,float x, float y, float z);
RLAPI void SetNodeDirectionVec(double n,Vector3 v);
RLAPI void SetNodeTranslate(double node_ptr,float x,float y,float z , TransformSpace space);
RLAPI void SetNodeVecTranslate(double node_ptr,Vector3 position, TransformSpace space);
RLAPI void SetNodeYaw(double n,float angle, TransformSpace space);
RLAPI void SetNodePitch(double n,float angle, TransformSpace space);
RLAPI void SetNodeRoll(double n,float angle, TransformSpace space);
RLAPI void SetNodeRotate(double n,Vector3 axis,float angle, TransformSpace space);
RLAPI void SetNodeQuatRotate(double n,Quaternion q, TransformSpace space);
RLAPI Quaternion GetNodeOrientation(double n);
RLAPI Vector3 GetNodePosition(double n);
RLAPI Vector3 GetNodeScale(double n);
RLAPI Quaternion GetNodeDerivedOrientation(double n);
RLAPI Vector3 GetNodeDerivedPosition(double n);
RLAPI Vector3 GetNodeDerivedScale(double n);




RLAPI void MoveNode(double node_ptr,Vector3 direction,float dt);
RLAPI void MoveNodeZ(double node_ptr,float speed);
RLAPI void MoveNodeX(double node_ptr,float speed);
RLAPI void MoveNodeY(double node_ptr,float speed);

//------------------------------------------------------------------------------------
// Functions Declaration - Camera operations
//------------------------------------------------------------------------------------
RLAPI void DestroyCamera(double scene,double camera_ptr);
RLAPI void SetCameraAspect(double camera_ptr, float aspect);
RLAPI void SetCameraClipDistances(double camera_ptr, float znear, float zfar);
RLAPI void SetCameraAutoAspectRatio(double camera_ptr, bool value);
RLAPI void SetCameraViewMatrix(double camera_ptr, Matrix mat);
RLAPI void SetCameraLookAt(double camera_ptr, Vector3 position, Vector3 direction, Vector3 up);

RLAPI void  SetNodeCameraControl(double n,float maxSpeed);
RLAPI void SetNodeCameraTarget(double n);
RLAPI void SetCameraOffset(float x,float y, float z);
RLAPI void SetCameraOrbit();
RLAPI void SetCameraFree();
RLAPI void CameraMouseLook(float xrel, float yrel);
RLAPI void MoveCameraDepth(float speed);
RLAPI void MoveCameraForward(float timeSinceLastFrame);
RLAPI void MoveCameraBack(float timeSinceLastFrame);
RLAPI void MoveCameraUp(float timeSinceLastFrame);
RLAPI void MoveCameraDown(float timeSinceLastFrame);
RLAPI void MoveCameraLeft(float timeSinceLastFrame);
RLAPI void MoveCameraRight(float timeSinceLastFrame);

//------------------------------------------------------------------------------------
// Functions Declaration - Entity operations
//------------------------------------------------------------------------------------

RLAPI  void EnableEntityCastShadows(double e, bool enable);
RLAPI  void SetEntityMaterialName(double e,const char* name);

//------------------------------------------------------------------------------------
// Functions Declaration - Mesh operations
//------------------------------------------------------------------------------------

RLAPI void DestroyLight(double scene,double light_ptr);
RLAPI void SetLightType(double light_ptr, LightType type);
RLAPI void SetLightAttenuation(double light_ptr, float range, float constant, float linear, float quadratic);
RLAPI void SetLightDiffuseColor(double light_ptr,Color c);
RLAPI void SetLightSpecularColor(double light_ptr, Color c);
RLAPI void SetLightSpotlightRange(double light_ptr, float inner_angle, float outer_angle, float falloff);
RLAPI void EnableLightCastShadows(double light_ptr, bool enable);

//------------------------------------------------------------------------------------
// Functions Declaration - Mesh operations
//------------------------------------------------------------------------------------


RLAPI  double CreatePlaneMesh(const char* name,float width, float height, float xsegments, float ysegments, float utile, float vtile);
//------------------------------------------------------------------------------------
// Functions Declaration - SceneManager operations
//------------------------------------------------------------------------------------
RLAPI double CreateScene();
RLAPI double CreateSceneByName(const char* name);
RLAPI double CreateSceneByMask(const char* name,int mask);
RLAPI  void DestroyScene(double scene);


//FogMode mode = FOG_NONE, Real expDensity = 0.001f,  linearStart = 0.0f,  linearEnd = 1.0f);
RLAPI  void SetSceneFog(double scene,FogMode type,Color color,float expDensity,float linearStart, float linearEnd );


RLAPI void SetSceneSkyBox(double scene,bool enable,const char* name,float distance);
//curvature = 10,tiling = 8,distance = 4000,
RLAPI void SetSceneSkyDome(double scene,bool enable,const char* name,float curvature,float tiling, float distance);

//plane_distance=1000  scale = 1000  tiling = 10 bend=0  xsegments = 1,  ysegments = 1,
RLAPI  void SetSceneSkyPlane(double scene,bool enable,const char* name,float plane_distance,float scale,float tiling , float bend, int xsegments, int ysegments);

RLAPI double GetSceneRootNode(double scene);
RLAPI double AddEntityToScene(double scene,double e);



RLAPI void   SetSceneAmbientLight(double scene,Color c);
RLAPI double CreateCamera(double scene,const char* name);
RLAPI  double CreateCameraEx(double scene,const char* name,double aspect, double znear, double zfar, double fov);

RLAPI double CreateEntity(double scene,const char* name);
RLAPI double CreateEntityFromMesh(double scene,const char* name,double mesh);
RLAPI double CreateEntityFromFile(double scene,const char* filename);
RLAPI double CreateLight(double scene,const char* name);
RLAPI double CreateLightType(double scene,const char* name,LightType lt);

//------------------------------------------------------------------------------------
// Functions Declaration - Terrain operations
//------------------------------------------------------------------------------------
RLAPI double CreateTerrain(double scene,double light_ptr);
RLAPI void DestroyTerrain(double terrain_ptr);

//------------------------------------------------------------------------------------
// Functions Declaration - Matrix operations
//------------------------------------------------------------------------------------
RLAPI void rlMatrixMode(int mode);                    // Choose the current matrix to be transformed
RLAPI void rlPushMatrix(void);                        // Push the current matrix to stack
RLAPI void rlPopMatrix(void);                         // Pop lattest inserted matrix from stack
RLAPI void rlLoadIdentity(void);                      // Reset current matrix to identity matrix
RLAPI void rlTranslatef(float x, float y, float z);   // Multiply the current matrix by a translation matrix
RLAPI void rlRotatef(float angleDeg, float x, float y, float z);  // Multiply the current matrix by a rotation matrix
RLAPI void rlScalef(float x, float y, float z);       // Multiply the current matrix by a scaling matrix
RLAPI void rlMultMatrixf(float *matf);                // Multiply the current matrix by another matrix
RLAPI void rlFrustum(double left, double right, double bottom, double top, double znear, double zfar);
RLAPI void rlOrtho(double left, double right, double bottom, double top, double znear, double zfar);
RLAPI void rlViewport(int x, int y, int width, int height); // Set the viewport area

//------------------------------------------------------------------------------------
// Functions Declaration - Vertex level operations
//------------------------------------------------------------------------------------
RLAPI void rlBegin(int mode);                         // Initialize drawing mode (how to organize vertex)
RLAPI void rlEnd(void);                               // Finish vertex providing
RLAPI void rlVertex2i(int x, int y);                  // Define one vertex (position) - 2 int
RLAPI void rlVertex2f(float x, float y);              // Define one vertex (position) - 2 float
RLAPI void rlVertex3f(float x, float y, float z);     // Define one vertex (position) - 3 float
RLAPI void rlTexCoord2f(float x, float y);            // Define one vertex (texture coordinate) - 2 float
RLAPI void rlNormal3f(float x, float y, float z);     // Define one vertex (normal) - 3 float
RLAPI void rlColor4ub(unsigned char r, unsigned char g, unsigned char b, unsigned char a);  // Define one vertex (color) - 4 byte
RLAPI void rlColor3f(float x, float y, float z);          // Define one vertex (color) - 3 float
RLAPI void rlColor4f(float x, float y, float z, float w); // Define one vertex (color) - 4 float

//------------------------------------------------------------------------------------
// Functions Declaration - Core level operations
//------------------------------------------------------------------------------------

RLAPI void EnableCursor();
RLAPI void DisableCursor();

//------------------------------------------------------------------------------------
// Functions Declaration - OpenGL style functions (common to 1.1, 3.3+, ES2)
// NOTE: This functions are used to completely abstract raylib code from OpenGL layer,
// some of them are direct wrappers over OpenGL calls, some others are custom
//------------------------------------------------------------------------------------

// Vertex buffers state
RLAPI bool rlEnableVertexArray(unsigned int vaoId);     // Enable vertex array (VAO, if supported)
RLAPI void rlDisableVertexArray(void);                  // Disable vertex array (VAO, if supported)
RLAPI void rlEnableVertexBuffer(unsigned int id);       // Enable vertex buffer (VBO)
RLAPI void rlDisableVertexBuffer(void);                 // Disable vertex buffer (VBO)
RLAPI void rlEnableVertexBufferElement(unsigned int id);// Enable vertex buffer element (VBO element)
RLAPI void rlDisableVertexBufferElement(void);          // Disable vertex buffer element (VBO element)
RLAPI void rlEnableVertexAttribute(unsigned int index); // Enable vertex attribute index
RLAPI void rlDisableVertexAttribute(unsigned int index);// Disable vertex attribute index


// Textures state
RLAPI void rlActiveTextureSlot(int slot);               // Select and active a texture slot
RLAPI void rlEnableTexture(unsigned int id);            // Enable texture
RLAPI void rlDisableTexture(void);                      // Disable texture
RLAPI void rlEnableTextureCubemap(unsigned int id);     // Enable texture cubemap
RLAPI void rlDisableTextureCubemap(void);               // Disable texture cubemap
RLAPI void rlTextureParameters(unsigned int id, int param, int value); // Set texture parameters (filter, wrap)

// Shader state
RLAPI void rlEnableShader(unsigned int id);             // Enable shader program
RLAPI void rlDisableShader(void);                       // Disable shader program

// Framebuffer state
RLAPI void rlEnableFramebuffer(unsigned int id);        // Enable render texture (fbo)
RLAPI void rlDisableFramebuffer(void);                  // Disable render texture (fbo), return to default framebuffer

// General render state
RLAPI void rlEnableDepthTest(void);                     // Enable depth test
RLAPI void rlDisableDepthTest(void);                    // Disable depth test
RLAPI void rlEnableDepthMask(void);                     // Enable depth write
RLAPI void rlDisableDepthMask(void);                    // Disable depth write
RLAPI void rlEnableBackfaceCulling(void);               // Enable backface culling
RLAPI void rlDisableBackfaceCulling(void);              // Disable backface culling
RLAPI void rlEnableScissorTest(void);                   // Enable scissor test
RLAPI void rlDisableScissorTest(void);                  // Disable scissor test
RLAPI void rlScissor(int x, int y, int width, int height); // Scissor test
RLAPI void rlEnableWireMode(void);                      // Enable wire mode
RLAPI void rlDisableWireMode(void);                     // Disable wire mode
RLAPI void rlSetLineWidth(float width);                 // Set the line drawing width
RLAPI float rlGetLineWidth(void);                       // Get the line drawing width
RLAPI void rlEnableSmoothLines(void);                   // Enable line aliasing
RLAPI void rlDisableSmoothLines(void);                  // Disable line aliasing
RLAPI void rlEnableStereoRender(void);                  // Enable stereo rendering
RLAPI void rlDisableStereoRender(void);                 // Disable stereo rendering
RLAPI bool rlIsStereoRenderEnabled(void);               // Check if stereo render is enabled

RLAPI void BeginScissorMode(int x, int y, int width, int height);
RLAPI void EndScissorMode(void);


RLAPI void rlClearColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a); // Clear color buffer with color
RLAPI void rlClearScreenBuffers(void);                  // Clear used screen buffers (color and depth)
RLAPI void rlCheckErrors(void);                         // Check and log OpenGL error codes
RLAPI void rlSetBlendMode(int mode);                    // Set blending mode
RLAPI void rlSetBlendFactors(int glSrcFactor, int glDstFactor, int glEquation); // Set blending mode factor and equation (using OpenGL factors)

//------------------------------------------------------------------------------------
// Functions Declaration - rlgl functionality
//------------------------------------------------------------------------------------
// rlgl initialization functions
RLAPI void rlglInit(int width, int height);           // Initialize rlgl (buffers, shaders, textures, states)
RLAPI void rlglClose(void);                           // De-inititialize rlgl (buffers, shaders, textures)
RLAPI void rlLoadExtensions(void* loader);            // Load OpenGL extensions (loader function pointer required)
RLAPI int rlGetVersion(void);                         // Returns current OpenGL version
RLAPI int rlGetFramebufferWidth(void);                // Get default framebuffer width
RLAPI int rlGetFramebufferHeight(void);               // Get default framebuffer height

RLAPI Shader rlGetShaderDefault(void);                // Get default shader
RLAPI Texture2D rlGetTextureDefault(void);            // Get default texture

// Render batch management
// NOTE: rlgl provides a default render batch to behave like OpenGL 1.1 immediate mode
// but this render batch API is exposed in case of custom batches are required
RLAPI RenderBatch rlLoadRenderBatch(int numBuffers, int bufferElements);  // Load a render batch system
RLAPI void rlUnloadRenderBatch(RenderBatch batch);                        // Unload render batch system
RLAPI void rlDrawRenderBatch(RenderBatch *batch);                         // Draw render batch data (Update->Draw->Reset)
RLAPI void rlSetRenderBatchActive(RenderBatch *batch);                    // Set the active render batch for rlgl (NULL for default internal)
RLAPI void rlDrawRenderBatchActive(void);                                 // Update and draw internal render batch
RLAPI bool rlCheckRenderBatchLimit(int vCount);                           // Check internal buffer overflow for a given number of vertex
RLAPI void rlSetTexture(unsigned int id);           // Set current texture for render batch and check buffers limits

//------------------------------------------------------------------------------------------------------------------------

// Vertex buffers management
RLAPI unsigned int rlLoadVertexArray(void);                               // Load vertex array (vao) if supported
RLAPI unsigned int rlLoadVertexBuffer(void *buffer, int size, bool dynamic);            // Load a vertex buffer attribute
RLAPI unsigned int rlLoadVertexBufferElement(void *buffer, int size, bool dynamic);     // Load a new attributes element buffer
RLAPI void rlUpdateVertexBuffer(int bufferId, void *data, int dataSize, int offset);    // Update GPU buffer with new data
RLAPI void rlUnloadVertexArray(unsigned int vaoId);
RLAPI void rlUnloadVertexBuffer(unsigned int vboId);
RLAPI void rlSetVertexAttribute(unsigned int index, int compSize, int type, bool normalized, int stride, void *pointer);
RLAPI void rlSetVertexAttributeDivisor(unsigned int index, int divisor);
RLAPI void rlSetVertexAttributeDefault(int locIndex, const void *value, int attribType, int count); // Set vertex attribute default value
RLAPI void rlDrawVertexArray(int offset, int count);
RLAPI void rlDrawVertexArrayElements(int offset, int count, void *buffer);
RLAPI void rlDrawVertexArrayInstanced(int offset, int count, int instances);
RLAPI void rlDrawVertexArrayElementsInstanced(int offset, int count, void *buffer, int instances);

// Textures management
RLAPI unsigned int rlLoadTexture(void *data, int width, int height, int format, int mipmapCount); // Load texture in GPU
RLAPI unsigned int rlLoadTextureDepth(int width, int height, bool useRenderBuffer);               // Load depth texture/renderbuffer (to be attached to fbo)
RLAPI unsigned int rlLoadTextureCubemap(void *data, int size, int format);                        // Load texture cubemap
RLAPI void rlUpdateTexture(unsigned int id, int offsetX, int offsetY, int width, int height, int format, const void *data);  // Update GPU texture with new data
RLAPI void rlGetGlTextureFormats(int format, unsigned int *glInternalFormat, unsigned int *glFormat, unsigned int *glType);  // Get OpenGL internal formats
RLAPI void rlUnloadTexture(unsigned int id);                              // Unload texture from GPU memory
RLAPI void rlGenerateMipmaps(Texture2D *texture);                         // Generate mipmap data for selected texture
RLAPI void *rlReadTexturePixels(Texture2D texture);                       // Read texture pixel data
RLAPI unsigned char *rlReadScreenPixels(int width, int height);           // Read screen pixel data (color buffer)

// Framebuffer management (fbo)
RLAPI unsigned int rlLoadFramebuffer(int width, int height);              // Load an empty framebuffer
RLAPI void rlFramebufferAttach(unsigned int fboId, unsigned int texId, int attachType, int texType, int mipLevel);  // Attach texture/renderbuffer to a framebuffer
RLAPI bool rlFramebufferComplete(unsigned int id);                        // Verify framebuffer is complete
RLAPI void rlUnloadFramebuffer(unsigned int id);                          // Delete framebuffer from GPU

// Shaders management
RLAPI unsigned int rlLoadShaderCode(const char *vsCode, const char *fsCode);    // Load shader from code strings
RLAPI unsigned int rlCompileShader(const char *shaderCode, int type);           // Compile custom shader and return shader id (type: GL_VERTEX_SHADER, GL_FRAGMENT_SHADER)
RLAPI unsigned int rlLoadShaderProgram(unsigned int vShaderId, unsigned int fShaderId); // Load custom shader program
RLAPI void rlUnloadShaderProgram(unsigned int id);                              // Unload shader program
RLAPI int rlGetLocationUniform(unsigned int shaderId, const char *uniformName); // Get shader location uniform
RLAPI int rlGetLocationAttrib(unsigned int shaderId, const char *attribName);   // Get shader location attribute
RLAPI void rlSetUniform(int locIndex, const void *value, int uniformType, int count); // Set shader value uniform
RLAPI void rlSetUniformMatrix(int locIndex, Matrix mat);                        // Set shader value matrix
RLAPI void rlSetUniformSampler(int locIndex, unsigned int textureId);           // Set shader value sampler
RLAPI void rlSetShader(Shader shader);                                    // Set shader currently active

// Matrix state management
RLAPI Matrix rlGetMatrixModelview(void);                                  // Get internal modelview matrix
RLAPI Matrix rlGetMatrixProjection(void);                                 // Get internal projection matrix
RLAPI Matrix rlGetMatrixTransform(void);                                  // Get internal accumulated transform matrix
RLAPI Matrix rlGetMatrixProjectionStereo(int eye);                        // Get internal projection matrix for stereo render (selected eye)
RLAPI Matrix rlGetMatrixViewOffsetStereo(int eye);                        // Get internal view offset matrix for stereo render (selected eye)
RLAPI void rlSetMatrixProjection(Matrix proj);                            // Set a custom projection matrix (replaces internal projection matrix)
RLAPI void rlSetMatrixModelview(Matrix view);                             // Set a custom modelview matrix (replaces internal modelview matrix)
RLAPI void rlSetMatrixProjectionStereo(Matrix right, Matrix left);        // Set eyes projection matrices for stereo rendering
RLAPI void rlSetMatrixViewOffsetStereo(Matrix right, Matrix left);        // Set eyes view offsets matrices for stereo rendering

// Quick and dirty cube/quad buffers load->draw->unload
RLAPI void rlLoadDrawCube(void);     // Load and draw a cube
RLAPI void rlLoadDrawQuad(void);     // Load and draw a quad







void DrawCube(Vector3 position, float width, float height, float length, Color color);
void DrawCubeV(Vector3 position, Vector3 size, Color color);
void DrawCubeWires(Vector3 position, float width, float height, float length, Color color);
void DrawCubeWiresV(Vector3 position, Vector3 size, Color color);
void DrawSphereEx(Vector3 centerPos, float radius, int rings, int slices, Color color);
void DrawSphere(Vector3 centerPos, float radius, Color color);
void DrawSphereWires(Vector3 centerPos, float radius, int rings, int slices, Color color);
void DrawCylinder(Vector3 position, float radiusTop, float radiusBottom, float height, int sides, Color color);
void DrawCylinderWires(Vector3 position, float radiusTop, float radiusBottom, float height, int sides, Color color);
void DrawPlane(Vector3 centerPos, Vector2 size, Color color);
void DrawGrid(int slices, float spacing);

void SetShapesTexture(Texture2D texture, Rectangle source);
void DrawCircle(int centerX, int centerY, float radius, Color color);
void DrawRectangleV(float x, float y,float w, float h, Color color);
void DrawRectangle(int posX, int posY, int width, int height, Color color);
void DrawRectangleRec(Rectangle rec, Color color);
void DrawPixelV(Vector2 position, Color color);
void DrawLine(int startPosX, int startPosY, int endPosX, int endPosY, Color color);
void DrawLineV(Vector2 startPos, Vector2 endPos, Color color);
void DrawLineEx(Vector2 startPos, Vector2 endPos, float thick, Color color);


void DrawTexturePro(Texture2D texture, Rectangle source, Rectangle dest, Vector2 origin, float rotation, Color tint);
void DrawTextureSize(Texture2D texture, int posX, int posY,int Width, int Height ,Color tint);
void DrawTextureClip(Texture2D texture, int posX, int posY,int Width, int Height,Rectangle source ,Color tint);
void DrawTextureEx(Texture2D texture, Vector2 position, float rotation, float scale, Color tint);
void DrawTextureV(Texture2D texture, Vector2 position, Color tint);
void DrawTexture(Texture2D texture, int posX, int posY, Color tint);
void DrawTextureRec(Texture2D texture, Rectangle source, Vector2 position, Color tint);
void DrawTextureQuad(Texture2D texture, Vector2 tiling, Vector2 offset, Rectangle quad, Color tint);


int GetRandomValue(int min, int max);
double GetCurrentTime(void);



TextFont GetFontDefault();
void UnloadFontDefault(void);
void LoadFontDefault(void);
int GetGlyphIndex(TextFont font, int codepoint);
Vector2 MeasureTextEx(TextFont font, const char *text, float fontSize, float spacing);
int MeasureText(const char *text, int fontSize);



TextFont LoadFont(const char *fileName);                                                  // Load font from file into GPU memory (VRAM)
TextFont LoadFontEx(const char *fileName, int fontSize, int *fontChars, int charsCount);  // Load font from file with extended parameters
//TextFont LoadFontFromImage(Ogre::Image image, Color key, int firstChar);                        // Load font from Image (XNA style)
//TextFont LoadFontFromMemory(const char *fileType, const unsigned char *fileData, int dataSize, int fontSize, int *fontChars, int charsCount); // Load font from memory buffer, fileType refers to extension: i.e. "ttf"
CharInfo *LoadFontData(const unsigned char *fileData, int dataSize, int fontSize, int *fontChars, int charsCount, int type);      // Load font data for further use
void UnloadFontData(CharInfo *chars, int charsCount);                                 // Unload font chars info data (RAM)
void UnloadFont(TextFont font);                                                           // Unload Font from GPU memory (VRAM)
Vector2 MeasureTextEx(TextFont font, const char *text, float fontSize, float spacing);

int GetGlyphIndex(TextFont font, int codepoint);

void DrawTextCodepoint(TextFont font, int codepoint, Vector2 position, float fontSize, Color tint);
void DrawText(const char *text, int posX, int posY, int fontSize, Color color);       // Draw text (using default font)
 void DrawTextEx(TextFont font, const char *text, Vector2 position, float fontSize, float spacing, Color tint);                // Draw text using font and additional parameters
 void DrawTextRec(TextFont font, const char *text, Rectangle rec, float fontSize, float spacing, bool wordWrap, Color tint);   // Draw text using font inside rectangle limits
 void DrawTextRecEx(TextFont font, const char *text, Rectangle rec, float fontSize, float spacing, bool wordWrap, Color tint,
                         int selectStart, int selectLength, Color selectTint, Color selectBackTint);    // Draw text using font inside rectangle limits with support for text selection


Texture2D LoadTexture(const char* texture_path);

Color Fade(Color color, float alpha);                                 // Returns color with alpha applied, alpha goes from 0.0f to 1.0f
 int ColorToInt(Color color);                                          // Returns hexadecimal value for a Color
 Vector4 ColorNormalize(Color color);                                  // Returns Color normalized as float [0..1]
 Color ColorFromNormalized(Vector4 normalized);                        // Returns Color from normalized values [0..1]
 Vector3 ColorToHSV(Color color);                                      // Returns HSV values for a Color
 Color ColorFromHSV(float hue, float saturation, float value);         // Returns a Color from HSV values
 Color ColorAlpha(Color color, float alpha);                           // Returns color with alpha applied, alpha goes from 0.0f to 1.0f
 Color ColorAlphaBlend(Color dst, Color src, Color tint);              // Returns src alpha-blended into dst color with tint
 Color GetColor(int hexValue);                                         // Get Color structure from hexadecimal value
 Color GetPixelColor(void *srcPtr, int format);                        // Get Color from a source pixel pointer of certain format
 void SetPixelColor(void *dstPtr, Color color, int format);            // Set color formatted into destination pixel pointer
 int GetPixelDataSize(int width, int height, int format);              // Get pixel data size in bytes for certain format


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
 Vector2 GetMousePosition(void);                         // Returns mouse position XY
 void SetMousePosition(int x, int y);                    // Set mouse position XY
 void SetMouseOffset(int offsetX, int offsetY);          // Set mouse offset
 void SetMouseScale(float scaleX, float scaleY);         // Set mouse scaling
 float GetMouseWheelMove(void);                          // Returns mouse wheel movement Y
 int GetMouseCursor(void);                               // Returns mouse cursor if (MouseCursor enum)
 void SetMouseCursor(int cursor);                        // Set mouse cursor

// Input-related functions: touch
 int GetTouchX(void);                                    // Returns touch position X for touch point 0 (relative to screen size)
 int GetTouchY(void);                                    // Returns touch position Y for touch point 0 (relative to screen size)
 Vector2 GetTouchPosition(int index);                    // Returns touch position XY for a touch point index (relative to screen size)

//------------------------------------------------------------------------------------
// Gestures and Touch Handling Functions (Module: gestures)
//------------------------------------------------------------------------------------
 void SetGesturesEnabled(unsigned int gestureFlags);     // Enable a set of gestures using flags
 bool IsGestureDetected(int gesture);                    // Check if a gesture have been detected
 int GetGestureDetected(void);                           // Get latest detected gesture
 int GetTouchPointsCount(void);                          // Get touch points count
 float GetGestureHoldDuration(void);                     // Get gesture hold time in milliseconds
 Vector2 GetGestureDragVector(void);                     // Get gesture drag vector
 float GetGestureDragAngle(void);                        // Get gesture drag angle
 Vector2 GetGesturePinchVector(void);                    // Get gesture pinch delta
 float GetGesturePinchAngle(void);                       // Get gesture pinch angle
//DECLSPEC void SDLCALL  InitInput(SDL_Window* Window);

 void *MemAlloc(int size);                                   // Internal memory allocator
 void MemFree(void *ptr);                                    // Internal memory free


 // Basic shapes collision detection functions
 bool CheckCollisionRecs(Rectangle rec1, Rectangle rec2);                                           // Check collision between two rectangles
 bool CheckCollisionCircles(Vector2 center1, float radius1, Vector2 center2, float radius2);        // Check collision between two circles
 bool CheckCollisionCircleRec(Vector2 center, float radius, Rectangle rec);                         // Check collision between circle and rectangle
 bool CheckCollisionPointRec(Vector2 point, Rectangle rec);                                         // Check if point is inside rectangle
 bool CheckCollisionPointCircle(Vector2 point, Vector2 center, float radius);                       // Check if point is inside circle
 bool CheckCollisionPointTriangle(Vector2 point, Vector2 p1, Vector2 p2, Vector2 p3);               // Check if point is inside a triangle
 bool CheckCollisionLines(Vector2 startPos1, Vector2 endPos1, Vector2 startPos2, Vector2 endPos2, Vector2 *collisionPoint); // Check the collision between two lines defined by two points each, returns collision point by reference
 Rectangle GetCollisionRec(Rectangle rec1, Rectangle rec2);

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

#endif // RLGL_H


