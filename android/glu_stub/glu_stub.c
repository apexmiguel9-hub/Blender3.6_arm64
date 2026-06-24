#include <GL/gl.h>
#include <GL/glu.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct {
	GLenum drawStyle;
} GLUquadric;

GLUquadric *gluNewQuadric(void)
{
	GLUquadric *q = calloc(1, sizeof(GLUquadric));
	return q;
}

void gluDeleteQuadric(GLUquadric *q)
{
	free(q);
}

void gluQuadricDrawStyle(GLUquadric *q, GLenum style)
{
	q->drawStyle = style;
}

void gluSphere(GLUquadric *q, GLdouble radius, GLint slices, GLint stacks)
{
	(void)q;
	int i, j;
	for (i = 0; i < stacks; i++) {
		float lat0 = (float)(M_PI * (-0.5 + (double)i / stacks));
		float lat1 = (float)(M_PI * (-0.5 + (double)(i + 1) / stacks));
		float z0 = sinf(lat0) * (float)radius;
		float zr0 = cosf(lat0) * (float)radius;
		float z1 = sinf(lat1) * (float)radius;
		float zr1 = cosf(lat1) * (float)radius;
		glBegin(GL_QUAD_STRIP);
		for (j = 0; j <= slices; j++) {
			float lng = (float)(2 * M_PI * (double)j / slices);
			float x = cosf(lng);
			float y = sinf(lng);
			glNormal3f(x * zr1, y * zr1, z1);
			glVertex3f(x * zr1, y * zr1, z1);
			glNormal3f(x * zr0, y * zr0, z0);
			glVertex3f(x * zr0, y * zr0, z0);
		}
		glEnd();
	}
}

static void matmul(GLdouble out[16], const GLdouble a[16], const GLdouble b[16])
{
	int i, j;
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
			out[j * 4 + i] = a[i * 4 + 0] * b[0 * 4 + j] +
			                 a[i * 4 + 1] * b[1 * 4 + j] +
			                 a[i * 4 + 2] * b[2 * 4 + j] +
			                 a[i * 4 + 3] * b[3 * 4 + j];
}

