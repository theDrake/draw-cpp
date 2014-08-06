#include "Shapes.h"

//
// Point2D methods:
//

Point2D::Point2D(const double x, const double y)
{
	mX = x;
	mY = y;
}

void Point2D::Draw()
{
	glColor3d(DEFAULT_POINT_RED, DEFAULT_POINT_GREEN, DEFAULT_POINT_BLUE);
	DrawCircle(mX, mY, POINT_RADIUS);
}

bool Point2D::Contains(double x, double y) const
{
	double distance = sqrt((x - mX) * (x - mX) + (y - mY) * (y - mY));

	return distance < POINT_RADIUS;
}

//
// Shape methods:
//

Shape::Shape(vector<Point2D *> points, const double r, const double g, const double b, bool filled)
{
	vector<Point2D *>::iterator iter;
	for (iter = points.begin(); iter < points.end(); ++iter)
	{
		mVertices.push_back(*iter);
	}

	SetColor(r, g, b);
	mShapeType = NONE;
	mFilled = filled;
	mSelected = true;	// Shapes are "selected" by default when they're created.
}

Shape::~Shape()
{
	mVertices.clear();
}

void Shape::DrawPoints()
{
	if (!mSelected)
		return;

	vector<Point2D *>::iterator iter;
	for (iter = mVertices.begin(); iter < mVertices.end(); ++iter)
	{
		(*iter)->Draw();
	}
}

void Shape::Adjust(double x, double y, Point2D * selectedPoint)
{
	selectedPoint->SetX(x);
	selectedPoint->SetY(y);
}

void Shape::Move(double x, double y, Point2D * selectedPoint)
{
	double dx, dy;

	vector<Point2D *>::iterator iter;
	for (iter = mVertices.begin(); iter < mVertices.end(); ++iter)
	{
		if (*iter == selectedPoint)
			continue;

		dx = (*iter)->GetX() - selectedPoint->GetX();
		dy = (*iter)->GetY() - selectedPoint->GetY();
		(*iter)->SetX(x + dx);
		(*iter)->SetY(y + dy);
	}

	selectedPoint->SetX(x);
	selectedPoint->SetY(y);
}

void Shape::SetColor(double r, double g, double b)
{
	mRed = r;
	mGreen = g;
	mBlue = b;
}

//
// Line methods:
//

Line::Line(vector<Point2D *> points, const double r, const double g, const double b)
	: Shape(points, r, g, b, false)
{
	if (mVertices.size() != 2)
		cerr << "Error: " << mVertices.size() << " vertices passed to Line constructor." << endl;

	mShapeType = LINE;
}

void Line::Draw()
{
	glColor3d(mRed, mGreen, mBlue);
	glBegin(GL_LINES);
	glVertex2d(mVertices[0]->GetX(), mVertices[0]->GetY());
	glVertex2d(mVertices[1]->GetX(), mVertices[1]->GetY());
	glEnd();

	DrawPoints();
}

//
// BezierCurve methods:
//

BezierCurve::BezierCurve(vector<Point2D *> points, const double r, const double g, const double b)
	: Shape(points, r, g, b, false)
{
	if (mVertices.size() != 4)
		cerr << "Error: " << mVertices.size() << " vertices passed to BezierCurve constructor." << endl;

	mShapeType = BEZIER_CURVE;
}

Point2D * BezierCurve::Evaluate(double t) const
{
	double x = mVertices[0]->GetX() * (1 - t) * (1 - t) * (1 - t) +
			   3 * mVertices[1]->GetX() * (1 - t) * (1 - t) * t +
			   3 * mVertices[2]->GetX() * (1 - t) * t * t + 
			   mVertices[3]->GetX() * t * t * t;
	double y = mVertices[0]->GetY() * (1 - t) * (1 - t) * (1 - t) +
			   3 * mVertices[1]->GetY() * (1 - t) * (1 - t) * t +
			   3 * mVertices[2]->GetY() * (1 - t) * t * t + 
			   mVertices[3]->GetY() * t * t * t;

	return new Point2D(x, y);
}

