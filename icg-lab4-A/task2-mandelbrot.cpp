/**
 * @author Ivan Skorupan
 */
#include <iostream>
#include <cmath>
#include <GL/glut.h>

GLuint window;
GLuint width = 500, height = 500;

typedef struct {
  double re;
  double im;
} complex;

void renderScene();
void display(void);
void reshape(int, int);
int divergence_test(complex, int, double);

int limit = 16;
double eps = 100;
double umin = -1.5, umax = 0.5, vmin = -1, vmax = 1;

int ymin = 0, xmin = 0, ymax = height, xmax = width;

int main(int argc, char **argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE);
  glutInitWindowSize(width, height);
  glutInitWindowPosition(100, 100);
  window = glutCreateWindow("Fractals");
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);

  printf("Fractals\n");
  printf("----------------------\n\n");

  glutMainLoop();
  return 0;
}

int divergence_test(complex c, int limit, double eps) {
  complex z;
  z.re = 0; z.im = 0;

  for (int i = 1; i <= limit; i++) {
    double next_re = z.re * z.re - z.im * z.im + c.re;
    double next_im = 2 * z.re * z.im + c.im;
    z.re = next_re;
    z.im = next_im;
    double modul2 = z.re * z.re + z.im * z.im;
    if (modul2 > (eps * eps)) return i;
  }

  return -1;
}

void renderScene() {
  glPointSize(1);
  glBegin(GL_POINTS);
  for (int y = ymin; y <= ymax; y++) {
    for (int x = xmin; x <= xmax; x++) {
      complex c;
      c.re = (x - xmin) / (double)(xmax - xmin) * (umax - umin) + umin;
      c.im = (y - ymin) / (double)(ymax - ymin) * (vmax - vmin) + vmin;
      int n = divergence_test(c, limit, eps);
      if (n == -1) {
        glColor3f(0.0f, 0.0f, 0.0f);
      } else {
        glColor3f((double)n / limit,
          1.0 - (double)n / limit / 2.0,
          0.8 - (double)n / limit / 3.0);
      }

      glVertex2i(x, y);
    }
  }
  glEnd();
}

void display(void)
{
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  renderScene();
  glutSwapBuffers();
}

void reshape(int w, int h)
{
  xmax = width = w;
  ymax = height = h;

  glDisable(GL_DEPTH_TEST);
  glViewport(0, 0, width, height);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, width - 1, 0, height - 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}
