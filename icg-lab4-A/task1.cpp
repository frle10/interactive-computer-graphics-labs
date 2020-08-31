/**
 * @author Ivan Skorupan
 */
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
void normalizeObject();
void updateTransformationMatrix();
void calculateConstantColorComponents();
void calculateVertexNormals();
void keyPressed(unsigned char, int, int);
void keySpecial(int, int, int);
void updateMaxCoordinates(double, double, double);

unsigned int calculateGoraudColorComponent(int);

glm::vec2 getProcessedPoint(glm::vec4);
glm::vec4 getFaceNormal(Face);
glm::vec4 getPointByIndex(int);
glm::vec4 getObjectCenter(vector<glm::vec4>);

glm::mat4 transformationMatrix;
glm::vec4 viewpoint;
glm::vec4 view;
glm::vec4 lightSource;

double H;
double xmin, xmax, ymin, ymax, zmin, zmax;

int activeCoordinate = 0;
glm::vec4 *activePoint;
double step = 0.1;

vector<glm::vec4> vertexNormals;

struct Face
{
  int v1;
  int v2;
  int v3;
  double ambient;
  double diffuse;
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
  window = glutCreateWindow("Lighting");
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyPressed);
  glutSpecialFunc(keySpecial);

  printf("Lighting\n");
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

        updateMaxCoordinates(x, y, z);
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

  normalizeObject();

  double x, y, z;
  cout << "Enter viewpoint coordinates:\n";
  cin >> x >> y >> z;
  double viewpointArr[4] = {x, y, z, 1};
  viewpoint = glm::make_vec4(viewpointArr);

  cout << "Enter light source coordinates:\n";
  cin >> x >> y >> z;
  double lightSourceArr[4] = {x, y, z, 1};
  lightSource = glm::make_vec4(lightSourceArr);

  view = getObjectCenter(object.vertices);
  activePoint = &viewpoint;
  updateTransformationMatrix();
  calculateConstantColorComponents();
  calculateVertexNormals();

  glutMainLoop();
  return 0;
}

/**
 * This function calculates each vertex's normal vector.
 */
void calculateVertexNormals() {
  for (int i = 0; i < object.vertices.size(); i++) {
    vector<glm::vec4> containingFacesNormals;
    for (int j = 0; j < object.faces.size(); j++) {
      Face face = object.faces[j];
      if (face.v1 - 1 == i || face.v2 - 1 == i || face.v3 - 1 == i) {
        containingFacesNormals.push_back(getFaceNormal(face));
      }
    }

    glm::vec4 vertexNormal = containingFacesNormals[0];
    for (int j = 1; j < containingFacesNormals.size(); j++) {
      vertexNormal += containingFacesNormals[j];
    }

    float factor = 1. / containingFacesNormals.size();
    vertexNormals.push_back(factor * vertexNormal);
  }
}

/**
 * Calculates the constant color component for each face.
 * For this we need the coordinates of the point and the light source.
 * The function uses hardcoded values for light intensity.
 */
void calculateConstantColorComponents() {
  for (int i = 0; i < object.faces.size(); i++) {
    vector<glm::vec4> points = {getPointByIndex(object.faces[i].v1), getPointByIndex(object.faces[i].v2), getPointByIndex(object.faces[i].v3)};
    double scalarProduct = glm::dot(glm::normalize(getFaceNormal(object.faces[i])), glm::normalize(lightSource - getObjectCenter(points)));

    object.faces[i].ambient = 125 * 1;
    object.faces[i].diffuse = 130 * 1 * max(0.0, scalarProduct);
  }
}

/**
 * Calculates the Goraud color component for a point at given index.
 * For this we need the coordinates of the point and the light source.
 * The function uses hardcoded values for light intensity.
 */
