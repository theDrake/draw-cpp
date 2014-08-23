/******************************************************************************
   Filename: Draw.cpp

     Author: David C. Drake (http://davidcdrake.com), with initial assistance
             from Dr. Barton Stander (http://cit.dixie.edu/faculty/stander.php)

Description: Main C++ file for "Draw," a simple drawing program for
             experimenting with OpenGL, Bezier curves, etc.
******************************************************************************/

#include "Draw.h"
#include "Shapes.h"

double gScreenX          = 900;
double gScreenY          = 600;
bool gLeftDragging       = false;
bool gRightDragging      = false;
bool gPressingShift      = false;
Point2D * gSelectedPoint = NULL;
Shape * gSelectedShape   = NULL;

vector<Point2D *> gPoints;
vector<Shape *> gShapes;
vector<Button *> gButtons;
vector<Label *> gLabels;
ShapeType gShapeMode;
double gRed, gGreen, gBlue;
bool gFilled;

//
// Functions that draw basic primitives:
//

void DrawRectangle(double x1, double y1, double x2, double y2)
{
  glBegin(GL_QUADS);
  glVertex2d(x1, y1);
  glVertex2d(x2, y1);
  glVertex2d(x2, y2);
  glVertex2d(x1, y2);
  glEnd();
}

void DrawTriangle(double x1, double y1,
                  double x2, double y2,
                  double x3, double y3)
{
  glBegin(GL_TRIANGLES);
  glVertex2d(x1,y1);
  glVertex2d(x2,y2);
  glVertex2d(x3,y3);
  glEnd();
}

void DrawCircle(double x1, double y1, double radius)
{
  glBegin(GL_POLYGON);
  for(int i = 0; i < CURVE_RESOLUTION; i++)
  {
    double theta = (double) i / CURVE_RESOLUTION * 2.0 * PI;
    double x = x1 + radius * cos(theta);
    double y = y1 + radius * sin(theta);
    glVertex2d(x, y);
  }
  glEnd();
}

void DrawText(double x, double y, char * string)
{
  void *font = GLUT_BITMAP_9_BY_15;

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

  int len, i;
  glRasterPos2d(x, y);
  len = (int) strlen(string);
  for (i = 0; i < len; i++)
  {
    glutBitmapCharacter(font, string[i]);
  }

    glDisable(GL_BLEND);
}

//
// GLUT callback functions:
//

void display(void)
{
  glClear(GL_COLOR_BUFFER_BIT);

  // Draw all user-created shapes:
  vector<Shape *>::iterator shapeIter;
  for (shapeIter = gShapes.begin(); shapeIter < gShapes.end(); ++shapeIter)
  {
    (*shapeIter)->Draw();
  }

  // Draw all user-created points that are not yet part of a shape:
  vector<Point2D *>::iterator pointIter;
  for (pointIter = gPoints.begin(); pointIter < gPoints.end(); ++pointIter)
  {
    (*pointIter)->Draw();
  }

  // Draw the control panel on the left side of the screen:
  glColor3d(CONTROL_PANEL_RED, CONTROL_PANEL_GREEN, CONTROL_PANEL_BLUE);
  DrawRectangle(0, 0, CONTROL_PANEL_WIDTH, gScreenY);
  vector<Label *>::iterator labelIter;
  for (labelIter = gLabels.begin(); labelIter < gLabels.end(); ++labelIter)
  {
    (*labelIter)->Draw();
  }
  vector<Button *>::iterator buttonIter;
  for (buttonIter = gButtons.begin();
       buttonIter < gButtons.end();
       ++buttonIter)
  {
    (*buttonIter)->Draw();
  }

  glutSwapBuffers();
}

void keyboard(unsigned char c, int x, int y)
{
  switch (c)
  {
    case 27:  // Escape character.
      exit(0);
      break;
    case 'L':
    case 'l':
      SetShapeMode(LINE);
      break;
    case 'B':
    case 'b':
      SetShapeMode(BEZIER_CURVE);
      break;
    case 'R':
    case 'r':
      SetShapeMode(RECTANGLE);
      break;
    case 'T':
    case 't':
      SetShapeMode(TRIANGLE);
      break;
    case 'P':
    case 'p':
      SetShapeMode(PENTAGON);
      break;
    case 'C':
    case 'c':
      SetShapeMode(CIRCLE);
      break;
    default:
      return;  // If insignificant, return without calling glutPostRedisplay().
  }

  glutPostRedisplay();
}

void reshape(int w, int h)
{
  // Reset our global variables to the new width and height:
  gScreenX = w;
  gScreenY = h;

  // Set the pixel resolution of the final picture (screen coordinates):
  glViewport(0, 0, w, h);

  // Set the projection mode to 2D orthographic, and set the world coordinates:
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, w, 0, h);
  glMatrixMode(GL_MODELVIEW);
}

