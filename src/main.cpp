#include <stdio.h>
#include <string.h>
#include "ui.h"
#include "geometry.hpp"  // Vec3D, Matrix3D
#include "rubiks.hpp"  // RubiksCube
#include "rubiks_handler.hpp"  // AnimationHandler, MouseHander, Scrambler

rubiks::RubiksCube g_rubiks;
rubiks::AnimationHandler *g_animation_handler;
rubiks::MouseHandler *g_mouse_handler;
uiAreaHandler handler;

// helper to quickly set a brush color
static void SetSolidBrush(uiDrawBrush *brush, uint32_t color, double alpha)
{
	uint8_t component;

	brush->Type = uiDrawBrushTypeSolid;
	component = (uint8_t) ((color >> 16) & 0xFF);
	brush->R = ((double) component) / 255;
	component = (uint8_t) ((color >> 8) & 0xFF);
	brush->G = ((double) component) / 255;
	component = (uint8_t) (color & 0xFF);
	brush->B = ((double) component) / 255;
	brush->A = alpha;
}

// helper to draw a quad face
static void DrawQuad(uiAreaDrawParams *p, const std::vector<Vec3D>& vertices, const Quad& face)
{
    uiDrawPath *path;
    uiDrawBrush brush;
    SetSolidBrush(&brush, face.color, 1.0);
    path = uiDrawNewPath(uiDrawFillModeWinding);

    Vec3D v1 = vertices[face.v1];
    Vec3D v2 = vertices[face.v2];
    Vec3D v3 = vertices[face.v3];
    Vec3D v4 = vertices[face.v4];
	uiDrawPathNewFigure(path, v1.x, v1.y);
	uiDrawPathLineTo(path, v2.x, v2.y);
	uiDrawPathLineTo(path, v3.x, v3.y);
	uiDrawPathLineTo(path, v4.x, v4.y);
	uiDrawPathCloseFigure(path);

    uiDrawPathEnd(path);
    uiDrawFill(p->Context, path, &brush);
    uiDrawFreePath(path);
}

// This will be called by uiAreaQueueRedrawAll
static void HandlerDraw(uiAreaHandler *a, uiArea *area, uiAreaDrawParams *p)
{
    // Project rubiks cube to screen
    std::vector<Vec3D> projected_vertices;
    std::vector<Quad> visible_faces;
    g_rubiks.Project(projected_vertices, visible_faces);

    // fill the area
    uiDrawPath *path;
    uiDrawBrush brush;
	SetSolidBrush(&brush, rubiks::COLOR_GRAY, 1.0);
	path = uiDrawNewPath(uiDrawFillModeWinding);
	uiDrawPathAddRectangle(path, 0, 0, p->AreaWidth, p->AreaHeight);
	uiDrawPathEnd(path);
	uiDrawFill(p->Context, path, &brush);
	uiDrawFreePath(path);

    // Draw faces    
    for (const Quad& face: visible_faces) {
        DrawQuad(p, projected_vertices, face);
    }
}

static void HandlerMouseEvent(uiAreaHandler *a, uiArea *area, uiAreaMouseEvent *e)
{
    if (g_animation_handler->IsAnimating()) return;

    Vec3D mouse_pos = Vec3D(e->X, e->Y, 0.0);

    int animated = g_mouse_handler->Step(mouse_pos, e->Down, e->Up);

    if (animated)
        uiAreaQueueRedrawAll(area);
}

static void HandlerMouseCrossed(uiAreaHandler *ah, uiArea *a, int left)
{
	// do nothing
}

static void HandlerDragBroken(uiAreaHandler *ah, uiArea *a)
{
	// do nothing
}

static int HandlerKeyEvent(uiAreaHandler *ah, uiArea *a, uiAreaKeyEvent *e)
{
	// reject all keys
	return 0;
}

static int OnClosing(uiWindow *w, void *data)
{
    uiQuit();
    return 1;
}