unsigned int calculateGouraudColorComponent(int pointIndex) {
  glm::vec4 point = object.vertices[pointIndex];
  glm::vec4 pointToLightSource = lightSource - point;
  glm::vec4 pointNormal = vertexNormals[pointIndex];

  double scalarProduct = glm::dot(glm::normalize(pointNormal), glm::normalize(pointToLightSource));

  double ambient = 75 * 1;
  double diffuse = 180 * 1 * max(0.0, scalarProduct);

  return ambient + diffuse;
}

/**
 * Every time viewpoint or view coordinates change, this method should be called (if we are using OpenGL given commands).
 */
void updatePerspective()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (float)width/height, 0.5, 8.0);
	gluLookAt(viewpoint[0], viewpoint[1], viewpoint[2], view[0], view[1], view[2], 0.0, 1.0, 0.0);
	glMatrixMode(GL_MODELVIEW);
}

/**
 * Tansforms object's vertices to be in [-1, 1] range.
 */
void normalizeObject()
{
  for (int i = 0; i < object.vertices.size(); i++)
  {
    object.vertices[i][0] -= (xmax + xmin) / 2.;
    object.vertices[i][1] -= (ymax + ymin) / 2.;
    object.vertices[i][2] -= (zmax + zmin) / 2.;
  }

  double maxRange = max(xmax - xmin, max(ymax - ymin, ymax - zmin));
  for (int i = 0; i < object.vertices.size(); i++)
  {
    object.vertices[i][0] *= 2 / maxRange;
    object.vertices[i][1] *= 2 / maxRange;
    object.vertices[i][2] *= 2 / maxRange;
  }
}

/**
 * This function checks if the given coordinates are bigger than current max remembered
 * coordinates.
 */
void updateMaxCoordinates(double x, double y, double z)
{
  if (object.vertices.size() == 0)
  {
    xmin = xmax = x;
    ymin = ymax = y;
    zmin = zmax = z;
  }
  else
  {
    if (x < xmin)
      xmin = x;
    if (x > xmax)
      xmax = x;
    if (y < ymin)
      ymin = y;
    if (y > ymax)
      ymax = y;
    if (z < zmin)
      zmin = z;
    if (z > zmax)
      zmax = z;
  }
}

/**
 * Takes a vector of object's vertices and returns a point that is the
 * center of that object.
 */
