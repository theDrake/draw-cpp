/******************************************************************************
   Filename: Draw.h

     Author: David C. Drake (http://davidcdrake.com), with initial assistance
             from Dr. Barton Stander (http://cit.dixie.edu/faculty/stander.php)

Description: Header file for "Draw," a simple drawing program for experimenting
             with OpenGL, Bezier curves, etc.
******************************************************************************/

#pragma once

#include <iostream>
#include <fstream>
#include <cmath>
#include <cstring>
#include <vector>
#include "glut.h"

using namespace std;

const int INPUT_STR_LEN = 500;
const int FILLED        = 0;
const int OUTLINED      = 1;

enum ShapeType
{
  NONE,
  LINE,
  BEZIER_CURVE,
  RECTANGLE,
  TRIANGLE,
  PENTAGON,
  CIRCLE,

  NUM_SHAPE_TYPES
};

enum Color
{
  WHITE,
  SILVER,
  GRAY,
  BLACK,
  RED,
  MAROON,
  YELLOW,
  OLIVE,
  LIME,
  GREEN,
  AQUA,
  TEAL,
  BLUE,
  NAVY,
  FUCHSIA,
  PURPLE,

  NUM_COLORS
};

const ShapeType DEFAULT_MODE         = LINE;
const double DEFAULT_RED             = 0.25;
const double DEFAULT_GREEN           = 0.5;
const double DEFAULT_BLUE            = 0.75;
const double CONTROL_PANEL_WIDTH     = 200.0;
const double CONTROL_PANEL_RED       = 0.7;
const double CONTROL_PANEL_GREEN     = 0.7;
const double CONTROL_PANEL_BLUE      = 0.7;
const double DEFAULT_BUTTON_MARGIN_X = 15.0;
const double DEFAULT_BUTTON_MARGIN_Y = 2.0;
const double DEFAULT_BUTTON_WIDTH    = CONTROL_PANEL_WIDTH -
                                       (2 * DEFAULT_BUTTON_MARGIN_X);
const double DEFAULT_BUTTON_HEIGHT   = 20.0;
const double DEFAULT_BUTTON_RED      = 0.9;
const double DEFAULT_BUTTON_GREEN    = 0.9;
const double DEFAULT_BUTTON_BLUE     = 0.9;
const double PI                      = atan(1.0) * 4.0;
const int    CURVE_RESOLUTION        = 32;

void DrawRectangle(double x1, double y1, double x2, double y2);
void DrawTriangle(double x1, double y1,
                  double x2, double y2,
                  double x3, double y3);
void DrawCircle(double x1, double y1, double radius);
void DrawText(double x, double y, char * string);
void SetColor(double r, double g, double b);
ShapeType SetShapeMode(ShapeType m);
void SetFilled(bool b);
void DeselectAllShapes();