static int OnShouldQuit(void *data)
{
    uiWindow *mainwin = uiWindow(data);
    uiControlDestroy(uiControl(mainwin));
    return 1;
}

static void OnResetView(uiButton *sender, void *data) {
    g_rubiks.InitializeGlobalRotation();
    g_rubiks.InitializeFaceRotation();
    g_mouse_handler->InitializeState();
    uiAreaQueueRedrawAll(uiArea(data));
}

static void OnResetColors(uiButton *sender, void *data) {
    g_rubiks.InitializeGlobalRotation();
    g_rubiks.InitializeFaceRotation();
    g_rubiks.InitializeColors();
    g_mouse_handler->InitializeState();
    g_animation_handler->ClearAnimations();
    uiAreaQueueRedrawAll(uiArea(data));
}

const int SCRAMBLE_STEPS = 50;

static void OnScramble(uiButton *sender, void *data) {
    if (g_animation_handler->IsAnimating()) return;

    g_mouse_handler->InitializeState();
    g_rubiks.InitializeFaceRotation();

    rubiks::Scrambler scrambler;
    for (int i = 0; i < SCRAMBLE_STEPS; i++) {
        rubiks::AnimationQueue queue = scrambler.GenerateFaceRotation();
        g_animation_handler->Push(queue);
    }
}

static int OnAnimating(void *data)
{
    // Process animation queues
    int animated = g_animation_handler->Step();

    if (animated)
        uiAreaQueueRedrawAll(uiArea(data));

    return 1;
}

void CreateWindow()
{
    // Main window
    uiWindow* mainwin = uiNewWindow("libui Rubiks Demo", 400, 400, 1);
    uiWindowOnClosing(mainwin, OnClosing, NULL);
    uiOnShouldQuit(OnShouldQuit, mainwin);
    uiWindowSetMargined(mainwin, 1);

    // Main container
    uiBox *vbox = uiNewVerticalBox();
	uiBoxSetPadded(vbox, 1);
	uiWindowSetChild(mainwin, uiControl(vbox));

    // Drawing area
    handler.Draw = HandlerDraw;
	handler.MouseEvent = HandlerMouseEvent;
	handler.MouseCrossed = HandlerMouseCrossed;
	handler.DragBroken = HandlerDragBroken;
	handler.KeyEvent = HandlerKeyEvent;

    uiArea *area = uiNewArea(&handler);
	uiBoxAppend(vbox, uiControl(area), 1);

    uiTimer(10, OnAnimating, area);

    // Buttons
    uiBox *button_box = uiNewHorizontalBox();
	uiBoxSetPadded(button_box, 1);

    uiButton *button = uiNewButton("Reset View");
    uiButtonOnClicked(button, OnResetView, area);
	uiBoxAppend(button_box, uiControl(button), 0);

    button = uiNewButton("Reset Colors");
    uiButtonOnClicked(button, OnResetColors, area);
	uiBoxAppend(button_box, uiControl(button), 0);

    button = uiNewButton("Scramble");
    uiButtonOnClicked(button, OnScramble, area);
	uiBoxAppend(button_box, uiControl(button), 0);

	uiBoxAppend(vbox, uiControl(button_box), 0);

    // Make them visible
    uiControlShow(uiControl(mainwin));
}

int main(void)
{
    // Initialize libui
    uiInitOptions options;
    const char *err;

    memset(&options, 0, sizeof (uiInitOptions));
    err = uiInit(&options);
    if (err != NULL) {
        fprintf(stderr, "error initializing libui: %s", err);
        uiFreeInitError(err);
        return 1;
    }

    // Initialize rubiks cube
    g_rubiks.Initialize();
    g_animation_handler = new rubiks::AnimationHandler(&g_rubiks);
    g_mouse_handler = new rubiks::MouseHandler(&g_rubiks, g_animation_handler);

    // Craete main window
    CreateWindow();

    // Start main loop
    uiMain();

    delete g_animation_handler;
    delete g_mouse_handler;
    return 0;
}