glm::vec4 getObjectCenter(vector<glm::vec4> points)
{
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

/**
 * Test for key presses that update the active point or active coordinate.
 */
void keyPressed(unsigned char key, int x, int y)
{
  if (key == 'x')
    activeCoordinate = 0;
  else if (key == 'y')
    activeCoordinate = 1;
  else if (key == 'z')
    activeCoordinate = 2;
  else if (key == 'o')
    activePoint = &viewpoint;
  else if (key == 'g')
    activePoint = &view;
  else if (key == 'l')
    activePoint = &lightSource;

  cout << "Detected key press: " << key << endl;
}

/**
 * Test for up or down arrow key presses to update the active point coordinates.
 */
void keySpecial(int key, int x, int y)
{
  if (key == GLUT_KEY_UP)
  {
    (*activePoint)[activeCoordinate] += step;
    updateTransformationMatrix();
    calculateConstantColorComponents();
    // updatePerspective();
    glutPostRedisplay();
  }
  else if (key == GLUT_KEY_DOWN)
  {
    (*activePoint)[activeCoordinate] -= step;
    updateTransformationMatrix();
    calculateConstantColorComponents();
    // updatePerspective();
    glutPostRedisplay();
  }

  cout << "Detected special press: " << key << endl;
}

/**
 * Return a point at given index in the vertices array.
 */
glm::vec4 getPointByIndex(int index)
{
  return object.vertices[index];
}

/**
 * Takes an object's face and returns its normal vector.
 */
glm::vec4 getFaceNormal(Face face)
{
  glm::vec4 p1 = getPointByIndex(face.v1 - 1);
  glm::vec4 p2 = getPointByIndex(face.v2 - 1);
  glm::vec4 p3 = getPointByIndex(face.v3 - 1);

  double A = (p2[1] - p1[1]) * (p3[2] - p1[2]) - (p2[2] - p1[2]) * (p3[1] - p1[1]);
  double B = -(p2[0] - p1[0]) * (p3[2] - p1[2]) + (p2[2] - p1[2]) * (p3[0] - p1[0]);
  double C = (p2[0] - p1[0]) * (p3[1] - p1[1]) - (p2[1] - p1[1]) * (p3[0] - p1[0]);

  double normalArr[4] = {A, B, C, 0};
  return glm::make_vec4(normalArr);
}

/**
 * Updates the transformation matrix. This method should be called every time the
 * viewpoint or view coordinates change.
 */
void updateTransformationMatrix()
{
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

/**
 * Takes a point and passes it through the transformation and projection
 * matrix after which the transformed 2D point is returned.
 */
glm::vec2 getProcessedPoint(glm::vec4 scenePoint)
{
  // Transform the point
  glm::vec4 transformedPoint = scenePoint * transformationMatrix;

  // Project the 3D transformed point to 2D plane
  double xP = transformedPoint[0] / transformedPoint[2] * H;
  double yP = transformedPoint[1] / transformedPoint[2] * H;

  double pointArr[2] = {xP, yP};
  return glm::make_vec2(pointArr);
}

/**
 * Draws the object and centers it on the screen. The method implements
 * back face culling using the scalar product method.
 */
void drawObject()
{
  glPushMatrix();
  glTranslatef(width / 2, height / 2, 0);
  glScalef(50, 50, 50);

  for (int i = 0; i < object.faces.size(); i++)
  {
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
    if (scalarProduct >= 0)
    {
      glm::vec2 v1 = getProcessedPoint(object.vertices[face.v1 - 1]);
      glm::vec2 v2 = getProcessedPoint(object.vertices[face.v2 - 1]);
      glm::vec2 v3 = getProcessedPoint(object.vertices[face.v3 - 1]);

      // glm::vec3 v1NoProcess = object.vertices[face.v1 - 1];
      // glm::vec3 v2NoProcess = object.vertices[face.v2 - 1];
      // glm::vec3 v3NoProcess = object.vertices[face.v3 - 1];

      glBegin(GL_TRIANGLES);
      {
        glColor3ub(30, 30, calculateGouraudColorComponent(face.v1 - 1)); glVertex2d(v1[0], v1[1]);
        glColor3ub(30, 30, calculateGouraudColorComponent(face.v2 - 1)); glVertex2d(v2[0], v2[1]);
        glColor3ub(30, 30, calculateGouraudColorComponent(face.v3 - 1)); glVertex2d(v3[0], v3[1]);

        // glColor3ub(0, 0, face.ambient + face.diffuse); glVertex2d(v1[0], v1[1]);
        // glColor3ub(0, 0, face.ambient + face.diffuse); glVertex2d(v2[0], v2[1]);
        // glColor3ub(0, 0, face.ambient + face.diffuse); glVertex2d(v3[0], v3[1]);

        // glVertex3d(v1NoProcess[0], v1NoProcess[1], v1NoProcess[2]);
        // glVertex3d(v2NoProcess[0], v2NoProcess[1], v2NoProcess[2]);
        // glVertex3d(v3NoProcess[0], v3NoProcess[1], v3NoProcess[2]);
      }
      glEnd();
    }
  }

  glPopMatrix();
}

/**
 * Function that renders the scene content on the screen.
 */
void display(void)
{
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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

  // glFrontFace(GL_CCW);
  // glEnable(GL_CULL_FACE);
  // glCullFace(GL_BACK);

  glDisable(GL_DEPTH_TEST);
  glViewport(0, 0, width, height);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  // gluPerspective(45.0, (float)width / height, 0.5, 8.0);
  // gluLookAt (viewpoint[0], viewpoint[1], viewpoint[2], view[0], view[1], view[2], 0.0, 1.0, 0.0);
  gluOrtho2D(0, width - 1, 0, height - 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}
