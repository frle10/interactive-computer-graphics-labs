#include <iostream>
#include <cmath>
#include <vector>
#include <sstream>
#include <fstream>
#include <string>
#include <GL/glut.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

using namespace std;

typedef struct GeomObject GeomObject;
typedef struct Face Face;

GLuint window;
GLuint width = 500, height = 500;

void display(void);
void reshape(int, int);
void drawControlAndBezier();
void drawObject();
void calculateBezier();
void updateTransformationMatrix();
void doAnimation(int);
void computeFactors(int, int*);
void keyPressed(unsigned char, int, int);

glm::vec4 getFaceNormal(Face);
glm::vec4 getObjectCenter(vector<glm::vec4>);
glm::vec4 getPointByIndex(int);
glm::vec2 getProcessedPoint(glm::vec4);

glm::mat4 transformationMatrix;
glm::vec4 viewpoint;
glm::vec4 view;
glm::vec4 bodyCenter;

bool animate = false;
double H;
int divs = 100;
int currentViewpointIndex = 0;
vector<glm::vec4> controlVertices;
vector<glm::vec4> bezierLinePoints;

struct Face
{
    int v1;
    int v2;
    int v3;
};

struct GeomObject
{
    vector<glm::vec4> vertices;
    vector<Face> faces;
} object;

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE);
    glutInitWindowSize(width, height);
    glutInitWindowPosition(100, 100);
    window = glutCreateWindow("Bezier");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyPressed);

    printf("Bezier\n");
    printf("---------------------------------------\n\n");

    ifstream objFile;
    objFile.open("../objects/convex/kocka.obj");

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

                double vertexArr[4] = {x, y, z, 1};
                glm::vec4 vertex = glm::make_vec4(vertexArr);

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
                object.faces.push_back(face);
            }
        }
    }
    objFile.close();

    ifstream controlPolygonFile;
    objFile.open("control_polygon.obj");
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

                double vertexArr[4] = {x, y, z, 1};
                glm::vec4 vertex = glm::make_vec4(vertexArr);

                controlVertices.push_back(vertex);
            }
        }
    }
    controlPolygonFile.close();

    calculateBezier();
    updateTransformationMatrix();
    
    view = getObjectCenter(object.vertices);
    viewpoint = bezierLinePoints[0];

    glutMainLoop();
    return 0;
}

glm::vec4 getPointByIndex(int index) {
  return object.vertices[index];
}

glm::vec4 getObjectCenter(vector<glm::vec4> points) {
  double centerX = 0, centerY = 0, centerZ = 0;
  int n = points.size();
  for (int i = 0; i < n; i++) {
    centerX += points[i][0];
    centerY += points[i][1];
    centerZ += points[i][2];
  }

  centerX /= n;
  centerY /= n;
  centerZ /= n;

  double centerArr[4] = {centerX, centerY, centerZ, 1};
  return glm::make_vec4(centerArr);
}

glm::vec4 getFaceNormal(Face face) {
    glm::vec4 p1 = getPointByIndex(face.v1 - 1);
    glm::vec4 p2 = getPointByIndex(face.v2 - 1);
    glm::vec4 p3 = getPointByIndex(face.v3 - 1);

    double A = (p2[1] - p1[1]) * (p3[2] - p1[2]) - (p2[2] - p1[2]) * (p3[1] - p1[1]);
    double B = -(p2[0] - p1[0]) * (p3[2] - p1[2]) + (p2[2] - p1[2]) * (p3[0] - p1[0]);
    double C = (p2[0] - p1[0]) * (p3[1] - p1[1]) - (p2[1] - p1[1]) * (p3[0] - p1[0]);

    double normalArr[4] = {A, B, C, 0};
    return glm::make_vec4(normalArr);
}

void computeFactors(int n, int *factors) {
  int i, a = 1;

  for (i = 1; i <= n + 1; i++) {
    factors[i - 1] = a;
    a = a * (n - i + 1) / i;
  }
}

void calculateBezier() {
  int n = controlVertices.size() - 1;
  int *factors = (int*)malloc(sizeof(int) * controlVertices.size());
  double t, b;

  computeFactors(n, factors);
  for (int i = 0; i <= divs; i++) {
    t = 1.0 / divs * i;
    double x = 0, y = 0, z = 0;

    for (int j = 0; j <= n; j++) {
      if (j == 0) b = factors[j] * pow(1 - t, n);
      else if (j == n) b = factors[j] * pow(t, n);
      else b = factors[j] * pow(t, j) * pow(1 - t, n - j);

      x += b * controlVertices[j][0];
      y += b * controlVertices[j][1];
      z += b * controlVertices[j][2];
    }

    double pointArr[4] = {x, y, z, 1};
    bezierLinePoints.push_back(glm::make_vec4(pointArr));
  }

  free(factors);
}

void doAnimation(int value) {
  if (currentViewpointIndex >= bezierLinePoints.size()) currentViewpointIndex = 0;
  viewpoint = bezierLinePoints[currentViewpointIndex++];
  updateTransformationMatrix();
  glutPostRedisplay();
}