void BezierCurve::Draw()
{
	Point2D * p1, * p2;
	for (int i = 0; i < CURVE_RESOLUTION; ++i)
	{
		p1 = Evaluate((double) i / CURVE_RESOLUTION);
		p2 = Evaluate((double) (i + 1) / CURVE_RESOLUTION);

		glColor3d(mRed, mGreen, mBlue);
		glBegin(GL_LINES);
		glVertex2d(p1->GetX(), p1->GetY());
		glVertex2d(p2->GetX(), p2->GetY());
		glEnd();

		delete p1, p2;
	}
	DrawPoints();
}

//
// Rectangle methods:
//

Rectangle::Rectangle(vector<Point2D *> points, const double r, const double g, const double b, bool filled)
	: Shape(points, r, g, b, filled)
{
	if (mVertices.size() != 2 && mVertices.size() != 4)
		cerr << "Error: " << mVertices.size() << " vertices passed to Rectangle constructor." << endl;

	// Add the other two corner points, if necessary, to allow resizing:
	if (mVertices.size() == 2)
	{
		mVertices.push_back(new Point2D(mVertices[0]->GetX(), mVertices[1]->GetY()));
		mVertices.push_back(new Point2D(mVertices[1]->GetX(), mVertices[0]->GetY()));
	}

	mLeft   = mVertices[0]->GetX() < mVertices[1]->GetX() ? mVertices[0]->GetX() : mVertices[1]->GetX();
	mRight  = mVertices[0]->GetX() > mVertices[1]->GetX() ? mVertices[0]->GetX() : mVertices[1]->GetX();
	mTop    = mVertices[0]->GetY() > mVertices[1]->GetY() ? mVertices[0]->GetY() : mVertices[1]->GetY();
	mBottom = mVertices[0]->GetY() < mVertices[1]->GetY() ? mVertices[0]->GetY() : mVertices[1]->GetY();

	mShapeType = RECTANGLE;
}

void Rectangle::Draw()
{
	if (mFilled)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	glColor3d(mRed, mGreen, mBlue);
	DrawRectangle(mLeft, mTop, mRight, mBottom);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	DrawPoints();
}

void Rectangle::Move(double x, double y, Point2D * selectedPoint)
{
	Shape::Move(x, y, selectedPoint);
	
	mLeft   = mVertices[0]->GetX() < mVertices[1]->GetX() ? mVertices[0]->GetX() : mVertices[1]->GetX();
	mRight  = mVertices[0]->GetX() > mVertices[1]->GetX() ? mVertices[0]->GetX() : mVertices[1]->GetX();
	mTop    = mVertices[0]->GetY() > mVertices[1]->GetY() ? mVertices[0]->GetY() : mVertices[1]->GetY();
	mBottom = mVertices[0]->GetY() < mVertices[1]->GetY() ? mVertices[0]->GetY() : mVertices[1]->GetY();
}

void Rectangle::Adjust(double x, double y, Point2D * selectedPoint)
{
	vector<Point2D *>::iterator iter;
	for (iter = mVertices.begin(); iter < mVertices.end(); ++iter)
	{
		if ((*iter) != selectedPoint)
		{
			if ((*iter)->GetX() == selectedPoint->GetX())
			{
				(*iter)->SetX(x);
			}
			if ((*iter)->GetY() == selectedPoint->GetY())
			{
				(*iter)->SetY(y);
			}
		}
	}
	selectedPoint->SetX(x);
	selectedPoint->SetY(y);

	mLeft   = mVertices[0]->GetX() < mVertices[1]->GetX() ? mVertices[0]->GetX() : mVertices[1]->GetX();
	mRight  = mVertices[0]->GetX() > mVertices[1]->GetX() ? mVertices[0]->GetX() : mVertices[1]->GetX();
	mTop    = mVertices[0]->GetY() > mVertices[1]->GetY() ? mVertices[0]->GetY() : mVertices[1]->GetY();
	mBottom = mVertices[0]->GetY() < mVertices[1]->GetY() ? mVertices[0]->GetY() : mVertices[1]->GetY();
}

