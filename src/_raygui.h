/*******************************************************************************************
*
*   raygui v3.0 - A simple and easy-to-use immediate-mode gui library
*
*   DESCRIPTION:
*
*   raygui is a tools-dev-focused immediate-mode-gui library based on raylib but also
*   available as a standalone library, as long as input and drawing functions are provided.
*
*   Controls provided:
*
*   # Container/separators Controls
*       - WindowBox
*       - GroupBox
*       - Line
*       - Panel
*
*   # Basic Controls
*       - Label
*       - Button
*       - LabelButton   --> Label
*       - Toggle
*       - ToggleGroup   --> Toggle
*       - CheckBox
*       - ComboBox
*       - DropdownBox
*       - TextBox
*       - TextBoxMulti
*       - ValueBox      --> TextBox
*       - Spinner       --> Button, ValueBox
*       - Slider
*       - SliderBar     --> Slider
*       - ProgressBar
*       - StatusBar
*       - ScrollBar
*       - ScrollPanel
*       - DummyRec
*       - Grid
*
*   # Advance Controls
*       - ListView
*       - ColorPicker   --> ColorPanel, ColorBarHue
*       - MessageBox    --> Window, Label, Button
*       - TextInputBox  --> Window, Label, TextBox, Button
*
*   It also provides a set of functions for styling the controls based on its properties (size, color).
*
*
*   GUI STYLE (guiStyle):
*
*   raygui uses a global data array for all gui style properties (allocated on data segment by default),
*   when a new style is loaded, it is loaded over the global style... but a default gui style could always be
*   recovered with GuiLoadStyleDefault() function, that overwrites the current style to the default one
*
*   The global style array size is fixed and depends on the number of controls and properties:
*
*       static unsigned int guiStyle[RAYGUI_MAX_CONTROLS*(RAYGUI_MAX_PROPS_BASE + RAYGUI_MAX_PROPS_EXTENDED)];
*
*   guiStyle size is by default: 16*(16 + 8) = 384*4 = 1536 bytes = 1.5 KB
*
*   Note that the first set of BASE properties (by default guiStyle[0..15]) belong to the generic style
*   used for all controls, when any of those base values is set, it is automatically populated to all
*   controls, so, specific control values overwriting generic style should be set after base values.
*
*   After the first BASE set we have the EXTENDED properties (by default guiStyle[16..23]), those
*   properties are actually common to all controls and can not be overwritten individually (like BASE ones)
*   Some of those properties are: TEXT_SIZE, TEXT_SPACING, LINE_COLOR, BACKGROUND_COLOR
*
*   Custom control properties can be defined using the EXTENDED properties for each independent control.
*
*   TOOL: rGuiStyler is a visual tool to customize raygui style.
*
*
*   GUI ICONS (guiIcons):
*
*   raygui could use a global array containing icons data (allocated on data segment by default),
*   a custom icons set could be loaded over this array using GuiLoadIcons(), but loaded icons set
*   must be same RICON_SIZE and no more than RICON_MAX_ICONS will be loaded
*
*   Every icon is codified in binary form, using 1 bit per pixel, so, every 16x16 icon
*   requires 8 integers (16*16/32) to be stored in memory.
*
*   When the icon is draw, actually one quad per pixel is drawn if the bit for that pixel is set.
*
*   The global icons array size is fixed and depends on the number of icons and size:
*
*       static unsigned int guiIcons[RICON_MAX_ICONS*RICON_DATA_ELEMENTS];
*
*   guiIcons size is by default: 256*(16*16/32) = 2048*4 = 8192 bytes = 8 KB
*
*   TOOL: rGuiIcons is a visual tool to customize raygui icons.
*
*
*   CONFIGURATION:
*
*   #define RAYGUI_IMPLEMENTATION
*       Generates the implementation of the library into the included file.
*       If not defined, the library is in header only mode and can be included in other headers
*       or source files without problems. But only ONE file should hold the implementation.
*
*   #define RAYGUI_STANDALONE
*       Avoid raylib.h header inclusion in this file. Data types defined on raylib are defined
*       internally in the library and input management and drawing functions must be provided by
*       the user (check library implementation for further details).
*
*   #define RAYGUI_NO_RICONS
*       Avoid including embedded ricons data (256 icons, 16x16 pixels, 1-bit per pixel, 2KB)
*
*   #define RAYGUI_CUSTOM_RICONS
*       Includes custom ricons.h header defining a set of custom icons,
*       this file can be generated using rGuiIcons tool
*
*
*   VERSIONS HISTORY:
*
*       3.0 (xx-Sep-2021) Integrated ricons data to avoid external file
*                         REDESIGNED: GuiTextBoxMulti()
*                         REMOVED: GuiImageButton*()
*                         Multiple minor tweaks and bugs corrected
*       2.9 (17-Mar-2021) REMOVED: Tooltip API
*       2.8 (03-May-2020) Centralized rectangles drawing to GuiDrawRectangle()
*       2.7 (20-Feb-2020) ADDED: Possible tooltips API
*       2.6 (09-Sep-2019) ADDED: GuiTextInputBox()
*                         REDESIGNED: GuiListView*(), GuiDropdownBox(), GuiSlider*(), GuiProgressBar(), GuiMessageBox()
*                         REVIEWED: GuiTextBox(), GuiSpinner(), GuiValueBox(), GuiLoadStyle()
*                         Replaced property INNER_PADDING by TEXT_PADDING, renamed some properties
*                         ADDED: 8 new custom styles ready to use
*                         Multiple minor tweaks and bugs corrected
*       2.5 (28-May-2019) Implemented extended GuiTextBox(), GuiValueBox(), GuiSpinner()
*       2.3 (29-Apr-2019) ADDED: rIcons auxiliar library and support for it, multiple controls reviewed
*                         Refactor all controls drawing mechanism to use control state
*       2.2 (05-Feb-2019) ADDED: GuiScrollBar(), GuiScrollPanel(), reviewed GuiListView(), removed Gui*Ex() controls
*       2.1 (26-Dec-2018) REDESIGNED: GuiCheckBox(), GuiComboBox(), GuiDropdownBox(), GuiToggleGroup() > Use combined text string
*                         REDESIGNED: Style system (breaking change)
*       2.0 (08-Nov-2018) ADDED: Support controls guiLock and custom fonts
*                         REVIEWED: GuiComboBox(), GuiListView()...
*       1.9 (09-Oct-2018) REVIEWED: GuiGrid(), GuiTextBox(), GuiTextBoxMulti(), GuiValueBox()...
*       1.8 (01-May-2018) Lot of rework and redesign to align with rGuiStyler and rGuiLayout
*       1.5 (21-Jun-2017) Working in an improved styles system
*       1.4 (15-Jun-2017) Rewritten all GUI functions (removed useless ones)
*       1.3 (12-Jun-2017) Complete redesign of style system
*       1.1 (01-Jun-2017) Complete review of the library
*       1.0 (07-Jun-2016) Converted to header-only by Ramon Santamaria.
*       0.9 (07-Mar-2016) Reviewed and tested by Albert Martos, Ian Eito, Sergio Martinez and Ramon Santamaria.
*       0.8 (27-Aug-2015) Initial release. Implemented by Kevin Gato, Daniel Nicolás and Ramon Santamaria.
*
*
*   CONTRIBUTORS:
*
*       Ramon Santamaria:   Supervision, review, redesign, update and maintenance
*       Vlad Adrian:        Complete rewrite of GuiTextBox() to support extended features (2019)
*       Sergio Martinez:    Review, testing (2015) and redesign of multiple controls (2018)
*       Adria Arranz:       Testing and Implementation of additional controls (2018)
*       Jordi Jorba:        Testing and Implementation of additional controls (2018)
*       Albert Martos:      Review and testing of the library (2015)
*       Ian Eito:           Review and testing of the library (2015)
*       Kevin Gato:         Initial implementation of basic components (2014)
*       Daniel Nicolas:     Initial implementation of basic components (2014)
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2014-2021 Ramon Santamaria (@raysan5)
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

#ifndef RAYGUI_H
#define RAYGUI_H

#define RAYGUI_VERSION  "3.0"




    #define RAYGUIAPI       // Functions defined as 'extern' by default (implicit specifiers)

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------

// Allow custom memory allocators
    #define RAYGUI_MALLOC(sz)       malloc(sz)
    #define RAYGUI_CALLOC(n,sz)     calloc(n,sz)
    #define RAYGUI_FREE(p)          free(p)

    #ifndef __cplusplus
    // Boolean type
        #ifndef true
            typedef enum { false, true } bool;
        #endif
    #endif


// Style property
typedef struct GuiStyleProp {
    unsigned short controlId;
    unsigned short propertyId;
    int propertyValue;
} GuiStyleProp;

// Gui control state
typedef enum {
    GUI_STATE_NORMAL = 0,
    GUI_STATE_FOCUSED,
    GUI_STATE_PRESSED,
    GUI_STATE_DISABLED,
} GuiControlState;

// Gui control text alignment
typedef enum {
    GUI_TEXT_ALIGN_LEFT = 0,
    GUI_TEXT_ALIGN_CENTER,
    GUI_TEXT_ALIGN_RIGHT,
} GuiTextAlignment;

// Gui controls
typedef enum {
    DEFAULT = 0,    // Generic control -> populates to all controls when set
    LABEL,          // Used also for: LABELBUTTON
    BUTTON,
    TOGGLE,         // Used also for: TOGGLEGROUP
    SLIDER,         // Used also for: SLIDERBAR
    PROGRESSBAR,
    CHECKBOX,
    COMBOBOX,
    DROPDOWNBOX,
    TEXTBOX,        // Used also for: TEXTBOXMULTI
    VALUEBOX,
    SPINNER,
    LISTVIEW,
    COLORPICKER,
    SCROLLBAR,
    STATUSBAR
} GuiControl;

// Gui base properties for every control
// NOTE: RAYGUI_MAX_PROPS_BASE properties (by default 16 properties)
typedef enum {
    BORDER_COLOR_NORMAL = 0,
    BASE_COLOR_NORMAL,
    TEXT_COLOR_NORMAL,
    BORDER_COLOR_FOCUSED,
    BASE_COLOR_FOCUSED,
    TEXT_COLOR_FOCUSED,
    BORDER_COLOR_PRESSED,
    BASE_COLOR_PRESSED,
    TEXT_COLOR_PRESSED,
    BORDER_COLOR_DISABLED,
    BASE_COLOR_DISABLED,
    TEXT_COLOR_DISABLED,
    BORDER_WIDTH,
    TEXT_PADDING,
    TEXT_ALIGNMENT,
    RESERVED
} GuiControlProperty;

// Gui extended properties depend on control
// NOTE: RAYGUI_MAX_PROPS_EXTENDED properties (by default 8 properties)

// DEFAULT extended properties
// NOTE: Those properties are actually common to all controls
typedef enum {
    TEXT_SIZE = 16,
    TEXT_SPACING,
    LINE_COLOR,
    BACKGROUND_COLOR,
} GuiDefaultProperty;

// Label
//typedef enum { } GuiLabelProperty;

// Button
//typedef enum { } GuiButtonProperty;

// Toggle/ToggleGroup
typedef enum {
    GROUP_PADDING = 16,
} GuiToggleProperty;

// Slider/SliderBar
typedef enum {
    SLIDER_WIDTH = 16,
    SLIDER_PADDING
} GuiSliderProperty;

// ProgressBar
typedef enum {
    PROGRESS_PADDING = 16,
} GuiProgressBarProperty;

// CheckBox
typedef enum {
    CHECK_PADDING = 16
} GuiCheckBoxProperty;

// ComboBox
typedef enum {
    COMBO_BUTTON_WIDTH = 16,
    COMBO_BUTTON_PADDING
} GuiComboBoxProperty;

// DropdownBox
typedef enum {
    ARROW_PADDING = 16,
    DROPDOWN_ITEMS_PADDING
} GuiDropdownBoxProperty;

// TextBox/TextBoxMulti/ValueBox/Spinner
typedef enum {
    TEXT_INNER_PADDING = 16,
    TEXT_LINES_PADDING,
    COLOR_SELECTED_FG,
    COLOR_SELECTED_BG
} GuiTextBoxProperty;

// Spinner
typedef enum {
    SPIN_BUTTON_WIDTH = 16,
    SPIN_BUTTON_PADDING,
} GuiSpinnerProperty;

// ScrollBar
typedef enum {
    ARROWS_SIZE = 16,
    ARROWS_VISIBLE,
    SCROLL_SLIDER_PADDING,
    SCROLL_SLIDER_SIZE,
    SCROLL_PADDING,
    SCROLL_SPEED,
} GuiScrollBarProperty;

// ScrollBar side
typedef enum {
    SCROLLBAR_LEFT_SIDE = 0,
    SCROLLBAR_RIGHT_SIDE
} GuiScrollBarSide;

// ListView
typedef enum {
    LIST_ITEMS_HEIGHT = 16,
    LIST_ITEMS_PADDING,
    SCROLLBAR_WIDTH,
    SCROLLBAR_SIDE,
} GuiListViewProperty;

// ColorPicker
typedef enum {
    COLOR_SELECTOR_SIZE = 16,
    HUEBAR_WIDTH,                  // Right hue bar width
    HUEBAR_PADDING,                // Right hue bar separation from panel
    HUEBAR_SELECTOR_HEIGHT,        // Right hue bar selector height
    HUEBAR_SELECTOR_OVERFLOW       // Right hue bar selector overflow
} GuiColorPickerProperty;


// A bunch of usefull macros for modifying the flags of each property

// Set flag `F` of property `P`. `P` must be a pointer!
#define PROP_SET_FLAG(P, F) ((P)->flags |= (F))
// Clear flag `F` of property `P`. `P` must be a pointer!
#define PROP_CLEAR_FLAG(P, F) ((P)->flags &= ~(F))
// Toggles flag `F` of property `P`. `P` must be a pointer!
#define PROP_TOGGLE_FLAG(P, F) ((P)->flags ^= (F))
// Checks if flag `F` of property `P` is set . `P` must be a pointer!
#define PROP_CHECK_FLAG(P, F) ((P)->flags & (F))

// Some usefull macros for creating properties

// Create a bool property with name `N`, flags `F` and value `V`
#define PBOOL(N, F, V) RAYGUI_CLITERAL(GuiDMProperty){N, GUI_PROP_BOOL, F, .value.vbool = V}
// Create a int property with name `N`, flags `F` and value `V`
#define PINT(N, F, V) RAYGUI_CLITERAL(GuiDMProperty){N, GUI_PROP_INT, F, .value.vint = {V,0,0,1}}
// Create a ranged int property within `MIN` and `MAX` with name `N`, flags `F` value `V`.
// Pressing the spinner buttons will increase/decrease the value by `S`.
#define PINT_RANGE(N, F, V, S, MIN, MAX) RAYGUI_CLITERAL(GuiDMProperty){N, GUI_PROP_INT, F, .value.vint = {V,MIN,MAX,S}}
// Create a float property with name `N`, flags `F` and value `V`
#define PFLOAT(N, F, V) RAYGUI_CLITERAL(GuiDMProperty){N, GUI_PROP_FLOAT, F, .value.vfloat = {V,0.f,0.f,1.0f,3}}
// Create a ranged float property within `MIN` and `MAX` with name `N`, flags `F` value `V` with `P` decimal digits to show.
// Pressing the spinner buttons will increase/decrease the value by `S`.
#define PFLOAT_RANGE(N, F, V, S, P, MIN, MAX) RAYGUI_CLITERAL(GuiDMProperty){N, GUI_PROP_FLOAT, F, .value.vfloat = {V,MIN,MAX,S,P}}
// Create a text property with name `N`, flags `F` value `V` and max text size `S`
#define PTEXT(N, F, V, S) RAYGUI_CLITERAL(GuiDMProperty){N, GUI_PROP_TEXT, F, .value.vtext = {V, S} }
// Create a text property with name `N`, flags `F` value `V` and max text size `S`
#define PSELECT(N, F, V, A) RAYGUI_CLITERAL(GuiDMProperty){N, GUI_PROP_SELECT, F, .value.vselect = {V, A} }
// Create a 2D vector property with name `N`, flags `F` and the `X`, `Y` coordinates
#define PVEC2(N, F, X, Y) RAYGUI_CLITERAL(GuiDMProperty){N, GUI_PROP_VECTOR2, F, .value.v2 = {X, Y} }
// Create a 3D vector property with name `N`, flags `F` and the `X`, `Y`, `Z` coordinates
#define PVEC3(N, F, X, Y, Z) RAYGUI_CLITERAL(GuiDMProperty){N, GUI_PROP_VECTOR3, F, .value.v3 = {X, Y, Z} }
// Create a 4D vector property with name `N`, flags `F` and the `X`, `Y`, `Z`, `W` coordinates
#define PVEC4(N, F, X, Y, Z, W) RAYGUI_CLITERAL(GuiDMProperty){N, GUI_PROP_VECTOR4, F, .value.v4 = {X, Y, Z, W} }
// Create a rectangle property with name `N`, flags `F`, `X`, `Y` coordinates and `W` and `H` size
#define PRECT(N, F, X, Y, W, H) RAYGUI_CLITERAL(GuiDMProperty){N, GUI_PROP_RECT, F, .value.vrect = {X, Y, W, H} }
// Create a 3D vector property with name `N`, flags `F` and the `R`, `G`, `B`, `A` channel values
#define PCOLOR(N, F, R, G, B, A) RAYGUI_CLITERAL(GuiDMProperty){N, GUI_PROP_COLOR, F, .value.vcolor = {R, G, B, A} }
// Create a collapsable section named `N` with `F` flags and the next `C` properties as children.
// !! A section cannot hold another section as a child !!
#define PSECTION(N, F, C) RAYGUI_CLITERAL(GuiDMProperty){N, GUI_PROP_SECTION, F, .value.vsection = (C)}
#define SIZEOF(A) (sizeof(A)/sizeof(A[0]))
//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
enum GuiDMPropertyTypes {
    GUI_PROP_BOOL = 0,
    GUI_PROP_INT,
    GUI_PROP_FLOAT,
    GUI_PROP_TEXT,
    GUI_PROP_SELECT,
    GUI_PROP_VECTOR2,
    GUI_PROP_VECTOR3,
    GUI_PROP_VECTOR4,
    GUI_PROP_RECT,
    GUI_PROP_COLOR,
    GUI_PROP_SECTION,
};

enum GuiDMPropertyFlags {
    GUI_PFLAG_COLLAPSED = 1 << 0, // is the property expanded or collapsed?
    GUI_PFLAG_DISABLED = 1 << 1, // is this property disabled or enabled?
};

// Data structure for each property
typedef struct {
    char* name;
    short type;
    short flags;
    union {
        bool vbool;
        struct { int val; int min; int max; int step; } vint;
        struct { float val; float min; float max; float step; int precision; } vfloat;
        struct { char* val; int size; } vtext;
        struct { char* val; int active; } vselect;
        int vsection;
        Vec2 v2;
        Vec3 v3;
        Vec4 v4;
        Rectangle vrect;
        Color vcolor;
    } value;
} GuiDMProperty;



//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
// ...

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------

#if defined(__cplusplus)
extern "C" {            // Prevents name mangling of functions
#endif

// Global gui state control functions
RAYGUIAPI void GuiEnable(void);                                         // Enable gui controls (global state)
RAYGUIAPI void GuiDisable(void);                                        // Disable gui controls (global state)
RAYGUIAPI void GuiLock(void);                                           // Lock gui controls (global state)
RAYGUIAPI void GuiUnlock(void);                                         // Unlock gui controls (global state)
RAYGUIAPI bool GuiIsLocked(void);                                       // Check if gui is locked (global state)
RAYGUIAPI void GuiFade(float alpha);                                    // Set gui controls alpha (global state), alpha goes from 0.0f to 1.0f
RAYGUIAPI void GuiSetState(int state);                                  // Set gui state (global state)
RAYGUIAPI int GuiGetState(void);                                        // Get gui state (global state)

// Font set/get functions
RAYGUIAPI void GuiSetFont(TextFont font);                                   // Set gui custom font (global state)
RAYGUIAPI TextFont GuiGetFont(void);                                        // Get gui custom font (global state)

// Style set/get functions
RAYGUIAPI void GuiSetStyle(int control, int property, int value);       // Set one style property
RAYGUIAPI int GuiGetStyle(int control, int property);                   // Get one style property

// Container/separator controls, useful for controls organization
RAYGUIAPI bool GuiWindowBox(Rectangle bounds, const char *title);                                       // Window Box control, shows a window that can be closed
RAYGUIAPI void GuiGroupBox(Rectangle bounds, const char *text);                                         // Group Box control with text name
RAYGUIAPI void GuiLine(Rectangle bounds, const char *text);                                             // Line separator control, could contain text
RAYGUIAPI void GuiPanel(Rectangle bounds);                                                              // Panel control, useful to group controls
RAYGUIAPI Rectangle GuiScrollPanel(Rectangle bounds, Rectangle content, Vector2 *scroll);               // Scroll Panel control

// Basic controls set
RAYGUIAPI void GuiLabel(Rectangle bounds, const char *text);                                            // Label control, shows text
RAYGUIAPI bool GuiButton(Rectangle bounds, const char *text);                                           // Button control, returns true when clicked
RAYGUIAPI bool GuiLabelButton(Rectangle bounds, const char *text);                                      // Label button control, show true when clicked
RAYGUIAPI bool GuiToggle(Rectangle bounds, const char *text, bool active);                              // Toggle Button control, returns true when active
RAYGUIAPI int GuiToggleGroup(Rectangle bounds, const char *text, int active);                           // Toggle Group control, returns active toggle index
RAYGUIAPI bool GuiCheckBox(Rectangle bounds, const char *text, bool checked);                           // Check Box control, returns true when active
RAYGUIAPI int GuiComboBox(Rectangle bounds, const char *text, int active);                              // Combo Box control, returns selected item index
RAYGUIAPI bool GuiDropdownBox(Rectangle bounds, const char *text, int *active, bool editMode);          // Dropdown Box control, returns selected item
RAYGUIAPI bool GuiSpinner(Rectangle bounds, const char *text, int *value, int minValue, int maxValue, bool editMode);     // Spinner control, returns selected value
RAYGUIAPI bool GuiValueBox(Rectangle bounds, const char *text, int *value, int minValue, int maxValue, bool editMode);    // Value Box control, updates input text with numbers
RAYGUIAPI bool GuiTextBox(Rectangle bounds, char *text, int textSize, bool editMode);                   // Text Box control, updates input text
RAYGUIAPI bool GuiTextBoxMulti(Rectangle bounds, char *text, int textSize, bool editMode);              // Text Box control with multiple lines
RAYGUIAPI float GuiSlider(Rectangle bounds, const char *textLeft, const char *textRight, float value, float minValue, float maxValue);       // Slider control, returns selected value
RAYGUIAPI float GuiSliderBar(Rectangle bounds, const char *textLeft, const char *textRight, float value, float minValue, float maxValue);    // Slider Bar control, returns selected value
RAYGUIAPI float GuiProgressBar(Rectangle bounds, const char *textLeft, const char *textRight, float value, float minValue, float maxValue);  // Progress Bar control, shows current progress value
RAYGUIAPI void GuiStatusBar(Rectangle bounds, const char *text);                                        // Status Bar control, shows info text
RAYGUIAPI void GuiDummyRec(Rectangle bounds, const char *text);                                         // Dummy control for placeholders
RAYGUIAPI int GuiScrollBar(Rectangle bounds, int value, int minValue, int maxValue);                    // Scroll Bar control
RAYGUIAPI Vector2 GuiGrid(Rectangle bounds, float spacing, int subdivs);                                // Grid control


// Advance controls set
RAYGUIAPI int GuiListView(Rectangle bounds, const char *text, int *scrollIndex, int active);            // List View control, returns selected list item index
RAYGUIAPI int GuiListViewEx(Rectangle bounds, const char **text, int count, int *focus, int *scrollIndex, int active);      // List View with extended parameters
RAYGUIAPI int GuiMessageBox(Rectangle bounds, const char *title, const char *message, const char *buttons);                 // Message Box control, displays a message
RAYGUIAPI int GuiTextInputBox(Rectangle bounds, const char *title, const char *message, const char *buttons, char *text);   // Text Input Box control, ask for text
RAYGUIAPI Color GuiColorPicker(Rectangle bounds, Color color);                                          // Color Picker control (multiple color controls)
RAYGUIAPI Color GuiColorPanel(Rectangle bounds, Color color);                                           // Color Panel control
RAYGUIAPI float GuiColorBarAlpha(Rectangle bounds, float alpha);                                        // Color Bar Alpha control
RAYGUIAPI float GuiColorBarHue(Rectangle bounds, float value);                                          // Color Bar Hue control

// Styles loading functions
RAYGUIAPI void GuiLoadStyle(const char *fileName);              // Load style file over global style variable (.rgs)
RAYGUIAPI void GuiLoadStyleDefault(void);                       // Load style default over global style

/*
typedef GuiStyle (unsigned int *)
RAYGUIAPI GuiStyle LoadGuiStyle(const char *fileName);          // Load style from file (.rgs)
RAYGUIAPI void UnloadGuiStyle(GuiStyle style);                  // Unload style
*/

