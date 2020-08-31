#include <stdio.h>
#include <GL/glut.h>

GLdouble Lx[2], Ly[2];
GLint pointIndex;

GLuint window;
GLuint width = 500, height = 500;

void display(void);
void reshape(int, int);
void drawLines(int, int, int, int);
void mousePressedOrReleased(int, int, int, int);
void bresenhamDraw(int, int, int, int);
void bresenhamDraw0To90(int, int, int, int);
void bresenhamDraw0ToNeg90(int, int, int, int);

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(width, height);
    glutInitWindowPosition(100, 100);
    window = glutCreateWindow("Bresenham vs. LINE");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mousePressedOrReleased);

    printf("Bresenham algorithm\n");
    printf("-----------------------------\n\n");
    printf("Set line start/end coordinates using the left mouse button.\n\n");

    glutMainLoop();
    return 0;
}

void display(void)
{
    // glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glFlush();
}

void reshape(int w, int h)
{
    width = w;
    height = h;
    pointIndex = 0;

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, width, 0, height);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glPointSize(1.0);
    glColor3f(0.0f, 0.0f, 0.0f);
}

void mousePressedOrReleased(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        Lx[pointIndex] = x;
        Ly[pointIndex] = height - y;
        pointIndex ^= 1;

        if (pointIndex == 0)
        {
            bresenhamDraw((int)Lx[0], (int)Ly[0], (int)Lx[1], (int)Ly[1]);
        }
        else
        {
            glVertex2i(x, height - y);
        }

        glFlush();
    }
    else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
    {
        reshape(width, height);
    }
}

void bresenhamDraw(int startX, int startY, int endX, int endY)
{
    glBegin(GL_LINES);
    {
        glVertex2i(startX, startY + 20);
        glVertex2i(endX, endY + 20);
    }
    glEnd();

    if (startX <= endX)
    {
        if (startY <= endY)
        {
            bresenhamDraw0To90(startX, startY, endX, endY);
        }
        else
        {
            bresenhamDraw0ToNeg90(startX, startY, endX, endY);
        }
    }
    else
    {
        if (startY >= endY)
        {
            bresenhamDraw0To90(endX, endY, startX, startY);
        }
        else
        {
            bresenhamDraw0ToNeg90(endX, endY, startX, startY);
        }
    }
}

void bresenhamDraw0To90(int startX, int startY, int endX, int endY)
{
    GLint x, yc, correction;
    GLint a, yf;

    if (endY - startY <= endX - startX)
    {
        a = 2 * (endY - startY);
        yc = startY;
        yf = -(endX - startX);
        correction = -2 * (endX - startX);

        glBegin(GL_POINTS);
        for (x = startX; x <= endX; x++)
        {
            glVertex2i(x, yc);
            yf += a;

            if (yf >= 0)
            {
                yf += correction;
                yc++;
            }
        }
        glEnd();
    }
    else
    {
        x = endX;
        endX = endY;
        endY = x;
        x = startX;
        startX = startY;
        startY = x;
        a = 2 * (endY - startY);

        yc = startY;
        yf = -(endX - startX);
        correction = -2 * (endX - startX);

        glBegin(GL_POINTS);
        for (x = startX; x <= endX; x++)
        {
            glVertex2i(yc, x);
            yf += a;

            if (yf >= 0)
            {
                yf += correction;
                yc++;
            }
        }
        glEnd();
    }
}

void bresenhamDraw0ToNeg90(int startX, int startY, int endX, int endY)
{
    GLint x, yc, correction;
    GLint a, yf;

    if (-(endY - startY) <= endX - startX)
    {
        a = 2 * (endY - startY);
        yc = startY;
        yf = endX - startX;
        correction = 2 * (endX - startX);

        glBegin(GL_POINTS);
        for (x = startX; x <= endX; x++)
        {
            glVertex2i(x, yc);
            yf += a;

            if (yf <= 0)
            {
                yf += correction;
                yc--;
            }
        }
        glEnd();
    }
    else
    {
        x = endX;
        endX = startY;
        startY = x;
        x = startX;
        startX = endY;
        endY = x;
        a = 2 * (endY - startY);

        yc = startY;
        yf = endX - startX;
        correction = 2 * (endX - startX);

        glBegin(GL_POINTS);
        for (x = startX; x <= endX; x++)
        {
            glVertex2i(yc, x);
            yf += a;

            if (yf <= 0)
            {
                yf += correction;
                yc--;
            }
        }
        glEnd();
    }
}