bool Rectangle::Contains(double x, double y) const
{
	return x > mLeft && x < mRight && y < mTop && y > mBottom;
}

//
// Triangle methods:
//

Triangle::Triangle(vector<Point2D *> points, const double r, const double g, const double b, bool filled)
	: Shape(points, r, g, b, filled)
{
	if (mVertices.size() != 3)
		cerr << "Error: " << mVertices.size() << " vertices passed to Triangle constructor." << endl;

	mShapeType = TRIANGLE;
}

void Triangle::Draw()
{
	if (mFilled)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	glColor3d(mRed, mGreen, mBlue);
	DrawTriangle(mVertices[0]->GetX(), mVertices[0]->GetY(),
				 mVertices[1]->GetX(), mVertices[1]->GetY(),
				 mVertices[2]->GetX(), mVertices[2]->GetY());
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	DrawPoints();
}

//
// Pentagon methods:
//

Pentagon::Pentagon(vector<Point2D *> points, const double r, const double g, const double b, bool filled)
	: Shape(points, r, g, b, filled)
{
	if (mVertices.size() != 5)
		cerr << "Error: " << mVertices.size() << " vertices passed to Pentagon constructor." << endl;

	mShapeType = PENTAGON;
}

void Pentagon::Draw()
{
	if (mFilled)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	glColor3d(mRed, mGreen, mBlue);
	glBegin(GL_POLYGON);
	glVertex2d(mVertices[0]->GetX(), mVertices[0]->GetY());
	glVertex2d(mVertices[1]->GetX(), mVertices[1]->GetY());
	glVertex2d(mVertices[2]->GetX(), mVertices[2]->GetY());
	glVertex2d(mVertices[3]->GetX(), mVertices[3]->GetY());
	glVertex2d(mVertices[4]->GetX(), mVertices[4]->GetY());
	glEnd();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	DrawPoints();
}

//
// Circle methods:
//

Circle::Circle(vector<Point2D *> points, double r, double g, double b, bool filled)
	: Shape(points, r, g, b, filled)
{
	if (mVertices.size() != 2)
	{
		cerr << "Error: " << mVertices.size() << " vertices passed to Circle constructor." << endl;
	}
	else
	{
		// Use the second vertex to determine the radius:
		mRadius = sqrt((mVertices[0]->GetX() - mVertices[1]->GetX()) * (mVertices[0]->GetX() - mVertices[1]->GetX()) +
					   (mVertices[0]->GetY() - mVertices[1]->GetY()) * (mVertices[0]->GetY() - mVertices[1]->GetY()));
	}

	mShapeType = CIRCLE;
}

void Circle::Draw()
{
	if (mFilled)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	glColor3d(mRed, mGreen, mBlue);
	DrawCircle(mVertices[0]->GetX(), mVertices[0]->GetY(), mRadius);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	DrawPoints();
}

void Circle::Adjust(double x, double y, Point2D * selectedPoint)
{
	Shape::Adjust(x, y, selectedPoint);

	mRadius = sqrt((mVertices[0]->GetX() - mVertices[1]->GetX()) * (mVertices[0]->GetX() - mVertices[1]->GetX()) +
		           (mVertices[0]->GetY() - mVertices[1]->GetY()) * (mVertices[0]->GetY() - mVertices[1]->GetY()));
}

//
// Button methods:
//

Button::Button(vector<Point2D *> points,
			   const double r,
			   const double g,
			   const double b,
			   const char * text,
			   const int buttonType,
			   const int associatedID)
	: Rectangle (points, r, g, b, true)
{
	SetText(text);
	mButtonType = buttonType;
	mAssociatedID = associatedID;
	mPressed = false;
	mSelected = false;
}