RAYGUIAPI const char *GuiIconText(int iconId, const char *text); // Get text with icon id prepended (if supported)

#if !defined(RAYGUI_NO_RICONS)
// Gui icons functionality
RAYGUIAPI void GuiDrawIcon(int iconId, int posX, int posY, int pixelSize, Color color);

RAYGUIAPI unsigned int *GuiGetIcons(void);                      // Get full icons data pointer
RAYGUIAPI unsigned int *GuiGetIconData(int iconId);             // Get icon bit data
RAYGUIAPI void GuiSetIconData(int iconId, unsigned int *data);  // Set icon bit data

RAYGUIAPI void GuiSetIconPixel(int iconId, int x, int y);       // Set icon pixel value
RAYGUIAPI void GuiClearIconPixel(int iconId, int x, int y);     // Clear icon pixel value
RAYGUIAPI bool GuiCheckIconPixel(int iconId, int x, int y);     // Check icon pixel value
#endif

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------

// A more advanced `GuiValueBox()` supporting float/int values with specified `precision`, cursor movements, cut/copy/paste and
// other keybord shortcuts. Needed by `GuiDMSpinner()` !!
// `precision` should be between 1-7 for float values and 0 for int values (maybe 15 for doubles but that was not tested)
// WARNING: The bounds should be set big enough else the text will overflow and things will break
// WARNING: Sometimes the last decimal value could differ, this is probably due to rounding
double GuiDMValueBox(Rectangle bounds, double value, double minValue, double maxValue, int precision, bool editMode);

// A more advanced `GuiSpinner()` using `GuiDMValueBox()` for displaying the values.
// This was needed because `GuiSpinner()` can't display float values and editing values is somewhat hard.
// This is by no means perfect but should be more user friendly than the default control provided by raygui.
double GuiDMSpinner(Rectangle bounds, double value, double minValue, double maxValue, double step, int precision, bool editMode);

// Works just like `GuiListViewEx()` but with an array of properties instead of text.
void GuiDMPropertyList(Rectangle bounds, GuiDMProperty* props, int count, int* focus, int* scrollIndex);

// Handy function to save properties to a file. Returns false on failure or true otherwise.
bool GuiDMSaveProperties(const char* file, GuiDMProperty* props, int count);


#if defined(__cplusplus)
}            // Prevents name mangling of functions
#endif

#endif // RAYGUI_H

/***********************************************************************************
*
*   RAYGUI IMPLEMENTATION
*
************************************************************************************/

#if defined(RAYGUI_IMPLEMENTATION)


#endif      // RAYGUI_IMPLEMENTATION
