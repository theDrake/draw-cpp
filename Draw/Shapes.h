//*****************************************************************************
//    Filename: Shapes.h
//
//      Author: David C. Drake (http://www.davidcdrake.com), with assistance
//              from Bart Stander (http://cit.dixie.edu/faculty/stander.php).
//
// Description: Header file for the following shape-related classes: Point2D,
//              Shape, Line, BezierCurve, Rectangle, Triangle, Pentagon,
//              Circle, Button, Slider, and Label.
//*****************************************************************************

#pragma once

#include "Draw.h"

enum ButtonType
{
  MODE_BUTTON,
  COLOR_BUTTON,
  RGB_SLIDER,
  FILL_BUTTON,
  OUTLINE_BUTTON,
  UNDO_BUTTON,
  CLEAR_BUTTON,
  SAVE_BUTTON,
  LOAD_BUTTON,
  QUIT_BUTTON,
  LABEL,

  NUM_BUTTON_TYPES
};

const double POINT_RADIUS             = 4.0;
const double DEFAULT_POINT_RED        = 0.0;
const double DEFAULT_POINT_GREEN      = 0.0;
const double DEFAULT_POINT_BLUE       = 0.0;
const double BUTTON_OUTLINE_THICKNESS = 2.0;
const double BUTTON_TEXT_OFFSET_X     = 10.0;
const double BUTTON_TEXT_OFFSET_Y     = 15.0;
const int    BUTTON_TEXT_MAX_LEN      = 30;

class Point2D
{
public:
  Point2D(const double x, const double y);
  ~Point2D() {}
  void Draw();
  const double SetX(double x) { return mX = x; }
  const double SetY(double y) { return mY = y; }
  const double GetX() const   { return mX;     }
  const double GetY() const   { return mY;     }
  bool Contains(double x, double y) const;
private:
  double mX, mY;
};

class Shape
{
public:
  Shape(vector<Point2D *> points,
        const double r,
        const double g,
        const double b,
        bool filled);
  virtual ~Shape();
  virtual void Draw() = 0;
  virtual void DrawPoints();
  virtual void Adjust(double x, double y, Point2D * selectedPoint);
  virtual void Move(double x, double y, Point2D * selectedPoint);
  void SetColor(double r, double g, double b);
  const double SetRed(double r)        { return mRed = r;         }
  const double SetGreen(double g)      { return mGreen = g;       }
  const double SetBlue(double b)       { return mBlue = b;        }
  const double GetRed() const          { return mRed;             }
  const double GetGreen() const        { return mGreen;           }
  const double GetBlue() const         { return mBlue;            }
  const bool IsFilled() const          { return mFilled;          }
  const ShapeType GetShapeType() const { return mShapeType;       }
  Point2D * GetPointAt(int i) const    { return mVertices[i];     }
  int NumPoints() const                { return mVertices.size(); }
  bool SetSelected(const bool b)       { return mSelected = b;    }
  bool IsSelected() const              { return mSelected;        }
protected:
  vector<Point2D *> mVertices;
  double mRed, mGreen, mBlue;
  ShapeType mShapeType;
  bool mSelected, mFilled;
};

class Line : public Shape
{
public:
  Line(vector<Point2D *> points,
       const double r,
       const double g,
       const double b);
  void Draw();
};

class BezierCurve : public Shape
{
public:
  BezierCurve(vector<Point2D *> points,
              const double r,
              const double g,
              const double b);
  void Draw();
  Point2D * Evaluate(double t) const;
};

class Rectangle : public Shape
{
public:
  Rectangle(vector<Point2D *> points,
            const double r,
            const double g,
            const double b,
            bool filled);
  void Draw();
  void Adjust(double x, double y, Point2D * selectedPoint);
  void Move(double x, double y, Point2D * selectedPoint);
  bool Contains(double x, double y) const;
  double GetTop() const    { return mTop;           }
  double GetBottom() const { return mBottom;        }
  double GetRight() const  { return mRight;         }
  double GetLeft() const   { return mLeft;          }
  double GetLength() const { return mRight - mLeft; }
  double GetHeight() const { return mTop - mBottom; }
protected:
  double mTop, mBottom, mLeft, mRight;
};

class Triangle : public Shape
{
public:
  Triangle(vector<Point2D *> points,
           const double r,
           const double g,
           const double b,
           bool filled);
  void Draw();
};

class Pentagon : public Shape
{
public:
  Pentagon(vector<Point2D *> points,
           const double r,
           const double g,
           const double b,
           bool filled);
  void Draw();
};

class Circle : public Shape
{
public:
  Circle(vector<Point2D *> points, const double r, const double g, const double b, bool filled);
  Point2D * GetCenter() const     { return mVertices[0];           }
  double GetRadius() const        { return mRadius;                }
  double GetArea() const          { return PI * mRadius * mRadius; }
  double GetCircumference() const { return 2 * PI * mRadius;       }
  void Draw();
  void Adjust(double x, double y, Point2D * selectedPoint);
protected:
  double mRadius;
};

class Button : public Rectangle
{
public:
  Button(vector<Point2D *> points,
       const double r,
       const double g,
       const double b,
       const char * text,
       const int buttonType,
       const int associatedID);
  virtual void Draw();
  bool IsButtonType(const int type) const { return mButtonType == type; }
  int GetAssociatedID() const             { return mAssociatedID;       }
  bool SetPressed(const bool b)           { return mPressed = b;        }
  bool IsPressed() const                  { return mPressed;            }
  const char * SetText(const char * text);
protected:
  char mText[BUTTON_TEXT_MAX_LEN + 1];
  int mButtonType;
  int mAssociatedID;
  bool mPressed;
};

class Slider : public Button
{
public:
  Slider(vector<Point2D *> points,
       const double r,
       const double g,
       const double b,
       const int associatedID);
  virtual void Draw();
  double SetSliderLength(const double length) {return mSliderLength = length;}
private:
  double mSliderLength;
};

class Label : public Button
{
public:
  Label(vector<Point2D *> points,
      const double r,
      const double g,
      const double b,
      const char * text);
  virtual void Draw();
};