void Button::Draw()
{
	// Draw a black outline if the button is currently selected:
	if (mSelected)
	{
		glColor3d(0, 0, 0);
		double x1 = mVertices[0]->GetX() < mVertices[1]->GetX() ? mVertices[0]->GetX() : mVertices[1]->GetX();
		double x2 = mVertices[0]->GetX() > mVertices[1]->GetX() ? mVertices[0]->GetX() : mVertices[1]->GetX();
		double y1 = mVertices[0]->GetY() > mVertices[1]->GetY() ? mVertices[0]->GetY() : mVertices[1]->GetY();
		double y2 = mVertices[0]->GetY() < mVertices[1]->GetY() ? mVertices[0]->GetY() : mVertices[1]->GetY();
		DrawRectangle(x1 - BUTTON_OUTLINE_THICKNESS, y1 + BUTTON_OUTLINE_THICKNESS,
					  x2 + BUTTON_OUTLINE_THICKNESS, y2 - BUTTON_OUTLINE_THICKNESS);
	}

	// Draw the button (using alternative RGB values if currently being pressed/clicked):
	if (mPressed)
		glColor3d(mRed - 0.5, mGreen - 0.5, mBlue - 0.5);
	else
		glColor3d(mRed, mGreen, mBlue);
	DrawRectangle(mVertices[0]->GetX(), mVertices[0]->GetY(),
				  mVertices[1]->GetX(), mVertices[1]->GetY());

	// Draw the button's text, if any (using alternative RGB values if currently being pressed/clicked):
	if (mPressed)
		glColor3d(1.0, 1.0, 1.0);
	else
		glColor3d(0, 0, 0);
	DrawText((mVertices[0]->GetX() < mVertices[1]->GetX() ? mVertices[0]->GetX() : mVertices[1]->GetX()) + BUTTON_TEXT_OFFSET_X,
			 (mVertices[0]->GetY() > mVertices[1]->GetY() ? mVertices[0]->GetY() : mVertices[1]->GetY()) - BUTTON_TEXT_OFFSET_Y,
			 mText);
}

const char * Button::SetText(const char * text)
{
	strncpy(mText, text, BUTTON_TEXT_MAX_LEN);
	mText[BUTTON_TEXT_MAX_LEN] = '\0';

	return mText;
}

//
// Slider methods:
//

Slider::Slider(vector<Point2D *> points,
			   const double r,
			   const double g,
			   const double b,
			   const int associatedID)
	: Button (points, r, g, b, "", RGB_SLIDER, associatedID)
{
	mSliderLength = 0.0;
}

void Slider::Draw()
{
	// Draw a black background rectangle:
	glColor3d(0, 0, 0);
	DrawRectangle(mVertices[0]->GetX(), mVertices[0]->GetY(),
				  mVertices[1]->GetX(), mVertices[1]->GetY());

	// Draw the slider rectangle:
	glColor3d(mRed, mGreen, mBlue);
	DrawRectangle(mVertices[0]->GetX(), mVertices[0]->GetY(),
				  mSliderLength + mLeft, mVertices[1]->GetY());
}

//
// Label methods:
//

Label::Label(vector<Point2D *> points,
			 const double r,
			 const double g,
			 const double b,
			 const char * text)
	: Button (points, r, g, b, text, LABEL, NONE)
{}

void Label::Draw()
{
	// Draw the background rectangle:
	glColor3d(mRed, mGreen, mBlue);
	DrawRectangle(mVertices[0]->GetX(), mVertices[0]->GetY(),
				  mVertices[1]->GetX(), mVertices[1]->GetY());

	// Draw the label's text:
	glColor3d(0, 0, 0);
	DrawText((mVertices[0]->GetX() < mVertices[1]->GetX() ? mVertices[0]->GetX() : mVertices[1]->GetX()) + BUTTON_TEXT_OFFSET_X,
			 (mVertices[0]->GetY() > mVertices[1]->GetY() ? mVertices[0]->GetY() : mVertices[1]->GetY()) - BUTTON_TEXT_OFFSET_Y,
			 mText);
}
