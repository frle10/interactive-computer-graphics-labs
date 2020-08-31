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
void drawObject();
void updateTransformationMatrix();
void keyPressed(unsigned char, int, int);
void keySpecial(int, int, int);

glm::vec2 getProcessedPoint(glm::vec4);
glm::vec4 getObjectCenter(vector<glm::vec4>);

glm::mat4 transformationMatrix;
glm::vec4 viewpoint;
glm::vec4 view;

double H;

int activeCoordinate = 0;
glm::vec4 *activePoint;
double step = 0.1;

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
    window = glutCreateWindow("Projections");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyPressed);
    glutSpecialFunc(keySpecial);

    printf("Projections\n");
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

    double x, y, z;
    cout << "Enter viewpoint coordinates:\n";
    cin >> x >> y >> z;
    double viewpointArr[4] = {x, y, z, 1};
    viewpoint = glm::make_vec4(viewpointArr);

    // cout << endl;
    // cout << "Enter view coordinates:\n";
    // cin >> x >> y >> z;

    view = getObjectCenter(object.vertices);

    // double viewArr[4] = {x, y, z, 1};
    // view = glm::make_vec4(viewArr);

    activePoint = &viewpoint;
    updateTransformationMatrix();

    glutMainLoop();
    return 0;
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

void keyPressed(unsigned char key, int x, int y) {
  if (key == 'x') activeCoordinate = 0;
  else if (key == 'y') activeCoordinate = 1;
  else if (key == 'z') activeCoordinate = 2;
  else if (key == 'o') activePoint = &viewpoint;
  else if (key == 'g') activePoint = &view;

  cout << "Detected key press: " << key << endl;
}

void keySpecial(int key, int x, int y) {
  if (key == GLUT_KEY_UP) {
    (*activePoint)[activeCoordinate] += step;
    updateTransformationMatrix();
    glutPostRedisplay();
  }
  else if (key == GLUT_KEY_DOWN) {
    (*activePoint)[activeCoordinate] -= step;
    updateTransformationMatrix();
    glutPostRedisplay();
  }

  cout << "Detected special press: " << key << endl;
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
    glm::vec2 v1 = getProcessedPoint(object.vertices[face.v1 - 1]);
    glm::vec2 v2 = getProcessedPoint(object.vertices[face.v2 - 1]);
    glm::vec2 v3 = getProcessedPoint(object.vertices[face.v3 - 1]);

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

  glPopMatrix();
}

void display(void)
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    drawObject();
    glutSwapBuffers();
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