void mouse(int mouse_button, int state, int x, int y)
{
  y = gScreenY - y;

  // Left mouse button:
  if (mouse_button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
  {
    // Left-clicking within the canvas:
    if (x > CONTROL_PANEL_WIDTH)
    {
      // If not already dragging and a point's clicked, select it for dragging:
      if (!gLeftDragging)
      {
        vector<Point2D *>::iterator pointIter;
        for (pointIter = gPoints.begin();
             pointIter < gPoints.end();
             ++pointIter)
        {
          if ((*pointIter)->Contains(x, y))
          {
            gSelectedPoint = (*pointIter);
            gLeftDragging = true;
            break;
          }
        }
        if (!gLeftDragging)
        {
          vector<Shape *>::iterator shapeIter;
          for (shapeIter = gShapes.begin();
               shapeIter < gShapes.end();
               ++shapeIter)
          {
            for (int i = 0; i < (*shapeIter)->NumPoints(); ++i)
            {
              if ((*shapeIter)->GetPointAt(i)->Contains(x, y))
              {
                gSelectedPoint = (*shapeIter)->GetPointAt(i);
                DeselectAllShapes();
                gSelectedShape = (*shapeIter);
                gSelectedShape->SetSelected(true);
                gLeftDragging = true;
                break;
              }
            }
            if (gLeftDragging)
              break;
          }
        }
      }

      // If a point wasn't clicked, a new point should be created:
      if (!gLeftDragging)
      {
        if (gPoints.empty())
        {
          DeselectAllShapes();
        }
        gPoints.push_back(new Point2D(x, y));
        switch(gShapeMode)
        {
        case LINE:
          if (gPoints.size() >= 2)
          {
            DeselectAllShapes();
            gShapes.push_back(new Line(gPoints, gRed, gGreen, gBlue));
            gPoints.clear();
          }
          break;
        case BEZIER_CURVE:
          if (gPoints.size() >= 4)
          {
            DeselectAllShapes();
            gShapes.push_back(new BezierCurve(gPoints, gRed, gGreen, gBlue));
            gPoints.clear();
          }
          break;
        case RECTANGLE:
          if (gPoints.size() >= 2)
          {
            DeselectAllShapes();
            gShapes.push_back(new Rectangle(gPoints,
                                            gRed,
                                            gGreen,
                                            gBlue,
                                            gFilled));
            gPoints.clear();
          }
          break;
        case TRIANGLE:
          if (gPoints.size() >= 3)
          {
            DeselectAllShapes();
            gShapes.push_back(new Triangle(gPoints,
                                           gRed,
                                           gGreen,
                                           gBlue,
                                           gFilled));
            gPoints.clear();
          }
          break;
        case PENTAGON:
          if (gPoints.size() >= 5)
          {
            DeselectAllShapes();
            gShapes.push_back(new Pentagon(gPoints,
                                           gRed,
                                           gGreen,
                                           gBlue,
                                           gFilled));
            gPoints.clear();
          }
          break;
        case CIRCLE:
          if (gPoints.size() >= 2)
          {
            DeselectAllShapes();
            gShapes.push_back(new Circle(gPoints,
                                         gRed,
                                         gGreen,
                                         gBlue,
                                         gFilled));
            gPoints.clear();
          }
          break;
        default:
          break;
        }
      }
    }

    // Left-clicking within the control panel:
    else
    {
      vector<Button *>::iterator buttonIter;
      for (buttonIter = gButtons.begin();
           buttonIter < gButtons.end();
           ++buttonIter)
      {
        if ((*buttonIter)->Contains(x, y))
        {
          (*buttonIter)->SetPressed(true);
          if ((*buttonIter)->IsButtonType(MODE_BUTTON))
          {
            SetShapeMode((ShapeType) (*buttonIter)->GetAssociatedID());
          }
          else if ((*buttonIter)->IsButtonType(FILL_BUTTON))
          {
            SetFilled(true);
          }
          else if ((*buttonIter)->IsButtonType(OUTLINE_BUTTON))
          {
            SetFilled(false);
          }
          else if ((*buttonIter)->IsButtonType(COLOR_BUTTON))
          {
            SetColor((*buttonIter)->GetRed(),
                 (*buttonIter)->GetGreen(),
                 (*buttonIter)->GetBlue());
          }
          else if ((*buttonIter)->IsButtonType(RGB_SLIDER))
          {
            ((Slider *) *buttonIter)->SetSliderLength(x -
              (*buttonIter)->GetLeft());

            double newRGB = (x - (*buttonIter)->GetLeft()) /
                            (*buttonIter)->GetLength();
            switch((*buttonIter)->GetAssociatedID())
            {
            case RED:
              gRed = newRGB;
              break;
            case GREEN:
              gGreen = newRGB;
              break;
            case BLUE:
              gBlue = newRGB;
              break;
            default:
              break;
            }
          }
          else if ((*buttonIter)->IsButtonType(SAVE_BUTTON))
          {
            ofstream fout("save.txt");

            vector<Shape *>::iterator shapeIter;
            for (shapeIter = gShapes.begin();
                 shapeIter < gShapes.end();
                 ++shapeIter)
            {
              fout << (*shapeIter)->GetShapeType() << " ";
              for (int i = 0; i < (*shapeIter)->NumPoints(); ++i)
              {
                fout << (*shapeIter)->GetPointAt(i)->GetX() << " ";
                fout << (*shapeIter)->GetPointAt(i)->GetY() << " ";
              }
              fout << (*shapeIter)->GetRed() << " ";
              fout << (*shapeIter)->GetGreen() << " ";
              fout << (*shapeIter)->GetBlue() << " ";
              fout << (*shapeIter)->IsFilled() << endl;
            }

            if (!gPoints.empty())
            {
              fout << NONE << " ";
              vector<Point2D *>::iterator pointIter;
              for (pointIter = gPoints.begin();
                   pointIter < gPoints.end();
                   ++pointIter)
              {
                fout << (*pointIter)->GetX() << " ";
                fout << (*pointIter)->GetY() << " ";
              }
              fout << endl;
            }

            fout.close();
          }
          else if ((*buttonIter)->IsButtonType(LOAD_BUTTON))
          {
            ifstream fin("save.txt");

            // First, clear the canvas:
            if (fin.good())
            {
              gShapes.clear();
              gPoints.clear();
            }

            // Now read input data from the save file:
            int currentShapeType;
            double r, g, b;
            bool filled;
            char * line = NULL;
            vector<double> input;
            while (fin.good())
            {
              line = (char *) malloc((INPUT_STR_LEN + 1) * sizeof(char));
              fin >> currentShapeType;
              if (fin.good())
              {
                fin.getline(line, INPUT_STR_LEN);
                line = strtok(line, " \n");
                while (line)
                {
                  input.push_back(atof(line));
                  line = strtok(NULL, " \n");
                }
              }
              if (fin.good())
              {
                vector<double>::iterator doubleIter;
                for (doubleIter = input.begin();
                     distance(doubleIter, input.end()) > 4;
                     doubleIter += 2)
                {
                  gPoints.push_back(new Point2D(*doubleIter,
                                                *(doubleIter + 1)));
                }
                if (currentShapeType == NONE)
                {
                  while (distance(doubleIter, input.end()) > 1)
                  {
                    gPoints.push_back(new Point2D(*doubleIter,
                                                  *(doubleIter + 1)));
                    doubleIter += 2;
                  }
                }
                else if (distance(doubleIter, input.end()) == 4)
                {
                  r = *(doubleIter++);
                  g = *(doubleIter++);
                  b = *(doubleIter++);
                  filled = *doubleIter;
                }
                else
                {
                  cerr << "Error: invalid data stored in save file." << endl;
                  break;
                }
                input.clear();

                switch(currentShapeType)
                {
                case LINE:
                  gShapes.push_back(new Line(gPoints, r, g, b));
                  gPoints.clear();
                  break;
                case BEZIER_CURVE:
                  gShapes.push_back(new BezierCurve(gPoints, r, g, b));
                  gPoints.clear();
                  break;
                case RECTANGLE:
                  gShapes.push_back(new Rectangle(gPoints, r, g, b, filled));
                  gPoints.clear();
                  break;
                case TRIANGLE:
                  gShapes.push_back(new Triangle(gPoints, r, g, b, filled));
                  gPoints.clear();
                  break;
                case PENTAGON:
                  gShapes.push_back(new Pentagon(gPoints, r, g, b, filled));
                  gPoints.clear();
                  break;
                case CIRCLE:
                  gShapes.push_back(new Circle(gPoints, r, g, b, filled));
                  gPoints.clear();
                  break;
                default:  // currentShapeType == NONE
                  break;
                }
              }

              if (line != NULL)
              {
                free(line);
                line = NULL;
              }
            }

            if (line != NULL)
              free(line);
            fin.close();
          }
          else if ((*buttonIter)->IsButtonType(UNDO_BUTTON))
          {
            if (!gPoints.empty())
            {
              gPoints.pop_back();
            }
            else if (!gShapes.empty())
            {
              gShapes.pop_back();
            }
          }
          else if ((*buttonIter)->IsButtonType(CLEAR_BUTTON))
          {
            gShapes.clear();
            gPoints.clear();
          }
          else if ((*buttonIter)->IsButtonType(QUIT_BUTTON))
          {
            exit(0);
          }
          break;
        }
      }
    }
  }

  // When left button is released, ensure no point is selected for dragging:
  if (mouse_button == GLUT_LEFT_BUTTON && state == GLUT_UP)
  {
    gLeftDragging  = false;
    gSelectedPoint = NULL;

    vector<Button *>::iterator iter;
    for (iter = gButtons.begin(); iter < gButtons.end(); ++iter)
    {
      if ((*iter)->IsPressed())
      {
        (*iter)->SetPressed(false);
      }
    }
  }

  // Right mouse button:
  if (mouse_button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
  {
    // Right-clicking within the canvas:
    if (x > CONTROL_PANEL_WIDTH)
    {
      // If not dragging and a point's been clicked, select it for dragging:
      if (!gRightDragging)
      {
        vector<Point2D *>::iterator pointIter;
        for (pointIter = gPoints.begin();
             pointIter < gPoints.end();
             ++pointIter)
        {
          if ((*pointIter)->Contains(x, y))
          {
            gSelectedPoint = (*pointIter);
            gRightDragging = true;
            break;
          }
        }
        if (!gRightDragging)
        {
          vector<Shape *>::iterator shapeIter;
          for (shapeIter = gShapes.begin();
               shapeIter < gShapes.end();
               ++shapeIter)
          {
            for (int i = 0; i < (*shapeIter)->NumPoints(); ++i)
            {
              if ((*shapeIter)->GetPointAt(i)->Contains(x, y))
              {
                gSelectedPoint = (*shapeIter)->GetPointAt(i);
                DeselectAllShapes();
                gSelectedShape = (*shapeIter);
                gSelectedShape->SetSelected(true);
                gRightDragging = true;
                break;
              }
            }
            if (gRightDragging)
              break;
          }
        }
      }
    }
  }

  // When right button is released, ensure no point is selected for dragging:
  if (mouse_button == GLUT_RIGHT_BUTTON && state == GLUT_UP)
  {
    gRightDragging = false;
    gSelectedPoint = NULL;

    vector<Button *>::iterator iter;
    for (iter = gButtons.begin(); iter < gButtons.end(); ++iter)
    {
      if ((*iter)->IsPressed())
      {
        (*iter)->SetPressed(false);
      }
    }
  }

  // Middle mouse button:
  if (mouse_button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN)
  {
  }
  if (mouse_button == GLUT_MIDDLE_BUTTON && state == GLUT_UP)
  {
  }

  glutPostRedisplay();
}

void motion(int x, int y)
{
  y = gScreenY - y;

  if (gRightDragging)
  {
    if (gSelectedShape)
    {
      if (gSelectedPoint)
      {
        gSelectedShape->Adjust(x, y, gSelectedPoint);
      }
    }
    else if (gSelectedPoint)
    {
      gSelectedPoint->SetX(x);
      gSelectedPoint->SetY(y);
    }
  }
  else if (gLeftDragging)
  {
    if (gSelectedShape)
    {
      gSelectedShape->Move(x, y, gSelectedPoint);
    }
    else if (gSelectedPoint)
    {
      gSelectedPoint->SetX(x);
      gSelectedPoint->SetY(y);
    }
  }

  glutPostRedisplay();
}

void colorMenu(int id)
{
  /*
  if (lastSelectedCurve != -1)
  {
    switch(id)
    {
    case 1:
      b[lastSelectedCurve].SetColor(redMaterial); // or use RGB value
      break;
    default:
      break;
    }
  }
  */
}

//
// Initialization and main functions:
//

void AddButton(const double x1,
         const double y1,
         const double x2,
         const double y2,
         const double r,
         const double g,
         const double b,
         const char * text,
         const int buttonType,
         const int associatedID)
{
  gPoints.clear();
  gPoints.push_back(new Point2D(x1, y1));
  gPoints.push_back(new Point2D(x2, y2));
  gButtons.push_back(new Button(gPoints,
                                r,
                                g,
                                b,
                                text,
                                buttonType,
                                associatedID));
  gPoints.clear();

}

void AddSlider(const double x1,
               const double y1,
               const double x2,
               const double y2,
               const double r,
               const double g,
               const double b,
               const int associatedID)
{
  gPoints.clear();
  gPoints.push_back(new Point2D(x1, y1));
  gPoints.push_back(new Point2D(x2, y2));
  gButtons.push_back(new Slider(gPoints, r, g, b, associatedID));
  gPoints.clear();

}

void AddLabel(const double x1,
              const double y1,
              const double x2,
              const double y2,
              const double r,
              const double g,
              const double b,
              const char * text)
{
  gPoints.clear();
  gPoints.push_back(new Point2D(x1, y1));
  gPoints.push_back(new Point2D(x2, y2));
  gLabels.push_back(new Label(gPoints, r, g, b, text));
  gPoints.clear();
}

void InitializeMyStuff()
{
  int n = 1;  // Serves as each button's y-offset multiplier.

  gShapes.clear();
  gPoints.clear();
  gButtons.clear();
  gLabels.clear();
  gRed   = DEFAULT_RED;
  gGreen = DEFAULT_GREEN;
  gBlue  = DEFAULT_BLUE;

  // Drawing mode label and buttons:

  AddLabel(DEFAULT_BUTTON_MARGIN_X,
           gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
             (n - 1) * DEFAULT_BUTTON_HEIGHT,
           DEFAULT_BUTTON_MARGIN_X + DEFAULT_BUTTON_WIDTH,
           gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
             n * DEFAULT_BUTTON_HEIGHT,
           CONTROL_PANEL_RED,
           CONTROL_PANEL_GREEN,
           CONTROL_PANEL_BLUE,
           "Drawing Mode:");
  ++n;
  AddButton(DEFAULT_BUTTON_MARGIN_X,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              (n - 1) * DEFAULT_BUTTON_HEIGHT,
            DEFAULT_BUTTON_MARGIN_X + DEFAULT_BUTTON_WIDTH,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              n * DEFAULT_BUTTON_HEIGHT,
            DEFAULT_BUTTON_RED,
            DEFAULT_BUTTON_GREEN,
            DEFAULT_BUTTON_BLUE,
            "(L)ine",
            MODE_BUTTON,
            LINE);
  ++n;
  AddButton(DEFAULT_BUTTON_MARGIN_X,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              (n - 1) * DEFAULT_BUTTON_HEIGHT,
            DEFAULT_BUTTON_MARGIN_X + DEFAULT_BUTTON_WIDTH,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              n * DEFAULT_BUTTON_HEIGHT,
            DEFAULT_BUTTON_RED,
            DEFAULT_BUTTON_GREEN,
            DEFAULT_BUTTON_BLUE,
            "(B)ezier Curve",
            MODE_BUTTON,
            BEZIER_CURVE);
  ++n;
  AddButton(DEFAULT_BUTTON_MARGIN_X,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              (n - 1) * DEFAULT_BUTTON_HEIGHT,
            DEFAULT_BUTTON_MARGIN_X + DEFAULT_BUTTON_WIDTH,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              n * DEFAULT_BUTTON_HEIGHT,
            DEFAULT_BUTTON_RED,
            DEFAULT_BUTTON_GREEN,
            DEFAULT_BUTTON_BLUE,
            "(R)ectangle",
            MODE_BUTTON,
            RECTANGLE);
  ++n;
  AddButton(DEFAULT_BUTTON_MARGIN_X,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              (n - 1) * DEFAULT_BUTTON_HEIGHT,
            DEFAULT_BUTTON_MARGIN_X + DEFAULT_BUTTON_WIDTH,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              n * DEFAULT_BUTTON_HEIGHT,
            DEFAULT_BUTTON_RED,
            DEFAULT_BUTTON_GREEN,
            DEFAULT_BUTTON_BLUE,
            "(T)riangle",
            MODE_BUTTON,
            TRIANGLE);
  ++n;
  AddButton(DEFAULT_BUTTON_MARGIN_X,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              (n - 1) * DEFAULT_BUTTON_HEIGHT,
            DEFAULT_BUTTON_MARGIN_X + DEFAULT_BUTTON_WIDTH,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              n * DEFAULT_BUTTON_HEIGHT,
            DEFAULT_BUTTON_RED,
            DEFAULT_BUTTON_GREEN,
            DEFAULT_BUTTON_BLUE,
            "(P)entagon",
            MODE_BUTTON,
            PENTAGON);
  ++n;
  AddButton(DEFAULT_BUTTON_MARGIN_X,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              (n - 1) * DEFAULT_BUTTON_HEIGHT,
            DEFAULT_BUTTON_MARGIN_X + DEFAULT_BUTTON_WIDTH,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              n * DEFAULT_BUTTON_HEIGHT,
            DEFAULT_BUTTON_RED,
            DEFAULT_BUTTON_GREEN,
            DEFAULT_BUTTON_BLUE,
            "(C)ircle",
            MODE_BUTTON,
            CIRCLE);
  ++n;

  // Color label and buttons:

  ++n;
  AddLabel(DEFAULT_BUTTON_MARGIN_X,
           gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
             (n - 1) * DEFAULT_BUTTON_HEIGHT,
           DEFAULT_BUTTON_MARGIN_X + DEFAULT_BUTTON_WIDTH,
           gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
             n * DEFAULT_BUTTON_HEIGHT,
           CONTROL_PANEL_RED,
           CONTROL_PANEL_GREEN,
           CONTROL_PANEL_BLUE,
           "Color:");
  ++n;
  AddSlider(DEFAULT_BUTTON_MARGIN_X,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              (n - 1) * DEFAULT_BUTTON_HEIGHT,
            DEFAULT_BUTTON_MARGIN_X + DEFAULT_BUTTON_WIDTH,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              n * DEFAULT_BUTTON_HEIGHT,
            1.0,  // red
            0.0,  // green
            0.0,  // blue
            RED);
  ++n;
  AddSlider(DEFAULT_BUTTON_MARGIN_X,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              (n - 1) * DEFAULT_BUTTON_HEIGHT,
            DEFAULT_BUTTON_MARGIN_X + DEFAULT_BUTTON_WIDTH,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              n * DEFAULT_BUTTON_HEIGHT,
            0.0,  // red
            1.0,  // green
            0.0,  // blue
            GREEN);
  ++n;
  AddSlider(DEFAULT_BUTTON_MARGIN_X,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              (n - 1) * DEFAULT_BUTTON_HEIGHT,
            DEFAULT_BUTTON_MARGIN_X + DEFAULT_BUTTON_WIDTH,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              n * DEFAULT_BUTTON_HEIGHT,
            0.0,  // red
            0.0,  // green
            1.0,  // blue
            BLUE);
  ++n;

  /*
  AddButton(DEFAULT_BUTTON_MARGIN_X,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              (n - 1) * DEFAULT_BUTTON_HEIGHT,
            DEFAULT_BUTTON_MARGIN_X + DEFAULT_BUTTON_WIDTH,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              n * DEFAULT_BUTTON_HEIGHT,
            1.0,  // red
            1.0,  // green
            1.0,  // blue
            "",
            COLOR_BUTTON,
            WHITE);
  ++n;
  AddButton(DEFAULT_BUTTON_MARGIN_X,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              (n - 1) * DEFAULT_BUTTON_HEIGHT,
            DEFAULT_BUTTON_MARGIN_X + DEFAULT_BUTTON_WIDTH,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              n * DEFAULT_BUTTON_HEIGHT,
            0.75,  // red
            0.75,  // green
            0.75,  // blue
            "",
            COLOR_BUTTON,
            SILVER);
  ++n;
  AddButton(DEFAULT_BUTTON_MARGIN_X,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              (n - 1) * DEFAULT_BUTTON_HEIGHT,
            DEFAULT_BUTTON_MARGIN_X + DEFAULT_BUTTON_WIDTH,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              n * DEFAULT_BUTTON_HEIGHT,
            0.5,  // red
            0.5,  // green
            0.5,  // blue
            "",
            COLOR_BUTTON,
            GRAY);
  ++n;
  AddButton(DEFAULT_BUTTON_MARGIN_X,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              (n - 1) * DEFAULT_BUTTON_HEIGHT,
            DEFAULT_BUTTON_MARGIN_X + DEFAULT_BUTTON_WIDTH,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              n * DEFAULT_BUTTON_HEIGHT,
            0.0,  // red
            0.0,  // green
            0.0,  // blue
            "",
            COLOR_BUTTON,
            BLACK);
  ++n;
  AddButton(DEFAULT_BUTTON_MARGIN_X,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              (n - 1) * DEFAULT_BUTTON_HEIGHT,
            DEFAULT_BUTTON_MARGIN_X + DEFAULT_BUTTON_WIDTH,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              n * DEFAULT_BUTTON_HEIGHT,
            1.0,  // red
            0.0,  // green
            0.0,  // blue
            "",
            COLOR_BUTTON,
            RED);
  ++n;
  AddButton(DEFAULT_BUTTON_MARGIN_X,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              (n - 1) * DEFAULT_BUTTON_HEIGHT,
            DEFAULT_BUTTON_MARGIN_X + DEFAULT_BUTTON_WIDTH,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              n * DEFAULT_BUTTON_HEIGHT,
            0.5,  // red
            0.0,  // green
            0.0,  // blue
            "",
            COLOR_BUTTON,
            MAROON);
  ++n;
  AddButton(DEFAULT_BUTTON_MARGIN_X,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              (n - 1) * DEFAULT_BUTTON_HEIGHT,
            DEFAULT_BUTTON_MARGIN_X + DEFAULT_BUTTON_WIDTH,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              n * DEFAULT_BUTTON_HEIGHT,
            1.0,  // red
            1.0,  // green
            0.0,  // blue
            "",
            COLOR_BUTTON,
            YELLOW);
  ++n;
  AddButton(DEFAULT_BUTTON_MARGIN_X,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              (n - 1) * DEFAULT_BUTTON_HEIGHT,
            DEFAULT_BUTTON_MARGIN_X + DEFAULT_BUTTON_WIDTH,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              n * DEFAULT_BUTTON_HEIGHT,
            0.5,  // red
            0.5,  // green
            0.0,  // blue
            "",
            COLOR_BUTTON,
            OLIVE);
  ++n;
  AddButton(DEFAULT_BUTTON_MARGIN_X,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              (n - 1) * DEFAULT_BUTTON_HEIGHT,
            DEFAULT_BUTTON_MARGIN_X + DEFAULT_BUTTON_WIDTH,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              n * DEFAULT_BUTTON_HEIGHT,
            0.0,  // red
            1.0,  // green
            0.0,  // blue
            "",
            COLOR_BUTTON,
            LIME);
  ++n;
  AddButton(DEFAULT_BUTTON_MARGIN_X,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              (n - 1) * DEFAULT_BUTTON_HEIGHT,
            DEFAULT_BUTTON_MARGIN_X + DEFAULT_BUTTON_WIDTH,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              n * DEFAULT_BUTTON_HEIGHT,
            0.0,  // red
            0.5,  // green
            0.0,  // blue
            "",
            COLOR_BUTTON,
            GREEN);
  ++n;
  AddButton(DEFAULT_BUTTON_MARGIN_X,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              (n - 1) * DEFAULT_BUTTON_HEIGHT,
            DEFAULT_BUTTON_MARGIN_X + DEFAULT_BUTTON_WIDTH,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              n * DEFAULT_BUTTON_HEIGHT,
            0.0,  // red
            1.0,  // green
            1.0,  // blue
            "",
            COLOR_BUTTON,
            AQUA);
  ++n;
  AddButton(DEFAULT_BUTTON_MARGIN_X,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              (n - 1) * DEFAULT_BUTTON_HEIGHT,
            DEFAULT_BUTTON_MARGIN_X + DEFAULT_BUTTON_WIDTH,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              n * DEFAULT_BUTTON_HEIGHT,
            0.0,  // red
            0.5,  // green
            0.5,  // blue
            "",
            COLOR_BUTTON,
            TEAL);
  ++n;
  AddButton(DEFAULT_BUTTON_MARGIN_X,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              (n - 1) * DEFAULT_BUTTON_HEIGHT,
            DEFAULT_BUTTON_MARGIN_X + DEFAULT_BUTTON_WIDTH,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              n * DEFAULT_BUTTON_HEIGHT,
            0.0,  // red
            0.0,  // green
            1.0,  // blue
            "",
            COLOR_BUTTON,
            BLUE);
  ++n;
  AddButton(DEFAULT_BUTTON_MARGIN_X,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              (n - 1) * DEFAULT_BUTTON_HEIGHT,
            DEFAULT_BUTTON_MARGIN_X + DEFAULT_BUTTON_WIDTH,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              n * DEFAULT_BUTTON_HEIGHT,
            0.0,  // red
            0.0,  // green
            0.5,  // blue
            "",
            COLOR_BUTTON,
            NAVY);
  ++n;
  AddButton(DEFAULT_BUTTON_MARGIN_X,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              (n - 1) * DEFAULT_BUTTON_HEIGHT,
            DEFAULT_BUTTON_MARGIN_X + DEFAULT_BUTTON_WIDTH,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              n * DEFAULT_BUTTON_HEIGHT,
            1.0,  // red
            0.0,  // green
            1.0,  // blue
            "",
            COLOR_BUTTON,
            FUCHSIA);
  ++n;
  AddButton(DEFAULT_BUTTON_MARGIN_X,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              (n - 1) * DEFAULT_BUTTON_HEIGHT,
            DEFAULT_BUTTON_MARGIN_X + DEFAULT_BUTTON_WIDTH,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              n * DEFAULT_BUTTON_HEIGHT,
            0.5,  // red
            0.0,  // green
            0.5,  // blue
            "",
            COLOR_BUTTON,
            PURPLE);
  ++n;
  */

  // Additional buttons:

  ++n;
  AddButton(DEFAULT_BUTTON_MARGIN_X,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              (n - 1) * DEFAULT_BUTTON_HEIGHT,
            DEFAULT_BUTTON_MARGIN_X + DEFAULT_BUTTON_WIDTH,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              n * DEFAULT_BUTTON_HEIGHT,
            DEFAULT_BUTTON_RED,
            DEFAULT_BUTTON_GREEN,
            DEFAULT_BUTTON_BLUE,
            "Filled",
            FILL_BUTTON,
            NONE);
  ++n;
  AddButton(DEFAULT_BUTTON_MARGIN_X,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              (n - 1) * DEFAULT_BUTTON_HEIGHT,
            DEFAULT_BUTTON_MARGIN_X + DEFAULT_BUTTON_WIDTH,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              n * DEFAULT_BUTTON_HEIGHT,
            DEFAULT_BUTTON_RED,
            DEFAULT_BUTTON_GREEN,
            DEFAULT_BUTTON_BLUE,
            "Outlined",
            OUTLINE_BUTTON,
            NONE);
  ++n;
  ++n;
  AddButton(DEFAULT_BUTTON_MARGIN_X,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              (n - 1) * DEFAULT_BUTTON_HEIGHT,
            DEFAULT_BUTTON_MARGIN_X + DEFAULT_BUTTON_WIDTH,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              n * DEFAULT_BUTTON_HEIGHT,
            DEFAULT_BUTTON_RED,
            DEFAULT_BUTTON_GREEN,
            DEFAULT_BUTTON_BLUE,
            "Save",
            SAVE_BUTTON,
            NONE);
  ++n;
  AddButton(DEFAULT_BUTTON_MARGIN_X,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              (n - 1) * DEFAULT_BUTTON_HEIGHT,
            DEFAULT_BUTTON_MARGIN_X + DEFAULT_BUTTON_WIDTH,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              n * DEFAULT_BUTTON_HEIGHT,
            DEFAULT_BUTTON_RED,
            DEFAULT_BUTTON_GREEN,
            DEFAULT_BUTTON_BLUE,
            "Load",
            LOAD_BUTTON,
            NONE);
  ++n;
  AddButton(DEFAULT_BUTTON_MARGIN_X,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              (n - 1) * DEFAULT_BUTTON_HEIGHT,
            DEFAULT_BUTTON_MARGIN_X + DEFAULT_BUTTON_WIDTH,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              n * DEFAULT_BUTTON_HEIGHT,
            DEFAULT_BUTTON_RED,
            DEFAULT_BUTTON_GREEN,
            DEFAULT_BUTTON_BLUE,
            "Undo",
            UNDO_BUTTON,
            NONE);
  ++n;
  AddButton(DEFAULT_BUTTON_MARGIN_X,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              (n - 1) * DEFAULT_BUTTON_HEIGHT,
            DEFAULT_BUTTON_MARGIN_X + DEFAULT_BUTTON_WIDTH,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              n * DEFAULT_BUTTON_HEIGHT,
            DEFAULT_BUTTON_RED,
            DEFAULT_BUTTON_GREEN,
            DEFAULT_BUTTON_BLUE,
            "Clear",
            CLEAR_BUTTON,
            NONE);
  ++n;
  AddButton(DEFAULT_BUTTON_MARGIN_X,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              (n - 1) * DEFAULT_BUTTON_HEIGHT,
            DEFAULT_BUTTON_MARGIN_X + DEFAULT_BUTTON_WIDTH,
            gScreenY - n * DEFAULT_BUTTON_MARGIN_Y -
              n * DEFAULT_BUTTON_HEIGHT,
            DEFAULT_BUTTON_RED,
            DEFAULT_BUTTON_GREEN,
            DEFAULT_BUTTON_BLUE,
            "Quit",
            QUIT_BUTTON,
            NONE);
  ++n;

  // Build right-click menu:
  /*int subMenu = glutCreateMenu(colorMenu);
  glutAddMenuEntry("Red", 1);
  glutAddMenuEntry("Orange", 2);
  glutAddMenuEntry("Yellow", 3);
  glutAddMenuEntry("Green", 4);
  glutAddMenuEntry("Blue", 5);
  glutAddMenuEntry("Indigo", 6);
  glutAddMenuEntry("Violet", 7);
  glutAddMenuEntry("White", 8);
  glutAddMenuEntry("Gray", 9);
  glutAddMenuEntry("Black", 10);

  glutCreateMenu(topMenu);
  glutAddMenuEntry("Quit", 1);
  glutAddMenuEntry("Add Curve", 2);
  glutAddSubMenu("Set Color", subMenu);
  glutAttachMenu(GLUT_RIGHT_BUTTON);*/

  // Start with a Bezier curve on the canvas:
  /*gPoints.push_back(new Point2D(300, 300));
  gPoints.push_back(new Point2D(400, 400));
  gPoints.push_back(new Point2D(500, 200));
  gPoints.push_back(new Point2D(600, 300));
  gShapes.push_back(new BezierCurve(gPoints, gRed, gGreen, gBlue));
  gPoints.clear();*/

  SetShapeMode(DEFAULT_MODE);
  SetColor(DEFAULT_RED, DEFAULT_GREEN, DEFAULT_BLUE);
  SetFilled(true);
}

void SetFilled(bool b)
{
  gFilled = b;

  vector<Button *>::iterator iter;
  for (iter = gButtons.begin(); iter < gButtons.end(); ++iter)
  {
    if ((*iter)->IsButtonType(FILL_BUTTON))
    {
      if (gFilled)
        (*iter)->SetSelected(true);
      else
        (*iter)->SetSelected(false);
    }
    else if ((*iter)->IsButtonType(OUTLINE_BUTTON))
    {
      if (!gFilled)
        (*iter)->SetSelected(true);
      else
        (*iter)->SetSelected(false);
    }
  }
}

void SetColor(double r, double g, double b)
{
  gRed   = r;
  gGreen = g;
  gBlue  = b;

  vector<Button *>::iterator iter;
  for (iter = gButtons.begin(); iter < gButtons.end(); ++iter)
  {
    if ((*iter)->IsButtonType(COLOR_BUTTON))
    {
      if ((*iter)->GetRed()   == r &&
          (*iter)->GetGreen() == g &&
          (*iter)->GetBlue()  == b)
      {
        (*iter)->SetSelected(true);
      }
      else
      {
        (*iter)->SetSelected(false);
      }
    }
    else if ((*iter)->IsButtonType(RGB_SLIDER))
    {
      switch((*iter)->GetAssociatedID())
      {
      case RED:
        ((Slider *) *iter)->SetSliderLength((*iter)->GetLength() * r);
        break;
      case GREEN:
        ((Slider *) *iter)->SetSliderLength((*iter)->GetLength() * g);
        break;
      case BLUE:
        ((Slider *) *iter)->SetSliderLength((*iter)->GetLength() * b);
        break;
      default:
        break;
      }
    }
  }

  if (gSelectedShape)
  {
    gSelectedShape->SetColor(r, g, b);
  }
}

ShapeType SetShapeMode(ShapeType m)
{
  gShapeMode = m;
  gPoints.clear();

  vector<Button *>::iterator iter;
  for (iter = gButtons.begin(); iter < gButtons.end(); ++iter)
  {
    if ((*iter)->IsButtonType(MODE_BUTTON))
    {
      if ((*iter)->GetAssociatedID() == gShapeMode)
      {
        (*iter)->SetSelected(true);
      }
      else
      {
        (*iter)->SetSelected(false);
      }
    }
  }

  return gShapeMode;
}

void DeselectAllShapes()
{
  vector<Shape *>::iterator iter;
  for (iter = gShapes.begin(); iter < gShapes.end(); ++iter)
  {
    (*iter)->SetSelected(false);
  }
}

int main(int argc, char **argv)
{
  glutInit(&argc, argv);

  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize(gScreenX, gScreenY);
  glutInitWindowPosition(50, 50);

  bool fullscreen = false;
  if (fullscreen)
  {
    glutGameModeString("800x600:32");
    glutEnterGameMode();
  }
  else
  {
    glutCreateWindow("Shapes");
  }

  glutDisplayFunc(display);
  glutKeyboardFunc(keyboard);
  glutReshapeFunc(reshape);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);

  // Background color:
  glClearColor(1, 1, 1, 0);

  InitializeMyStuff();

  glutMainLoop();

  return 0;
}
