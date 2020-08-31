#include <iostream>
#include <cmath>
#include <vector>
#include <sstream>
#include <fstream>
#include <string>
#include <GL/glut.h>

using namespace std;

typedef struct GeomObject GeomObject;
typedef struct Point Point;
typedef struct Face Face;

GLuint window;
GLuint width = 500, height = 500;

const int ENLARGE_FACTOR = width / 2;

int xMin, xMax, yMin, yMax, zMin, zMax;

void display(void);
void reshape(int, int);
void determineMinMaxXYZ(int, int, int);
void calculateFace(int, int, int);
void extractData(ifstream);
void transformBody(void);
void drawBody();

int getNumberOfVertices(GeomObject);

bool testPoint(double, double, double);

struct Point
{
    double x;
    double y;
    double z;
};

struct Face
{
    int v1;
    int v2;
    int v3;
    double A;
    double B;
    double C;
    double D;
};

struct GeomObject
{
    vector<Point> vertices;
    vector<Face> faces;
} object;

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(width, height);
    glutInitWindowPosition(100, 100);
    window = glutCreateWindow("Objects");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);

    printf("Object Drawing\n");
    printf("---------------------------------------\n\n");

    ifstream objFile;
    objFile.open("tetrahedron.obj");

    string line;
    while (getline(objFile, line))
    {
        stringstream ss(line);
        string lineToken;
        while (getline(ss, lineToken, ' '))
        {
            // parse a vertex
            if (lineToken == "v")
            {
                getline(ss, lineToken, ' ');
                double x = stod(lineToken);

                getline(ss, lineToken, ' ');
                double y = stod(lineToken);

                getline(ss, lineToken, ' ');
                double z = stod(lineToken);

                Point vertex;
                vertex.x = x;
                vertex.y = y;
                vertex.z = z;

                // update min and max values for all coordinates after parsing a new vertex
                if (getNumberOfVertices(object) == 1)
                {
                    xMin = x; xMax = x;
                    yMin = y; yMax = y;
                    zMin = z; zMax = z;
                }
                else
                {
                    determineMinMaxXYZ(x, y, z);
                }

                object.vertices.push_back(vertex);
            }
            else if (lineToken == "f") // parse a face
            {
                getline(ss, lineToken, ' ');
                int v1 = stoi(lineToken);

                getline(ss, lineToken, ' ');
                int v2 = stoi(lineToken);

                getline(ss, lineToken, ' ');
                int v3 = stoi(lineToken);

                Face face;
                face.v1 = v1;
                face.v2 = v2;
                face.v3 = v3;

                // calculate plane parameters on which the face lies
                calculateFace(v1, v2, v3);
            }
        }
    }
    objFile.close();

    transformBody();

    double testX, testY, testZ;
    cout << "Set test point X (-1.0 - 1.0): ";
    cin >> testX;

    cout << "Set test point Y (-1.0 - 1.0): ";
    cin >> testY;

    cout << "Set test point Z (-1.0 - 1.0): ";
    cin >> testZ;

    if (testPoint(testX, testY, testZ)) {
        cout << "The point is inside the object!\n";
    } else {
        cout << "The point is outside the object!\n";
    }

    
    glutMainLoop();
    return 0;
}

/**
 * Draws the parsed object in 2D XY plane.
 * */
void drawBody() {
    reshape(width, height);
    glColor3f(0.0f, 0.0f, 0.0f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBegin(GL_POLYGON);
    for (int i = 0; i < getNumberOfVertices(object); i++) {
        Point vertex = object.vertices[i];
        int x = round(vertex.x * ENLARGE_FACTOR + width / 2.);
        int y = round(vertex.y * ENLARGE_FACTOR + height / 2.);
        glVertex2i(x, y);
    }
    glEnd();
}

/**
 * Transforms the body in two ways:
 *  1. scales all coordinates to be between -1.0 and 1.0
 *  2. transforms all coordinates so that the center is at (0, 0)
 * */
void transformBody(void) {
    Point center;
    int numOfVertices = getNumberOfVertices(object);
    double centerX = 0, centerY = 0, centerZ = 0;
    for (int i = 0; i < numOfVertices; i++) {
        centerX += object.vertices[i].x;
        centerY += object.vertices[i].y;
        centerZ += object.vertices[i].z;
    }

    center.x = centerX / numOfVertices;
    center.y = centerY / numOfVertices;
    center.z = centerZ / numOfVertices;

    double xScaleFactor = (abs(xMin) > abs(xMax)) ? abs(xMin) : abs(xMax);
    double yScaleFactor = (abs(yMin) > abs(yMax)) ? abs(yMin) : abs(yMax);
    double zScaleFactor = (abs(zMin) > abs(zMax)) ? abs(zMin) : abs(zMax);

    double scaleFactor;
    if (xScaleFactor > yScaleFactor && xScaleFactor > zScaleFactor) scaleFactor = xScaleFactor;
    else if (yScaleFactor > xScaleFactor && yScaleFactor > zScaleFactor) scaleFactor = yScaleFactor;
    else scaleFactor = zScaleFactor;

    center.x /= scaleFactor;
    center.y /= scaleFactor;
    center.z /= scaleFactor;

    for (int i = 0; i < numOfVertices; i++) {
        object.vertices[i].x /= scaleFactor;
        object.vertices[i].x -= center.x;

        object.vertices[i].y /= scaleFactor;
        object.vertices[i].y -= center.y;

        object.vertices[i].z /= scaleFactor;
        object.vertices[i].z -= center.z;
    }
}

/**
 * Calculates the given face's plane parameters.
 * */
void calculateFace(int v1, int v2, int v3) {
    Face face;
    Point p1 = object.vertices[v1 - 1];
    Point p2 = object.vertices[v2 - 1];
    Point p3 = object.vertices[v3 - 1];

    face.A = (p2.y - p1.y) * (p3.z - p1.z) - (p2.z - p1.z) * (p3.y - p1.y);
    face.B = -(p2.x - p1.x) * (p3.z - p1.z) + (p2.z - p1.z) * (p3.x - p1.x);
    face.C = (p2.x - p1.x) * (p3.y - p1.y) - (p2.y - p1.y) * (p3.x - p1.x);
    face.D = -p1.x * face.A - p1.y * face.B - p1.z * face.C;

    object.faces.push_back(face);
}

/**
 * Tests if the given point is inside the object.
 * */
bool testPoint(double x, double y, double z)
{
    for (int i = 0; i < object.faces.size(); i++) {
        Face face = object.faces[i];

        double scalarProduct = x * face.A + y * face.B + z * face.C + face.D;
        if (scalarProduct >= 0) return false;
    }

    return true;
}

/**
 * Updates the min and max values for all coordinates upon parsing a new vertex.
 * */
void determineMinMaxXYZ(int x, int y, int z)
{
    if (x < xMin)
        xMin = x;
    else if (x > xMax)
        xMax = x;

    if (y < yMin)
        yMin = y;
    else if (y > yMax)
        yMax = y;

    if (z < zMin)
        zMin = z;
    else if (z > zMax)
        zMax = z;
}

/**
 * Returns the number of vertices of the given object.
 * */
int getNumberOfVertices(GeomObject object)
{
    return object.vertices.size();
}

void display(void)
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    drawBody();
    glFlush();
}

/**
 * This function is called every time the window is resized.
 * */
void reshape(int w, int h)
{
    width = w;
    height = h;

    glDisable(GL_DEPTH_TEST);
    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, width - 1, 0, height - 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glPointSize(1.0);
    glColor3f(0.0f, 0.0f, 0.0f);
}
