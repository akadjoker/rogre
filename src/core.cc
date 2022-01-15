
#include <stdio.h>              // Required for: FILE, fopen(), fclose(), fprintf(), feof(), fscanf(), vsprintf() [GuiLoadStyle(), GuiLoadIcons()]
#include <stdlib.h>             // Required for: malloc(), calloc(), free() [GuiLoadStyle(), GuiLoadIcons()]
#include <string.h>             // Required for: strlen() [GuiTextBox(), GuiTextBoxMulti(), GuiValueBox()], memset(), memcpy()
#include <stdarg.h>             // Required for: va_list, va_start(), vfprintf(), va_end() [TextFormat()]
#include <math.h>               // Required for: roundf() [GuiColorPicker()]




#if defined(__cplusplus)
    #define RAYGUI_CLITERAL(name)      name
#else
    #define RAYGUI_CLITERAL(name)      (name)
#endif

#define RICON_SIZE               16       // Size of icons (squared)
#define RICON_MAX_ICONS         256       // Maximum number of icons
#define RICON_MAX_NAME_LENGTH    32       // Maximum length of icon name id

// Icons data is defined by bit array (every bit represents one pixel)
// Those arrays are stored as unsigned int data arrays, so every array
// element defines 32 pixels (bits) of information
// Number of elemens depend on RICON_SIZE (by default 16x16 pixels)
#define RICON_DATA_ELEMENTS   (RICON_SIZE*RICON_SIZE/32)


#if !defined(RLGL_STANDALONE)
    // Check if config flags have been externally provided on compilation line
    #if !defined(EXTERNAL_CONFIG_FLAGS)
        #include "config.h"             // Defines module configuration flags
    #endif
    #include "raymath.h"                // Required for: Vec3 and Matrix functions
#endif

#include <stdlib.h>                     // Required for: malloc(), free()
#include <string.h>                     // Required for: strcmp(), strlen() [Used in rlglInit(), on extensions loading]

#if defined(GRAPHICS_API_OPENGL_11)
    #if defined(__APPLE__)
        #include <OpenGL/gl.h>          // OpenGL 1.1 library for OSX
        #include <OpenGL/glext.h>
    #else
        // APIENTRY for OpenGL function pointer declarations is required
        #ifndef APIENTRY
            #if defined(_WIN32)
                #define APIENTRY __stdcall
            #else
                #define APIENTRY
            #endif
        #endif
        // WINGDIAPI definition. Some Windows OpenGL headers need it
        #if !defined(WINGDIAPI) && defined(_WIN32)
            #define WINGDIAPI __declspec(dllimport)
        #endif

        #include <GL/gl.h>              // OpenGL 1.1 library
    #endif
#endif

#if defined(GRAPHICS_API_OPENGL_33)
    #if defined(__APPLE__)
        #include <OpenGL/gl3.h>         // OpenGL 3 library for OSX
        #include <OpenGL/gl3ext.h>      // OpenGL 3 extensions library for OSX
    #else
        /*#define GLAD_REALLOC RL_REALLOC
        #define GLAD_FREE RL_FREE

        #define GLAD_IMPLEMENTATION
        #if defined(RLGL_STANDALONE)
            #include "glad.h"           // GLAD extensions loading library, includes OpenGL headers
        #else
            #include "external/glad.h"  // GLAD extensions loading library, includes OpenGL headers
        #endif*/
        #include<SDL2/SDL_opengl.h>
    #endif
#endif

#if defined(GRAPHICS_API_OPENGL_ES2)
    #define GL_GLEXT_PROTOTYPES
    #include <EGL/egl.h>                // EGL library
    #include <GLES2/gl2.h>              // OpenGL ES 2.0 library
    #include <GLES2/gl2ext.h>           // OpenGL ES 2.0 extensions library

    // It seems OpenGL ES 2.0 instancing entry points are not defined on Raspberry Pi
    // provided headers (despite being defined in official Khronos GLES2 headers)
    #if defined(PLATFORM_RPI) || defined(PLATFORM_DRM)
    typedef void (GL_APIENTRYP PFNGLDRAWARRAYSINSTANCEDEXTPROC) (GLenum mode, GLint start, GLsizei count, GLsizei primcount);
    typedef void (GL_APIENTRYP PFNGLDRAWELEMENTSINSTANCEDEXTPROC) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount);
    typedef void (GL_APIENTRYP PFNGLVERTEXATTRIBDIVISOREXTPROC) (GLuint index, GLuint divisor);
    #endif
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#ifndef GL_SHADING_LANGUAGE_VERSION
    #define GL_SHADING_LANGUAGE_VERSION         0x8B8C
#endif

#ifndef GL_COMPRESSED_RGB_S3TC_DXT1_EXT
    #define GL_COMPRESSED_RGB_S3TC_DXT1_EXT     0x83F0
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
    #define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT    0x83F1
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
    #define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT    0x83F2
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
    #define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT    0x83F3
#endif
#ifndef GL_ETC1_RGB8_OES
    #define GL_ETC1_RGB8_OES                    0x8D64
#endif
#ifndef GL_COMPRESSED_RGB8_ETC2
    #define GL_COMPRESSED_RGB8_ETC2             0x9274
#endif
#ifndef GL_COMPRESSED_RGBA8_ETC2_EAC
    #define GL_COMPRESSED_RGBA8_ETC2_EAC        0x9278
#endif
#ifndef GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG
    #define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG  0x8C00
#endif
#ifndef GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG
    #define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG 0x8C02
#endif
#ifndef GL_COMPRESSED_RGBA_ASTC_4x4_KHR
    #define GL_COMPRESSED_RGBA_ASTC_4x4_KHR     0x93b0
#endif
#ifndef GL_COMPRESSED_RGBA_ASTC_8x8_KHR
    #define GL_COMPRESSED_RGBA_ASTC_8x8_KHR     0x93b7
#endif

#ifndef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
    #define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT   0x84FF
#endif
#ifndef GL_TEXTURE_MAX_ANISOTROPY_EXT
    #define GL_TEXTURE_MAX_ANISOTROPY_EXT       0x84FE
#endif

#if defined(GRAPHICS_API_OPENGL_11)
    #define GL_UNSIGNED_SHORT_5_6_5             0x8363
    #define GL_UNSIGNED_SHORT_5_5_5_1           0x8034
    #define GL_UNSIGNED_SHORT_4_4_4_4           0x8033
#endif

#if defined(GRAPHICS_API_OPENGL_21)
    #define GL_LUMINANCE                        0x1909
    #define GL_LUMINANCE_ALPHA                  0x190A
#endif

#if defined(GRAPHICS_API_OPENGL_ES2)
    #define glClearDepth                 glClearDepthf
    #define GL_READ_FRAMEBUFFER         GL_FRAMEBUFFER
    #define GL_DRAW_FRAMEBUFFER         GL_FRAMEBUFFER
#endif

// Default shader vertex attribute names to set location points
#ifndef DEFAULT_SHADER_ATTRIB_NAME_POSITION
    #define DEFAULT_SHADER_ATTRIB_NAME_POSITION    "vertexPosition"    // Binded by default to shader location: 0
#endif
#ifndef DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD
    #define DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD    "vertexTexCoord"    // Binded by default to shader location: 1
#endif
#ifndef DEFAULT_SHADER_ATTRIB_NAME_NORMAL
    #define DEFAULT_SHADER_ATTRIB_NAME_NORMAL      "vertexNormal"      // Binded by default to shader location: 2
#endif
#ifndef DEFAULT_SHADER_ATTRIB_NAME_COLOR
    #define DEFAULT_SHADER_ATTRIB_NAME_COLOR       "vertexColor"       // Binded by default to shader location: 3
#endif
#ifndef DEFAULT_SHADER_ATTRIB_NAME_TANGENT
    #define DEFAULT_SHADER_ATTRIB_NAME_TANGENT     "vertexTangent"     // Binded by default to shader location: 4
#endif
#ifndef DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD2
    #define DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD2   "vertexTexCoord2"   // Binded by default to shader location: 5
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
typedef struct rlglData {
    RenderBatch *currentBatch;              // Current render batch
    RenderBatch defaultBatch;               // Default internal render batch

    struct {
        int currentMatrixMode;              // Current matrix mode
        Matrix *currentMatrix;              // Current matrix pointer
        Matrix modelview;                   // Default modelview matrix
        Matrix projection;                  // Default projection matrix
        Matrix transform;                   // Transform matrix to be used with rlTranslate, rlRotate, rlScale
        bool transformRequired;             // Require transform matrix application to current draw-call vertex (if required)
        Matrix stack[MAX_MATRIX_STACK_SIZE];// Matrix stack for push/pop
        int stackCounter;                   // Matrix stack counter

        unsigned int defaultTextureId;      // Default texture used on shapes/poly drawing (required by shader)
        unsigned int activeTextureId[MAX_BATCH_ACTIVE_TEXTURES];    // Active texture ids to be enabled on batch drawing (0 active by default)
        unsigned int defaultVShaderId;      // Default vertex shader id (used by default shader program)
        unsigned int defaultFShaderId;      // Default fragment shader Id (used by default shader program)
        Shader defaultShader;               // Basic shader, support vertex color and diffuse texture
        Shader currentShader;               // Shader to be used on rendering (by default, defaultShader)

        bool stereoRender;                  // Stereo rendering flag
        Matrix projectionStereo[2];         // VR stereo rendering eyes projection matrices
        Matrix viewOffsetStereo[2];         // VR stereo rendering eyes view offset matrices

        int currentBlendMode;               // Blending mode active
        int glBlendSrcFactor;               // Blending source factor
        int glBlendDstFactor;               // Blending destination factor
        int glBlendEquation;                // Blending equation

        int framebufferWidth;               // Default framebuffer width
        int framebufferHeight;              // Default framebuffer height

    } State;            // Renderer state
    struct {
        bool vao;                           // VAO support (OpenGL ES2 could not support VAO extension) (GL_ARB_vertex_array_object)
        bool instancing;                    // Instancing supported (GL_ANGLE_instanced_arrays, GL_EXT_draw_instanced + GL_EXT_instanced_arrays)
        bool texNPOT;                       // NPOT textures full support (GL_ARB_texture_non_power_of_two, GL_OES_texture_npot)
        bool texDepth;                      // Depth textures supported (GL_ARB_depth_texture, GL_WEBGL_depth_texture, GL_OES_depth_texture)
        bool texFloat32;                    // float textures support (32 bit per channel) (GL_OES_texture_float)
        bool texCompDXT;                    // DDS texture compression support (GL_EXT_texture_compression_s3tc, GL_WEBGL_compressed_texture_s3tc, GL_WEBKIT_WEBGL_compressed_texture_s3tc)
        bool texCompETC1;                   // ETC1 texture compression support (GL_OES_compressed_ETC1_RGB8_texture, GL_WEBGL_compressed_texture_etc1)
        bool texCompETC2;                   // ETC2/EAC texture compression support (GL_ARB_ES3_compatibility)
        bool texCompPVRT;                   // PVR texture compression support (GL_IMG_texture_compression_pvrtc)
        bool texCompASTC;                   // ASTC texture compression support (GL_KHR_texture_compression_astc_hdr, GL_KHR_texture_compression_astc_ldr)
        bool texMirrorClamp;                // Clamp mirror wrap mode supported (GL_EXT_texture_mirror_clamp)
        bool texAnisoFilter;                // Anisotropic texture filtering support (GL_EXT_texture_filter_anisotropic)

        float maxAnisotropicLevel;          // Maximum anisotropy level supported (minimum is 2.0f)
        int maxDepthBits;                   // Maximum bits for depth component

    } ExtSupported;     // Extensions supported flags
} rlglData;
#endif  // GRAPHICS_API_OPENGL_33 || GRAPHICS_API_OPENGL_ES2

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
static rlglData RLGL = { 0 };
#endif  // GRAPHICS_API_OPENGL_33 || GRAPHICS_API_OPENGL_ES2

#if defined(GRAPHICS_API_OPENGL_ES2)
// NOTE: VAO functionality is exposed through extensions (OES)
static PFNGLGENVERTEXARRAYSOESPROC glGenVertexArrays = NULL;
static PFNGLBINDVERTEXARRAYOESPROC glBindVertexArray = NULL;
static PFNGLDELETEVERTEXARRAYSOESPROC glDeleteVertexArrays = NULL;

// NOTE: Instancing functionality could also be available through extension
static PFNGLDRAWARRAYSINSTANCEDEXTPROC glDrawArraysInstanced = NULL;
static PFNGLDRAWELEMENTSINSTANCEDEXTPROC glDrawElementsInstanced = NULL;
static PFNGLVERTEXATTRIBDIVISOREXTPROC glVertexAttribDivisor = NULL;
#endif

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
static void rlLoadShaderDefault(void);      // Load default shader (RLGL.State.defaultShader)
static void rlUnloadShaderDefault(void);    // Unload default shader (RLGL.State.defaultShader)
#endif  // GRAPHICS_API_OPENGL_33 || GRAPHICS_API_OPENGL_ES2
#if defined(GRAPHICS_API_OPENGL_11)
static int rlGenerateMipmapsData(unsigned char *data, int baseWidth, int baseHeight);   // Generate mipmaps data on CPU side
static Color *rlGenNextMipmapData(Color *srcData, int srcWidth, int srcHeight);         // Generate next mipmap level on CPU side
#endif
static int rlGetPixelDataSize(int width, int height, int format);   // Get pixel data size in bytes (image or texture)

//----------------------------------------------------------------------------------
// Module Functions Definition - Matrix operations
//----------------------------------------------------------------------------------


Vector2 Vec2(float x, float y)
{
 Vector2 v;
 v.x=x;
 v.y=y;
 return v;
}
Vector3 Vec3(float x, float y,float z)
{
 Vector3 v;
 v.x=x;
 v.y=y;
 v.z=z;
 return v;
}


#if defined(GRAPHICS_API_OPENGL_11)
// Fallback to OpenGL 1.1 function calls
//---------------------------------------
void rlMatrixMode(int mode)
{
    switch (mode)
    {
        case RL_PROJECTION: glMatrixMode(GL_PROJECTION); break;
        case RL_MODELVIEW: glMatrixMode(GL_MODELVIEW); break;
        case RL_TEXTURE: glMatrixMode(GL_TEXTURE); break;
        default: break;
    }
}

void rlFrustum(double left, double right, double bottom, double top, double znear, double zfar)
{
    glFrustum(left, right, bottom, top, znear, zfar);
}

void rlOrtho(double left, double right, double bottom, double top, double znear, double zfar)
{
    glOrtho(left, right, bottom, top, znear, zfar);
}

void rlPushMatrix(void) { glPushMatrix(); }
void rlPopMatrix(void) { glPopMatrix(); }
void rlLoadIdentity(void) { glLoadIdentity(); }
void rlTranslatef(float x, float y, float z) { glTranslatef(x, y, z); }
void rlRotatef(float angleDeg, float x, float y, float z) { glRotatef(angleDeg, x, y, z); }
void rlScalef(float x, float y, float z) { glScalef(x, y, z); }
void rlMultMatrixf(float *matf) { glMultMatrixf(matf); }
#endif
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
// Choose the current matrix to be transformed
void rlMatrixMode(int mode)
{
    if (mode == RL_PROJECTION) RLGL.State.currentMatrix = &RLGL.State.projection;
    else if (mode == RL_MODELVIEW) RLGL.State.currentMatrix = &RLGL.State.modelview;
    //else if (mode == RL_TEXTURE) // Not supported

    RLGL.State.currentMatrixMode = mode;
}

// Push the current matrix into RLGL.State.stack
void rlPushMatrix(void)
{
    if (RLGL.State.stackCounter >= MAX_MATRIX_STACK_SIZE) TRACELOG(LOG_ERROR, "RLGL: Matrix stack overflow (MAX_MATRIX_STACK_SIZE)");

    if (RLGL.State.currentMatrixMode == RL_MODELVIEW)
    {
        RLGL.State.transformRequired = true;
        RLGL.State.currentMatrix = &RLGL.State.transform;
    }

    RLGL.State.stack[RLGL.State.stackCounter] = *RLGL.State.currentMatrix;
    RLGL.State.stackCounter++;
}

// Pop lattest inserted matrix from RLGL.State.stack
void rlPopMatrix(void)
{
    if (RLGL.State.stackCounter > 0)
    {
        Matrix mat = RLGL.State.stack[RLGL.State.stackCounter - 1];
        *RLGL.State.currentMatrix = mat;
        RLGL.State.stackCounter--;
    }

    if ((RLGL.State.stackCounter == 0) && (RLGL.State.currentMatrixMode == RL_MODELVIEW))
    {
        RLGL.State.currentMatrix = &RLGL.State.modelview;
        RLGL.State.transformRequired = false;
    }
}

// Reset current matrix to identity matrix
void rlLoadIdentity(void)
{
    *RLGL.State.currentMatrix = MatrixIdentity();
}

// Multiply the current matrix by a translation matrix
void rlTranslatef(float x, float y, float z)
{
    Matrix matTranslation = MatrixTranslate(x, y, z);

    // NOTE: We transpose matrix with multiplication order
    *RLGL.State.currentMatrix = MatrixMultiply(matTranslation, *RLGL.State.currentMatrix);
}

// Multiply the current matrix by a rotation matrix
void rlRotatef(float angleDeg, float x, float y, float z)
{
    Matrix matRotation = MatrixIdentity();
    Vector3 axis = (Vector3){ x, y, z };
    matRotation = MatrixRotate(Vector3Normalize(axis), angleDeg*DEG2RAD);

    // NOTE: We transpose matrix with multiplication order
    *RLGL.State.currentMatrix = MatrixMultiply(matRotation, *RLGL.State.currentMatrix);
}

// Multiply the current matrix by a scaling matrix
void rlScalef(float x, float y, float z)
{
    Matrix matScale = MatrixScale(x, y, z);

    // NOTE: We transpose matrix with multiplication order
    *RLGL.State.currentMatrix = MatrixMultiply(matScale, *RLGL.State.currentMatrix);
}

// Multiply the current matrix by another matrix
void rlMultMatrixf(float *matf)
{
    // Matrix creation from array
    Matrix mat = { matf[0], matf[4], matf[8], matf[12],
                   matf[1], matf[5], matf[9], matf[13],
                   matf[2], matf[6], matf[10], matf[14],
                   matf[3], matf[7], matf[11], matf[15] };

    *RLGL.State.currentMatrix = MatrixMultiply(*RLGL.State.currentMatrix, mat);
}

// Multiply the current matrix by a perspective matrix generated by parameters
void rlFrustum(double left, double right, double bottom, double top, double znear, double zfar)
{
    Matrix matPerps = MatrixFrustum(left, right, bottom, top, znear, zfar);

    *RLGL.State.currentMatrix = MatrixMultiply(*RLGL.State.currentMatrix, matPerps);
}

// Multiply the current matrix by an orthographic matrix generated by parameters
void rlOrtho(double left, double right, double bottom, double top, double znear, double zfar)
{
    // NOTE: If left-right and top-botton values are equal it could create
    // a division by zero on MatrixOrtho(), response to it is platform/compiler dependant
    Matrix matOrtho = MatrixOrtho(left, right, bottom, top, znear, zfar);

    *RLGL.State.currentMatrix = MatrixMultiply(*RLGL.State.currentMatrix, matOrtho);
}
#endif

// Set the viewport area (transformation from normalized device coordinates to window coordinates)
void rlViewport(int x, int y, int width, int height)
{
    glViewport(x, y, width, height);
}

//----------------------------------------------------------------------------------
// Module Functions Definition - Vertex level operations
//----------------------------------------------------------------------------------
#if defined(GRAPHICS_API_OPENGL_11)
// Fallback to OpenGL 1.1 function calls
//---------------------------------------
void rlBegin(int mode)
{
    switch (mode)
    {
        case RL_LINES: glBegin(GL_LINES); break;
        case RL_TRIANGLES: glBegin(GL_TRIANGLES); break;
        case RL_QUADS: glBegin(GL_QUADS); break;
        default: break;
    }
}

void rlEnd() { glEnd(); }
void rlVertex2i(int x, int y) { glVertex2i(x, y); }
void rlVertex2f(float x, float y) { glVertex2f(x, y); }
void rlVertex3f(float x, float y, float z) { glVertex3f(x, y, z); }
void rlTexCoord2f(float x, float y) { glTexCoord2f(x, y); }
void rlNormal3f(float x, float y, float z) { glNormal3f(x, y, z); }
void rlColor4ub(unsigned char r, unsigned char g, unsigned char b, unsigned char a) { glColor4ub(r, g, b, a); }
void rlColor3f(float x, float y, float z) { glColor3f(x, y, z); }
void rlColor4f(float x, float y, float z, float w) { glColor4f(x, y, z, w); }
#endif
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
// Initialize drawing mode (how to organize vertex)
void rlBegin(int mode)
{
    // Draw mode can be RL_LINES, RL_TRIANGLES and RL_QUADS
    // NOTE: In all three cases, vertex are accumulated over default internal vertex buffer
    if (RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].mode != mode)
    {
        if (RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexCount > 0)
        {
            // Make sure current RLGL.currentBatch->draws[i].vertexCount is aligned a multiple of 4,
            // that way, following QUADS drawing will keep aligned with index processing
            // It implies adding some extra alignment vertex at the end of the draw,
            // those vertex are not processed but they are considered as an additional offset
            // for the next set of vertex to be drawn
            if (RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].mode == RL_LINES) RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexAlignment = ((RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexCount < 4)? RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexCount : RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexCount%4);
            else if (RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].mode == RL_TRIANGLES) RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexAlignment = ((RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexCount < 4)? 1 : (4 - (RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexCount%4)));
            else RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexAlignment = 0;

            if (!rlCheckRenderBatchLimit(RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexAlignment))
            {
                RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].vCounter += RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexAlignment;
                RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].cCounter += RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexAlignment;
                RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].tcCounter += RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexAlignment;

                RLGL.currentBatch->drawsCounter++;
            }
        }

        if (RLGL.currentBatch->drawsCounter >= DEFAULT_BATCH_DRAWCALLS) rlDrawRenderBatch(RLGL.currentBatch);

        RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].mode = mode;
        RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexCount = 0;
        RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].textureId = RLGL.State.defaultTextureId;
    }
}

// Finish vertex providing
void rlEnd(void)
{
    // Make sure vertexCount is the same for vertices, texcoords, colors and normals
    // NOTE: In OpenGL 1.1, one glColor call can be made for all the subsequent glVertex calls

    // Make sure colors count match vertex count
    if (RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].vCounter != RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].cCounter)
    {
        int addColors = RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].vCounter - RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].cCounter;

        for (int i = 0; i < addColors; i++)
        {
            RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].colors[4*RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].cCounter] = RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].colors[4*RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].cCounter - 4];
            RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].colors[4*RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].cCounter + 1] = RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].colors[4*RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].cCounter - 3];
            RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].colors[4*RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].cCounter + 2] = RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].colors[4*RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].cCounter - 2];
            RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].colors[4*RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].cCounter + 3] = RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].colors[4*RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].cCounter - 1];
            RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].cCounter++;
        }
    }

    // Make sure texcoords count match vertex count
    if (RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].vCounter != RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].tcCounter)
    {
        int addTexCoords = RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].vCounter - RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].tcCounter;

        for (int i = 0; i < addTexCoords; i++)
        {
            RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].texcoords[2*RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].tcCounter] = 0.0f;
            RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].texcoords[2*RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].tcCounter + 1] = 0.0f;
            RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].tcCounter++;
        }
    }

    // TODO: Make sure normals count match vertex count... if normals support is added in a future... :P

    // NOTE: Depth increment is dependant on rlOrtho(): z-near and z-far values,
    // as well as depth buffer bit-depth (16bit or 24bit or 32bit)
    // Correct increment formula would be: depthInc = (zfar - znear)/pow(2, bits)
    RLGL.currentBatch->currentDepth += (1.0f/20000.0f);

    // Verify internal buffers limits
    // NOTE: This check is combined with usage of rlCheckRenderBatchLimit()
    if ((RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].vCounter) >=
        (RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].elementsCount*4 - 4))
    {
        // WARNING: If we are between rlPushMatrix() and rlPopMatrix() and we need to force a rlDrawRenderBatch(),
        // we need to call rlPopMatrix() before to recover *RLGL.State.currentMatrix (RLGL.State.modelview) for the next forced draw call!
        // If we have multiple matrix pushed, it will require "RLGL.State.stackCounter" pops before launching the draw
        for (int i = RLGL.State.stackCounter; i >= 0; i--) rlPopMatrix();
        rlDrawRenderBatch(RLGL.currentBatch);
    }
}

// Define one vertex (position)
// NOTE: Vertex position data is the basic information required for drawing
void rlVertex3f(float x, float y, float z)
{
    Vector3 vec = { x, y, z };

    // Transform provided vector if required
    if (RLGL.State.transformRequired) vec = Vector3Transform(vec, RLGL.State.transform);

    // Verify that current vertex buffer elements limit has not been reached
    if (RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].vCounter < (RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].elementsCount*4))
    {
        RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].vertices[3*RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].vCounter] = vec.x;
        RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].vertices[3*RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].vCounter + 1] = vec.y;
        RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].vertices[3*RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].vCounter + 2] = vec.z;
        RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].vCounter++;

        RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexCount++;
    }
    else TRACELOG(LOG_ERROR, "RLGL: Batch elements overflow");
}

// Define one vertex (position)
void rlVertex2f(float x, float y)
{
    rlVertex3f(x, y, RLGL.currentBatch->currentDepth);
}

// Define one vertex (position)
void rlVertex2i(int x, int y)
{
    rlVertex3f((float)x, (float)y, RLGL.currentBatch->currentDepth);
}

// Define one vertex (texture coordinate)
// NOTE: Texture coordinates are limited to QUADS only
void rlTexCoord2f(float x, float y)
{
    RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].texcoords[2*RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].tcCounter] = x;
    RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].texcoords[2*RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].tcCounter + 1] = y;
    RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].tcCounter++;
}

// Define one vertex (normal)
// NOTE: Normals limited to TRIANGLES only?
void rlNormal3f(float x, float y, float z)
{
    // TODO: Normals usage...
}

// Define one vertex (color)
void rlColor4ub(unsigned char x, unsigned char y, unsigned char z, unsigned char w)
{
    RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].colors[4*RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].cCounter] = x;
    RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].colors[4*RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].cCounter + 1] = y;
    RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].colors[4*RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].cCounter + 2] = z;
    RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].colors[4*RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].cCounter + 3] = w;
    RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].cCounter++;
}

// Define one vertex (color)
void rlColor4f(float r, float g, float b, float a)
{
    rlColor4ub((unsigned char)(r*255), (unsigned char)(g*255), (unsigned char)(b*255), (unsigned char)(a*255));
}

// Define one vertex (color)
void rlColor3f(float x, float y, float z)
{
    rlColor4ub((unsigned char)(x*255), (unsigned char)(y*255), (unsigned char)(z*255), 255);
}

#endif

//--------------------------------------------------------------------------------------
// Module Functions Definition - OpenGL style functions (common to 1.1, 3.3+, ES2)
//--------------------------------------------------------------------------------------

// Set current texture to use
void rlSetTexture(unsigned int id)
{
    if (id == 0)
    {
#if defined(GRAPHICS_API_OPENGL_11)
        rlDisableTexture();
#else
        // NOTE: If quads batch limit is reached, we force a draw call and next batch starts
        if (RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].vCounter >=
            RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].elementsCount*4)
        {
            rlDrawRenderBatch(RLGL.currentBatch);
        }
#endif
    }
    else
    {
#if defined(GRAPHICS_API_OPENGL_11)
        rlEnableTexture(id);
#else
        if (RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].textureId != id)
        {
            if (RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexCount > 0)
            {
                // Make sure current RLGL.currentBatch->draws[i].vertexCount is aligned a multiple of 4,
                // that way, following QUADS drawing will keep aligned with index processing
                // It implies adding some extra alignment vertex at the end of the draw,
                // those vertex are not processed but they are considered as an additional offset
                // for the next set of vertex to be drawn
                if (RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].mode == RL_LINES) RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexAlignment = ((RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexCount < 4)? RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexCount : RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexCount%4);
                else if (RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].mode == RL_TRIANGLES) RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexAlignment = ((RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexCount < 4)? 1 : (4 - (RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexCount%4)));
                else RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexAlignment = 0;

                if (!rlCheckRenderBatchLimit(RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexAlignment))
                {
                    RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].vCounter += RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexAlignment;
                    RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].cCounter += RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexAlignment;
                    RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].tcCounter += RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexAlignment;

                    RLGL.currentBatch->drawsCounter++;
                }
            }

            if (RLGL.currentBatch->drawsCounter >= DEFAULT_BATCH_DRAWCALLS) rlDrawRenderBatch(RLGL.currentBatch);

            RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].textureId = id;
            RLGL.currentBatch->draws[RLGL.currentBatch->drawsCounter - 1].vertexCount = 0;
        }
#endif
    }
}

// Select and active a texture slot
void rlActiveTextureSlot(int slot)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glActiveTexture(GL_TEXTURE0 + slot);
#endif
}

// Enable texture
void rlEnableTexture(unsigned int id)
{
#if defined(GRAPHICS_API_OPENGL_11)
    glEnable(GL_TEXTURE_2D);
#endif
    glBindTexture(GL_TEXTURE_2D, id);
}

// Disable texture
void rlDisableTexture(void)
{
#if defined(GRAPHICS_API_OPENGL_11)
    glDisable(GL_TEXTURE_2D);
#endif
    glBindTexture(GL_TEXTURE_2D, 0);
}

// Enable texture cubemap
void rlEnableTextureCubemap(unsigned int id)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glEnable(GL_TEXTURE_CUBE_MAP);   // Core in OpenGL 1.4
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);
#endif
}

// Disable texture cubemap
void rlDisableTextureCubemap(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glDisable(GL_TEXTURE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
#endif
}

// Set texture parameters (wrap mode/filter mode)
void rlTextureParameters(unsigned int id, int param, int value)
{
    glBindTexture(GL_TEXTURE_2D, id);

    switch (param)
    {
        case RL_TEXTURE_WRAP_S:
        case RL_TEXTURE_WRAP_T:
        {
            if (value == RL_TEXTURE_WRAP_MIRROR_CLAMP)
            {
#if !defined(GRAPHICS_API_OPENGL_11)
                if (RLGL.ExtSupported.texMirrorClamp) glTexParameteri(GL_TEXTURE_2D, param, value);
                else TRACELOG(LOG_WARNING, "GL: Clamp mirror wrap mode not supported (GL_MIRROR_CLAMP_EXT)");
#endif
            }
            else glTexParameteri(GL_TEXTURE_2D, param, value);

        } break;
        case RL_TEXTURE_MAG_FILTER:
        case RL_TEXTURE_MIN_FILTER: glTexParameteri(GL_TEXTURE_2D, param, value); break;
        case RL_TEXTURE_FILTER_ANISOTROPIC:
        {
#if !defined(GRAPHICS_API_OPENGL_11)
            if (value <= RLGL.ExtSupported.maxAnisotropicLevel) glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, (float)value);
            else if (RLGL.ExtSupported.maxAnisotropicLevel > 0.0f)
            {
                TRACELOG(LOG_WARNING, "GL: Maximum anisotropic filter level supported is %iX", id, RLGL.ExtSupported.maxAnisotropicLevel);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, (float)value);
            }
            else TRACELOG(LOG_WARNING, "GL: Anisotropic filtering not supported");
#endif
        } break;
        default: break;
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}

// Enable shader program
void rlEnableShader(unsigned int id)
{
#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2))
    glUseProgram(id);
#endif
}

// Disable shader program
void rlDisableShader(void)
{
#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2))
    glUseProgram(0);
#endif
}

// Enable rendering to texture (fbo)
void rlEnableFramebuffer(unsigned int id)
{
#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)) && defined(SUPPORT_RENDER_TEXTURES_HINT)
    glBindFramebuffer(GL_FRAMEBUFFER, id);
#endif
}

// Disable rendering to texture
void rlDisableFramebuffer(void)
{
#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)) && defined(SUPPORT_RENDER_TEXTURES_HINT)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
}

// Enable depth test
void rlEnableDepthTest(void) { glEnable(GL_DEPTH_TEST); }

// Disable depth test
void rlDisableDepthTest(void) { glDisable(GL_DEPTH_TEST); }

// Enable depth write
void rlEnableDepthMask(void) { glDepthMask(GL_TRUE); }

// Disable depth write
void rlDisableDepthMask(void) { glDepthMask(GL_FALSE); }

// Enable backface culling
void rlEnableBackfaceCulling(void) { glEnable(GL_CULL_FACE); }

// Disable backface culling
void rlDisableBackfaceCulling(void) { glDisable(GL_CULL_FACE); }

// Enable scissor test
void rlEnableScissorTest(void) { glEnable(GL_SCISSOR_TEST); }

// Disable scissor test
void rlDisableScissorTest(void) { glDisable(GL_SCISSOR_TEST); }

// Scissor test
void rlScissor(int x, int y, int width, int height) { glScissor(x, y, width, height); }

// Enable wire mode
void rlEnableWireMode(void)
{
#if defined(GRAPHICS_API_OPENGL_11) || defined(GRAPHICS_API_OPENGL_33)
    // NOTE: glPolygonMode() not available on OpenGL ES
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#endif
}

// Disable wire mode
void rlDisableWireMode(void)
{
#if defined(GRAPHICS_API_OPENGL_11) || defined(GRAPHICS_API_OPENGL_33)
    // NOTE: glPolygonMode() not available on OpenGL ES
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif
}
// Set the line drawing width
void rlSetLineWidth(float width)
{
    glLineWidth(width);
}

// Get the line drawing width
float rlGetLineWidth(void)
{
    float width = 0;
    glGetFloatv(GL_LINE_WIDTH, &width);
    return width;
}

// Enable line aliasing
void rlEnableSmoothLines(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_11)
    glEnable(GL_LINE_SMOOTH);
#endif
}

// Disable line aliasing
void rlDisableSmoothLines(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_11)
    glDisable(GL_LINE_SMOOTH);
#endif
}

// Enable stereo rendering
void rlEnableStereoRender(void)
{
#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2))
    RLGL.State.stereoRender = true;
#endif
}

// Disable stereo rendering
void rlDisableStereoRender(void)
{
#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2))
    RLGL.State.stereoRender = false;
#endif
}

// Check if stereo render is enabled
bool rlIsStereoRenderEnabled(void)
{
#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2))
    return RLGL.State.stereoRender;
#else
    return false;
#endif
}

// Clear color buffer with color
void rlClearColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    // Color values clamp to 0.0f(0) and 1.0f(255)
    float cr = (float)r/255;
    float cg = (float)g/255;
    float cb = (float)b/255;
    float ca = (float)a/255;

    glClearColor(cr, cg, cb, ca);
}

// Clear used screen buffers (color and depth)
void rlClearScreenBuffers(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);     // Clear used buffers: Color and Depth (Depth is used for 3D)
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);     // Stencil buffer not used...
}

// Check and log OpenGL error codes
void rlCheckErrors()
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    int check = 1;
    while (check)
    {
        const GLenum err = glGetError();
        switch (err)
        {
            case GL_NO_ERROR: check = 0; break;
            case 0x0500: TRACELOG(LOG_WARNING, "GL: Error detected: GL_INVALID_ENUM"); break;
            case 0x0501: TRACELOG(LOG_WARNING, "GL: Error detected: GL_INVALID_VALUE"); break;
            case 0x0502: TRACELOG(LOG_WARNING, "GL: Error detected: GL_INVALID_OPERATION"); break;
            case 0x0503: TRACELOG(LOG_WARNING, "GL: Error detected: GL_STACK_OVERFLOW"); break;
            case 0x0504: TRACELOG(LOG_WARNING, "GL: Error detected: GL_STACK_UNDERFLOW"); break;
            case 0x0505: TRACELOG(LOG_WARNING, "GL: Error detected: GL_OUT_OF_MEMORY"); break;
            case 0x0506: TRACELOG(LOG_WARNING, "GL: Error detected: GL_INVALID_FRAMEBUFFER_OPERATION"); break;
            default: TRACELOG(LOG_WARNING, "GL: Error detected: Unknown error code: %x", err); break;
        }
    }
#endif
}

// Set blend mode
void rlSetBlendMode(int mode)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    if (RLGL.State.currentBlendMode != mode)
    {
        rlDrawRenderBatch(RLGL.currentBatch);

        switch (mode)
        {
            case BLEND_ALPHA: glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); glBlendEquation(GL_FUNC_ADD); break;
            case BLEND_ADDITIVE: glBlendFunc(GL_SRC_ALPHA, GL_ONE); glBlendEquation(GL_FUNC_ADD); break;
            case BLEND_MULTIPLIED: glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA); glBlendEquation(GL_FUNC_ADD); break;
            case BLEND_ADD_COLORS: glBlendFunc(GL_ONE, GL_ONE); glBlendEquation(GL_FUNC_ADD); break;
            case BLEND_SUBTRACT_COLORS: glBlendFunc(GL_ONE, GL_ONE); glBlendEquation(GL_FUNC_SUBTRACT); break;
            case BLEND_CUSTOM: glBlendFunc(RLGL.State.glBlendSrcFactor, RLGL.State.glBlendDstFactor); glBlendEquation(RLGL.State.glBlendEquation); break;
            default: break;
        }

        RLGL.State.currentBlendMode = mode;
    }
#endif
}

// Set blending mode factor and equation
void rlSetBlendFactors(int glSrcFactor, int glDstFactor, int glEquation)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    RLGL.State.glBlendSrcFactor = glSrcFactor;
    RLGL.State.glBlendDstFactor = glDstFactor;
    RLGL.State.glBlendEquation = glEquation;
#endif
}

//----------------------------------------------------------------------------------
// Module Functions Definition - rlgl functionality
//----------------------------------------------------------------------------------

// Initialize rlgl: OpenGL extensions, default buffers/shaders/textures, OpenGL states
void rlglInit(int width, int height)
{
    // Check OpenGL information and capabilities
    //------------------------------------------------------------------------------
    // Print current OpenGL and GLSL version
    TRACELOG(LOG_INFO, "GL: OpenGL device information:");
    TRACELOG(LOG_INFO, "    > Vendor:   %s", glGetString(GL_VENDOR));
    TRACELOG(LOG_INFO, "    > Renderer: %s", glGetString(GL_RENDERER));
    TRACELOG(LOG_INFO, "    > Version:  %s", glGetString(GL_VERSION));
    TRACELOG(LOG_INFO, "    > GLSL:     %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

    // NOTE: We can get a bunch of extra information about GPU capabilities (glGet*)
    //int maxTexSize;
    //glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
    //TRACELOG(LOG_INFO, "GL: Maximum texture size: %i", maxTexSize);

    //GL_MAX_TEXTURE_IMAGE_UNITS
    //GL_MAX_VIEWPORT_DIMS

    //int numAuxBuffers;
    //glGetIntegerv(GL_AUX_BUFFERS, &numAuxBuffers);
    //TRACELOG(LOG_INFO, "GL: Number of aixiliar buffers: %i", numAuxBuffers);

    //GLint numComp = 0;
    //GLint format[32] = { 0 };
    //glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &numComp);
    //glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS, format);
    //for (int i = 0; i < numComp; i++) TRACELOG(LOG_INFO, "GL: Supported compressed format: 0x%x", format[i]);

    // NOTE: We don't need that much data on screen... right now...

    // TODO: Automatize extensions loading using rlLoadExtensions() and GLAD
    // Actually, when rlglInit() is called in InitWindow() in core.c,
    // OpenGL context has already been created and required extensions loaded

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    // Get supported extensions list
    GLint numExt = 0;

#if defined(GRAPHICS_API_OPENGL_33) && !defined(GRAPHICS_API_OPENGL_21)
    // OpenGL 3.3 extensions supported by default (core)
    RLGL.ExtSupported.vao = true;
    RLGL.ExtSupported.instancing = true;
    RLGL.ExtSupported.texNPOT = true;
    RLGL.ExtSupported.texFloat32 = true;
    RLGL.ExtSupported.texDepth = true;

    // We get a list of available extensions and we check for some of them (compressed textures)
    // NOTE: We don't need to check again supported extensions but we do (GLAD already dealt with that)
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExt);

    // Allocate numExt strings pointers
    char **extList =(char**) RL_MALLOC(sizeof(char *)*numExt);

    // Get extensions strings
    for (int i = 0; i < numExt; i++) extList[i] = (char *)glGetStringi(GL_EXTENSIONS, i);
#endif
#if defined(GRAPHICS_API_OPENGL_ES2) || defined(GRAPHICS_API_OPENGL_21)
    // Allocate 512 strings pointers (2 KB)
    const char **extList = RL_MALLOC(512*sizeof(const char *));

    const char *extensions = (const char *)glGetString(GL_EXTENSIONS);  // One big const string

    // NOTE: We have to duplicate string because glGetString() returns a const string
    int len = strlen(extensions) + 1;
    char *extensionsDup = (char *)RL_CALLOC(len, sizeof(char));
    strcpy(extensionsDup, extensions);

    extList[numExt] = extensionsDup;

    for (int i = 0; i < len; i++)
    {
        if (extensionsDup[i] == ' ')
        {
            extensionsDup[i] = '\0';

            numExt++;
            extList[numExt] = &extensionsDup[i + 1];
        }
    }

    // NOTE: Duplicated string (extensionsDup) must be deallocated
#endif

    TRACELOG(LOG_INFO, "GL: Supported extensions count: %i", numExt);

    // Show supported extensions
    //for (int i = 0; i < numExt; i++)  TRACELOG(LOG_INFO, "Supported extension: %s", extList[i]);

    // Check required extensions
    for (int i = 0; i < numExt; i++)
    {
#if defined(GRAPHICS_API_OPENGL_ES2)
        // Check VAO support
        // NOTE: Only check on OpenGL ES, OpenGL 3.3 has VAO support as core feature
        if (strcmp(extList[i], (const char *)"GL_OES_vertex_array_object") == 0)
        {
            // The extension is supported by our hardware and driver, try to get related functions pointers
            // NOTE: emscripten does not support VAOs natively, it uses emulation and it reduces overall performance...
            glGenVertexArrays = (PFNGLGENVERTEXARRAYSOESPROC)eglGetProcAddress("glGenVertexArraysOES");
            glBindVertexArray = (PFNGLBINDVERTEXARRAYOESPROC)eglGetProcAddress("glBindVertexArrayOES");
            glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSOESPROC)eglGetProcAddress("glDeleteVertexArraysOES");
            //glIsVertexArray = (PFNGLISVERTEXARRAYOESPROC)eglGetProcAddress("glIsVertexArrayOES");     // NOTE: Fails in WebGL, omitted

            if ((glGenVertexArrays != NULL) && (glBindVertexArray != NULL) && (glDeleteVertexArrays != NULL)) RLGL.ExtSupported.vao = true;
        }

        // Check instanced rendering support
        if (strcmp(extList[i], (const char *)"GL_ANGLE_instanced_arrays") == 0)         // Web ANGLE
        {
            glDrawArraysInstanced = (PFNGLDRAWARRAYSINSTANCEDEXTPROC)eglGetProcAddress("glDrawArraysInstancedANGLE");
            glDrawElementsInstanced = (PFNGLDRAWELEMENTSINSTANCEDEXTPROC)eglGetProcAddress("glDrawElementsInstancedANGLE");
            glVertexAttribDivisor = (PFNGLVERTEXATTRIBDIVISOREXTPROC)eglGetProcAddress("glVertexAttribDivisorANGLE");

            if ((glDrawArraysInstanced != NULL) && (glDrawElementsInstanced != NULL) && (glVertexAttribDivisor != NULL)) RLGL.ExtSupported.instancing = true;
        }
        else
        {
            if ((strcmp(extList[i], (const char *)"GL_EXT_draw_instanced") == 0) &&     // Standard EXT
                (strcmp(extList[i], (const char *)"GL_EXT_instanced_arrays") == 0))
            {
                glDrawArraysInstanced = (PFNGLDRAWARRAYSINSTANCEDEXTPROC)eglGetProcAddress("glDrawArraysInstancedEXT");
                glDrawElementsInstanced = (PFNGLDRAWELEMENTSINSTANCEDEXTPROC)eglGetProcAddress("glDrawElementsInstancedEXT");
                glVertexAttribDivisor = (PFNGLVERTEXATTRIBDIVISOREXTPROC)eglGetProcAddress("glVertexAttribDivisorEXT");

                if ((glDrawArraysInstanced != NULL) && (glDrawElementsInstanced != NULL) && (glVertexAttribDivisor != NULL)) RLGL.ExtSupported.instancing = true;
            }
        }

        // Check NPOT textures support
        // NOTE: Only check on OpenGL ES, OpenGL 3.3 has NPOT textures full support as core feature
        if (strcmp(extList[i], (const char *)"GL_OES_texture_npot") == 0) RLGL.ExtSupported.texNPOT = true;

        // Check texture float support
        if (strcmp(extList[i], (const char *)"GL_OES_texture_float") == 0) RLGL.ExtSupported.texFloat32 = true;

        // Check depth texture support
        if ((strcmp(extList[i], (const char *)"GL_OES_depth_texture") == 0) ||
            (strcmp(extList[i], (const char *)"GL_WEBGL_depth_texture") == 0)) RLGL.ExtSupported.texDepth = true;

        if (strcmp(extList[i], (const char *)"GL_OES_depth24") == 0) RLGL.ExtSupported.maxDepthBits = 24;
        if (strcmp(extList[i], (const char *)"GL_OES_depth32") == 0) RLGL.ExtSupported.maxDepthBits = 32;
#endif
        // DDS texture compression support
        if ((strcmp(extList[i], (const char *)"GL_EXT_texture_compression_s3tc") == 0) ||
            (strcmp(extList[i], (const char *)"GL_WEBGL_compressed_texture_s3tc") == 0) ||
            (strcmp(extList[i], (const char *)"GL_WEBKIT_WEBGL_compressed_texture_s3tc") == 0)) RLGL.ExtSupported.texCompDXT = true;

        // ETC1 texture compression support
        if ((strcmp(extList[i], (const char *)"GL_OES_compressed_ETC1_RGB8_texture") == 0) ||
            (strcmp(extList[i], (const char *)"GL_WEBGL_compressed_texture_etc1") == 0)) RLGL.ExtSupported.texCompETC1 = true;

        // ETC2/EAC texture compression support
        if (strcmp(extList[i], (const char *)"GL_ARB_ES3_compatibility") == 0) RLGL.ExtSupported.texCompETC2 = true;

        // PVR texture compression support
        if (strcmp(extList[i], (const char *)"GL_IMG_texture_compression_pvrtc") == 0) RLGL.ExtSupported.texCompPVRT = true;

        // ASTC texture compression support
        if (strcmp(extList[i], (const char *)"GL_KHR_texture_compression_astc_hdr") == 0) RLGL.ExtSupported.texCompASTC = true;

        // Anisotropic texture filter support
        if (strcmp(extList[i], (const char *)"GL_EXT_texture_filter_anisotropic") == 0)
        {
            RLGL.ExtSupported.texAnisoFilter = true;
            glGetFloatv(0x84FF, &RLGL.ExtSupported.maxAnisotropicLevel);   // GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
        }

        // Clamp mirror wrap mode supported
        if (strcmp(extList[i], (const char *)"GL_EXT_texture_mirror_clamp") == 0) RLGL.ExtSupported.texMirrorClamp = true;
    }

    // Free extensions pointers
    RL_FREE(extList);

#if defined(GRAPHICS_API_OPENGL_ES2) || defined(GRAPHICS_API_OPENGL_21)
    RL_FREE(extensionsDup);    // Duplicated string must be deallocated
#endif

#if defined(GRAPHICS_API_OPENGL_ES2)
    if (RLGL.ExtSupported.vao) TRACELOG(LOG_INFO, "GL: VAO extension detected, VAO functions initialized successfully");
    else TRACELOG(LOG_WARNING, "GL: VAO extension not found, VAO not supported");

    if (RLGL.ExtSupported.texNPOT) TRACELOG(LOG_INFO, "GL: NPOT textures extension detected, full NPOT textures supported");
    else TRACELOG(LOG_WARNING, "GL: NPOT textures extension not found, limited NPOT support (no-mipmaps, no-repeat)");
#endif

    if (RLGL.ExtSupported.texCompDXT) TRACELOG(LOG_INFO, "GL: DXT compressed textures supported");
    if (RLGL.ExtSupported.texCompETC1) TRACELOG(LOG_INFO, "GL: ETC1 compressed textures supported");
    if (RLGL.ExtSupported.texCompETC2) TRACELOG(LOG_INFO, "GL: ETC2/EAC compressed textures supported");
    if (RLGL.ExtSupported.texCompPVRT) TRACELOG(LOG_INFO, "GL: PVRT compressed textures supported");
    if (RLGL.ExtSupported.texCompASTC) TRACELOG(LOG_INFO, "GL: ASTC compressed textures supported");

    if (RLGL.ExtSupported.texAnisoFilter) TRACELOG(LOG_INFO, "GL: Anisotropic textures filtering supported (max: %.0fX)", RLGL.ExtSupported.maxAnisotropicLevel);
    if (RLGL.ExtSupported.texMirrorClamp) TRACELOG(LOG_INFO, "GL: Mirror clamp wrap texture mode supported");

    // Initialize buffers, default shaders and default textures
    //----------------------------------------------------------
    // Init default white texture
    unsigned char pixels[4] = { 255, 255, 255, 255 };   // 1 pixel RGBA (4 bytes)
    RLGL.State.defaultTextureId = rlLoadTexture(pixels, 1, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, 1);

    if (RLGL.State.defaultTextureId != 0) TRACELOG(LOG_INFO, "TEXTURE: [ID %i] Default texture loaded successfully", RLGL.State.defaultTextureId);
    else TRACELOG(LOG_WARNING, "TEXTURE: Failed to load default texture");

    // Init default Shader (customized for GL 3.3 and ES2)
    rlLoadShaderDefault(); // RLGL.State.defaultShader
    RLGL.State.currentShader = RLGL.State.defaultShader;

    // Init default vertex arrays buffers
    RLGL.defaultBatch = rlLoadRenderBatch(DEFAULT_BATCH_BUFFERS, DEFAULT_BATCH_BUFFER_ELEMENTS);
    RLGL.currentBatch = &RLGL.defaultBatch;

    // Init stack matrices (emulating OpenGL 1.1)
    for (int i = 0; i < MAX_MATRIX_STACK_SIZE; i++) RLGL.State.stack[i] = MatrixIdentity();

    // Init internal matrices
    RLGL.State.transform = MatrixIdentity();
    RLGL.State.projection = MatrixIdentity();
    RLGL.State.modelview = MatrixIdentity();
    RLGL.State.currentMatrix = &RLGL.State.modelview;

#endif      // GRAPHICS_API_OPENGL_33 || GRAPHICS_API_OPENGL_ES2

    // Initialize OpenGL default states
    //----------------------------------------------------------
    // Init state: Depth test
    glDepthFunc(GL_LEQUAL);                                 // Type of depth testing to apply
    glDisable(GL_DEPTH_TEST);                               // Disable depth testing for 2D (only used for 3D)

    // Init state: Blending mode
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);      // Color blending function (how colors are mixed)
    glEnable(GL_BLEND);                                     // Enable color blending (required to work with transparencies)

    // Init state: Culling
    // NOTE: All shapes/models triangles are drawn CCW
    glCullFace(GL_BACK);                                    // Cull the back face (default)
    glFrontFace(GL_CCW);                                    // Front face are defined counter clockwise (default)
    glEnable(GL_CULL_FACE);                                 // Enable backface culling

    // Init state: Cubemap seamless
#if defined(GRAPHICS_API_OPENGL_33)
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);                 // Seamless cubemaps (not supported on OpenGL ES 2.0)
#endif

#if defined(GRAPHICS_API_OPENGL_11)
    // Init state: Color hints (deprecated in OpenGL 3.0+)
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);      // Improve quality of color and texture coordinate interpolation
    glShadeModel(GL_SMOOTH);                                // Smooth shading between vertex (vertex colors interpolation)
#endif

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    // Store screen size into global variables
    RLGL.State.framebufferWidth = width;
    RLGL.State.framebufferHeight = height;

    TRACELOG(LOG_INFO, "RLGL: Default state initialized successfully");
#endif

    // Init state: Color/Depth buffers clear
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);                   // Set clear color (black)
    glClearDepth(1.0f);                                     // Set clear depth value (default)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);     // Clear color and depth buffers (depth buffer required for 3D)

    LoadFontDefault();
    Rectangle rec = GetFontDefault().recs[95];
    SetShapesTexture(GetFontDefault().texture, (Rectangle){ rec.x + 1, rec.y + 1, rec.width - 2, rec.height - 2 });
}

// Vertex Buffer Object deinitialization (memory free)
void rlglClose(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    UnloadFontDefault();
    rlUnloadRenderBatch(RLGL.defaultBatch);

    rlUnloadShaderDefault();          // Unload default shader
    glDeleteTextures(1, &RLGL.State.defaultTextureId); // Unload default texture

    TRACELOG(LOG_INFO, "TEXTURE: [ID %i] Unloaded default texture data from VRAM (GPU)", RLGL.State.defaultTextureId);
#endif
}

// Load OpenGL extensions
// NOTE: External loader function could be passed as a pointer
void rlLoadExtensions(void *loader)
{/*
#if defined(GRAPHICS_API_OPENGL_33)
    // NOTE: glad is generated and contains only required OpenGL 3.3 Core extensions (and lower versions)
    #if !defined(__APPLE__)
//        if (!gladLoadGLLoader((GLADloadproc)loader)) TRACELOG(LOG_WARNING, "GLAD: Cannot load OpenGL extensions");
  //      else TRACELOG(LOG_INFO, "GLAD: OpenGL extensions loaded successfully");

        #if defined(GRAPHICS_API_OPENGL_21)
      ///  if (GLAD_GL_VERSION_2_1) TRACELOG(LOG_INFO, "GL: OpenGL 2.1 profile supported");
   //     #else
//        if (GLAD_GL_VERSION_3_3) TRACELOG(LOG_INFO, "GL: OpenGL 3.3 Core profile supported");
    //    else TRACELOG(LOG_ERROR, "GL: OpenGL 3.3 Core profile not supported");
        #endif
  //  #endif

    // With GLAD, we can check if an extension is supported using the GLAD_GL_xxx booleans
    //if (GLAD_GL_ARB_vertex_array_object) // Use GL_ARB_vertex_array_object
#endif*/
}

// Returns current OpenGL version
int rlGetVersion(void)
{
#if defined(GRAPHICS_API_OPENGL_11)
    return OPENGL_11;
#endif
#if defined(GRAPHICS_API_OPENGL_21)
    #if defined(__APPLE__)
        return OPENGL_33;           // NOTE: Force OpenGL 3.3 on OSX
    #else
        return OPENGL_21;
    #endif
#elif defined(GRAPHICS_API_OPENGL_33)
    return OPENGL_33;
#endif
#if defined(GRAPHICS_API_OPENGL_ES2)
    return OPENGL_ES_20;
#endif
}

// Get default framebuffer width
int rlGetFramebufferWidth(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    return RLGL.State.framebufferWidth;
#else
    return 0;
#endif
}

// Get default framebuffer height
int rlGetFramebufferHeight(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    return RLGL.State.framebufferHeight;
#else
    return 0;
#endif
}

// Get default internal shader (simple texture + tint color)
Shader rlGetShaderDefault(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    return RLGL.State.defaultShader;
#else
    Shader shader = { 0 };
    return shader;
#endif
}

// Get default internal texture (white texture)
Texture2D rlGetTextureDefault(void)
{
    Texture2D texture = { 0 };
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    texture.id = RLGL.State.defaultTextureId;
    texture.width = 1;
    texture.height = 1;
    texture.mipmaps = 1;
    texture.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
#endif
    return texture;
}

// Render batch management
//------------------------------------------------------------------------------------------------
// Load render batch
RenderBatch rlLoadRenderBatch(int numBuffers, int bufferElements)
{
    RenderBatch batch = { 0 };

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    // Initialize CPU (RAM) vertex buffers (position, texcoord, color data and indexes)
    //--------------------------------------------------------------------------------------------
    batch.vertexBuffer = (VertexBuffer *)RL_MALLOC(sizeof(VertexBuffer)*numBuffers);

    for (int i = 0; i < numBuffers; i++)
    {
        batch.vertexBuffer[i].elementsCount = bufferElements;

        batch.vertexBuffer[i].vertices = (float *)RL_MALLOC(bufferElements*3*4*sizeof(float));        // 3 float by vertex, 4 vertex by quad
        batch.vertexBuffer[i].texcoords = (float *)RL_MALLOC(bufferElements*2*4*sizeof(float));       // 2 float by texcoord, 4 texcoord by quad
        batch.vertexBuffer[i].colors = (unsigned char *)RL_MALLOC(bufferElements*4*4*sizeof(unsigned char));   // 4 float by color, 4 colors by quad
#if defined(GRAPHICS_API_OPENGL_33)
        batch.vertexBuffer[i].indices = (unsigned int *)RL_MALLOC(bufferElements*6*sizeof(unsigned int));      // 6 int by quad (indices)
#endif
#if defined(GRAPHICS_API_OPENGL_ES2)
        batch.vertexBuffer[i].indices = (unsigned short *)RL_MALLOC(bufferElements*6*sizeof(unsigned short));  // 6 int by quad (indices)
#endif

        for (int j = 0; j < (3*4*bufferElements); j++) batch.vertexBuffer[i].vertices[j] = 0.0f;
        for (int j = 0; j < (2*4*bufferElements); j++) batch.vertexBuffer[i].texcoords[j] = 0.0f;
        for (int j = 0; j < (4*4*bufferElements); j++) batch.vertexBuffer[i].colors[j] = 0;

        int k = 0;

        // Indices can be initialized right now
        for (int j = 0; j < (6*bufferElements); j += 6)
        {
            batch.vertexBuffer[i].indices[j] = 4*k;
            batch.vertexBuffer[i].indices[j + 1] = 4*k + 1;
            batch.vertexBuffer[i].indices[j + 2] = 4*k + 2;
            batch.vertexBuffer[i].indices[j + 3] = 4*k;
            batch.vertexBuffer[i].indices[j + 4] = 4*k + 2;
            batch.vertexBuffer[i].indices[j + 5] = 4*k + 3;

            k++;
        }

        batch.vertexBuffer[i].vCounter = 0;
        batch.vertexBuffer[i].tcCounter = 0;
        batch.vertexBuffer[i].cCounter = 0;
    }

    TRACELOG(LOG_INFO, "RLGL: Internal vertex buffers initialized successfully in RAM (CPU)");
    //--------------------------------------------------------------------------------------------

    // Upload to GPU (VRAM) vertex data and initialize VAOs/VBOs
    //--------------------------------------------------------------------------------------------
    for (int i = 0; i < numBuffers; i++)
    {
        if (RLGL.ExtSupported.vao)
        {
            // Initialize Quads VAO
            glGenVertexArrays(1, &batch.vertexBuffer[i].vaoId);
            glBindVertexArray(batch.vertexBuffer[i].vaoId);
        }

        // Quads - Vertex buffers binding and attributes enable
        // Vertex position buffer (shader-location = 0)
        glGenBuffers(1, &batch.vertexBuffer[i].vboId[0]);
        glBindBuffer(GL_ARRAY_BUFFER, batch.vertexBuffer[i].vboId[0]);
        glBufferData(GL_ARRAY_BUFFER, bufferElements*3*4*sizeof(float), batch.vertexBuffer[i].vertices, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(RLGL.State.currentShader.locs[SHADER_LOC_VERTEX_POSITION]);
        glVertexAttribPointer(RLGL.State.currentShader.locs[SHADER_LOC_VERTEX_POSITION], 3, GL_FLOAT, 0, 0, 0);

        // Vertex texcoord buffer (shader-location = 1)
        glGenBuffers(1, &batch.vertexBuffer[i].vboId[1]);
        glBindBuffer(GL_ARRAY_BUFFER, batch.vertexBuffer[i].vboId[1]);
        glBufferData(GL_ARRAY_BUFFER, bufferElements*2*4*sizeof(float), batch.vertexBuffer[i].texcoords, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(RLGL.State.currentShader.locs[SHADER_LOC_VERTEX_TEXCOORD01]);
        glVertexAttribPointer(RLGL.State.currentShader.locs[SHADER_LOC_VERTEX_TEXCOORD01], 2, GL_FLOAT, 0, 0, 0);

        // Vertex color buffer (shader-location = 3)
        glGenBuffers(1, &batch.vertexBuffer[i].vboId[2]);
        glBindBuffer(GL_ARRAY_BUFFER, batch.vertexBuffer[i].vboId[2]);
        glBufferData(GL_ARRAY_BUFFER, bufferElements*4*4*sizeof(unsigned char), batch.vertexBuffer[i].colors, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(RLGL.State.currentShader.locs[SHADER_LOC_VERTEX_COLOR]);
        glVertexAttribPointer(RLGL.State.currentShader.locs[SHADER_LOC_VERTEX_COLOR], 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);

        // Fill index buffer
        glGenBuffers(1, &batch.vertexBuffer[i].vboId[3]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batch.vertexBuffer[i].vboId[3]);
#if defined(GRAPHICS_API_OPENGL_33)
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, bufferElements*6*sizeof(int), batch.vertexBuffer[i].indices, GL_STATIC_DRAW);
#endif
#if defined(GRAPHICS_API_OPENGL_ES2)
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, bufferElements*6*sizeof(short), batch.vertexBuffer[i].indices, GL_STATIC_DRAW);
#endif
    }

    TRACELOG(LOG_INFO, "RLGL: Render batch vertex buffers loaded successfully");

    // Unbind the current VAO
    if (RLGL.ExtSupported.vao) glBindVertexArray(0);
    //--------------------------------------------------------------------------------------------

    // Init draw calls tracking system
    //--------------------------------------------------------------------------------------------
    batch.draws = (DrawCall *)RL_MALLOC(DEFAULT_BATCH_DRAWCALLS*sizeof(DrawCall));

    for (int i = 0; i < DEFAULT_BATCH_DRAWCALLS; i++)
    {
        batch.draws[i].mode = RL_QUADS;
        batch.draws[i].vertexCount = 0;
        batch.draws[i].vertexAlignment = 0;
        //batch.draws[i].vaoId = 0;
        //batch.draws[i].shaderId = 0;
        batch.draws[i].textureId = RLGL.State.defaultTextureId;
        //batch.draws[i].RLGL.State.projection = MatrixIdentity();
        //batch.draws[i].RLGL.State.modelview = MatrixIdentity();
    }

    batch.buffersCount = numBuffers;    // Record buffer count
    batch.drawsCounter = 1;             // Reset draws counter
    batch.currentDepth = -1.0f;         // Reset depth value
    //--------------------------------------------------------------------------------------------
#endif

    return batch;
}

// Unload default internal buffers vertex data from CPU and GPU
void rlUnloadRenderBatch(RenderBatch batch)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    // Unbind everything
    if (RLGL.ExtSupported.vao) glBindVertexArray(0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Unload all vertex buffers data
    for (int i = 0; i < batch.buffersCount; i++)
    {
        // Delete VBOs from GPU (VRAM)
        glDeleteBuffers(1, &batch.vertexBuffer[i].vboId[0]);
        glDeleteBuffers(1, &batch.vertexBuffer[i].vboId[1]);
        glDeleteBuffers(1, &batch.vertexBuffer[i].vboId[2]);
        glDeleteBuffers(1, &batch.vertexBuffer[i].vboId[3]);

        // Delete VAOs from GPU (VRAM)
        if (RLGL.ExtSupported.vao) glDeleteVertexArrays(1, &batch.vertexBuffer[i].vaoId);

        // Free vertex arrays memory from CPU (RAM)
        RL_FREE(batch.vertexBuffer[i].vertices);
        RL_FREE(batch.vertexBuffer[i].texcoords);
        RL_FREE(batch.vertexBuffer[i].colors);
        RL_FREE(batch.vertexBuffer[i].indices);
    }

    // Unload arrays
    RL_FREE(batch.vertexBuffer);
    RL_FREE(batch.draws);
#endif
}

// Draw render batch
// NOTE: We require a pointer to reset batch and increase current buffer (multi-buffer)
void rlDrawRenderBatch(RenderBatch *batch)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    // Update batch vertex buffers
    //------------------------------------------------------------------------------------------------------------
    // NOTE: If there is not vertex data, buffers doesn't need to be updated (vertexCount > 0)
    // TODO: If no data changed on the CPU arrays --> No need to re-update GPU arrays (change flag required)
    if (batch->vertexBuffer[batch->currentBuffer].vCounter > 0)
    {
        // Activate elements VAO
        if (RLGL.ExtSupported.vao) glBindVertexArray(batch->vertexBuffer[batch->currentBuffer].vaoId);

        // Vertex positions buffer
        glBindBuffer(GL_ARRAY_BUFFER, batch->vertexBuffer[batch->currentBuffer].vboId[0]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, batch->vertexBuffer[batch->currentBuffer].vCounter*3*sizeof(float), batch->vertexBuffer[batch->currentBuffer].vertices);
        //glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*4*batch->vertexBuffer[batch->currentBuffer].elementsCount, batch->vertexBuffer[batch->currentBuffer].vertices, GL_DYNAMIC_DRAW);  // Update all buffer

        // Texture coordinates buffer
        glBindBuffer(GL_ARRAY_BUFFER, batch->vertexBuffer[batch->currentBuffer].vboId[1]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, batch->vertexBuffer[batch->currentBuffer].vCounter*2*sizeof(float), batch->vertexBuffer[batch->currentBuffer].texcoords);
        //glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2*4*batch->vertexBuffer[batch->currentBuffer].elementsCount, batch->vertexBuffer[batch->currentBuffer].texcoords, GL_DYNAMIC_DRAW); // Update all buffer

        // Colors buffer
        glBindBuffer(GL_ARRAY_BUFFER, batch->vertexBuffer[batch->currentBuffer].vboId[2]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, batch->vertexBuffer[batch->currentBuffer].vCounter*4*sizeof(unsigned char), batch->vertexBuffer[batch->currentBuffer].colors);
        //glBufferData(GL_ARRAY_BUFFER, sizeof(float)*4*4*batch->vertexBuffer[batch->currentBuffer].elementsCount, batch->vertexBuffer[batch->currentBuffer].colors, GL_DYNAMIC_DRAW);    // Update all buffer

        // NOTE: glMapBuffer() causes sync issue.
        // If GPU is working with this buffer, glMapBuffer() will wait(stall) until GPU to finish its job.
        // To avoid waiting (idle), you can call first glBufferData() with NULL pointer before glMapBuffer().
        // If you do that, the previous data in PBO will be discarded and glMapBuffer() returns a new
        // allocated pointer immediately even if GPU is still working with the previous data.

        // Another option: map the buffer object into client's memory
        // Probably this code could be moved somewhere else...
        // batch->vertexBuffer[batch->currentBuffer].vertices = (float *)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
        // if (batch->vertexBuffer[batch->currentBuffer].vertices)
        // {
            // Update vertex data
        // }
        // glUnmapBuffer(GL_ARRAY_BUFFER);

        // Unbind the current VAO
        if (RLGL.ExtSupported.vao) glBindVertexArray(0);
    }
    //------------------------------------------------------------------------------------------------------------

    // Draw batch vertex buffers (considering VR stereo if required)
    //------------------------------------------------------------------------------------------------------------
    Matrix matProjection = RLGL.State.projection;
    Matrix matModelView = RLGL.State.modelview;

    int eyesCount = 1;
    if (RLGL.State.stereoRender) eyesCount = 2;

    for (int eye = 0; eye < eyesCount; eye++)
    {
        if (eyesCount == 2)
        {
            // Setup current eye viewport (half screen width)
            rlViewport(eye*RLGL.State.framebufferWidth/2, 0, RLGL.State.framebufferWidth/2, RLGL.State.framebufferHeight);

            // Set current eye view offset to modelview matrix
            rlSetMatrixModelview(MatrixMultiply(matModelView, RLGL.State.viewOffsetStereo[eye]));
            // Set current eye projection matrix
            rlSetMatrixProjection(RLGL.State.projectionStereo[eye]);
        }

        // Draw buffers
        if (batch->vertexBuffer[batch->currentBuffer].vCounter > 0)
        {
            // Set current shader and upload current MVP matrix
            glUseProgram(RLGL.State.currentShader.id);

            // Create modelview-projection matrix and upload to shader
            Matrix matMVP = MatrixMultiply(RLGL.State.modelview, RLGL.State.projection);
            glUniformMatrix4fv(RLGL.State.currentShader.locs[SHADER_LOC_MATRIX_MVP], 1, false, MatrixToFloat(matMVP));

            if (RLGL.ExtSupported.vao) glBindVertexArray(batch->vertexBuffer[batch->currentBuffer].vaoId);
            else
            {
                // Bind vertex attrib: position (shader-location = 0)
                glBindBuffer(GL_ARRAY_BUFFER, batch->vertexBuffer[batch->currentBuffer].vboId[0]);
                glVertexAttribPointer(RLGL.State.currentShader.locs[SHADER_LOC_VERTEX_POSITION], 3, GL_FLOAT, 0, 0, 0);
                glEnableVertexAttribArray(RLGL.State.currentShader.locs[SHADER_LOC_VERTEX_POSITION]);

                // Bind vertex attrib: texcoord (shader-location = 1)
                glBindBuffer(GL_ARRAY_BUFFER, batch->vertexBuffer[batch->currentBuffer].vboId[1]);
                glVertexAttribPointer(RLGL.State.currentShader.locs[SHADER_LOC_VERTEX_TEXCOORD01], 2, GL_FLOAT, 0, 0, 0);
                glEnableVertexAttribArray(RLGL.State.currentShader.locs[SHADER_LOC_VERTEX_TEXCOORD01]);

                // Bind vertex attrib: color (shader-location = 3)
                glBindBuffer(GL_ARRAY_BUFFER, batch->vertexBuffer[batch->currentBuffer].vboId[2]);
                glVertexAttribPointer(RLGL.State.currentShader.locs[SHADER_LOC_VERTEX_COLOR], 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);
                glEnableVertexAttribArray(RLGL.State.currentShader.locs[SHADER_LOC_VERTEX_COLOR]);

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batch->vertexBuffer[batch->currentBuffer].vboId[3]);
            }

            // Setup some default shader values
            glUniform4f(RLGL.State.currentShader.locs[SHADER_LOC_COLOR_DIFFUSE], 1.0f, 1.0f, 1.0f, 1.0f);
            glUniform1i(RLGL.State.currentShader.locs[SHADER_LOC_MAP_DIFFUSE], 0);  // Active default sampler2D: texture0

            // Activate additional sampler textures
            // Those additional textures will be common for all draw calls of the batch
            for (int i = 0; i < MAX_BATCH_ACTIVE_TEXTURES; i++)
            {
                if (RLGL.State.activeTextureId[i] > 0)
                {
                    glActiveTexture(GL_TEXTURE0 + 1 + i);
                    glBindTexture(GL_TEXTURE_2D, RLGL.State.activeTextureId[i]);
                }
            }

            // Activate default sampler2D texture0 (one texture is always active for default batch shader)
            // NOTE: Batch system accumulates calls by texture0 changes, additional textures are enabled for all the draw calls
            glActiveTexture(GL_TEXTURE0);

            for (int i = 0, vertexOffset = 0; i < batch->drawsCounter; i++)
            {
                // Bind current draw call texture, activated as GL_TEXTURE0 and binded to sampler2D texture0 by default
                glBindTexture(GL_TEXTURE_2D, batch->draws[i].textureId);

                if ((batch->draws[i].mode == RL_LINES) || (batch->draws[i].mode == RL_TRIANGLES)) glDrawArrays(batch->draws[i].mode, vertexOffset, batch->draws[i].vertexCount);
                else
                {
#if defined(GRAPHICS_API_OPENGL_33)
                    // We need to define the number of indices to be processed: quadsCount*6
                    // NOTE: The final parameter tells the GPU the offset in bytes from the
                    // start of the index buffer to the location of the first index to process
                    glDrawElements(GL_TRIANGLES, batch->draws[i].vertexCount/4*6, GL_UNSIGNED_INT, (GLvoid *)(vertexOffset/4*6*sizeof(GLuint)));
#endif
#if defined(GRAPHICS_API_OPENGL_ES2)
                    glDrawElements(GL_TRIANGLES, batch->draws[i].vertexCount/4*6, GL_UNSIGNED_SHORT, (GLvoid *)(vertexOffset/4*6*sizeof(GLushort)));
#endif
                }

                vertexOffset += (batch->draws[i].vertexCount + batch->draws[i].vertexAlignment);
            }

            if (!RLGL.ExtSupported.vao)
            {
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            }

            glBindTexture(GL_TEXTURE_2D, 0);    // Unbind textures
        }

        if (RLGL.ExtSupported.vao) glBindVertexArray(0); // Unbind VAO

        glUseProgram(0);    // Unbind shader program
    }
    //------------------------------------------------------------------------------------------------------------

    // Reset batch buffers
    //------------------------------------------------------------------------------------------------------------
    // Reset vertex counters for next frame
    batch->vertexBuffer[batch->currentBuffer].vCounter = 0;
    batch->vertexBuffer[batch->currentBuffer].tcCounter = 0;
    batch->vertexBuffer[batch->currentBuffer].cCounter = 0;

    // Reset depth for next draw
    batch->currentDepth = -1.0f;

    // Restore projection/modelview matrices
    RLGL.State.projection = matProjection;
    RLGL.State.modelview = matModelView;

    // Reset RLGL.currentBatch->draws array
    for (int i = 0; i < DEFAULT_BATCH_DRAWCALLS; i++)
    {
        batch->draws[i].mode = RL_QUADS;
        batch->draws[i].vertexCount = 0;
        batch->draws[i].textureId = RLGL.State.defaultTextureId;
    }

    // Reset active texture units for next batch
    for (int i = 0; i < MAX_BATCH_ACTIVE_TEXTURES; i++) RLGL.State.activeTextureId[i] = 0;

    // Reset draws counter to one draw for the batch
    batch->drawsCounter = 1;
    //------------------------------------------------------------------------------------------------------------

    // Change to next buffer in the list (in case of multi-buffering)
    batch->currentBuffer++;
    if (batch->currentBuffer >= batch->buffersCount) batch->currentBuffer = 0;
#endif
}

// Set the active render batch for rlgl
void rlSetRenderBatchActive(RenderBatch *batch)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    rlDrawRenderBatch(RLGL.currentBatch);

    if (batch != NULL) RLGL.currentBatch = batch;
    else RLGL.currentBatch = &RLGL.defaultBatch;
#endif
}

void BeginScissorMode(int x, int y, int width, int height)
{
    rlDrawRenderBatchActive();  // Force drawing elements

    rlEnableScissorTest();
    //rlScissor(x, CORE.Window.currentFbo.height - (y + height), width, height);
    rlScissor(x, y, width, height);
}

// End scissor mode
void EndScissorMode(void)
{
    rlDrawRenderBatchActive(); // Force drawing elements
    rlDisableScissorTest();
}


// Update and draw internal render batch
void rlDrawRenderBatchActive(void)
{

     // Backup GL state
    GLenum last_active_texture;
    glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint*)&last_active_texture);
    glActiveTexture(GL_TEXTURE0);
    GLuint last_program;
    glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*)&last_program);
    GLuint last_texture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint*)&last_texture);
    GLuint last_vertex_array_object;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, (GLint*)&last_vertex_array_object);
    GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
    GLint last_scissor_box[4]; glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
    GLenum last_blend_src_rgb; glGetIntegerv(GL_BLEND_SRC_RGB, (GLint*)&last_blend_src_rgb);
    GLenum last_blend_dst_rgb; glGetIntegerv(GL_BLEND_DST_RGB, (GLint*)&last_blend_dst_rgb);
    GLenum last_blend_src_alpha; glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint*)&last_blend_src_alpha);
    GLenum last_blend_dst_alpha; glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint*)&last_blend_dst_alpha);
    GLenum last_blend_equation_rgb; glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint*)&last_blend_equation_rgb);
    GLenum last_blend_equation_alpha; glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (GLint*)&last_blend_equation_alpha);
    GLboolean last_enable_blend = glIsEnabled(GL_BLEND);
    GLboolean last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
    GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
    GLboolean last_enable_stencil_test = glIsEnabled(GL_STENCIL_TEST);
    GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);



glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);


    // Setup desired GL state

    rlDrawRenderBatch(RLGL.currentBatch);    // NOTE: Stereo rendering is checked inside



    glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb, last_blend_src_alpha, last_blend_dst_alpha);
    if (last_enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
    if (last_enable_cull_face) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
    if (last_enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
    if (last_enable_stencil_test) glEnable(GL_STENCIL_TEST); else glDisable(GL_STENCIL_TEST);
    if (last_enable_scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
    glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
    glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);

}

// Check internal buffer overflow for a given number of vertex
// and force a RenderBatch draw call if required
bool rlCheckRenderBatchLimit(int vCount)
{
    bool overflow = false;

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    if ((RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].vCounter + vCount) >=
        (RLGL.currentBatch->vertexBuffer[RLGL.currentBatch->currentBuffer].elementsCount*4))
    {
        overflow = true;
        rlDrawRenderBatch(RLGL.currentBatch);    // NOTE: Stereo rendering is checked inside
    }
#endif

    return overflow;
}

// Textures data management
//-----------------------------------------------------------------------------------------
// Convert image data to OpenGL texture (returns OpenGL valid Id)
unsigned int rlLoadTexture(void *data, int width, int height, int format, int mipmapCount)
{
    glBindTexture(GL_TEXTURE_2D, 0);    // Free any old binding

    unsigned int id = 0;

    // Check texture format support by OpenGL 1.1 (compressed textures not supported)
#if defined(GRAPHICS_API_OPENGL_11)
    if (format >= PIXELFORMAT_COMPRESSED_DXT1_RGB)
    {
        TRACELOG(LOG_WARNING, "GL: OpenGL 1.1 does not support GPU compressed texture formats");
        return id;
    }
#else
    if ((!RLGL.ExtSupported.texCompDXT) && ((format == PIXELFORMAT_COMPRESSED_DXT1_RGB) || (format == PIXELFORMAT_COMPRESSED_DXT1_RGBA) ||
        (format == PIXELFORMAT_COMPRESSED_DXT3_RGBA) || (format == PIXELFORMAT_COMPRESSED_DXT5_RGBA)))
    {
        TRACELOG(LOG_WARNING, "GL: DXT compressed texture format not supported");
        return id;
    }
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    if ((!RLGL.ExtSupported.texCompETC1) && (format == PIXELFORMAT_COMPRESSED_ETC1_RGB))
    {
        TRACELOG(LOG_WARNING, "GL: ETC1 compressed texture format not supported");
        return id;
    }

    if ((!RLGL.ExtSupported.texCompETC2) && ((format == PIXELFORMAT_COMPRESSED_ETC2_RGB) || (format == PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA)))
    {
        TRACELOG(LOG_WARNING, "GL: ETC2 compressed texture format not supported");
        return id;
    }

    if ((!RLGL.ExtSupported.texCompPVRT) && ((format == PIXELFORMAT_COMPRESSED_PVRT_RGB) || (format == PIXELFORMAT_COMPRESSED_PVRT_RGBA)))
    {
        TRACELOG(LOG_WARNING, "GL: PVRT compressed texture format not supported");
        return id;
    }

    if ((!RLGL.ExtSupported.texCompASTC) && ((format == PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA) || (format == PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA)))
    {
        TRACELOG(LOG_WARNING, "GL: ASTC compressed texture format not supported");
        return id;
    }
#endif
#endif      // GRAPHICS_API_OPENGL_11

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glGenTextures(1, &id);              // Generate texture id

    glBindTexture(GL_TEXTURE_2D, id);

    int mipWidth = width;
    int mipHeight = height;
    int mipOffset = 0;          // Mipmap data offset

    // Load the different mipmap levels
    for (int i = 0; i < mipmapCount; i++)
    {
        unsigned int mipSize = rlGetPixelDataSize(mipWidth, mipHeight, format);

        unsigned int glInternalFormat, glFormat, glType;
        rlGetGlTextureFormats(format, &glInternalFormat, &glFormat, &glType);

        TRACELOGD("TEXTURE: Load mipmap level %i (%i x %i), size: %i, offset: %i", i, mipWidth, mipHeight, mipSize, mipOffset);

        if (glInternalFormat != -1)
        {
            if (format < PIXELFORMAT_COMPRESSED_DXT1_RGB) glTexImage2D(GL_TEXTURE_2D, i, glInternalFormat, mipWidth, mipHeight, 0, glFormat, glType, (unsigned char *)data + mipOffset);
#if !defined(GRAPHICS_API_OPENGL_11)
            else glCompressedTexImage2D(GL_TEXTURE_2D, i, glInternalFormat, mipWidth, mipHeight, 0, mipSize, (unsigned char *)data + mipOffset);
#endif

#if defined(GRAPHICS_API_OPENGL_33)
            if (format == PIXELFORMAT_UNCOMPRESSED_GRAYSCALE)
            {
                GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_ONE };
                glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
            }
            else if (format == PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA)
            {
#if defined(GRAPHICS_API_OPENGL_21)
                GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_ALPHA };
#elif defined(GRAPHICS_API_OPENGL_33)
                GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_GREEN };
#endif
                glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
            }
#endif
        }

        mipWidth /= 2;
        mipHeight /= 2;
        mipOffset += mipSize;

        // Security check for NPOT textures
        if (mipWidth < 1) mipWidth = 1;
        if (mipHeight < 1) mipHeight = 1;
    }

    // Texture parameters configuration
    // NOTE: glTexParameteri does NOT affect texture uploading, just the way it's used
#if defined(GRAPHICS_API_OPENGL_ES2)
    // NOTE: OpenGL ES 2.0 with no GL_OES_texture_npot support (i.e. WebGL) has limited NPOT support, so CLAMP_TO_EDGE must be used
    if (RLGL.ExtSupported.texNPOT)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);       // Set texture to repeat on x-axis
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);       // Set texture to repeat on y-axis
    }
    else
    {
        // NOTE: If using negative texture coordinates (LoadOBJ()), it does not work!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);       // Set texture to clamp on x-axis
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);       // Set texture to clamp on y-axis
    }
#else
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);       // Set texture to repeat on x-axis
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);       // Set texture to repeat on y-axis
#endif

    // Magnification and minification filters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  // Alternative: GL_LINEAR
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  // Alternative: GL_LINEAR

#if defined(GRAPHICS_API_OPENGL_33)
    if (mipmapCount > 1)
    {
        // Activate Trilinear filtering if mipmaps are available
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }
#endif

    // At this point we have the texture loaded in GPU and texture parameters configured

    // NOTE: If mipmaps were not in data, they are not generated automatically

    // Unbind current texture
    glBindTexture(GL_TEXTURE_2D, 0);

    if (id > 0) TRACELOG(LOG_INFO, "TEXTURE: [ID %i] Texture created successfully (%ix%i - %i mipmaps)", id, width, height, mipmapCount);
    else TRACELOG(LOG_WARNING, "TEXTURE: Failed to load texture");

    return id;
}

// Load depth texture/renderbuffer (to be attached to fbo)
// WARNING: OpenGL ES 2.0 requires GL_OES_depth_texture/WEBGL_depth_texture extensions
unsigned int rlLoadTextureDepth(int width, int height, bool useRenderBuffer)
{
    unsigned int id = 0;

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    // In case depth textures not supported, we force renderbuffer usage
    if (!RLGL.ExtSupported.texDepth) useRenderBuffer = true;

    // NOTE: We let the implementation to choose the best bit-depth
    // Possible formats: GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT32 and GL_DEPTH_COMPONENT32F
    unsigned int glInternalFormat = GL_DEPTH_COMPONENT;

#if defined(GRAPHICS_API_OPENGL_ES2)
    if (RLGL.ExtSupported.maxDepthBits == 32) glInternalFormat = GL_DEPTH_COMPONENT32_OES;
    else if (RLGL.ExtSupported.maxDepthBits == 24) glInternalFormat = GL_DEPTH_COMPONENT24_OES;
    else glInternalFormat = GL_DEPTH_COMPONENT16;
#endif

    if (!useRenderBuffer && RLGL.ExtSupported.texDepth)
    {
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, glInternalFormat, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindTexture(GL_TEXTURE_2D, 0);

        TRACELOG(LOG_INFO, "TEXTURE: Depth texture loaded successfully");
    }
    else
    {
        // Create the renderbuffer that will serve as the depth attachment for the framebuffer
        // NOTE: A renderbuffer is simpler than a texture and could offer better performance on embedded devices
        glGenRenderbuffers(1, &id);
        glBindRenderbuffer(GL_RENDERBUFFER, id);
        glRenderbufferStorage(GL_RENDERBUFFER, glInternalFormat, width, height);

        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        TRACELOG(LOG_INFO, "TEXTURE: [ID %i] Depth renderbuffer loaded successfully (%i bits)", id, (RLGL.ExtSupported.maxDepthBits >= 24)? RLGL.ExtSupported.maxDepthBits : 16);
    }
#endif

    return id;
}

// Load texture cubemap
// NOTE: Cubemap data is expected to be 6 images in a single data array (one after the other),
// expected the following convention: +X, -X, +Y, -Y, +Z, -Z
unsigned int rlLoadTextureCubemap(void *data, int size, int format)
{
    unsigned int id = 0;

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    unsigned int dataSize = rlGetPixelDataSize(size, size, format);

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);

    unsigned int glInternalFormat, glFormat, glType;
    rlGetGlTextureFormats(format, &glInternalFormat, &glFormat, &glType);

    if (glInternalFormat != -1)
    {
        // Load cubemap faces
        for (unsigned int i = 0; i < 6; i++)
        {
            if (data == NULL)
            {
                if (format < PIXELFORMAT_COMPRESSED_DXT1_RGB)
                {
                    if (format == PIXELFORMAT_UNCOMPRESSED_R32G32B32)
                    {
                        // Instead of using a sized internal texture format (GL_RGB16F, GL_RGB32F), we let the driver to choose the better format for us (GL_RGB)
                        if (RLGL.ExtSupported.texFloat32) glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, size, size, 0, GL_RGB, GL_FLOAT, NULL);
                        else TRACELOG(LOG_WARNING, "TEXTURES: Cubemap requested format not supported");
                    }
                    else if ((format == PIXELFORMAT_UNCOMPRESSED_R32) || (format == PIXELFORMAT_UNCOMPRESSED_R32G32B32A32)) TRACELOG(LOG_WARNING, "TEXTURES: Cubemap requested format not supported");
                    else glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, glInternalFormat, size, size, 0, glFormat, glType, NULL);
                }
                else TRACELOG(LOG_WARNING, "TEXTURES: Empty cubemap creation does not support compressed format");
            }
            else
            {
                if (format < PIXELFORMAT_COMPRESSED_DXT1_RGB) glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, glInternalFormat, size, size, 0, glFormat, glType, (unsigned char *)data + i*dataSize);
                else glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, glInternalFormat, size, size, 0, dataSize, (unsigned char *)data + i*dataSize);
            }

#if defined(GRAPHICS_API_OPENGL_33)
            if (format == PIXELFORMAT_UNCOMPRESSED_GRAYSCALE)
            {
                GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_ONE };
                glTexParameteriv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
            }
            else if (format == PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA)
            {
#if defined(GRAPHICS_API_OPENGL_21)
                GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_ALPHA };
#elif defined(GRAPHICS_API_OPENGL_33)
                GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_GREEN };
#endif
                glTexParameteriv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
            }
#endif
        }
    }

    // Set cubemap texture sampling parameters
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#if defined(GRAPHICS_API_OPENGL_33)
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);  // Flag not supported on OpenGL ES 2.0
#endif

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
#endif

    if (id > 0) TRACELOG(LOG_INFO, "TEXTURE: [ID %i] Cubemap texture created successfully (%ix%i)", id, size, size);
    else TRACELOG(LOG_WARNING, "TEXTURE: Failed to load cubemap texture");

    return id;
}

// Update already loaded texture in GPU with new data
// NOTE: We don't know safely if internal texture format is the expected one...
void rlUpdateTexture(unsigned int id, int offsetX, int offsetY, int width, int height, int format, const void *data)
{
    glBindTexture(GL_TEXTURE_2D, id);

    unsigned int glInternalFormat, glFormat, glType;
    rlGetGlTextureFormats(format, &glInternalFormat, &glFormat, &glType);

    if ((glInternalFormat != -1) && (format < PIXELFORMAT_COMPRESSED_DXT1_RGB))
    {
        glTexSubImage2D(GL_TEXTURE_2D, 0, offsetX, offsetY, width, height, glFormat, glType, (unsigned char *)data);
    }
    else TRACELOG(LOG_WARNING, "TEXTURE: [ID %i] Failed to update for current texture format (%i)", id, format);
}

// Get OpenGL internal formats and data type from raylib PixelFormat
void rlGetGlTextureFormats(int format, unsigned int *glInternalFormat, unsigned int *glFormat, unsigned int *glType)
{
    *glInternalFormat = -1;
    *glFormat = -1;
    *glType = -1;

    switch (format)
    {
    #if defined(GRAPHICS_API_OPENGL_11) || defined(GRAPHICS_API_OPENGL_21) || defined(GRAPHICS_API_OPENGL_ES2)
        // NOTE: on OpenGL ES 2.0 (WebGL), internalFormat must match format and options allowed are: GL_LUMINANCE, GL_RGB, GL_RGBA
        case PIXELFORMAT_UNCOMPRESSED_GRAYSCALE: *glInternalFormat = GL_LUMINANCE; *glFormat = GL_LUMINANCE; *glType = GL_UNSIGNED_BYTE; break;
        case PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA: *glInternalFormat = GL_LUMINANCE_ALPHA; *glFormat = GL_LUMINANCE_ALPHA; *glType = GL_UNSIGNED_BYTE; break;
        case PIXELFORMAT_UNCOMPRESSED_R5G6B5: *glInternalFormat = GL_RGB; *glFormat = GL_RGB; *glType = GL_UNSIGNED_SHORT_5_6_5; break;
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8: *glInternalFormat = GL_RGB; *glFormat = GL_RGB; *glType = GL_UNSIGNED_BYTE; break;
        case PIXELFORMAT_UNCOMPRESSED_R5G5B5A1: *glInternalFormat = GL_RGBA; *glFormat = GL_RGBA; *glType = GL_UNSIGNED_SHORT_5_5_5_1; break;
        case PIXELFORMAT_UNCOMPRESSED_R4G4B4A4: *glInternalFormat = GL_RGBA; *glFormat = GL_RGBA; *glType = GL_UNSIGNED_SHORT_4_4_4_4; break;
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8: *glInternalFormat = GL_RGBA; *glFormat = GL_RGBA; *glType = GL_UNSIGNED_BYTE; break;
        #if !defined(GRAPHICS_API_OPENGL_11)
        case PIXELFORMAT_UNCOMPRESSED_R32: if (RLGL.ExtSupported.texFloat32) *glInternalFormat = GL_LUMINANCE; *glFormat = GL_LUMINANCE; *glType = GL_FLOAT; break;   // NOTE: Requires extension OES_texture_float
        case PIXELFORMAT_UNCOMPRESSED_R32G32B32: if (RLGL.ExtSupported.texFloat32) *glInternalFormat = GL_RGB; *glFormat = GL_RGB; *glType = GL_FLOAT; break;         // NOTE: Requires extension OES_texture_float
        case PIXELFORMAT_UNCOMPRESSED_R32G32B32A32: if (RLGL.ExtSupported.texFloat32) *glInternalFormat = GL_RGBA; *glFormat = GL_RGBA; *glType = GL_FLOAT; break;    // NOTE: Requires extension OES_texture_float
        #endif
    #elif defined(GRAPHICS_API_OPENGL_33)
        case PIXELFORMAT_UNCOMPRESSED_GRAYSCALE: *glInternalFormat = GL_R8; *glFormat = GL_RED; *glType = GL_UNSIGNED_BYTE; break;
        case PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA: *glInternalFormat = GL_RG8; *glFormat = GL_RG; *glType = GL_UNSIGNED_BYTE; break;
        case PIXELFORMAT_UNCOMPRESSED_R5G6B5: *glInternalFormat = GL_RGB565; *glFormat = GL_RGB; *glType = GL_UNSIGNED_SHORT_5_6_5; break;
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8: *glInternalFormat = GL_RGB8; *glFormat = GL_RGB; *glType = GL_UNSIGNED_BYTE; break;
        case PIXELFORMAT_UNCOMPRESSED_R5G5B5A1: *glInternalFormat = GL_RGB5_A1; *glFormat = GL_RGBA; *glType = GL_UNSIGNED_SHORT_5_5_5_1; break;
        case PIXELFORMAT_UNCOMPRESSED_R4G4B4A4: *glInternalFormat = GL_RGBA4; *glFormat = GL_RGBA; *glType = GL_UNSIGNED_SHORT_4_4_4_4; break;
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8: *glInternalFormat = GL_RGBA8; *glFormat = GL_RGBA; *glType = GL_UNSIGNED_BYTE; break;
        case PIXELFORMAT_UNCOMPRESSED_R32: if (RLGL.ExtSupported.texFloat32) *glInternalFormat = GL_R32F; *glFormat = GL_RED; *glType = GL_FLOAT; break;
        case PIXELFORMAT_UNCOMPRESSED_R32G32B32: if (RLGL.ExtSupported.texFloat32) *glInternalFormat = GL_RGB32F; *glFormat = GL_RGB; *glType = GL_FLOAT; break;
        case PIXELFORMAT_UNCOMPRESSED_R32G32B32A32: if (RLGL.ExtSupported.texFloat32) *glInternalFormat = GL_RGBA32F; *glFormat = GL_RGBA; *glType = GL_FLOAT; break;
    #endif
    #if !defined(GRAPHICS_API_OPENGL_11)
        case PIXELFORMAT_COMPRESSED_DXT1_RGB: if (RLGL.ExtSupported.texCompDXT) *glInternalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT; break;
        case PIXELFORMAT_COMPRESSED_DXT1_RGBA: if (RLGL.ExtSupported.texCompDXT) *glInternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT; break;
        case PIXELFORMAT_COMPRESSED_DXT3_RGBA: if (RLGL.ExtSupported.texCompDXT) *glInternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT; break;
        case PIXELFORMAT_COMPRESSED_DXT5_RGBA: if (RLGL.ExtSupported.texCompDXT) *glInternalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT; break;
        case PIXELFORMAT_COMPRESSED_ETC1_RGB: if (RLGL.ExtSupported.texCompETC1) *glInternalFormat = GL_ETC1_RGB8_OES; break;                      // NOTE: Requires OpenGL ES 2.0 or OpenGL 4.3
        case PIXELFORMAT_COMPRESSED_ETC2_RGB: if (RLGL.ExtSupported.texCompETC2) *glInternalFormat = GL_COMPRESSED_RGB8_ETC2; break;               // NOTE: Requires OpenGL ES 3.0 or OpenGL 4.3
        case PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA: if (RLGL.ExtSupported.texCompETC2) *glInternalFormat = GL_COMPRESSED_RGBA8_ETC2_EAC; break;     // NOTE: Requires OpenGL ES 3.0 or OpenGL 4.3
        case PIXELFORMAT_COMPRESSED_PVRT_RGB: if (RLGL.ExtSupported.texCompPVRT) *glInternalFormat = GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG; break;    // NOTE: Requires PowerVR GPU
        case PIXELFORMAT_COMPRESSED_PVRT_RGBA: if (RLGL.ExtSupported.texCompPVRT) *glInternalFormat = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG; break;  // NOTE: Requires PowerVR GPU
        case PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA: if (RLGL.ExtSupported.texCompASTC) *glInternalFormat = GL_COMPRESSED_RGBA_ASTC_4x4_KHR; break;  // NOTE: Requires OpenGL ES 3.1 or OpenGL 4.3
        case PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA: if (RLGL.ExtSupported.texCompASTC) *glInternalFormat = GL_COMPRESSED_RGBA_ASTC_8x8_KHR; break;  // NOTE: Requires OpenGL ES 3.1 or OpenGL 4.3
    #endif
        default: TRACELOG(LOG_WARNING, "TEXTURE: Current format not supported (%i)", format); break;
    }
}

// Unload texture from GPU memory
void rlUnloadTexture(unsigned int id)
{
    glDeleteTextures(1, &id);
}

// Generate mipmap data for selected texture
void rlGenerateMipmaps(Texture2D *texture)
{
    glBindTexture(GL_TEXTURE_2D, texture->id);

    // Check if texture is power-of-two (POT)
    bool texIsPOT = false;

    if (((texture->width > 0) && ((texture->width & (texture->width - 1)) == 0)) &&
        ((texture->height > 0) && ((texture->height & (texture->height - 1)) == 0))) texIsPOT = true;

#if defined(GRAPHICS_API_OPENGL_11)
    if (texIsPOT)
    {
        // WARNING: Manual mipmap generation only works for RGBA 32bit textures!
        if (texture->format == PIXELFORMAT_UNCOMPRESSED_R8G8B8A8)
        {
            // Retrieve texture data from VRAM
            void *texData = rlReadTexturePixels(*texture);

            // NOTE: Texture data size is reallocated to fit mipmaps data
            // NOTE: CPU mipmap generation only supports RGBA 32bit data
            int mipmapCount = rlGenerateMipmapsData(texData, texture->width, texture->height);

            int size = texture->width*texture->height*4;
            int offset = size;

            int mipWidth = texture->width/2;
            int mipHeight = texture->height/2;

            // Load the mipmaps
            for (int level = 1; level < mipmapCount; level++)
            {
                glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA8, mipWidth, mipHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char *)texData + offset);

                size = mipWidth*mipHeight*4;
                offset += size;

                mipWidth /= 2;
                mipHeight /= 2;
            }

            texture->mipmaps = mipmapCount + 1;
            RL_FREE(texData); // Once mipmaps have been generated and data has been uploaded to GPU VRAM, we can discard RAM data

            TRACELOG(LOG_WARNING, "TEXTURE: [ID %i] Mipmaps generated manually on CPU side, total: %i", texture->id, texture->mipmaps);
        }
        else TRACELOG(LOG_WARNING, "TEXTURE: [ID %i] Failed to generate mipmaps for provided texture format", texture->id);
    }
#endif
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    if ((texIsPOT) || (RLGL.ExtSupported.texNPOT))
    {
        //glHint(GL_GENERATE_MIPMAP_HINT, GL_DONT_CARE);   // Hint for mipmaps generation algorythm: GL_FASTEST, GL_NICEST, GL_DONT_CARE
        glGenerateMipmap(GL_TEXTURE_2D);    // Generate mipmaps automatically

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);   // Activate Trilinear filtering for mipmaps

        #define MIN(a,b) (((a)<(b))?(a):(b))
        #define MAX(a,b) (((a)>(b))?(a):(b))

        texture->mipmaps =  1 + (int)floor(log(MAX(texture->width, texture->height))/log(2));
        TRACELOG(LOG_INFO, "TEXTURE: [ID %i] Mipmaps generated automatically, total: %i", texture->id, texture->mipmaps);
    }
#endif
    else TRACELOG(LOG_WARNING, "TEXTURE: [ID %i] Failed to generate mipmaps", texture->id);

    glBindTexture(GL_TEXTURE_2D, 0);
}


// Read texture pixel data
void *rlReadTexturePixels(Texture2D texture)
{
    void *pixels = NULL;

#if defined(GRAPHICS_API_OPENGL_11) || defined(GRAPHICS_API_OPENGL_33)
    glBindTexture(GL_TEXTURE_2D, texture.id);

    // NOTE: Using texture.id, we can retrieve some texture info (but not on OpenGL ES 2.0)
    // Possible texture info: GL_TEXTURE_RED_SIZE, GL_TEXTURE_GREEN_SIZE, GL_TEXTURE_BLUE_SIZE, GL_TEXTURE_ALPHA_SIZE
    //int width, height, format;
    //glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
    //glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
    //glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &format);

    // NOTE: Each row written to or read from by OpenGL pixel operations like glGetTexImage are aligned to a 4 byte boundary by default, which may add some padding.
    // Use glPixelStorei to modify padding with the GL_[UN]PACK_ALIGNMENT setting.
    // GL_PACK_ALIGNMENT affects operations that read from OpenGL memory (glReadPixels, glGetTexImage, etc.)
    // GL_UNPACK_ALIGNMENT affects operations that write to OpenGL memory (glTexImage, etc.)
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    unsigned int glInternalFormat, glFormat, glType;
    rlGetGlTextureFormats(texture.format, &glInternalFormat, &glFormat, &glType);
    unsigned int size = rlGetPixelDataSize(texture.width, texture.height, texture.format);

    if ((glInternalFormat != -1) && (texture.format < PIXELFORMAT_COMPRESSED_DXT1_RGB))
    {
        pixels = RL_MALLOC(size);
        glGetTexImage(GL_TEXTURE_2D, 0, glFormat, glType, pixels);
    }
    else TRACELOG(LOG_WARNING, "TEXTURE: [ID %i] Data retrieval not suported for pixel format (%i)", texture.id, texture.format);

    glBindTexture(GL_TEXTURE_2D, 0);
#endif

#if defined(GRAPHICS_API_OPENGL_ES2)
    // glGetTexImage() is not available on OpenGL ES 2.0
    // Texture width and height are required on OpenGL ES 2.0. There is no way to get it from texture id.
    // Two possible Options:
    // 1 - Bind texture to color fbo attachment and glReadPixels()
    // 2 - Create an fbo, activate it, render quad with texture, glReadPixels()
    // We are using Option 1, just need to care for texture format on retrieval
    // NOTE: This behaviour could be conditioned by graphic driver...
    unsigned int fboId = rlLoadFramebuffer(texture.width, texture.height);

    // TODO: Create depth texture/renderbuffer for fbo?

    glBindFramebuffer(GL_FRAMEBUFFER, fboId);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Attach our texture to FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.id, 0);

    // We read data as RGBA because FBO texture is configured as RGBA, despite binding another texture format
    pixels = (unsigned char *)RL_MALLOC(rlGetPixelDataSize(texture.width, texture.height, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8));
    glReadPixels(0, 0, texture.width, texture.height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Clean up temporal fbo
    rlUnloadFramebuffer(fboId);
#endif

    return pixels;
}


// Read screen pixel data (color buffer)
unsigned char *rlReadScreenPixels(int width, int height)
{
    unsigned char *screenData = (unsigned char *)RL_CALLOC(width*height*4, sizeof(unsigned char));

    // NOTE 1: glReadPixels returns image flipped vertically -> (0,0) is the bottom left corner of the framebuffer
    // NOTE 2: We are getting alpha channel! Be careful, it can be transparent if not cleared properly!
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, screenData);

    // Flip image vertically!
    unsigned char *imgData = (unsigned char *)RL_MALLOC(width*height*4*sizeof(unsigned char));

    for (int y = height - 1; y >= 0; y--)
    {
        for (int x = 0; x < (width*4); x++)
        {
            imgData[((height - 1) - y)*width*4 + x] = screenData[(y*width*4) + x];  // Flip line

            // Set alpha component value to 255 (no trasparent image retrieval)
            // NOTE: Alpha value has already been applied to RGB in framebuffer, we don't need it!
            if (((x + 1)%4) == 0) imgData[((height - 1) - y)*width*4 + x] = 255;
        }
    }

    RL_FREE(screenData);

    return imgData;     // NOTE: image data should be freed
}

// Framebuffer management (fbo)
//-----------------------------------------------------------------------------------------
// Load a framebuffer to be used for rendering
// NOTE: No textures attached
unsigned int rlLoadFramebuffer(int width, int height)
{
    unsigned int fboId = 0;

#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)) && defined(SUPPORT_RENDER_TEXTURES_HINT)
    glGenFramebuffers(1, &fboId);       // Create the framebuffer object
    glBindFramebuffer(GL_FRAMEBUFFER, 0);   // Unbind any framebuffer
#endif

    return fboId;
}

// Attach color buffer texture to an fbo (unloads previous attachment)
// NOTE: Attach type: 0-Color, 1-Depth renderbuffer, 2-Depth texture
void rlFramebufferAttach(unsigned int fboId, unsigned int texId, int attachType, int texType, int mipLevel)
{
#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)) && defined(SUPPORT_RENDER_TEXTURES_HINT)
    glBindFramebuffer(GL_FRAMEBUFFER, fboId);

    switch (attachType)
    {
        case RL_ATTACHMENT_COLOR_CHANNEL0:
        case RL_ATTACHMENT_COLOR_CHANNEL1:
        case RL_ATTACHMENT_COLOR_CHANNEL2:
        case RL_ATTACHMENT_COLOR_CHANNEL3:
        case RL_ATTACHMENT_COLOR_CHANNEL4:
        case RL_ATTACHMENT_COLOR_CHANNEL5:
        case RL_ATTACHMENT_COLOR_CHANNEL6:
        case RL_ATTACHMENT_COLOR_CHANNEL7:
        {
            if (texType == RL_ATTACHMENT_TEXTURE2D) glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachType, GL_TEXTURE_2D, texId, mipLevel);
            else if (texType == RL_ATTACHMENT_RENDERBUFFER) glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachType, GL_RENDERBUFFER, texId);
            else if (texType >= RL_ATTACHMENT_CUBEMAP_POSITIVE_X) glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachType, GL_TEXTURE_CUBE_MAP_POSITIVE_X + texType, texId, mipLevel);

        } break;
        case RL_ATTACHMENT_DEPTH:
        {
            if (texType == RL_ATTACHMENT_TEXTURE2D) glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texId, mipLevel);
            else if (texType == RL_ATTACHMENT_RENDERBUFFER)  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, texId);

        } break;
        case RL_ATTACHMENT_STENCIL:
        {
            if (texType == RL_ATTACHMENT_TEXTURE2D) glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, texId, mipLevel);
            else if (texType == RL_ATTACHMENT_RENDERBUFFER)  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, texId);

        } break;
        default: break;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
}

// Verify render texture is complete
bool rlFramebufferComplete(unsigned int id)
{
    bool result = false;

#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)) && defined(SUPPORT_RENDER_TEXTURES_HINT)
    glBindFramebuffer(GL_FRAMEBUFFER, id);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        switch (status)
        {
            case GL_FRAMEBUFFER_UNSUPPORTED: TRACELOG(LOG_WARNING, "FBO: [ID %i] Framebuffer is unsupported", id); break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: TRACELOG(LOG_WARNING, "FBO: [ID %i] Framebuffer has incomplete attachment", id); break;
#if defined(GRAPHICS_API_OPENGL_ES2)
            case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS: TRACELOG(LOG_WARNING, "FBO: [ID %i] Framebuffer has incomplete dimensions", id); break;
#endif
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: TRACELOG(LOG_WARNING, "FBO: [ID %i] Framebuffer has a missing attachment", id); break;
            default: break;
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    result = (status == GL_FRAMEBUFFER_COMPLETE);
#endif

    return result;
}

// Unload framebuffer from GPU memory
// NOTE: All attached textures/cubemaps/renderbuffers are also deleted
void rlUnloadFramebuffer(unsigned int id)
{
#if (defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)) && defined(SUPPORT_RENDER_TEXTURES_HINT)

    // Query depth attachment to automatically delete texture/renderbuffer
    int depthType = 0, depthId = 0;
    glBindFramebuffer(GL_FRAMEBUFFER, id);   // Bind framebuffer to query depth texture type
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &depthType);
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &depthId);

    unsigned int depthIdU = (unsigned int)depthId;
    if (depthType == GL_RENDERBUFFER) glDeleteRenderbuffers(1, &depthIdU);
    else if (depthType == GL_RENDERBUFFER) glDeleteTextures(1, &depthIdU);

    // NOTE: If a texture object is deleted while its image is attached to the *currently bound* framebuffer,
    // the texture image is automatically detached from the currently bound framebuffer.

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &id);

    TRACELOG(LOG_INFO, "FBO: [ID %i] Unloaded framebuffer from VRAM (GPU)", id);
#endif
}

// Vertex data management
//-----------------------------------------------------------------------------------------
// Load a new attributes buffer
unsigned int rlLoadVertexBuffer(void *buffer, int size, bool dynamic)
{
    unsigned int id = 0;

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glGenBuffers(1, &id);
    glBindBuffer(GL_ARRAY_BUFFER, id);
    glBufferData(GL_ARRAY_BUFFER, size, buffer, dynamic? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
#endif

    return id;
}

// Load a new attributes element buffer
unsigned int rlLoadVertexBufferElement(void *buffer, int size, bool dynamic)
{
    unsigned int id = 0;

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glGenBuffers(1, &id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, buffer, dynamic? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
#endif

    return id;
}

void rlEnableVertexBuffer(unsigned int id)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glBindBuffer(GL_ARRAY_BUFFER, id);
#endif
}

void rlDisableVertexBuffer(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif
}

void rlEnableVertexBufferElement(unsigned int id)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
#endif
}

void rlDisableVertexBufferElement(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
#endif
}

// Update GPU buffer with new data
// NOTE: dataSize and offset must be provided in bytes
void rlUpdateVertexBuffer(int bufferId, void *data, int dataSize, int offset)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glBindBuffer(GL_ARRAY_BUFFER, bufferId);
    glBufferSubData(GL_ARRAY_BUFFER, offset, dataSize, data);
#endif
}

bool rlEnableVertexArray(unsigned int vaoId)
{
    bool result = false;
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    if (RLGL.ExtSupported.vao)
    {
        glBindVertexArray(vaoId);
        result = true;
    }
#endif
    return result;
}

void rlDisableVertexArray(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    if (RLGL.ExtSupported.vao) glBindVertexArray(0);
#endif
}

void rlEnableVertexAttribute(unsigned int index)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glEnableVertexAttribArray(index);
#endif
}

void rlDisableVertexAttribute(unsigned int index)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glDisableVertexAttribArray(index);
#endif
}

void rlDrawVertexArray(int offset, int count)
{
    glDrawArrays(GL_TRIANGLES, offset, count);
}

void rlDrawVertexArrayElements(int offset, int count, void *buffer)
{
    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_SHORT, (unsigned short*)buffer + offset);
}

void rlDrawVertexArrayInstanced(int offset, int count, int instances)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glDrawArraysInstanced(GL_TRIANGLES, 0, count, instances);
#endif
}

void rlDrawVertexArrayElementsInstanced(int offset, int count, void *buffer, int instances)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glDrawElementsInstanced(GL_TRIANGLES, count, GL_UNSIGNED_SHORT, (unsigned short*)buffer + offset, instances);
#endif
}

#if defined(GRAPHICS_API_OPENGL_11)
void rlEnableStatePointer(int vertexAttribType, void *buffer)
{
    if (buffer != NULL) glEnableClientState(vertexAttribType);
    switch (vertexAttribType)
    {
        case GL_VERTEX_ARRAY: glVertexPointer(3, GL_FLOAT, 0, buffer); break;
        case GL_TEXTURE_COORD_ARRAY: glTexCoordPointer(2, GL_FLOAT, 0, buffer); break;
        case GL_NORMAL_ARRAY: if (buffer != NULL) glNormalPointer(GL_FLOAT, 0, buffer); break;
        case GL_COLOR_ARRAY: if (buffer != NULL) glColorPointer(4, GL_UNSIGNED_BYTE, 0, buffer); break;
        //case GL_INDEX_ARRAY: if (buffer != NULL) glIndexPointer(GL_SHORT, 0, buffer); break; // Indexed colors
        default: break;
    }
}

void rlDisableStatePointer(int vertexAttribType)
{
    glDisableClientState(vertexAttribType);
}
#endif

unsigned int rlLoadVertexArray(void)
{
    unsigned int vaoId = 0;
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glGenVertexArrays(1, &vaoId);
#endif
    return vaoId;
}

void rlSetVertexAttribute(unsigned int index, int compSize, int type, bool normalized, int stride, void *pointer)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glVertexAttribPointer(index, compSize, type, normalized, stride, pointer);
#endif
}

void rlSetVertexAttributeDivisor(unsigned int index, int divisor)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glVertexAttribDivisor(index, divisor);
#endif
}

void rlUnloadVertexArray(unsigned int vaoId)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    if (RLGL.ExtSupported.vao)
    {
        glBindVertexArray(0);
        glDeleteVertexArrays(1, &vaoId);
        TRACELOG(LOG_INFO, "VAO: [ID %i] Unloaded vertex array data from VRAM (GPU)", vaoId);
    }
#endif
}

void rlUnloadVertexBuffer(unsigned int vboId)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glDeleteBuffers(1, &vboId);
    //TRACELOG(LOG_INFO, "VBO: Unloaded vertex data from VRAM (GPU)");
#endif
}

// Shaders management
//-----------------------------------------------------------------------------------------------
// Load shader from code strings
// NOTE: If shader string is NULL, using default vertex/fragment shaders
unsigned int rlLoadShaderCode(const char *vsCode, const char *fsCode)
{
    unsigned int id = 0;

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    unsigned int vertexShaderId = RLGL.State.defaultVShaderId;
    unsigned int fragmentShaderId = RLGL.State.defaultFShaderId;

    if (vsCode != NULL) vertexShaderId = rlCompileShader(vsCode, GL_VERTEX_SHADER);
    if (fsCode != NULL) fragmentShaderId = rlCompileShader(fsCode, GL_FRAGMENT_SHADER);

    if ((vertexShaderId == RLGL.State.defaultVShaderId) && (fragmentShaderId == RLGL.State.defaultFShaderId)) id = RLGL.State.defaultShader.id;
    else
    {
        id = rlLoadShaderProgram(vertexShaderId, fragmentShaderId);

        if (vertexShaderId != RLGL.State.defaultVShaderId)
        {
            // Detach shader before deletion to make sure memory is freed
            glDetachShader(id, vertexShaderId);
            glDeleteShader(vertexShaderId);
        }
        if (fragmentShaderId != RLGL.State.defaultFShaderId)
        {
            // Detach shader before deletion to make sure memory is freed
            glDetachShader(id, fragmentShaderId);
            glDeleteShader(fragmentShaderId);
        }

        if (id == 0)
        {
            TRACELOG(LOG_WARNING, "SHADER: Failed to load custom shader code");
            id = RLGL.State.defaultShader.id;
        }
    }

    // Get available shader uniforms
    // NOTE: This information is useful for debug...
    int uniformCount = -1;

    glGetProgramiv(id, GL_ACTIVE_UNIFORMS, &uniformCount);

    for (int i = 0; i < uniformCount; i++)
    {
        int namelen = -1;
        int num = -1;
        char name[256]; // Assume no variable names longer than 256
        GLenum type = GL_ZERO;

        // Get the name of the uniforms
        glGetActiveUniform(id, i, sizeof(name) - 1, &namelen, &num, &type, name);

        name[namelen] = 0;

        TRACELOGD("SHADER: [ID %i] Active uniform (%s) set at location: %i", id, name, glGetUniformLocation(id, name));
    }
#endif

    return id;
}

// Compile custom shader and return shader id
unsigned int rlCompileShader(const char *shaderCode, int type)
{
    unsigned int shader = 0;

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    shader = glCreateShader(type);
    glShaderSource(shader, 1, &shaderCode, NULL);

    GLint success = 0;
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (success == GL_FALSE)
    {
        TRACELOG(LOG_WARNING, "SHADER: [ID %i] Failed to compile shader code", shader);

        int maxLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

        if (maxLength > 0)
        {
            int length = 0;
            char *log =(char*) RL_CALLOC(maxLength, sizeof(char));
            glGetShaderInfoLog(shader, maxLength, &length, log);
            TRACELOG(LOG_WARNING, "SHADER: [ID %i] Compile error: %s", shader, log);
            RL_FREE(log);
        }
    }
    else TRACELOG(LOG_INFO, "SHADER: [ID %i] Compiled successfully", shader);
#endif

    return shader;
}

// Load custom shader strings and return program id
unsigned int rlLoadShaderProgram(unsigned int vShaderId, unsigned int fShaderId)
{
    unsigned int program = 0;

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    GLint success = 0;
    program = glCreateProgram();

    glAttachShader(program, vShaderId);
    glAttachShader(program, fShaderId);

    // NOTE: Default attribute shader locations must be binded before linking
    glBindAttribLocation(program, 0, DEFAULT_SHADER_ATTRIB_NAME_POSITION);
    glBindAttribLocation(program, 1, DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD);
    glBindAttribLocation(program, 2, DEFAULT_SHADER_ATTRIB_NAME_NORMAL);
    glBindAttribLocation(program, 3, DEFAULT_SHADER_ATTRIB_NAME_COLOR);
    glBindAttribLocation(program, 4, DEFAULT_SHADER_ATTRIB_NAME_TANGENT);
    glBindAttribLocation(program, 5, DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD2);

    // NOTE: If some attrib name is no found on the shader, it locations becomes -1

    glLinkProgram(program);

    // NOTE: All uniform variables are intitialised to 0 when a program links

    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (success == GL_FALSE)
    {
        TRACELOG(LOG_WARNING, "SHADER: [ID %i] Failed to link shader program", program);

        int maxLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

        if (maxLength > 0)
        {
            int length = 0;
            char *log = (char*)RL_CALLOC(maxLength, sizeof(char));
            glGetProgramInfoLog(program, maxLength, &length, log);
            TRACELOG(LOG_WARNING, "SHADER: [ID %i] Link error: %s", program, log);
            RL_FREE(log);
        }

        glDeleteProgram(program);

        program = 0;
    }
    else TRACELOG(LOG_INFO, "SHADER: [ID %i] Program loaded successfully", program);
#endif
    return program;
}

// Unload shader program
void rlUnloadShaderProgram(unsigned int id)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glDeleteProgram(id);

    TRACELOG(LOG_INFO, "SHADER: [ID %i] Unloaded shader program data from VRAM (GPU)", id);
#endif
}

// Get shader location uniform
int rlGetLocationUniform(unsigned int shaderId, const char *uniformName)
{
    int location = -1;
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    location = glGetUniformLocation(shaderId, uniformName);

    if (location == -1) TRACELOG(LOG_WARNING, "SHADER: [ID %i] Failed to find shader uniform: %s", shaderId, uniformName);
    else TRACELOG(LOG_INFO, "SHADER: [ID %i] Shader uniform (%s) set at location: %i", shaderId, uniformName, location);
#endif
    return location;
}

// Get shader location attribute
int rlGetLocationAttrib(unsigned int shaderId, const char *attribName)
{
    int location = -1;
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    location = glGetAttribLocation(shaderId, attribName);

    if (location == -1) TRACELOG(LOG_WARNING, "SHADER: [ID %i] Failed to find shader attribute: %s", shaderId, attribName);
    else TRACELOG(LOG_INFO, "SHADER: [ID %i] Shader attribute (%s) set at location: %i", shaderId, attribName, location);
#endif
    return location;
}

// Set shader value uniform
void rlSetUniform(int locIndex, const void *value, int uniformType, int count)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    switch (uniformType)
    {
        case SHADER_UNIFORM_FLOAT: glUniform1fv(locIndex, count, (float *)value); break;
        case SHADER_UNIFORM_VEC2: glUniform2fv(locIndex, count, (float *)value); break;
        case SHADER_UNIFORM_VEC3: glUniform3fv(locIndex, count, (float *)value); break;
        case SHADER_UNIFORM_VEC4: glUniform4fv(locIndex, count, (float *)value); break;
        case SHADER_UNIFORM_INT: glUniform1iv(locIndex, count, (int *)value); break;
        case SHADER_UNIFORM_IVEC2: glUniform2iv(locIndex, count, (int *)value); break;
        case SHADER_UNIFORM_IVEC3: glUniform3iv(locIndex, count, (int *)value); break;
        case SHADER_UNIFORM_IVEC4: glUniform4iv(locIndex, count, (int *)value); break;
        case SHADER_UNIFORM_SAMPLER2D: glUniform1iv(locIndex, count, (int *)value); break;
        default: TRACELOG(LOG_WARNING, "SHADER: Failed to set uniform value, data type not recognized");
    }
#endif
}

// Set shader value attribute
void rlSetVertexAttributeDefault(int locIndex, const void *value, int attribType, int count)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    switch (attribType)
    {
        case SHADER_ATTRIB_FLOAT: if (count == 1) glVertexAttrib1fv(locIndex, (float *)value); break;
        case SHADER_ATTRIB_VEC2: if (count == 2) glVertexAttrib2fv(locIndex, (float *)value); break;
        case SHADER_ATTRIB_VEC3: if (count == 3) glVertexAttrib3fv(locIndex, (float *)value); break;
        case SHADER_ATTRIB_VEC4: if (count == 4) glVertexAttrib4fv(locIndex, (float *)value); break;
        default: TRACELOG(LOG_WARNING, "SHADER: Failed to set attrib default value, data type not recognized");
    }
#endif
}

// Set shader value uniform matrix
void rlSetUniformMatrix(int locIndex, Matrix mat)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    glUniformMatrix4fv(locIndex, 1, false, MatrixToFloat(mat));
#endif
}

// Set shader value uniform sampler
void rlSetUniformSampler(int locIndex, unsigned int textureId)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    // Check if texture is already active
    for (int i = 0; i < MAX_BATCH_ACTIVE_TEXTURES; i++) if (RLGL.State.activeTextureId[i] == textureId) return;

    // Register a new active texture for the internal batch system
    // NOTE: Default texture is always activated as GL_TEXTURE0
    for (int i = 0; i < MAX_BATCH_ACTIVE_TEXTURES; i++)
    {
        if (RLGL.State.activeTextureId[i] == 0)
        {
            glUniform1i(locIndex, 1 + i);              // Activate new texture unit
            RLGL.State.activeTextureId[i] = textureId; // Save texture id for binding on drawing
            break;
        }
    }
#endif
}

// Set shader currently active
void rlSetShader(Shader shader)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    if (RLGL.State.currentShader.id != shader.id)
    {
        rlDrawRenderBatch(RLGL.currentBatch);
        RLGL.State.currentShader = shader;
    }
#endif
}

// Matrix state management
//-----------------------------------------------------------------------------------------
// Return internal modelview matrix
Matrix rlGetMatrixModelview(void)
{
    Matrix matrix = MatrixIdentity();
#if defined(GRAPHICS_API_OPENGL_11)
    float mat[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, mat);
    matrix.m0  = mat[0];     matrix.m1  = mat[1];     matrix.m2  = mat[2];     matrix.m3  = mat[3];
    matrix.m4  = mat[4];     matrix.m5  = mat[5];     matrix.m6  = mat[6];     matrix.m7  = mat[7];
    matrix.m8  = mat[8];     matrix.m9  = mat[9];     matrix.m10 = mat[10];    matrix.m11 = mat[11];
    matrix.m12 = mat[12];    matrix.m13 = mat[13];    matrix.m14 = mat[14];    matrix.m15 = mat[15];
#else
    matrix = RLGL.State.modelview;
#endif
    return matrix;
}

// Return internal projection matrix
Matrix rlGetMatrixProjection(void)
{
#if defined(GRAPHICS_API_OPENGL_11)
    float mat[16];
    glGetFloatv(GL_PROJECTION_MATRIX,mat);
    Matrix m;
    m.m0  = mat[0];     m.m1  = mat[1];     m.m2  = mat[2];     m.m3  = mat[3];
    m.m4  = mat[4];     m.m5  = mat[5];     m.m6  = mat[6];     m.m7  = mat[7];
    m.m8  = mat[8];     m.m9  = mat[9];     m.m10 = mat[10];    m.m11 = mat[11];
    m.m12 = mat[12];    m.m13 = mat[13];    m.m14 = mat[14];    m.m15 = mat[15];
    return m;
#else
    return RLGL.State.projection;
#endif
}

// Get internal accumulated transform matrix
Matrix rlGetMatrixTransform(void)
{
    Matrix mat = MatrixIdentity();
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    // TODO: Consider possible transform matrices in the RLGL.State.stack
    // Is this the right order? or should we start with the first stored matrix instead of the last one?
    //Matrix matStackTransform = MatrixIdentity();
    //for (int i = RLGL.State.stackCounter; i > 0; i--) matStackTransform = MatrixMultiply(RLGL.State.stack[i], matStackTransform);
    mat = RLGL.State.transform;
#endif
    return mat;
}

// Get internal projection matrix for stereo render (selected eye)
RLAPI Matrix rlGetMatrixProjectionStereo(int eye)
{
    Matrix mat = MatrixIdentity();
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    mat = RLGL.State.projectionStereo[eye];
#endif
    return mat;
}

// Get internal view offset matrix for stereo render (selected eye)
RLAPI Matrix rlGetMatrixViewOffsetStereo(int eye)
{
    Matrix mat = MatrixIdentity();
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    mat = RLGL.State.viewOffsetStereo[eye];
#endif
    return mat;
}

// Set a custom modelview matrix (replaces internal modelview matrix)
void rlSetMatrixModelview(Matrix view)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    RLGL.State.modelview = view;
#endif
}

// Set a custom projection matrix (replaces internal projection matrix)
void rlSetMatrixProjection(Matrix projection)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    RLGL.State.projection = projection;
#endif
}

// Set eyes projection matrices for stereo rendering
void rlSetMatrixProjectionStereo(Matrix right, Matrix left)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    RLGL.State.projectionStereo[0] = right;
    RLGL.State.projectionStereo[1] = left;
#endif
}

// Set eyes view offsets matrices for stereo rendering
void rlSetMatrixViewOffsetStereo(Matrix right, Matrix left)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    RLGL.State.viewOffsetStereo[0] = right;
    RLGL.State.viewOffsetStereo[1] = left;
#endif
}

// Load and draw a 1x1 XY quad in NDC
void rlLoadDrawQuad(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    unsigned int quadVAO = 0;
    unsigned int quadVBO = 0;

    float vertices[] = {
         // Positions         Texcoords
        -1.0f,  1.0f, 0.0f,   0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f,   0.0f, 0.0f,
         1.0f,  1.0f, 0.0f,   1.0f, 1.0f,
         1.0f, -1.0f, 0.0f,   1.0f, 0.0f,
    };

    // Gen VAO to contain VBO
    glGenVertexArrays(1, &quadVAO);
    glBindVertexArray(quadVAO);

    // Gen and fill vertex buffer (VBO)
    glGenBuffers(1, &quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

    // Bind vertex attributes (position, texcoords)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void *)0); // Positions
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void *)(3*sizeof(float))); // Texcoords

    // Draw quad
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);

    // Delete buffers (VBO and VAO)
    glDeleteBuffers(1, &quadVBO);
    glDeleteVertexArrays(1, &quadVAO);
#endif
}

// Load and draw a 1x1 3D cube in NDC
void rlLoadDrawCube(void)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    unsigned int cubeVAO = 0;
    unsigned int cubeVBO = 0;

    float vertices[] = {
         // Positions          Normals               Texcoords
        -1.0f, -1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   0.0f, 0.0f,
         1.0f,  1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f,
         1.0f, -1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   1.0f, 0.0f,
         1.0f,  1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   0.0f, 0.0f,
        -1.0f,  1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   0.0f, 1.0f,
        -1.0f, -1.0f,  1.0f,   0.0f,  0.0f,  1.0f,   0.0f, 0.0f,
         1.0f, -1.0f,  1.0f,   0.0f,  0.0f,  1.0f,   1.0f, 0.0f,
         1.0f,  1.0f,  1.0f,   0.0f,  0.0f,  1.0f,   1.0f, 1.0f,
         1.0f,  1.0f,  1.0f,   0.0f,  0.0f,  1.0f,   1.0f, 1.0f,
        -1.0f,  1.0f,  1.0f,   0.0f,  0.0f,  1.0f,   0.0f, 1.0f,
        -1.0f, -1.0f,  1.0f,   0.0f,  0.0f,  1.0f,   0.0f, 0.0f,
        -1.0f,  1.0f,  1.0f,  -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
        -1.0f,  1.0f, -1.0f,  -1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,  -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,  -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
        -1.0f, -1.0f,  1.0f,  -1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
        -1.0f,  1.0f,  1.0f,  -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
         1.0f,  1.0f,  1.0f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
         1.0f, -1.0f, -1.0f,   1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
         1.0f,  1.0f, -1.0f,   1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
         1.0f, -1.0f, -1.0f,   1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
         1.0f,  1.0f,  1.0f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
         1.0f, -1.0f,  1.0f,   1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
        -1.0f, -1.0f, -1.0f,   0.0f, -1.0f,  0.0f,   0.0f, 1.0f,
         1.0f, -1.0f, -1.0f,   0.0f, -1.0f,  0.0f,   1.0f, 1.0f,
         1.0f, -1.0f,  1.0f,   0.0f, -1.0f,  0.0f,   1.0f, 0.0f,
         1.0f, -1.0f,  1.0f,   0.0f, -1.0f,  0.0f,   1.0f, 0.0f,
        -1.0f, -1.0f,  1.0f,   0.0f, -1.0f,  0.0f,   0.0f, 0.0f,
        -1.0f, -1.0f, -1.0f,   0.0f, -1.0f,  0.0f,   0.0f, 1.0f,
        -1.0f,  1.0f, -1.0f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f,
         1.0f,  1.0f,  1.0f,   0.0f,  1.0f,  0.0f,   1.0f, 0.0f,
         1.0f,  1.0f, -1.0f,   0.0f,  1.0f,  0.0f,   1.0f, 1.0f,
         1.0f,  1.0f,  1.0f,   0.0f,  1.0f,  0.0f,   1.0f, 0.0f,
        -1.0f,  1.0f, -1.0f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f,
        -1.0f,  1.0f,  1.0f,   0.0f,  1.0f,  0.0f,   0.0f, 0.0f
    };

    // Gen VAO to contain VBO
    glGenVertexArrays(1, &cubeVAO);
    glBindVertexArray(cubeVAO);

    // Gen and fill vertex buffer (VBO)
    glGenBuffers(1, &cubeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Bind vertex attributes (position, normals, texcoords)
    glBindVertexArray(cubeVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void *)0); // Positions
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void *)(3*sizeof(float))); // Normals
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void *)(6*sizeof(float))); // Texcoords
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Draw cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    // Delete VBO and VAO
    glDeleteBuffers(1, &cubeVBO);
    glDeleteVertexArrays(1, &cubeVAO);
#endif
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
// Load default shader (just vertex positioning and texture coloring)
// NOTE: This shader program is used for internal buffers
// NOTE: It uses global variable: RLGL.State.defaultShader
static void rlLoadShaderDefault(void)
{
    RLGL.State.defaultShader.locs = (int *)RL_CALLOC(MAX_SHADER_LOCATIONS, sizeof(int));

    // NOTE: All locations must be reseted to -1 (no location)
    for (int i = 0; i < MAX_SHADER_LOCATIONS; i++) RLGL.State.defaultShader.locs[i] = -1;

    // Vertex shader directly defined, no external file required
    const char *vShaderDefault =
#if defined(GRAPHICS_API_OPENGL_21)
    "#version 120                       \n"
    "attribute vec3 vertexPosition;     \n"
    "attribute vec2 vertexTexCoord;     \n"
    "attribute vec4 vertexColor;        \n"
    "varying vec2 fragTexCoord;         \n"
    "varying vec4 fragColor;            \n"
#elif defined(GRAPHICS_API_OPENGL_33)
    "#version 330                       \n"
    "in vec3 vertexPosition;            \n"
    "in vec2 vertexTexCoord;            \n"
    "in vec4 vertexColor;               \n"
    "out vec2 fragTexCoord;             \n"
    "out vec4 fragColor;                \n"
#endif
#if defined(GRAPHICS_API_OPENGL_ES2)
    "#version 100                       \n"
    "attribute vec3 vertexPosition;     \n"
    "attribute vec2 vertexTexCoord;     \n"
    "attribute vec4 vertexColor;        \n"
    "varying vec2 fragTexCoord;         \n"
    "varying vec4 fragColor;            \n"
#endif
    "uniform mat4 mvp;                  \n"
    "void main()                        \n"
    "{                                  \n"
    "    fragTexCoord = vertexTexCoord; \n"
    "    fragColor = vertexColor;       \n"
    "    gl_Position = mvp*vec4(vertexPosition, 1.0); \n"
    "}                                  \n";

    // Fragment shader directly defined, no external file required
    const char *fShaderDefault =
#if defined(GRAPHICS_API_OPENGL_21)
    "#version 120                       \n"
    "varying vec2 fragTexCoord;         \n"
    "varying vec4 fragColor;            \n"
    "uniform sampler2D texture0;        \n"
    "uniform vec4 colDiffuse;           \n"
    "void main()                        \n"
    "{                                  \n"
    "    vec4 texelColor = texture2D(texture0, fragTexCoord); \n"
    "    gl_FragColor = texelColor*colDiffuse*fragColor;      \n"
    "}                                  \n";
#elif defined(GRAPHICS_API_OPENGL_33)
    "#version 330       \n"
    "in vec2 fragTexCoord;              \n"
    "in vec4 fragColor;                 \n"
    "out vec4 finalColor;               \n"
    "uniform sampler2D texture0;        \n"
    "uniform vec4 colDiffuse;           \n"
    "void main()                        \n"
    "{                                  \n"
    "    vec4 texelColor = texture(texture0, fragTexCoord);   \n"
    "    finalColor = texelColor*colDiffuse*fragColor;        \n"
    "}                                  \n";
#endif
#if defined(GRAPHICS_API_OPENGL_ES2)
    "#version 100                       \n"
    "precision mediump float;           \n"     // Precision required for OpenGL ES2 (WebGL)
    "varying vec2 fragTexCoord;         \n"
    "varying vec4 fragColor;            \n"
    "uniform sampler2D texture0;        \n"
    "uniform vec4 colDiffuse;           \n"
    "void main()                        \n"
    "{                                  \n"
    "    vec4 texelColor = texture2D(texture0, fragTexCoord); \n"
    "    gl_FragColor = texelColor*colDiffuse*fragColor;      \n"
    "}                                  \n";
#endif

    // NOTE: Compiled vertex/fragment shaders are kept for re-use
    RLGL.State.defaultVShaderId = rlCompileShader(vShaderDefault, GL_VERTEX_SHADER);     // Compile default vertex shader
    RLGL.State.defaultFShaderId = rlCompileShader(fShaderDefault, GL_FRAGMENT_SHADER);   // Compile default fragment shader

    RLGL.State.defaultShader.id = rlLoadShaderProgram(RLGL.State.defaultVShaderId, RLGL.State.defaultFShaderId);

    if (RLGL.State.defaultShader.id > 0)
    {
        TRACELOG(LOG_INFO, "SHADER: [ID %i] Default shader loaded successfully", RLGL.State.defaultShader.id);

        // Set default shader locations: attributes locations
        RLGL.State.defaultShader.locs[SHADER_LOC_VERTEX_POSITION] = glGetAttribLocation(RLGL.State.defaultShader.id, "vertexPosition");
        RLGL.State.defaultShader.locs[SHADER_LOC_VERTEX_TEXCOORD01] = glGetAttribLocation(RLGL.State.defaultShader.id, "vertexTexCoord");
        RLGL.State.defaultShader.locs[SHADER_LOC_VERTEX_COLOR] = glGetAttribLocation(RLGL.State.defaultShader.id, "vertexColor");

        // Set default shader locations: uniform locations
        RLGL.State.defaultShader.locs[SHADER_LOC_MATRIX_MVP]  = glGetUniformLocation(RLGL.State.defaultShader.id, "mvp");
        RLGL.State.defaultShader.locs[SHADER_LOC_COLOR_DIFFUSE] = glGetUniformLocation(RLGL.State.defaultShader.id, "colDiffuse");
        RLGL.State.defaultShader.locs[SHADER_LOC_MAP_DIFFUSE] = glGetUniformLocation(RLGL.State.defaultShader.id, "texture0");
    }
    else TRACELOG(LOG_WARNING, "SHADER: [ID %i] Failed to load default shader", RLGL.State.defaultShader.id);
}

// Unload default shader
// NOTE: It uses global variable: RLGL.State.defaultShader
static void rlUnloadShaderDefault(void)
{
    glUseProgram(0);

    glDetachShader(RLGL.State.defaultShader.id, RLGL.State.defaultVShaderId);
    glDetachShader(RLGL.State.defaultShader.id, RLGL.State.defaultFShaderId);
    glDeleteShader(RLGL.State.defaultVShaderId);
    glDeleteShader(RLGL.State.defaultFShaderId);

    glDeleteProgram(RLGL.State.defaultShader.id);

    RL_FREE(RLGL.State.defaultShader.locs);
}
#endif  // GRAPHICS_API_OPENGL_33 || GRAPHICS_API_OPENGL_ES2

#if defined(GRAPHICS_API_OPENGL_11)
// Mipmaps data is generated after image data
// NOTE: Only works with RGBA (4 bytes) data!
static int rlGenerateMipmapsData(unsigned char *data, int baseWidth, int baseHeight)
{
    int mipmapCount = 1;                // Required mipmap levels count (including base level)
    int width = baseWidth;
    int height = baseHeight;
    int size = baseWidth*baseHeight*4;  // Size in bytes (will include mipmaps...), RGBA only

    // Count mipmap levels required
    while ((width != 1) && (height != 1))
    {
        width /= 2;
        height /= 2;

        TRACELOGD("TEXTURE: Next mipmap size: %i x %i", width, height);

        mipmapCount++;

        size += (width*height*4);       // Add mipmap size (in bytes)
    }

    TRACELOGD("TEXTURE: Total mipmaps required: %i", mipmapCount);
    TRACELOGD("TEXTURE: Total size of data required: %i", size);

    unsigned char *temp = RL_REALLOC(data, size);

    if (temp != NULL) data = temp;
    else TRACELOG(LOG_WARNING, "TEXTURE: Failed to re-allocate required mipmaps memory");

    width = baseWidth;
    height = baseHeight;
    size = (width*height*4);

    // Generate mipmaps
    // NOTE: Every mipmap data is stored after data
    Color *image = (Color *)RL_MALLOC(width*height*sizeof(Color));
    Color *mipmap = NULL;
    int offset = 0;
    int j = 0;

    for (int i = 0; i < size; i += 4)
    {
        image[j].r = data[i];
        image[j].g = data[i + 1];
        image[j].b = data[i + 2];
        image[j].a = data[i + 3];
        j++;
    }

    TRACELOGD("TEXTURE: Mipmap base size (%ix%i)", width, height);

    for (int mip = 1; mip < mipmapCount; mip++)
    {
        mipmap = rlGenNextMipmapData(image, width, height);

        offset += (width*height*4); // Size of last mipmap
        j = 0;

        width /= 2;
        height /= 2;
        size = (width*height*4);    // Mipmap size to store after offset

        // Add mipmap to data
        for (int i = 0; i < size; i += 4)
        {
            data[offset + i] = mipmap[j].r;
            data[offset + i + 1] = mipmap[j].g;
            data[offset + i + 2] = mipmap[j].b;
            data[offset + i + 3] = mipmap[j].a;
            j++;
        }

        RL_FREE(image);

        image = mipmap;
        mipmap = NULL;
    }

    RL_FREE(mipmap);       // free mipmap data

    return mipmapCount;
}

// Manual mipmap generation (basic scaling algorithm)
static Color *rlGenNextMipmapData(Color *srcData, int srcWidth, int srcHeight)
{
    int x2, y2;
    Color prow, pcol;

    int width = srcWidth/2;
    int height = srcHeight/2;

    Color *mipmap = (Color *)RL_MALLOC(width*height*sizeof(Color));

    // Scaling algorithm works perfectly (box-filter)
    for (int y = 0; y < height; y++)
    {
        y2 = 2*y;

        for (int x = 0; x < width; x++)
        {
            x2 = 2*x;

            prow.r = (srcData[y2*srcWidth + x2].r + srcData[y2*srcWidth + x2 + 1].r)/2;
            prow.g = (srcData[y2*srcWidth + x2].g + srcData[y2*srcWidth + x2 + 1].g)/2;
            prow.b = (srcData[y2*srcWidth + x2].b + srcData[y2*srcWidth + x2 + 1].b)/2;
            prow.a = (srcData[y2*srcWidth + x2].a + srcData[y2*srcWidth + x2 + 1].a)/2;

            pcol.r = (srcData[(y2+1)*srcWidth + x2].r + srcData[(y2+1)*srcWidth + x2 + 1].r)/2;
            pcol.g = (srcData[(y2+1)*srcWidth + x2].g + srcData[(y2+1)*srcWidth + x2 + 1].g)/2;
            pcol.b = (srcData[(y2+1)*srcWidth + x2].b + srcData[(y2+1)*srcWidth + x2 + 1].b)/2;
            pcol.a = (srcData[(y2+1)*srcWidth + x2].a + srcData[(y2+1)*srcWidth + x2 + 1].a)/2;

            mipmap[y*width + x].r = (prow.r + pcol.r)/2;
            mipmap[y*width + x].g = (prow.g + pcol.g)/2;
            mipmap[y*width + x].b = (prow.b + pcol.b)/2;
            mipmap[y*width + x].a = (prow.a + pcol.a)/2;
        }
    }

    TRACELOGD("TEXTURE: Mipmap generated successfully (%ix%i)", width, height);

    return mipmap;
}
#endif      // GRAPHICS_API_OPENGL_11

// Get pixel data size in bytes (image or texture)
// NOTE: Size depends on pixel format
static int rlGetPixelDataSize(int width, int height, int format)
{
    int dataSize = 0;       // Size in bytes
    int bpp = 0;            // Bits per pixel

    switch (format)
    {
        case PIXELFORMAT_UNCOMPRESSED_GRAYSCALE: bpp = 8; break;
        case PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA:
        case PIXELFORMAT_UNCOMPRESSED_R5G6B5:
        case PIXELFORMAT_UNCOMPRESSED_R5G5B5A1:
        case PIXELFORMAT_UNCOMPRESSED_R4G4B4A4: bpp = 16; break;
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8: bpp = 32; break;
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8: bpp = 24; break;
        case PIXELFORMAT_UNCOMPRESSED_R32: bpp = 32; break;
        case PIXELFORMAT_UNCOMPRESSED_R32G32B32: bpp = 32*3; break;
        case PIXELFORMAT_UNCOMPRESSED_R32G32B32A32: bpp = 32*4; break;
        case PIXELFORMAT_COMPRESSED_DXT1_RGB:
        case PIXELFORMAT_COMPRESSED_DXT1_RGBA:
        case PIXELFORMAT_COMPRESSED_ETC1_RGB:
        case PIXELFORMAT_COMPRESSED_ETC2_RGB:
        case PIXELFORMAT_COMPRESSED_PVRT_RGB:
        case PIXELFORMAT_COMPRESSED_PVRT_RGBA: bpp = 4; break;
        case PIXELFORMAT_COMPRESSED_DXT3_RGBA:
        case PIXELFORMAT_COMPRESSED_DXT5_RGBA:
        case PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA:
        case PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA: bpp = 8; break;
        case PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA: bpp = 2; break;
        default: break;
    }

    dataSize = width*height*bpp/8;  // Total data size in bytes

    // Most compressed formats works on 4x4 blocks,
    // if texture is smaller, minimum dataSize is 8 or 16
    if ((width < 4) && (height < 4))
    {
        if ((format >= PIXELFORMAT_COMPRESSED_DXT1_RGB) && (format < PIXELFORMAT_COMPRESSED_DXT3_RGBA)) dataSize = 8;
        else if ((format >= PIXELFORMAT_COMPRESSED_DXT3_RGBA) && (format < PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA)) dataSize = 16;
    }

    return dataSize;
}

//*********************************************************************************************************************************************RAYLIB
Texture2D texShapes = { 1, 1, 1, 1, 7 };        // Texture used on shapes drawing (usually a white pixel)
Rectangle texShapesRec = { 0, 0, 1, 1 };        // Texture source rectangle used on shapes drawing
    #define SMOOTH_CIRCLE_ERROR_RATE  0.5f

void SetShapesTexture(Texture2D texture, Rectangle source)
{
    texShapes = texture;
    texShapesRec = source;
}

// Draw a color-filled rectangle with pro parameters
void DrawRectanglePro(Rectangle rec, Vector2 origin, float rotation, Color color)
{
    rlCheckRenderBatchLimit(4);

    Vector2 topLeft;
    Vector2 topRight;
    Vector2 bottomLeft;
    Vector2 bottomRight;

    // Only calculate rotation if needed
    if (rotation == 0.0f)
    {
        float x = rec.x - origin.x;
        float y = rec.y - origin.y;
        topLeft = Vec2( x, y );
        topRight = Vec2( x + rec.width, y );
        bottomLeft = Vec2( x, y + rec.height );
        bottomRight = Vec2( x + rec.width, y + rec.height );
    }
    else
    {
        float sinRotation = sinf(rotation*DEG2RAD);
        float cosRotation = cosf(rotation*DEG2RAD);
        float x = rec.x;
        float y = rec.y;
        float dx = -origin.x;
        float dy = -origin.y;

        topLeft.x = x + dx*cosRotation - dy*sinRotation;
        topLeft.y = y + dx*sinRotation + dy*cosRotation;

        topRight.x = x + (dx + rec.width)*cosRotation - dy*sinRotation;
        topRight.y = y + (dx + rec.width)*sinRotation + dy*cosRotation;

        bottomLeft.x = x + dx*cosRotation - (dy + rec.height)*sinRotation;
        bottomLeft.y = y + dx*sinRotation + (dy + rec.height)*cosRotation;

        bottomRight.x = x + (dx + rec.width)*cosRotation - (dy + rec.height)*sinRotation;
        bottomRight.y = y + (dx + rec.width)*sinRotation + (dy + rec.height)*cosRotation;
    }

    rlSetTexture(texShapes.id);
    rlBegin(RL_QUADS);

        rlNormal3f(0.0f, 0.0f, 1.0f);
        rlColor4ub(color.r, color.g, color.b, color.a);

        rlTexCoord2f(texShapesRec.x/texShapes.width, texShapesRec.y/texShapes.height);
        rlVertex2f(topLeft.x, topLeft.y);

        rlTexCoord2f(texShapesRec.x/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
        rlVertex2f(bottomLeft.x, bottomLeft.y);

        rlTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
        rlVertex2f(bottomRight.x, bottomRight.y);

        rlTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, texShapesRec.y/texShapes.height);
        rlVertex2f(topRight.x, topRight.y);

    rlEnd();
    rlSetTexture(0);
}

void DrawRectangleRec(Rectangle rec, Color color)
{
    DrawRectanglePro(rec, Vec2(0.0f, 0.0f ), 0.0f, color);
}


// Draw rectangle using rlgl OpenGL 1.1 style coding (translated to OpenGL 3.3 internally)
 void DrawRectangleV(float x, float y,float w, float h, Color color)
{
    rlBegin(RL_TRIANGLES);
        rlColor4ub(color.r, color.g, color.b, color.a);

        rlVertex2f(x, y);
        rlVertex2f(x, y + h);
        rlVertex2f(x + w, y + h);

        rlVertex2f(x, y);
        rlVertex2f(x + w, y + h);
        rlVertex2f(x + w, y);
    rlEnd();
}

void DrawRectangle(int posX, int posY, int width, int height, Color color)
{
    DrawRectangleV( (float)posX, (float)posY ,(float)width, (float)height, color);
}


// Draw a gradient-filled rectangle
// NOTE: Colors refer to corners, starting at top-lef corner and counter-clockwise
void DrawRectangleGradientEx(Rectangle rec, Color col1, Color col2, Color col3, Color col4)
{
    rlSetTexture(texShapes.id);

    rlPushMatrix();
        rlBegin(RL_QUADS);
            rlNormal3f(0.0f, 0.0f, 1.0f);

            // NOTE: Default raylib font character 95 is a white square
            rlColor4ub(col1.r, col1.g, col1.b, col1.a);
            rlTexCoord2f(texShapesRec.x/texShapes.width, texShapesRec.y/texShapes.height);
            rlVertex2f(rec.x, rec.y);

            rlColor4ub(col2.r, col2.g, col2.b, col2.a);
            rlTexCoord2f(texShapesRec.x/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
            rlVertex2f(rec.x, rec.y + rec.height);

            rlColor4ub(col3.r, col3.g, col3.b, col3.a);
            rlTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
            rlVertex2f(rec.x + rec.width, rec.y + rec.height);

            rlColor4ub(col4.r, col4.g, col4.b, col4.a);
            rlTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, texShapesRec.y/texShapes.height);
            rlVertex2f(rec.x + rec.width, rec.y);
        rlEnd();
    rlPopMatrix();

    rlSetTexture(0);
}
// Draw a vertical-gradient-filled rectangle
// NOTE: Gradient goes from bottom (color1) to top (color2)
void DrawRectangleGradientV(int posX, int posY, int width, int height, Color color1, Color color2)
{
    DrawRectangleGradientEx((Rectangle){ (float)posX, (float)posY, (float)width, (float)height }, color1, color2, color2, color1);
}

// Draw a horizontal-gradient-filled rectangle
// NOTE: Gradient goes from bottom (color1) to top (color2)
void DrawRectangleGradientH(int posX, int posY, int width, int height, Color color1, Color color2)
{
    DrawRectangleGradientEx((Rectangle){ (float)posX, (float)posY, (float)width, (float)height }, color1, color1, color2, color2);
}

// Draw rectangle outline
// NOTE: On OpenGL 3.3 and ES2 we use QUADS to avoid drawing order issues
void DrawRectangleLines(int posX, int posY, int width, int height, Color color)
{

    rlBegin(RL_LINES);
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlVertex2i(posX + 1, posY + 1);
        rlVertex2i(posX + width, posY + 1);

        rlVertex2i(posX + width, posY + 1);
        rlVertex2i(posX + width, posY + height);

        rlVertex2i(posX + width, posY + height);
        rlVertex2i(posX + 1, posY + height);

        rlVertex2i(posX + 1, posY + height);
        rlVertex2i(posX + 1, posY + 1);
    rlEnd();

}

// Draw rectangle outline with extended parameters
void DrawRectangleLinesEx(Rectangle rec, int lineThick, Color color)
{
    if ((lineThick > rec.width) || (lineThick > rec.height))
    {
        if (rec.width > rec.height) lineThick = (int)rec.height/2;
        else if (rec.width < rec.height) lineThick = (int)rec.width/2;
    }

    // When rec = { x, y, 8.0f, 6.0f } and lineThick = 2, the following
    // four rectangles are drawn ([T]op, [B]ottom, [L]eft, [R]ight):
    //
    //   TTTTTTTT
    //   TTTTTTTT
    //   LL    RR
    //   LL    RR
    //   BBBBBBBB
    //   BBBBBBBB
    //
    float thick = (float)lineThick;
    Rectangle top = { rec.x, rec.y, rec.width, thick };
    Rectangle bottom = { rec.x, rec.y - thick + rec.height, rec.width, thick };
    Rectangle left = { rec.x, rec.y + thick, thick, rec.height - thick*2.0f };
    Rectangle right = { rec.x - thick + rec.width, rec.y + thick, thick, rec.height - thick*2.0f };

    DrawRectangleRec(top, color);
    DrawRectangleRec(bottom, color);
    DrawRectangleRec(left, color);
    DrawRectangleRec(right, color);
}

// Draw rectangle with rounded edges
void DrawRectangleRounded(Rectangle rec, float roundness, int segments, Color color)
{
    // Not a rounded rectangle
    if ((roundness <= 0.0f) || (rec.width < 1) || (rec.height < 1 ))
    {
        DrawRectangleRec(rec, color);
        return;
    }

    if (roundness >= 1.0f) roundness = 1.0f;

    // Calculate corner radius
    float radius = (rec.width > rec.height)? (rec.height*roundness)/2 : (rec.width*roundness)/2;
    if (radius <= 0.0f) return;

    // Calculate number of segments to use for the corners
    if (segments < 4)
    {
        // Calculate the maximum angle between segments based on the error rate (usually 0.5f)
        float th = acosf(2*powf(1 - SMOOTH_CIRCLE_ERROR_RATE/radius, 2) - 1);
        segments = (int)(ceilf(2*PI/th)/4.0f);
        if (segments <= 0) segments = 4;
    }

    float stepLength = 90.0f/(float)segments;

    /*
    Quick sketch to make sense of all of this,
    there are 9 parts to draw, also mark the 12 points we'll use

          P0____________________P1
          /|                    |\
         /1|          2         |3\
     P7 /__|____________________|__\ P2
       |   |P8                P9|   |
       | 8 |          9         | 4 |
       | __|____________________|__ |
     P6 \  |P11              P10|  / P3
         \7|          6         |5/
          \|____________________|/
          P5                    P4
    */
    // Coordinates of the 12 points that define the rounded rect
    const Vector2 point[12] = {
        {(float)rec.x + radius, rec.y}, {(float)(rec.x + rec.width) - radius, rec.y}, { rec.x + rec.width, (float)rec.y + radius },     // PO, P1, P2
        {rec.x + rec.width, (float)(rec.y + rec.height) - radius}, {(float)(rec.x + rec.width) - radius, rec.y + rec.height},           // P3, P4
        {(float)rec.x + radius, rec.y + rec.height}, { rec.x, (float)(rec.y + rec.height) - radius}, {rec.x, (float)rec.y + radius},    // P5, P6, P7
        {(float)rec.x + radius, (float)rec.y + radius}, {(float)(rec.x + rec.width) - radius, (float)rec.y + radius},                   // P8, P9
        {(float)(rec.x + rec.width) - radius, (float)(rec.y + rec.height) - radius}, {(float)rec.x + radius, (float)(rec.y + rec.height) - radius} // P10, P11
    };

    const Vector2 centers[4] = { point[8], point[9], point[10], point[11] };
    const float angles[4] = { 180.0f, 90.0f, 0.0f, 270.0f };


    rlCheckRenderBatchLimit(16*segments/2 + 5*4);

    rlSetTexture(texShapes.id);

    rlBegin(RL_QUADS);
        // Draw all of the 4 corners: [1] Upper Left Corner, [3] Upper Right Corner, [5] Lower Right Corner, [7] Lower Left Corner
        for (int k = 0; k < 4; ++k) // Hope the compiler is smart enough to unroll this loop
        {
            float angle = angles[k];
            const Vector2 center = centers[k];

            // NOTE: Every QUAD actually represents two segments
            for (int i = 0; i < segments/2; i++)
            {
                rlColor4ub(color.r, color.g, color.b, color.a);
                rlTexCoord2f(texShapesRec.x/texShapes.width, texShapesRec.y/texShapes.height);
                rlVertex2f(center.x, center.y);
                rlTexCoord2f(texShapesRec.x/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
                rlVertex2f(center.x + sinf(DEG2RAD*angle)*radius, center.y + cosf(DEG2RAD*angle)*radius);
                rlTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
                rlVertex2f(center.x + sinf(DEG2RAD*(angle + stepLength))*radius, center.y + cosf(DEG2RAD*(angle + stepLength))*radius);
                rlTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, texShapesRec.y/texShapes.height);
                rlVertex2f(center.x + sinf(DEG2RAD*(angle + stepLength*2))*radius, center.y + cosf(DEG2RAD*(angle + stepLength*2))*radius);
                angle += (stepLength*2);
            }

            // NOTE: In case number of segments is odd, we add one last piece to the cake
            if (segments%2)
            {
                rlColor4ub(color.r, color.g, color.b, color.a);
                rlTexCoord2f(texShapesRec.x/texShapes.width, texShapesRec.y/texShapes.height);
                rlVertex2f(center.x, center.y);
                rlTexCoord2f(texShapesRec.x/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
                rlVertex2f(center.x + sinf(DEG2RAD*angle)*radius, center.y + cosf(DEG2RAD*angle)*radius);
                rlTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
                rlVertex2f(center.x + sinf(DEG2RAD*(angle + stepLength))*radius, center.y + cosf(DEG2RAD*(angle + stepLength))*radius);
                rlTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, texShapesRec.y/texShapes.height);
                rlVertex2f(center.x, center.y);
            }
        }

        // [2] Upper Rectangle
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlTexCoord2f(texShapesRec.x/texShapes.width, texShapesRec.y/texShapes.height);
        rlVertex2f(point[0].x, point[0].y);
        rlTexCoord2f(texShapesRec.x/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
        rlVertex2f(point[8].x, point[8].y);
        rlTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
        rlVertex2f(point[9].x, point[9].y);
        rlTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, texShapesRec.y/texShapes.height);
        rlVertex2f(point[1].x, point[1].y);

        // [4] Right Rectangle
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlTexCoord2f(texShapesRec.x/texShapes.width, texShapesRec.y/texShapes.height);
        rlVertex2f(point[2].x, point[2].y);
        rlTexCoord2f(texShapesRec.x/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
        rlVertex2f(point[9].x, point[9].y);
        rlTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
        rlVertex2f(point[10].x, point[10].y);
        rlTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, texShapesRec.y/texShapes.height);
        rlVertex2f(point[3].x, point[3].y);

        // [6] Bottom Rectangle
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlTexCoord2f(texShapesRec.x/texShapes.width, texShapesRec.y/texShapes.height);
        rlVertex2f(point[11].x, point[11].y);
        rlTexCoord2f(texShapesRec.x/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
        rlVertex2f(point[5].x, point[5].y);
        rlTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
        rlVertex2f(point[4].x, point[4].y);
        rlTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, texShapesRec.y/texShapes.height);
        rlVertex2f(point[10].x, point[10].y);

        // [8] Left Rectangle
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlTexCoord2f(texShapesRec.x/texShapes.width, texShapesRec.y/texShapes.height);
        rlVertex2f(point[7].x, point[7].y);
        rlTexCoord2f(texShapesRec.x/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
        rlVertex2f(point[6].x, point[6].y);
        rlTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
        rlVertex2f(point[11].x, point[11].y);
        rlTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, texShapesRec.y/texShapes.height);
        rlVertex2f(point[8].x, point[8].y);

        // [9] Middle Rectangle
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlTexCoord2f(texShapesRec.x/texShapes.width, texShapesRec.y/texShapes.height);
        rlVertex2f(point[8].x, point[8].y);
        rlTexCoord2f(texShapesRec.x/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
        rlVertex2f(point[11].x, point[11].y);
        rlTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
        rlVertex2f(point[10].x, point[10].y);
        rlTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, texShapesRec.y/texShapes.height);
        rlVertex2f(point[9].x, point[9].y);

    rlEnd();
    rlSetTexture(0);

}

// Draw a triangle using lines
// NOTE: Vertex must be provided in counter-clockwise order
void DrawTriangleLines(Vector2 v1, Vector2 v2, Vector2 v3, Color color)
{
    rlCheckRenderBatchLimit(6);

    rlBegin(RL_LINES);
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlVertex2f(v1.x, v1.y);
        rlVertex2f(v2.x, v2.y);

        rlVertex2f(v2.x, v2.y);
        rlVertex2f(v3.x, v3.y);

        rlVertex2f(v3.x, v3.y);
        rlVertex2f(v1.x, v1.y);
    rlEnd();
}

// Draw a triangle fan defined by points
// NOTE: First vertex provided is the center, shared by all triangles
// By default, following vertex should be provided in counter-clockwise order
void DrawTriangleFan(Vector2 *points, int pointsCount, Color color)
{
    if (pointsCount >= 3)
    {
        rlCheckRenderBatchLimit((pointsCount - 2)*4);

        rlSetTexture(texShapes.id);
        rlBegin(RL_QUADS);
            rlColor4ub(color.r, color.g, color.b, color.a);

            for (int i = 1; i < pointsCount - 1; i++)
            {
                rlTexCoord2f(texShapesRec.x/texShapes.width, texShapesRec.y/texShapes.height);
                rlVertex2f(points[0].x, points[0].y);

                rlTexCoord2f(texShapesRec.x/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
                rlVertex2f(points[i].x, points[i].y);

                rlTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
                rlVertex2f(points[i + 1].x, points[i + 1].y);

                rlTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, texShapesRec.y/texShapes.height);
                rlVertex2f(points[i + 1].x, points[i + 1].y);
            }
        rlEnd();
        rlSetTexture(0);
    }
}

// Draw a triangle strip defined by points
// NOTE: Every new vertex connects with previous two
void DrawTriangleStrip(Vector2 *points, int pointsCount, Color color)
{
    if (pointsCount >= 3)
    {
        rlCheckRenderBatchLimit(3*(pointsCount - 2));

        rlBegin(RL_TRIANGLES);
            rlColor4ub(color.r, color.g, color.b, color.a);

            for (int i = 2; i < pointsCount; i++)
            {
                if ((i%2) == 0)
                {
                    rlVertex2f(points[i].x, points[i].y);
                    rlVertex2f(points[i - 2].x, points[i - 2].y);
                    rlVertex2f(points[i - 1].x, points[i - 1].y);
                }
                else
                {
                    rlVertex2f(points[i].x, points[i].y);
                    rlVertex2f(points[i - 1].x, points[i - 1].y);
                    rlVertex2f(points[i - 2].x, points[i - 2].y);
                }
            }
        rlEnd();
    }
}

// Draw a regular polygon of n sides (Vector version)
void DrawPoly(Vector2 center, int sides, float radius, float rotation, Color color)
{
    if (sides < 3) sides = 3;
    float centralAngle = 0.0f;

    rlCheckRenderBatchLimit(4*(360/sides));

    rlPushMatrix();
        rlTranslatef(center.x, center.y, 0.0f);
        rlRotatef(rotation, 0.0f, 0.0f, 1.0f);

        rlSetTexture(texShapes.id);

        rlBegin(RL_QUADS);
            for (int i = 0; i < sides; i++)
            {
                rlColor4ub(color.r, color.g, color.b, color.a);

                rlTexCoord2f(texShapesRec.x/texShapes.width, texShapesRec.y/texShapes.height);
                rlVertex2f(0, 0);

                rlTexCoord2f(texShapesRec.x/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
                rlVertex2f(sinf(DEG2RAD*centralAngle)*radius, cosf(DEG2RAD*centralAngle)*radius);

                rlTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
                rlVertex2f(sinf(DEG2RAD*centralAngle)*radius, cosf(DEG2RAD*centralAngle)*radius);

                centralAngle += 360.0f/(float)sides;
                rlTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, texShapesRec.y/texShapes.height);
                rlVertex2f(sinf(DEG2RAD*centralAngle)*radius, cosf(DEG2RAD*centralAngle)*radius);
            }
        rlEnd();
        rlSetTexture(0);

    rlPopMatrix();
}

// Draw a polygon outline of n sides
void DrawPolyLines(Vector2 center, int sides, float radius, float rotation, Color color)
{
    if (sides < 3) sides = 3;
    float centralAngle = 0.0f;

    rlCheckRenderBatchLimit(3*(360/sides));

    rlPushMatrix();
        rlTranslatef(center.x, center.y, 0.0f);
        rlRotatef(rotation, 0.0f, 0.0f, 1.0f);

        rlBegin(RL_LINES);
            for (int i = 0; i < sides; i++)
            {
                rlColor4ub(color.r, color.g, color.b, color.a);

                rlVertex2f(sinf(DEG2RAD*centralAngle)*radius, cosf(DEG2RAD*centralAngle)*radius);
                centralAngle += 360.0f/(float)sides;
                rlVertex2f(sinf(DEG2RAD*centralAngle)*radius, cosf(DEG2RAD*centralAngle)*radius);
            }
        rlEnd();
    rlPopMatrix();
}
// Draw a pixel (Vector version)
void DrawPixelV(Vector2 position, Color color)
{
    rlBegin(RL_LINES);
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlVertex2f(position.x, position.y);
        rlVertex2f(position.x + 1.0f, position.y + 1.0f);
    rlEnd();
}

// Draw a line
void DrawLine(int startPosX, int startPosY, int endPosX, int endPosY, Color color)
{
    rlBegin(RL_LINES);
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlVertex2i(startPosX, startPosY);
        rlVertex2i(endPosX, endPosY);
    rlEnd();
}

// Draw a line  (Vector version)
void DrawLineV(Vector2 startPos, Vector2 endPos, Color color)
{
    rlBegin(RL_LINES);
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlVertex2f(startPos.x, startPos.y);
        rlVertex2f(endPos.x, endPos.y);
    rlEnd();
}

// Draw a line defining thickness
void DrawLineEx(Vector2 startPos, Vector2 endPos, float thick, Color color)
{
    Vector2 delta = {endPos.x-startPos.x, endPos.y-startPos.y};
    float   length = sqrtf(delta.x*delta.x + delta.y*delta.y);

    if (length > 0  &&  thick > 0)
    {
        float   scale = thick/(2*length);
        Vector2 radius = {-scale*delta.y, scale*delta.x};
        Vector2 strip[] = {{startPos.x-radius.x, startPos.y-radius.y}, {startPos.x+radius.x, startPos.y+radius.y},
                           {endPos.x-radius.x, endPos.y-radius.y}, {endPos.x+radius.x, endPos.y+radius.y}};

        DrawTriangleStrip(strip, 4, color);
    }
}

// Draw rectangle with rounded edges outline
void DrawRectangleRoundedLines(Rectangle rec, float roundness, int segments, int lineThick, Color color)
{
    if (lineThick < 0) lineThick = 0;

    // Not a rounded rectangle
    if (roundness <= 0.0f)
    {
        DrawRectangleLinesEx((Rectangle){rec.x-lineThick, rec.y-lineThick, rec.width+2*lineThick, rec.height+2*lineThick}, lineThick, color);
        return;
    }

    if (roundness >= 1.0f) roundness = 1.0f;

    // Calculate corner radius
    float radius = (rec.width > rec.height)? (rec.height*roundness)/2 : (rec.width*roundness)/2;
    if (radius <= 0.0f) return;

    // Calculate number of segments to use for the corners
    if (segments < 4)
    {
        // Calculate the maximum angle between segments based on the error rate (usually 0.5f)
        float th = acosf(2*powf(1 - SMOOTH_CIRCLE_ERROR_RATE/radius, 2) - 1);
        segments = (int)(ceilf(2*PI/th)/2.0f);
        if (segments <= 0) segments = 4;
    }

    float stepLength = 90.0f/(float)segments;
    const float outerRadius = radius + (float)lineThick, innerRadius = radius;

    /*
    Quick sketch to make sense of all of this,
    marks the 16 + 4(corner centers P16-19) points we'll use

           P0 ================== P1
          // P8                P9 \\
         //                        \\
     P7 // P15                  P10 \\ P2
       ||   *P16             P17*    ||
       ||                            ||
       || P14                   P11  ||
     P6 \\  *P19             P18*   // P3
         \\                        //
          \\ P13              P12 //
           P5 ================== P4
    */
    const Vector2 point[16] = {
        {(float)rec.x + innerRadius, rec.y - lineThick}, {(float)(rec.x + rec.width) - innerRadius, rec.y - lineThick}, { rec.x + rec.width + lineThick, (float)rec.y + innerRadius }, // PO, P1, P2
        {rec.x + rec.width + lineThick, (float)(rec.y + rec.height) - innerRadius}, {(float)(rec.x + rec.width) - innerRadius, rec.y + rec.height + lineThick}, // P3, P4
        {(float)rec.x + innerRadius, rec.y + rec.height + lineThick}, { rec.x - lineThick, (float)(rec.y + rec.height) - innerRadius}, {rec.x - lineThick, (float)rec.y + innerRadius}, // P5, P6, P7
        {(float)rec.x + innerRadius, rec.y}, {(float)(rec.x + rec.width) - innerRadius, rec.y}, // P8, P9
        { rec.x + rec.width, (float)rec.y + innerRadius }, {rec.x + rec.width, (float)(rec.y + rec.height) - innerRadius}, // P10, P11
        {(float)(rec.x + rec.width) - innerRadius, rec.y + rec.height}, {(float)rec.x + innerRadius, rec.y + rec.height}, // P12, P13
        { rec.x, (float)(rec.y + rec.height) - innerRadius}, {rec.x, (float)rec.y + innerRadius} // P14, P15
    };

    const Vector2 centers[4] = {
        {(float)rec.x + innerRadius, (float)rec.y + innerRadius}, {(float)(rec.x + rec.width) - innerRadius, (float)rec.y + innerRadius}, // P16, P17
        {(float)(rec.x + rec.width) - innerRadius, (float)(rec.y + rec.height) - innerRadius}, {(float)rec.x + innerRadius, (float)(rec.y + rec.height) - innerRadius} // P18, P19
    };

    const float angles[4] = { 180.0f, 90.0f, 0.0f, 270.0f };

    if (lineThick > 1)
    {

        rlCheckRenderBatchLimit(4*4*segments + 4*4); // 4 corners with 4 vertices for each segment + 4 rectangles with 4 vertices each

        rlSetTexture(texShapes.id);

        rlBegin(RL_QUADS);

            // Draw all of the 4 corners first: Upper Left Corner, Upper Right Corner, Lower Right Corner, Lower Left Corner
            for (int k = 0; k < 4; ++k) // Hope the compiler is smart enough to unroll this loop
            {
                float angle = angles[k];
                const Vector2 center = centers[k];
                for (int i = 0; i < segments; i++)
                {
                    rlColor4ub(color.r, color.g, color.b, color.a);
                    rlTexCoord2f(texShapesRec.x/texShapes.width, texShapesRec.y/texShapes.height);
                    rlVertex2f(center.x + sinf(DEG2RAD*angle)*innerRadius, center.y + cosf(DEG2RAD*angle)*innerRadius);
                    rlTexCoord2f(texShapesRec.x/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
                    rlVertex2f(center.x + sinf(DEG2RAD*angle)*outerRadius, center.y + cosf(DEG2RAD*angle)*outerRadius);
                    rlTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
                    rlVertex2f(center.x + sinf(DEG2RAD*(angle + stepLength))*outerRadius, center.y + cosf(DEG2RAD*(angle + stepLength))*outerRadius);
                    rlTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, texShapesRec.y/texShapes.height);
                    rlVertex2f(center.x + sinf(DEG2RAD*(angle + stepLength))*innerRadius, center.y + cosf(DEG2RAD*(angle + stepLength))*innerRadius);

                    angle += stepLength;
                }
            }

            // Upper rectangle
            rlColor4ub(color.r, color.g, color.b, color.a);
            rlTexCoord2f(texShapesRec.x/texShapes.width, texShapesRec.y/texShapes.height);
            rlVertex2f(point[0].x, point[0].y);
            rlTexCoord2f(texShapesRec.x/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
            rlVertex2f(point[8].x, point[8].y);
            rlTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
            rlVertex2f(point[9].x, point[9].y);
            rlTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, texShapesRec.y/texShapes.height);
            rlVertex2f(point[1].x, point[1].y);

            // Right rectangle
            rlColor4ub(color.r, color.g, color.b, color.a);
            rlTexCoord2f(texShapesRec.x/texShapes.width, texShapesRec.y/texShapes.height);
            rlVertex2f(point[2].x, point[2].y);
            rlTexCoord2f(texShapesRec.x/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
            rlVertex2f(point[10].x, point[10].y);
            rlTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
            rlVertex2f(point[11].x, point[11].y);
            rlTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, texShapesRec.y/texShapes.height);
            rlVertex2f(point[3].x, point[3].y);

            // Lower rectangle
            rlColor4ub(color.r, color.g, color.b, color.a);
            rlTexCoord2f(texShapesRec.x/texShapes.width, texShapesRec.y/texShapes.height);
            rlVertex2f(point[13].x, point[13].y);
            rlTexCoord2f(texShapesRec.x/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
            rlVertex2f(point[5].x, point[5].y);
            rlTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
            rlVertex2f(point[4].x, point[4].y);
            rlTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, texShapesRec.y/texShapes.height);
            rlVertex2f(point[12].x, point[12].y);

            // Left rectangle
            rlColor4ub(color.r, color.g, color.b, color.a);
            rlTexCoord2f(texShapesRec.x/texShapes.width, texShapesRec.y/texShapes.height);
            rlVertex2f(point[15].x, point[15].y);
            rlTexCoord2f(texShapesRec.x/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
            rlVertex2f(point[7].x, point[7].y);
            rlTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
            rlVertex2f(point[6].x, point[6].y);
            rlTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, texShapesRec.y/texShapes.height);
            rlVertex2f(point[14].x, point[14].y);

        rlEnd();
        rlSetTexture(0);
    }
    else
    {
        // Use LINES to draw the outline
        rlCheckRenderBatchLimit(8*segments + 4*2); // 4 corners with 2 vertices for each segment + 4 rectangles with 2 vertices each

        rlBegin(RL_LINES);

            // Draw all of the 4 corners first: Upper Left Corner, Upper Right Corner, Lower Right Corner, Lower Left Corner
            for (int k = 0; k < 4; ++k) // Hope the compiler is smart enough to unroll this loop
            {
                float angle = angles[k];
                const Vector2 center = centers[k];

                for (int i = 0; i < segments; i++)
                {
                    rlColor4ub(color.r, color.g, color.b, color.a);
                    rlVertex2f(center.x + sinf(DEG2RAD*angle)*outerRadius, center.y + cosf(DEG2RAD*angle)*outerRadius);
                    rlVertex2f(center.x + sinf(DEG2RAD*(angle + stepLength))*outerRadius, center.y + cosf(DEG2RAD*(angle + stepLength))*outerRadius);
                    angle += stepLength;
                }
            }

            // And now the remaining 4 lines
            for (int i = 0; i < 8; i += 2)
            {
                rlColor4ub(color.r, color.g, color.b, color.a);
                rlVertex2f(point[i].x, point[i].y);
                rlVertex2f(point[i + 1].x, point[i + 1].y);
            }

        rlEnd();
    }
}


// Draw a grid centered at (0, 0, 0)
 void DrawGrid(int slices, float spacing)
{
    int halfSlices = slices / 2;

    rlBegin(RL_LINES);
        for(int i = -halfSlices; i <= halfSlices; i++)
        {
            if (i == 0)
            {
                rlColor3f(0.5f, 0.5f, 0.5f);
                rlColor3f(0.5f, 0.5f, 0.5f);
                rlColor3f(0.5f, 0.5f, 0.5f);
                rlColor3f(0.5f, 0.5f, 0.5f);
            }
            else
            {
                rlColor3f(0.75f, 0.75f, 0.75f);
                rlColor3f(0.75f, 0.75f, 0.75f);
                rlColor3f(0.75f, 0.75f, 0.75f);
                rlColor3f(0.75f, 0.75f, 0.75f);
            }

            rlVertex3f((float)i*spacing, 0.0f, (float)-halfSlices*spacing);
            rlVertex3f((float)i*spacing, 0.0f, (float)halfSlices*spacing);

            rlVertex3f((float)-halfSlices*spacing, 0.0f, (float)i*spacing);
            rlVertex3f((float)halfSlices*spacing, 0.0f, (float)i*spacing);
        }
    rlEnd();
}

// Draw cube
// NOTE: Cube position is the center position
void DrawCube(Vector3 position, float width, float height, float length, Color color)
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    rlCheckRenderBatchLimit(36);

    rlPushMatrix();
        // NOTE: Transformation is applied in inverse order (scale -> rotate -> translate)
        rlTranslatef(position.x, position.y, position.z);
        //rlRotatef(45, 0, 1, 0);
        //rlScalef(1.0f, 1.0f, 1.0f);   // NOTE: Vertices are directly scaled on definition

        rlBegin(RL_TRIANGLES);
            rlColor4ub(color.r, color.g, color.b, color.a);

            // Front face
            rlVertex3f(x - width/2, y - height/2, z + length/2);  // Bottom Left
            rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Right
            rlVertex3f(x - width/2, y + height/2, z + length/2);  // Top Left

            rlVertex3f(x + width/2, y + height/2, z + length/2);  // Top Right
            rlVertex3f(x - width/2, y + height/2, z + length/2);  // Top Left
            rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Right

            // Back face
            rlVertex3f(x - width/2, y - height/2, z - length/2);  // Bottom Left
            rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Left
            rlVertex3f(x + width/2, y - height/2, z - length/2);  // Bottom Right

            rlVertex3f(x + width/2, y + height/2, z - length/2);  // Top Right
            rlVertex3f(x + width/2, y - height/2, z - length/2);  // Bottom Right
            rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Left

            // Top face
            rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Left
            rlVertex3f(x - width/2, y + height/2, z + length/2);  // Bottom Left
            rlVertex3f(x + width/2, y + height/2, z + length/2);  // Bottom Right

            rlVertex3f(x + width/2, y + height/2, z - length/2);  // Top Right
            rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Left
            rlVertex3f(x + width/2, y + height/2, z + length/2);  // Bottom Right

            // Bottom face
            rlVertex3f(x - width/2, y - height/2, z - length/2);  // Top Left
            rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Right
            rlVertex3f(x - width/2, y - height/2, z + length/2);  // Bottom Left

            rlVertex3f(x + width/2, y - height/2, z - length/2);  // Top Right
            rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Right
            rlVertex3f(x - width/2, y - height/2, z - length/2);  // Top Left

            // Right face
            rlVertex3f(x + width/2, y - height/2, z - length/2);  // Bottom Right
            rlVertex3f(x + width/2, y + height/2, z - length/2);  // Top Right
            rlVertex3f(x + width/2, y + height/2, z + length/2);  // Top Left

            rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Left
            rlVertex3f(x + width/2, y - height/2, z - length/2);  // Bottom Right
            rlVertex3f(x + width/2, y + height/2, z + length/2);  // Top Left

            // Left face
            rlVertex3f(x - width/2, y - height/2, z - length/2);  // Bottom Right
            rlVertex3f(x - width/2, y + height/2, z + length/2);  // Top Left
            rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Right

            rlVertex3f(x - width/2, y - height/2, z + length/2);  // Bottom Left
            rlVertex3f(x - width/2, y + height/2, z + length/2);  // Top Left
            rlVertex3f(x - width/2, y - height/2, z - length/2);  // Bottom Right
        rlEnd();
    rlPopMatrix();
}

// Draw cube (Vector version)
void DrawCubeV(Vector3 position, Vector3 size, Color color)
{
    DrawCube(position, size.x, size.y, size.z, color);
}

// Draw cube wires
void DrawCubeWires(Vector3 position, float width, float height, float length, Color color)
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    rlCheckRenderBatchLimit(36);

    rlPushMatrix();
        rlTranslatef(position.x, position.y, position.z);

        rlBegin(RL_LINES);
            rlColor4ub(color.r, color.g, color.b, color.a);

            // Front Face -----------------------------------------------------
            // Bottom Line
            rlVertex3f(x-width/2, y-height/2, z+length/2);  // Bottom Left
            rlVertex3f(x+width/2, y-height/2, z+length/2);  // Bottom Right

            // Left Line
            rlVertex3f(x+width/2, y-height/2, z+length/2);  // Bottom Right
            rlVertex3f(x+width/2, y+height/2, z+length/2);  // Top Right

            // Top Line
            rlVertex3f(x+width/2, y+height/2, z+length/2);  // Top Right
            rlVertex3f(x-width/2, y+height/2, z+length/2);  // Top Left

            // Right Line
            rlVertex3f(x-width/2, y+height/2, z+length/2);  // Top Left
            rlVertex3f(x-width/2, y-height/2, z+length/2);  // Bottom Left

            // Back Face ------------------------------------------------------
            // Bottom Line
            rlVertex3f(x-width/2, y-height/2, z-length/2);  // Bottom Left
            rlVertex3f(x+width/2, y-height/2, z-length/2);  // Bottom Right

            // Left Line
            rlVertex3f(x+width/2, y-height/2, z-length/2);  // Bottom Right
            rlVertex3f(x+width/2, y+height/2, z-length/2);  // Top Right

            // Top Line
            rlVertex3f(x+width/2, y+height/2, z-length/2);  // Top Right
            rlVertex3f(x-width/2, y+height/2, z-length/2);  // Top Left

            // Right Line
            rlVertex3f(x-width/2, y+height/2, z-length/2);  // Top Left
            rlVertex3f(x-width/2, y-height/2, z-length/2);  // Bottom Left

            // Top Face -------------------------------------------------------
            // Left Line
            rlVertex3f(x-width/2, y+height/2, z+length/2);  // Top Left Front
            rlVertex3f(x-width/2, y+height/2, z-length/2);  // Top Left Back

            // Right Line
            rlVertex3f(x+width/2, y+height/2, z+length/2);  // Top Right Front
            rlVertex3f(x+width/2, y+height/2, z-length/2);  // Top Right Back

            // Bottom Face  ---------------------------------------------------
            // Left Line
            rlVertex3f(x-width/2, y-height/2, z+length/2);  // Top Left Front
            rlVertex3f(x-width/2, y-height/2, z-length/2);  // Top Left Back

            // Right Line
            rlVertex3f(x+width/2, y-height/2, z+length/2);  // Top Right Front
            rlVertex3f(x+width/2, y-height/2, z-length/2);  // Top Right Back
        rlEnd();
    rlPopMatrix();
}

// Draw cube wires (vector version)
void DrawCubeWiresV(Vector3 position, Vector3 size, Color color)
{
    DrawCubeWires(position, size.x, size.y, size.z, color);
}



// Draw sphere with extended parameters
void DrawSphereEx(Vector3 centerPos, float radius, int rings, int slices, Color color)
{
    int numVertex = (rings + 2)*slices*6;
    rlCheckRenderBatchLimit(numVertex);

    rlPushMatrix();
        // NOTE: Transformation is applied in inverse order (scale -> translate)
        rlTranslatef(centerPos.x, centerPos.y, centerPos.z);
        rlScalef(radius, radius, radius);

        rlBegin(RL_TRIANGLES);
            rlColor4ub(color.r, color.g, color.b, color.a);

            for (int i = 0; i < (rings + 2); i++)
            {
                for (int j = 0; j < slices; j++)
                {
                    rlVertex3f(cosf(DEG2RAD*(270+(180/(rings + 1))*i))*sinf(DEG2RAD*(j*360/slices)),
                               sinf(DEG2RAD*(270+(180/(rings + 1))*i)),
                               cosf(DEG2RAD*(270+(180/(rings + 1))*i))*cosf(DEG2RAD*(j*360/slices)));
                    rlVertex3f(cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*sinf(DEG2RAD*((j+1)*360/slices)),
                               sinf(DEG2RAD*(270+(180/(rings + 1))*(i+1))),
                               cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*cosf(DEG2RAD*((j+1)*360/slices)));
                    rlVertex3f(cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*sinf(DEG2RAD*(j*360/slices)),
                               sinf(DEG2RAD*(270+(180/(rings + 1))*(i+1))),
                               cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*cosf(DEG2RAD*(j*360/slices)));

                    rlVertex3f(cosf(DEG2RAD*(270+(180/(rings + 1))*i))*sinf(DEG2RAD*(j*360/slices)),
                               sinf(DEG2RAD*(270+(180/(rings + 1))*i)),
                               cosf(DEG2RAD*(270+(180/(rings + 1))*i))*cosf(DEG2RAD*(j*360/slices)));
                    rlVertex3f(cosf(DEG2RAD*(270+(180/(rings + 1))*(i)))*sinf(DEG2RAD*((j+1)*360/slices)),
                               sinf(DEG2RAD*(270+(180/(rings + 1))*(i))),
                               cosf(DEG2RAD*(270+(180/(rings + 1))*(i)))*cosf(DEG2RAD*((j+1)*360/slices)));
                    rlVertex3f(cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*sinf(DEG2RAD*((j+1)*360/slices)),
                               sinf(DEG2RAD*(270+(180/(rings + 1))*(i+1))),
                               cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*cosf(DEG2RAD*((j+1)*360/slices)));
                }
            }
        rlEnd();
    rlPopMatrix();
}
// Draw sphere
void DrawSphere(Vector3 centerPos, float radius, Color color)
{
    DrawSphereEx(centerPos, radius, 16, 16, color);
}

// Draw sphere wires
void DrawSphereWires(Vector3 centerPos, float radius, int rings, int slices, Color color)
{
    int numVertex = (rings + 2)*slices*6;
    rlCheckRenderBatchLimit(numVertex);

    rlPushMatrix();
        // NOTE: Transformation is applied in inverse order (scale -> translate)
        rlTranslatef(centerPos.x, centerPos.y, centerPos.z);
        rlScalef(radius, radius, radius);

        rlBegin(RL_LINES);
            rlColor4ub(color.r, color.g, color.b, color.a);

            for (int i = 0; i < (rings + 2); i++)
            {
                for (int j = 0; j < slices; j++)
                {
                    rlVertex3f(cosf(DEG2RAD*(270+(180/(rings + 1))*i))*sinf(DEG2RAD*(j*360/slices)),
                               sinf(DEG2RAD*(270+(180/(rings + 1))*i)),
                               cosf(DEG2RAD*(270+(180/(rings + 1))*i))*cosf(DEG2RAD*(j*360/slices)));
                    rlVertex3f(cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*sinf(DEG2RAD*((j+1)*360/slices)),
                               sinf(DEG2RAD*(270+(180/(rings + 1))*(i+1))),
                               cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*cosf(DEG2RAD*((j+1)*360/slices)));

                    rlVertex3f(cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*sinf(DEG2RAD*((j+1)*360/slices)),
                               sinf(DEG2RAD*(270+(180/(rings + 1))*(i+1))),
                               cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*cosf(DEG2RAD*((j+1)*360/slices)));
                    rlVertex3f(cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*sinf(DEG2RAD*(j*360/slices)),
                               sinf(DEG2RAD*(270+(180/(rings + 1))*(i+1))),
                               cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*cosf(DEG2RAD*(j*360/slices)));

                    rlVertex3f(cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*sinf(DEG2RAD*(j*360/slices)),
                               sinf(DEG2RAD*(270+(180/(rings + 1))*(i+1))),
                               cosf(DEG2RAD*(270+(180/(rings + 1))*(i+1)))*cosf(DEG2RAD*(j*360/slices)));
                    rlVertex3f(cosf(DEG2RAD*(270+(180/(rings + 1))*i))*sinf(DEG2RAD*(j*360/slices)),
                               sinf(DEG2RAD*(270+(180/(rings + 1))*i)),
                               cosf(DEG2RAD*(270+(180/(rings + 1))*i))*cosf(DEG2RAD*(j*360/slices)));
                }
            }
        rlEnd();
    rlPopMatrix();
}

// Draw a cylinder
// NOTE: It could be also used for pyramid and cone
void DrawCylinder(Vector3 position, float radiusTop, float radiusBottom, float height, int sides, Color color)
{
    if (sides < 3) sides = 3;

    int numVertex = sides*6;
    rlCheckRenderBatchLimit(numVertex);

    rlPushMatrix();
        rlTranslatef(position.x, position.y, position.z);

        rlBegin(RL_TRIANGLES);
            rlColor4ub(color.r, color.g, color.b, color.a);

            if (radiusTop > 0)
            {
                // Draw Body -------------------------------------------------------------------------------------
                for (int i = 0; i < 360; i += 360/sides)
                {
                    rlVertex3f(sinf(DEG2RAD*i)*radiusBottom, 0, cosf(DEG2RAD*i)*radiusBottom); //Bottom Left
                    rlVertex3f(sinf(DEG2RAD*(i + 360/sides))*radiusBottom, 0, cosf(DEG2RAD*(i + 360/sides))*radiusBottom); //Bottom Right
                    rlVertex3f(sinf(DEG2RAD*(i + 360/sides))*radiusTop, height, cosf(DEG2RAD*(i + 360/sides))*radiusTop); //Top Right

                    rlVertex3f(sinf(DEG2RAD*i)*radiusTop, height, cosf(DEG2RAD*i)*radiusTop); //Top Left
                    rlVertex3f(sinf(DEG2RAD*i)*radiusBottom, 0, cosf(DEG2RAD*i)*radiusBottom); //Bottom Left
                    rlVertex3f(sinf(DEG2RAD*(i + 360/sides))*radiusTop, height, cosf(DEG2RAD*(i + 360/sides))*radiusTop); //Top Right
                }

                // Draw Cap --------------------------------------------------------------------------------------
                for (int i = 0; i < 360; i += 360/sides)
                {
                    rlVertex3f(0, height, 0);
                    rlVertex3f(sinf(DEG2RAD*i)*radiusTop, height, cosf(DEG2RAD*i)*radiusTop);
                    rlVertex3f(sinf(DEG2RAD*(i + 360/sides))*radiusTop, height, cosf(DEG2RAD*(i + 360/sides))*radiusTop);
                }
            }
            else
            {
                // Draw Cone -------------------------------------------------------------------------------------
                for (int i = 0; i < 360; i += 360/sides)
                {
                    rlVertex3f(0, height, 0);
                    rlVertex3f(sinf(DEG2RAD*i)*radiusBottom, 0, cosf(DEG2RAD*i)*radiusBottom);
                    rlVertex3f(sinf(DEG2RAD*(i + 360/sides))*radiusBottom, 0, cosf(DEG2RAD*(i + 360/sides))*radiusBottom);
                }
            }

            // Draw Base -----------------------------------------------------------------------------------------
            for (int i = 0; i < 360; i += 360/sides)
            {
                rlVertex3f(0, 0, 0);
                rlVertex3f(sinf(DEG2RAD*(i + 360/sides))*radiusBottom, 0, cosf(DEG2RAD*(i + 360/sides))*radiusBottom);
                rlVertex3f(sinf(DEG2RAD*i)*radiusBottom, 0, cosf(DEG2RAD*i)*radiusBottom);
            }
        rlEnd();
    rlPopMatrix();
}

// Draw a wired cylinder
// NOTE: It could be also used for pyramid and cone
void DrawCylinderWires(Vector3 position, float radiusTop, float radiusBottom, float height, int sides, Color color)
{
    if (sides < 3) sides = 3;

    int numVertex = sides*8;
    rlCheckRenderBatchLimit(numVertex);

    rlPushMatrix();
        rlTranslatef(position.x, position.y, position.z);

        rlBegin(RL_LINES);
            rlColor4ub(color.r, color.g, color.b, color.a);

            for (int i = 0; i < 360; i += 360/sides)
            {
                rlVertex3f(sinf(DEG2RAD*i)*radiusBottom, 0, cosf(DEG2RAD*i)*radiusBottom);
                rlVertex3f(sinf(DEG2RAD*(i + 360/sides))*radiusBottom, 0, cosf(DEG2RAD*(i + 360/sides))*radiusBottom);

                rlVertex3f(sinf(DEG2RAD*(i + 360/sides))*radiusBottom, 0, cosf(DEG2RAD*(i + 360/sides))*radiusBottom);
                rlVertex3f(sinf(DEG2RAD*(i + 360/sides))*radiusTop, height, cosf(DEG2RAD*(i + 360/sides))*radiusTop);

                rlVertex3f(sinf(DEG2RAD*(i + 360/sides))*radiusTop, height, cosf(DEG2RAD*(i + 360/sides))*radiusTop);
                rlVertex3f(sinf(DEG2RAD*i)*radiusTop, height, cosf(DEG2RAD*i)*radiusTop);

                rlVertex3f(sinf(DEG2RAD*i)*radiusTop, height, cosf(DEG2RAD*i)*radiusTop);
                rlVertex3f(sinf(DEG2RAD*i)*radiusBottom, 0, cosf(DEG2RAD*i)*radiusBottom);
            }
        rlEnd();
    rlPopMatrix();
}

// Draw a plane
void DrawPlane(Vector3 centerPos, Vector2 size, Color color)
{
    rlCheckRenderBatchLimit(4);

    // NOTE: Plane is always created on XZ ground
    rlPushMatrix();
        rlTranslatef(centerPos.x, centerPos.y, centerPos.z);
        rlScalef(size.x, 1.0f, size.y);

        rlBegin(RL_QUADS);
            rlColor4ub(color.r, color.g, color.b, color.a);
            rlNormal3f(0.0f, 1.0f, 0.0f);

            rlVertex3f(-0.5f, 0.0f, -0.5f);
            rlVertex3f(-0.5f, 0.0f, 0.5f);
            rlVertex3f(0.5f, 0.0f, 0.5f);
            rlVertex3f(0.5f, 0.0f, -0.5f);
        rlEnd();
    rlPopMatrix();
}


// Draw a part of a texture (defined by a rectangle) with 'pro' parameters
// NOTE: origin is relative to destination rectangle size
void DrawTexturePro(Texture2D texture, Rectangle source, Rectangle dest, Vector2 origin, float rotation, Color tint)
{
    // Check if texture is valid
  //  if (texture.id > 0)
    {
        float width = (float)texture.width;
        float height = (float)texture.height;

        bool flipX = false;

        if (source.width < 0) { flipX = true; source.width *= -1; }
        if (source.height < 0) source.y -= source.height;

        Vector2 topLeft ;
        Vector2 topRight ;
        Vector2 bottomLeft;
        Vector2 bottomRight;

        // Only calculate rotation if needed
        if (rotation == 0.0f)
        {
            float x = dest.x - origin.x;
            float y = dest.y - origin.y;
            topLeft      = Vec2( x, y );
            topRight     = Vec2( x + dest.width, y );
            bottomLeft   = Vec2( x, y + dest.height );
            bottomRight  = Vec2( x + dest.width, y + dest.height );
        }
        else
        {
            float sinRotation = sinf(rotation*DEG2RAD);
            float cosRotation = cosf(rotation*DEG2RAD);
            float x = dest.x;
            float y = dest.y;
            float dx = -origin.x;
            float dy = -origin.y;

            topLeft.x = x + dx*cosRotation - dy*sinRotation;
            topLeft.y = y + dx*sinRotation + dy*cosRotation;

            topRight.x = x + (dx + dest.width)*cosRotation - dy*sinRotation;
            topRight.y = y + (dx + dest.width)*sinRotation + dy*cosRotation;

            bottomLeft.x = x + dx*cosRotation - (dy + dest.height)*sinRotation;
            bottomLeft.y = y + dx*sinRotation + (dy + dest.height)*cosRotation;

            bottomRight.x = x + (dx + dest.width)*cosRotation - (dy + dest.height)*sinRotation;
            bottomRight.y = y + (dx + dest.width)*sinRotation + (dy + dest.height)*cosRotation;
        }

        rlCheckRenderBatchLimit(4);     // Make sure there is enough free space on the batch buffer

        rlSetTexture(texture.id);
        rlBegin(RL_QUADS);

            rlColor4ub(tint.r, tint.g, tint.b, tint.a);
            rlNormal3f(0.0f, 0.0f, 1.0f);                          // Normal vector pointing towards viewer

            // Top-left corner for texture and quad
            if (flipX) rlTexCoord2f((source.x + source.width)/width, source.y/height);
            else rlTexCoord2f(source.x/width, source.y/height);
            rlVertex2f(topLeft.x, topLeft.y);

            // Bottom-left corner for texture and quad
            if (flipX) rlTexCoord2f((source.x + source.width)/width, (source.y + source.height)/height);
            else rlTexCoord2f(source.x/width, (source.y + source.height)/height);
            rlVertex2f(bottomLeft.x, bottomLeft.y);

            // Bottom-right corner for texture and quad
            if (flipX) rlTexCoord2f(source.x/width, (source.y + source.height)/height);
            else rlTexCoord2f((source.x + source.width)/width, (source.y + source.height)/height);
            rlVertex2f(bottomRight.x, bottomRight.y);

            // Top-right corner for texture and quad
            if (flipX) rlTexCoord2f(source.x/width, source.y/height);
            else rlTexCoord2f((source.x + source.width)/width, source.y/height);
            rlVertex2f(topRight.x, topRight.y);

        rlEnd();
        rlSetTexture(0);


    }
}

void DrawTextureSize(Texture2D texture, int posX, int posY,int Width, int Height ,Color tint)
{
    Rectangle source = { 0.0f, 0.0f, (float)texture.width, (float)texture.height };
    Rectangle dest = { posX, posY, Width, Height};
    Vector2 origin=Vec2(0,0);

    DrawTexturePro(texture, source, dest, origin, 0, tint);
}

void DrawTextureClip(Texture2D texture, int posX, int posY,int Width, int Height,Rectangle source ,Color tint)
{

    Rectangle dest = { posX, posY, Width, Height};
    Vector2 origin = Vec2(0,0);

    DrawTexturePro(texture, source, dest, origin, 0, tint);
}

// Draw a Texture2D with extended parameters
void DrawTextureEx(Texture2D texture, Vector2 position, float rotation, float scale, Color tint)
{
    Rectangle source = { 0.0f, 0.0f, (float)texture.width, (float)texture.height };
    Rectangle dest = { position.x, position.y, (float)texture.width*scale, (float)texture.height*scale };
    Vector2 origin = Vec2(0,0);

    DrawTexturePro(texture, source, dest, origin, rotation, tint);
}
// Draw a Texture2D with position defined as Vector2
void DrawTextureV(Texture2D texture, Vector2 position, Color tint)
{
    DrawTextureEx(texture, position, 0, 1.0f, tint);
}


void DrawTexture(Texture2D texture, int posX, int posY, Color tint)
{
    DrawTextureEx(texture, Vec2( (float)posX, (float)posY ), 0.0f, 1.0f, tint);
}


// Draw a part of a texture (defined by a rectangle)
void DrawTextureRec(Texture2D texture, Rectangle source, Vector2 position, Color tint)
{
    Rectangle dest = { position.x, position.y, fabsf(source.width), fabsf(source.height) };
    Vector2 origin =  Vec2(0,0);

    DrawTexturePro(texture, source, dest, origin, 0.0f, tint);
}

// Draw texture quad with tiling and offset parameters
// NOTE: Tiling and offset should be provided considering normalized texture values [0..1]
// i.e tiling = { 1.0f, 1.0f } refers to all texture, offset = { 0.5f, 0.5f } moves texture origin to center
void DrawTextureQuad(Texture2D texture, Vector2 tiling, Vector2 offset, Rectangle quad, Color tint)
{
    Rectangle source = { offset.x*texture.width, offset.y*texture.height, tiling.x*texture.width, tiling.y*texture.height };
    Vector2 origin =  Vec2(0,0);

    DrawTexturePro(texture, source, quad, origin, 0.0f, tint);
}


int GetRandomValue(int min, int max)
{
    if (min > max)
    {
        int tmp = max;
        max = min;
        min = tmp;
    }

    return (rand()%(abs(max - min) + 1) + min);
}

//*************************************************************************************************************************
#if defined(__linux__)
    #define MAX_FILEPATH_LENGTH     4096        // Maximum length for filepaths (Linux PATH_MAX default value)
#else
    #define MAX_FILEPATH_LENGTH      512        // Maximum length supported for filepaths
#endif

    #define MAX_TEXT_BUFFER_LENGTH              1024        // Size of internal static buffers used on some functions:
                                                            // TextFormat(), TextSubtext(), TextToUpper(), TextToLower(), TextToPascal(), TextSplit()
    #define MAX_TEXT_UNICODE_CHARS               512        // Maximum number of unicode codepoints: GetCodepoints()
    #define MAX_TEXTSPLIT_COUNT                  128        // Maximum number of substrings to split: TextSplit()




void *MemAlloc(int size)
{
    return SDL_malloc(size);
}

// Internal memory free
void MemFree(void *ptr)
{
    SDL_free(ptr);
}



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

// Encode text codepoint into utf8 text (memory must be freed!)
char *TextToUtf8(int *codepoints, int length)
{
    // We allocate enough memory fo fit all possible codepoints
    // NOTE: 5 bytes for every codepoint should be enough
    char *text = (char*)SCALLOC(length*5, 1);
    const char *utf8 = NULL;
    int size = 0;

    for (int i = 0, bytes = 0; i < length; i++)
    {
        utf8 = CodepointToUtf8(codepoints[i], &bytes);
        memcpy(text + size, utf8, bytes);
        size += bytes;
    }

    // Resize memory to text length + string NULL terminator
    void *ptr = SREALLOC(text, size + 1);

    if (ptr != NULL) text = (char *)ptr;

    return text;
}

// Encode codepoint into utf8 text (char array length returned as parameter)
 const char *CodepointToUtf8(int codepoint, int *byteLength)
{
    static char utf8[6] = { 0 };
    int length = 0;

    if (codepoint <= 0x7f)
    {
        utf8[0] = (char)codepoint;
        length = 1;
    }
    else if (codepoint <= 0x7ff)
    {
        utf8[0] = (char)(((codepoint >> 6) & 0x1f) | 0xc0);
        utf8[1] = (char)((codepoint & 0x3f) | 0x80);
        length = 2;
    }
    else if (codepoint <= 0xffff)
    {
        utf8[0] = (char)(((codepoint >> 12) & 0x0f) | 0xe0);
        utf8[1] = (char)(((codepoint >>  6) & 0x3f) | 0x80);
        utf8[2] = (char)((codepoint & 0x3f) | 0x80);
        length = 3;
    }
    else if (codepoint <= 0x10ffff)
    {
        utf8[0] = (char)(((codepoint >> 18) & 0x07) | 0xf0);
        utf8[1] = (char)(((codepoint >> 12) & 0x3f) | 0x80);
        utf8[2] = (char)(((codepoint >>  6) & 0x3f) | 0x80);
        utf8[3] = (char)((codepoint & 0x3f) | 0x80);
        length = 4;
    }

    *byteLength = length;

    return utf8;
}

// Get all codepoints in a string, codepoints count returned by parameters
int *GetCodepoints(const char *text, int *count)
{
    static int codepoints[MAX_TEXT_UNICODE_CHARS] = { 0 };
    memset(codepoints, 0, MAX_TEXT_UNICODE_CHARS*sizeof(int));

    int bytesProcessed = 0;
    int textLength = TextLength(text);
    int codepointsCount = 0;

    for (int i = 0; i < textLength; codepointsCount++)
    {
        codepoints[codepointsCount] = GetNextCodepoint(text + i, &bytesProcessed);
        i += bytesProcessed;
    }

    *count = codepointsCount;

    return codepoints;
}

// Returns total number of characters(codepoints) in a UTF8 encoded text, until '\0' is found
// NOTE: If an invalid UTF8 sequence is encountered a '?'(0x3f) codepoint is counted instead
int GetCodepointsCount(const char *text)
{
    unsigned int len = 0;
    char *ptr = (char *)&text[0];

    while (*ptr != '\0')
    {
        int next = 0;
        int letter = GetNextCodepoint(ptr, &next);

        if (letter == 0x3f) ptr += 1;
        else ptr += next;

        len++;
    }

    return len;
}


// Returns next codepoint in a UTF8 encoded text, scanning until '\0' is found
// When a invalid UTF8 byte is encountered we exit as soon as possible and a '?'(0x3f) codepoint is returned
// Total number of bytes processed are returned as a parameter
// NOTE: the standard says U+FFFD should be returned in case of errors
// but that character is not supported by the default font in raylib
// TODO: optimize this code for speed!!
int GetNextCodepoint(const char *text, int *bytesProcessed)
{
/*
    UTF8 specs from https://www.ietf.org/rfc/rfc3629.txt

    Char. number range  |        UTF-8 octet sequence
      (hexadecimal)    |              (binary)
    --------------------+---------------------------------------------
    0000 0000-0000 007F | 0xxxxxxx
    0000 0080-0000 07FF | 110xxxxx 10xxxxxx
    0000 0800-0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx
    0001 0000-0010 FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
*/
    // NOTE: on decode errors we return as soon as possible

    int code = 0x3f;   // Codepoint (defaults to '?')
    int octet = (unsigned char)(text[0]); // The first UTF8 octet
    *bytesProcessed = 1;

    if (octet <= 0x7f)
    {
        // Only one octet (ASCII range x00-7F)
        code = text[0];
    }
    else if ((octet & 0xe0) == 0xc0)
    {
        // Two octets
        // [0]xC2-DF    [1]UTF8-tail(x80-BF)
        unsigned char octet1 = text[1];

        if ((octet1 == '\0') || ((octet1 >> 6) != 2)) { *bytesProcessed = 2; return code; } // Unexpected sequence

        if ((octet >= 0xc2) && (octet <= 0xdf))
        {
            code = ((octet & 0x1f) << 6) | (octet1 & 0x3f);
            *bytesProcessed = 2;
        }
    }
    else if ((octet & 0xf0) == 0xe0)
    {
        // Three octets
        unsigned char octet1 = text[1];
        unsigned char octet2 = '\0';

        if ((octet1 == '\0') || ((octet1 >> 6) != 2)) { *bytesProcessed = 2; return code; } // Unexpected sequence

        octet2 = text[2];

        if ((octet2 == '\0') || ((octet2 >> 6) != 2)) { *bytesProcessed = 3; return code; } // Unexpected sequence

        /*
            [0]xE0    [1]xA0-BF       [2]UTF8-tail(x80-BF)
            [0]xE1-EC [1]UTF8-tail    [2]UTF8-tail(x80-BF)
            [0]xED    [1]x80-9F       [2]UTF8-tail(x80-BF)
            [0]xEE-EF [1]UTF8-tail    [2]UTF8-tail(x80-BF)
        */

        if (((octet == 0xe0) && !((octet1 >= 0xa0) && (octet1 <= 0xbf))) ||
            ((octet == 0xed) && !((octet1 >= 0x80) && (octet1 <= 0x9f)))) { *bytesProcessed = 2; return code; }

        if ((octet >= 0xe0) && (0 <= 0xef))
        {
            code = ((octet & 0xf) << 12) | ((octet1 & 0x3f) << 6) | (octet2 & 0x3f);
            *bytesProcessed = 3;
        }
    }
    else if ((octet & 0xf8) == 0xf0)
    {
        // Four octets
        if (octet > 0xf4) return code;

        unsigned char octet1 = text[1];
        unsigned char octet2 = '\0';
        unsigned char octet3 = '\0';

        if ((octet1 == '\0') || ((octet1 >> 6) != 2)) { *bytesProcessed = 2; return code; }  // Unexpected sequence

        octet2 = text[2];

        if ((octet2 == '\0') || ((octet2 >> 6) != 2)) { *bytesProcessed = 3; return code; }  // Unexpected sequence

        octet3 = text[3];

        if ((octet3 == '\0') || ((octet3 >> 6) != 2)) { *bytesProcessed = 4; return code; }  // Unexpected sequence

        /*
            [0]xF0       [1]x90-BF       [2]UTF8-tail  [3]UTF8-tail
            [0]xF1-F3    [1]UTF8-tail    [2]UTF8-tail  [3]UTF8-tail
            [0]xF4       [1]x80-8F       [2]UTF8-tail  [3]UTF8-tail
        */

        if (((octet == 0xf0) && !((octet1 >= 0x90) && (octet1 <= 0xbf))) ||
            ((octet == 0xf4) && !((octet1 >= 0x80) && (octet1 <= 0x8f)))) { *bytesProcessed = 2; return code; } // Unexpected sequence

        if (octet >= 0xf0)
        {
            code = ((octet & 0x7) << 18) | ((octet1 & 0x3f) << 12) | ((octet2 & 0x3f) << 6) | (octet3 & 0x3f);
            *bytesProcessed = 4;
        }
    }

    if (code > 0x10ffff) code = 0x3f;     // Codepoints after U+10ffff are invalid

    return code;
}







// Draw a piece of a circle
void DrawCircleSector(Vector2 center, float radius, float startAngle, float endAngle, int segments, Color color)
{
    if (radius <= 0.0f) radius = 0.1f;  // Avoid div by zero

    // Function expects (endAngle > startAngle)
    if (endAngle < startAngle)
    {
        // Swap values
        float tmp = startAngle;
        startAngle = endAngle;
        endAngle = tmp;
    }

    int minSegments = (int)ceilf((endAngle - startAngle)/90);

    if (segments < minSegments)
    {
        // Calculate the maximum angle between segments based on the error rate (usually 0.5f)
        float th = acosf(2*powf(1 - SMOOTH_CIRCLE_ERROR_RATE/radius, 2) - 1);
        segments = (int)((endAngle - startAngle)*ceilf(2*PI/th)/360);

        if (segments <= 0) segments = minSegments;
    }

    float stepLength = (endAngle - startAngle)/(float)segments;
    float angle = startAngle;


  rlCheckRenderBatchLimit(3*segments);

    rlBegin(RL_TRIANGLES);
        for (int i = 0; i < segments; i++)
        {
            rlColor4ub(color.r, color.g, color.b, color.a);

            rlVertex2f(center.x, center.y);
            rlVertex2f(center.x + sinf(DEG2RAD*angle)*radius, center.y + cosf(DEG2RAD*angle)*radius);
            rlVertex2f(center.x + sinf(DEG2RAD*(angle + stepLength))*radius, center.y + cosf(DEG2RAD*(angle + stepLength))*radius);

            angle += stepLength;
        }
    rlEnd();
    rlSetTexture(0);

}

// Draw a piece of a circle
void DrawCircleSectorTexture(Vector2 center, float radius, float startAngle, float endAngle, int segments, Color color)
{
    if (radius <= 0.0f) radius = 0.1f;  // Avoid div by zero

    // Function expects (endAngle > startAngle)
    if (endAngle < startAngle)
    {
        // Swap values
        float tmp = startAngle;
        startAngle = endAngle;
        endAngle = tmp;
    }

    int minSegments = (int)ceilf((endAngle - startAngle)/90);

    if (segments < minSegments)
    {
        // Calculate the maximum angle between segments based on the error rate (usually 0.5f)
        float th = acosf(2*powf(1 - SMOOTH_CIRCLE_ERROR_RATE/radius, 2) - 1);
        segments = (int)((endAngle - startAngle)*ceilf(2*PI/th)/360);

        if (segments <= 0) segments = minSegments;
    }

    float stepLength = (endAngle - startAngle)/(float)segments;
    float angle = startAngle;


    rlCheckRenderBatchLimit(4*segments/2);

    rlSetTexture(texShapes.id);

    rlBegin(RL_QUADS);
        // NOTE: Every QUAD actually represents two segments
        for (int i = 0; i < segments/2; i++)
        {
            rlColor4ub(color.r, color.g, color.b, color.a);

            rlTexCoord2f(texShapesRec.x/texShapes.width, texShapesRec.y/texShapes.height);
            rlVertex2f(center.x, center.y);

            rlTexCoord2f(texShapesRec.x/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
            rlVertex2f(center.x + sinf(DEG2RAD*angle)*radius, center.y + cosf(DEG2RAD*angle)*radius);

            rlTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
            rlVertex2f(center.x + sinf(DEG2RAD*(angle + stepLength))*radius, center.y + cosf(DEG2RAD*(angle + stepLength))*radius);

            rlTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, texShapesRec.y/texShapes.height);
            rlVertex2f(center.x + sinf(DEG2RAD*(angle + stepLength*2))*radius, center.y + cosf(DEG2RAD*(angle + stepLength*2))*radius);

            angle += (stepLength*2);
        }

        // NOTE: In case number of segments is odd, we add one last piece to the cake
        if (segments%2)
        {
            rlColor4ub(color.r, color.g, color.b, color.a);

            rlTexCoord2f(texShapesRec.x/texShapes.width, texShapesRec.y/texShapes.height);
            rlVertex2f(center.x, center.y);

            rlTexCoord2f(texShapesRec.x/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
            rlVertex2f(center.x + sinf(DEG2RAD*angle)*radius, center.y + cosf(DEG2RAD*angle)*radius);

            rlTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
            rlVertex2f(center.x + sinf(DEG2RAD*(angle + stepLength))*radius, center.y + cosf(DEG2RAD*(angle + stepLength))*radius);

            rlTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, texShapesRec.y/texShapes.height);
            rlVertex2f(center.x, center.y);
        }
    rlEnd();

    rlSetTexture(0);

}

void DrawCircleSectorLines(Vector2 center, float radius, float startAngle, float endAngle, int segments, Color color)
{
    if (radius <= 0.0f) radius = 0.1f;  // Avoid div by zero issue

    // Function expects (endAngle > startAngle)
    if (endAngle < startAngle)
    {
        // Swap values
        float tmp = startAngle;
        startAngle = endAngle;
        endAngle = tmp;
    }

    int minSegments = (int)ceilf((endAngle - startAngle)/90);

    if (segments < minSegments)
    {
        // Calculate the maximum angle between segments based on the error rate (usually 0.5f)
        float th = acosf(2*powf(1 - SMOOTH_CIRCLE_ERROR_RATE/radius, 2) - 1);
        segments = (int)((endAngle - startAngle)*ceilf(2*PI/th)/360);

        if (segments <= 0) segments = minSegments;
    }

    float stepLength = (endAngle - startAngle)/(float)segments;
    float angle = startAngle;

    // Hide the cap lines when the circle is full
    bool showCapLines = true;
    int limit = 2*(segments + 2);
    if ((int)(endAngle - startAngle)%360 == 0) { limit = 2*segments; showCapLines = false; }

    rlCheckRenderBatchLimit(limit);

    rlBegin(RL_LINES);
        if (showCapLines)
        {
            rlColor4ub(color.r, color.g, color.b, color.a);
            rlVertex2f(center.x, center.y);
            rlVertex2f(center.x + sinf(DEG2RAD*angle)*radius, center.y + cosf(DEG2RAD*angle)*radius);
        }

        for (int i = 0; i < segments; i++)
        {
            rlColor4ub(color.r, color.g, color.b, color.a);

            rlVertex2f(center.x + sinf(DEG2RAD*angle)*radius, center.y + cosf(DEG2RAD*angle)*radius);
            rlVertex2f(center.x + sinf(DEG2RAD*(angle + stepLength))*radius, center.y + cosf(DEG2RAD*(angle + stepLength))*radius);

            angle += stepLength;
        }

        if (showCapLines)
        {
            rlColor4ub(color.r, color.g, color.b, color.a);
            rlVertex2f(center.x, center.y);
            rlVertex2f(center.x + sinf(DEG2RAD*angle)*radius, center.y + cosf(DEG2RAD*angle)*radius);
        }
    rlEnd();
}


// Draw a gradient-filled circle
// NOTE: Gradient goes from center (color1) to border (color2)
void DrawCircleGradient(int centerX, int centerY, float radius, Color color1, Color color2)
{
    rlCheckRenderBatchLimit(3*36);

    rlBegin(RL_TRIANGLES);
        for (int i = 0; i < 360; i += 10)
        {
            rlColor4ub(color1.r, color1.g, color1.b, color1.a);
            rlVertex2f((float)centerX, (float)centerY);
            rlColor4ub(color2.r, color2.g, color2.b, color2.a);
            rlVertex2f((float)centerX + sinf(DEG2RAD*i)*radius, (float)centerY + cosf(DEG2RAD*i)*radius);
            rlColor4ub(color2.r, color2.g, color2.b, color2.a);
            rlVertex2f((float)centerX + sinf(DEG2RAD*(i + 10))*radius, (float)centerY + cosf(DEG2RAD*(i + 10))*radius);
        }
    rlEnd();
}

// Draw a color-filled circle (Vector version)
// NOTE: On OpenGL 3.3 and ES2 we use QUADS to avoid drawing order issues
void DrawCircleV(Vector2 center, float radius, Color color)
{
    DrawCircleSector(center, radius, 0, 360, 36, color);
}
// Draw a color-filled circle
void DrawCircle(int centerX, int centerY, float radius, Color color)
{
    DrawCircleV(Vec2( (float)centerX, (float)centerY ), radius, color);
}

// Draw circle outline
void DrawCircleLines(int centerX, int centerY, float radius, Color color)
{
    rlCheckRenderBatchLimit(2*36);

    rlBegin(RL_LINES);
        rlColor4ub(color.r, color.g, color.b, color.a);

        // NOTE: Circle outline is drawn pixel by pixel every degree (0 to 360)
        for (int i = 0; i < 360; i += 10)
        {
            rlVertex2f(centerX + sinf(DEG2RAD*i)*radius, centerY + cosf(DEG2RAD*i)*radius);
            rlVertex2f(centerX + sinf(DEG2RAD*(i + 10))*radius, centerY + cosf(DEG2RAD*(i + 10))*radius);
        }
    rlEnd();
}

// Draw ellipse
void DrawEllipse(int centerX, int centerY, float radiusH, float radiusV, Color color)
{
    rlCheckRenderBatchLimit(3*36);

    rlBegin(RL_TRIANGLES);
        for (int i = 0; i < 360; i += 10)
        {
            rlColor4ub(color.r, color.g, color.b, color.a);
            rlVertex2f((float)centerX, (float)centerY);
            rlVertex2f((float)centerX + sinf(DEG2RAD*i)*radiusH, (float)centerY + cosf(DEG2RAD*i)*radiusV);
            rlVertex2f((float)centerX + sinf(DEG2RAD*(i + 10))*radiusH, (float)centerY + cosf(DEG2RAD*(i + 10))*radiusV);
        }
    rlEnd();
}

// Draw ellipse outline
void DrawEllipseLines(int centerX, int centerY, float radiusH, float radiusV, Color color)
{
    rlCheckRenderBatchLimit(2*36);

    rlBegin(RL_LINES);
        for (int i = 0; i < 360; i += 10)
        {
            rlColor4ub(color.r, color.g, color.b, color.a);
            rlVertex2f(centerX + sinf(DEG2RAD*i)*radiusH, centerY + cosf(DEG2RAD*i)*radiusV);
            rlVertex2f(centerX + sinf(DEG2RAD*(i + 10))*radiusH, centerY + cosf(DEG2RAD*(i + 10))*radiusV);
        }
    rlEnd();
}
//***********************************************************************************************************************************************

Texture2D LoadTextureFromImage(Ogre::Image img);
Ogre::Image LoadImageFromFile( const char* texture_path);


Texture2D LoadTexture(const char* tp)
{
	Texture2D texture={0};
	Ogre::String texture_path=Ogre::String(tp);
	std::ifstream ifs(texture_path.c_str(), std::ios::binary|std::ios::in);
	if (ifs.is_open())
	{
	   SDL_Log("[IMAGE] Loaded ");
		Ogre::String tex_ext;
		Ogre::String::size_type index_of_extension = texture_path.find_last_of('.');
		if (index_of_extension != Ogre::String::npos)
		{
			tex_ext = texture_path.substr(index_of_extension+1);
			Ogre::DataStreamPtr data_stream(new Ogre::FileStreamDataStream(texture_path, &ifs, false));
			Ogre::Image img;
			img.load(data_stream, tex_ext);
			//img = img.flipAroundY();
			//img = img.flipAroundX();
			//Ogre::TextureManager::getSingleton().loadImage(texture_name,				Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, img, Ogre::TEX_TYPE_2D, 0, 1.0f);
        //img.getFormat()

        int format=PIXELFORMAT_UNCOMPRESSED_R8G8B8;
        if (img.getHasAlpha())
        {
          format=PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
        }
		texture.id=rlLoadTexture(img.getData(),img.getWidth(),img.getHeight(),format,1);
		texture.width =img.getWidth();
		texture.height=img.getHeight();
		texture.mipmaps=1;
		}
		ifs.close();
	} else{
	SDL_Log("[IMAGE] Fail load %s image ",texture_path.c_str());
	}
	return texture;
}

Texture2D LoadTextureFromImage(Ogre::Image img)
{
	    Texture2D texture={0};

	    int format  =PIXELFORMAT_UNCOMPRESSED_R8G8B8;


	    switch (img.getFormat())
	    {

	       case Ogre::PF_BYTE_LA:format=PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA;break;
	       case Ogre::PF_R5G6B5:format  =PIXELFORMAT_UNCOMPRESSED_R5G6B5;break;
	       case Ogre::PF_A1R5G5B5:format=PIXELFORMAT_UNCOMPRESSED_R5G5B5A1;break;
	       case Ogre::PF_A4R4G4B4:format=PIXELFORMAT_UNCOMPRESSED_R4G4B4A4;break;
           case Ogre::PF_R8G8B8:format  =PIXELFORMAT_UNCOMPRESSED_R8G8B8;break;
	       case Ogre::PF_R8G8B8A8:format=PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;break;



	       }

		texture.id=rlLoadTexture(img.getData(),img.getWidth(),img.getHeight(),format,1);
		texture.width =img.getWidth();
		texture.height=img.getHeight();
		texture.mipmaps=1;

	return texture;
}

TextFont defaultFont = { 0 };

// Get the default font, useful to be used with extended parameters
TextFont GetFontDefault()
{
    return defaultFont;

}

// Returns index position for a unicode character on spritefont
int GetGlyphIndex(TextFont font, int codepoint)
{
    #define GLYPH_NOTFOUND_CHAR_FALLBACK     63      // Character used if requested codepoint is not found: '?'

// Support charsets with any characters order
    int index = GLYPH_NOTFOUND_CHAR_FALLBACK;

    for (int i = 0; i < font.charsCount; i++)
    {
        if (font.chars[i].value == codepoint)
        {
            index = i;
            break;
        }
    }

    return index;

}



// Measure string size for Font
Vector2 MeasureTextEx(TextFont font, const char *text, float fontSize, float spacing)
{
    int len = TextLength(text);
    int tempLen = 0;                // Used to count longer text line num chars
    int lenCounter = 0;

    float textWidth = 0.0f;
    float tempTextWidth = 0.0f;     // Used to count longer text line width

    float textHeight = (float)font.baseSize;
    float scaleFactor = fontSize/(float)font.baseSize;

    int letter = 0;                 // Current character
    int index = 0;                  // Index position in sprite font

    for (int i = 0; i < len; i++)
    {
        lenCounter++;

        int next = 0;
        letter = GetNextCodepoint(&text[i], &next);
        index = GetGlyphIndex(font, letter);

        // NOTE: normally we exit the decoding sequence as soon as a bad byte is found (and return 0x3f)
        // but we need to draw all of the bad bytes using the '?' symbol so to not skip any we set next = 1
        if (letter == 0x3f) next = 1;
        i += next - 1;

        if (letter != '\n')
        {
            if (font.chars[index].advanceX != 0) textWidth += font.chars[index].advanceX;
            else textWidth += (font.recs[index].width + font.chars[index].offsetX);
        }
        else
        {
            if (tempTextWidth < textWidth) tempTextWidth = textWidth;
            lenCounter = 0;
            textWidth = 0;
            textHeight += ((float)font.baseSize*1.5f); // NOTE: Fixed line spacing of 1.5 lines
        }

        if (tempLen < lenCounter) tempLen = lenCounter;
    }

    if (tempTextWidth < textWidth) tempTextWidth = textWidth;

    Vector2 vec;
    vec.x = tempTextWidth*scaleFactor + (float)((tempLen - 1)*spacing); // Adds chars spacing to measure
    vec.y = textHeight*scaleFactor;

    return vec;
}
// Measure string width for default font
int MeasureText(const char *text, int fontSize)
{
    Vector2 vec;

    // Check if default font has been loaded
    if (GetFontDefault().texture.id != 0)
    {
        int defaultFontSize = 10;   // Default Font chars height in pixel
        if (fontSize < defaultFontSize) fontSize = defaultFontSize;
        int spacing = fontSize/defaultFontSize;

        vec = MeasureTextEx(GetFontDefault(), text, (float)fontSize, (float)spacing);
    }

    return (int)vec.x;
}


Ogre::Image LoadImageFromFile(  const char* tp)
{
	Ogre::Image img;
	Ogre::String texture_path=Ogre::String(tp);
	std::ifstream ifs(texture_path.c_str(), std::ios::binary|std::ios::in);
	if (ifs.is_open())
	{
		Ogre::String tex_ext;
		Ogre::String::size_type index_of_extension = texture_path.find_last_of('.');
		if (index_of_extension != Ogre::String::npos)
		{
			tex_ext = texture_path.substr(index_of_extension+1);
			Ogre::DataStreamPtr data_stream(new Ogre::FileStreamDataStream(texture_path, &ifs, false));

			img.load(data_stream, tex_ext);


		}
		ifs.close();
	}
	return img;
}



// Load raylib default font
void LoadFontDefault(void)
{
    #define BIT_CHECK(a,b) ((a) & (1u << (b)))

    // NOTE: Using UTF8 encoding table for Unicode U+0000..U+00FF Basic Latin + Latin-1 Supplement
    // Ref: http://www.utf8-chartable.de/unicode-utf8-table.pl

    defaultFont.charsCount = 224;   // Number of chars included in our default font
    defaultFont.charsPadding = 0;   // Characters padding

    // Default font is directly defined here (data generated from a sprite font image)
    // This way, we reconstruct Font without creating large global variables
    // This data is automatically allocated to Stack and automatically deallocated at the end of this function
    unsigned int defaultFontData[512] = {
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00200020, 0x0001b000, 0x00000000, 0x00000000, 0x8ef92520, 0x00020a00, 0x7dbe8000, 0x1f7df45f,
        0x4a2bf2a0, 0x0852091e, 0x41224000, 0x10041450, 0x2e292020, 0x08220812, 0x41222000, 0x10041450, 0x10f92020, 0x3efa084c, 0x7d22103c, 0x107df7de,
        0xe8a12020, 0x08220832, 0x05220800, 0x10450410, 0xa4a3f000, 0x08520832, 0x05220400, 0x10450410, 0xe2f92020, 0x0002085e, 0x7d3e0281, 0x107df41f,
        0x00200000, 0x8001b000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xc0000fbe, 0xfbf7e00f, 0x5fbf7e7d, 0x0050bee8, 0x440808a2, 0x0a142fe8, 0x50810285, 0x0050a048,
        0x49e428a2, 0x0a142828, 0x40810284, 0x0048a048, 0x10020fbe, 0x09f7ebaf, 0xd89f3e84, 0x0047a04f, 0x09e48822, 0x0a142aa1, 0x50810284, 0x0048a048,
        0x04082822, 0x0a142fa0, 0x50810285, 0x0050a248, 0x00008fbe, 0xfbf42021, 0x5f817e7d, 0x07d09ce8, 0x00008000, 0x00000fe0, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x000c0180,
        0xdfbf4282, 0x0bfbf7ef, 0x42850505, 0x004804bf, 0x50a142c6, 0x08401428, 0x42852505, 0x00a808a0, 0x50a146aa, 0x08401428, 0x42852505, 0x00081090,
        0x5fa14a92, 0x0843f7e8, 0x7e792505, 0x00082088, 0x40a15282, 0x08420128, 0x40852489, 0x00084084, 0x40a16282, 0x0842022a, 0x40852451, 0x00088082,
        0xc0bf4282, 0xf843f42f, 0x7e85fc21, 0x3e0900bf, 0x00000000, 0x00000004, 0x00000000, 0x000c0180, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x04000402, 0x41482000, 0x00000000, 0x00000800,
        0x04000404, 0x4100203c, 0x00000000, 0x00000800, 0xf7df7df0, 0x514bef85, 0xbefbefbe, 0x04513bef, 0x14414500, 0x494a2885, 0xa28a28aa, 0x04510820,
        0xf44145f0, 0x474a289d, 0xa28a28aa, 0x04510be0, 0x14414510, 0x494a2884, 0xa28a28aa, 0x02910a00, 0xf7df7df0, 0xd14a2f85, 0xbefbe8aa, 0x011f7be0,
        0x00000000, 0x00400804, 0x20080000, 0x00000000, 0x00000000, 0x00600f84, 0x20080000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0xac000000, 0x00000f01, 0x00000000, 0x00000000, 0x24000000, 0x00000f01, 0x00000000, 0x06000000, 0x24000000, 0x00000f01, 0x00000000, 0x09108000,
        0x24fa28a2, 0x00000f01, 0x00000000, 0x013e0000, 0x2242252a, 0x00000f52, 0x00000000, 0x038a8000, 0x2422222a, 0x00000f29, 0x00000000, 0x010a8000,
        0x2412252a, 0x00000f01, 0x00000000, 0x010a8000, 0x24fbe8be, 0x00000f01, 0x00000000, 0x0ebe8000, 0xac020000, 0x00000f01, 0x00000000, 0x00048000,
        0x0003e000, 0x00000f00, 0x00000000, 0x00008000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000038, 0x8443b80e, 0x00203a03,
        0x02bea080, 0xf0000020, 0xc452208a, 0x04202b02, 0xf8029122, 0x07f0003b, 0xe44b388e, 0x02203a02, 0x081e8a1c, 0x0411e92a, 0xf4420be0, 0x01248202,
        0xe8140414, 0x05d104ba, 0xe7c3b880, 0x00893a0a, 0x283c0e1c, 0x04500902, 0xc4400080, 0x00448002, 0xe8208422, 0x04500002, 0x80400000, 0x05200002,
        0x083e8e00, 0x04100002, 0x804003e0, 0x07000042, 0xf8008400, 0x07f00003, 0x80400000, 0x04000022, 0x00000000, 0x00000000, 0x80400000, 0x04000002,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00800702, 0x1848a0c2, 0x84010000, 0x02920921, 0x01042642, 0x00005121, 0x42023f7f, 0x00291002,
        0xefc01422, 0x7efdfbf7, 0xefdfa109, 0x03bbbbf7, 0x28440f12, 0x42850a14, 0x20408109, 0x01111010, 0x28440408, 0x42850a14, 0x2040817f, 0x01111010,
        0xefc78204, 0x7efdfbf7, 0xe7cf8109, 0x011111f3, 0x2850a932, 0x42850a14, 0x2040a109, 0x01111010, 0x2850b840, 0x42850a14, 0xefdfbf79, 0x03bbbbf7,
        0x001fa020, 0x00000000, 0x00001000, 0x00000000, 0x00002070, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x08022800, 0x00012283, 0x02430802, 0x01010001, 0x8404147c, 0x20000144, 0x80048404, 0x00823f08, 0xdfbf4284, 0x7e03f7ef, 0x142850a1, 0x0000210a,
        0x50a14684, 0x528a1428, 0x142850a1, 0x03efa17a, 0x50a14a9e, 0x52521428, 0x142850a1, 0x02081f4a, 0x50a15284, 0x4a221428, 0xf42850a1, 0x03efa14b,
        0x50a16284, 0x4a521428, 0x042850a1, 0x0228a17a, 0xdfbf427c, 0x7e8bf7ef, 0xf7efdfbf, 0x03efbd0b, 0x00000000, 0x04000000, 0x00000000, 0x00000008,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00200508, 0x00840400, 0x11458122, 0x00014210,
        0x00514294, 0x51420800, 0x20a22a94, 0x0050a508, 0x00200000, 0x00000000, 0x00050000, 0x08000000, 0xfefbefbe, 0xfbefbefb, 0xfbeb9114, 0x00fbefbe,
        0x20820820, 0x8a28a20a, 0x8a289114, 0x3e8a28a2, 0xfefbefbe, 0xfbefbe0b, 0x8a289114, 0x008a28a2, 0x228a28a2, 0x08208208, 0x8a289114, 0x088a28a2,
        0xfefbefbe, 0xfbefbefb, 0xfa2f9114, 0x00fbefbe, 0x00000000, 0x00000040, 0x00000000, 0x00000000, 0x00000000, 0x00000020, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00210100, 0x00000004, 0x00000000, 0x00000000, 0x14508200, 0x00001402, 0x00000000, 0x00000000,
        0x00000010, 0x00000020, 0x00000000, 0x00000000, 0xa28a28be, 0x00002228, 0x00000000, 0x00000000, 0xa28a28aa, 0x000022e8, 0x00000000, 0x00000000,
        0xa28a28aa, 0x000022a8, 0x00000000, 0x00000000, 0xa28a28aa, 0x000022e8, 0x00000000, 0x00000000, 0xbefbefbe, 0x00003e2f, 0x00000000, 0x00000000,
        0x00000004, 0x00002028, 0x00000000, 0x00000000, 0x80000000, 0x00003e0f, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 };

    int charsHeight = 10;
    int charsDivisor = 1;    // Every char is separated from the consecutive by a 1 pixel divisor, horizontally and vertically

    int charsWidth[224] = { 3, 1, 4, 6, 5, 7, 6, 2, 3, 3, 5, 5, 2, 4, 1, 7, 5, 2, 5, 5, 5, 5, 5, 5, 5, 5, 1, 1, 3, 4, 3, 6,
                            7, 6, 6, 6, 6, 6, 6, 6, 6, 3, 5, 6, 5, 7, 6, 6, 6, 6, 6, 6, 7, 6, 7, 7, 6, 6, 6, 2, 7, 2, 3, 5,
                            2, 5, 5, 5, 5, 5, 4, 5, 5, 1, 2, 5, 2, 5, 5, 5, 5, 5, 5, 5, 4, 5, 5, 5, 5, 5, 5, 3, 1, 3, 4, 4,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 1, 5, 5, 5, 7, 1, 5, 3, 7, 3, 5, 4, 1, 7, 4, 3, 5, 3, 3, 2, 5, 6, 1, 2, 2, 3, 5, 6, 6, 6, 6,
                            6, 6, 6, 6, 6, 6, 7, 6, 6, 6, 6, 6, 3, 3, 3, 3, 7, 6, 6, 6, 6, 6, 6, 5, 6, 6, 6, 6, 6, 6, 4, 6,
                            5, 5, 5, 5, 5, 5, 9, 5, 5, 5, 5, 5, 2, 2, 3, 3, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 3, 5 };

    // Re-construct image from defaultFontData and generate OpenGL texture
    //----------------------------------------------------------------------

     Ogre::uchar *data = (Ogre::uchar*)calloc(128*128, 2);  // 2 bytes per pixel (gray + alpha)

    /*
        data = calloc(128*128, 2),  // 2 bytes per pixel (gray + alpha)
        .width = 128,
        .height = 128,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA,

    };*/

    // Fill image.data with defaultFontData (convert from bit to pixel!)
    for (int i = 0, counter = 0; i < 128*128; i += 32)
    {
        for (int j = 31; j >= 0; j--)
        {
            if (BIT_CHECK(defaultFontData[counter], j))
            {
                // NOTE: We are unreferencing data as short, so,
                // we must consider data as little-endian order (alpha + gray)
                ((unsigned short *)data)[i + j] = 0xffff;
            }
            else ((unsigned short *)data)[i + j] = 0x00ff;
        }

        counter++;
    }

    Ogre::Image imFont =Ogre::Image(Ogre::PF_BYTE_LA,128,128,1,data);
    //imFont=LoadImageFromFile("/media/djoker/code/linux/cpp/ogre/samples/teste_sdl/font.png");

    //imFont=imFont.flipAroundX();
   // imFont=imFont.flipAroundY();


  //  imFont.save("font.png");


    defaultFont.texture = LoadTextureFromImage(imFont);

    // Reconstruct charSet using charsWidth[], charsHeight, charsDivisor, charsCount
    //------------------------------------------------------------------------------

    // Allocate space for our characters info data
    // NOTE: This memory should be freed at end! --> CloseWindow()
    defaultFont.chars = (CharInfo *)RL_MALLOC(defaultFont.charsCount*sizeof(CharInfo));
    defaultFont.recs  = (Rectangle *)RL_MALLOC(defaultFont.charsCount*sizeof(Rectangle));

    int currentLine = 0;
    int currentPosX = charsDivisor;
    int testPosX = charsDivisor;

    for (int i = 0; i < defaultFont.charsCount; i++)
    {
        defaultFont.chars[i].value = 32 + i;  // First char is 32

        defaultFont.recs[i].x = (float)currentPosX;
        defaultFont.recs[i].y = (float)(charsDivisor + currentLine*(charsHeight + charsDivisor));
        defaultFont.recs[i].width = (float)charsWidth[i];
        defaultFont.recs[i].height = (float)charsHeight;

        testPosX += (int)(defaultFont.recs[i].width + (float)charsDivisor);

        if (testPosX >= defaultFont.texture.width)
        {
            currentLine++;
            currentPosX = 2*charsDivisor + charsWidth[i];
            testPosX = currentPosX;

            defaultFont.recs[i].x = (float)charsDivisor;
            defaultFont.recs[i].y = (float)(charsDivisor + currentLine*(charsHeight + charsDivisor));
        }
        else currentPosX = testPosX;

        // NOTE: On default font character offsets and xAdvance are not required
        defaultFont.chars[i].offsetX = 0;
        defaultFont.chars[i].offsetY = 0;
        defaultFont.chars[i].advanceX = 0;

        // Fill character image data from fontClear data
      //  defaultFont.chars[i].image = ImageFromImage(imFont, defaultFont.recs[i]);
    }

   // UnloadImage(imFont);

    defaultFont.baseSize = (int)defaultFont.recs[0].height;

    TRACELOG(LOG_INFO, "FONT: Default font loaded successfully");
}

// Unload raylib default font
 void UnloadFontDefault(void)
{
//    for (int i = 0; i < defaultFont.charsCount; i++) UnloadImage(defaultFont.chars[i].image);
    rlUnloadTexture(defaultFont.texture.id);
    RL_FREE(defaultFont.chars);
    RL_FREE(defaultFont.recs);
}





// Draw one character (codepoint)
void DrawTextCodepoint(TextFont font, int codepoint, Vector2 position, float fontSize, Color tint)
{
    // Character index position in sprite font
    // NOTE: In case a codepoint is not available in the font, index returned points to '?'
    int index = GetGlyphIndex(font, codepoint);
    float scaleFactor = fontSize/font.baseSize;     // Character quad scaling factor

    // Character destination rectangle on screen
    // NOTE: We consider charsPadding on drawing
    Rectangle dstRec = { position.x + font.chars[index].offsetX*scaleFactor - (float)font.charsPadding*scaleFactor,
                      position.y + font.chars[index].offsetY*scaleFactor - (float)font.charsPadding*scaleFactor,
                      (font.recs[index].width + 2.0f*font.charsPadding)*scaleFactor,
                      (font.recs[index].height + 2.0f*font.charsPadding)*scaleFactor };

    // Character source rectangle from font texture atlas
    // NOTE: We consider chars padding when drawing, it could be required for outline/glow shader effects
    Rectangle srcRec = { font.recs[index].x - (float)font.charsPadding, font.recs[index].y - (float)font.charsPadding,
                         font.recs[index].width + 2.0f*font.charsPadding, font.recs[index].height + 2.0f*font.charsPadding };

    // Draw the character texture on the screen
    DrawTexturePro(font.texture, srcRec, dstRec, Vec2(0,0), 0.0f, tint);
}



// Draw text using Font
// NOTE: chars spacing is NOT proportional to fontSize
void DrawTextEx(TextFont font, const char *text, Vector2 position, float fontSize, float spacing, Color tint)
{
    int length = TextLength(text);      // Total length in bytes of the text, scanned by codepoints in loop

    int textOffsetY = 0;            // Offset between lines (on line break '\n')
    float textOffsetX = 0.0f;       // Offset X to next character to draw

    float scaleFactor = fontSize/font.baseSize;     // Character quad scaling factor

    for (int i = 0; i < length;)
    {
        // Get next codepoint from byte string and glyph index in font
        int codepointByteCount = 0;
        int codepoint = GetNextCodepoint(&text[i], &codepointByteCount);
        int index = GetGlyphIndex(font, codepoint);

        // NOTE: Normally we exit the decoding sequence as soon as a bad byte is found (and return 0x3f)
        // but we need to draw all of the bad bytes using the '?' symbol moving one byte
        if (codepoint == 0x3f) codepointByteCount = 1;

        if (codepoint == '\n')
        {
            // NOTE: Fixed line spacing of 1.5 line-height
            // TODO: Support custom line spacing defined by user
            textOffsetY += (int)((font.baseSize + font.baseSize/2)*scaleFactor);
            textOffsetX = 0.0f;
        }
        else
        {
            if ((codepoint != ' ') && (codepoint != '\t'))
            {
                DrawTextCodepoint(font, codepoint,Vec2( position.x + textOffsetX, position.y + textOffsetY ), fontSize, tint);
            }

            if (font.chars[index].advanceX == 0) textOffsetX += ((float)font.recs[index].width*scaleFactor + spacing);
            else textOffsetX += ((float)font.chars[index].advanceX*scaleFactor + spacing);
        }

        i += codepointByteCount;   // Move text bytes counter to next codepoint
    }
}
// Draw text (using default font)
// NOTE: fontSize work like in any drawing program but if fontSize is lower than font-base-size, then font-base-size is used
// NOTE: chars spacing is proportional to fontSize
void DrawText(const char *text, int posX, int posY, int fontSize, Color color)
{
    // Check if default font has been loaded
    if (GetFontDefault().texture.id != 0)
    {
        Vector2 position = Vec2((float)posX, (float)posY );

        int defaultFontSize = 10;   // Default Font chars height in pixel
        if (fontSize < defaultFontSize) fontSize = defaultFontSize;
        int spacing = fontSize/defaultFontSize;

        DrawTextEx(GetFontDefault(), text, position, (float)fontSize, (float)spacing, color);
    }
}

// Draw current FPS
// NOTE: Uses default font
void DrawStats(Ogre::RenderWindow* render,int posX, int posY)
{
    Color color = LIME; // good fps
    int lastFPS,avgFPS,bestFPS,worstFPS;

    Ogre::RenderWindow::FrameStats stats = render->getStatistics();


     lastFPS=(int)stats.lastFPS;
     avgFPS =(int)stats.avgFPS;
     bestFPS =(int)stats.bestFPS;
     worstFPS=(int)stats.worstFPS;


	int fps =(int)	 avgFPS;

    if (fps < 30 && fps >= 15) color = ORANGE;  // warning FPS
    else if (fps < 15) color = RED;    // bad FPS

    //DrawRectangle(posX-3,posY-24,190,70,YELLOW);
    DrawRectangle( posX-3,posY-24,190,70, Fade(SKYBLUE, 0.5f));
    DrawRectangleLines( posX-2,posY-24,190,70, BLUE);

    DrawText(TextFormat("Average FPS: %2i",avgFPS), posX, posY-21, 20, color);
    DrawText(TextFormat("Best    FPS: %2i",bestFPS)  , posX, posY, 20, color);
    DrawText(TextFormat("Worst   FPS: %2i",worstFPS), posX, posY+21, 20, color);
}



// Draw text using font inside rectangle limits with support for text selection
void DrawTextRecEx(TextFont font, const char *text, Rectangle rec, float fontSize, float spacing, bool wordWrap, Color tint, int selectStart, int selectLength, Color selectTint, Color selectBackTint)
{
    int length = TextLength(text);  // Total length in bytes of the text, scanned by codepoints in loop

    int textOffsetY = 0;            // Offset between lines (on line break '\n')
    float textOffsetX = 0.0f;       // Offset X to next character to draw

    float scaleFactor = fontSize/font.baseSize;     // Character quad scaling factor

    // Word/character wrapping mechanism variables
    enum { MEASURE_STATE = 0, DRAW_STATE = 1 };
    int state = wordWrap? MEASURE_STATE : DRAW_STATE;

    int startLine = -1;         // Index where to begin drawing (where a line begins)
    int endLine = -1;           // Index where to stop drawing (where a line ends)
    int lastk = -1;             // Holds last value of the character position

    for (int i = 0, k = 0; i < length; i++, k++)
    {
        // Get next codepoint from byte string and glyph index in font
        int codepointByteCount = 0;
        int codepoint = GetNextCodepoint(&text[i], &codepointByteCount);
        int index = GetGlyphIndex(font, codepoint);

        // NOTE: Normally we exit the decoding sequence as soon as a bad byte is found (and return 0x3f)
        // but we need to draw all of the bad bytes using the '?' symbol moving one byte
        if (codepoint == 0x3f) codepointByteCount = 1;
        i += (codepointByteCount - 1);

        int glyphWidth = 0;
        if (codepoint != '\n')
        {
            glyphWidth = (font.chars[index].advanceX == 0)?
                         (int)(font.recs[index].width*scaleFactor + spacing):
                         (int)(font.chars[index].advanceX*scaleFactor + spacing);
        }

        // NOTE: When wordWrap is ON we first measure how much of the text we can draw before going outside of the rec container
        // We store this info in startLine and endLine, then we change states, draw the text between those two variables
        // and change states again and again recursively until the end of the text (or until we get outside of the container).
        // When wordWrap is OFF we don't need the measure state so we go to the drawing state immediately
        // and begin drawing on the next line before we can get outside the container.
        if (state == MEASURE_STATE)
        {
            // TODO: There are multiple types of spaces in UNICODE, maybe it's a good idea to add support for more
            // Ref: http://jkorpela.fi/chars/spaces.html
            if ((codepoint == ' ') || (codepoint == '\t') || (codepoint == '\n')) endLine = i;

            if ((textOffsetX + glyphWidth + 1) >= rec.width)
            {
                endLine = (endLine < 1)? i : endLine;
                if (i == endLine) endLine -= codepointByteCount;
                if ((startLine + codepointByteCount) == endLine) endLine = (i - codepointByteCount);

                state = !state;
            }
            else if ((i + 1) == length)
            {
                endLine = i;

                state = !state;
            }
            else if (codepoint == '\n') state = !state;

            if (state == DRAW_STATE)
            {
                textOffsetX = 0;
                i = startLine;
                glyphWidth = 0;

                // Save character position when we switch states
                int tmp = lastk;
                lastk = k - 1;
                k = tmp;
            }
        }
        else
        {
            if (codepoint == '\n')
            {
                if (!wordWrap)
                {
                    textOffsetY += (int)((font.baseSize + font.baseSize/2)*scaleFactor);
                    textOffsetX = 0;
                }
            }
            else
            {
                if (!wordWrap && ((textOffsetX + glyphWidth + 1) >= rec.width))
                {
                    textOffsetY += (int)((font.baseSize + font.baseSize/2)*scaleFactor);
                    textOffsetX = 0;
                }

                // When text overflows rectangle height limit, just stop drawing
                if ((textOffsetY + (int)(font.baseSize*scaleFactor)) > rec.height) break;

                // Draw selection background
                bool isGlyphSelected = false;
                if ((selectStart >= 0) && (k >= selectStart) && (k < (selectStart + selectLength)))
                {
                    DrawRectangleRec((Rectangle){ rec.x + textOffsetX - 1, rec.y + textOffsetY, (float)glyphWidth, (float)font.baseSize*scaleFactor }, selectBackTint);
                    isGlyphSelected = true;
                }

                // Draw current character glyph
                if ((codepoint != ' ') && (codepoint != '\t'))
                {
                    DrawTextCodepoint(font, codepoint,Vec2( rec.x + textOffsetX, rec.y + textOffsetY ), fontSize, isGlyphSelected? selectTint : tint);
                }
            }

            if (wordWrap && (i == endLine))
            {
                textOffsetY += (int)((font.baseSize + font.baseSize/2)*scaleFactor);
                textOffsetX = 0;
                startLine = endLine;
                endLine = -1;
                glyphWidth = 0;
                selectStart += lastk - k;
                k = lastk;

                state = !state;
            }
        }

        textOffsetX += glyphWidth;
    }
}

// Draw text using font inside rectangle limits
void DrawTextRec(TextFont font, const char *text, Rectangle rec, float fontSize, float spacing, bool wordWrap, Color tint)
{
    DrawTextRecEx(font, text, rec, fontSize, spacing, wordWrap, tint, 0, 0, WHITE, WHITE);
}


//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------


// Read a line from memory
// REQUIRES: memcpy()
// NOTE: Returns the number of bytes read
 int GetLine(const char *origin, char *buffer, int maxLength)
{
    int count = 0;
    for (; count < maxLength; count++) if (origin[count] == '\n') break;
    memcpy(buffer, origin, count);
    return count;
}

// Load a BMFont file (AngelCode font file)
// REQUIRES: strstr(), sscanf(), strrchr(), memcpy()
 TextFont LoadBMFont(const char *fileName)
{
    #define MAX_BUFFER_SIZE     256

    TextFont font = { 0 };

    char buffer[MAX_BUFFER_SIZE] = { 0 };
    char *searchPoint = NULL;

    int fontSize = 0;
    int charsCount = 0;

    int imWidth = 0;
    int imHeight = 0;
    char imFileName[129];

    int base = 0;   // Useless data

    char *fileText = LoadFileText(fileName);

    if (fileText == NULL) return font;

    char *fileTextPtr = fileText;

    // NOTE: We skip first line, it contains no useful information
    int lineBytes = GetLine(fileTextPtr, buffer, MAX_BUFFER_SIZE);
    fileTextPtr += (lineBytes + 1);

    // Read line data
    lineBytes = GetLine(fileTextPtr, buffer, MAX_BUFFER_SIZE);
    searchPoint = strstr(buffer, "lineHeight");
    sscanf(searchPoint, "lineHeight=%i base=%i scaleW=%i scaleH=%i", &fontSize, &base, &imWidth, &imHeight);
    fileTextPtr += (lineBytes + 1);

    TRACELOGD("FONT: [%s] Loaded font info:", fileName);
    TRACELOGD("    > Base size: %i", fontSize);
    TRACELOGD("    > Texture scale: %ix%i", imWidth, imHeight);

    lineBytes = GetLine(fileTextPtr, buffer, MAX_BUFFER_SIZE);
    searchPoint = strstr(buffer, "file");
    sscanf(searchPoint, "file=\"%128[^\"]\"", imFileName);
    fileTextPtr += (lineBytes + 1);

    TRACELOGD("    > Texture filename: %s", imFileName);

    lineBytes = GetLine(fileTextPtr, buffer, MAX_BUFFER_SIZE);
    searchPoint = strstr(buffer, "count");
    sscanf(searchPoint, "count=%i", &charsCount);
    fileTextPtr += (lineBytes + 1);

    TRACELOGD("    > Chars count: %i", charsCount);

    // Compose correct path using route of .fnt file (fileName) and imFileName
    char *imPath = NULL;
    char *lastSlash = NULL;

    lastSlash = (char*)strrchr(fileName, '/');
    if (lastSlash == NULL) lastSlash = (char*)strrchr(fileName, '\\');

    if (lastSlash != NULL)
    {
        // NOTE: We need some extra space to avoid memory corruption on next allocations!
        imPath =(char*) RL_CALLOC(TextLength(fileName) - TextLength(lastSlash) + TextLength(imFileName) + 4, 1);
        memcpy(imPath, fileName, TextLength(fileName) - TextLength(lastSlash) + 1);
        memcpy(imPath + TextLength(fileName) - TextLength(lastSlash) + 1, imFileName, TextLength(imFileName));
    }
    else imPath = imFileName;

    TRACELOGD("    > Image loading path: %s", imPath);

   /* Image imFont = LoadImage(imPath);

    if (imFont.format == PIXELFORMAT_UNCOMPRESSED_GRAYSCALE)
    {
        // Convert image to GRAYSCALE + ALPHA, using the mask as the alpha channel
        Image imFontAlpha = {
            .data = calloc(imFont.width*imFont.height, 2),
            .width = imFont.width,
            .height = imFont.height,
            .format = PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA,
            //.mipmaps = 1
        };

        for (int p = 0, i = 0; p < (imFont.width*imFont.height*2); p += 2, i++)
        {
            ((unsigned char *)(imFontAlpha.data))[p] = 0xff;
            ((unsigned char *)(imFontAlpha.data))[p + 1] = ((unsigned char *)imFont.data)[i];
        }

        UnloadImage(imFont);
        imFont = imFontAlpha;
    }

    font.texture = LoadTextureFromImage(imFont);
    */

    if (lastSlash != NULL) RL_FREE(imPath);

    // Fill font characters info data
    font.baseSize = fontSize;
    font.charsCount = charsCount;
    font.charsPadding = 0;
    font.chars = (CharInfo *)RL_MALLOC(charsCount*sizeof(CharInfo));
    font.recs = (Rectangle *)RL_MALLOC(charsCount*sizeof(Rectangle));

    int charId, charX, charY, charWidth, charHeight, charOffsetX, charOffsetY, charAdvanceX;

    for (int i = 0; i < charsCount; i++)
    {
        lineBytes = GetLine(fileTextPtr, buffer, MAX_BUFFER_SIZE);
        sscanf(buffer, "char id=%i x=%i y=%i width=%i height=%i xoffset=%i yoffset=%i xadvance=%i",
                       &charId, &charX, &charY, &charWidth, &charHeight, &charOffsetX, &charOffsetY, &charAdvanceX);
        fileTextPtr += (lineBytes + 1);

        // Get character rectangle in the font atlas texture
        font.recs[i] = (Rectangle){ (float)charX, (float)charY, (float)charWidth, (float)charHeight };

        // Save data properly in sprite font
        font.chars[i].value = charId;
        font.chars[i].offsetX = charOffsetX;
        font.chars[i].offsetY = charOffsetY;
        font.chars[i].advanceX = charAdvanceX;

        // Fill character image data from imFont data
     //   font.chars[i].image = ImageFromImage(imFont, font.recs[i]);
    }

//    UnloadImage(imFont);
    RL_FREE(fileText);

    if (font.texture.id == 0)
    {
        UnloadFont(font);
        font = GetFontDefault();
        TRACELOG(LOG_WARNING, "FONT: [%s] Failed to load texture, reverted to default font", fileName);
    }
    else TRACELOG(LOG_INFO, "FONT: [%s] Font loaded successfully", fileName);

    return font;
}

void UnloadFontData(CharInfo *chars, int charsCount)
{
//    for (int i = 0; i < charsCount; i++) UnloadImage(chars[i].image);

    RL_FREE(chars);
}

void UnloadFont(TextFont font)
{
    // NOTE: Make sure font is not default font (fallback)
    if (font.texture.id != GetFontDefault().texture.id)
    {
        UnloadFontData(font.chars, font.charsCount);
        rlUnloadTexture(font.texture.id);
        RL_FREE(font.recs);

        TRACELOGD("FONT: Unloaded font data from RAM and VRAM");
    }
}




// Load an Image font file (XNA style)
TextFont LoadFontFromImage(Ogre::Image image, Color key, int firstChar)
{
/*
    #define MAX_GLYPHS_FROM_IMAGE   256     // Maximum number of glyphs supported on image scan

    #define COLOR_EQUAL(col1, col2) ((col1.r == col2.r)&&(col1.g == col2.g)&&(col1.b == col2.b)&&(col1.a == col2.a))

    int charSpacing = 0;
    int lineSpacing = 0;

    int x = 0;
    int y = 0;

    // We allocate a temporal arrays for chars data measures,
    // once we get the actual number of chars, we copy data to a sized arrays
    int tempCharValues[MAX_GLYPHS_FROM_IMAGE];
    Rectangle tempCharRecs[MAX_GLYPHS_FROM_IMAGE];

    Color *pixels = LoadImageColors(image);

    // Parse image data to get charSpacing and lineSpacing
    for (y = 0; y < image.height; y++)
    {
        for (x = 0; x < image.width; x++)
        {
            if (!COLOR_EQUAL(pixels[y*image.width + x], key)) break;
        }

        if (!COLOR_EQUAL(pixels[y*image.width + x], key)) break;
    }

    charSpacing = x;
    lineSpacing = y;

    int charHeight = 0;
    int j = 0;

    while (!COLOR_EQUAL(pixels[(lineSpacing + j)*image.width + charSpacing], key)) j++;

    charHeight = j;

    // Check array values to get characters: value, x, y, w, h
    int index = 0;
    int lineToRead = 0;
    int xPosToRead = charSpacing;

    // Parse image data to get rectangle sizes
    while ((lineSpacing + lineToRead*(charHeight + lineSpacing)) < image.height)
    {
        while ((xPosToRead < image.width) &&
              !COLOR_EQUAL((pixels[(lineSpacing + (charHeight+lineSpacing)*lineToRead)*image.width + xPosToRead]), key))
        {
            tempCharValues[index] = firstChar + index;

            tempCharRecs[index].x = (float)xPosToRead;
            tempCharRecs[index].y = (float)(lineSpacing + lineToRead*(charHeight + lineSpacing));
            tempCharRecs[index].height = (float)charHeight;

            int charWidth = 0;

            while (!COLOR_EQUAL(pixels[(lineSpacing + (charHeight+lineSpacing)*lineToRead)*image.width + xPosToRead + charWidth], key)) charWidth++;

            tempCharRecs[index].width = (float)charWidth;

            index++;

            xPosToRead += (charWidth + charSpacing);
        }

        lineToRead++;
        xPosToRead = charSpacing;
    }

    // NOTE: We need to remove key color borders from image to avoid weird
    // artifacts on texture scaling when using TEXTURE_FILTER_BILINEAR or TEXTURE_FILTER_TRILINEAR
    for (int i = 0; i < image.height*image.width; i++) if (COLOR_EQUAL(pixels[i], key)) pixels[i] = BLANK;

    // Create a new image with the processed color data (key color replaced by BLANK)
    Image fontClear = {
        .data = pixels,
        .width = image.width,
        .height = image.height,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,

    };

    // Create spritefont with all data parsed from image
    Font font = { 0 };

    font.texture = LoadTextureFromImage(fontClear); // Convert processed image to OpenGL texture
    font.charsCount = index;
    font.charsPadding = 0;

    // We got tempCharValues and tempCharsRecs populated with chars data
    // Now we move temp data to sized charValues and charRecs arrays
    font.chars = (CharInfo *)RL_MALLOC(font.charsCount*sizeof(CharInfo));
    font.recs = (Rectangle *)RL_MALLOC(font.charsCount*sizeof(Rectangle));

    for (int i = 0; i < font.charsCount; i++)
    {
        font.chars[i].value = tempCharValues[i];

        // Get character rectangle in the font atlas texture
        font.recs[i] = tempCharRecs[i];

        // NOTE: On image based fonts (XNA style), character offsets and xAdvance are not required (set to 0)
        font.chars[i].offsetX = 0;
        font.chars[i].offsetY = 0;
        font.chars[i].advanceX = 0;

        // Fill character image data from fontClear data
        font.chars[i].image = ImageFromImage(fontClear, tempCharRecs[i]);
    }

    UnloadImage(fontClear);     // Unload processed image once converted to texture

    font.baseSize = (int)font.recs[0].height;


    return font;
    */


    return defaultFont;
}


// Load Font from file into GPU memory (VRAM)
TextFont LoadFont(const char *fileName)
{
    // Default values for ttf font generation
    #define FONT_TTF_DEFAULT_SIZE           32      // TTF font generation default char size (char-height)
    #define FONT_TTF_DEFAULT_NUMCHARS       95      // TTF font generation default charset: 95 glyphs (ASCII 32..126)
    #define FONT_TTF_DEFAULT_FIRST_CHAR     32      // TTF font generation default first char for image sprite font (32-Space)
    #define FONT_TTF_DEFAULT_CHARS_PADDING   4      // TTF font generation default chars padding

    TextFont font = { 0 };

    if (IsFileExtension(fileName, ".fnt")) font = LoadBMFont(fileName);
    else
    {
        //Image image = LoadImage(fileName);
        	Ogre::Image image;
			image.load(fileName, "2d");

        //if (image.data != NULL) font = LoadFontFromImage(image, MAGENTA, FONT_TTF_DEFAULT_FIRST_CHAR);
        //UnloadImage(image);
    }

    if (font.texture.id == 0)
    {
        TRACELOG(LOG_WARNING, "FONT: [%s] Failed to load font texture -> Using default font", fileName);
        font = GetFontDefault();
    }
  //  else TextureFilter(font.texture.id, TEXTURE_FILTER_POINT);    // By default we set point filter (best performance)

    return font;
}

TextFont LoadFontEx(const char *fileName, int fontSize, int *fontChars, int charsCount)
{
    TextFont font = { 0 };

    // Loading file to memory
    unsigned int fileSize = 0;
    unsigned char *fileData = LoadFileData(fileName, &fileSize);



    if (fileData != NULL)
    {
        // Loading font from memory data
       // font = LoadFontFromMemory(GetFileExtension(fileName), fileData, fileSize, fontSize, fontChars, charsCount);

        RL_FREE(fileData);
    }
    else

    font = GetFontDefault();

    return font;
}


// Convert color data from RGB to HSV
// NOTE: Color data should be passed normalized
 Vector3 ConvertRGBtoHSV(Vector3 rgb)
{
    Vector3 hsv;
    float min = 0.0f;
    float max = 0.0f;
    float delta = 0.0f;

    min = (rgb.x < rgb.y)? rgb.x : rgb.y;
    min = (min < rgb.z)? min  : rgb.z;

    max = (rgb.x > rgb.y)? rgb.x : rgb.y;
    max = (max > rgb.z)? max  : rgb.z;

    hsv.z = max;            // Value
    delta = max - min;

    if (delta < 0.00001f)
    {
        hsv.y = 0.0f;
        hsv.x = 0.0f;       // Undefined, maybe NAN?
        return hsv;
    }

    if (max > 0.0f)
    {
        // NOTE: If max is 0, this divide would cause a crash
        hsv.y = (delta/max);    // Saturation
    }
    else
    {
        // NOTE: If max is 0, then r = g = b = 0, s = 0, h is undefined
        hsv.y = 0.0f;
        hsv.x = 0.0f;        // Undefined, maybe NAN?
        return hsv;
    }

    // NOTE: Comparing float values could not work properly
    if (rgb.x >= max) hsv.x = (rgb.y - rgb.z)/delta;    // Between yellow & magenta
    else
    {
        if (rgb.y >= max) hsv.x = 2.0f + (rgb.z - rgb.x)/delta;  // Between cyan & yellow
        else hsv.x = 4.0f + (rgb.x - rgb.y)/delta;      // Between magenta & cyan
    }

    hsv.x *= 60.0f;     // Convert to degrees

    if (hsv.x < 0.0f) hsv.x += 360.0f;

    return hsv;
}

// Convert color data from HSV to RGB
// NOTE: Color data should be passed normalized
 Vector3 ConvertHSVtoRGB(Vector3 hsv)
{
    Vector3 rgb ;
    float hh = 0.0f, p = 0.0f, q = 0.0f, t = 0.0f, ff = 0.0f;
    long i = 0;

    // NOTE: Comparing float values could not work properly
    if (hsv.y <= 0.0f)
    {
        rgb.x = hsv.z;
        rgb.y = hsv.z;
        rgb.z = hsv.z;
        return rgb;
    }

    hh = hsv.x;
    if (hh >= 360.0f) hh = 0.0f;
    hh /= 60.0f;

    i = (long)hh;
    ff = hh - i;
    p = hsv.z*(1.0f - hsv.y);
    q = hsv.z*(1.0f - (hsv.y*ff));
    t = hsv.z*(1.0f - (hsv.y*(1.0f - ff)));

    switch (i)
    {
        case 0:
        {
            rgb.x = hsv.z;
            rgb.y = t;
            rgb.z = p;
        } break;
        case 1:
        {
            rgb.x = q;
            rgb.y = hsv.z;
            rgb.z = p;
        } break;
        case 2:
        {
            rgb.x = p;
            rgb.y = hsv.z;
            rgb.z = t;
        } break;
        case 3:
        {
            rgb.x = p;
            rgb.y = q;
            rgb.z = hsv.z;
        } break;
        case 4:
        {
            rgb.x = t;
            rgb.y = p;
            rgb.z = hsv.z;
        } break;
        case 5:
        default:
        {
            rgb.x = hsv.z;
            rgb.y = p;
            rgb.z = q;
        } break;
    }

    return rgb;
}


 void SetClipboardText( const char* string)
{
    SDL_SetClipboardText(string);

}
 const char* GetClipboardText()
{
return SDL_GetClipboardText();

}


// Returns color with alpha applied, alpha goes from 0.0f to 1.0f
Color Fade(Color color, float alpha)
{
    if (alpha < 0.0f) alpha = 0.0f;
    else if (alpha > 1.0f) alpha = 1.0f;

    return (Color){color.r, color.g, color.b, (unsigned char)(255.0f*alpha)};
}

// Returns hexadecimal value for a Color
int ColorToInt(Color color)
{
    return (((int)color.r << 24) | ((int)color.g << 16) | ((int)color.b << 8) | (int)color.a);
}

// Returns color normalized as float [0..1]
Vector4 ColorNormalize(Color color)
{
    Vector4 result;

    result.x = (float)color.r/255.0f;
    result.y = (float)color.g/255.0f;
    result.z = (float)color.b/255.0f;
    result.w = (float)color.a/255.0f;

    return result;
}

// Returns color from normalized values [0..1]
Color ColorFromNormalized(Vector4 normalized)
{
    Color result;

    result.r = (unsigned char)(normalized.x*255.0f);
    result.g = (unsigned char)(normalized.y*255.0f);
    result.b = (unsigned char)(normalized.z*255.0f);
    result.a = (unsigned char)(normalized.w*255.0f);

    return result;
}

// Returns HSV values for a Color
// NOTE: Hue is returned as degrees [0..360]
Vector3 ColorToHSV(Color color)
{
    Vector3 hsv  ;
    Vector3 rgb = Vec3( (float)color.r/255.0f, (float)color.g/255.0f, (float)color.b/255.0f );
    float min, max, delta;

    min = rgb.x < rgb.y? rgb.x : rgb.y;
    min = min  < rgb.z? min  : rgb.z;

    max = rgb.x > rgb.y? rgb.x : rgb.y;
    max = max  > rgb.z? max  : rgb.z;

    hsv.z = max;            // Value
    delta = max - min;

    if (delta < 0.00001f)
    {
        hsv.y = 0.0f;
        hsv.x = 0.0f;       // Undefined, maybe NAN?
        return hsv;
    }

    if (max > 0.0f)
    {
        // NOTE: If max is 0, this divide would cause a crash
        hsv.y = (delta/max);    // Saturation
    }
    else
    {
        // NOTE: If max is 0, then r = g = b = 0, s = 0, h is undefined
        hsv.y = 0.0f;
        hsv.x = NAN;        // Undefined
        return hsv;
    }

    // NOTE: Comparing float values could not work properly
    if (rgb.x >= max) hsv.x = (rgb.y - rgb.z)/delta;    // Between yellow & magenta
    else
    {
        if (rgb.y >= max) hsv.x = 2.0f + (rgb.z - rgb.x)/delta;  // Between cyan & yellow
        else hsv.x = 4.0f + (rgb.x - rgb.y)/delta;      // Between magenta & cyan
    }

    hsv.x *= 60.0f;     // Convert to degrees

    if (hsv.x < 0.0f) hsv.x += 360.0f;

    return hsv;
}

// Returns a Color from HSV values
// Implementation reference: https://en.wikipedia.org/wiki/HSL_and_HSV#Alternative_HSV_conversion
// NOTE: Color->HSV->Color conversion will not yield exactly the same color due to rounding errors
// Hue is provided in degrees: [0..360]
// Saturation/Value are provided normalized: [0.0f..1.0f]
Color ColorFromHSV(float hue, float saturation, float value)
{
    Color color = { 0, 0, 0, 255 };

    // Red channel
    float k = fmodf((5.0f + hue/60.0f), 6);
    float t = 4.0f - k;
    k = (t < k)? t : k;
    k = (k < 1)? k : 1;
    k = (k > 0)? k : 0;
    color.r = (unsigned char)((value - value*saturation*k)*255.0f);

    // Green channel
    k = fmodf((3.0f + hue/60.0f), 6);
    t = 4.0f - k;
    k = (t < k)? t : k;
    k = (k < 1)? k : 1;
    k = (k > 0)? k : 0;
    color.g = (unsigned char)((value - value*saturation*k)*255.0f);

    // Blue channel
    k = fmodf((1.0f + hue/60.0f), 6);
    t = 4.0f - k;
    k = (t < k)? t : k;
    k = (k < 1)? k : 1;
    k = (k > 0)? k : 0;
    color.b = (unsigned char)((value - value*saturation*k)*255.0f);

    return color;
}

// Returns color with alpha applied, alpha goes from 0.0f to 1.0f
Color ColorAlpha(Color color, float alpha)
{
    if (alpha < 0.0f) alpha = 0.0f;
    else if (alpha > 1.0f) alpha = 1.0f;

    return (Color){color.r, color.g, color.b, (unsigned char)(255.0f*alpha)};
}

// Returns src alpha-blended into dst color with tint
Color ColorAlphaBlend(Color dst, Color src, Color tint)
{
    Color out = WHITE;

    // Apply color tint to source color
    src.r = (unsigned char)(((unsigned int)src.r*(unsigned int)tint.r) >> 8);
    src.g = (unsigned char)(((unsigned int)src.g*(unsigned int)tint.g) >> 8);
    src.b = (unsigned char)(((unsigned int)src.b*(unsigned int)tint.b) >> 8);
    src.a = (unsigned char)(((unsigned int)src.a*(unsigned int)tint.a) >> 8);

//#define COLORALPHABLEND_FLOAT
#define COLORALPHABLEND_INTEGERS
#if defined(COLORALPHABLEND_INTEGERS)
    if (src.a == 0) out = dst;
    else if (src.a == 255) out = src;
    else
    {
        unsigned int alpha = (unsigned int)src.a + 1;     // We are shifting by 8 (dividing by 256), so we need to take that excess into account
        out.a = (unsigned char)(((unsigned int)alpha*256 + (unsigned int)dst.a*(256 - alpha)) >> 8);

        if (out.a > 0)
        {
            out.r = (unsigned char)((((unsigned int)src.r*alpha*256 + (unsigned int)dst.r*(unsigned int)dst.a*(256 - alpha))/out.a) >> 8);
            out.g = (unsigned char)((((unsigned int)src.g*alpha*256 + (unsigned int)dst.g*(unsigned int)dst.a*(256 - alpha))/out.a) >> 8);
            out.b = (unsigned char)((((unsigned int)src.b*alpha*256 + (unsigned int)dst.b*(unsigned int)dst.a*(256 - alpha))/out.a) >> 8);
        }
    }
#endif
#if defined(COLORALPHABLEND_FLOAT)
    if (src.a == 0) out = dst;
    else if (src.a == 255) out = src;
    else
    {
        Vector4 fdst = ColorNormalize(dst);
        Vector4 fsrc = ColorNormalize(src);
        Vector4 ftint = ColorNormalize(tint);
        Vector4 fout = { 0 };

        fout.w = fsrc.w + fdst.w*(1.0f - fsrc.w);

        if (fout.w > 0.0f)
        {
            fout.x = (fsrc.x*fsrc.w + fdst.x*fdst.w*(1 - fsrc.w))/fout.w;
            fout.y = (fsrc.y*fsrc.w + fdst.y*fdst.w*(1 - fsrc.w))/fout.w;
            fout.z = (fsrc.z*fsrc.w + fdst.z*fdst.w*(1 - fsrc.w))/fout.w;
        }

        out = (Color){ (unsigned char)(fout.x*255.0f), (unsigned char)(fout.y*255.0f), (unsigned char)(fout.z*255.0f), (unsigned char)(fout.w*255.0f) };
    }
#endif

    return out;
}

// Returns a Color struct from hexadecimal value
Color GetColor(int hexValue)
{
    Color color;

    color.r = (unsigned char)(hexValue >> 24) & 0xFF;
    color.g = (unsigned char)(hexValue >> 16) & 0xFF;
    color.b = (unsigned char)(hexValue >> 8) & 0xFF;
    color.a = (unsigned char)hexValue & 0xFF;

    return color;
}

// Get color from a pixel from certain format
Color GetPixelColor(void *srcPtr, int format)
{
    Color col = { 0 };

    switch (format)
    {
        case PIXELFORMAT_UNCOMPRESSED_GRAYSCALE: col = (Color){ ((unsigned char *)srcPtr)[0], ((unsigned char *)srcPtr)[0], ((unsigned char *)srcPtr)[0], 255 }; break;
        case PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA: col = (Color){ ((unsigned char *)srcPtr)[0], ((unsigned char *)srcPtr)[0], ((unsigned char *)srcPtr)[0], ((unsigned char *)srcPtr)[1] }; break;
        case PIXELFORMAT_UNCOMPRESSED_R5G6B5:
        {
            col.r = (unsigned char)((((unsigned short *)srcPtr)[0] >> 11)*255/31);
            col.g = (unsigned char)(((((unsigned short *)srcPtr)[0] >> 5) & 0b0000000000111111)*255/63);
            col.b = (unsigned char)((((unsigned short *)srcPtr)[0] & 0b0000000000011111)*255/31);
            col.a = 255;

        } break;
        case PIXELFORMAT_UNCOMPRESSED_R5G5B5A1:
        {
            col.r = (unsigned char)((((unsigned short *)srcPtr)[0] >> 11)*255/31);
            col.g = (unsigned char)(((((unsigned short *)srcPtr)[0] >> 6) & 0b0000000000011111)*255/31);
            col.b = (unsigned char)((((unsigned short *)srcPtr)[0] & 0b0000000000011111)*255/31);
            col.a = (((unsigned short *)srcPtr)[0] & 0b0000000000000001)? 255 : 0;

        } break;
        case PIXELFORMAT_UNCOMPRESSED_R4G4B4A4:
        {
            col.r = (unsigned char)((((unsigned short *)srcPtr)[0] >> 12)*255/15);
            col.g = (unsigned char)(((((unsigned short *)srcPtr)[0] >> 8) & 0b0000000000001111)*255/15);
            col.b = (unsigned char)(((((unsigned short *)srcPtr)[0] >> 4) & 0b0000000000001111)*255/15);
            col.a = (unsigned char)((((unsigned short *)srcPtr)[0] & 0b0000000000001111)*255/15);

        } break;
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8: col = (Color){ ((unsigned char *)srcPtr)[0], ((unsigned char *)srcPtr)[1], ((unsigned char *)srcPtr)[2], ((unsigned char *)srcPtr)[3] }; break;
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8: col = (Color){ ((unsigned char *)srcPtr)[0], ((unsigned char *)srcPtr)[1], ((unsigned char *)srcPtr)[2], 255 }; break;
        // TODO: case PIXELFORMAT_UNCOMPRESSED_R32: break;
        // TODO: case PIXELFORMAT_UNCOMPRESSED_R32G32B32: break;
        // TODO: case PIXELFORMAT_UNCOMPRESSED_R32G32B32A32: break;
        default: break;
    }

    return col;
}
#define PIXELFORMAT_UNCOMPRESSED_R5G5B5A1_ALPHA_THRESHOLD  50    // Threshold over 255 to set alpha as 0
// Set pixel color formatted into destination pointer
void SetPixelColor(void *dstPtr, Color color, int format)
{
    switch (format)
    {
        case PIXELFORMAT_UNCOMPRESSED_GRAYSCALE:
        {
            // NOTE: Calculate grayscale equivalent color
            Vector3 coln = { (float)color.r/255.0f, (float)color.g/255.0f, (float)color.b/255.0f };
            unsigned char gray = (unsigned char)((coln.x*0.299f + coln.y*0.587f + coln.z*0.114f)*255.0f);

            ((unsigned char *)dstPtr)[0] = gray;

        } break;
        case PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA:
        {
            // NOTE: Calculate grayscale equivalent color
            Vector3 coln = { (float)color.r/255.0f, (float)color.g/255.0f, (float)color.b/255.0f };
            unsigned char gray = (unsigned char)((coln.x*0.299f + coln.y*0.587f + coln.z*0.114f)*255.0f);

            ((unsigned char *)dstPtr)[0] = gray;
            ((unsigned char *)dstPtr)[1] = color.a;

        } break;
        case PIXELFORMAT_UNCOMPRESSED_R5G6B5:
        {
            // NOTE: Calculate R5G6B5 equivalent color
            Vector3 coln = { (float)color.r/255.0f, (float)color.g/255.0f, (float)color.b/255.0f };

            unsigned char r = (unsigned char)(round(coln.x*31.0f));
            unsigned char g = (unsigned char)(round(coln.y*63.0f));
            unsigned char b = (unsigned char)(round(coln.z*31.0f));

            ((unsigned short *)dstPtr)[0] = (unsigned short)r << 11 | (unsigned short)g << 5 | (unsigned short)b;

        } break;
        case PIXELFORMAT_UNCOMPRESSED_R5G5B5A1:
        {
            // NOTE: Calculate R5G5B5A1 equivalent color
            Vector4 coln = { (float)color.r/255.0f, (float)color.g/255.0f, (float)color.b/255.0f, (float)color.a/255.0f };

            unsigned char r = (unsigned char)(round(coln.x*31.0f));
            unsigned char g = (unsigned char)(round(coln.y*31.0f));
            unsigned char b = (unsigned char)(round(coln.z*31.0f));
            unsigned char a = (coln.w > ((float)PIXELFORMAT_UNCOMPRESSED_R5G5B5A1_ALPHA_THRESHOLD/255.0f))? 1 : 0;;

            ((unsigned short *)dstPtr)[0] = (unsigned short)r << 11 | (unsigned short)g << 6 | (unsigned short)b << 1 | (unsigned short)a;

        } break;
        case PIXELFORMAT_UNCOMPRESSED_R4G4B4A4:
        {
            // NOTE: Calculate R5G5B5A1 equivalent color
            Vector4 coln = { (float)color.r/255.0f, (float)color.g/255.0f, (float)color.b/255.0f, (float)color.a/255.0f };

            unsigned char r = (unsigned char)(round(coln.x*15.0f));
            unsigned char g = (unsigned char)(round(coln.y*15.0f));
            unsigned char b = (unsigned char)(round(coln.z*15.0f));
            unsigned char a = (unsigned char)(round(coln.w*15.0f));

            ((unsigned short *)dstPtr)[0] = (unsigned short)r << 12 | (unsigned short)g << 8 | (unsigned short)b << 4 | (unsigned short)a;

        } break;
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8:
        {
            ((unsigned char *)dstPtr)[0] = color.r;
            ((unsigned char *)dstPtr)[1] = color.g;
            ((unsigned char *)dstPtr)[2] = color.b;

        } break;
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8:
        {
            ((unsigned char *)dstPtr)[0] = color.r;
            ((unsigned char *)dstPtr)[1] = color.g;
            ((unsigned char *)dstPtr)[2] = color.b;
            ((unsigned char *)dstPtr)[3] = color.a;

        } break;
        default: break;
    }
}

// Get pixel data size in bytes for certain format
// NOTE: Size can be requested for Image or Texture data
int GetPixelDataSize(int width, int height, int format)
{
    int dataSize = 0;       // Size in bytes
    int bpp = 0;            // Bits per pixel

    switch (format)
    {
        case PIXELFORMAT_UNCOMPRESSED_GRAYSCALE: bpp = 8; break;
        case PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA:
        case PIXELFORMAT_UNCOMPRESSED_R5G6B5:
        case PIXELFORMAT_UNCOMPRESSED_R5G5B5A1:
        case PIXELFORMAT_UNCOMPRESSED_R4G4B4A4: bpp = 16; break;
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8: bpp = 32; break;
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8: bpp = 24; break;
        case PIXELFORMAT_UNCOMPRESSED_R32: bpp = 32; break;
        case PIXELFORMAT_UNCOMPRESSED_R32G32B32: bpp = 32*3; break;
        case PIXELFORMAT_UNCOMPRESSED_R32G32B32A32: bpp = 32*4; break;
        case PIXELFORMAT_COMPRESSED_DXT1_RGB:
        case PIXELFORMAT_COMPRESSED_DXT1_RGBA:
        case PIXELFORMAT_COMPRESSED_ETC1_RGB:
        case PIXELFORMAT_COMPRESSED_ETC2_RGB:
        case PIXELFORMAT_COMPRESSED_PVRT_RGB:
        case PIXELFORMAT_COMPRESSED_PVRT_RGBA: bpp = 4; break;
        case PIXELFORMAT_COMPRESSED_DXT3_RGBA:
        case PIXELFORMAT_COMPRESSED_DXT5_RGBA:
        case PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA:
        case PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA: bpp = 8; break;
        case PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA: bpp = 2; break;
        default: break;
    }

    dataSize = width*height*bpp/8;  // Total data size in bytes

    // Most compressed formats works on 4x4 blocks,
    // if texture is smaller, minimum dataSize is 8 or 16
    if ((width < 4) && (height < 4))
    {
        if ((format >= PIXELFORMAT_COMPRESSED_DXT1_RGB) && (format < PIXELFORMAT_COMPRESSED_DXT3_RGBA)) dataSize = 8;
        else if ((format >= PIXELFORMAT_COMPRESSED_DXT3_RGBA) && (format < PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA)) dataSize = 16;
    }

    return dataSize;
}



//----------------------------------------------------------------------------------
// Module Functions Definition - Collision Detection functions
//----------------------------------------------------------------------------------

// Check if point is inside rectangle
bool CheckCollisionPointRec(Vector2 point, Rectangle rec)
{
    bool collision = false;

    if ((point.x >= rec.x) && (point.x <= (rec.x + rec.width)) && (point.y >= rec.y) && (point.y <= (rec.y + rec.height))) collision = true;

    return collision;
}

// Check if point is inside circle
bool CheckCollisionPointCircle(Vector2 point, Vector2 center, float radius)
{
    return CheckCollisionCircles(point, 0, center, radius);
}

// Check if point is inside a triangle defined by three points (p1, p2, p3)
bool CheckCollisionPointTriangle(Vector2 point, Vector2 p1, Vector2 p2, Vector2 p3)
{
    bool collision = false;

    float alpha = ((p2.y - p3.y)*(point.x - p3.x) + (p3.x - p2.x)*(point.y - p3.y)) /
                  ((p2.y - p3.y)*(p1.x - p3.x) + (p3.x - p2.x)*(p1.y - p3.y));

    float beta = ((p3.y - p1.y)*(point.x - p3.x) + (p1.x - p3.x)*(point.y - p3.y)) /
                 ((p2.y - p3.y)*(p1.x - p3.x) + (p3.x - p2.x)*(p1.y - p3.y));

    float gamma = 1.0f - alpha - beta;

    if ((alpha > 0) && (beta > 0) && (gamma > 0)) collision = true;

    return collision;
}

// Check collision between two rectangles
bool CheckCollisionRecs(Rectangle rec1, Rectangle rec2)
{
    bool collision = false;

    if ((rec1.x < (rec2.x + rec2.width) && (rec1.x + rec1.width) > rec2.x) &&
        (rec1.y < (rec2.y + rec2.height) && (rec1.y + rec1.height) > rec2.y)) collision = true;

    return collision;
}

// Check collision between two circles
bool CheckCollisionCircles(Vector2 center1, float radius1, Vector2 center2, float radius2)
{
    bool collision = false;

    float dx = center2.x - center1.x;      // X distance between centers
    float dy = center2.y - center1.y;      // Y distance between centers

    float distance = sqrtf(dx*dx + dy*dy); // Distance between centers

    if (distance <= (radius1 + radius2)) collision = true;

    return collision;
}

// Check collision between circle and rectangle
// NOTE: Reviewed version to take into account corner limit case
bool CheckCollisionCircleRec(Vector2 center, float radius, Rectangle rec)
{
    int recCenterX = (int)(rec.x + rec.width/2.0f);
    int recCenterY = (int)(rec.y + rec.height/2.0f);

    float dx = fabsf(center.x - (float)recCenterX);
    float dy = fabsf(center.y - (float)recCenterY);

    if (dx > (rec.width/2.0f + radius)) { return false; }
    if (dy > (rec.height/2.0f + radius)) { return false; }

    if (dx <= (rec.width/2.0f)) { return true; }
    if (dy <= (rec.height/2.0f)) { return true; }

    float cornerDistanceSq = (dx - rec.width/2.0f)*(dx - rec.width/2.0f) +
                             (dy - rec.height/2.0f)*(dy - rec.height/2.0f);

    return (cornerDistanceSq <= (radius*radius));
}

// Check the collision between two lines defined by two points each, returns collision point by reference
bool CheckCollisionLines(Vector2 startPos1, Vector2 endPos1, Vector2 startPos2, Vector2 endPos2, Vector2 *collisionPoint)
{
    const float div = (endPos2.y - startPos2.y)*(endPos1.x - startPos1.x) - (endPos2.x - startPos2.x)*(endPos1.y - startPos1.y);

    if (div == 0.0f) return false;      // WARNING: This check could not work due to float precision rounding issues...

    const float xi = ((startPos2.x - endPos2.x)*(startPos1.x*endPos1.y - startPos1.y*endPos1.x) - (startPos1.x - endPos1.x)*(startPos2.x*endPos2.y - startPos2.y*endPos2.x))/div;
    const float yi = ((startPos2.y - endPos2.y)*(startPos1.x*endPos1.y - startPos1.y*endPos1.x) - (startPos1.y - endPos1.y)*(startPos2.x*endPos2.y - startPos2.y*endPos2.x))/div;

    if (xi < fminf(startPos1.x, endPos1.x) || xi > fmaxf(startPos1.x, endPos1.x)) return false;
    if (xi < fminf(startPos2.x, endPos2.x) || xi > fmaxf(startPos2.x, endPos2.x)) return false;
    if (yi < fminf(startPos1.y, endPos1.y) || yi > fmaxf(startPos1.y, endPos1.y)) return false;
    if (yi < fminf(startPos2.y, endPos2.y) || yi > fmaxf(startPos2.y, endPos2.y)) return false;

    if (collisionPoint != 0)
    {
        collisionPoint->x = xi;
        collisionPoint->y = yi;
    }

    return true;
}

// Check if point belongs to line created between two points [p1] and [p2] with defined margin in pixels [threshold]
bool CheckCollisionPointLine(Vector2 point, Vector2 p1, Vector2 p2, int threshold)
{
    bool collision = false;
    float dxc = point.x - p1.x;
    float dyc = point.y - p1.y;
    float dxl = p2.x - p1.x;
    float dyl = p2.y - p1.y;
    float cross = dxc*dyl - dyc*dxl;

    if (fabsf(cross) < (threshold*fmaxf(fabsf(dxl), fabsf(dyl))))
    {
        if (fabsf(dxl) >= fabsf(dyl)) collision = (dxl > 0)? ((p1.x <= point.x) && (point.x <= p2.x)) : ((p2.x <= point.x) && (point.x <= p1.x));
        else collision = (dyl > 0)? ((p1.y <= point.y) && (point.y <= p2.y)) : ((p2.y <= point.y) && (point.y <= p1.y));
    }

    return collision;
}

// Get collision rectangle for two rectangles collision
Rectangle GetCollisionRec(Rectangle rec1, Rectangle rec2)
{
    Rectangle rec = { 0, 0, 0, 0 };

    if (CheckCollisionRecs(rec1, rec2))
    {
        float dxx = fabsf(rec1.x - rec2.x);
        float dyy = fabsf(rec1.y - rec2.y);

        if (rec1.x <= rec2.x)
        {
            if (rec1.y <= rec2.y)
            {
                rec.x = rec2.x;
                rec.y = rec2.y;
                rec.width = rec1.width - dxx;
                rec.height = rec1.height - dyy;
            }
            else
            {
                rec.x = rec2.x;
                rec.y = rec1.y;
                rec.width = rec1.width - dxx;
                rec.height = rec2.height - dyy;
            }
        }
        else
        {
            if (rec1.y <= rec2.y)
            {
                rec.x = rec1.x;
                rec.y = rec2.y;
                rec.width = rec2.width - dxx;
                rec.height = rec1.height - dyy;
            }
            else
            {
                rec.x = rec1.x;
                rec.y = rec1.y;
                rec.width = rec2.width - dxx;
                rec.height = rec2.height - dyy;
            }
        }

        if (rec1.width > rec2.width)
        {
            if (rec.width >= rec2.width) rec.width = rec2.width;
        }
        else
        {
            if (rec.width >= rec1.width) rec.width = rec1.width;
        }

        if (rec1.height > rec2.height)
        {
            if (rec.height >= rec2.height) rec.height = rec2.height;
        }
        else
        {
           if (rec.height >= rec1.height) rec.height = rec1.height;
        }
    }

    return rec;
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------

// Cubic easing in-out
// NOTE: Required for DrawLineBezier()
static float EaseCubicInOut(float t, float b, float c, float d)
{
    if ((t /= 0.5f*d) < 1) return 0.5f*c*t*t*t + b;

    t -= 2;

    return 0.5f*c*(t*t*t + 2.0f) + b;
}


#include <stdio.h>              // Required for: FILE, fopen(), fclose(), fprintf(), feof(), fscanf(), vsprintf() [GuiLoadStyle(), GuiLoadIcons()]
#include <stdlib.h>             // Required for: malloc(), calloc(), free() [GuiLoadStyle(), GuiLoadIcons()]
#include <string.h>             // Required for: strlen() [GuiTextBox(), GuiTextBoxMulti(), GuiValueBox()], memset(), memcpy()
#include <stdarg.h>             // Required for: va_list, va_start(), vfprintf(), va_end() [TextFormat()]
#include <math.h>               // Required for: roundf() [GuiColorPicker()]



#if defined(__cplusplus)
    #define RAYGUI_CLITERAL(name)      name
#else
    #define RAYGUI_CLITERAL(name)      (name)
#endif

#define RICON_SIZE               16       // Size of icons (squared)
#define RICON_MAX_ICONS         256       // Maximum number of icons
#define RICON_MAX_NAME_LENGTH    32       // Maximum length of icon name id

// Icons data is defined by bit array (every bit represents one pixel)
// Those arrays are stored as unsigned int data arrays, so every array
// element defines 32 pixels (bits) of information
// Number of elemens depend on RICON_SIZE (by default 16x16 pixels)
#define RICON_DATA_ELEMENTS   (RICON_SIZE*RICON_SIZE/32)

//----------------------------------------------------------------------------------
// Icons enumeration
//----------------------------------------------------------------------------------
typedef enum {
    RICON_NONE                     = 0,
    RICON_FOLDER_FILE_OPEN         = 1,
    RICON_FILE_SAVE_CLASSIC        = 2,
    RICON_FOLDER_OPEN              = 3,
    RICON_FOLDER_SAVE              = 4,
    RICON_FILE_OPEN                = 5,
    RICON_FILE_SAVE                = 6,
    RICON_FILE_EXPORT              = 7,
    RICON_FILE_NEW                 = 8,
    RICON_FILE_DELETE              = 9,
    RICON_FILETYPE_TEXT            = 10,
    RICON_FILETYPE_AUDIO           = 11,
    RICON_FILETYPE_IMAGE           = 12,
    RICON_FILETYPE_PLAY            = 13,
    RICON_FILETYPE_VIDEO           = 14,
    RICON_FILETYPE_INFO            = 15,
    RICON_FILE_COPY                = 16,
    RICON_FILE_CUT                 = 17,
    RICON_FILE_PASTE               = 18,
    RICON_CURSOR_HAND              = 19,
    RICON_CURSOR_POINTER           = 20,
    RICON_CURSOR_CLASSIC           = 21,
    RICON_PENCIL                   = 22,
    RICON_PENCIL_BIG               = 23,
    RICON_BRUSH_CLASSIC            = 24,
    RICON_BRUSH_PAINTER            = 25,
    RICON_WATER_DROP               = 26,
    RICON_COLOR_PICKER             = 27,
    RICON_RUBBER                   = 28,
    RICON_COLOR_BUCKET             = 29,
    RICON_TEXT_T                   = 30,
    RICON_TEXT_A                   = 31,
    RICON_SCALE                    = 32,
    RICON_RESIZE                   = 33,
    RICON_FILTER_POINT             = 34,
    RICON_FILTER_BILINEAR          = 35,
    RICON_CROP                     = 36,
    RICON_CROP_ALPHA               = 37,
    RICON_SQUARE_TOGGLE            = 38,
    RICON_SYMMETRY                 = 39,
    RICON_SYMMETRY_HORIZONTAL      = 40,
    RICON_SYMMETRY_VERTICAL        = 41,
    RICON_LENS                     = 42,
    RICON_LENS_BIG                 = 43,
    RICON_EYE_ON                   = 44,
    RICON_EYE_OFF                  = 45,
    RICON_FILTER_TOP               = 46,
    RICON_FILTER                   = 47,
    RICON_TARGET_POINT             = 48,
    RICON_TARGET_SMALL             = 49,
    RICON_TARGET_BIG               = 50,
    RICON_TARGET_MOVE              = 51,
    RICON_CURSOR_MOVE              = 52,
    RICON_CURSOR_SCALE             = 53,
    RICON_CURSOR_SCALE_RIGHT       = 54,
    RICON_CURSOR_SCALE_LEFT        = 55,
    RICON_UNDO                     = 56,
    RICON_REDO                     = 57,
    RICON_REREDO                   = 58,
    RICON_MUTATE                   = 59,
    RICON_ROTATE                   = 60,
    RICON_REPEAT                   = 61,
    RICON_SHUFFLE                  = 62,
    RICON_EMPTYBOX                 = 63,
    RICON_TARGET                   = 64,
    RICON_TARGET_SMALL_FILL        = 65,
    RICON_TARGET_BIG_FILL          = 66,
    RICON_TARGET_MOVE_FILL         = 67,
    RICON_CURSOR_MOVE_FILL         = 68,
    RICON_CURSOR_SCALE_FILL        = 69,
    RICON_CURSOR_SCALE_RIGHT_FILL  = 70,
    RICON_CURSOR_SCALE_LEFT_FILL   = 71,
    RICON_UNDO_FILL                = 72,
    RICON_REDO_FILL                = 73,
    RICON_REREDO_FILL              = 74,
    RICON_MUTATE_FILL              = 75,
    RICON_ROTATE_FILL              = 76,
    RICON_REPEAT_FILL              = 77,
    RICON_SHUFFLE_FILL             = 78,
    RICON_EMPTYBOX_SMALL           = 79,
    RICON_BOX                      = 80,
    RICON_BOX_TOP                  = 81,
    RICON_BOX_TOP_RIGHT            = 82,
    RICON_BOX_RIGHT                = 83,
    RICON_BOX_BOTTOM_RIGHT         = 84,
    RICON_BOX_BOTTOM               = 85,
    RICON_BOX_BOTTOM_LEFT          = 86,
    RICON_BOX_LEFT                 = 87,
    RICON_BOX_TOP_LEFT             = 88,
    RICON_BOX_CENTER               = 89,
    RICON_BOX_CIRCLE_MASK          = 90,
    RICON_POT                      = 91,
    RICON_ALPHA_MULTIPLY           = 92,
    RICON_ALPHA_CLEAR              = 93,
    RICON_DITHERING                = 94,
    RICON_MIPMAPS                  = 95,
    RICON_BOX_GRID                 = 96,
    RICON_GRID                     = 97,
    RICON_BOX_CORNERS_SMALL        = 98,
    RICON_BOX_CORNERS_BIG          = 99,
    RICON_FOUR_BOXES               = 100,
    RICON_GRID_FILL                = 101,
    RICON_BOX_MULTISIZE            = 102,
    RICON_ZOOM_SMALL               = 103,
    RICON_ZOOM_MEDIUM              = 104,
    RICON_ZOOM_BIG                 = 105,
    RICON_ZOOM_ALL                 = 106,
    RICON_ZOOM_CENTER              = 107,
    RICON_BOX_DOTS_SMALL           = 108,
    RICON_BOX_DOTS_BIG             = 109,
    RICON_BOX_CONCENTRIC           = 110,
    RICON_BOX_GRID_BIG             = 111,
    RICON_OK_TICK                  = 112,
    RICON_CROSS                    = 113,
    RICON_ARROW_LEFT               = 114,
    RICON_ARROW_RIGHT              = 115,
    RICON_ARROW_DOWN               = 116,
    RICON_ARROW_UP                 = 117,
    RICON_ARROW_LEFT_FILL          = 118,
    RICON_ARROW_RIGHT_FILL         = 119,
    RICON_ARROW_DOWN_FILL          = 120,
    RICON_ARROW_UP_FILL            = 121,
    RICON_AUDIO                    = 122,
    RICON_FX                       = 123,
    RICON_WAVE                     = 124,
    RICON_WAVE_SINUS               = 125,
    RICON_WAVE_SQUARE              = 126,
    RICON_WAVE_TRIANGULAR          = 127,
    RICON_CROSS_SMALL              = 128,
    RICON_PLAYER_PREVIOUS          = 129,
    RICON_PLAYER_PLAY_BACK         = 130,
    RICON_PLAYER_PLAY              = 131,
    RICON_PLAYER_PAUSE             = 132,
    RICON_PLAYER_STOP              = 133,
    RICON_PLAYER_NEXT              = 134,
    RICON_PLAYER_RECORD            = 135,
    RICON_MAGNET                   = 136,
    RICON_LOCK_CLOSE               = 137,
    RICON_LOCK_OPEN                = 138,
    RICON_CLOCK                    = 139,
    RICON_TOOLS                    = 140,
    RICON_GEAR                     = 141,
    RICON_GEAR_BIG                 = 142,
    RICON_BIN                      = 143,
    RICON_HAND_POINTER             = 144,
    RICON_LASER                    = 145,
    RICON_COIN                     = 146,
    RICON_EXPLOSION                = 147,
    RICON_1UP                      = 148,
    RICON_PLAYER                   = 149,
    RICON_PLAYER_JUMP              = 150,
    RICON_KEY                      = 151,
    RICON_DEMON                    = 152,
    RICON_TEXT_POPUP               = 153,
    RICON_GEAR_EX                  = 154,
    RICON_CRACK                    = 155,
    RICON_CRACK_POINTS             = 156,
    RICON_STAR                     = 157,
    RICON_DOOR                     = 158,
    RICON_EXIT                     = 159,
    RICON_MODE_2D                  = 160,
    RICON_MODE_3D                  = 161,
    RICON_CUBE                     = 162,
    RICON_CUBE_FACE_TOP            = 163,
    RICON_CUBE_FACE_LEFT           = 164,
    RICON_CUBE_FACE_FRONT          = 165,
    RICON_CUBE_FACE_BOTTOM         = 166,
    RICON_CUBE_FACE_RIGHT          = 167,
    RICON_CUBE_FACE_BACK           = 168,
    RICON_CAMERA                   = 169,
    RICON_SPECIAL                  = 170,
    RICON_LINK_NET                 = 171,
    RICON_LINK_BOXES               = 172,
    RICON_LINK_MULTI               = 173,
    RICON_LINK                     = 174,
    RICON_LINK_BROKE               = 175,
    RICON_TEXT_NOTES               = 176,
    RICON_NOTEBOOK                 = 177,
    RICON_SUITCASE                 = 178,
    RICON_SUITCASE_ZIP             = 179,
    RICON_MAILBOX                  = 180,
    RICON_MONITOR                  = 181,
    RICON_PRINTER                  = 182,
    RICON_PHOTO_CAMERA             = 183,
    RICON_PHOTO_CAMERA_FLASH       = 184,
    RICON_HOUSE                    = 185,
    RICON_HEART                    = 186,
    RICON_CORNER                   = 187,
    RICON_VERTICAL_BARS            = 188,
    RICON_VERTICAL_BARS_FILL       = 189,
    RICON_LIFE_BARS                = 190,
    RICON_INFO                     = 191,
    RICON_CROSSLINE                = 192,
    RICON_HELP                     = 193,
    RICON_FILETYPE_ALPHA           = 194,
    RICON_FILETYPE_HOME            = 195,
    RICON_LAYERS_VISIBLE           = 196,
    RICON_LAYERS                   = 197,
    RICON_WINDOW                   = 198,
    RICON_HIDPI                    = 199,
    RICON_200                      = 200,
    RICON_201                      = 201,
    RICON_202                      = 202,
    RICON_203                      = 203,
    RICON_204                      = 204,
    RICON_205                      = 205,
    RICON_206                      = 206,
    RICON_207                      = 207,
    RICON_208                      = 208,
    RICON_209                      = 209,
    RICON_210                      = 210,
    RICON_211                      = 211,
    RICON_212                      = 212,
    RICON_213                      = 213,
    RICON_214                      = 214,
    RICON_215                      = 215,
    RICON_216                      = 216,
    RICON_217                      = 217,
    RICON_218                      = 218,
    RICON_219                      = 219,
    RICON_220                      = 220,
    RICON_221                      = 221,
    RICON_222                      = 222,
    RICON_223                      = 223,
    RICON_224                      = 224,
    RICON_225                      = 225,
    RICON_226                      = 226,
    RICON_227                      = 227,
    RICON_228                      = 228,
    RICON_229                      = 229,
    RICON_230                      = 230,
    RICON_231                      = 231,
    RICON_232                      = 232,
    RICON_233                      = 233,
    RICON_234                      = 234,
    RICON_235                      = 235,
    RICON_236                      = 236,
    RICON_237                      = 237,
    RICON_238                      = 238,
    RICON_239                      = 239,
    RICON_240                      = 240,
    RICON_241                      = 241,
    RICON_242                      = 242,
    RICON_243                      = 243,
    RICON_244                      = 244,
    RICON_245                      = 245,
    RICON_246                      = 246,
    RICON_247                      = 247,
    RICON_248                      = 248,
    RICON_249                      = 249,
    RICON_250                      = 250,
    RICON_251                      = 251,
    RICON_252                      = 252,
    RICON_253                      = 253,
    RICON_254                      = 254,
    RICON_255                      = 255,
} guiIconName;

//----------------------------------------------------------------------------------
// Icons data for all gui possible icons (allocated on data segment by default)
//
// NOTE 1: Every icon is codified in binary form, using 1 bit per pixel, so,
// every 16x16 icon requires 8 integers (16*16/32) to be stored
//
// NOTE 2: A new icon set could be loaded over this array using GuiLoadIcons(),
// but loaded icons set must be same RICON_SIZE and no more than RICON_MAX_ICONS
//
// guiIcons size is by default: 256*(16*16/32) = 2048*4 = 8192 bytes = 8 KB
//----------------------------------------------------------------------------------
static unsigned int guiIcons[RICON_MAX_ICONS*RICON_DATA_ELEMENTS] = {
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_NONE
    0x3ff80000, 0x2f082008, 0x2042207e, 0x40027fc2, 0x40024002, 0x40024002, 0x40024002, 0x00007ffe,     // RICON_FOLDER_FILE_OPEN
    0x3ffe0000, 0x44226422, 0x400247e2, 0x5ffa4002, 0x57ea500a, 0x500a500a, 0x40025ffa, 0x00007ffe,     // RICON_FILE_SAVE_CLASSIC
    0x00000000, 0x0042007e, 0x40027fc2, 0x40024002, 0x41024002, 0x44424282, 0x793e4102, 0x00000100,     // RICON_FOLDER_OPEN
    0x00000000, 0x0042007e, 0x40027fc2, 0x40024002, 0x41024102, 0x44424102, 0x793e4282, 0x00000000,     // RICON_FOLDER_SAVE
    0x3ff00000, 0x201c2010, 0x20042004, 0x21042004, 0x24442284, 0x21042104, 0x20042104, 0x00003ffc,     // RICON_FILE_OPEN
    0x3ff00000, 0x201c2010, 0x20042004, 0x21042004, 0x21042104, 0x22842444, 0x20042104, 0x00003ffc,     // RICON_FILE_SAVE
    0x3ff00000, 0x201c2010, 0x00042004, 0x20041004, 0x20844784, 0x00841384, 0x20042784, 0x00003ffc,     // RICON_FILE_EXPORT
    0x3ff00000, 0x201c2010, 0x20042004, 0x20042004, 0x22042204, 0x22042f84, 0x20042204, 0x00003ffc,     // RICON_FILE_NEW
    0x3ff00000, 0x201c2010, 0x20042004, 0x20042004, 0x25042884, 0x25042204, 0x20042884, 0x00003ffc,     // RICON_FILE_DELETE
    0x3ff00000, 0x201c2010, 0x20042004, 0x20042ff4, 0x20042ff4, 0x20042ff4, 0x20042004, 0x00003ffc,     // RICON_FILETYPE_TEXT
    0x3ff00000, 0x201c2010, 0x27042004, 0x244424c4, 0x26442444, 0x20642664, 0x20042004, 0x00003ffc,     // RICON_FILETYPE_AUDIO
    0x3ff00000, 0x201c2010, 0x26042604, 0x20042004, 0x35442884, 0x2414222c, 0x20042004, 0x00003ffc,     // RICON_FILETYPE_IMAGE
    0x3ff00000, 0x201c2010, 0x20c42004, 0x22442144, 0x22442444, 0x20c42144, 0x20042004, 0x00003ffc,     // RICON_FILETYPE_PLAY
    0x3ff00000, 0x3ffc2ff0, 0x3f3c2ff4, 0x3dbc2eb4, 0x3dbc2bb4, 0x3f3c2eb4, 0x3ffc2ff4, 0x00002ff4,     // RICON_FILETYPE_VIDEO
    0x3ff00000, 0x201c2010, 0x21842184, 0x21842004, 0x21842184, 0x21842184, 0x20042184, 0x00003ffc,     // RICON_FILETYPE_INFO
    0x0ff00000, 0x381c0810, 0x28042804, 0x28042804, 0x28042804, 0x28042804, 0x20102ffc, 0x00003ff0,     // RICON_FILE_COPY
    0x00000000, 0x701c0000, 0x079c1e14, 0x55a000f0, 0x079c00f0, 0x701c1e14, 0x00000000, 0x00000000,     // RICON_FILE_CUT
    0x01c00000, 0x13e41bec, 0x3f841004, 0x204420c4, 0x20442044, 0x20442044, 0x207c2044, 0x00003fc0,     // RICON_FILE_PASTE
    0x00000000, 0x3aa00fe0, 0x2abc2aa0, 0x2aa42aa4, 0x20042aa4, 0x20042004, 0x3ffc2004, 0x00000000,     // RICON_CURSOR_HAND
    0x00000000, 0x003c000c, 0x030800c8, 0x30100c10, 0x10202020, 0x04400840, 0x01800280, 0x00000000,     // RICON_CURSOR_POINTER
    0x00000000, 0x00180000, 0x01f00078, 0x03e007f0, 0x07c003e0, 0x04000e40, 0x00000000, 0x00000000,     // RICON_CURSOR_CLASSIC
    0x00000000, 0x04000000, 0x11000a00, 0x04400a80, 0x01100220, 0x00580088, 0x00000038, 0x00000000,     // RICON_PENCIL
    0x04000000, 0x15000a00, 0x50402880, 0x14102820, 0x05040a08, 0x015c028c, 0x007c00bc, 0x00000000,     // RICON_PENCIL_BIG
    0x01c00000, 0x01400140, 0x01400140, 0x0ff80140, 0x0ff80808, 0x0aa80808, 0x0aa80aa8, 0x00000ff8,     // RICON_BRUSH_CLASSIC
    0x1ffc0000, 0x5ffc7ffe, 0x40004000, 0x00807f80, 0x01c001c0, 0x01c001c0, 0x01c001c0, 0x00000080,     // RICON_BRUSH_PAINTER
    0x00000000, 0x00800000, 0x01c00080, 0x03e001c0, 0x07f003e0, 0x036006f0, 0x000001c0, 0x00000000,     // RICON_WATER_DROP
    0x00000000, 0x3e003800, 0x1f803f80, 0x0c201e40, 0x02080c10, 0x00840104, 0x00380044, 0x00000000,     // RICON_COLOR_PICKER
    0x00000000, 0x07800300, 0x1fe00fc0, 0x3f883fd0, 0x0e021f04, 0x02040402, 0x00f00108, 0x00000000,     // RICON_RUBBER
    0x00c00000, 0x02800140, 0x08200440, 0x20081010, 0x2ffe3004, 0x03f807fc, 0x00e001f0, 0x00000040,     // RICON_COLOR_BUCKET
    0x00000000, 0x21843ffc, 0x01800180, 0x01800180, 0x01800180, 0x01800180, 0x03c00180, 0x00000000,     // RICON_TEXT_T
    0x00800000, 0x01400180, 0x06200340, 0x0c100620, 0x1ff80c10, 0x380c1808, 0x70067004, 0x0000f80f,     // RICON_TEXT_A
    0x78000000, 0x50004000, 0x00004800, 0x03c003c0, 0x03c003c0, 0x00100000, 0x0002000a, 0x0000000e,     // RICON_SCALE
    0x75560000, 0x5e004002, 0x54001002, 0x41001202, 0x408200fe, 0x40820082, 0x40820082, 0x00006afe,     // RICON_RESIZE
    0x00000000, 0x3f003f00, 0x3f003f00, 0x3f003f00, 0x00400080, 0x001c0020, 0x001c001c, 0x00000000,     // RICON_FILTER_POINT
    0x6d800000, 0x00004080, 0x40804080, 0x40800000, 0x00406d80, 0x001c0020, 0x001c001c, 0x00000000,     // RICON_FILTER_BILINEAR
    0x40080000, 0x1ffe2008, 0x14081008, 0x11081208, 0x10481088, 0x10081028, 0x10047ff8, 0x00001002,     // RICON_CROP
    0x00100000, 0x3ffc0010, 0x2ab03550, 0x22b02550, 0x20b02150, 0x20302050, 0x2000fff0, 0x00002000,     // RICON_CROP_ALPHA
    0x40000000, 0x1ff82000, 0x04082808, 0x01082208, 0x00482088, 0x00182028, 0x35542008, 0x00000002,     // RICON_SQUARE_TOGGLE
    0x00000000, 0x02800280, 0x06c006c0, 0x0ea00ee0, 0x1e901eb0, 0x3e883e98, 0x7efc7e8c, 0x00000000,     // RICON_SIMMETRY
    0x01000000, 0x05600100, 0x1d480d50, 0x7d423d44, 0x3d447d42, 0x0d501d48, 0x01000560, 0x00000100,     // RICON_SIMMETRY_HORIZONTAL
    0x01800000, 0x04200240, 0x10080810, 0x00001ff8, 0x00007ffe, 0x0ff01ff8, 0x03c007e0, 0x00000180,     // RICON_SIMMETRY_VERTICAL
    0x00000000, 0x010800f0, 0x02040204, 0x02040204, 0x07f00308, 0x1c000e00, 0x30003800, 0x00000000,     // RICON_LENS
    0x00000000, 0x061803f0, 0x08240c0c, 0x08040814, 0x0c0c0804, 0x23f01618, 0x18002400, 0x00000000,     // RICON_LENS_BIG
    0x00000000, 0x00000000, 0x1c7007c0, 0x638e3398, 0x1c703398, 0x000007c0, 0x00000000, 0x00000000,     // RICON_EYE_ON
    0x00000000, 0x10002000, 0x04700fc0, 0x610e3218, 0x1c703098, 0x001007a0, 0x00000008, 0x00000000,     // RICON_EYE_OFF
    0x00000000, 0x00007ffc, 0x40047ffc, 0x10102008, 0x04400820, 0x02800280, 0x02800280, 0x00000100,     // RICON_FILTER_TOP
    0x00000000, 0x40027ffe, 0x10082004, 0x04200810, 0x02400240, 0x02400240, 0x01400240, 0x000000c0,     // RICON_FILTER
    0x00800000, 0x00800080, 0x00000080, 0x3c9e0000, 0x00000000, 0x00800080, 0x00800080, 0x00000000,     // RICON_TARGET_POINT
    0x00800000, 0x00800080, 0x00800080, 0x3f7e01c0, 0x008001c0, 0x00800080, 0x00800080, 0x00000000,     // RICON_TARGET_SMALL
    0x00800000, 0x00800080, 0x03e00080, 0x3e3e0220, 0x03e00220, 0x00800080, 0x00800080, 0x00000000,     // RICON_TARGET_BIG
    0x01000000, 0x04400280, 0x01000100, 0x43842008, 0x43849ab2, 0x01002008, 0x04400100, 0x01000280,     // RICON_TARGET_MOVE
    0x01000000, 0x04400280, 0x01000100, 0x41042108, 0x41049ff2, 0x01002108, 0x04400100, 0x01000280,     // RICON_CURSOR_MOVE
    0x781e0000, 0x500a4002, 0x04204812, 0x00000240, 0x02400000, 0x48120420, 0x4002500a, 0x0000781e,     // RICON_CURSOR_SCALE
    0x00000000, 0x20003c00, 0x24002800, 0x01000200, 0x00400080, 0x00140024, 0x003c0004, 0x00000000,     // RICON_CURSOR_SCALE_RIGHT
    0x00000000, 0x0004003c, 0x00240014, 0x00800040, 0x02000100, 0x28002400, 0x3c002000, 0x00000000,     // RICON_CURSOR_SCALE_LEFT
    0x00000000, 0x00100020, 0x10101fc8, 0x10001020, 0x10001000, 0x10001000, 0x00001fc0, 0x00000000,     // RICON_UNDO
    0x00000000, 0x08000400, 0x080813f8, 0x00080408, 0x00080008, 0x00080008, 0x000003f8, 0x00000000,     // RICON_REDO
    0x00000000, 0x3ffc0000, 0x20042004, 0x20002000, 0x20402000, 0x3f902020, 0x00400020, 0x00000000,     // RICON_REREDO
    0x00000000, 0x3ffc0000, 0x20042004, 0x27fc2004, 0x20202000, 0x3fc82010, 0x00200010, 0x00000000,     // RICON_MUTATE
    0x00000000, 0x0ff00000, 0x10081818, 0x11801008, 0x10001180, 0x18101020, 0x00100fc8, 0x00000020,     // RICON_ROTATE
    0x00000000, 0x04000200, 0x240429fc, 0x20042204, 0x20442004, 0x3f942024, 0x00400020, 0x00000000,     // RICON_REPEAT
    0x00000000, 0x20001000, 0x22104c0e, 0x00801120, 0x11200040, 0x4c0e2210, 0x10002000, 0x00000000,     // RICON_SHUFFLE
    0x7ffe0000, 0x50024002, 0x44024802, 0x41024202, 0x40424082, 0x40124022, 0x4002400a, 0x00007ffe,     // RICON_EMPTYBOX
    0x00800000, 0x03e00080, 0x08080490, 0x3c9e0808, 0x08080808, 0x03e00490, 0x00800080, 0x00000000,     // RICON_TARGET
    0x00800000, 0x00800080, 0x00800080, 0x3ffe01c0, 0x008001c0, 0x00800080, 0x00800080, 0x00000000,     // RICON_TARGET_SMALL_FILL
    0x00800000, 0x00800080, 0x03e00080, 0x3ffe03e0, 0x03e003e0, 0x00800080, 0x00800080, 0x00000000,     // RICON_TARGET_BIG_FILL
    0x01000000, 0x07c00380, 0x01000100, 0x638c2008, 0x638cfbbe, 0x01002008, 0x07c00100, 0x01000380,     // RICON_TARGET_MOVE_FILL
    0x01000000, 0x07c00380, 0x01000100, 0x610c2108, 0x610cfffe, 0x01002108, 0x07c00100, 0x01000380,     // RICON_CURSOR_MOVE_FILL
    0x781e0000, 0x6006700e, 0x04204812, 0x00000240, 0x02400000, 0x48120420, 0x700e6006, 0x0000781e,     // RICON_CURSOR_SCALE_FILL
    0x00000000, 0x38003c00, 0x24003000, 0x01000200, 0x00400080, 0x000c0024, 0x003c001c, 0x00000000,     // RICON_CURSOR_SCALE_RIGHT
    0x00000000, 0x001c003c, 0x0024000c, 0x00800040, 0x02000100, 0x30002400, 0x3c003800, 0x00000000,     // RICON_CURSOR_SCALE_LEFT
    0x00000000, 0x00300020, 0x10301ff8, 0x10001020, 0x10001000, 0x10001000, 0x00001fc0, 0x00000000,     // RICON_UNDO_FILL
    0x00000000, 0x0c000400, 0x0c081ff8, 0x00080408, 0x00080008, 0x00080008, 0x000003f8, 0x00000000,     // RICON_REDO_FILL
    0x00000000, 0x3ffc0000, 0x20042004, 0x20002000, 0x20402000, 0x3ff02060, 0x00400060, 0x00000000,     // RICON_REREDO_FILL
    0x00000000, 0x3ffc0000, 0x20042004, 0x27fc2004, 0x20202000, 0x3ff82030, 0x00200030, 0x00000000,     // RICON_MUTATE_FILL
    0x00000000, 0x0ff00000, 0x10081818, 0x11801008, 0x10001180, 0x18301020, 0x00300ff8, 0x00000020,     // RICON_ROTATE_FILL
    0x00000000, 0x06000200, 0x26042ffc, 0x20042204, 0x20442004, 0x3ff42064, 0x00400060, 0x00000000,     // RICON_REPEAT_FILL
    0x00000000, 0x30001000, 0x32107c0e, 0x00801120, 0x11200040, 0x7c0e3210, 0x10003000, 0x00000000,     // RICON_SHUFFLE_FILL
    0x00000000, 0x30043ffc, 0x24042804, 0x21042204, 0x20442084, 0x20142024, 0x3ffc200c, 0x00000000,     // RICON_EMPTYBOX_SMALL
    0x00000000, 0x20043ffc, 0x20042004, 0x20042004, 0x20042004, 0x20042004, 0x3ffc2004, 0x00000000,     // RICON_BOX
    0x00000000, 0x23c43ffc, 0x23c423c4, 0x200423c4, 0x20042004, 0x20042004, 0x3ffc2004, 0x00000000,     // RICON_BOX_TOP
    0x00000000, 0x3e043ffc, 0x3e043e04, 0x20043e04, 0x20042004, 0x20042004, 0x3ffc2004, 0x00000000,     // RICON_BOX_TOP_RIGHT
    0x00000000, 0x20043ffc, 0x20042004, 0x3e043e04, 0x3e043e04, 0x20042004, 0x3ffc2004, 0x00000000,     // RICON_BOX_RIGHT
    0x00000000, 0x20043ffc, 0x20042004, 0x20042004, 0x3e042004, 0x3e043e04, 0x3ffc3e04, 0x00000000,     // RICON_BOX_BOTTOM_RIGHT
    0x00000000, 0x20043ffc, 0x20042004, 0x20042004, 0x23c42004, 0x23c423c4, 0x3ffc23c4, 0x00000000,     // RICON_BOX_BOTTOM
    0x00000000, 0x20043ffc, 0x20042004, 0x20042004, 0x207c2004, 0x207c207c, 0x3ffc207c, 0x00000000,     // RICON_BOX_BOTTOM_LEFT
    0x00000000, 0x20043ffc, 0x20042004, 0x207c207c, 0x207c207c, 0x20042004, 0x3ffc2004, 0x00000000,     // RICON_BOX_LEFT
    0x00000000, 0x207c3ffc, 0x207c207c, 0x2004207c, 0x20042004, 0x20042004, 0x3ffc2004, 0x00000000,     // RICON_BOX_TOP_LEFT
    0x00000000, 0x20043ffc, 0x20042004, 0x23c423c4, 0x23c423c4, 0x20042004, 0x3ffc2004, 0x00000000,     // RICON_BOX_CIRCLE_MASK
    0x7ffe0000, 0x40024002, 0x47e24182, 0x4ff247e2, 0x47e24ff2, 0x418247e2, 0x40024002, 0x00007ffe,     // RICON_BOX_CENTER
    0x7fff0000, 0x40014001, 0x40014001, 0x49555ddd, 0x4945495d, 0x400149c5, 0x40014001, 0x00007fff,     // RICON_POT
    0x7ffe0000, 0x53327332, 0x44ce4cce, 0x41324332, 0x404e40ce, 0x48125432, 0x4006540e, 0x00007ffe,     // RICON_ALPHA_MULTIPLY
    0x7ffe0000, 0x53327332, 0x44ce4cce, 0x41324332, 0x5c4e40ce, 0x44124432, 0x40065c0e, 0x00007ffe,     // RICON_ALPHA_CLEAR
    0x7ffe0000, 0x42fe417e, 0x42fe417e, 0x42fe417e, 0x42fe417e, 0x42fe417e, 0x42fe417e, 0x00007ffe,     // RICON_DITHERING
    0x07fe0000, 0x1ffa0002, 0x7fea000a, 0x402a402a, 0x5b2a512a, 0x5128552a, 0x40205128, 0x00007fe0,     // RICON_MIPMAPS
    0x00000000, 0x1ff80000, 0x12481248, 0x12481ff8, 0x1ff81248, 0x12481248, 0x00001ff8, 0x00000000,     // RICON_BOX_GRID
    0x12480000, 0x7ffe1248, 0x12481248, 0x12487ffe, 0x7ffe1248, 0x12481248, 0x12487ffe, 0x00001248,     // RICON_GRID
    0x00000000, 0x1c380000, 0x1c3817e8, 0x08100810, 0x08100810, 0x17e81c38, 0x00001c38, 0x00000000,     // RICON_BOX_CORNERS_SMALL
    0x700e0000, 0x700e5ffa, 0x20042004, 0x20042004, 0x20042004, 0x20042004, 0x5ffa700e, 0x0000700e,     // RICON_BOX_CORNERS_BIG
    0x3f7e0000, 0x21422142, 0x21422142, 0x00003f7e, 0x21423f7e, 0x21422142, 0x3f7e2142, 0x00000000,     // RICON_FOUR_BOXES
    0x00000000, 0x3bb80000, 0x3bb83bb8, 0x3bb80000, 0x3bb83bb8, 0x3bb80000, 0x3bb83bb8, 0x00000000,     // RICON_GRID_FILL
    0x7ffe0000, 0x7ffe7ffe, 0x77fe7000, 0x77fe77fe, 0x777e7700, 0x777e777e, 0x777e777e, 0x0000777e,     // RICON_BOX_MULTISIZE
    0x781e0000, 0x40024002, 0x00004002, 0x01800000, 0x00000180, 0x40020000, 0x40024002, 0x0000781e,     // RICON_ZOOM_SMALL
    0x781e0000, 0x40024002, 0x00004002, 0x03c003c0, 0x03c003c0, 0x40020000, 0x40024002, 0x0000781e,     // RICON_ZOOM_MEDIUM
    0x781e0000, 0x40024002, 0x07e04002, 0x07e007e0, 0x07e007e0, 0x400207e0, 0x40024002, 0x0000781e,     // RICON_ZOOM_BIG
    0x781e0000, 0x5ffa4002, 0x1ff85ffa, 0x1ff81ff8, 0x1ff81ff8, 0x5ffa1ff8, 0x40025ffa, 0x0000781e,     // RICON_ZOOM_ALL
    0x00000000, 0x2004381c, 0x00002004, 0x00000000, 0x00000000, 0x20040000, 0x381c2004, 0x00000000,     // RICON_ZOOM_CENTER
    0x00000000, 0x1db80000, 0x10081008, 0x10080000, 0x00001008, 0x10081008, 0x00001db8, 0x00000000,     // RICON_BOX_DOTS_SMALL
    0x35560000, 0x00002002, 0x00002002, 0x00002002, 0x00002002, 0x00002002, 0x35562002, 0x00000000,     // RICON_BOX_DOTS_BIG
    0x7ffe0000, 0x40024002, 0x48124ff2, 0x49924812, 0x48124992, 0x4ff24812, 0x40024002, 0x00007ffe,     // RICON_BOX_CONCENTRIC
    0x00000000, 0x10841ffc, 0x10841084, 0x1ffc1084, 0x10841084, 0x10841084, 0x00001ffc, 0x00000000,     // RICON_BOX_GRID_BIG
    0x00000000, 0x00000000, 0x10000000, 0x04000800, 0x01040200, 0x00500088, 0x00000020, 0x00000000,     // RICON_OK_TICK
    0x00000000, 0x10080000, 0x04200810, 0x01800240, 0x02400180, 0x08100420, 0x00001008, 0x00000000,     // RICON_CROSS
    0x00000000, 0x02000000, 0x00800100, 0x00200040, 0x00200010, 0x00800040, 0x02000100, 0x00000000,     // RICON_ARROW_LEFT
    0x00000000, 0x00400000, 0x01000080, 0x04000200, 0x04000800, 0x01000200, 0x00400080, 0x00000000,     // RICON_ARROW_RIGHT
    0x00000000, 0x00000000, 0x00000000, 0x08081004, 0x02200410, 0x00800140, 0x00000000, 0x00000000,     // RICON_ARROW_DOWN
    0x00000000, 0x00000000, 0x01400080, 0x04100220, 0x10040808, 0x00000000, 0x00000000, 0x00000000,     // RICON_ARROW_UP
    0x00000000, 0x02000000, 0x03800300, 0x03e003c0, 0x03e003f0, 0x038003c0, 0x02000300, 0x00000000,     // RICON_ARROW_LEFT_FILL
    0x00000000, 0x00400000, 0x01c000c0, 0x07c003c0, 0x07c00fc0, 0x01c003c0, 0x004000c0, 0x00000000,     // RICON_ARROW_RIGHT_FILL
    0x00000000, 0x00000000, 0x00000000, 0x0ff81ffc, 0x03e007f0, 0x008001c0, 0x00000000, 0x00000000,     // RICON_ARROW_DOWN_FILL
    0x00000000, 0x00000000, 0x01c00080, 0x07f003e0, 0x1ffc0ff8, 0x00000000, 0x00000000, 0x00000000,     // RICON_ARROW_UP_FILL
    0x00000000, 0x18a008c0, 0x32881290, 0x24822686, 0x26862482, 0x12903288, 0x08c018a0, 0x00000000,     // RICON_AUDIO
    0x00000000, 0x04800780, 0x004000c0, 0x662000f0, 0x08103c30, 0x130a0e18, 0x0000318e, 0x00000000,     // RICON_FX
    0x00000000, 0x00800000, 0x08880888, 0x2aaa0a8a, 0x0a8a2aaa, 0x08880888, 0x00000080, 0x00000000,     // RICON_WAVE
    0x00000000, 0x00600000, 0x01080090, 0x02040108, 0x42044204, 0x24022402, 0x00001800, 0x00000000,     // RICON_WAVE_SINUS
    0x00000000, 0x07f80000, 0x04080408, 0x04080408, 0x04080408, 0x7c0e0408, 0x00000000, 0x00000000,     // RICON_WAVE_SQUARE
    0x00000000, 0x00000000, 0x00a00040, 0x22084110, 0x08021404, 0x00000000, 0x00000000, 0x00000000,     // RICON_WAVE_TRIANGULAR
    0x00000000, 0x00000000, 0x04200000, 0x01800240, 0x02400180, 0x00000420, 0x00000000, 0x00000000,     // RICON_CROSS_SMALL
    0x00000000, 0x18380000, 0x12281428, 0x10a81128, 0x112810a8, 0x14281228, 0x00001838, 0x00000000,     // RICON_PLAYER_PREVIOUS
    0x00000000, 0x18000000, 0x11801600, 0x10181060, 0x10601018, 0x16001180, 0x00001800, 0x00000000,     // RICON_PLAYER_PLAY_BACK
    0x00000000, 0x00180000, 0x01880068, 0x18080608, 0x06081808, 0x00680188, 0x00000018, 0x00000000,     // RICON_PLAYER_PLAY
    0x00000000, 0x1e780000, 0x12481248, 0x12481248, 0x12481248, 0x12481248, 0x00001e78, 0x00000000,     // RICON_PLAYER_PAUSE
    0x00000000, 0x1ff80000, 0x10081008, 0x10081008, 0x10081008, 0x10081008, 0x00001ff8, 0x00000000,     // RICON_PLAYER_STOP
    0x00000000, 0x1c180000, 0x14481428, 0x15081488, 0x14881508, 0x14281448, 0x00001c18, 0x00000000,     // RICON_PLAYER_NEXT
    0x00000000, 0x03c00000, 0x08100420, 0x10081008, 0x10081008, 0x04200810, 0x000003c0, 0x00000000,     // RICON_PLAYER_RECORD
    0x00000000, 0x0c3007e0, 0x13c81818, 0x14281668, 0x14281428, 0x1c381c38, 0x08102244, 0x00000000,     // RICON_MAGNET
    0x07c00000, 0x08200820, 0x3ff80820, 0x23882008, 0x21082388, 0x20082108, 0x1ff02008, 0x00000000,     // RICON_LOCK_CLOSE
    0x07c00000, 0x08000800, 0x3ff80800, 0x23882008, 0x21082388, 0x20082108, 0x1ff02008, 0x00000000,     // RICON_LOCK_OPEN
    0x01c00000, 0x0c180770, 0x3086188c, 0x60832082, 0x60034781, 0x30062002, 0x0c18180c, 0x01c00770,     // RICON_CLOCK
    0x0a200000, 0x1b201b20, 0x04200e20, 0x04200420, 0x04700420, 0x0e700e70, 0x0e700e70, 0x04200e70,     // RICON_TOOLS
    0x01800000, 0x3bdc318c, 0x0ff01ff8, 0x7c3e1e78, 0x1e787c3e, 0x1ff80ff0, 0x318c3bdc, 0x00000180,     // RICON_GEAR
    0x01800000, 0x3ffc318c, 0x1c381ff8, 0x781e1818, 0x1818781e, 0x1ff81c38, 0x318c3ffc, 0x00000180,     // RICON_GEAR_BIG
    0x00000000, 0x08080ff8, 0x08081ffc, 0x0aa80aa8, 0x0aa80aa8, 0x0aa80aa8, 0x08080aa8, 0x00000ff8,     // RICON_BIN
    0x00000000, 0x00000000, 0x20043ffc, 0x08043f84, 0x04040f84, 0x04040784, 0x000007fc, 0x00000000,     // RICON_HAND_POINTER
    0x00000000, 0x24400400, 0x00001480, 0x6efe0e00, 0x00000e00, 0x24401480, 0x00000400, 0x00000000,     // RICON_LASER
    0x00000000, 0x03c00000, 0x08300460, 0x11181118, 0x11181118, 0x04600830, 0x000003c0, 0x00000000,     // RICON_COIN
    0x00000000, 0x10880080, 0x06c00810, 0x366c07e0, 0x07e00240, 0x00001768, 0x04200240, 0x00000000,     // RICON_EXPLOSION
    0x00000000, 0x3d280000, 0x2528252c, 0x3d282528, 0x05280528, 0x05e80528, 0x00000000, 0x00000000,     // RICON_1UP
    0x01800000, 0x03c003c0, 0x018003c0, 0x0ff007e0, 0x0bd00bd0, 0x0a500bd0, 0x02400240, 0x02400240,     // RICON_PLAYER
    0x01800000, 0x03c003c0, 0x118013c0, 0x03c81ff8, 0x07c003c8, 0x04400440, 0x0c080478, 0x00000000,     // RICON_PLAYER_JUMP
    0x3ff80000, 0x30183ff8, 0x30183018, 0x3ff83ff8, 0x03000300, 0x03c003c0, 0x03e00300, 0x000003e0,     // RICON_KEY
    0x3ff80000, 0x3ff83ff8, 0x33983ff8, 0x3ff83398, 0x3ff83ff8, 0x00000540, 0x0fe00aa0, 0x00000fe0,     // RICON_DEMON
    0x00000000, 0x0ff00000, 0x20041008, 0x25442004, 0x10082004, 0x06000bf0, 0x00000300, 0x00000000,     // RICON_TEXT_POPUP
    0x00000000, 0x11440000, 0x07f00be8, 0x1c1c0e38, 0x1c1c0c18, 0x07f00e38, 0x11440be8, 0x00000000,     // RICON_GEAR_EX
    0x00000000, 0x20080000, 0x0c601010, 0x07c00fe0, 0x07c007c0, 0x0c600fe0, 0x20081010, 0x00000000,     // RICON_CRACK
    0x00000000, 0x20080000, 0x0c601010, 0x04400fe0, 0x04405554, 0x0c600fe0, 0x20081010, 0x00000000,     // RICON_CRACK_POINTS
    0x00000000, 0x00800080, 0x01c001c0, 0x1ffc3ffe, 0x03e007f0, 0x07f003e0, 0x0c180770, 0x00000808,     // RICON_STAR
    0x0ff00000, 0x08180810, 0x08100818, 0x0a100810, 0x08180810, 0x08100818, 0x08100810, 0x00001ff8,     // RICON_DOOR
    0x0ff00000, 0x08100810, 0x08100810, 0x10100010, 0x4f902010, 0x10102010, 0x08100010, 0x00000ff0,     // RICON_EXIT
    0x00040000, 0x001f000e, 0x0ef40004, 0x12f41284, 0x0ef41214, 0x10040004, 0x7ffc3004, 0x10003000,     // RICON_MODE_2D
    0x78040000, 0x501f600e, 0x0ef44004, 0x12f41284, 0x0ef41284, 0x10140004, 0x7ffc300c, 0x10003000,     // RICON_MODE_3D
    0x7fe00000, 0x50286030, 0x47fe4804, 0x44224402, 0x44224422, 0x241275e2, 0x0c06140a, 0x000007fe,     // RICON_CUBE
    0x7fe00000, 0x5ff87ff0, 0x47fe4ffc, 0x44224402, 0x44224422, 0x241275e2, 0x0c06140a, 0x000007fe,     // RICON_CUBE_FACE_TOP
    0x7fe00000, 0x50386030, 0x47fe483c, 0x443e443e, 0x443e443e, 0x241e75fe, 0x0c06140e, 0x000007fe,     // RICON_CUBE_FACE_LEFT
    0x7fe00000, 0x50286030, 0x47fe4804, 0x47fe47fe, 0x47fe47fe, 0x27fe77fe, 0x0ffe17fe, 0x000007fe,     // RICON_CUBE_FACE_FRONT
    0x7fe00000, 0x50286030, 0x47fe4804, 0x44224402, 0x44224422, 0x3ff27fe2, 0x0ffe1ffa, 0x000007fe,     // RICON_CUBE_FACE_BOTTOM
    0x7fe00000, 0x70286030, 0x7ffe7804, 0x7c227c02, 0x7c227c22, 0x3c127de2, 0x0c061c0a, 0x000007fe,     // RICON_CUBE_FACE_RIGHT
    0x7fe00000, 0x7fe87ff0, 0x7ffe7fe4, 0x7fe27fe2, 0x7fe27fe2, 0x24127fe2, 0x0c06140a, 0x000007fe,     // RICON_CUBE_FACE_BACK
    0x00000000, 0x2a0233fe, 0x22022602, 0x22022202, 0x2a022602, 0x00a033fe, 0x02080110, 0x00000000,     // RICON_CAMERA
    0x00000000, 0x200c3ffc, 0x000c000c, 0x3ffc000c, 0x30003000, 0x30003000, 0x3ffc3004, 0x00000000,     // RICON_SPECIAL
    0x00000000, 0x0022003e, 0x012201e2, 0x0100013e, 0x01000100, 0x79000100, 0x4f004900, 0x00007800,     // RICON_LINK_NET
    0x00000000, 0x44007c00, 0x45004600, 0x00627cbe, 0x00620022, 0x45007cbe, 0x44004600, 0x00007c00,     // RICON_LINK_BOXES
    0x00000000, 0x0044007c, 0x0010007c, 0x3f100010, 0x3f1021f0, 0x3f100010, 0x3f0021f0, 0x00000000,     // RICON_LINK_MULTI
    0x00000000, 0x0044007c, 0x00440044, 0x0010007c, 0x00100010, 0x44107c10, 0x440047f0, 0x00007c00,     // RICON_LINK
    0x00000000, 0x0044007c, 0x00440044, 0x0000007c, 0x00000010, 0x44007c10, 0x44004550, 0x00007c00,     // RICON_LINK_BROKE
    0x02a00000, 0x22a43ffc, 0x20042004, 0x20042ff4, 0x20042ff4, 0x20042ff4, 0x20042004, 0x00003ffc,     // RICON_TEXT_NOTES
    0x3ffc0000, 0x20042004, 0x245e27c4, 0x27c42444, 0x2004201e, 0x201e2004, 0x20042004, 0x00003ffc,     // RICON_NOTEBOOK
    0x00000000, 0x07e00000, 0x04200420, 0x24243ffc, 0x24242424, 0x24242424, 0x3ffc2424, 0x00000000,     // RICON_SUITCASE
    0x00000000, 0x0fe00000, 0x08200820, 0x40047ffc, 0x7ffc5554, 0x40045554, 0x7ffc4004, 0x00000000,     // RICON_SUITCASE_ZIP
    0x00000000, 0x20043ffc, 0x3ffc2004, 0x13c81008, 0x100813c8, 0x10081008, 0x1ff81008, 0x00000000,     // RICON_MAILBOX
    0x00000000, 0x40027ffe, 0x5ffa5ffa, 0x5ffa5ffa, 0x40025ffa, 0x03c07ffe, 0x1ff81ff8, 0x00000000,     // RICON_MONITOR
    0x0ff00000, 0x6bfe7ffe, 0x7ffe7ffe, 0x68167ffe, 0x08106816, 0x08100810, 0x0ff00810, 0x00000000,     // RICON_PRINTER
    0x3ff80000, 0xfffe2008, 0x870a8002, 0x904a888a, 0x904a904a, 0x870a888a, 0xfffe8002, 0x00000000,     // RICON_PHOTO_CAMERA
    0x0fc00000, 0xfcfe0cd8, 0x8002fffe, 0x84428382, 0x84428442, 0x80028382, 0xfffe8002, 0x00000000,     // RICON_PHOTO_CAMERA_FLASH
    0x00000000, 0x02400180, 0x08100420, 0x20041008, 0x23c42004, 0x22442244, 0x3ffc2244, 0x00000000,     // RICON_HOUSE
    0x00000000, 0x1c700000, 0x3ff83ef8, 0x3ff83ff8, 0x0fe01ff0, 0x038007c0, 0x00000100, 0x00000000,     // RICON_HEART
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x80000000, 0xe000c000,     // RICON_CORNER
    0x00000000, 0x14001c00, 0x15c01400, 0x15401540, 0x155c1540, 0x15541554, 0x1ddc1554, 0x00000000,     // RICON_VERTICAL_BARS
    0x00000000, 0x03000300, 0x1b001b00, 0x1b601b60, 0x1b6c1b60, 0x1b6c1b6c, 0x1b6c1b6c, 0x00000000,     // RICON_VERTICAL_BARS_FILL
    0x00000000, 0x00000000, 0x403e7ffe, 0x7ffe403e, 0x7ffe0000, 0x43fe43fe, 0x00007ffe, 0x00000000,     // RICON_LIFE_BARS
    0x7ffc0000, 0x43844004, 0x43844284, 0x43844004, 0x42844284, 0x42844284, 0x40044384, 0x00007ffc,     // RICON_INFO
    0x40008000, 0x10002000, 0x04000800, 0x01000200, 0x00400080, 0x00100020, 0x00040008, 0x00010002,     // RICON_CROSSLINE
    0x00000000, 0x1ff01ff0, 0x18301830, 0x1f001830, 0x03001f00, 0x00000300, 0x03000300, 0x00000000,     // RICON_HELP
    0x3ff00000, 0x2abc3550, 0x2aac3554, 0x2aac3554, 0x2aac3554, 0x2aac3554, 0x2aac3554, 0x00003ffc,     // RICON_FILETYPE_ALPHA
    0x3ff00000, 0x201c2010, 0x22442184, 0x28142424, 0x29942814, 0x2ff42994, 0x20042004, 0x00003ffc,     // RICON_FILETYPE_HOME
    0x07fe0000, 0x04020402, 0x7fe20402, 0x44224422, 0x44224422, 0x402047fe, 0x40204020, 0x00007fe0,     // RICON_LAYERS_VISIBLE
    0x07fe0000, 0x04020402, 0x7c020402, 0x44024402, 0x44024402, 0x402047fe, 0x40204020, 0x00007fe0,     // RICON_LAYERS
    0x00000000, 0x40027ffe, 0x7ffe4002, 0x40024002, 0x40024002, 0x40024002, 0x7ffe4002, 0x00000000,     // RICON_WINDOW
    0x09100000, 0x09f00910, 0x09100910, 0x00000910, 0x24a2779e, 0x27a224a2, 0x709e20a2, 0x00000000,     // RICON_HIDPI
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_200
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_201
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_202
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_203
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_204
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_205
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_206
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_207
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_208
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_209
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_210
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_211
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_212
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_213
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_214
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_215
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_216
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_217
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_218
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_219
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_220
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_221
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_222
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_223
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_224
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_225
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_226
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_227
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_228
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_229
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_230
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_231
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_232
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_233
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_234
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_235
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_236
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_237
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_238
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_239
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_240
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_241
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_242
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_243
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_244
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_245
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_246
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_247
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_248
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_249
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_250
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_251
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_252
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_253
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_254
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,     // RICON_255
};


#ifndef RICON_SIZE
    #define RICON_SIZE                   0
#endif

#define RAYGUI_MAX_CONTROLS             16      // Maximum number of standard controls
#define RAYGUI_MAX_PROPS_BASE           16      // Maximum number of standard properties
#define RAYGUI_MAX_PROPS_EXTENDED        8      // Maximum number of extended properties

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// Gui control property style color element
typedef enum { BORDER = 0, BASE, TEXT, OTHER } GuiPropertyElement;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static GuiControlState guiState = GUI_STATE_NORMAL;

static TextFont guiFont = { 0 };            // Gui current font (WARNING: highly coupled to raylib)
static bool guiLocked = false;          // Gui lock state (no inputs processed)
static float guiAlpha = 1.0f;           // Gui element transpacency on drawing

//----------------------------------------------------------------------------------
// Style data array for all gui style properties (allocated on data segment by default)
//
// NOTE 1: First set of BASE properties are generic to all controls but could be individually
// overwritten per control, first set of EXTENDED properties are generic to all controls and
// can not be overwritten individually but custom EXTENDED properties can be used by control
//
// NOTE 2: A new style set could be loaded over this array using GuiLoadStyle(),
// but default gui style could always be recovered with GuiLoadStyleDefault()
//
// guiStyle size is by default: 16*(16 + 8) = 384*4 = 1536 bytes = 1.5 KB
//----------------------------------------------------------------------------------
static unsigned int guiStyle[RAYGUI_MAX_CONTROLS*(RAYGUI_MAX_PROPS_BASE + RAYGUI_MAX_PROPS_EXTENDED)] = { 0 };

static bool guiStyleLoaded = false;     // Style loaded flag for lazy style initialization


//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
static int GetTextWidth(const char *text);                      // Gui get text width using default font
static Rectangle GetTextBounds(int control, Rectangle bounds);  // Get text bounds considering control bounds
static const char *GetTextIcon(const char *text, int *iconId);  // Get text icon if provided and move text cursor

static void GuiDrawText(const char *text, Rectangle bounds, int alignment, Color tint);         // Gui draw text using default font
static void GuiDrawRectangle(Rectangle rec, int borderWidth, Color borderColor, Color color);   // Gui draw rectangle using default raygui style

static const char **GuiTextSplit(const char *text, int *count, int *textRow);   // Split controls text into multiple strings
/*

// Convert color data from RGB to HSV
// NOTE: Color data should be passed normalized
static Vector3 ConvertRGBtoHSV(Vector3 rgb)
{
    Vector3 hsv = { 0 };
    float min = 0.0f;
    float max = 0.0f;
    float delta = 0.0f;

    min = (rgb.x < rgb.y)? rgb.x : rgb.y;
    min = (min < rgb.z)? min  : rgb.z;

    max = (rgb.x > rgb.y)? rgb.x : rgb.y;
    max = (max > rgb.z)? max  : rgb.z;

    hsv.z = max;            // Value
    delta = max - min;

    if (delta < 0.00001f)
    {
        hsv.y = 0.0f;
        hsv.x = 0.0f;           // Undefined, maybe NAN?
        return hsv;
    }

    if (max > 0.0f)
    {
        // NOTE: If max is 0, this divide would cause a crash
        hsv.y = (delta/max);    // Saturation
    }
    else
    {
        // NOTE: If max is 0, then r = g = b = 0, s = 0, h is undefined
        hsv.y = 0.0f;
        hsv.x = 0.0f;           // Undefined, maybe NAN?
        return hsv;
    }

    // NOTE: Comparing float values could not work properly
    if (rgb.x >= max) hsv.x = (rgb.y - rgb.z)/delta;    // Between yellow & magenta
    else
    {
        if (rgb.y >= max) hsv.x = 2.0f + (rgb.z - rgb.x)/delta;  // Between cyan & yellow
        else hsv.x = 4.0f + (rgb.x - rgb.y)/delta;      // Between magenta & cyan
    }

    hsv.x *= 60.0f;     // Convert to degrees

    if (hsv.x < 0.0f) hsv.x += 360.0f;

    return hsv;
}

// Convert color data from HSV to RGB
// NOTE: Color data should be passed normalized
static Vector3 ConvertHSVtoRGB(Vector3 hsv)
{
    Vector3 rgb = { 0 };
    float hh = 0.0f, p = 0.0f, q = 0.0f, t = 0.0f, ff = 0.0f;
    long i = 0;

    // NOTE: Comparing float values could not work properly
    if (hsv.y <= 0.0f)
    {
        rgb.x = hsv.z;
        rgb.y = hsv.z;
        rgb.z = hsv.z;
        return rgb;
    }

    hh = hsv.x;
    if (hh >= 360.0f) hh = 0.0f;
    hh /= 60.0f;

    i = (long)hh;
    ff = hh - i;
    p = hsv.z*(1.0f - hsv.y);
    q = hsv.z*(1.0f - (hsv.y*ff));
    t = hsv.z*(1.0f - (hsv.y*(1.0f - ff)));

    switch (i)
    {
        case 0:
        {
            rgb.x = hsv.z;
            rgb.y = t;
            rgb.z = p;
        } break;
        case 1:
        {
            rgb.x = q;
            rgb.y = hsv.z;
            rgb.z = p;
        } break;
        case 2:
        {
            rgb.x = p;
            rgb.y = hsv.z;
            rgb.z = t;
        } break;
        case 3:
        {
            rgb.x = p;
            rgb.y = q;
            rgb.z = hsv.z;
        } break;
        case 4:
        {
            rgb.x = t;
            rgb.y = p;
            rgb.z = hsv.z;
        } break;
        case 5:
        default:
        {
            rgb.x = hsv.z;
            rgb.y = p;
            rgb.z = q;
        } break;
    }

    return rgb;
}*/
//*****************************************************************************************************************************************************************************************

//----------------------------------------------------------------------------------
// Gui Setup Functions Definition
//----------------------------------------------------------------------------------
// Enable gui global state
void GuiEnable(void) { guiState = GUI_STATE_NORMAL; }

// Disable gui global state
void GuiDisable(void) { guiState = GUI_STATE_DISABLED; }

// Lock gui global state
void GuiLock(void) { guiLocked = true; }

// Unlock gui global state
void GuiUnlock(void) { guiLocked = false; }

// Check if gui is locked (global state)
bool GuiIsLocked(void) { return guiLocked; }

// Set gui controls alpha global state
void GuiFade(float alpha)
{
    if (alpha < 0.0f) alpha = 0.0f;
    else if (alpha > 1.0f) alpha = 1.0f;

    guiAlpha = alpha;
}

// Set gui state (global state)
void GuiSetState(int state) { guiState = (GuiControlState)state; }

// Get gui state (global state)
int GuiGetState(void) { return guiState; }

// Set custom gui font
// NOTE: Font loading/unloading is external to raygui
void GuiSetFont(TextFont font)
{
    if (font.texture.id > 0)
    {
        // NOTE: If we try to setup a font but default style has not been
        // lazily loaded before, it will be overwritten, so we need to force
        // default style loading first
        if (!guiStyleLoaded) GuiLoadStyleDefault();

        guiFont = font;
        GuiSetStyle(DEFAULT, TEXT_SIZE, font.baseSize);
    }
}

// Get custom gui font
TextFont GuiGetFont(void)
{
    return guiFont;
}

// Set control style property value
void GuiSetStyle(int control, int property, int value)
{
    if (!guiStyleLoaded) GuiLoadStyleDefault();
    guiStyle[control*(RAYGUI_MAX_PROPS_BASE + RAYGUI_MAX_PROPS_EXTENDED) + property] = value;

    // Default properties are propagated to all controls
    if ((control == 0) && (property < RAYGUI_MAX_PROPS_BASE))
    {
        for (int i = 1; i < RAYGUI_MAX_CONTROLS; i++) guiStyle[i*(RAYGUI_MAX_PROPS_BASE + RAYGUI_MAX_PROPS_EXTENDED) + property] = value;
    }
}

// Get control style property value
int GuiGetStyle(int control, int property)
{
    if (!guiStyleLoaded) GuiLoadStyleDefault();
    return guiStyle[control*(RAYGUI_MAX_PROPS_BASE + RAYGUI_MAX_PROPS_EXTENDED) + property];
}

//----------------------------------------------------------------------------------
// Gui Controls Functions Definition
//----------------------------------------------------------------------------------

// Window Box control
bool GuiWindowBox(Rectangle bounds, const char *title)
{
    // NOTE: This define is also used by GuiMessageBox() and GuiTextInputBox()
    #define WINDOW_STATUSBAR_HEIGHT        22

    //GuiControlState state = guiState;
    bool clicked = false;

    int statusBarHeight = WINDOW_STATUSBAR_HEIGHT + 2*GuiGetStyle(STATUSBAR, BORDER_WIDTH);
    statusBarHeight += (statusBarHeight%2);

    Rectangle statusBar = { bounds.x, bounds.y, bounds.width, (float)statusBarHeight };
    if (bounds.height < statusBarHeight*2.0f) bounds.height = statusBarHeight*2.0f;

    Rectangle windowPanel = { bounds.x, bounds.y + (float)statusBarHeight - 1, bounds.width, bounds.height - (float)statusBarHeight };
    Rectangle closeButtonRec = { statusBar.x + statusBar.width - GuiGetStyle(STATUSBAR, BORDER_WIDTH) - 20,
                                 statusBar.y + statusBarHeight/2.0f - 18.0f/2.0f, 18, 18 };

    // Update control
    //--------------------------------------------------------------------
    // NOTE: Logic is directly managed by button
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    GuiStatusBar(statusBar, title); // Draw window header as status bar
    GuiPanel(windowPanel);          // Draw window base

    // Draw window close button
    int tempBorderWidth = GuiGetStyle(BUTTON, BORDER_WIDTH);
    int tempTextAlignment = GuiGetStyle(BUTTON, TEXT_ALIGNMENT);
    GuiSetStyle(BUTTON, BORDER_WIDTH, 1);
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_CENTER);
#if defined(RAYGUI_NO_RICONS)
    clicked = GuiButton(closeButtonRec, "x");
#else
    clicked = GuiButton(closeButtonRec, GuiIconText(RICON_CROSS_SMALL, NULL));
#endif
    GuiSetStyle(BUTTON, BORDER_WIDTH, tempBorderWidth);
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, tempTextAlignment);
    //--------------------------------------------------------------------

    return clicked;
}

// Group Box control with text name
void GuiGroupBox(Rectangle bounds, const char *text)
{
    #define GROUPBOX_LINE_THICK     1
    #define GROUPBOX_TEXT_PADDING  10

    GuiControlState state = guiState;

    // Draw control
    //--------------------------------------------------------------------
    GuiDrawRectangle(RAYGUI_CLITERAL(Rectangle){ bounds.x, bounds.y, GROUPBOX_LINE_THICK, bounds.height }, 0, BLANK, Fade(GetColor(GuiGetStyle(DEFAULT, (state == GUI_STATE_DISABLED)? BORDER_COLOR_DISABLED : LINE_COLOR)), guiAlpha));
    GuiDrawRectangle(RAYGUI_CLITERAL(Rectangle){ bounds.x, bounds.y + bounds.height - 1, bounds.width, GROUPBOX_LINE_THICK }, 0, BLANK, Fade(GetColor(GuiGetStyle(DEFAULT, (state == GUI_STATE_DISABLED)? BORDER_COLOR_DISABLED : LINE_COLOR)), guiAlpha));
    GuiDrawRectangle(RAYGUI_CLITERAL(Rectangle){ bounds.x + bounds.width - 1, bounds.y, GROUPBOX_LINE_THICK, bounds.height }, 0, BLANK, Fade(GetColor(GuiGetStyle(DEFAULT, (state == GUI_STATE_DISABLED)? BORDER_COLOR_DISABLED : LINE_COLOR)), guiAlpha));

    GuiLine(RAYGUI_CLITERAL(Rectangle){ bounds.x, bounds.y, bounds.width, 1 }, text);
    //--------------------------------------------------------------------
}

// Line control
void GuiLine(Rectangle bounds, const char *text)
{
    #define LINE_TEXT_PADDING  10

    GuiControlState state = guiState;

    Color color = Fade(GetColor(GuiGetStyle(DEFAULT, (state == GUI_STATE_DISABLED)? BORDER_COLOR_DISABLED : LINE_COLOR)), guiAlpha);

    // Draw control
    //--------------------------------------------------------------------
    if (text == NULL) GuiDrawRectangle(RAYGUI_CLITERAL(Rectangle){ bounds.x, bounds.y + bounds.height/2, bounds.width, 1 }, 0, BLANK, color);
    else
    {
        Rectangle textBounds = { 0 };
        textBounds.width = (float)GetTextWidth(text);
        textBounds.height = (float)GuiGetStyle(DEFAULT, TEXT_SIZE);
        textBounds.x = bounds.x + LINE_TEXT_PADDING;
        textBounds.y = bounds.y - (float)GuiGetStyle(DEFAULT, TEXT_SIZE)/2;

        // Draw line with embedded text label: "--- text --------------"
        GuiDrawRectangle(RAYGUI_CLITERAL(Rectangle){ bounds.x, bounds.y, LINE_TEXT_PADDING - 2, 1 }, 0, BLANK, color);
        GuiLabel(textBounds, text);
        GuiDrawRectangle(RAYGUI_CLITERAL(Rectangle){ bounds.x + LINE_TEXT_PADDING + textBounds.width + 4, bounds.y, bounds.width - textBounds.width - LINE_TEXT_PADDING - 4, 1 }, 0, BLANK, color);
    }
    //--------------------------------------------------------------------
}

// Panel control
void GuiPanel(Rectangle bounds)
{
    #define PANEL_BORDER_WIDTH   1

    GuiControlState state = guiState;

    // Draw control
    //--------------------------------------------------------------------
    GuiDrawRectangle(bounds, PANEL_BORDER_WIDTH, Fade(GetColor(GuiGetStyle(DEFAULT, (state == GUI_STATE_DISABLED)? BORDER_COLOR_DISABLED: LINE_COLOR)), guiAlpha),
                     Fade(GetColor(GuiGetStyle(DEFAULT, (state == GUI_STATE_DISABLED)? BASE_COLOR_DISABLED : BACKGROUND_COLOR)), guiAlpha));
    //--------------------------------------------------------------------
}

// Scroll Panel control
Rectangle GuiScrollPanel(Rectangle bounds, Rectangle content, Vector2 *scroll)
{
    GuiControlState state = guiState;

    Vector2 scrollPos = { 0.0f, 0.0f };
    if (scroll != NULL) scrollPos = *scroll;

    bool hasHorizontalScrollBar = (content.width > bounds.width - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH))? true : false;
    bool hasVerticalScrollBar = (content.height > bounds.height - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH))? true : false;

    // Recheck to account for the other scrollbar being visible
    if (!hasHorizontalScrollBar) hasHorizontalScrollBar = (hasVerticalScrollBar && (content.width > (bounds.width - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH) - GuiGetStyle(LISTVIEW, SCROLLBAR_WIDTH))))? true : false;
    if (!hasVerticalScrollBar) hasVerticalScrollBar = (hasHorizontalScrollBar && (content.height > (bounds.height - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH) - GuiGetStyle(LISTVIEW, SCROLLBAR_WIDTH))))? true : false;

    const int horizontalScrollBarWidth = hasHorizontalScrollBar? GuiGetStyle(LISTVIEW, SCROLLBAR_WIDTH) : 0;
    const int verticalScrollBarWidth =  hasVerticalScrollBar? GuiGetStyle(LISTVIEW, SCROLLBAR_WIDTH) : 0;
    const Rectangle horizontalScrollBar = { (float)((GuiGetStyle(LISTVIEW, SCROLLBAR_SIDE) == SCROLLBAR_LEFT_SIDE)? (float)bounds.x + verticalScrollBarWidth : (float)bounds.x) + GuiGetStyle(DEFAULT, BORDER_WIDTH), (float)bounds.y + bounds.height - horizontalScrollBarWidth - GuiGetStyle(DEFAULT, BORDER_WIDTH), (float)bounds.width - verticalScrollBarWidth - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH), (float)horizontalScrollBarWidth };
    const Rectangle verticalScrollBar = { (float)((GuiGetStyle(LISTVIEW, SCROLLBAR_SIDE) == SCROLLBAR_LEFT_SIDE)? (float)bounds.x + GuiGetStyle(DEFAULT, BORDER_WIDTH) : (float)bounds.x + bounds.width - verticalScrollBarWidth - GuiGetStyle(DEFAULT, BORDER_WIDTH)), (float)bounds.y + GuiGetStyle(DEFAULT, BORDER_WIDTH), (float)verticalScrollBarWidth, (float)bounds.height - horizontalScrollBarWidth - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH) };

    // Calculate view area (area without the scrollbars)
    Rectangle view = (GuiGetStyle(LISTVIEW, SCROLLBAR_SIDE) == SCROLLBAR_LEFT_SIDE)?
                RAYGUI_CLITERAL(Rectangle){ bounds.x + verticalScrollBarWidth + GuiGetStyle(DEFAULT, BORDER_WIDTH), bounds.y + GuiGetStyle(DEFAULT, BORDER_WIDTH), bounds.width - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH) - verticalScrollBarWidth, bounds.height - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH) - horizontalScrollBarWidth } :
                RAYGUI_CLITERAL(Rectangle){ bounds.x + GuiGetStyle(DEFAULT, BORDER_WIDTH), bounds.y + GuiGetStyle(DEFAULT, BORDER_WIDTH), bounds.width - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH) - verticalScrollBarWidth, bounds.height - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH) - horizontalScrollBarWidth };

    // Clip view area to the actual content size
    if (view.width > content.width) view.width = content.width;
    if (view.height > content.height) view.height = content.height;

    const float horizontalMin = hasHorizontalScrollBar? ((GuiGetStyle(LISTVIEW, SCROLLBAR_SIDE) == SCROLLBAR_LEFT_SIDE)? (float)-verticalScrollBarWidth : 0) - (float)GuiGetStyle(DEFAULT, BORDER_WIDTH) : (((float)GuiGetStyle(LISTVIEW, SCROLLBAR_SIDE) == SCROLLBAR_LEFT_SIDE)? (float)-verticalScrollBarWidth : 0) - (float)GuiGetStyle(DEFAULT, BORDER_WIDTH);
    const float horizontalMax = hasHorizontalScrollBar? content.width - bounds.width + (float)verticalScrollBarWidth + GuiGetStyle(DEFAULT, BORDER_WIDTH) - (((float)GuiGetStyle(LISTVIEW, SCROLLBAR_SIDE) == SCROLLBAR_LEFT_SIDE)? (float)verticalScrollBarWidth : 0) : (float)-GuiGetStyle(DEFAULT, BORDER_WIDTH);
    const float verticalMin = hasVerticalScrollBar? (float)-GuiGetStyle(DEFAULT, BORDER_WIDTH) : (float)-GuiGetStyle(DEFAULT, BORDER_WIDTH);
    const float verticalMax = hasVerticalScrollBar? content.height - bounds.height + (float)horizontalScrollBarWidth + (float)GuiGetStyle(DEFAULT, BORDER_WIDTH) : (float)-GuiGetStyle(DEFAULT, BORDER_WIDTH);

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && !guiLocked)
    {
        Vector2 mousePoint = GetMousePosition();

        // Check button state
        if (CheckCollisionPointRec(mousePoint, bounds))
        {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) state = GUI_STATE_PRESSED;
            else state = GUI_STATE_FOCUSED;

            if (hasHorizontalScrollBar)
            {
                if (IsKeyDown(KEY_RIGHT)) scrollPos.x -= GuiGetStyle(SCROLLBAR, SCROLL_SPEED);
                if (IsKeyDown(KEY_LEFT)) scrollPos.x += GuiGetStyle(SCROLLBAR, SCROLL_SPEED);
            }

            if (hasVerticalScrollBar)
            {
                if (IsKeyDown(KEY_DOWN)) scrollPos.y -= GuiGetStyle(SCROLLBAR, SCROLL_SPEED);
                if (IsKeyDown(KEY_UP)) scrollPos.y += GuiGetStyle(SCROLLBAR, SCROLL_SPEED);
            }

            float wheelMove = GetMouseWheelMove();

            // Horizontal scroll (Shift + Mouse wheel)
            if (hasHorizontalScrollBar && (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))) scrollPos.x += wheelMove*20;
            else scrollPos.y += wheelMove*20; // Vertical scroll
        }
    }

    // Normalize scroll values
    if (scrollPos.x > -horizontalMin) scrollPos.x = -horizontalMin;
    if (scrollPos.x < -horizontalMax) scrollPos.x = -horizontalMax;
    if (scrollPos.y > -verticalMin) scrollPos.y = -verticalMin;
    if (scrollPos.y < -verticalMax) scrollPos.y = -verticalMax;
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    GuiDrawRectangle(bounds, 0, BLANK, GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));        // Draw background

    // Save size of the scrollbar slider
    const int slider = GuiGetStyle(SCROLLBAR, SCROLL_SLIDER_SIZE);

    // Draw horizontal scrollbar if visible
    if (hasHorizontalScrollBar)
    {
        // Change scrollbar slider size to show the diff in size between the content width and the widget width
        GuiSetStyle(SCROLLBAR, SCROLL_SLIDER_SIZE, (int)(((bounds.width - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH) - verticalScrollBarWidth)/(int)content.width)*((int)bounds.width - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH) - verticalScrollBarWidth)));
        scrollPos.x = (float)-GuiScrollBar(horizontalScrollBar, (int)-scrollPos.x, (int)horizontalMin, (int)horizontalMax);
    }

    // Draw vertical scrollbar if visible
    if (hasVerticalScrollBar)
    {
        // Change scrollbar slider size to show the diff in size between the content height and the widget height
        GuiSetStyle(SCROLLBAR, SCROLL_SLIDER_SIZE, (int)(((bounds.height - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH) - horizontalScrollBarWidth)/(int)content.height)*((int)bounds.height - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH) - horizontalScrollBarWidth)));
        scrollPos.y = (float)-GuiScrollBar(verticalScrollBar, (int)-scrollPos.y, (int)verticalMin, (int)verticalMax);
    }

    // Draw detail corner rectangle if both scroll bars are visible
    if (hasHorizontalScrollBar && hasVerticalScrollBar)
    {
        Rectangle corner = { (GuiGetStyle(LISTVIEW, SCROLLBAR_SIDE) == SCROLLBAR_LEFT_SIDE) ? (bounds.x + GuiGetStyle(DEFAULT, BORDER_WIDTH) + 2) : (horizontalScrollBar.x + horizontalScrollBar.width + 2), verticalScrollBar.y + verticalScrollBar.height + 2, (float)horizontalScrollBarWidth - 4, (float)verticalScrollBarWidth - 4 };
        GuiDrawRectangle(corner, 0, BLANK, Fade(GetColor(GuiGetStyle(LISTVIEW, TEXT + (state*3))), guiAlpha));
    }

    // Draw scrollbar lines depending on current state
    GuiDrawRectangle(bounds, GuiGetStyle(DEFAULT, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(LISTVIEW, BORDER + (state*3))), guiAlpha), BLANK);

    // Set scrollbar slider size back to the way it was before
    GuiSetStyle(SCROLLBAR, SCROLL_SLIDER_SIZE, slider);
    //--------------------------------------------------------------------

    if (scroll != NULL) *scroll = scrollPos;

    return view;
}

// Label control
void GuiLabel(Rectangle bounds, const char *text)
{
    GuiControlState state = guiState;

    // Update control
    //--------------------------------------------------------------------
    // ...
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    GuiDrawText(text, GetTextBounds(LABEL, bounds), GuiGetStyle(LABEL, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(LABEL, (state == GUI_STATE_DISABLED)? TEXT_COLOR_DISABLED : TEXT_COLOR_NORMAL)), guiAlpha));
    //--------------------------------------------------------------------
}

// Button control, returns true when clicked
bool GuiButton(Rectangle bounds, const char *text)
{
    GuiControlState state = guiState;
    bool pressed = false;

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && !guiLocked)
    {
        Vector2 mousePoint = GetMousePosition();

        // Check button state
        if (CheckCollisionPointRec(mousePoint, bounds))
        {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) state = GUI_STATE_PRESSED;
            else state = GUI_STATE_FOCUSED;

            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) pressed = true;
        }
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    GuiDrawRectangle(bounds, GuiGetStyle(BUTTON, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(BUTTON, BORDER + (state*3))), guiAlpha), Fade(GetColor(GuiGetStyle(BUTTON, BASE + (state*3))), guiAlpha));
    GuiDrawText(text, GetTextBounds(BUTTON, bounds), GuiGetStyle(BUTTON, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(BUTTON, TEXT + (state*3))), guiAlpha));
    //------------------------------------------------------------------

    return pressed;
}

// Label button control
bool GuiLabelButton(Rectangle bounds, const char *text)
{
    GuiControlState state = guiState;
    bool pressed = false;

    // NOTE: We force bounds.width to be all text
    float textWidth = MeasureTextEx(guiFont, text, (float)GuiGetStyle(DEFAULT, TEXT_SIZE), (float)GuiGetStyle(DEFAULT, TEXT_SPACING)).x;
    if (bounds.width < textWidth) bounds.width = textWidth;

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && !guiLocked)
    {
        Vector2 mousePoint = GetMousePosition();

        // Check checkbox state
        if (CheckCollisionPointRec(mousePoint, bounds))
        {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) state = GUI_STATE_PRESSED;
            else state = GUI_STATE_FOCUSED;

            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) pressed = true;
        }
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    GuiDrawText(text, GetTextBounds(LABEL, bounds), GuiGetStyle(LABEL, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(LABEL, TEXT + (state*3))), guiAlpha));
    //--------------------------------------------------------------------

    return pressed;
}

// Toggle Button control, returns true when active
bool GuiToggle(Rectangle bounds, const char *text, bool active)
{
    GuiControlState state = guiState;

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && !guiLocked)
    {
        Vector2 mousePoint = GetMousePosition();

        // Check toggle button state
        if (CheckCollisionPointRec(mousePoint, bounds))
        {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) state = GUI_STATE_PRESSED;
            else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
            {
                state = GUI_STATE_NORMAL;
                active = !active;
            }
            else state = GUI_STATE_FOCUSED;
        }
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    if (state == GUI_STATE_NORMAL)
    {
        GuiDrawRectangle(bounds, GuiGetStyle(TOGGLE, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(TOGGLE, (active? BORDER_COLOR_PRESSED : (BORDER + state*3)))), guiAlpha), Fade(GetColor(GuiGetStyle(TOGGLE, (active? BASE_COLOR_PRESSED : (BASE + state*3)))), guiAlpha));
        GuiDrawText(text, GetTextBounds(TOGGLE, bounds), GuiGetStyle(TOGGLE, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(TOGGLE, (active? TEXT_COLOR_PRESSED : (TEXT + state*3)))), guiAlpha));
    }
    else
    {
        GuiDrawRectangle(bounds, GuiGetStyle(TOGGLE, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(TOGGLE, BORDER + state*3)), guiAlpha), Fade(GetColor(GuiGetStyle(TOGGLE, BASE + state*3)), guiAlpha));
        GuiDrawText(text, GetTextBounds(TOGGLE, bounds), GuiGetStyle(TOGGLE, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(TOGGLE, TEXT + state*3)), guiAlpha));
    }
    //--------------------------------------------------------------------

    return active;
}

// Toggle Group control, returns toggled button index
int GuiToggleGroup(Rectangle bounds, const char *text, int active)
{
    #if !defined(TOGGLEGROUP_MAX_ELEMENTS)
        #define TOGGLEGROUP_MAX_ELEMENTS    32
    #endif

    float initBoundsX = bounds.x;

    // Get substrings items from text (items pointers)
    int rows[TOGGLEGROUP_MAX_ELEMENTS] = { 0 };
    int itemCount = 0;
    const char **items = GuiTextSplit(text, &itemCount, rows);

    int prevRow = rows[0];

    for (int i = 0; i < itemCount; i++)
    {
        if (prevRow != rows[i])
        {
            bounds.x = initBoundsX;
            bounds.y += (bounds.height + GuiGetStyle(TOGGLE, GROUP_PADDING));
            prevRow = rows[i];
        }

        if (i == active) GuiToggle(bounds, items[i], true);
        else if (GuiToggle(bounds, items[i], false) == true) active = i;

        bounds.x += (bounds.width + GuiGetStyle(TOGGLE, GROUP_PADDING));
    }

    return active;
}

// Check Box control, returns true when active
bool GuiCheckBox(Rectangle bounds, const char *text, bool checked)
{
    GuiControlState state = guiState;

    Rectangle textBounds = { 0 };

    if (text != NULL)
    {
        textBounds.width = (float)GetTextWidth(text);
        textBounds.height = (float)GuiGetStyle(DEFAULT, TEXT_SIZE);
        textBounds.x = bounds.x + bounds.width + GuiGetStyle(CHECKBOX, TEXT_PADDING);
        textBounds.y = bounds.y + bounds.height/2 - GuiGetStyle(DEFAULT, TEXT_SIZE)/2;
        if (GuiGetStyle(CHECKBOX, TEXT_ALIGNMENT) == GUI_TEXT_ALIGN_LEFT) textBounds.x = bounds.x - textBounds.width - GuiGetStyle(CHECKBOX, TEXT_PADDING);
    }

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && !guiLocked)
    {
        Vector2 mousePoint = GetMousePosition();

        Rectangle totalBounds = {
            (GuiGetStyle(CHECKBOX, TEXT_ALIGNMENT) == GUI_TEXT_ALIGN_LEFT)? textBounds.x : bounds.x,
            bounds.y,
            bounds.width + textBounds.width + GuiGetStyle(CHECKBOX, TEXT_PADDING),
            bounds.height,
        };

        // Check checkbox state
        if (CheckCollisionPointRec(mousePoint, totalBounds))
        {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) state = GUI_STATE_PRESSED;
            else state = GUI_STATE_FOCUSED;

            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) checked = !checked;
        }
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    GuiDrawRectangle(bounds, GuiGetStyle(CHECKBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(CHECKBOX, BORDER + (state*3))), guiAlpha), BLANK);

    if (checked)
    {
        Rectangle check = { bounds.x + GuiGetStyle(CHECKBOX, BORDER_WIDTH) + GuiGetStyle(CHECKBOX, CHECK_PADDING),
                            bounds.y + GuiGetStyle(CHECKBOX, BORDER_WIDTH) + GuiGetStyle(CHECKBOX, CHECK_PADDING),
                            bounds.width - 2*(GuiGetStyle(CHECKBOX, BORDER_WIDTH) + GuiGetStyle(CHECKBOX, CHECK_PADDING)),
                            bounds.height - 2*(GuiGetStyle(CHECKBOX, BORDER_WIDTH) + GuiGetStyle(CHECKBOX, CHECK_PADDING)) };
        GuiDrawRectangle(check, 0, BLANK, Fade(GetColor(GuiGetStyle(CHECKBOX, TEXT + state*3)), guiAlpha));
    }

    GuiDrawText(text, textBounds, (GuiGetStyle(CHECKBOX, TEXT_ALIGNMENT) == GUI_TEXT_ALIGN_RIGHT)? GUI_TEXT_ALIGN_LEFT : GUI_TEXT_ALIGN_RIGHT, Fade(GetColor(GuiGetStyle(LABEL, TEXT + (state*3))), guiAlpha));
    //--------------------------------------------------------------------

    return checked;
}

// Combo Box control, returns selected item index
int GuiComboBox(Rectangle bounds, const char *text, int active)
{
    GuiControlState state = guiState;

    bounds.width -= (GuiGetStyle(COMBOBOX, COMBO_BUTTON_WIDTH) + GuiGetStyle(COMBOBOX, COMBO_BUTTON_PADDING));

    Rectangle selector = { (float)bounds.x + bounds.width + GuiGetStyle(COMBOBOX, COMBO_BUTTON_PADDING),
                           (float)bounds.y, (float)GuiGetStyle(COMBOBOX, COMBO_BUTTON_WIDTH), (float)bounds.height };

    // Get substrings items from text (items pointers, lengths and count)
    int itemCount = 0;
    const char **items = GuiTextSplit(text, &itemCount, NULL);

    if (active < 0) active = 0;
    else if (active > itemCount - 1) active = itemCount - 1;

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && !guiLocked && (itemCount > 1))
    {
        Vector2 mousePoint = GetMousePosition();

        if (CheckCollisionPointRec(mousePoint, bounds) ||
            CheckCollisionPointRec(mousePoint, selector))
        {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                active += 1;
                if (active >= itemCount) active = 0;
            }

            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) state = GUI_STATE_PRESSED;
            else state = GUI_STATE_FOCUSED;
        }
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    // Draw combo box main
    GuiDrawRectangle(bounds, GuiGetStyle(COMBOBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(COMBOBOX, BORDER + (state*3))), guiAlpha), Fade(GetColor(GuiGetStyle(COMBOBOX, BASE + (state*3))), guiAlpha));
    GuiDrawText(items[active], GetTextBounds(COMBOBOX, bounds), GuiGetStyle(COMBOBOX, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(COMBOBOX, TEXT + (state*3))), guiAlpha));

    // Draw selector using a custom button
    // NOTE: BORDER_WIDTH and TEXT_ALIGNMENT forced values
    int tempBorderWidth = GuiGetStyle(BUTTON, BORDER_WIDTH);
    int tempTextAlign = GuiGetStyle(BUTTON, TEXT_ALIGNMENT);
    GuiSetStyle(BUTTON, BORDER_WIDTH, 1);
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_CENTER);

    GuiButton(selector, TextFormat("%i/%i", active + 1, itemCount));

    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, tempTextAlign);
    GuiSetStyle(BUTTON, BORDER_WIDTH, tempBorderWidth);
    //--------------------------------------------------------------------

    return active;
}

// Dropdown Box control
// NOTE: Returns mouse click
bool GuiDropdownBox(Rectangle bounds, const char *text, int *active, bool editMode)
{
    GuiControlState state = guiState;
    int itemSelected = *active;
    int itemFocused = -1;

    // Get substrings items from text (items pointers, lengths and count)
    int itemCount = 0;
    const char **items = GuiTextSplit(text, &itemCount, NULL);

    Rectangle boundsOpen = bounds;
    boundsOpen.height = (itemCount + 1)*(bounds.height + GuiGetStyle(DROPDOWNBOX, DROPDOWN_ITEMS_PADDING));

    Rectangle itemBounds = bounds;

    bool pressed = false;       // Check mouse button pressed

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && (editMode || !guiLocked) && (itemCount > 1))
    {
        Vector2 mousePoint = GetMousePosition();

        if (editMode)
        {
            state = GUI_STATE_PRESSED;

            // Check if mouse has been pressed or released outside limits
            if (!CheckCollisionPointRec(mousePoint, boundsOpen))
            {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) pressed = true;
            }

            // Check if already selected item has been pressed again
            if (CheckCollisionPointRec(mousePoint, bounds) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) pressed = true;

            // Check focused and selected item
            for (int i = 0; i < itemCount; i++)
            {
                // Update item rectangle y position for next item
                itemBounds.y += (bounds.height + GuiGetStyle(DROPDOWNBOX, DROPDOWN_ITEMS_PADDING));

                if (CheckCollisionPointRec(mousePoint, itemBounds))
                {
                    itemFocused = i;
                    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
                    {
                        itemSelected = i;
                        pressed = true;     // Item selected, change to editMode = false
                    }
                    break;
                }
            }

            itemBounds = bounds;
        }
        else
        {
            if (CheckCollisionPointRec(mousePoint, bounds))
            {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                {
                    pressed = true;
                    state = GUI_STATE_PRESSED;
                }
                else state = GUI_STATE_FOCUSED;
            }
        }
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    if (editMode) GuiPanel(boundsOpen);

    GuiDrawRectangle(bounds, GuiGetStyle(DROPDOWNBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(DROPDOWNBOX, BORDER + state*3)), guiAlpha), Fade(GetColor(GuiGetStyle(DROPDOWNBOX, BASE + state*3)), guiAlpha));
    GuiDrawText(items[itemSelected], GetTextBounds(DEFAULT, bounds), GuiGetStyle(DROPDOWNBOX, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(DROPDOWNBOX, TEXT + state*3)), guiAlpha));

    if (editMode)
    {
        // Draw visible items
        for (int i = 0; i < itemCount; i++)
        {
            // Update item rectangle y position for next item
            itemBounds.y += (bounds.height + GuiGetStyle(DROPDOWNBOX, DROPDOWN_ITEMS_PADDING));

            if (i == itemSelected)
            {
                GuiDrawRectangle(itemBounds, GuiGetStyle(DROPDOWNBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(DROPDOWNBOX, BORDER_COLOR_PRESSED)), guiAlpha), Fade(GetColor(GuiGetStyle(DROPDOWNBOX, BASE_COLOR_PRESSED)), guiAlpha));
                GuiDrawText(items[i], GetTextBounds(DEFAULT, itemBounds), GuiGetStyle(DROPDOWNBOX, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(DROPDOWNBOX, TEXT_COLOR_PRESSED)), guiAlpha));
            }
            else if (i == itemFocused)
            {
                GuiDrawRectangle(itemBounds, GuiGetStyle(DROPDOWNBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(DROPDOWNBOX, BORDER_COLOR_FOCUSED)), guiAlpha), Fade(GetColor(GuiGetStyle(DROPDOWNBOX, BASE_COLOR_FOCUSED)), guiAlpha));
                GuiDrawText(items[i], GetTextBounds(DEFAULT, itemBounds), GuiGetStyle(DROPDOWNBOX, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(DROPDOWNBOX, TEXT_COLOR_FOCUSED)), guiAlpha));
            }
            else GuiDrawText(items[i], GetTextBounds(DEFAULT, itemBounds), GuiGetStyle(DROPDOWNBOX, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(DROPDOWNBOX, TEXT_COLOR_NORMAL)), guiAlpha));
        }
    }

    // Draw arrows (using icon if available)
#if defined(RAYGUI_NO_RICONS)
    GuiDrawText("v", RAYGUI_CLITERAL(Rectangle){ bounds.x + bounds.width - GuiGetStyle(DROPDOWNBOX, ARROW_PADDING), bounds.y + bounds.height/2 - 2, 10, 10 },
                GUI_TEXT_ALIGN_CENTER, Fade(GetColor(GuiGetStyle(DROPDOWNBOX, TEXT + (state*3))), guiAlpha));
#else
    GuiDrawText("#120#", RAYGUI_CLITERAL(Rectangle){ bounds.x + bounds.width - GuiGetStyle(DROPDOWNBOX, ARROW_PADDING), bounds.y + bounds.height/2 - 6, 10, 10 },
                GUI_TEXT_ALIGN_CENTER, Fade(GetColor(GuiGetStyle(DROPDOWNBOX, TEXT + (state*3))), guiAlpha));   // RICON_ARROW_DOWN_FILL
#endif
    //--------------------------------------------------------------------

    *active = itemSelected;
    return pressed;
}
/*
// Encode codepoint into utf8 text (char array length returned as parameter)
 const char *CodepointToUtf8(int codepoint, int *byteLength)
{
    static char utf8[6] = { 0 };
    int length = 0;

    if (codepoint <= 0x7f)
    {
        utf8[0] = (char)codepoint;
        length = 1;
    }
    else if (codepoint <= 0x7ff)
    {
        utf8[0] = (char)(((codepoint >> 6) & 0x1f) | 0xc0);
        utf8[1] = (char)((codepoint & 0x3f) | 0x80);
        length = 2;
    }
    else if (codepoint <= 0xffff)
    {
        utf8[0] = (char)(((codepoint >> 12) & 0x0f) | 0xe0);
        utf8[1] = (char)(((codepoint >>  6) & 0x3f) | 0x80);
        utf8[2] = (char)((codepoint & 0x3f) | 0x80);
        length = 3;
    }
    else if (codepoint <= 0x10ffff)
    {
        utf8[0] = (char)(((codepoint >> 18) & 0x07) | 0xf0);
        utf8[1] = (char)(((codepoint >> 12) & 0x3f) | 0x80);
        utf8[2] = (char)(((codepoint >>  6) & 0x3f) | 0x80);
        utf8[3] = (char)((codepoint & 0x3f) | 0x80);
        length = 4;
    }

    *byteLength = length;

    return utf8;
}



// Encode codepoint into utf8 text (char array length returned as parameter)
// NOTE: It uses a static array to store UTF-8 bytes
 const char *CodepointToUTF8(int codepoint, int *byteSize)
{
    static char utf8[6] = { 0 };
    int size = 0;   // Byte size of codepoint

    if (codepoint <= 0x7f)
    {
        utf8[0] = (char)codepoint;
        size = 1;
    }
    else if (codepoint <= 0x7ff)
    {
        utf8[0] = (char)(((codepoint >> 6) & 0x1f) | 0xc0);
        utf8[1] = (char)((codepoint & 0x3f) | 0x80);
        size = 2;
    }
    else if (codepoint <= 0xffff)
    {
        utf8[0] = (char)(((codepoint >> 12) & 0x0f) | 0xe0);
        utf8[1] = (char)(((codepoint >>  6) & 0x3f) | 0x80);
        utf8[2] = (char)((codepoint & 0x3f) | 0x80);
        size = 3;
    }
    else if (codepoint <= 0x10ffff)
    {
        utf8[0] = (char)(((codepoint >> 18) & 0x07) | 0xf0);
        utf8[1] = (char)(((codepoint >> 12) & 0x3f) | 0x80);
        utf8[2] = (char)(((codepoint >>  6) & 0x3f) | 0x80);
        utf8[3] = (char)((codepoint & 0x3f) | 0x80);
        size = 4;
    }

    *byteSize = size;

    return utf8;
}
*/
// Text Box control, updates input text
// NOTE 2: Returns if KEY_ENTER pressed (useful for data validation)
bool GuiTextBox(Rectangle bounds, char *text, int textSize, bool editMode)
{
    GuiControlState state = guiState;
    bool pressed = false;

    Rectangle cursor = {
        bounds.x + GuiGetStyle(TEXTBOX, TEXT_PADDING) + GetTextWidth(text) + 2,
        bounds.y + bounds.height/2 - GuiGetStyle(DEFAULT, TEXT_SIZE),
        4,
        (float)GuiGetStyle(DEFAULT, TEXT_SIZE)*2
    };

    if (cursor.height > bounds.height) cursor.height = bounds.height - GuiGetStyle(TEXTBOX, BORDER_WIDTH)*2;

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && !guiLocked)
    {
        Vector2 mousePoint = GetMousePosition();

        if (editMode)
        {
            state = GUI_STATE_PRESSED;

            int key = GetCharPressed();      // Returns codepoint as Unicode

            int keyCount = (int)strlen(text);

            SDL_Log("%d %d",key,keyCount);

            // Only allow keys in range [32..125]
            if (keyCount < (textSize - 1))
            {
                float maxWidth = (bounds.width - (GuiGetStyle(TEXTBOX, TEXT_INNER_PADDING)*2));

                if ((GetTextWidth(text) < (maxWidth - GuiGetStyle(DEFAULT, TEXT_SIZE))) && (key >= 32))
                {
                    int byteSize = 0;
                    const char *textUTF8 = CodepointToUtf8(key, &byteSize);

                    for (int i = 0; i < byteSize; i++)
                    {
                        text[keyCount] = textUTF8[i];
                        keyCount++;
                    }

                    text[keyCount] = '\0';
                }
            }

            // Delete text
            if (keyCount > 0)
            {
                if (IsKeyPressed(KEY_BACKSPACE))
                {
                    keyCount--;
                    text[keyCount] = '\0';
                    if (keyCount < 0) keyCount = 0;
                }
            }

            if (IsKeyPressed(KEY_ENTER) || (!CheckCollisionPointRec(mousePoint, bounds) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))) pressed = true;

            // Check text alignment to position cursor properly
            int textAlignment = GuiGetStyle(TEXTBOX, TEXT_ALIGNMENT);
            if (textAlignment == GUI_TEXT_ALIGN_CENTER) cursor.x = bounds.x + GetTextWidth(text)/2 + bounds.width/2 + 1;
            else if (textAlignment == GUI_TEXT_ALIGN_RIGHT) cursor.x = bounds.x + bounds.width - GuiGetStyle(TEXTBOX, TEXT_INNER_PADDING);
        }
        else
        {
            if (CheckCollisionPointRec(mousePoint, bounds))
            {
                state = GUI_STATE_FOCUSED;
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) pressed = true;
            }
        }
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    if (state == GUI_STATE_PRESSED)
    {
        GuiDrawRectangle(bounds, GuiGetStyle(TEXTBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(TEXTBOX, BORDER + (state*3))), guiAlpha), Fade(GetColor(GuiGetStyle(TEXTBOX, BASE_COLOR_PRESSED)), guiAlpha));
    }
    else if (state == GUI_STATE_DISABLED)
    {
        GuiDrawRectangle(bounds, GuiGetStyle(TEXTBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(TEXTBOX, BORDER + (state*3))), guiAlpha), Fade(GetColor(GuiGetStyle(TEXTBOX, BASE_COLOR_DISABLED)), guiAlpha));
    }
    else GuiDrawRectangle(bounds, 1, Fade(GetColor(GuiGetStyle(TEXTBOX, BORDER + (state*3))), guiAlpha), BLANK);

    GuiDrawText(text, GetTextBounds(TEXTBOX, bounds), GuiGetStyle(TEXTBOX, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(TEXTBOX, TEXT + (state*3))), guiAlpha));

    // Draw cursor
    if (editMode) GuiDrawRectangle(cursor, 0, BLANK, Fade(GetColor(GuiGetStyle(TEXTBOX, BORDER_COLOR_PRESSED)), guiAlpha));
    //--------------------------------------------------------------------

    return pressed;
}

// Spinner control, returns selected value
bool GuiSpinner(Rectangle bounds, const char *text, int *value, int minValue, int maxValue, bool editMode)
{
    GuiControlState state = guiState;

    bool pressed = false;
    int tempValue = *value;

    Rectangle spinner = { bounds.x + GuiGetStyle(SPINNER, SPIN_BUTTON_WIDTH) + GuiGetStyle(SPINNER, SPIN_BUTTON_PADDING), bounds.y,
                          bounds.width - 2*(GuiGetStyle(SPINNER, SPIN_BUTTON_WIDTH) + GuiGetStyle(SPINNER, SPIN_BUTTON_PADDING)), bounds.height };
    Rectangle leftButtonBound = { (float)bounds.x, (float)bounds.y, (float)GuiGetStyle(SPINNER, SPIN_BUTTON_WIDTH), (float)bounds.height };
    Rectangle rightButtonBound = { (float)bounds.x + bounds.width - GuiGetStyle(SPINNER, SPIN_BUTTON_WIDTH), (float)bounds.y, (float)GuiGetStyle(SPINNER, SPIN_BUTTON_WIDTH), (float)bounds.height };

    Rectangle textBounds = { 0 };
    if (text != NULL)
    {
        textBounds.width = (float)GetTextWidth(text);
        textBounds.height = (float)GuiGetStyle(DEFAULT, TEXT_SIZE);
        textBounds.x = bounds.x + bounds.width + GuiGetStyle(SPINNER, TEXT_PADDING);
        textBounds.y = bounds.y + bounds.height/2 - GuiGetStyle(DEFAULT, TEXT_SIZE)/2;
        if (GuiGetStyle(SPINNER, TEXT_ALIGNMENT) == GUI_TEXT_ALIGN_LEFT) textBounds.x = bounds.x - textBounds.width - GuiGetStyle(SPINNER, TEXT_PADDING);
    }

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && !guiLocked)
    {
        Vector2 mousePoint = GetMousePosition();

        // Check spinner state
        if (CheckCollisionPointRec(mousePoint, bounds))
        {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) state = GUI_STATE_PRESSED;
            else state = GUI_STATE_FOCUSED;
        }
    }

    if (!editMode)
    {
        if (tempValue < minValue) tempValue = minValue;
        if (tempValue > maxValue) tempValue = maxValue;
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    // TODO: Set Spinner properties for ValueBox
    pressed = GuiValueBox(spinner, NULL, &tempValue, minValue, maxValue, editMode);

    // Draw value selector custom buttons
    // NOTE: BORDER_WIDTH and TEXT_ALIGNMENT forced values
    int tempBorderWidth = GuiGetStyle(BUTTON, BORDER_WIDTH);
    int tempTextAlign = GuiGetStyle(BUTTON, TEXT_ALIGNMENT);
    GuiSetStyle(BUTTON, BORDER_WIDTH, GuiGetStyle(SPINNER, BORDER_WIDTH));
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_CENTER);

#if defined(RAYGUI_NO_RICONS)
    if (GuiButton(leftButtonBound, "<")) tempValue--;
    if (GuiButton(rightButtonBound, ">")) tempValue++;
#else
    if (GuiButton(leftButtonBound, GuiIconText(RICON_ARROW_LEFT_FILL, NULL))) tempValue--;
    if (GuiButton(rightButtonBound, GuiIconText(RICON_ARROW_RIGHT_FILL, NULL))) tempValue++;
#endif

    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, tempTextAlign);
    GuiSetStyle(BUTTON, BORDER_WIDTH, tempBorderWidth);

    // Draw text label if provided
    GuiDrawText(text, textBounds, (GuiGetStyle(SPINNER, TEXT_ALIGNMENT) == GUI_TEXT_ALIGN_RIGHT)? GUI_TEXT_ALIGN_LEFT : GUI_TEXT_ALIGN_RIGHT, Fade(GetColor(GuiGetStyle(LABEL, TEXT + (state*3))), guiAlpha));
    //--------------------------------------------------------------------

    *value = tempValue;
    return pressed;
}

// Value Box control, updates input text with numbers
// NOTE: Requires static variables: frameCounter
bool GuiValueBox(Rectangle bounds, const char *text, int *value, int minValue, int maxValue, bool editMode)
{
    #if !defined(VALUEBOX_MAX_CHARS)
        #define VALUEBOX_MAX_CHARS  32
    #endif

    GuiControlState state = guiState;
    bool pressed = false;

    char textValue[VALUEBOX_MAX_CHARS + 1] = "\0";
    sprintf(textValue, "%i", *value);

    Rectangle textBounds = { 0 };
    if (text != NULL)
    {
        textBounds.width = (float)GetTextWidth(text);
        textBounds.height = (float)GuiGetStyle(DEFAULT, TEXT_SIZE);
        textBounds.x = bounds.x + bounds.width + GuiGetStyle(VALUEBOX, TEXT_PADDING);
        textBounds.y = bounds.y + bounds.height/2 - GuiGetStyle(DEFAULT, TEXT_SIZE)/2;
        if (GuiGetStyle(VALUEBOX, TEXT_ALIGNMENT) == GUI_TEXT_ALIGN_LEFT) textBounds.x = bounds.x - textBounds.width - GuiGetStyle(VALUEBOX, TEXT_PADDING);
    }

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && !guiLocked)
    {
        Vector2 mousePoint = GetMousePosition();

        bool valueHasChanged = false;

        if (editMode)
        {
            state = GUI_STATE_PRESSED;

            int keyCount = (int)strlen(textValue);

            // Only allow keys in range [48..57]
            if (keyCount < VALUEBOX_MAX_CHARS)
            {
                if (GetTextWidth(textValue) < bounds.width)
                {
                    int key = GetCharPressed();
                    if ((key >= 48) && (key <= 57))
                    {
                        textValue[keyCount] = (char)key;
                        keyCount++;
                        valueHasChanged = true;
                    }
                }
            }

            // Delete text
            if (keyCount > 0)
            {
                if (IsKeyPressed(KEY_BACKSPACE))
                {
                    keyCount--;
                    textValue[keyCount] = '\0';
                    if (keyCount < 0) keyCount = 0;
                    valueHasChanged = true;
                }
            }

            if (valueHasChanged) *value = TextToInteger(textValue);

            if (IsKeyPressed(KEY_ENTER) || (!CheckCollisionPointRec(mousePoint, bounds) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))) pressed = true;
        }
        else
        {
            if (*value > maxValue) *value = maxValue;
            else if (*value < minValue) *value = minValue;

            if (CheckCollisionPointRec(mousePoint, bounds))
            {
                state = GUI_STATE_FOCUSED;
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) pressed = true;
            }
        }
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    Color baseColor = BLANK;
    if (state == GUI_STATE_PRESSED) baseColor = GetColor(GuiGetStyle(VALUEBOX, BASE_COLOR_PRESSED));
    else if (state == GUI_STATE_DISABLED) baseColor = GetColor(GuiGetStyle(VALUEBOX, BASE_COLOR_DISABLED));

    // WARNING: BLANK color does not work properly with Fade()
    GuiDrawRectangle(bounds, GuiGetStyle(VALUEBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(VALUEBOX, BORDER + (state*3))), guiAlpha), baseColor);
    GuiDrawText(textValue, GetTextBounds(VALUEBOX, bounds), GUI_TEXT_ALIGN_CENTER, Fade(GetColor(GuiGetStyle(VALUEBOX, TEXT + (state*3))), guiAlpha));

    // Draw cursor
    if (editMode)
    {
        // NOTE: ValueBox internal text is always centered
        Rectangle cursor = { bounds.x + GetTextWidth(textValue)/2 + bounds.width/2 + 2, bounds.y + 2*GuiGetStyle(VALUEBOX, BORDER_WIDTH), 4, bounds.height - 4*GuiGetStyle(VALUEBOX, BORDER_WIDTH) };
        GuiDrawRectangle(cursor, 0, BLANK, Fade(GetColor(GuiGetStyle(VALUEBOX, BORDER_COLOR_PRESSED)), guiAlpha));
    }

    // Draw text label if provided
    GuiDrawText(text, textBounds, (GuiGetStyle(VALUEBOX, TEXT_ALIGNMENT) == GUI_TEXT_ALIGN_RIGHT)? GUI_TEXT_ALIGN_LEFT : GUI_TEXT_ALIGN_RIGHT, Fade(GetColor(GuiGetStyle(LABEL, TEXT + (state*3))), guiAlpha));
    //--------------------------------------------------------------------

    return pressed;
}

// Get next codepoint in a UTF-8 encoded text, scanning until '\0' is found
// When a invalid UTF-8 byte is encountered we exit as soon as possible and a '?'(0x3f) codepoint is returned
// Total number of bytes processed are returned as a parameter
// NOTE: The standard says U+FFFD should be returned in case of errors
// but that character is not supported by the default font in raylib
int GetCodepoint(const char *text, int *bytesProcessed)
{
/*
    UTF-8 specs from https://www.ietf.org/rfc/rfc3629.txt

    Char. number range  |        UTF-8 octet sequence
      (hexadecimal)    |              (binary)
    --------------------+---------------------------------------------
    0000 0000-0000 007F | 0xxxxxxx
    0000 0080-0000 07FF | 110xxxxx 10xxxxxx
    0000 0800-0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx
    0001 0000-0010 FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
*/
    // NOTE: on decode errors we return as soon as possible

    int code = 0x3f;   // Codepoint (defaults to '?')
    int octet = (unsigned char)(text[0]); // The first UTF8 octet
    *bytesProcessed = 1;

    if (octet <= 0x7f)
    {
        // Only one octet (ASCII range x00-7F)
        code = text[0];
    }
    else if ((octet & 0xe0) == 0xc0)
    {
        // Two octets

        // [0]xC2-DF    [1]UTF8-tail(x80-BF)
        unsigned char octet1 = text[1];

        if ((octet1 == '\0') || ((octet1 >> 6) != 2)) { *bytesProcessed = 2; return code; } // Unexpected sequence

        if ((octet >= 0xc2) && (octet <= 0xdf))
        {
            code = ((octet & 0x1f) << 6) | (octet1 & 0x3f);
            *bytesProcessed = 2;
        }
    }
    else if ((octet & 0xf0) == 0xe0)
    {
        // Three octets
        unsigned char octet1 = text[1];
        unsigned char octet2 = '\0';

        if ((octet1 == '\0') || ((octet1 >> 6) != 2)) { *bytesProcessed = 2; return code; } // Unexpected sequence

        octet2 = text[2];

        if ((octet2 == '\0') || ((octet2 >> 6) != 2)) { *bytesProcessed = 3; return code; } // Unexpected sequence

        // [0]xE0    [1]xA0-BF       [2]UTF8-tail(x80-BF)
        // [0]xE1-EC [1]UTF8-tail    [2]UTF8-tail(x80-BF)
        // [0]xED    [1]x80-9F       [2]UTF8-tail(x80-BF)
        // [0]xEE-EF [1]UTF8-tail    [2]UTF8-tail(x80-BF)

        if (((octet == 0xe0) && !((octet1 >= 0xa0) && (octet1 <= 0xbf))) ||
            ((octet == 0xed) && !((octet1 >= 0x80) && (octet1 <= 0x9f)))) { *bytesProcessed = 2; return code; }

        if ((octet >= 0xe0) && (0 <= 0xef))
        {
            code = ((octet & 0xf) << 12) | ((octet1 & 0x3f) << 6) | (octet2 & 0x3f);
            *bytesProcessed = 3;
        }
    }
    else if ((octet & 0xf8) == 0xf0)
    {
        // Four octets
        if (octet > 0xf4) return code;

        unsigned char octet1 = text[1];
        unsigned char octet2 = '\0';
        unsigned char octet3 = '\0';

        if ((octet1 == '\0') || ((octet1 >> 6) != 2)) { *bytesProcessed = 2; return code; }  // Unexpected sequence

        octet2 = text[2];

        if ((octet2 == '\0') || ((octet2 >> 6) != 2)) { *bytesProcessed = 3; return code; }  // Unexpected sequence

        octet3 = text[3];

        if ((octet3 == '\0') || ((octet3 >> 6) != 2)) { *bytesProcessed = 4; return code; }  // Unexpected sequence

        // [0]xF0       [1]x90-BF       [2]UTF8-tail  [3]UTF8-tail
        // [0]xF1-F3    [1]UTF8-tail    [2]UTF8-tail  [3]UTF8-tail
        // [0]xF4       [1]x80-8F       [2]UTF8-tail  [3]UTF8-tail

        if (((octet == 0xf0) && !((octet1 >= 0x90) && (octet1 <= 0xbf))) ||
            ((octet == 0xf4) && !((octet1 >= 0x80) && (octet1 <= 0x8f)))) { *bytesProcessed = 2; return code; } // Unexpected sequence

        if (octet >= 0xf0)
        {
            code = ((octet & 0x7) << 18) | ((octet1 & 0x3f) << 12) | ((octet2 & 0x3f) << 6) | (octet3 & 0x3f);
            *bytesProcessed = 4;
        }
    }

    if (code > 0x10ffff) code = 0x3f;     // Codepoints after U+10ffff are invalid

    return code;
}


// Encode codepoint into utf8 text (char array length returned as parameter)
// NOTE: It uses a static array to store UTF-8 bytes
 const char *CodepointToUTF8(int codepoint, int *byteSize)
{
    static char utf8[6] = { 0 };
    int size = 0;   // Byte size of codepoint

    if (codepoint <= 0x7f)
    {
        utf8[0] = (char)codepoint;
        size = 1;
    }
    else if (codepoint <= 0x7ff)
    {
        utf8[0] = (char)(((codepoint >> 6) & 0x1f) | 0xc0);
        utf8[1] = (char)((codepoint & 0x3f) | 0x80);
        size = 2;
    }
    else if (codepoint <= 0xffff)
    {
        utf8[0] = (char)(((codepoint >> 12) & 0x0f) | 0xe0);
        utf8[1] = (char)(((codepoint >>  6) & 0x3f) | 0x80);
        utf8[2] = (char)((codepoint & 0x3f) | 0x80);
        size = 3;
    }
    else if (codepoint <= 0x10ffff)
    {
        utf8[0] = (char)(((codepoint >> 18) & 0x07) | 0xf0);
        utf8[1] = (char)(((codepoint >> 12) & 0x3f) | 0x80);
        utf8[2] = (char)(((codepoint >>  6) & 0x3f) | 0x80);
        utf8[3] = (char)((codepoint & 0x3f) | 0x80);
        size = 4;
    }

    *byteSize = size;

    return utf8;
}


// Encode text codepoint into UTF-8 text
// REQUIRES: memcpy()
// WARNING: Allocated memory should be manually freed
char *TextCodepointsToUTF8(int *codepoints, int length)
{
    // We allocate enough memory fo fit all possible codepoints
    // NOTE: 5 bytes for every codepoint should be enough
    char *text = (char *)RL_CALLOC(length*5, 1);
    const char *utf8 = NULL;
    int size = 0;

    for (int i = 0, bytes = 0; i < length; i++)
    {
        utf8 = CodepointToUTF8(codepoints[i], &bytes);
        memcpy(text + size, utf8, bytes);
        size += bytes;
    }

    // Resize memory to text length + string NULL terminator
    void *ptr = RL_REALLOC(text, size + 1);

    if (ptr != NULL) text = (char *)ptr;

    return text;
}

// Text Box control with multiple lines
bool GuiTextBoxMulti(Rectangle bounds, char *text, int textSize, bool editMode)
{
     GuiControlState state = guiState;
    bool pressed = false;

    Rectangle textAreaBounds = {
        bounds.x + GuiGetStyle(TEXTBOX, BORDER_WIDTH) + GuiGetStyle(TEXTBOX, TEXT_INNER_PADDING),
        bounds.y + GuiGetStyle(TEXTBOX, BORDER_WIDTH) + GuiGetStyle(TEXTBOX, TEXT_INNER_PADDING),
        bounds.width - 2*(GuiGetStyle(TEXTBOX, BORDER_WIDTH) + GuiGetStyle(TEXTBOX, TEXT_INNER_PADDING)),
        bounds.height - 2*(GuiGetStyle(TEXTBOX, BORDER_WIDTH) + GuiGetStyle(TEXTBOX, TEXT_INNER_PADDING))
    };

    // Cursor position, [x, y] values should be updated
    Rectangle cursor = { 0, -1, 4, (float)GuiGetStyle(DEFAULT, TEXT_SIZE) + 2 };

    float scaleFactor = (float)GuiGetStyle(DEFAULT, TEXT_SIZE)/(float)guiFont.baseSize;     // Character rectangle scaling factor

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && !guiLocked)
    {
        Vector2 mousePoint = GetMousePosition();

        if (editMode)
        {
            state = GUI_STATE_PRESSED;

            // We get an Unicode codepoint
            int codepoint = GetCharPressed();
            int textLength = (int)strlen(text);     // Length in bytes (UTF-8 string)


            // Introduce characters
            if (textLength < (textSize - 1))
            {
                if (IsKeyPressed(KEY_ENTER))
                {
                    text[textLength] = '\n';
                    textLength++;
                }
                else if (codepoint >= 32)
                {
                    // Supports Unicode inputs -> Encoded to UTF-8
                    int charUTF8Length = 0;
                    const char *charEncoded = CodepointToUTF8(codepoint, &charUTF8Length);
                    memcpy(text + textLength, charEncoded, charUTF8Length);
                    textLength += charUTF8Length;
                }
            }

            // Delete characters
            if (textLength > 0)
            {
                if (IsKeyPressed(KEY_BACKSPACE))
                {
                    if ((unsigned char)text[textLength - 1] < 127)
                    {
                        // Remove ASCII equivalent character (1 byte)
                        textLength--;
                        text[textLength] = '\0';
                    }
                    else
                    {
                        // Remove latest UTF-8 unicode character introduced (n bytes)
                        int charUTF8Length = 0;
                        while (((unsigned char)text[textLength - 1 - charUTF8Length] & 0b01000000) == 0) charUTF8Length++;

                        textLength -= (charUTF8Length + 1);
                        text[textLength] = '\0';
                    }
                }
            }

            // Exit edit mode
            if (!CheckCollisionPointRec(mousePoint, bounds) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) pressed = true;
        }
        else
        {
            if (CheckCollisionPointRec(mousePoint, bounds))
            {
                state = GUI_STATE_FOCUSED;
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) pressed = true;
            }
        }
    }
    //--------------------------------------------------------------------


    // Draw control
    //--------------------------------------------------------------------
    if (state == GUI_STATE_PRESSED)
    {
        GuiDrawRectangle(bounds, GuiGetStyle(TEXTBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(TEXTBOX, BORDER + (state*3))), guiAlpha), Fade(GetColor(GuiGetStyle(TEXTBOX, BASE_COLOR_PRESSED)), guiAlpha));
    }
    else if (state == GUI_STATE_DISABLED)
    {
        GuiDrawRectangle(bounds, GuiGetStyle(TEXTBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(TEXTBOX, BORDER + (state*3))), guiAlpha), Fade(GetColor(GuiGetStyle(TEXTBOX, BASE_COLOR_DISABLED)), guiAlpha));
    }
    else GuiDrawRectangle(bounds, 1, Fade(GetColor(GuiGetStyle(TEXTBOX, BORDER + (state*3))), guiAlpha), BLANK);

    int wrapMode = 1;      // 0-No wrap, 1-Char wrap, 2-Word wrap
    Vector2 cursorPos = { textAreaBounds.x, textAreaBounds.y };




    for (int i = 0, codepointLength = 0; text[i] != '\0'; i += codepointLength)
    {

        int codepoint = GetCodepoint(text + i, &codepointLength);
        int index = GetGlyphIndex(guiFont, codepoint);      // If requested codepoint is not found, we get '?' (0x3f)



        Rectangle atlasRec = guiFont.recs[index];
        CharInfo glyphInfo =   guiFont.chars[index];        // Glyph measures



        if ((codepointLength == 1) && (codepoint == '\n'))
        {
            cursorPos.y += (guiFont.baseSize*scaleFactor + GuiGetStyle(TEXTBOX, TEXT_LINES_PADDING));   // Line feed
            cursorPos.x = textAreaBounds.x;                 // Carriage return
        }
        else
        {
            if (wrapMode == 1)
            {
                int glyphWidth = 0;
                if (glyphInfo.advanceX != 0) glyphWidth += glyphInfo.advanceX;
                else glyphWidth += (atlasRec.width + glyphInfo.offsetX);

                // Jump line if the end of the text box area has been reached
                if ((cursorPos.x + (glyphWidth*scaleFactor)) > (textAreaBounds.x + textAreaBounds.width))
                {
                    cursorPos.y += (guiFont.baseSize*scaleFactor + GuiGetStyle(TEXTBOX, TEXT_LINES_PADDING));   // Line feed
                    cursorPos.x = textAreaBounds.x;     // Carriage return
                }
            }

            // Draw current character glyph
            DrawTextCodepoint(guiFont, codepoint, cursorPos, (float)GuiGetStyle(DEFAULT, TEXT_SIZE), Fade(GetColor(GuiGetStyle(TEXTBOX, TEXT + (state*3))), guiAlpha));

            int glyphWidth = 0;
            if (glyphInfo.advanceX != 0) glyphWidth += glyphInfo.advanceX;
            else glyphWidth += (atlasRec.width + glyphInfo.offsetX);

            cursorPos.x += (glyphWidth*scaleFactor + (float)GuiGetStyle(DEFAULT, TEXT_SPACING));
        }

    }

    cursor.x = cursorPos.x;
    cursor.y = cursorPos.y;

    // Draw cursor position considering text glyphs
    if (editMode) GuiDrawRectangle(cursor, 0, BLANK, Fade(GetColor(GuiGetStyle(TEXTBOX, BORDER_COLOR_PRESSED)), guiAlpha));
    //--------------------------------------------------------------------



    return pressed;
}

// Slider control with pro parameters
// NOTE: Other GuiSlider*() controls use this one
float GuiSliderPro(Rectangle bounds, const char *textLeft, const char *textRight, float value, float minValue, float maxValue, int sliderWidth)
{
    GuiControlState state = guiState;

    int sliderValue = (int)(((value - minValue)/(maxValue - minValue))*(bounds.width - 2*GuiGetStyle(SLIDER, BORDER_WIDTH)));

    Rectangle slider = { bounds.x, bounds.y + GuiGetStyle(SLIDER, BORDER_WIDTH) + GuiGetStyle(SLIDER, SLIDER_PADDING),
                         0, bounds.height - 2*GuiGetStyle(SLIDER, BORDER_WIDTH) - 2*GuiGetStyle(SLIDER, SLIDER_PADDING) };

    if (sliderWidth > 0)        // Slider
    {
        slider.x += (sliderValue - sliderWidth/2);
        slider.width = (float)sliderWidth;
    }
    else if (sliderWidth == 0)  // SliderBar
    {
        slider.x += GuiGetStyle(SLIDER, BORDER_WIDTH);
        slider.width = (float)sliderValue;
    }

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && !guiLocked)
    {
        Vector2 mousePoint = GetMousePosition();

        if (CheckCollisionPointRec(mousePoint, bounds))
        {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
            {
                state = GUI_STATE_PRESSED;

                // Get equivalent value and slider position from mousePoint.x
                value = ((maxValue - minValue)*(mousePoint.x - (float)(bounds.x + sliderWidth/2)))/(float)(bounds.width - sliderWidth) + minValue;

                if (sliderWidth > 0) slider.x = mousePoint.x - slider.width/2;  // Slider
                else if (sliderWidth == 0) slider.width = (float)sliderValue;          // SliderBar
            }
            else state = GUI_STATE_FOCUSED;
        }

        if (value > maxValue) value = maxValue;
        else if (value < minValue) value = minValue;
    }

    // Bar limits check
    if (sliderWidth > 0)        // Slider
    {
        if (slider.x <= (bounds.x + GuiGetStyle(SLIDER, BORDER_WIDTH))) slider.x = bounds.x + GuiGetStyle(SLIDER, BORDER_WIDTH);
        else if ((slider.x + slider.width) >= (bounds.x + bounds.width)) slider.x = bounds.x + bounds.width - slider.width - GuiGetStyle(SLIDER, BORDER_WIDTH);
    }
    else if (sliderWidth == 0)  // SliderBar
    {
        if (slider.width > bounds.width) slider.width = bounds.width - 2*GuiGetStyle(SLIDER, BORDER_WIDTH);
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    GuiDrawRectangle(bounds, GuiGetStyle(SLIDER, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(SLIDER, BORDER + (state*3))), guiAlpha), Fade(GetColor(GuiGetStyle(SLIDER, (state != GUI_STATE_DISABLED)?  BASE_COLOR_NORMAL : BASE_COLOR_DISABLED)), guiAlpha));

    // Draw slider internal bar (depends on state)
    if ((state == GUI_STATE_NORMAL) || (state == GUI_STATE_PRESSED)) GuiDrawRectangle(slider, 0, BLANK, Fade(GetColor(GuiGetStyle(SLIDER, BASE_COLOR_PRESSED)), guiAlpha));
    else if (state == GUI_STATE_FOCUSED) GuiDrawRectangle(slider, 0, BLANK, Fade(GetColor(GuiGetStyle(SLIDER, TEXT_COLOR_FOCUSED)), guiAlpha));

    // Draw left/right text if provided
    if (textLeft != NULL)
    {
        Rectangle textBounds = { 0 };
        textBounds.width = (float)GetTextWidth(textLeft);
        textBounds.height = (float)GuiGetStyle(DEFAULT, TEXT_SIZE);
        textBounds.x = bounds.x - textBounds.width - GuiGetStyle(SLIDER, TEXT_PADDING);
        textBounds.y = bounds.y + bounds.height/2 - GuiGetStyle(DEFAULT, TEXT_SIZE)/2;

        GuiDrawText(textLeft, textBounds, GUI_TEXT_ALIGN_RIGHT, Fade(GetColor(GuiGetStyle(SLIDER, TEXT + (state*3))), guiAlpha));
    }

    if (textRight != NULL)
    {
        Rectangle textBounds = { 0 };
        textBounds.width = (float)GetTextWidth(textRight);
        textBounds.height = (float)GuiGetStyle(DEFAULT, TEXT_SIZE);
        textBounds.x = bounds.x + bounds.width + GuiGetStyle(SLIDER, TEXT_PADDING);
        textBounds.y = bounds.y + bounds.height/2 - GuiGetStyle(DEFAULT, TEXT_SIZE)/2;

        GuiDrawText(textRight, textBounds, GUI_TEXT_ALIGN_LEFT, Fade(GetColor(GuiGetStyle(SLIDER, TEXT + (state*3))), guiAlpha));
    }
    //--------------------------------------------------------------------

    return value;
}

// Slider control extended, returns selected value and has text
float GuiSlider(Rectangle bounds, const char *textLeft, const char *textRight, float value, float minValue, float maxValue)
{
    return GuiSliderPro(bounds, textLeft, textRight, value, minValue, maxValue, GuiGetStyle(SLIDER, SLIDER_WIDTH));
}

// Slider Bar control extended, returns selected value
float GuiSliderBar(Rectangle bounds, const char *textLeft, const char *textRight, float value, float minValue, float maxValue)
{
    return GuiSliderPro(bounds, textLeft, textRight, value, minValue, maxValue, 0);
}

// Progress Bar control extended, shows current progress value
float GuiProgressBar(Rectangle bounds, const char *textLeft, const char *textRight, float value, float minValue, float maxValue)
{
    GuiControlState state = guiState;

    Rectangle progress = { bounds.x + GuiGetStyle(PROGRESSBAR, BORDER_WIDTH),
                           bounds.y + GuiGetStyle(PROGRESSBAR, BORDER_WIDTH) + GuiGetStyle(PROGRESSBAR, PROGRESS_PADDING), 0,
                           bounds.height - 2*GuiGetStyle(PROGRESSBAR, BORDER_WIDTH) - 2*GuiGetStyle(PROGRESSBAR, PROGRESS_PADDING) };

    // Update control
    //--------------------------------------------------------------------
    if (state != GUI_STATE_DISABLED) progress.width = ((float)(value/(maxValue - minValue))*(float)(bounds.width - 2*GuiGetStyle(PROGRESSBAR, BORDER_WIDTH)));
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    GuiDrawRectangle(bounds, GuiGetStyle(PROGRESSBAR, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(PROGRESSBAR, BORDER + (state*3))), guiAlpha), BLANK);

    // Draw slider internal progress bar (depends on state)
    if ((state == GUI_STATE_NORMAL) || (state == GUI_STATE_PRESSED)) GuiDrawRectangle(progress, 0, BLANK, Fade(GetColor(GuiGetStyle(PROGRESSBAR, BASE_COLOR_PRESSED)), guiAlpha));
    else if (state == GUI_STATE_FOCUSED) GuiDrawRectangle(progress, 0, BLANK, Fade(GetColor(GuiGetStyle(PROGRESSBAR, TEXT_COLOR_FOCUSED)), guiAlpha));

    // Draw left/right text if provided
    if (textLeft != NULL)
    {
        Rectangle textBounds = { 0 };
        textBounds.width = (float)GetTextWidth(textLeft);
        textBounds.height = (float)GuiGetStyle(DEFAULT, TEXT_SIZE);
        textBounds.x = bounds.x - textBounds.width - GuiGetStyle(PROGRESSBAR, TEXT_PADDING);
        textBounds.y = bounds.y + bounds.height/2 - GuiGetStyle(DEFAULT, TEXT_SIZE)/2;

        GuiDrawText(textLeft, textBounds, GUI_TEXT_ALIGN_RIGHT, Fade(GetColor(GuiGetStyle(PROGRESSBAR, TEXT + (state*3))), guiAlpha));
    }

    if (textRight != NULL)
    {
        Rectangle textBounds = { 0 };
        textBounds.width = (float)GetTextWidth(textRight);
        textBounds.height = (float)GuiGetStyle(DEFAULT, TEXT_SIZE);
        textBounds.x = bounds.x + bounds.width + GuiGetStyle(PROGRESSBAR, TEXT_PADDING);
        textBounds.y = bounds.y + bounds.height/2 - GuiGetStyle(DEFAULT, TEXT_SIZE)/2;

        GuiDrawText(textRight, textBounds, GUI_TEXT_ALIGN_LEFT, Fade(GetColor(GuiGetStyle(PROGRESSBAR, TEXT + (state*3))), guiAlpha));
    }
    //--------------------------------------------------------------------

    return value;
}

// Status Bar control
void GuiStatusBar(Rectangle bounds, const char *text)
{
    GuiControlState state = guiState;

    // Draw control
    //--------------------------------------------------------------------
    GuiDrawRectangle(bounds, GuiGetStyle(STATUSBAR, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(STATUSBAR, (state != GUI_STATE_DISABLED)? BORDER_COLOR_NORMAL : BORDER_COLOR_DISABLED)), guiAlpha),
                     Fade(GetColor(GuiGetStyle(STATUSBAR, (state != GUI_STATE_DISABLED)? BASE_COLOR_NORMAL : BASE_COLOR_DISABLED)), guiAlpha));
    GuiDrawText(text, GetTextBounds(STATUSBAR, bounds), GuiGetStyle(STATUSBAR, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(STATUSBAR, (state != GUI_STATE_DISABLED)? TEXT_COLOR_NORMAL : TEXT_COLOR_DISABLED)), guiAlpha));
    //--------------------------------------------------------------------
}

// Dummy rectangle control, intended for placeholding
void GuiDummyRec(Rectangle bounds, const char *text)
{
    GuiControlState state = guiState;

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && !guiLocked)
    {
        Vector2 mousePoint = GetMousePosition();

        // Check button state
        if (CheckCollisionPointRec(mousePoint, bounds))
        {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) state = GUI_STATE_PRESSED;
            else state = GUI_STATE_FOCUSED;
        }
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    GuiDrawRectangle(bounds, 0, BLANK, Fade(GetColor(GuiGetStyle(DEFAULT, (state != GUI_STATE_DISABLED)? BASE_COLOR_NORMAL : BASE_COLOR_DISABLED)), guiAlpha));
    GuiDrawText(text, GetTextBounds(DEFAULT, bounds), GUI_TEXT_ALIGN_CENTER, Fade(GetColor(GuiGetStyle(BUTTON, (state != GUI_STATE_DISABLED)? TEXT_COLOR_NORMAL : TEXT_COLOR_DISABLED)), guiAlpha));
    //------------------------------------------------------------------
}

// Scroll Bar control
int GuiScrollBar(Rectangle bounds, int value, int minValue, int maxValue)
{
    GuiControlState state = guiState;

    // Is the scrollbar horizontal or vertical?
    bool isVertical = (bounds.width > bounds.height)? false : true;

    // The size (width or height depending on scrollbar type) of the spinner buttons
    const int spinnerSize = GuiGetStyle(SCROLLBAR, ARROWS_VISIBLE)? (isVertical? (int)bounds.width - 2*GuiGetStyle(SCROLLBAR, BORDER_WIDTH) : (int)bounds.height - 2*GuiGetStyle(SCROLLBAR, BORDER_WIDTH)) : 0;

    // Arrow buttons [<] [>] [∧] [∨]
    Rectangle arrowUpLeft = { 0 };
    Rectangle arrowDownRight = { 0 };

    // Actual area of the scrollbar excluding the arrow buttons
    Rectangle scrollbar = { 0 };

    // Slider bar that moves     --[///]-----
    Rectangle slider = { 0 };

    // Normalize value
    if (value > maxValue) value = maxValue;
    if (value < minValue) value = minValue;

    const int range = maxValue - minValue;
    int sliderSize = GuiGetStyle(SCROLLBAR, SCROLL_SLIDER_SIZE);

    // Calculate rectangles for all of the components
    arrowUpLeft = RAYGUI_CLITERAL(Rectangle){ (float)bounds.x + GuiGetStyle(SCROLLBAR, BORDER_WIDTH), (float)bounds.y + GuiGetStyle(SCROLLBAR, BORDER_WIDTH), (float)spinnerSize, (float)spinnerSize };

    if (isVertical)
    {
        arrowDownRight = RAYGUI_CLITERAL(Rectangle){ (float)bounds.x + GuiGetStyle(SCROLLBAR, BORDER_WIDTH), (float)bounds.y + bounds.height - spinnerSize - GuiGetStyle(SCROLLBAR, BORDER_WIDTH), (float)spinnerSize, (float)spinnerSize};
        scrollbar = RAYGUI_CLITERAL(Rectangle){ bounds.x + GuiGetStyle(SCROLLBAR, BORDER_WIDTH) + GuiGetStyle(SCROLLBAR, SCROLL_PADDING), arrowUpLeft.y + arrowUpLeft.height, bounds.width - 2*(GuiGetStyle(SCROLLBAR, BORDER_WIDTH) + GuiGetStyle(SCROLLBAR, SCROLL_PADDING)), bounds.height - arrowUpLeft.height - arrowDownRight.height - 2*GuiGetStyle(SCROLLBAR, BORDER_WIDTH) };
        sliderSize = (sliderSize >= scrollbar.height)? ((int)scrollbar.height - 2) : sliderSize;     // Make sure the slider won't get outside of the scrollbar
        slider = RAYGUI_CLITERAL(Rectangle){ (float)bounds.x + GuiGetStyle(SCROLLBAR, BORDER_WIDTH) + GuiGetStyle(SCROLLBAR, SCROLL_SLIDER_PADDING), (float)scrollbar.y + (int)(((float)(value - minValue)/range)*(scrollbar.height - sliderSize)), (float)bounds.width - 2*(GuiGetStyle(SCROLLBAR, BORDER_WIDTH) + GuiGetStyle(SCROLLBAR, SCROLL_SLIDER_PADDING)), (float)sliderSize };
    }
    else
    {
        arrowDownRight = RAYGUI_CLITERAL(Rectangle){ (float)bounds.x + bounds.width - spinnerSize - GuiGetStyle(SCROLLBAR, BORDER_WIDTH), (float)bounds.y + GuiGetStyle(SCROLLBAR, BORDER_WIDTH), (float)spinnerSize, (float)spinnerSize};
        scrollbar = RAYGUI_CLITERAL(Rectangle){ arrowUpLeft.x + arrowUpLeft.width, bounds.y + GuiGetStyle(SCROLLBAR, BORDER_WIDTH) + GuiGetStyle(SCROLLBAR, SCROLL_PADDING), bounds.width - arrowUpLeft.width - arrowDownRight.width - 2*GuiGetStyle(SCROLLBAR, BORDER_WIDTH), bounds.height - 2*(GuiGetStyle(SCROLLBAR, BORDER_WIDTH) + GuiGetStyle(SCROLLBAR, SCROLL_PADDING))};
        sliderSize = (sliderSize >= scrollbar.width)? ((int)scrollbar.width - 2) : sliderSize;       // Make sure the slider won't get outside of the scrollbar
        slider = RAYGUI_CLITERAL(Rectangle){ (float)scrollbar.x + (int)(((float)(value - minValue)/range)*(scrollbar.width - sliderSize)), (float)bounds.y + GuiGetStyle(SCROLLBAR, BORDER_WIDTH) + GuiGetStyle(SCROLLBAR, SCROLL_SLIDER_PADDING), (float)sliderSize, (float)bounds.height - 2*(GuiGetStyle(SCROLLBAR, BORDER_WIDTH) + GuiGetStyle(SCROLLBAR, SCROLL_SLIDER_PADDING)) };
    }

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && !guiLocked)
    {
        Vector2 mousePoint = GetMousePosition();

        if (CheckCollisionPointRec(mousePoint, bounds))
        {
            state = GUI_STATE_FOCUSED;

            // Handle mouse wheel
            int wheel = (int)GetMouseWheelMove();
            if (wheel != 0) value += wheel;

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                if (CheckCollisionPointRec(mousePoint, arrowUpLeft)) value -= range/GuiGetStyle(SCROLLBAR, SCROLL_SPEED);
                else if (CheckCollisionPointRec(mousePoint, arrowDownRight)) value += range/GuiGetStyle(SCROLLBAR, SCROLL_SPEED);

                state = GUI_STATE_PRESSED;
            }
            else if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
            {
                if (!isVertical)
                {
                    Rectangle scrollArea = { arrowUpLeft.x + arrowUpLeft.width, arrowUpLeft.y, scrollbar.width, bounds.height - 2*GuiGetStyle(SCROLLBAR, BORDER_WIDTH)};
                    if (CheckCollisionPointRec(mousePoint, scrollArea)) value = (int)(((float)(mousePoint.x - scrollArea.x - slider.width/2)*range)/(scrollArea.width - slider.width) + minValue);
                }
                else
                {
                    Rectangle scrollArea = { arrowUpLeft.x, arrowUpLeft.y+arrowUpLeft.height, bounds.width - 2*GuiGetStyle(SCROLLBAR, BORDER_WIDTH),  scrollbar.height};
                    if (CheckCollisionPointRec(mousePoint, scrollArea)) value = (int)(((float)(mousePoint.y - scrollArea.y - slider.height/2)*range)/(scrollArea.height - slider.height) + minValue);
                }
            }
        }

        // Normalize value
        if (value > maxValue) value = maxValue;
        if (value < minValue) value = minValue;
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    GuiDrawRectangle(bounds, GuiGetStyle(SCROLLBAR, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(LISTVIEW, BORDER + state*3)), guiAlpha), Fade(GetColor(GuiGetStyle(DEFAULT, BORDER_COLOR_DISABLED)), guiAlpha));   // Draw the background

    GuiDrawRectangle(scrollbar, 0, BLANK, Fade(GetColor(GuiGetStyle(BUTTON, BASE_COLOR_NORMAL)), guiAlpha));     // Draw the scrollbar active area background
    GuiDrawRectangle(slider, 0, BLANK, Fade(GetColor(GuiGetStyle(SLIDER, BORDER + state*3)), guiAlpha));         // Draw the slider bar

    // Draw arrows (using icon if available)
    if (GuiGetStyle(SCROLLBAR, ARROWS_VISIBLE))
    {
#if defined(RAYGUI_NO_RICONS)
        GuiDrawText(isVertical? "^" : "<", RAYGUI_CLITERAL(Rectangle){ arrowUpLeft.x, arrowUpLeft.y, isVertical? bounds.width : bounds.height, isVertical? bounds.width : bounds.height },
                    GUI_TEXT_ALIGN_CENTER, Fade(GetColor(GuiGetStyle(DROPDOWNBOX, TEXT + (state*3))), guiAlpha));
        GuiDrawText(isVertical? "v" : ">", RAYGUI_CLITERAL(Rectangle){ arrowDownRight.x, arrowDownRight.y, isVertical? bounds.width : bounds.height, isVertical? bounds.width : bounds.height },
                    GUI_TEXT_ALIGN_CENTER, Fade(GetColor(GuiGetStyle(DROPDOWNBOX, TEXT + (state*3))), guiAlpha));
#else
        GuiDrawText(isVertical? "#121#" : "#118#", RAYGUI_CLITERAL(Rectangle){ arrowUpLeft.x, arrowUpLeft.y, isVertical? bounds.width : bounds.height, isVertical? bounds.width : bounds.height },
                    GUI_TEXT_ALIGN_CENTER, Fade(GetColor(GuiGetStyle(SCROLLBAR, TEXT + state*3)), guiAlpha));   // RICON_ARROW_UP_FILL / RICON_ARROW_LEFT_FILL
        GuiDrawText(isVertical? "#120#" : "#119#", RAYGUI_CLITERAL(Rectangle){ arrowDownRight.x, arrowDownRight.y, isVertical? bounds.width : bounds.height, isVertical? bounds.width : bounds.height },
                    GUI_TEXT_ALIGN_CENTER, Fade(GetColor(GuiGetStyle(SCROLLBAR, TEXT + state*3)), guiAlpha));   // RICON_ARROW_DOWN_FILL / RICON_ARROW_RIGHT_FILL
#endif
    }
    //--------------------------------------------------------------------

    return value;
}

// List View control
int GuiListView(Rectangle bounds, const char *text, int *scrollIndex, int active)
{
    int itemCount = 0;
    const char **items = NULL;

    if (text != NULL) items = GuiTextSplit(text, &itemCount, NULL);

    return GuiListViewEx(bounds, items, itemCount, NULL, scrollIndex, active);
}

// List View control with extended parameters
int GuiListViewEx(Rectangle bounds, const char **text, int count, int *focus, int *scrollIndex, int active)
{
    GuiControlState state = guiState;
    int itemFocused = (focus == NULL)? -1 : *focus;
    int itemSelected = active;

    // Check if we need a scroll bar
    bool useScrollBar = false;
    if ((GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT) + GuiGetStyle(LISTVIEW, LIST_ITEMS_PADDING))*count > bounds.height) useScrollBar = true;

    // Define base item rectangle [0]
    Rectangle itemBounds = { 0 };
    itemBounds.x = bounds.x + GuiGetStyle(LISTVIEW, LIST_ITEMS_PADDING);
    itemBounds.y = bounds.y + GuiGetStyle(LISTVIEW, LIST_ITEMS_PADDING) + GuiGetStyle(DEFAULT, BORDER_WIDTH);
    itemBounds.width = bounds.width - 2*GuiGetStyle(LISTVIEW, LIST_ITEMS_PADDING) - GuiGetStyle(DEFAULT, BORDER_WIDTH);
    itemBounds.height = (float)GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT);
    if (useScrollBar) itemBounds.width -= GuiGetStyle(LISTVIEW, SCROLLBAR_WIDTH);

    // Get items on the list
    int visibleItems = (int)bounds.height/(GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT) + GuiGetStyle(LISTVIEW, LIST_ITEMS_PADDING));
    if (visibleItems > count) visibleItems = count;

    int startIndex = (scrollIndex == NULL)? 0 : *scrollIndex;
    if ((startIndex < 0) || (startIndex > (count - visibleItems))) startIndex = 0;
    int endIndex = startIndex + visibleItems;

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && !guiLocked)
    {
        Vector2 mousePoint = GetMousePosition();

        // Check mouse inside list view
        if (CheckCollisionPointRec(mousePoint, bounds))
        {
            state = GUI_STATE_FOCUSED;

            // Check focused and selected item
            for (int i = 0; i < visibleItems; i++)
            {
                if (CheckCollisionPointRec(mousePoint, itemBounds))
                {
                    itemFocused = startIndex + i;
                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                    {
                        if (itemSelected == (startIndex + i)) itemSelected = -1;
                        else itemSelected = startIndex + i;
                    }
                    break;
                }

                // Update item rectangle y position for next item
                itemBounds.y += (GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT) + GuiGetStyle(LISTVIEW, LIST_ITEMS_PADDING));
            }

            if (useScrollBar)
            {
                int wheelMove = (int)GetMouseWheelMove();
                startIndex -= wheelMove;

                if (startIndex < 0) startIndex = 0;
                else if (startIndex > (count - visibleItems)) startIndex = count - visibleItems;

                endIndex = startIndex + visibleItems;
                if (endIndex > count) endIndex = count;
            }
        }
        else itemFocused = -1;

        // Reset item rectangle y to [0]
        itemBounds.y = bounds.y + GuiGetStyle(LISTVIEW, LIST_ITEMS_PADDING) + GuiGetStyle(DEFAULT, BORDER_WIDTH);
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    GuiDrawRectangle(bounds, GuiGetStyle(DEFAULT, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(LISTVIEW, BORDER + state*3)), guiAlpha), GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));     // Draw background

    // Draw visible items
    for (int i = 0; ((i < visibleItems) && (text != NULL)); i++)
    {
        if (state == GUI_STATE_DISABLED)
        {
            if ((startIndex + i) == itemSelected) GuiDrawRectangle(itemBounds, GuiGetStyle(LISTVIEW, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(LISTVIEW, BORDER_COLOR_DISABLED)), guiAlpha), Fade(GetColor(GuiGetStyle(LISTVIEW, BASE_COLOR_DISABLED)), guiAlpha));

            GuiDrawText(text[startIndex + i], GetTextBounds(DEFAULT, itemBounds), GuiGetStyle(LISTVIEW, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(LISTVIEW, TEXT_COLOR_DISABLED)), guiAlpha));
        }
        else
        {
            if ((startIndex + i) == itemSelected)
            {
                // Draw item selected
                GuiDrawRectangle(itemBounds, GuiGetStyle(LISTVIEW, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(LISTVIEW, BORDER_COLOR_PRESSED)), guiAlpha), Fade(GetColor(GuiGetStyle(LISTVIEW, BASE_COLOR_PRESSED)), guiAlpha));
                GuiDrawText(text[startIndex + i], GetTextBounds(DEFAULT, itemBounds), GuiGetStyle(LISTVIEW, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(LISTVIEW, TEXT_COLOR_PRESSED)), guiAlpha));
            }
            else if ((startIndex + i) == itemFocused)
            {
                // Draw item focused
                GuiDrawRectangle(itemBounds, GuiGetStyle(LISTVIEW, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(LISTVIEW, BORDER_COLOR_FOCUSED)), guiAlpha), Fade(GetColor(GuiGetStyle(LISTVIEW, BASE_COLOR_FOCUSED)), guiAlpha));
                GuiDrawText(text[startIndex + i], GetTextBounds(DEFAULT, itemBounds), GuiGetStyle(LISTVIEW, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(LISTVIEW, TEXT_COLOR_FOCUSED)), guiAlpha));
            }
            else
            {
                // Draw item normal
                GuiDrawText(text[startIndex + i], GetTextBounds(DEFAULT, itemBounds), GuiGetStyle(LISTVIEW, TEXT_ALIGNMENT), Fade(GetColor(GuiGetStyle(LISTVIEW, TEXT_COLOR_NORMAL)), guiAlpha));
            }
        }

        // Update item rectangle y position for next item
        itemBounds.y += (GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT) + GuiGetStyle(LISTVIEW, LIST_ITEMS_PADDING));
    }

    if (useScrollBar)
    {
        Rectangle scrollBarBounds = {
            bounds.x + bounds.width - GuiGetStyle(LISTVIEW, BORDER_WIDTH) - GuiGetStyle(LISTVIEW, SCROLLBAR_WIDTH),
            bounds.y + GuiGetStyle(LISTVIEW, BORDER_WIDTH), (float)GuiGetStyle(LISTVIEW, SCROLLBAR_WIDTH),
            bounds.height - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH)
        };

        // Calculate percentage of visible items and apply same percentage to scrollbar
        float percentVisible = (float)(endIndex - startIndex)/count;
        float sliderSize = bounds.height*percentVisible;

        int prevSliderSize = GuiGetStyle(SCROLLBAR, SCROLL_SLIDER_SIZE);   // Save default slider size
        int prevScrollSpeed = GuiGetStyle(SCROLLBAR, SCROLL_SPEED); // Save default scroll speed
        GuiSetStyle(SCROLLBAR, SCROLL_SLIDER_SIZE, (int)sliderSize);            // Change slider size
        GuiSetStyle(SCROLLBAR, SCROLL_SPEED, count - visibleItems); // Change scroll speed

        startIndex = GuiScrollBar(scrollBarBounds, startIndex, 0, count - visibleItems);

        GuiSetStyle(SCROLLBAR, SCROLL_SPEED, prevScrollSpeed); // Reset scroll speed to default
        GuiSetStyle(SCROLLBAR, SCROLL_SLIDER_SIZE, prevSliderSize); // Reset slider size to default
    }
    //--------------------------------------------------------------------

    if (focus != NULL) *focus = itemFocused;
    if (scrollIndex != NULL) *scrollIndex = startIndex;

    return itemSelected;
}

// Color Panel control
Color GuiColorPanel(Rectangle bounds, Color color)
{
    const Color colWhite = { 255, 255, 255, 255 };
    const Color colBlack = { 0, 0, 0, 255 };

    GuiControlState state = guiState;
    Vector2 pickerSelector ;

    Vector3 vcolor = Vec3((float)color.r/255.0f, (float)color.g/255.0f, (float)color.b/255.0f );
    Vector3 hsv = ConvertRGBtoHSV(vcolor);

    pickerSelector.x = bounds.x + (float)hsv.y*bounds.width;            // HSV: Saturation
    pickerSelector.y = bounds.y + (1.0f - (float)hsv.z)*bounds.height;  // HSV: Value

    float hue = -1.0f;
    Vector3 maxHue = { hue >= 0.0f ? hue : hsv.x, 1.0f, 1.0f };
    Vector3 rgbHue = ConvertHSVtoRGB(maxHue);
    Color maxHueCol = { (unsigned char)(255.0f*rgbHue.x),
                      (unsigned char)(255.0f*rgbHue.y),
                      (unsigned char)(255.0f*rgbHue.z), 255 };

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && !guiLocked)
    {
        Vector2 mousePoint = GetMousePosition();

        if (CheckCollisionPointRec(mousePoint, bounds))
        {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
            {
                state = GUI_STATE_PRESSED;
                pickerSelector = mousePoint;

                // Calculate color from picker
                Vector2 colorPick = { pickerSelector.x - bounds.x, pickerSelector.y - bounds.y };

                colorPick.x /= (float)bounds.width;     // Get normalized value on x
                colorPick.y /= (float)bounds.height;    // Get normalized value on y

                hsv.y = colorPick.x;
                hsv.z = 1.0f - colorPick.y;

                Vector3 rgb = ConvertHSVtoRGB(hsv);

                // NOTE: Vector3ToColor() only available on raylib 1.8.1
                color = RAYGUI_CLITERAL(Color){ (unsigned char)(255.0f*rgb.x),
                                 (unsigned char)(255.0f*rgb.y),
                                 (unsigned char)(255.0f*rgb.z),
                                 (unsigned char)(255.0f*(float)color.a/255.0f) };

            }
            else state = GUI_STATE_FOCUSED;
        }
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    if (state != GUI_STATE_DISABLED)
    {
        DrawRectangleGradientEx(bounds, Fade(colWhite, guiAlpha), Fade(colWhite, guiAlpha), Fade(maxHueCol, guiAlpha), Fade(maxHueCol, guiAlpha));
        DrawRectangleGradientEx(bounds, Fade(colBlack, 0), Fade(colBlack, guiAlpha), Fade(colBlack, guiAlpha), Fade(colBlack, 0));

        // Draw color picker: selector
        Rectangle selector = { pickerSelector.x - GuiGetStyle(COLORPICKER, COLOR_SELECTOR_SIZE)/2, pickerSelector.y - GuiGetStyle(COLORPICKER, COLOR_SELECTOR_SIZE)/2, (float)GuiGetStyle(COLORPICKER, COLOR_SELECTOR_SIZE), (float)GuiGetStyle(COLORPICKER, COLOR_SELECTOR_SIZE) };
        GuiDrawRectangle(selector, 0, BLANK, Fade(colWhite, guiAlpha));
    }
    else
    {
        DrawRectangleGradientEx(bounds, Fade(Fade(GetColor(GuiGetStyle(COLORPICKER, BASE_COLOR_DISABLED)), 0.1f), guiAlpha), Fade(Fade(colBlack, 0.6f), guiAlpha), Fade(Fade(colBlack, 0.6f), guiAlpha), Fade(Fade(GetColor(GuiGetStyle(COLORPICKER, BORDER_COLOR_DISABLED)), 0.6f), guiAlpha));
    }

    GuiDrawRectangle(bounds, GuiGetStyle(COLORPICKER, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(COLORPICKER, BORDER + state*3)), guiAlpha), BLANK);
    //--------------------------------------------------------------------

    return color;
}

// Color Bar Alpha control
// NOTE: Returns alpha value normalized [0..1]
float GuiColorBarAlpha(Rectangle bounds, float alpha)
{
    #define COLORBARALPHA_CHECKED_SIZE   10

    GuiControlState state = guiState;
    Rectangle selector = { (float)bounds.x + alpha*bounds.width - GuiGetStyle(COLORPICKER, HUEBAR_SELECTOR_HEIGHT)/2, (float)bounds.y - GuiGetStyle(COLORPICKER, HUEBAR_SELECTOR_OVERFLOW), (float)GuiGetStyle(COLORPICKER, HUEBAR_SELECTOR_HEIGHT), (float)bounds.height + GuiGetStyle(COLORPICKER, HUEBAR_SELECTOR_OVERFLOW)*2 };

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && !guiLocked)
    {
        Vector2 mousePoint = GetMousePosition();

        if (CheckCollisionPointRec(mousePoint, bounds) ||
            CheckCollisionPointRec(mousePoint, selector))
        {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
            {
                state = GUI_STATE_PRESSED;

                alpha = (mousePoint.x - bounds.x)/bounds.width;
                if (alpha <= 0.0f) alpha = 0.0f;
                if (alpha >= 1.0f) alpha = 1.0f;
                //selector.x = bounds.x + (int)(((alpha - 0)/(100 - 0))*(bounds.width - 2*GuiGetStyle(SLIDER, BORDER_WIDTH))) - selector.width/2;
            }
            else state = GUI_STATE_FOCUSED;
        }
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------

    // Draw alpha bar: checked background
    if (state != GUI_STATE_DISABLED)
    {
        int checksX = (int)bounds.width/COLORBARALPHA_CHECKED_SIZE;
        int checksY = (int)bounds.height/COLORBARALPHA_CHECKED_SIZE;

        for (int x = 0; x < checksX; x++)
        {
            for (int y = 0; y < checksY; y++)
            {
                Rectangle check = { bounds.x + x*COLORBARALPHA_CHECKED_SIZE, bounds.y + y*COLORBARALPHA_CHECKED_SIZE, COLORBARALPHA_CHECKED_SIZE, COLORBARALPHA_CHECKED_SIZE };
                GuiDrawRectangle(check, 0, BLANK, ((x + y)%2)? Fade(Fade(GetColor(GuiGetStyle(COLORPICKER, BORDER_COLOR_DISABLED)), 0.4f), guiAlpha) : Fade(Fade(GetColor(GuiGetStyle(COLORPICKER, BASE_COLOR_DISABLED)), 0.4f), guiAlpha));
            }
        }

        DrawRectangleGradientEx(bounds, RAYGUI_CLITERAL(Color){ 255, 255, 255, 0 }, RAYGUI_CLITERAL(Color){ 255, 255, 255, 0 }, Fade(RAYGUI_CLITERAL(Color){ 0, 0, 0, 255 }, guiAlpha), Fade(RAYGUI_CLITERAL(Color){ 0, 0, 0, 255 }, guiAlpha));
    }
    else DrawRectangleGradientEx(bounds, Fade(GetColor(GuiGetStyle(COLORPICKER, BASE_COLOR_DISABLED)), 0.1f), Fade(GetColor(GuiGetStyle(COLORPICKER, BASE_COLOR_DISABLED)), 0.1f), Fade(GetColor(GuiGetStyle(COLORPICKER, BORDER_COLOR_DISABLED)), guiAlpha), Fade(GetColor(GuiGetStyle(COLORPICKER, BORDER_COLOR_DISABLED)), guiAlpha));

    GuiDrawRectangle(bounds, GuiGetStyle(COLORPICKER, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(COLORPICKER, BORDER + state*3)), guiAlpha), BLANK);

    // Draw alpha bar: selector
    GuiDrawRectangle(selector, 0, BLANK, Fade(GetColor(GuiGetStyle(COLORPICKER, BORDER + state*3)), guiAlpha));
    //--------------------------------------------------------------------

    return alpha;
}

// Color Bar Hue control
// Returns hue value normalized [0..1]
// NOTE: Other similar bars (for reference):
//      Color GuiColorBarSat() [WHITE->color]
//      Color GuiColorBarValue() [BLACK->color], HSV/HSL
//      float GuiColorBarLuminance() [BLACK->WHITE]
float GuiColorBarHue(Rectangle bounds, float hue)
{
    GuiControlState state = guiState;
    Rectangle selector = { (float)bounds.x - GuiGetStyle(COLORPICKER, HUEBAR_SELECTOR_OVERFLOW), (float)bounds.y + hue/360.0f*bounds.height - GuiGetStyle(COLORPICKER, HUEBAR_SELECTOR_HEIGHT)/2, (float)bounds.width + GuiGetStyle(COLORPICKER, HUEBAR_SELECTOR_OVERFLOW)*2, (float)GuiGetStyle(COLORPICKER, HUEBAR_SELECTOR_HEIGHT) };

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && !guiLocked)
    {
        Vector2 mousePoint = GetMousePosition();

        if (CheckCollisionPointRec(mousePoint, bounds) ||
            CheckCollisionPointRec(mousePoint, selector))
        {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
            {
                state = GUI_STATE_PRESSED;

                hue = (mousePoint.y - bounds.y)*360/bounds.height;
                if (hue <= 0.0f) hue = 0.0f;
                if (hue >= 359.0f) hue = 359.0f;

            }
            else state = GUI_STATE_FOCUSED;

            /*if (IsKeyDown(KEY_UP))
            {
                hue -= 2.0f;
                if (hue <= 0.0f) hue = 0.0f;
            }
            else if (IsKeyDown(KEY_DOWN))
            {
                hue += 2.0f;
                if (hue >= 360.0f) hue = 360.0f;
            }*/
        }
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    if (state != GUI_STATE_DISABLED)
    {
        // Draw hue bar:color bars
        DrawRectangleGradientV((int)bounds.x, (int)(bounds.y), (int)bounds.width, ceil(bounds.height/6),  Fade(RAYGUI_CLITERAL(Color) { 255, 0, 0, 255 }, guiAlpha), Fade(RAYGUI_CLITERAL(Color) { 255, 255, 0, 255 }, guiAlpha));
        DrawRectangleGradientV((int)bounds.x, (int)(bounds.y + bounds.height/6), (int)bounds.width, ceil(bounds.height/6), Fade(RAYGUI_CLITERAL(Color) { 255, 255, 0, 255 }, guiAlpha), Fade(RAYGUI_CLITERAL(Color) { 0, 255, 0, 255 }, guiAlpha));
        DrawRectangleGradientV((int)bounds.x, (int)(bounds.y + 2*(bounds.height/6)), (int)bounds.width, ceil(bounds.height/6), Fade(RAYGUI_CLITERAL(Color) { 0, 255, 0, 255 }, guiAlpha), Fade(RAYGUI_CLITERAL(Color) { 0, 255, 255, 255 }, guiAlpha));
        DrawRectangleGradientV((int)bounds.x, (int)(bounds.y + 3*(bounds.height/6)), (int)bounds.width, ceil(bounds.height/6), Fade(RAYGUI_CLITERAL(Color) { 0, 255, 255, 255 }, guiAlpha), Fade(RAYGUI_CLITERAL(Color) { 0, 0, 255, 255 }, guiAlpha));
        DrawRectangleGradientV((int)bounds.x, (int)(bounds.y + 4*(bounds.height/6)), (int)bounds.width, ceil(bounds.height/6), Fade(RAYGUI_CLITERAL(Color) { 0, 0, 255, 255 }, guiAlpha), Fade(RAYGUI_CLITERAL(Color) { 255, 0, 255, 255 }, guiAlpha));
        DrawRectangleGradientV((int)bounds.x, (int)(bounds.y + 5*(bounds.height/6)), (int)bounds.width, (int)(bounds.height/6), Fade(RAYGUI_CLITERAL(Color) { 255, 0, 255, 255 }, guiAlpha), Fade(RAYGUI_CLITERAL(Color) { 255, 0, 0, 255 }, guiAlpha));
    }
    else DrawRectangleGradientV((int)bounds.x, (int)bounds.y, (int)bounds.width, (int)bounds.height, Fade(Fade(GetColor(GuiGetStyle(COLORPICKER, BASE_COLOR_DISABLED)), 0.1f), guiAlpha), Fade(GetColor(GuiGetStyle(COLORPICKER, BORDER_COLOR_DISABLED)), guiAlpha));

   GuiDrawRectangle(bounds, GuiGetStyle(COLORPICKER, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(COLORPICKER, BORDER + state*3)), guiAlpha), BLANK);

    // Draw hue bar: selector
GuiDrawRectangle(selector, 0, BLANK, Fade(GetColor(GuiGetStyle(COLORPICKER, BORDER + state*3)), guiAlpha));
    //--------------------------------------------------------------------

    return hue;
}

// Color Picker control
// NOTE: It's divided in multiple controls:
//      Color GuiColorPanel(Rectangle bounds, Color color)
//      float GuiColorBarAlpha(Rectangle bounds, float alpha)
//      float GuiColorBarHue(Rectangle bounds, float value)
// NOTE: bounds define GuiColorPanel() size
Color GuiColorPicker(Rectangle bounds, Color color)
{
    color = GuiColorPanel(bounds, color);

    Rectangle boundsHue = { (float)bounds.x + bounds.width + GuiGetStyle(COLORPICKER, HUEBAR_PADDING), (float)bounds.y, (float)GuiGetStyle(COLORPICKER, HUEBAR_WIDTH), (float)bounds.height };
    //Rectangle boundsAlpha = { bounds.x, bounds.y + bounds.height + GuiGetStyle(COLORPICKER, BARS_PADDING), bounds.width, GuiGetStyle(COLORPICKER, BARS_THICK) };

    Vector3 hsv = ConvertRGBtoHSV(RAYGUI_CLITERAL(Vector3){ color.r/255.0f, color.g/255.0f, color.b/255.0f });
    hsv.x = GuiColorBarHue(boundsHue, hsv.x);
    //color.a = (unsigned char)(GuiColorBarAlpha(boundsAlpha, (float)color.a/255.0f)*255.0f);
    Vector3 rgb = ConvertHSVtoRGB(hsv);

    color = RAYGUI_CLITERAL(Color){ (unsigned char)roundf(rgb.x*255.0f), (unsigned char)roundf(rgb.y*255.0f), (unsigned char)roundf(rgb.z*255.0f), color.a };

    return color;
}

// Message Box control
int GuiMessageBox(Rectangle bounds, const char *title, const char *message, const char *buttons)
{
    #define MESSAGEBOX_BUTTON_HEIGHT    24
    #define MESSAGEBOX_BUTTON_PADDING   10

    int clicked = -1;    // Returns clicked button from buttons list, 0 refers to closed window button

    int buttonCount = 0;
    const char **buttonsText = GuiTextSplit(buttons, &buttonCount, NULL);
    Rectangle buttonBounds = { 0 };
    buttonBounds.x = bounds.x + MESSAGEBOX_BUTTON_PADDING;
    buttonBounds.y = bounds.y + bounds.height - MESSAGEBOX_BUTTON_HEIGHT - MESSAGEBOX_BUTTON_PADDING;
    buttonBounds.width = (bounds.width - MESSAGEBOX_BUTTON_PADDING*(buttonCount + 1))/buttonCount;
    buttonBounds.height = MESSAGEBOX_BUTTON_HEIGHT;

    Vector2 textSize = MeasureTextEx(guiFont, message, (float)GuiGetStyle(DEFAULT, TEXT_SIZE), 1);

    Rectangle textBounds = { 0 };
    textBounds.x = bounds.x + bounds.width/2 - textSize.x/2;
    textBounds.y = bounds.y + WINDOW_STATUSBAR_HEIGHT + (bounds.height - WINDOW_STATUSBAR_HEIGHT - MESSAGEBOX_BUTTON_HEIGHT - MESSAGEBOX_BUTTON_PADDING)/2 - textSize.y/2;
    textBounds.width = textSize.x;
    textBounds.height = textSize.y;

    // Draw control
    //--------------------------------------------------------------------
    if (GuiWindowBox(bounds, title)) clicked = 0;

    int prevTextAlignment = GuiGetStyle(LABEL, TEXT_ALIGNMENT);
    GuiSetStyle(LABEL, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_CENTER);
    GuiLabel(textBounds, message);
    GuiSetStyle(LABEL, TEXT_ALIGNMENT, prevTextAlignment);

    prevTextAlignment = GuiGetStyle(BUTTON, TEXT_ALIGNMENT);
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_CENTER);

    for (int i = 0; i < buttonCount; i++)
    {
        if (GuiButton(buttonBounds, buttonsText[i])) clicked = i + 1;
        buttonBounds.x += (buttonBounds.width + MESSAGEBOX_BUTTON_PADDING);
    }

    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, prevTextAlignment);
    //--------------------------------------------------------------------

    return clicked;
}

// Text Input Box control, ask for text
int GuiTextInputBox(Rectangle bounds, const char *title, const char *message, const char *buttons, char *text)
{
    #define TEXTINPUTBOX_BUTTON_HEIGHT      24
    #define TEXTINPUTBOX_BUTTON_PADDING     10
    #define TEXTINPUTBOX_HEIGHT             30

    #define TEXTINPUTBOX_MAX_TEXT_LENGTH   256

    // Used to enable text edit mode
    // WARNING: No more than one GuiTextInputBox() should be open at the same time
    static bool textEditMode = false;

    int btnIndex = -1;

    int buttonCount = 0;
    const char **buttonsText = GuiTextSplit(buttons, &buttonCount, NULL);
    Rectangle buttonBounds = { 0 };
    buttonBounds.x = bounds.x + TEXTINPUTBOX_BUTTON_PADDING;
    buttonBounds.y = bounds.y + bounds.height - TEXTINPUTBOX_BUTTON_HEIGHT - TEXTINPUTBOX_BUTTON_PADDING;
    buttonBounds.width = (bounds.width - TEXTINPUTBOX_BUTTON_PADDING*(buttonCount + 1))/buttonCount;
    buttonBounds.height = TEXTINPUTBOX_BUTTON_HEIGHT;

    int messageInputHeight = (int)bounds.height - WINDOW_STATUSBAR_HEIGHT - GuiGetStyle(STATUSBAR, BORDER_WIDTH) - TEXTINPUTBOX_BUTTON_HEIGHT - 2*TEXTINPUTBOX_BUTTON_PADDING;

    Rectangle textBounds = { 0 };
    if (message != NULL)
    {
        Vector2 textSize = MeasureTextEx(guiFont, message, (float)GuiGetStyle(DEFAULT, TEXT_SIZE), 1);

        textBounds.x = bounds.x + bounds.width/2 - textSize.x/2;
        textBounds.y = bounds.y + WINDOW_STATUSBAR_HEIGHT + messageInputHeight/4 - textSize.y/2;
        textBounds.width = textSize.x;
        textBounds.height = textSize.y;
    }

    Rectangle textBoxBounds = { 0 };
    textBoxBounds.x = bounds.x + TEXTINPUTBOX_BUTTON_PADDING;
    textBoxBounds.y = bounds.y + WINDOW_STATUSBAR_HEIGHT - TEXTINPUTBOX_HEIGHT/2;
    if (message == NULL) textBoxBounds.y += messageInputHeight/2;
    else textBoxBounds.y += (messageInputHeight/2 + messageInputHeight/4);
    textBoxBounds.width = bounds.width - TEXTINPUTBOX_BUTTON_PADDING*2;
    textBoxBounds.height = TEXTINPUTBOX_HEIGHT;

    // Draw control
    //--------------------------------------------------------------------
    if (GuiWindowBox(bounds, title)) btnIndex = 0;

    // Draw message if available
    if (message != NULL)
    {
        int prevTextAlignment = GuiGetStyle(LABEL, TEXT_ALIGNMENT);
        GuiSetStyle(LABEL, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_CENTER);
        GuiLabel(textBounds, message);
        GuiSetStyle(LABEL, TEXT_ALIGNMENT, prevTextAlignment);
    }

    if (GuiTextBox(textBoxBounds, text, TEXTINPUTBOX_MAX_TEXT_LENGTH, textEditMode)) textEditMode = !textEditMode;

    int prevBtnTextAlignment = GuiGetStyle(BUTTON, TEXT_ALIGNMENT);
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_CENTER);

    for (int i = 0; i < buttonCount; i++)
    {
        if (GuiButton(buttonBounds, buttonsText[i])) btnIndex = i + 1;
        buttonBounds.x += (buttonBounds.width + MESSAGEBOX_BUTTON_PADDING);
    }

    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, prevBtnTextAlignment);
    //--------------------------------------------------------------------

    return btnIndex;
}

// Grid control
// NOTE: Returns grid mouse-hover selected cell
// About drawing lines at subpixel spacing, simple put, not easy solution:
// https://stackoverflow.com/questions/4435450/2d-opengl-drawing-lines-that-dont-exactly-fit-pixel-raster
Vector2 GuiGrid(Rectangle bounds, float spacing, int subdivs)
{
    #if !defined(GRID_COLOR_ALPHA)
        #define GRID_COLOR_ALPHA    0.15f           // Grid lines alpha amount
    #endif

    GuiControlState state = guiState;
    Vector2 mousePoint = GetMousePosition();
    Vector2 currentCell = { -1, -1 };

    int linesV = ((int)(bounds.width/spacing))*subdivs + 1;
    int linesH = ((int)(bounds.height/spacing))*subdivs + 1;

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && !guiLocked)
    {
        if (CheckCollisionPointRec(mousePoint, bounds))
        {
            currentCell.x = (mousePoint.x - bounds.x)/spacing;
            currentCell.y = (mousePoint.y - bounds.y)/spacing;
        }
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    switch (state)
    {
        case GUI_STATE_NORMAL:
        {
            if (subdivs > 0)
            {
                // Draw vertical grid lines
                for (int i = 0; i < linesV; i++)
                {
                    Rectangle lineV = { bounds.x + spacing*i/subdivs, bounds.y, 1, bounds.height };
                    GuiDrawRectangle(lineV, 0, BLANK, ((i%subdivs) == 0) ? Fade(GetColor(GuiGetStyle(DEFAULT, LINE_COLOR)), GRID_COLOR_ALPHA*4) : Fade(GetColor(GuiGetStyle(DEFAULT, LINE_COLOR)), GRID_COLOR_ALPHA));
                }

                // Draw horizontal grid lines
                for (int i = 0; i < linesH; i++)
                {
                    Rectangle lineH = { bounds.x, bounds.y + spacing*i/subdivs, bounds.width, 1 };
                    GuiDrawRectangle(lineH, 0, BLANK, ((i%subdivs) == 0) ? Fade(GetColor(GuiGetStyle(DEFAULT, LINE_COLOR)), GRID_COLOR_ALPHA*4) : Fade(GetColor(GuiGetStyle(DEFAULT, LINE_COLOR)), GRID_COLOR_ALPHA));
                }
            }
        } break;
        default: break;
    }

    return currentCell;
}

//----------------------------------------------------------------------------------
// Styles loading functions
//----------------------------------------------------------------------------------

// Load raygui style file (.rgs)
void GuiLoadStyle(const char *fileName)
{
    bool tryBinary = false;

    // Try reading the files as text file first
    FILE *rgsFile = fopen(fileName, "rt");

    if (rgsFile != NULL)
    {
        char buffer[256] = { 0 };
        fgets(buffer, 256, rgsFile);

        if (buffer[0] == '#')
        {
            int controlId = 0;
            int propertyId = 0;
            unsigned int propertyValue = 0;

            while (!feof(rgsFile))
            {
                switch (buffer[0])
                {
                    case 'p':
                    {
                        // Style property: p <control_id> <property_id> <property_value> <property_name>

                        sscanf(buffer, "p %d %d 0x%x", &controlId, &propertyId, &propertyValue);

                        GuiSetStyle(controlId, propertyId, (int)propertyValue);

                    } break;
                    case 'f':
                    {
                        // Style font: f <gen_font_size> <charmap_file> <font_file>

                        int fontSize = 0;
                        char charmapFileName[256] = { 0 };
                        char fontFileName[256] = { 0 };
                        sscanf(buffer, "f %d %s %[^\r\n]s", &fontSize, charmapFileName, fontFileName);

                        TextFont font = { 0 };

                        if (charmapFileName[0] != '0')
                        {
                            // Load characters from charmap file,
                            // expected '\n' separated list of integer values
                            char *charValues = LoadFileText(charmapFileName);
                            if (charValues != NULL)
                            {
                                int glyphCount = 0;
                                const char **chars = TextSplit(charValues, '\n', &glyphCount);

                                int *values = (int *)RAYGUI_MALLOC(glyphCount*sizeof(int));
                                for (int i = 0; i < glyphCount; i++) values[i] = TextToInteger(chars[i]);

                                font = LoadFontEx(TextFormat("%s/%s", GetDirectoryPath(fileName), fontFileName), fontSize, values, glyphCount);

                                RAYGUI_FREE(values);
                            }
                        }
                        else font = LoadFontEx(TextFormat("%s/%s", GetDirectoryPath(fileName), fontFileName), fontSize, NULL, 0);

                        if ((font.texture.id > 0) && (font.charsCount > 0)) GuiSetFont(font);

                    } break;
                    default: break;
                }

                fgets(buffer, 256, rgsFile);
            }
        }
        else tryBinary = true;

        fclose(rgsFile);
    }

    if (tryBinary)
    {
        rgsFile = fopen(fileName, "rb");

        if (rgsFile == NULL) return;

        char signature[5] = "";
        short version = 0;
        short reserved = 0;
        int propertyCount = 0;

        fread(signature, 1, 4, rgsFile);
        fread(&version, 1, sizeof(short), rgsFile);
        fread(&reserved, 1, sizeof(short), rgsFile);
        fread(&propertyCount, 1, sizeof(int), rgsFile);

        if ((signature[0] == 'r') &&
            (signature[1] == 'G') &&
            (signature[2] == 'S') &&
            (signature[3] == ' '))
        {
            short controlId = 0;
            short propertyId = 0;
            int propertyValue = 0;

            for (int i = 0; i < propertyCount; i++)
            {
                fread(&controlId, 1, sizeof(short), rgsFile);
                fread(&propertyId, 1, sizeof(short), rgsFile);
                fread(&propertyValue, 1, sizeof(int), rgsFile);

                if (controlId == 0) // DEFAULT control
                {
                    // If a DEFAULT property is loaded, it is propagated to all controls
                    // NOTE: All DEFAULT properties should be defined first in the file
                    GuiSetStyle(0, (int)propertyId, propertyValue);

                    if (propertyId < RAYGUI_MAX_PROPS_BASE) for (int i = 1; i < RAYGUI_MAX_CONTROLS; i++) GuiSetStyle(i, (int)propertyId, propertyValue);
                }
                else GuiSetStyle((int)controlId, (int)propertyId, propertyValue);
            }

            // Font loading is highly dependant on raylib API to load font data and image
/*#if !defined(RAYGUI_STANDALONE)
            // Load custom font if available
            int fontDataSize = 0;
            fread(&fontDataSize, 1, sizeof(int), rgsFile);

            if (fontDataSize > 0)
            {
                Font font = { 0 };
                int fontType = 0;   // 0-Normal, 1-SDF
                Rectangle whiteRec = { 0 };

                fread(&font.baseSize, 1, sizeof(int), rgsFile);
                fread(&font.glyphCount, 1, sizeof(int), rgsFile);
                fread(&fontType, 1, sizeof(int), rgsFile);

                // Load font white rectangle
                fread(&whiteRec, 1, sizeof(Rectangle), rgsFile);

                // Load font image parameters
                int fontImageSize = 0;
                fread(&fontImageSize, 1, sizeof(int), rgsFile);

                if (fontImageSize > 0)
                {
                    Image imFont = { 0 };
                    imFont.mipmaps = 1;
                    fread(&imFont.width, 1, sizeof(int), rgsFile);
                    fread(&imFont.height, 1, sizeof(int), rgsFile);
                    fread(&imFont.format, 1, sizeof(int), rgsFile);

                    imFont.data = (unsigned char *)RAYGUI_MALLOC(fontImageSize);
                    fread(imFont.data, 1, fontImageSize, rgsFile);

                    font.texture = LoadTextureFromImage(imFont);

                    RAYGUI_FREE(imFont.data);
                }

                // Load font recs data
                font.recs = (Rectangle *)RAYGUI_CALLOC(font.glyphCount, sizeof(Rectangle));
                for (int i = 0; i < font.glyphCount; i++) fread(&font.recs[i], 1, sizeof(Rectangle), rgsFile);

                // Load font chars info data
                font.glyphs = (GlyphInfo *)RAYGUI_CALLOC(font.glyphCount, sizeof(GlyphInfo));
                for (int i = 0; i < font.glyphCount; i++)
                {
                    fread(&font.glyphs[i].value, 1, sizeof(int), rgsFile);
                    fread(&font.glyphs[i].offsetX, 1, sizeof(int), rgsFile);
                    fread(&font.glyphs[i].offsetY, 1, sizeof(int), rgsFile);
                    fread(&font.glyphs[i].advanceX, 1, sizeof(int), rgsFile);
                }

                GuiSetFont(font);

                // Set font texture source rectangle to be used as white texture to draw shapes
                // NOTE: This way, all gui can be draw using a single draw call
                if ((whiteRec.width != 0) && (whiteRec.height != 0)) SetShapesTexture(font.texture, whiteRec);
            }
#endif*/
        }

        fclose(rgsFile);
    }
}

// Load style default over global style
void GuiLoadStyleDefault(void)
{
    // We set this variable first to avoid cyclic function calls
    // when calling GuiSetStyle() and GuiGetStyle()
    guiStyleLoaded = true;

    // Initialize default LIGHT style property values
    GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, 0x838383ff);
    GuiSetStyle(DEFAULT, BASE_COLOR_NORMAL, 0xc9c9c9ff);
    GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, 0x686868ff);
    GuiSetStyle(DEFAULT, BORDER_COLOR_FOCUSED, 0x5bb2d9ff);
    GuiSetStyle(DEFAULT, BASE_COLOR_FOCUSED, 0xc9effeff);
    GuiSetStyle(DEFAULT, TEXT_COLOR_FOCUSED, 0x6c9bbcff);
    GuiSetStyle(DEFAULT, BORDER_COLOR_PRESSED, 0x0492c7ff);
    GuiSetStyle(DEFAULT, BASE_COLOR_PRESSED, 0x97e8ffff);
    GuiSetStyle(DEFAULT, TEXT_COLOR_PRESSED, 0x368bafff);
    GuiSetStyle(DEFAULT, BORDER_COLOR_DISABLED, 0xb5c1c2ff);
    GuiSetStyle(DEFAULT, BASE_COLOR_DISABLED, 0xe6e9e9ff);
    GuiSetStyle(DEFAULT, TEXT_COLOR_DISABLED, 0xaeb7b8ff);
    GuiSetStyle(DEFAULT, BORDER_WIDTH, 1);                       // WARNING: Some controls use other values
    GuiSetStyle(DEFAULT, TEXT_PADDING, 0);                       // WARNING: Some controls use other values
    GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_CENTER); // WARNING: Some controls use other values

    // Initialize control-specific property values
    // NOTE: Those properties are in default list but require specific values by control type
    GuiSetStyle(LABEL, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_LEFT);
    GuiSetStyle(BUTTON, BORDER_WIDTH, 2);
    GuiSetStyle(SLIDER, TEXT_PADDING, 5);
    GuiSetStyle(CHECKBOX, TEXT_PADDING, 5);
    GuiSetStyle(CHECKBOX, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_RIGHT);
    GuiSetStyle(TEXTBOX, TEXT_PADDING, 5);
    GuiSetStyle(TEXTBOX, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_LEFT);
    GuiSetStyle(VALUEBOX, TEXT_PADDING, 4);
    GuiSetStyle(VALUEBOX, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_LEFT);
    GuiSetStyle(SPINNER, TEXT_PADDING, 4);
    GuiSetStyle(SPINNER, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_LEFT);
    GuiSetStyle(STATUSBAR, TEXT_PADDING, 6);
    GuiSetStyle(STATUSBAR, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_LEFT);

    // Initialize extended property values
    // NOTE: By default, extended property values are initialized to 0
    GuiSetStyle(DEFAULT, TEXT_SIZE, 10);                // DEFAULT, shared by all controls
    GuiSetStyle(DEFAULT, TEXT_SPACING, 1);              // DEFAULT, shared by all controls
    GuiSetStyle(DEFAULT, LINE_COLOR, 0x90abb5ff);       // DEFAULT specific property
    GuiSetStyle(DEFAULT, BACKGROUND_COLOR, 0xf5f5f5ff); // DEFAULT specific property
    GuiSetStyle(TOGGLE, GROUP_PADDING, 2);
    GuiSetStyle(SLIDER, SLIDER_WIDTH, 15);
    GuiSetStyle(SLIDER, SLIDER_PADDING, 1);
    GuiSetStyle(PROGRESSBAR, PROGRESS_PADDING, 1);
    GuiSetStyle(CHECKBOX, CHECK_PADDING, 1);
    GuiSetStyle(COMBOBOX, COMBO_BUTTON_WIDTH, 30);
    GuiSetStyle(COMBOBOX, COMBO_BUTTON_PADDING, 2);
    GuiSetStyle(DROPDOWNBOX, ARROW_PADDING, 16);
    GuiSetStyle(DROPDOWNBOX, DROPDOWN_ITEMS_PADDING, 2);
    GuiSetStyle(TEXTBOX, TEXT_LINES_PADDING, 5);
    GuiSetStyle(TEXTBOX, TEXT_INNER_PADDING, 4);
    GuiSetStyle(TEXTBOX, COLOR_SELECTED_FG, 0xf0fffeff);
    GuiSetStyle(TEXTBOX, COLOR_SELECTED_BG, 0x839affe0);
    GuiSetStyle(SPINNER, SPIN_BUTTON_WIDTH, 20);
    GuiSetStyle(SPINNER, SPIN_BUTTON_PADDING, 2);
    GuiSetStyle(SCROLLBAR, BORDER_WIDTH, 0);
    GuiSetStyle(SCROLLBAR, ARROWS_VISIBLE, 0);
    GuiSetStyle(SCROLLBAR, ARROWS_SIZE, 6);
    GuiSetStyle(SCROLLBAR, SCROLL_SLIDER_PADDING, 0);
    GuiSetStyle(SCROLLBAR, SCROLL_SLIDER_SIZE, 16);
    GuiSetStyle(SCROLLBAR, SCROLL_PADDING, 0);
    GuiSetStyle(SCROLLBAR, SCROLL_SPEED, 10);
    GuiSetStyle(LISTVIEW, LIST_ITEMS_HEIGHT, 0x1e);
    GuiSetStyle(LISTVIEW, LIST_ITEMS_PADDING, 2);
    GuiSetStyle(LISTVIEW, SCROLLBAR_WIDTH, 10);
    GuiSetStyle(LISTVIEW, SCROLLBAR_SIDE, SCROLLBAR_RIGHT_SIDE);
    GuiSetStyle(COLORPICKER, COLOR_SELECTOR_SIZE, 6);
    GuiSetStyle(COLORPICKER, HUEBAR_WIDTH, 0x14);
    GuiSetStyle(COLORPICKER, HUEBAR_PADDING, 0xa);
    GuiSetStyle(COLORPICKER, HUEBAR_SELECTOR_HEIGHT, 6);
    GuiSetStyle(COLORPICKER, HUEBAR_SELECTOR_OVERFLOW, 2);

    guiFont = GetFontDefault();     // Initialize default font
}

// Get text with icon id prepended
// NOTE: Useful to add icons by name id (enum) instead of
// a number that can change between ricon versions
const char *GuiIconText(int iconId, const char *text)
{
#if defined(RAYGUI_NO_RICONS)
    return NULL;
#else
    static char buffer[1024] = { 0 };
    memset(buffer, 0, 1024);

    sprintf(buffer, "#%03i#", iconId);

    if (text != NULL)
    {
        for (int i = 5; i < 1024; i++)
        {
            buffer[i] = text[i - 5];
            if (text[i - 5] == '\0') break;
        }
    }

    return buffer;
#endif
}

#if !defined(RAYGUI_NO_RICONS)

// Get full icons data pointer
unsigned int *GuiGetIcons(void) { return guiIcons; }

// Load raygui icons file (.rgi)
// NOTE: In case nameIds are required, they can be requested with loadIconsName,
// they are returned as a guiIconsName[iconCount][RICON_MAX_NAME_LENGTH],
// WARNING: guiIconsName[]][] memory should be manually freed!
char **GuiLoadIcons(const char *fileName, bool loadIconsName)
{
    // Style File Structure (.rgi)
    // ------------------------------------------------------
    // Offset  | Size    | Type       | Description
    // ------------------------------------------------------
    // 0       | 4       | char       | Signature: "rGI "
    // 4       | 2       | short      | Version: 100
    // 6       | 2       | short      | reserved

    // 8       | 2       | short      | Num icons (N)
    // 10      | 2       | short      | Icons size (Options: 16, 32, 64) (S)

    // Icons name id (32 bytes per name id)
    // foreach (icon)
    // {
    //   12+32*i  | 32   | char       | Icon NameId
    // }

    // Icons data: One bit per pixel, stored as unsigned int array (depends on icon size)
    // S*S pixels/32bit per unsigned int = K unsigned int per icon
    // foreach (icon)
    // {
    //   ...   | K       | unsigned int | Icon Data
    // }

    FILE *rgiFile = fopen(fileName, "rb");

    char **guiIconsName = NULL;

    if (rgiFile != NULL)
    {
        char signature[5] = "";
        short version = 0;
        short reserved = 0;
        short iconCount = 0;
        short iconSize = 0;

        fread(signature, 1, 4, rgiFile);
        fread(&version, 1, sizeof(short), rgiFile);
        fread(&reserved, 1, sizeof(short), rgiFile);
        fread(&iconCount, 1, sizeof(short), rgiFile);
        fread(&iconSize, 1, sizeof(short), rgiFile);

        if ((signature[0] == 'r') &&
            (signature[1] == 'G') &&
            (signature[2] == 'I') &&
            (signature[3] == ' '))
        {
            if (loadIconsName)
            {
                guiIconsName = (char **)RAYGUI_MALLOC(iconCount*sizeof(char **));
                for (int i = 0; i < iconCount; i++)
                {
                    guiIconsName[i] = (char *)RAYGUI_MALLOC(RICON_MAX_NAME_LENGTH);
                    fread(guiIconsName[i], RICON_MAX_NAME_LENGTH, 1, rgiFile);
                }
            }
            else fseek(rgiFile, iconCount*RICON_MAX_NAME_LENGTH, SEEK_CUR);

            // Read icons data directly over guiIcons data array
            fread(guiIcons, iconCount*(iconSize*iconSize/32), sizeof(unsigned int), rgiFile);
        }

        fclose(rgiFile);
    }

    return guiIconsName;
}

// Draw selected icon using rectangles pixel-by-pixel
void GuiDrawIcon(int iconId, int posX, int posY, int pixelSize, Color color)
{
    #define BIT_CHECK(a,b) ((a) & (1<<(b)))

    for (int i = 0, y = 0; i < RICON_SIZE*RICON_SIZE/32; i++)
    {
        for (int k = 0; k < 32; k++)
        {
            if (BIT_CHECK(guiIcons[iconId*RICON_DATA_ELEMENTS + i], k))
            {
            #if !defined(RAYGUI_STANDALONE)
                DrawRectangle(posX + (k%RICON_SIZE)*pixelSize, posY + y*pixelSize, pixelSize, pixelSize, color);
            #endif
            }

            if ((k == 15) || (k == 31)) y++;
        }
    }
}

// Get icon bit data
// NOTE: Bit data array grouped as unsigned int (ICON_SIZE*ICON_SIZE/32 elements)
unsigned int *GuiGetIconData(int iconId)
{
    static unsigned int iconData[RICON_DATA_ELEMENTS] = { 0 };
    memset(iconData, 0, RICON_DATA_ELEMENTS*sizeof(unsigned int));

    if (iconId < RICON_MAX_ICONS) memcpy(iconData, &guiIcons[iconId*RICON_DATA_ELEMENTS], RICON_DATA_ELEMENTS*sizeof(unsigned int));

    return iconData;
}

// Set icon bit data
// NOTE: Data must be provided as unsigned int array (ICON_SIZE*ICON_SIZE/32 elements)
void GuiSetIconData(int iconId, unsigned int *data)
{
    if (iconId < RICON_MAX_ICONS) memcpy(&guiIcons[iconId*RICON_DATA_ELEMENTS], data, RICON_DATA_ELEMENTS*sizeof(unsigned int));
}

// Set icon pixel value
void GuiSetIconPixel(int iconId, int x, int y)
{
    #define BIT_SET(a,b)   ((a) |= (1<<(b)))

    // This logic works for any RICON_SIZE pixels icons,
    // For example, in case of 16x16 pixels, every 2 lines fit in one unsigned int data element
    BIT_SET(guiIcons[iconId*RICON_DATA_ELEMENTS + y/(sizeof(unsigned int)*8/RICON_SIZE)], x + (y%(sizeof(unsigned int)*8/RICON_SIZE)*RICON_SIZE));
}

// Clear icon pixel value
void GuiClearIconPixel(int iconId, int x, int y)
{
    #define BIT_CLEAR(a,b) ((a) &= ~((1)<<(b)))

    // This logic works for any RICON_SIZE pixels icons,
    // For example, in case of 16x16 pixels, every 2 lines fit in one unsigned int data element
    BIT_CLEAR(guiIcons[iconId*RICON_DATA_ELEMENTS + y/(sizeof(unsigned int)*8/RICON_SIZE)], x + (y%(sizeof(unsigned int)*8/RICON_SIZE)*RICON_SIZE));
}

// Check icon pixel value
bool GuiCheckIconPixel(int iconId, int x, int y)
{
    #define BIT_CHECK(a,b) ((a) & (1<<(b)))

    return (BIT_CHECK(guiIcons[iconId*8 + y/2], x + (y%2*16)));
}
#endif      // !RAYGUI_NO_RICONS

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------
// Gui get text width using default font
// NOTE: Icon is not considered here
static int GetTextWidth(const char *text)
{
    Vector2 size ;

    if ((text != NULL) && (text[0] != '\0'))
    {
        size = MeasureTextEx(guiFont, text, (float)GuiGetStyle(DEFAULT, TEXT_SIZE), (float)GuiGetStyle(DEFAULT, TEXT_SPACING));
    }

    return (int)size.x;
}

// Get text bounds considering control bounds
static Rectangle GetTextBounds(int control, Rectangle bounds)
{
    Rectangle textBounds = bounds;

    textBounds.x = bounds.x + GuiGetStyle(control, BORDER_WIDTH);
    textBounds.y = bounds.y + GuiGetStyle(control, BORDER_WIDTH);
    textBounds.width = bounds.width - 2*GuiGetStyle(control, BORDER_WIDTH);
    textBounds.height = bounds.height - 2*GuiGetStyle(control, BORDER_WIDTH);

    // Consider TEXT_PADDING properly, depends on control type and TEXT_ALIGNMENT
    switch (control)
    {
        case COMBOBOX: bounds.width -= (GuiGetStyle(control, COMBO_BUTTON_WIDTH) + GuiGetStyle(control, COMBO_BUTTON_PADDING)); break;
        case VALUEBOX: break;   // NOTE: ValueBox text value always centered, text padding applies to label
        default:
        {
            if (GuiGetStyle(control, TEXT_ALIGNMENT) == GUI_TEXT_ALIGN_RIGHT) textBounds.x -= GuiGetStyle(control, TEXT_PADDING);
            else textBounds.x += GuiGetStyle(control, TEXT_PADDING);
        } break;
    }

    // TODO: Special cases (no label): COMBOBOX, DROPDOWNBOX, LISTVIEW (scrollbar?)
    // More special cases (label on side): CHECKBOX, SLIDER, VALUEBOX, SPINNER

    return textBounds;
}

// Get text icon if provided and move text cursor
// NOTE: We support up to 999 values for iconId
static const char *GetTextIcon(const char *text, int *iconId)
{
#if !defined(RAYGUI_NO_RICONS)
    *iconId = -1;
    if (text[0] == '#')     // Maybe we have an icon!
    {
        char iconValue[4] = { 0 };  // Maximum length for icon value: 3 digits + '\0'

        int pos = 1;
        while ((pos < 4) && (text[pos] >= '0') && (text[pos] <= '9'))
        {
            iconValue[pos - 1] = text[pos];
            pos++;
        }

        if (text[pos] == '#')
        {
            *iconId = TextToInteger(iconValue);

            // Move text pointer after icon
            // WARNING: If only icon provided, it could point to EOL character: '\0'
            if (*iconId >= 0) text += (pos + 1);
        }
    }
#endif

    return text;
}

// Gui draw text using default font
static void GuiDrawText(const char *text, Rectangle bounds, int alignment, Color tint)
{
    #define TEXT_VALIGN_PIXEL_OFFSET(h)  ((int)h%2)     // Vertical alignment for pixel perfect

    if ((text != NULL) && (text[0] != '\0'))
    {
        int iconId = 0;
        text = GetTextIcon(text, &iconId);              // Check text for icon and move cursor

        // Get text position depending on alignment and iconId
        //---------------------------------------------------------------------------------
        #define RICON_TEXT_PADDING   4

        Vector2 position = { bounds.x, bounds.y };

        // NOTE: We get text size after icon has been processed
        int textWidth = GetTextWidth(text);
        int textHeight = GuiGetStyle(DEFAULT, TEXT_SIZE);

        // If text requires an icon, add size to measure
        if (iconId >= 0)
        {
            textWidth += RICON_SIZE;

            // WARNING: If only icon provided, text could be pointing to EOF character: '\0'
            if ((text != NULL) && (text[0] != '\0')) textWidth += RICON_TEXT_PADDING;
        }

        // Check guiTextAlign global variables
        switch (alignment)
        {
            case GUI_TEXT_ALIGN_LEFT:
            {
                position.x = bounds.x;
                position.y = bounds.y + bounds.height/2 - textHeight/2 + TEXT_VALIGN_PIXEL_OFFSET(bounds.height);
            } break;
            case GUI_TEXT_ALIGN_CENTER:
            {
                position.x = bounds.x + bounds.width/2 - textWidth/2;
                position.y = bounds.y + bounds.height/2 - textHeight/2 + TEXT_VALIGN_PIXEL_OFFSET(bounds.height);
            } break;
            case GUI_TEXT_ALIGN_RIGHT:
            {
                position.x = bounds.x + bounds.width - textWidth;
                position.y = bounds.y + bounds.height/2 - textHeight/2 + TEXT_VALIGN_PIXEL_OFFSET(bounds.height);
            } break;
            default: break;
        }

        // NOTE: Make sure we get pixel-perfect coordinates,
        // In case of decimals we got weird text positioning
        position.x = (float)((int)position.x);
        position.y = (float)((int)position.y);
        //---------------------------------------------------------------------------------

        // Draw text (with icon if available)
        //---------------------------------------------------------------------------------
#if !defined(RAYGUI_NO_RICONS)
        if (iconId >= 0)
        {
            // NOTE: We consider icon height, probably different than text size
            GuiDrawIcon(iconId, (int)position.x, (int)(bounds.y + bounds.height/2 - RICON_SIZE/2 + TEXT_VALIGN_PIXEL_OFFSET(bounds.height)), 1, tint);
            position.x += (RICON_SIZE + RICON_TEXT_PADDING);
        }
#endif
        DrawTextEx(guiFont, text, position, (float)GuiGetStyle(DEFAULT, TEXT_SIZE), (float)GuiGetStyle(DEFAULT, TEXT_SPACING), tint);
        //---------------------------------------------------------------------------------
    }
}

// Gui draw rectangle using default raygui plain style with borders
static void GuiDrawRectangle(Rectangle rec, int borderWidth, Color borderColor, Color color)
{
    if (color.a > 0)
    {
        // Draw rectangle filled with color
        DrawRectangle((int)rec.x, (int)rec.y, (int)rec.width, (int)rec.height, color);
    }

    if (borderWidth > 0)
    {
        // Draw rectangle border lines with color
        DrawRectangle((int)rec.x, (int)rec.y, (int)rec.width, borderWidth, borderColor);
        DrawRectangle((int)rec.x, (int)rec.y + borderWidth, borderWidth, (int)rec.height - 2*borderWidth, borderColor);
        DrawRectangle((int)rec.x + (int)rec.width - borderWidth, (int)rec.y + borderWidth, borderWidth, (int)rec.height - 2*borderWidth, borderColor);
        DrawRectangle((int)rec.x, (int)rec.y + (int)rec.height - borderWidth, (int)rec.width, borderWidth, borderColor);
    }
}

// Split controls text into multiple strings
// Also check for multiple columns (required by GuiToggleGroup())
static const char **GuiTextSplit(const char *text, int *count, int *textRow)
{
    // NOTE: Current implementation returns a copy of the provided string with '\0' (string end delimiter)
    // inserted between strings defined by "delimiter" parameter. No memory is dynamically allocated,
    // all used memory is static... it has some limitations:
    //      1. Maximum number of possible split strings is set by TEXTSPLIT_MAX_TEXT_ELEMENTS
    //      2. Maximum size of text to split is TEXTSPLIT_MAX_TEXT_LENGTH
    // NOTE: Those definitions could be externally provided if required

    #if !defined(TEXTSPLIT_MAX_TEXT_LENGTH)
        #define TEXTSPLIT_MAX_TEXT_LENGTH      1024
    #endif

    #if !defined(TEXTSPLIT_MAX_TEXT_ELEMENTS)
        #define TEXTSPLIT_MAX_TEXT_ELEMENTS     128
    #endif

    static const char *result[TEXTSPLIT_MAX_TEXT_ELEMENTS] = { NULL };
    static char buffer[TEXTSPLIT_MAX_TEXT_LENGTH] = { 0 };
    memset(buffer, 0, TEXTSPLIT_MAX_TEXT_LENGTH);

    result[0] = buffer;
    int counter = 1;

    if (textRow != NULL) textRow[0] = 0;

    // Count how many substrings we have on text and point to every one
    for (int i = 0; i < TEXTSPLIT_MAX_TEXT_LENGTH; i++)
    {
        buffer[i] = text[i];
        if (buffer[i] == '\0') break;
        else if ((buffer[i] == ';') || (buffer[i] == '\n'))
        {
            result[counter] = buffer + i + 1;

            if (textRow != NULL)
            {
                if (buffer[i] == '\n') textRow[counter] = textRow[counter - 1] + 1;
                else textRow[counter] = textRow[counter - 1];
            }

            buffer[i] = '\0';   // Set an end of string at this point

            counter++;
            if (counter == TEXTSPLIT_MAX_TEXT_ELEMENTS) break;
        }
    }

    *count = counter;

    return result;
}


double GuiDMValueBox(Rectangle bounds, double value, double minValue, double maxValue, int precision, bool editMode) {
    // FIXME: Hope all those `memmove()` functions are correctly used so we won't leak memory or overflow the buffer !!!
    static int framesCounter = 0;           // Required for blinking cursor
    static int cursor = 0;                  // Required for tracking the cursor position (only for a single active valuebox)

    enum {cursorTimer = 6, maxChars = 31, textPadding = 2};

    int state = GuiGetState();

    // Make sure value is in range
    if(maxValue != minValue){
        if(value < minValue) value = minValue;
        if(value > maxValue) value = maxValue;
    }

    char textValue[maxChars + 1] = "\0";
    snprintf(textValue, maxChars, "%.*f", precision, value); // NOTE: use `snprintf` here so we don't overflow the buffer
    int len = strlen(textValue);

    bool valueHasChanged = false;

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && !guiLocked)
    {
        if (editMode)
        {
            // Make sure cursor position is correct
            if(cursor > len) cursor = len;
            if(cursor < 0) cursor = 0;

            state = GUI_STATE_PRESSED;
            framesCounter++;

            if(IsKeyPressed(KEY_RIGHT) || (IsKeyDown(KEY_RIGHT) && (framesCounter%cursorTimer == 0))) {
                // MOVE CURSOR TO RIGHT
                ++cursor;
                framesCounter = 0;
            } else if(IsKeyPressed(KEY_LEFT) || (IsKeyDown(KEY_LEFT) && (framesCounter%cursorTimer == 0))) {
                // MOVE CURSOR TO LEFT
                --cursor;
                framesCounter = 0;
            } else if (IsKeyPressed(KEY_BACKSPACE) || (IsKeyDown(KEY_BACKSPACE) && (framesCounter%cursorTimer) == 0)) {
                // HANDLE BACKSPACE
                if(cursor > 0) {
                    if(textValue[cursor-1] != '.') {
                        if(cursor < len )
                            memmove(&textValue[cursor-1], &textValue[cursor], len-cursor);
                        textValue[len - 1] = '\0';
                        valueHasChanged = true;
                    }
                    --cursor;
                }
                framesCounter = 0;
            } else if (IsKeyPressed(KEY_DELETE) || (IsKeyDown(KEY_DELETE) && (framesCounter%cursorTimer) == 0)) {
                // HANDLE DEL
                if(len > 0 && cursor < len && textValue[cursor] != '.') {
                    memmove(&textValue[cursor], &textValue[cursor+1], len-cursor);
                    textValue[len] = '\0';
                    len -= 1;
                    valueHasChanged = true;
                }
            } else if (IsKeyPressed(KEY_HOME)) {
                // MOVE CURSOR TO START
                cursor = 0;
            } else if (IsKeyPressed(KEY_END)) {
                // MOVE CURSOR TO END
                cursor = len;
            }  else if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_C)) {
                // COPY
                SetClipboardText(textValue);
            } else if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_X)) {
                // CUT
                SetClipboardText(textValue);
                textValue[0] = '\0';
                cursor = len = 0;
                value = 0.0; // set it to 0 and pretend the value didn't change
            } else if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_V)) {
                // PASTE
                const char* clip = GetClipboardText();
                int clipLen = strlen(clip);
                clipLen = clipLen > maxChars ? maxChars : clipLen;
                memcpy(textValue, clip, clipLen);
                len = clipLen;
                textValue[len] = '\0';
                valueHasChanged = true;
            }
            else {
                // HANDLE KEY PRESS
                int key = GetKeyPressed();
                if( ((len < maxChars) && (key >= 48) && (key <= 57)) || (key == 46) || (key == 45)  ) // only allow 0..9, minus(-) and dot(.)
                {
                    if(precision != 0 && cursor < len) { // when we have decimals we can't insert at the end
                        memmove(&textValue[cursor], &textValue[cursor-1], len+1-cursor);
                        textValue[len+1] = '\0';
                        textValue[cursor] = (char)key;
                        cursor++;
                        valueHasChanged = true;
                    }
                    else if(precision == 0) {
                        if(cursor < len) memmove(&textValue[cursor], &textValue[cursor-1], len+1-cursor);
                        len += 1;
                        textValue[len+1] = '\0';
                        textValue[cursor] = (char)key;
                        cursor++;
                        valueHasChanged = true;
                    }
                }
            }

            // Make sure cursor position is correct
            if(cursor > len) cursor = len;
            if(cursor < 0) cursor = 0;
        }
        else
        {
            if (CheckCollisionPointRec(GetMousePosition(), bounds))
            {
                state = GUI_STATE_FOCUSED;
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) framesCounter = 0;
            }
        }
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    DrawRectangleLinesEx(bounds, GuiGetStyle(VALUEBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(VALUEBOX, BORDER + (state*3))), guiAlpha));

    Rectangle textBounds = {bounds.x + GuiGetStyle(VALUEBOX, BORDER_WIDTH) + textPadding, bounds.y + GuiGetStyle(VALUEBOX, BORDER_WIDTH),
        bounds.width - 2*(GuiGetStyle(VALUEBOX, BORDER_WIDTH) + textPadding), bounds.height - 2*GuiGetStyle(VALUEBOX, BORDER_WIDTH)};

    int textWidth = GetTextWidth(textValue);
    if(textWidth > textBounds.width) textBounds.width = textWidth;

    if (state == GUI_STATE_PRESSED)
    {
        DrawRectangle(bounds.x + GuiGetStyle(VALUEBOX, BORDER_WIDTH), bounds.y + GuiGetStyle(VALUEBOX, BORDER_WIDTH), bounds.width - 2*GuiGetStyle(VALUEBOX, BORDER_WIDTH), bounds.height - 2*GuiGetStyle(VALUEBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(VALUEBOX, BASE_COLOR_PRESSED)), guiAlpha));

        // Draw blinking cursor
        // NOTE: ValueBox internal text is always centered
        if (editMode && ((framesCounter/20)%2 == 0)) {
            // Measure text until the cursor
            int textWidthCursor = -2;
            if(cursor > 0) {
                char c = textValue[cursor];
                textValue[cursor] = '\0';
                textWidthCursor = GetTextWidth(textValue);
                textValue[cursor] = c;
            }
            //DrawRectangle(bounds.x + textWidthCursor + textPadding + 2, bounds.y + 2*GuiGetStyle(VALUEBOX, BORDER_WIDTH), 1, bounds.height - 4*GuiGetStyle(VALUEBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(VALUEBOX, BORDER_COLOR_PRESSED)), guiAlpha));
            DrawRectangle(bounds.x + textWidthCursor + (int)((bounds.width - textWidth - textPadding)/2.0f) + 2, bounds.y + 2*GuiGetStyle(VALUEBOX, BORDER_WIDTH), 1, bounds.height - 4*GuiGetStyle(VALUEBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(VALUEBOX, BORDER_COLOR_PRESSED)), guiAlpha));
        }
    }
    else if (state == GUI_STATE_DISABLED)
    {
        DrawRectangle(bounds.x + GuiGetStyle(VALUEBOX, BORDER_WIDTH), bounds.y + GuiGetStyle(VALUEBOX, BORDER_WIDTH), bounds.width - 2*GuiGetStyle(VALUEBOX, BORDER_WIDTH), bounds.height - 2*GuiGetStyle(VALUEBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(VALUEBOX, BASE_COLOR_DISABLED)), guiAlpha));
    }

    GuiDrawText(textValue, textBounds, GUI_TEXT_ALIGN_CENTER, Fade(GetColor(GuiGetStyle(VALUEBOX, TEXT + (state*3))), guiAlpha));

    value = valueHasChanged ? strtod(textValue, NULL) : value;

    // Make sure value is in range
    if(maxValue != minValue){
        if(value < minValue) value = minValue;
        if(value > maxValue) value = maxValue;
    }

    return value;
}



double GuiDMSpinner(Rectangle bounds, double value, double minValue, double maxValue, double step, int precision, bool editMode) {
    int state = GuiGetState();

    Rectangle spinner = { bounds.x + GuiGetStyle(SPINNER, SPIN_BUTTON_WIDTH) + GuiGetStyle(SPINNER, SPIN_BUTTON_PADDING), bounds.y,
                          bounds.width - 2*(GuiGetStyle(SPINNER, SPIN_BUTTON_WIDTH) + GuiGetStyle(SPINNER, SPIN_BUTTON_PADDING)), bounds.height };
    Rectangle leftButtonBound = { (float)bounds.x, (float)bounds.y, (float)GuiGetStyle(SPINNER, SPIN_BUTTON_WIDTH), (float)bounds.height };
    Rectangle rightButtonBound = { (float)bounds.x + bounds.width - GuiGetStyle(SPINNER, SPIN_BUTTON_WIDTH), (float)bounds.y, (float)GuiGetStyle(SPINNER, SPIN_BUTTON_WIDTH), (float)bounds.height };

    // Update control
    //--------------------------------------------------------------------
    if ((state != GUI_STATE_DISABLED) && !guiLocked)
    {
        Vector2 mousePoint = GetMousePosition();

        // Check spinner state
        if (CheckCollisionPointRec(mousePoint, bounds))
        {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) state = GUI_STATE_PRESSED;
            else state = GUI_STATE_FOCUSED;
        }
    }
    //--------------------------------------------------------------------


    // Draw control
    //--------------------------------------------------------------------
    // Draw value selector custom buttons
    // NOTE: BORDER_WIDTH and TEXT_ALIGNMENT forced values
    int tempBorderWidth = GuiGetStyle(BUTTON, BORDER_WIDTH);
    int tempTextAlign = GuiGetStyle(BUTTON, TEXT_ALIGNMENT);
    GuiSetStyle(BUTTON, BORDER_WIDTH, GuiGetStyle(SPINNER, BORDER_WIDTH));
    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_CENTER);

#if defined(RAYGUI_SUPPORT_RICONS)
    if (GuiButton(leftButtonBound, GuiIconText(RICON_ARROW_LEFT_FILL, NULL))) value -= step;
    if (GuiButton(rightButtonBound, GuiIconText(RICON_ARROW_RIGHT_FILL, NULL))) value += step;
#else
    if (GuiButton(leftButtonBound, "<")) value -= step;
    if (GuiButton(rightButtonBound, ">")) value += step;
#endif

    GuiSetStyle(BUTTON, TEXT_ALIGNMENT, tempTextAlign);
    GuiSetStyle(BUTTON, BORDER_WIDTH, tempBorderWidth);

    value = GuiDMValueBox(spinner, value, minValue, maxValue, precision, editMode);

    return value;
}



void GuiDMPropertyList(Rectangle bounds, GuiDMProperty* props, int count, int* focus, int* scrollIndex) {
    #ifdef RAYGUI_SUPPORT_RICONS
    #define PROPERTY_COLLAPSED_ICON "#120#"
    #define PROPERTY_EXPANDED_ICON "#121#"
    #else
    #define PROPERTY_COLLAPSED_ICON "+"
    #define PROPERTY_EXPANDED_ICON "-"
    #endif

    #define PROPERTY_PADDING 6
    #define PROPERTY_ICON_SIZE 16
    #define PROPERTY_DECIMAL_DIGITS 3  //how many digits to show (used only for the vector properties)

    // NOTE: Using ListView style for everything !!
    int state = GuiGetState();
    int propFocused = (focus == NULL)? -1 : *focus;
    int scroll = *scrollIndex > 0 ? 0 : *scrollIndex; // NOTE: scroll should always be negative or 0

    // Each property occupies a certain number of slots, highly synchronized with the properties enum (GUI_PROP_BOOL ... GUI_PROP_SECTION)
    // NOTE: If you add a custom property type make sure to add the number of slots it occupies here !!
    const int propSlots[] = {1,1,1,2,1,3,4,5,5,5,1};

    Rectangle absoluteBounds = {0}; // total bounds for all of the properties (unclipped)
    // We need to loop over all the properties to get total height so we can see if we need a scrollbar or not
    for(int p=0; p<count; ++p) {
        // Calculate height of this property (properties can occupy 1 or more slots)
        int height = GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT);
        if(props[p].type < (sizeof(propSlots)/sizeof(propSlots[0]))) {
            if(!PROP_CHECK_FLAG(&props[p], GUI_PFLAG_COLLAPSED)) height = propSlots[props[p].type]*GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT);
            if(props[p].type == GUI_PROP_SECTION && (PROP_CHECK_FLAG(&props[p], GUI_PFLAG_COLLAPSED))) p += props[p].value.vsection; // skip slots for collapsed section
        }
        absoluteBounds.height += height+1;
    }

    // Check if we need a scrollbar and adjust bounds when necesary
    bool useScrollBar = absoluteBounds.height > bounds.height - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH) ? true : false;
    if(!useScrollBar && scroll != 0) scroll = 0; // make sure scroll is 0 when there's no scrollbar

    Rectangle scrollBarBounds = {bounds.x + GuiGetStyle(LISTVIEW, BORDER_WIDTH), bounds.y + GuiGetStyle(LISTVIEW, BORDER_WIDTH),
                GuiGetStyle(LISTVIEW, SCROLLBAR_WIDTH),  bounds.height - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH)};

    absoluteBounds.x = bounds.x + GuiGetStyle(LISTVIEW, LIST_ITEMS_PADDING) + GuiGetStyle(DEFAULT, BORDER_WIDTH);
    absoluteBounds.y = bounds.y + GuiGetStyle(LISTVIEW, LIST_ITEMS_PADDING) + GuiGetStyle(DEFAULT, BORDER_WIDTH) + scroll;
    absoluteBounds.width = bounds.width - 2*(GuiGetStyle(LISTVIEW, LIST_ITEMS_PADDING) + GuiGetStyle(DEFAULT, BORDER_WIDTH));

    if(useScrollBar) {
        if(GuiGetStyle(LISTVIEW, SCROLLBAR_SIDE) == SCROLLBAR_LEFT_SIDE)
            absoluteBounds.x += GuiGetStyle(LISTVIEW, SCROLLBAR_WIDTH); // scrollbar is on the LEFT, adjust bounds
        else
            scrollBarBounds.x = bounds.x + bounds.width - GuiGetStyle(LISTVIEW, BORDER_WIDTH) - GuiGetStyle(LISTVIEW, SCROLLBAR_WIDTH); // scrollbar is on the RIGHT
        absoluteBounds.width -= GuiGetStyle(LISTVIEW, SCROLLBAR_WIDTH); // adjust width to fit the scrollbar
    }

    int maxScroll = absoluteBounds.height + 2*(GuiGetStyle(LISTVIEW, LIST_ITEMS_PADDING) + GuiGetStyle(DEFAULT, BORDER_WIDTH))-bounds.height;

    // Update control
    //--------------------------------------------------------------------
    Vector2 mousePos = GetMousePosition();
    // NOTE: most of the update code is actually done in the draw control section
    if ((state != GUI_STATE_DISABLED) && !guiLocked) {
        if(!CheckCollisionPointRec(mousePos, bounds)) {
            propFocused = -1;
        }

        if (useScrollBar)
        {
            int wheelMove = GetMouseWheelMove();
            scroll += wheelMove*count;
            if(-scroll > maxScroll) scroll = -maxScroll;
        }
    }
    //--------------------------------------------------------------------


    // Draw control
    //--------------------------------------------------------------------
    DrawRectangleRec(bounds, Fade(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)), guiAlpha) );     // Draw background
    DrawRectangleLinesEx(bounds, GuiGetStyle(DEFAULT, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(LISTVIEW, BORDER + state*3)), guiAlpha)); // Draw border

    BeginScissorMode(absoluteBounds.x, bounds.y + GuiGetStyle(DEFAULT, BORDER_WIDTH), absoluteBounds.width, bounds.height - 2*GuiGetStyle(DEFAULT, BORDER_WIDTH));
        int currentHeight = 0;
        for(int p=0; p<count; ++p)
        {
            int height = GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT);
            if(props[p].type < (sizeof(propSlots)/sizeof(propSlots[0])) && !PROP_CHECK_FLAG(&props[p], GUI_PFLAG_COLLAPSED) )
                height = propSlots[props[p].type]*GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT); // get property height based on how many slots it occupies

            // Even with scissor mode on, draw only properties that we can see (comment out both BeginScissorMode() / EndScissorMode() to see this)
            if(absoluteBounds.y + currentHeight + height >= bounds.y && absoluteBounds.y + currentHeight <= bounds.y + bounds.height)
            {
                Rectangle propBounds = {absoluteBounds.x, absoluteBounds.y + currentHeight, absoluteBounds.width, height};
                Color textColor = Fade(GetColor(GuiGetStyle(LISTVIEW, TEXT_COLOR_NORMAL)), guiAlpha);
                int propState = GUI_STATE_NORMAL;

                // Get the state of this property and do some initial drawing
                if(PROP_CHECK_FLAG(&props[p], GUI_PFLAG_DISABLED)) {
                    propState = GUI_STATE_DISABLED;
                    propBounds.height += 1;
                    DrawRectangleRec(propBounds, Fade(GetColor(GuiGetStyle(LISTVIEW, BASE_COLOR_DISABLED)), guiAlpha));
                    propBounds.height -= 1;
                    textColor = Fade(GetColor(GuiGetStyle(LISTVIEW, TEXT_COLOR_DISABLED)), guiAlpha);
                } else {
                    if(CheckCollisionPointRec(mousePos, propBounds) && !guiLocked) {
                        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                            propState = GUI_STATE_PRESSED;
                            //DrawRectangleRec(propRect, Fade(GetColor(GuiGetStyle(LISTVIEW, BASE_COLOR_PRESSED)), guiAlpha));
                            textColor = Fade(GetColor(GuiGetStyle(LISTVIEW, TEXT_COLOR_PRESSED)), guiAlpha);
                        } else {
                            propState = GUI_STATE_FOCUSED;
                            propFocused = p;
                            //DrawRectangleRec(propRect, Fade(GetColor(GuiGetStyle(LISTVIEW, BASE_COLOR_FOCUSED)), guiAlpha));
                            textColor = Fade(GetColor(GuiGetStyle(LISTVIEW, TEXT_COLOR_FOCUSED)), guiAlpha);
                        }
                    } else propState = GUI_STATE_NORMAL;
                }

                if(propState == GUI_STATE_DISABLED) GuiSetState(propState);
                switch(props[p].type)
                {
                    case GUI_PROP_BOOL: {
                        // draw property name
                        GuiDrawText(props[p].name, (Rectangle){propBounds.x + PROPERTY_PADDING, propBounds.y, propBounds.width/2-PROPERTY_PADDING, GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT)}, GUI_TEXT_ALIGN_LEFT, textColor);
                        if(propState == GUI_STATE_PRESSED) props[p].value.vbool = !props[p].value.vbool; // toggle the property value when clicked

                        // draw property value
                        const bool locked = guiLocked;
                        GuiLock(); // lock the checkbox since we changed the value manually
                        GuiCheckBox((Rectangle){propBounds.x+propBounds.width/2, propBounds.y + height/4, height/2, height/2}, props[p].value.vbool ? "Yes" : "No", props[p].value.vbool);
                        if(!locked) GuiUnlock(); // only unlock when needed
                    } break;

                    case GUI_PROP_INT:
                        // draw property name
                        GuiDrawText(props[p].name, (Rectangle){propBounds.x + PROPERTY_PADDING, propBounds.y, propBounds.width/2-PROPERTY_PADDING, GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT)}, GUI_TEXT_ALIGN_LEFT, textColor);
                        // draw property value
                        props[p].value.vint.val = GuiDMSpinner((Rectangle){propBounds.x+propBounds.width/2, propBounds.y + 1, propBounds.width/2, GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT) - 2},
                                    props[p].value.vint.val, props[p].value.vint.min, props[p].value.vint.max, props[p].value.vint.step, 0, (propState == GUI_STATE_FOCUSED) );
                    break;

                    case GUI_PROP_FLOAT:
                        // draw property name
                        GuiDrawText(props[p].name, (Rectangle){propBounds.x + PROPERTY_PADDING, propBounds.y, propBounds.width/2-PROPERTY_PADDING, GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT)}, GUI_TEXT_ALIGN_LEFT, textColor);
                        // draw property value
                        props[p].value.vfloat.val = GuiDMSpinner((Rectangle){propBounds.x+propBounds.width/2, propBounds.y + 1, propBounds.width/2, GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT) - 2},
                                    props[p].value.vfloat.val, props[p].value.vfloat.min, props[p].value.vfloat.max, props[p].value.vfloat.step, props[p].value.vfloat.precision, (propState == GUI_STATE_FOCUSED) );
                    break;

                    case GUI_PROP_TEXT: {
                        Rectangle titleBounds = { propBounds.x, propBounds.y, propBounds.width, GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT) };
                        // Collapse/Expand property on click
                        if((propState == GUI_STATE_PRESSED) && CheckCollisionPointRec(mousePos, titleBounds))
                            PROP_TOGGLE_FLAG(&props[p], GUI_PFLAG_COLLAPSED);

                        // draw property name
                        GuiDrawText(PROP_CHECK_FLAG(&props[p], GUI_PFLAG_COLLAPSED) ? PROPERTY_COLLAPSED_ICON : PROPERTY_EXPANDED_ICON, titleBounds, GUI_TEXT_ALIGN_LEFT, textColor);
                        GuiDrawText(props[p].name, (Rectangle){propBounds.x+PROPERTY_ICON_SIZE+PROPERTY_PADDING, propBounds.y, propBounds.width-PROPERTY_ICON_SIZE-PROPERTY_PADDING, GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT)}, GUI_TEXT_ALIGN_LEFT, textColor);
                        GuiDrawText(TextFormat("%i/%i", strlen(props[p].value.vtext.val), props[p].value.vtext.size), (Rectangle){propBounds.x+propBounds.width/2, propBounds.y + 1, propBounds.width/2, GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT) - 2}, GUI_TEXT_ALIGN_LEFT, textColor);

                        // draw property value
                        if(!PROP_CHECK_FLAG(&props[p], GUI_PFLAG_COLLAPSED))
                            GuiTextBox((Rectangle){propBounds.x, propBounds.y + GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT)+1, propBounds.width, GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT)-2}, props[p].value.vtext.val, props[p].value.vtext.size, (propState == GUI_STATE_FOCUSED));
                    } break;

                    case GUI_PROP_SELECT: {
                        // TODO: Create a custom dropdownbox control instead of using the raygui combobox
                        // draw property name
                        GuiDrawText(props[p].name, (Rectangle){propBounds.x + PROPERTY_PADDING, propBounds.y, propBounds.width/2-PROPERTY_PADDING, GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT)}, GUI_TEXT_ALIGN_LEFT, textColor);
                        // draw property value
                        props[p].value.vselect.active = GuiComboBox((Rectangle){propBounds.x+propBounds.width/2, propBounds.y + 1, propBounds.width/2, GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT) - 2},
                                props[p].value.vselect.val, props[p].value.vselect.active);
                    } break;

                    case GUI_PROP_VECTOR2: case GUI_PROP_VECTOR3: case GUI_PROP_VECTOR4: {
                        Rectangle titleBounds = { propBounds.x, propBounds.y, propBounds.width, GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT) };
                        // Collapse/Expand property on click
                        if((propState == GUI_STATE_PRESSED) && CheckCollisionPointRec(mousePos, titleBounds))
                            PROP_TOGGLE_FLAG(&props[p], GUI_PFLAG_COLLAPSED);

                        const char* fmt = "";
                        if(props[p].type == GUI_PROP_VECTOR2) fmt = TextFormat("[%.0f, %.0f]", props[p].value.v2.x, props[p].value.v2.y);
                        else if(props[p].type == GUI_PROP_VECTOR3) fmt = TextFormat("[%.0f, %.0f, %.0f]", props[p].value.v3.x, props[p].value.v3.y, props[p].value.v3.z);
                        else fmt = TextFormat("[%.0f, %.0f, %.0f, %.0f]", props[p].value.v4.x, props[p].value.v4.y, props[p].value.v4.z, props[p].value.v4.w);

                        // draw property name
                        GuiDrawText(PROP_CHECK_FLAG(&props[p], GUI_PFLAG_COLLAPSED) ? PROPERTY_COLLAPSED_ICON : PROPERTY_EXPANDED_ICON, titleBounds, GUI_TEXT_ALIGN_LEFT, textColor);
                        GuiDrawText(props[p].name, (Rectangle){propBounds.x+PROPERTY_ICON_SIZE+PROPERTY_PADDING, propBounds.y, propBounds.width-PROPERTY_ICON_SIZE-PROPERTY_PADDING, GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT)}, GUI_TEXT_ALIGN_LEFT, textColor);
                        GuiDrawText(fmt, (Rectangle){propBounds.x+propBounds.width/2, propBounds.y + 1, propBounds.width/2, GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT) - 2}, GUI_TEXT_ALIGN_LEFT, textColor);

                        // draw X, Y, Z, W values (only when expanded)
                        if(!PROP_CHECK_FLAG(&props[p], GUI_PFLAG_COLLAPSED)) {
                            Rectangle slotBounds = { propBounds.x, propBounds.y+GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT)+1, propBounds.width, GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT)-2};
                            Rectangle lblBounds = { propBounds.x+PROPERTY_PADDING, slotBounds.y, GetTextWidth("A"), slotBounds.height};
                            Rectangle valBounds = { lblBounds.x+lblBounds.width+PROPERTY_PADDING, slotBounds.y, propBounds.width-lblBounds.width-2*PROPERTY_PADDING, slotBounds.height};
                            GuiDrawText("X", lblBounds, GUI_TEXT_ALIGN_LEFT, textColor);
                            props[p].value.v2.x = GuiDMSpinner(valBounds, props[p].value.v2.x, 0.0, 0.0, 1.0, PROPERTY_DECIMAL_DIGITS, (propState == GUI_STATE_FOCUSED) && CheckCollisionPointRec(mousePos, slotBounds) );
                            slotBounds.y += GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT);
                            lblBounds.y = valBounds.y = slotBounds.y;
                            GuiDrawText("Y", lblBounds, GUI_TEXT_ALIGN_LEFT, textColor);
                            props[p].value.v2.y = GuiDMSpinner(valBounds, props[p].value.v2.y, 0.0, 0.0, 1.0, PROPERTY_DECIMAL_DIGITS, (propState == GUI_STATE_FOCUSED) && CheckCollisionPointRec(mousePos, slotBounds) );
                            slotBounds.y += GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT);
                            lblBounds.y = valBounds.y = slotBounds.y;
                            if(props[p].type >= GUI_PROP_VECTOR3) {
                                GuiDrawText("Z", lblBounds, GUI_TEXT_ALIGN_LEFT, textColor);
                                props[p].value.v3.z = GuiDMSpinner(valBounds, props[p].value.v3.z, 0.0, 0.0, 1.0, PROPERTY_DECIMAL_DIGITS, (propState == GUI_STATE_FOCUSED) && CheckCollisionPointRec(mousePos, slotBounds) );
                                slotBounds.y += GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT);
                                lblBounds.y = valBounds.y = slotBounds.y;
                            }

                            if(props[p].type >= GUI_PROP_VECTOR4) {
                                GuiDrawText("W", lblBounds, GUI_TEXT_ALIGN_LEFT, textColor);
                                props[p].value.v4.w = GuiDMSpinner(valBounds, props[p].value.v4.w, 0.0, 0.0, 1.0, PROPERTY_DECIMAL_DIGITS, (propState == GUI_STATE_FOCUSED) && CheckCollisionPointRec(mousePos, slotBounds) );
                            }
                        }
                    } break;

                    case GUI_PROP_RECT:{
                        Rectangle titleBounds = { propBounds.x, propBounds.y, propBounds.width, GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT) };
                        // Collapse/Expand property on click
                        if((propState == GUI_STATE_PRESSED) && CheckCollisionPointRec(mousePos, titleBounds))
                            PROP_TOGGLE_FLAG(&props[p], GUI_PFLAG_COLLAPSED);

                        // draw property name
                        GuiDrawText(PROP_CHECK_FLAG(&props[p], GUI_PFLAG_COLLAPSED) ? PROPERTY_COLLAPSED_ICON : PROPERTY_EXPANDED_ICON, titleBounds, GUI_TEXT_ALIGN_LEFT, textColor);
                        GuiDrawText(props[p].name, (Rectangle){propBounds.x+PROPERTY_ICON_SIZE+PROPERTY_PADDING, propBounds.y, propBounds.width-PROPERTY_ICON_SIZE-PROPERTY_PADDING, GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT)}, GUI_TEXT_ALIGN_LEFT, textColor);
                        GuiDrawText(TextFormat("[%.0f, %.0f, %.0f, %.0f]", props[p].value.vrect.x, props[p].value.vrect.y, props[p].value.vrect.width, props[p].value.vrect.height),
                                (Rectangle){propBounds.x+propBounds.width/2, propBounds.y + 1, propBounds.width/2, GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT) - 2}, GUI_TEXT_ALIGN_LEFT, textColor);

                        // draw X, Y, Width, Height values (only when expanded)
                        if(!PROP_CHECK_FLAG(&props[p], GUI_PFLAG_COLLAPSED)) {
                            Rectangle slotBounds = { propBounds.x, propBounds.y+GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT)+1, propBounds.width, GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT)-2};
                            Rectangle lblBounds = { propBounds.x+PROPERTY_PADDING, slotBounds.y, GetTextWidth("Height"), slotBounds.height};
                            Rectangle valBounds = { lblBounds.x+lblBounds.width+PROPERTY_PADDING, slotBounds.y, propBounds.width-lblBounds.width-2*PROPERTY_PADDING, slotBounds.height};
                            GuiDrawText("X", lblBounds, GUI_TEXT_ALIGN_LEFT, textColor);
                            props[p].value.vrect.x = GuiDMSpinner(valBounds, props[p].value.vrect.x, 0.0, 0.0, 1.0, 0, (propState == GUI_STATE_FOCUSED) && CheckCollisionPointRec(mousePos, slotBounds) );
                            slotBounds.y += GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT);
                            lblBounds.y = valBounds.y = slotBounds.y;
                            GuiDrawText("Y", lblBounds, GUI_TEXT_ALIGN_LEFT, textColor);
                            props[p].value.vrect.y = GuiDMSpinner(valBounds, props[p].value.vrect.y, 0.0, 0.0, 1.0, 0, (propState == GUI_STATE_FOCUSED) && CheckCollisionPointRec(mousePos, slotBounds) );
                            slotBounds.y += GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT);
                            lblBounds.y = valBounds.y = slotBounds.y;
                            GuiDrawText("Width", lblBounds, GUI_TEXT_ALIGN_LEFT, textColor);
                            props[p].value.vrect.width = GuiDMSpinner(valBounds, props[p].value.vrect.width, 0.0, 0.0, 1.0, 0, (propState == GUI_STATE_FOCUSED) && CheckCollisionPointRec(mousePos, slotBounds) );
                            slotBounds.y += GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT);
                            lblBounds.y = valBounds.y = slotBounds.y;
                            GuiDrawText("Height", lblBounds, GUI_TEXT_ALIGN_LEFT, textColor);
                            props[p].value.vrect.height = GuiDMSpinner(valBounds, props[p].value.vrect.height, 0.0, 0.0, 1.0, 0, (propState == GUI_STATE_FOCUSED) && CheckCollisionPointRec(mousePos, slotBounds) );
                        }
                    } break;


                    case GUI_PROP_COLOR: {
                        Rectangle titleBounds = { propBounds.x, propBounds.y, propBounds.width, GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT) };
                        // Collapse/Expand property on click
                        if((propState == GUI_STATE_PRESSED) && CheckCollisionPointRec(mousePos, titleBounds))
                            PROP_TOGGLE_FLAG(&props[p], GUI_PFLAG_COLLAPSED);

                        // draw property name
                        GuiDrawText(PROP_CHECK_FLAG(&props[p], GUI_PFLAG_COLLAPSED) ? PROPERTY_COLLAPSED_ICON : PROPERTY_EXPANDED_ICON, titleBounds, GUI_TEXT_ALIGN_LEFT, textColor);
                        GuiDrawText(props[p].name, (Rectangle){propBounds.x+PROPERTY_ICON_SIZE+PROPERTY_PADDING, propBounds.y+1, propBounds.width-PROPERTY_ICON_SIZE-PROPERTY_PADDING, GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT)-2}, GUI_TEXT_ALIGN_LEFT, textColor);
                        DrawLineEx( (Vector2){propBounds.x+propBounds.width/2, propBounds.y + GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT) - 5}, (Vector2){propBounds.x+propBounds.width, propBounds.y + GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT) - 5}, 6.0f, props[p].value.vcolor);
                        const char* fmt = TextFormat("#%02X%02X%02X%02X", props[p].value.vcolor.r, props[p].value.vcolor.g, props[p].value.vcolor.b, props[p].value.vcolor.a);
                        char clip[10] = "\0";
                        memcpy(clip, fmt, 10*sizeof(char)); // copy to temporary buffer since we can't be sure when TextFormat() will be called again and our text will be overwritten
                        GuiDrawText(fmt, (Rectangle){propBounds.x+propBounds.width/2, propBounds.y + 1, propBounds.width/2, GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT) - 2}, GUI_TEXT_ALIGN_LEFT, textColor);

                        // draw R, G, B, A values (only when expanded)
                        if(!PROP_CHECK_FLAG(&props[p], GUI_PFLAG_COLLAPSED)) {
                            Rectangle slotBounds = { propBounds.x, propBounds.y+GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT)+1, propBounds.width, GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT)-2};
                            Rectangle lblBounds = { propBounds.x+PROPERTY_PADDING, slotBounds.y, GetTextWidth("A"), slotBounds.height};
                            Rectangle valBounds = { lblBounds.x+lblBounds.width+PROPERTY_PADDING, slotBounds.y, GetTextWidth("000000"), slotBounds.height};
                            Rectangle sbarBounds = { valBounds.x + valBounds.width + PROPERTY_PADDING, slotBounds.y, slotBounds.width - 3*PROPERTY_PADDING - lblBounds.width - valBounds.width, slotBounds.height };

                            if(sbarBounds.width <= GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT)-2) valBounds.width = propBounds.width-lblBounds.width-2*PROPERTY_PADDING; // hide slider when no space
                            // save current scrollbar style
                            int tmpSliderPadding = GuiGetStyle(SCROLLBAR, SCROLL_SLIDER_PADDING);
                            int tmpPadding = GuiGetStyle(SCROLLBAR, SCROLL_PADDING);
                            int tmpBorder = GuiGetStyle(SCROLLBAR, BORDER_WIDTH);
                            int tmpSliderSize = GuiGetStyle(SCROLLBAR, SCROLL_SLIDER_SIZE);
                            int tmpArrows =  GuiGetStyle(SCROLLBAR, ARROWS_VISIBLE);
                            Color tmpBG1 = GetColor(GuiGetStyle(DEFAULT, BORDER_COLOR_DISABLED));
                            // set a custom scrollbar style
                            GuiSetStyle(SCROLLBAR, SCROLL_SLIDER_PADDING, 3);
                            GuiSetStyle(SCROLLBAR, SCROLL_PADDING, 10);
                            GuiSetStyle(SCROLLBAR, BORDER_WIDTH, 0);
                            GuiSetStyle(SCROLLBAR, SCROLL_SLIDER_SIZE, 6);
                            GuiSetStyle(SCROLLBAR, ARROWS_VISIBLE, 0);
                            GuiSetStyle(DEFAULT, BORDER_COLOR_DISABLED, GuiGetStyle(DEFAULT, BACKGROUND_COLOR)); // disable scrollbar background

                            GuiDrawText("R", lblBounds, GUI_TEXT_ALIGN_LEFT, textColor);
                            props[p].value.vcolor.r = GuiDMValueBox(valBounds, props[p].value.vcolor.r, 0.0, 255.0, 0, (propState == GUI_STATE_FOCUSED) && CheckCollisionPointRec(mousePos, slotBounds) );
                            if(sbarBounds.width > GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT)-2)
                                props[p].value.vcolor.r = GuiScrollBar(sbarBounds, props[p].value.vcolor.r, 0, 255);
                            slotBounds.y += GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT);
                            lblBounds.y = valBounds.y = sbarBounds.y = slotBounds.y;

                            GuiDrawText("G", lblBounds, GUI_TEXT_ALIGN_LEFT, textColor);
                            props[p].value.vcolor.g = GuiDMValueBox(valBounds, props[p].value.vcolor.g, 0.0, 255.0, 0, (propState == GUI_STATE_FOCUSED) && CheckCollisionPointRec(mousePos, slotBounds) );
                            if(sbarBounds.width > GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT)-2)
                                props[p].value.vcolor.g = GuiScrollBar(sbarBounds, props[p].value.vcolor.g, 0, 255);
                            slotBounds.y += GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT);
                            lblBounds.y = valBounds.y = sbarBounds.y = slotBounds.y;

                            GuiDrawText("B", lblBounds, GUI_TEXT_ALIGN_LEFT, textColor);
                            props[p].value.vcolor.b = GuiDMValueBox(valBounds, props[p].value.vcolor.b, 0.0, 255.0, 0, (propState == GUI_STATE_FOCUSED) && CheckCollisionPointRec(mousePos, slotBounds) );
                            if(sbarBounds.width > GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT)-2)
                                props[p].value.vcolor.b = GuiScrollBar(sbarBounds, props[p].value.vcolor.b, 0, 255);
                            slotBounds.y += GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT);
                            lblBounds.y = valBounds.y = sbarBounds.y = slotBounds.y;

                            GuiDrawText("A", lblBounds, GUI_TEXT_ALIGN_LEFT, textColor);
                            props[p].value.vcolor.a = GuiDMValueBox(valBounds, props[p].value.vcolor.a, 0.0, 255.0, 0, (propState == GUI_STATE_FOCUSED) && CheckCollisionPointRec(mousePos, slotBounds) );
                            if(sbarBounds.width > GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT)-2)
                                props[p].value.vcolor.a = GuiScrollBar(sbarBounds, props[p].value.vcolor.a, 0, 255);

                            // load saved scrollbar style
                            GuiSetStyle(SCROLLBAR, SCROLL_SLIDER_PADDING, tmpSliderPadding);
                            GuiSetStyle(SCROLLBAR, SCROLL_PADDING, tmpPadding);
                            GuiSetStyle(SCROLLBAR, BORDER_WIDTH, tmpBorder);
                            GuiSetStyle(SCROLLBAR, SCROLL_SLIDER_SIZE, tmpSliderSize);
                            GuiSetStyle(SCROLLBAR, ARROWS_VISIBLE, tmpArrows);
                            GuiSetStyle(DEFAULT, BORDER_COLOR_DISABLED, ColorToInt(tmpBG1));
                        }

                        // support COPY/PASTE (need to do this here since GuiDMValueBox() also has COPY/PASTE so we need to overwrite it)
                        if((propState == GUI_STATE_FOCUSED)) {
                            if(IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_C))
                                SetClipboardText(clip);
                            else if(IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_V)){
                                unsigned int a = props[p].value.vcolor.a, r = props[p].value.vcolor.r, g=props[p].value.vcolor.g, b=props[p].value.vcolor.b;
                                sscanf(GetClipboardText(), "#%02X%02X%02X%02X", &r, &g, &b, &a);
                                props[p].value.vcolor.r=r; props[p].value.vcolor.g=g; props[p].value.vcolor.b=b; props[p].value.vcolor.a=a;
                            }
                        }
                    } break;

                    case GUI_PROP_SECTION: {
                        Rectangle titleBounds = { propBounds.x, propBounds.y, propBounds.width, GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT) };
                        // Collapse/Expand section on click
                        if( (propState == GUI_STATE_PRESSED) && CheckCollisionPointRec(mousePos, titleBounds) )
                            PROP_TOGGLE_FLAG(&props[p], GUI_PFLAG_COLLAPSED);

                        if(!PROP_CHECK_FLAG(&props[p], GUI_PFLAG_COLLAPSED)) {
                            GuiDrawText(PROPERTY_EXPANDED_ICON, titleBounds, GUI_TEXT_ALIGN_LEFT, textColor);
                            GuiDrawText(props[p].name, (Rectangle){propBounds.x+PROPERTY_ICON_SIZE+PROPERTY_PADDING, propBounds.y, propBounds.width-PROPERTY_ICON_SIZE-PROPERTY_PADDING, GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT)}, GUI_TEXT_ALIGN_CENTER, textColor);
                        } else {
                            GuiDrawText(PROPERTY_COLLAPSED_ICON, titleBounds, GUI_TEXT_ALIGN_LEFT, textColor);
                            GuiDrawText(TextFormat("%s [%i]", props[p].name, props[p].value.vsection), (Rectangle){propBounds.x+PROPERTY_ICON_SIZE+PROPERTY_PADDING, propBounds.y, propBounds.width-PROPERTY_ICON_SIZE-PROPERTY_PADDING, GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT)}, GUI_TEXT_ALIGN_CENTER, textColor);
                        }
                    } break;


                    // NOTE: Add your custom property here !!
                    default: {
                        // draw property name
                        GuiDrawText(props[p].name, (Rectangle){propBounds.x + PROPERTY_PADDING, propBounds.y, propBounds.width/2-PROPERTY_PADDING, GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT)}, GUI_TEXT_ALIGN_LEFT, textColor);
                        // draw property type
                        GuiDrawText(TextFormat("TYPE %i", props[p].type), (Rectangle){propBounds.x+propBounds.width/2, propBounds.y + 1, propBounds.width/2, GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT) - 2}, GUI_TEXT_ALIGN_LEFT, textColor);
                    } break;

                } // end of switch()

                 GuiSetState(state);
            }

            currentHeight += height + 1;

            // Skip collapsed section. Don't put this code inside the switch !!
            if(props[p].type == GUI_PROP_SECTION && (PROP_CHECK_FLAG(&props[p], GUI_PFLAG_COLLAPSED))) p += props[p].value.vsection;
        } // end for
    EndScissorMode();

    if(useScrollBar) {
        scroll = -GuiScrollBar(scrollBarBounds, -scroll, 0, maxScroll);
        *scrollIndex = scroll;
    }
    //--------------------------------------------------------------------

    if(focus != NULL) *focus = propFocused;
}

bool GuiDMSaveProperties(const char* file, GuiDMProperty* props, int count) {
    if(file == NULL || props == NULL) return false;
    if(count == 0) return true;

    FILE* f = fopen(file, "w");
    if(f == NULL) return false;

    // write header
    fprintf(f, "#\n# Property types:\n"
            "#   b  <name> <flags> <value>                                    // Bool\n"
            "#   i  <name> <flags> <value> <min> <max> <step>                 // Int\n"
            "#   f  <name> <flags> <value> <min> <max> <step> <precision>     // Float\n"
            "#   t  <name> <flags> <value> <edit_length>                      // Text\n"
            "#   l  <name> <flags> <value> <active>                           // Select\n"
            "#   g  <name> <flags> <value>                                    // Section (Group)\n"
            "#   v2 <name> <flags> <x> <y>                                    // Vector 2D\n"
            "#   v3 <name> <flags> <x> <y> <z>                                // Vector 3D\n"
            "#   v4 <name> <flags> <x> <y> <z> <w>                            // Vector 4D\n"
            "#   r  <name> <flags> <x> <y> <width> <height>                   // Rectangle\n"
            "#   c  <name> <flags> <r> <g> <b> <a>                            // Color\n"
            "#\n\n");
    for(int p=0; p<count; ++p)
    {
        switch(props[p].type) {
            case GUI_PROP_BOOL: fprintf(f, "b  %s %i %i\n", props[p].name, (int)props[p].flags, (int)props[p].value.vbool);
            break;

            case GUI_PROP_INT: fprintf(f, "i  %s %i %i %i %i %i\n", props[p].name, (int)props[p].flags, props[p].value.vint.val, props[p].value.vint.min,
                        props[p].value.vint.max, props[p].value.vint.step);
            break;

            case GUI_PROP_FLOAT: fprintf(f, "f  %s %i %f %f %f %f %i\n", props[p].name, (int)props[p].flags, props[p].value.vfloat.val, props[p].value.vfloat.min,
                        props[p].value.vfloat.max, props[p].value.vfloat.step, props[p].value.vfloat.precision);
            break;

            case GUI_PROP_TEXT: fprintf(f, "t  %s %i %s %i\n", props[p].name, (int)props[p].flags, props[p].value.vtext.val, props[p].value.vtext.size);
            break;

            case GUI_PROP_SELECT: fprintf(f, "l  %s %i %s %i\n", props[p].name, (int)props[p].flags, props[p].value.vselect.val, props[p].value.vselect.active);
            break;

            case GUI_PROP_SECTION: fprintf(f, "g  %s %i %i\n", props[p].name, (int)props[p].flags, props[p].value.vsection);
            break;

            case GUI_PROP_VECTOR2: fprintf(f, "v2 %s %i %f %f\n", props[p].name, (int)props[p].flags, props[p].value.v2.x, props[p].value.v2.y);
            break;

            case GUI_PROP_VECTOR3: fprintf(f, "v3 %s %i %f %f %f\n", props[p].name, (int)props[p].flags, props[p].value.v3.x, props[p].value.v3.y, props[p].value.v3.z);
            break;

            case GUI_PROP_VECTOR4: fprintf(f, "v4 %s %i %f %f %f %f\n", props[p].name, (int)props[p].flags, props[p].value.v4.x, props[p].value.v4.y,
                        props[p].value.v4.z, props[p].value.v4.w);
            break;

            case GUI_PROP_RECT: fprintf(f, "r  %s %i %i %i %i %i\n", props[p].name, (int)props[p].flags, (int)props[p].value.vrect.x, (int)props[p].value.vrect.y,
                        (int)props[p].value.vrect.width, (int)props[p].value.vrect.height);
            break;

            case GUI_PROP_COLOR: fprintf(f, "c  %s %i %i %i %i %i\n", props[p].name, (int)props[p].flags, props[p].value.vcolor.r, props[p].value.vcolor.g,
                        props[p].value.vcolor.b, props[p].value.vcolor.a);
            break;
        }
    }

    fclose(f);
    return true;
}