static int invert(GLdouble inv[16], const GLdouble m[16])
{
	GLdouble det;
	int i;

	inv[0] = m[5]  * m[10] * m[15] -
	         m[5]  * m[11] * m[14] -
	         m[9]  * m[6]  * m[15] +
	         m[9]  * m[7]  * m[14] +
	         m[13] * m[6]  * m[11] -
	         m[13] * m[7]  * m[10];

	inv[4] = -m[4]  * m[10] * m[15] +
	          m[4]  * m[11] * m[14] +
	          m[8]  * m[6]  * m[15] -
	          m[8]  * m[7]  * m[14] -
	          m[12] * m[6]  * m[11] +
	          m[12] * m[7]  * m[10];

	inv[8] = m[4]  * m[9]  * m[15] -
	         m[4]  * m[11] * m[13] -
	         m[8]  * m[5]  * m[15] +
	         m[8]  * m[7]  * m[13] +
	         m[12] * m[5]  * m[11] -
	         m[12] * m[7]  * m[9];

	inv[12] = -m[4]  * m[9]  * m[14] +
	           m[4]  * m[10] * m[13] +
	           m[8]  * m[5]  * m[14] -
	           m[8]  * m[6]  * m[13] -
	           m[12] * m[5]  * m[10] +
	           m[12] * m[6]  * m[9];

	inv[1] = -m[1]  * m[10] * m[15] +
	          m[1]  * m[11] * m[14] +
	          m[9]  * m[2]  * m[15] -
	          m[9]  * m[3]  * m[14] -
	          m[13] * m[2]  * m[11] +
	          m[13] * m[3]  * m[10];

	inv[5] = m[0]  * m[10] * m[15] -
	         m[0]  * m[11] * m[14] -
	         m[8]  * m[2]  * m[15] +
	         m[8]  * m[3]  * m[14] +
	         m[12] * m[2]  * m[11] -
	         m[12] * m[3]  * m[10];

	inv[9] = -m[0]  * m[9]  * m[15] +
	          m[0]  * m[11] * m[13] +
	          m[8]  * m[1]  * m[15] -
	          m[8]  * m[3]  * m[13] -
	          m[12] * m[1]  * m[11] +
	          m[12] * m[3]  * m[9];

	inv[13] = m[0]  * m[9]  * m[14] -
	          m[0]  * m[10] * m[13] -
	          m[8]  * m[1]  * m[14] +
	          m[8]  * m[2]  * m[13] +
	          m[12] * m[1]  * m[10] -
	          m[12] * m[2]  * m[9];

	inv[2] = m[1]  * m[6]  * m[15] -
	         m[1]  * m[7]  * m[14] -
	         m[5]  * m[2]  * m[15] +
	         m[5]  * m[3]  * m[14] +
	         m[13] * m[2]  * m[7] -
	         m[13] * m[3]  * m[6];

	inv[6] = -m[0]  * m[6]  * m[15] +
	          m[0]  * m[7]  * m[14] +
	          m[4]  * m[2]  * m[15] -
	          m[4]  * m[3]  * m[14] -
	          m[12] * m[2]  * m[7] +
	          m[12] * m[3]  * m[6];

	inv[10] = m[0]  * m[5]  * m[15] -
	          m[0]  * m[7]  * m[13] -
	          m[4]  * m[1]  * m[15] +
	          m[4]  * m[3]  * m[13] +
	          m[12] * m[1]  * m[7] -
	          m[12] * m[3]  * m[5];

	inv[14] = -m[0]  * m[5]  * m[14] +
	           m[0]  * m[6]  * m[13] +
	           m[4]  * m[1]  * m[14] -
	           m[4]  * m[2]  * m[13] -
	           m[12] * m[1]  * m[6] +
	           m[12] * m[2]  * m[5];

	inv[3] = -m[1]  * m[6]  * m[11] +
	          m[1]  * m[7]  * m[10] +
	          m[5]  * m[2]  * m[11] -
	          m[5]  * m[3]  * m[10] -
	          m[9]  * m[2]  * m[7] +
	          m[9]  * m[3]  * m[6];

	inv[7] = m[0]  * m[6]  * m[11] -
	         m[0]  * m[7]  * m[10] -
	         m[4]  * m[2]  * m[11] +
	         m[4]  * m[3]  * m[10] +
	         m[8]  * m[2]  * m[7] -
	         m[8]  * m[3]  * m[6];

	inv[11] = -m[0]  * m[5]  * m[11] +
	           m[0]  * m[7]  * m[9] +
	           m[4]  * m[1]  * m[11] -
	           m[4]  * m[3]  * m[9] -
	           m[8]  * m[1]  * m[7] +
	           m[8]  * m[3]  * m[5];

	inv[15] = m[0]  * m[5]  * m[10] -
	          m[0]  * m[6]  * m[9] -
	          m[4]  * m[1]  * m[10] +
	          m[4]  * m[2]  * m[9] +
	          m[8]  * m[1]  * m[6] -
	          m[8]  * m[2]  * m[5];

	det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
	if (det == 0) return 0;
	det = 1.0 / det;
	for (i = 0; i < 16; i++) inv[i] *= det;
	return 1;
}

int gluUnProject(GLdouble winx, GLdouble winy, GLdouble winz,
                 const GLdouble modelMatrix[16],
                 const GLdouble projMatrix[16],
                 const GLint viewport[4],
                 GLdouble *objx, GLdouble *objy, GLdouble *objz)
{
	GLdouble m[16], inv[16], in[4], out[4];

	matmul(m, modelMatrix, projMatrix);
	if (!invert(inv, m)) return 0;

	in[0] = (winx - viewport[0]) / viewport[2] * 2.0 - 1.0;
	in[1] = (winy - viewport[1]) / viewport[3] * 2.0 - 1.0;
	in[2] = 2.0 * winz - 1.0;
	in[3] = 1.0;

	out[0] = inv[0] * in[0] + inv[4] * in[1] + inv[8]  * in[2] + inv[12] * in[3];
	out[1] = inv[1] * in[0] + inv[5] * in[1] + inv[9]  * in[2] + inv[13] * in[3];
	out[2] = inv[2] * in[0] + inv[6] * in[1] + inv[10] * in[2] + inv[14] * in[3];
	out[3] = inv[3] * in[0] + inv[7] * in[1] + inv[11] * in[2] + inv[15] * in[3];

	if (out[3] == 0.0) return 0;
	*objx = out[0] / out[3];
	*objy = out[1] / out[3];
	*objz = out[2] / out[3];
	return 1;
}