void drawControlAndBezier() {
  glPushMatrix();
  glTranslatef(width / 2, height / 2, 0);
  glScalef(50.0f, 50.0f, 50.0f);

  glBegin(GL_LINE_STRIP);
  for (int i = 0; i < controlVertices.size(); i++) {
    glm::vec4 vertex = controlVertices[i];
    glVertex2d(vertex[0], vertex[1]);
  }
  glEnd();

  glBegin(GL_LINE_STRIP);
  for (int i = 0; i < bezierLinePoints.size(); i++) {
    glm::vec4 vertex = bezierLinePoints[i];
    glVertex2d(vertex[0], vertex[1]);
  }
  glEnd();

  glPopMatrix();
}

void keyPressed(unsigned char key, int x, int y) {
  if (key == (char)13) {
    animate = true;
    glutPostRedisplay();
  }
}

void updateTransformationMatrix() {
  // Make the T1 matrix (translation of the viewpoint to scene origin)
  double t1Array[16] = {1, 0, 0, -viewpoint[0], 0, 1, 0, -viewpoint[1], 0, 0, 1, -viewpoint[2], 0, 0, 0, 1};
  transformationMatrix = glm::make_mat4(t1Array);

  // Get point G1 from instructions
  glm::vec4 view1 = view * transformationMatrix;

  // Fetch Xg1 and Yg1 for sin(a) and cos(a) from instructions for the next matrix
  double view1X = view1[0];
  double view1Y = view1[1];

  double sinAlpha = view1Y / sqrt(view1X * view1X + view1Y * view1Y);
  double cosAlpha = view1X / sqrt(view1X * view1X + view1Y * view1Y);

  // Calculate matrix T2, get G2 and update the transformation matrix
  double t2Array[16] = {cosAlpha, sinAlpha, 0, 0, -sinAlpha, cosAlpha, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
  glm::mat4 t2 = glm::make_mat4(t2Array);
  glm::vec4 view2 = view1 * t2;
  transformationMatrix *= t2;

  // Fetch Xg1 and Zg1 for sin(b) and cos(b) from instructions for the next matrix
  double view2X = view2[0];
  double view2Z = view2[2];

  double sinBeta = view2X / sqrt(view2X * view2X + view2Z * view2Z);
  double cosBeta = view2Z / sqrt(view2X * view2X + view2Z * view2Z);

  // Calculate matrix T3, get G3 and update the transformation matrix
  double t3Array[16] = {cosBeta, 0, -sinBeta, 0, 0, 1, 0, 0, sinBeta, 0, cosBeta, 0, 0, 0, 0, 1};
  glm::mat4 t3 = glm::make_mat4(t3Array);
  glm::vec4 view3 = view2 * t3;
  transformationMatrix *= t3;

  // Update H (distance of projection plane from viewpoint)
  H = view3[2];

  // Update transformation matrix by multiplying it with T4 and T5
  double t4Array[16] = {0, 1, 0, 0, -1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
  glm::mat4 t4 = glm::make_mat4(t4Array);
  transformationMatrix *= t4;

  double t5Array[16] = {-1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
  glm::mat4 t5 = glm::make_mat4(t5Array);
  transformationMatrix *= t5;
}

glm::vec2 getProcessedPoint(glm::vec4 scenePoint) {
  // Transform the point
  glm::vec4 transformedPoint = scenePoint * transformationMatrix;

  // Project the 3D transformed point to 2D plane
  double xP = transformedPoint[0] / transformedPoint[2] * H;
  double yP = transformedPoint[1] / transformedPoint[2] * H;

  double pointArr[2] = {xP, yP};
  return glm::make_vec2(pointArr);
}

void drawObject() {
  glPushMatrix();
  glTranslatef(width / 2, height / 2, 0);
  glScalef(50.0f, 50.0f, 1.0f);

  for (int i = 0; i < object.faces.size(); i++) {
    Face face = object.faces[i];
    glm::vec4 normal = getFaceNormal(face);

    vector<glm::vec4> points;
    glm::vec4 v1Orig = getPointByIndex(face.v1 - 1);
    glm::vec4 v2Orig = getPointByIndex(face.v2 - 1);
    glm::vec4 v3Orig = getPointByIndex(face.v3 - 1);
    points.push_back(v1Orig);
    points.push_back(v2Orig);
    points.push_back(v3Orig);

    glm::vec4 center = getObjectCenter(points);
    glm::vec4 centerViewpointVector = viewpoint - center;

    double scalarProduct = glm::dot(centerViewpointVector, normal);

    if (scalarProduct >= 0) {
      glm::vec2 v1 = getProcessedPoint(v1Orig);
      glm::vec2 v2 = getProcessedPoint(v2Orig);
      glm::vec2 v3 = getProcessedPoint(v3Orig);

      glColor3f(0.0f, 0.0f, 0.0f);
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      glBegin(GL_POLYGON);
      {
        glVertex2d(v1[0], v1[1]);
        glVertex2d(v2[0], v2[1]);
        glVertex2d(v3[0], v3[1]);
      }
      glEnd();
    }
  }

  glPopMatrix();
}

void display(void)
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    if (!animate) {
      drawControlAndBezier();
    } else {
      drawObject();
      glutTimerFunc(100, doAnimation, 0);
    }
    glutSwapBuffers();
}

/**
 * This function is called every time the window is resized.
 * */
void reshape(int w, int h)
{
    width = w;
    height = h;

    /*
    glFrontFace(GL_CCW);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    */

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
