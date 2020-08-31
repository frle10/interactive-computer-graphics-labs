#include <cstdio>
#include <vector>
#include <cmath>
#include <GL/glut.h>

using namespace std;

typedef struct Point Point;
typedef struct Edge Edge;
typedef struct PolyElement PolyElement;
typedef struct Poly Poly;

GLuint window;
GLuint width = 500, height = 500;

int xMin, xMax, yMin, yMax;

void display(void);
void reshape(int, int);
void mousePressedOrReleased(int, int, int, int);
void enterClicked(unsigned char, int, int);

void drawPolygon(void);
void calculateEdges(void);
void fillPolygon(void);
void determineMinMaxXY(int, int);
int getNumberOfVertices(Poly);
bool testPoint(int, int);

struct Point
{
    int x;
    int y;
};

struct Edge
{
    int a;
    int b;
    int c;
};

struct PolyElement
{
    Point vertex;
    Edge edge;
    bool left;
};

struct Poly
{
    vector<PolyElement> polygonElements;
} polygon;

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(width, height);
    glutInitWindowPosition(100, 100);
    window = glutCreateWindow("Polygon");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mousePressedOrReleased);
    glutKeyboardFunc(enterClicked);

    printf("Polygon drawing and filling tool\n");
    printf("---------------------------------------\n\n");
    printf("Use the left mouse button to set polygon vertices and click Enter to draw the polygon.\n\n");

    glutMainLoop();
    return 0;
}

/**
 * Tests if the given point is inside the polygon.
 * */
bool testPoint(int x, int y)
{
    for (int i = 0; i < getNumberOfVertices(polygon); i++) {
        Edge edge = polygon.polygonElements[i].edge;
        int value = x * edge.a + y * edge.b + edge.c;

        if (value > 0) return false;
    }

    return true;
}

/**
 * Handles keyboard events. Upon Enter key press, polygon drawing
 * and edge calculation is attempted.
 * If less than thre vertices have been defined, the method does nothing.
 * Also, the user is prompted for test point coordinate input, to test if
 * the given point is inside or outside the polygon.
 * */
void enterClicked(unsigned char key, int x, int y)
{
    // if the Enter key is pressed (ASCII decimal value for Enter is 13)
    if (key == (char)13)
    {
        if (getNumberOfVertices(polygon) < 3)
            return; // not enough vertices have been set for drawing a polygon

        drawPolygon();
        calculateEdges();

        char more = 'Y';
        while (more == 'Y')
        {
            int testX, testY;
            printf("Enter test point X: ");
            scanf("%d", &testX);
            printf("Enter test point Y: ");
            scanf("%d", &testY);

            if (testPoint(testX, testY)) printf("\nThe point is inside the polygon!\n\n");
            else printf("\nThe point is outside the polygon!\n\n");

            printf("Test another point (Y/N)? ");
            scanf(" %c", &more);
            printf("\n");
        }

        fillPolygon();

        polygon.polygonElements.clear();
    }
}

/**
 * Draws the polygon.
 * */
void drawPolygon()
{
    printf("\nDrawing polygon...\n");

    glColor3f(0.0f, 0.0f, 0.0f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBegin(GL_POLYGON);
    for (int i = 0; i < getNumberOfVertices(polygon); i++)
    {
        Point point = polygon.polygonElements[i].vertex;
        glVertex2i(point.x, point.y);
    }
    glEnd();
    glFlush();

    printf("Drew polygon!\n");
}

/**
 * Calculates edge line parameters for all edges of the polygon.
 * */
void calculateEdges(void)
{
    int i, i0;
    i0 = getNumberOfVertices(polygon) - 1;

    for (i = 0; i < getNumberOfVertices(polygon); i0 = i++)
    {
        Point startPoint = polygon.polygonElements[i0].vertex;
        Point endPoint = polygon.polygonElements[i].vertex;

        Edge edge;
        edge.a = startPoint.y - endPoint.y;
        edge.b = -startPoint.x + endPoint.x;
        edge.c = startPoint.x * endPoint.y - endPoint.x * startPoint.y;

        polygon.polygonElements[i0].edge = edge;
        polygon.polygonElements[i0].left = startPoint.y < endPoint.y;
    }
}

/**
 * Fills the polygon line by line using the method from
 * the book.
 * */
void fillPolygon()
{
    printf("\nFilling polygon!\n");

    // book algorithm for filling the polygon line by line
    for (int i = yMin; i <= yMax; i++)
    {
        double L = xMin, D = xMax;

        for (int j = 0; j < getNumberOfVertices(polygon); j++)
        {
            PolyElement pe = polygon.polygonElements[j];
            Edge edge = pe.edge;

            if (!edge.a == 0)
            {
                double intersectionX = (-edge.b * i - edge.c) / (double)edge.a;

                if (pe.left && intersectionX > L)
                {
                    L = intersectionX;
                }
                else if (!pe.left && intersectionX < D)
                {
                    D = intersectionX;
                }
            }
        }

        if (L < D)
        {
            glColor3f(0.0f, 0.0f, 1.0f);
            glBegin(GL_LINES);
            {
                glVertex2i(round(L), i);
                glVertex2i(round(D), i);
            }
            glEnd();
            glFlush();
        }
    }

    printf("Filled polygon!\n");
}

/**
 * This function returns the number of vertices in the given polygon.
 * */
int getNumberOfVertices(Poly polygon)
{
    return polygon.polygonElements.size();
}

/**
 * This function determines if the passed x and y parameters are
 * lowest or highest seen by now.
 * */
void determineMinMaxXY(int x, int y)
{
    if (x < xMin)
        xMin = x;
    else if (x > xMax)
        xMax = x;

    if (y < yMin)
        yMin = y;
    else if (y > yMax)
        yMax = y;
}

/**
 * This function handles the mouse events. On left click a new
 * vertex is generated for the polygon and on right click the
 * polygon is cleared from memory.
 * */
void mousePressedOrReleased(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        Point point;
        point.x = x;
        point.y = height - y;

        PolyElement pe;
        pe.vertex = point;
        polygon.polygonElements.push_back(pe);

        printf("Added vertex: (%d, %d)\n", point.x, point.y);

        if (getNumberOfVertices(polygon) == 1)
        {
            xMin = point.x;
            xMax = point.x;
            yMin = point.y;
            yMax = point.y;
        }
        else
        {
            determineMinMaxXY(point.x, point.y);
        }
    }
    else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
    {
        // clear the polygon and the screen on next update
        polygon.polygonElements.clear();
        reshape(width, height);
        printf("Cleared display!\n\n");
    }
}

void display(void)
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

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
