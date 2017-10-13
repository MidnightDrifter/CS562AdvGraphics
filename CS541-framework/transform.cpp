 #include <fstream>

#include "math.h"
#include "transform.h"

// This is used to communicate a MAT4's address to OpenGL
float* MAT4::Pntr()
{
    return &(M[0][0]);
}

// Return a rotation matrix around an axis (0:X, 1:Y, 2:Z) 
// by an angle measured in degrees.
// NOTE:  Make sure to convert degrees to radians before using sin and cos:
//        radians = degrees*PI/180
const float pi = 3.14159f;
MAT4 Rotate(const int i, const float theta)
{
    MAT4 R;

	float angle = theta * pi / 180.f;

	/*
	
		int b = i + 1 % 3;
		int c = b + 1 % 3;

		R[b][b] = cosf(angle);
		R[c][c] = R[b][b];
		R[c][b] = sinf(angle);
		R[b][c] = -1 * R[c][b];
	*/

	if (i == 2)
	{
		R[0][0] = cosf(angle);
		R[1][1] = R[0][0];
		R[1][0] = sinf(angle);
		R[0][1] = -1 * R[1][0];
	}

	else if (i == 0)
	{
		R[1][1] = cosf(angle);
		R[2][2] = R[1][1];
		R[2][1] = sinf(angle);
		R[1][2] = -1 * R[2][1];
	}

	else if (i == 1)
	{
		R[0][0] = cosf(angle);
		R[2][0] = sinf(angle);
		R[2][2] = R[0][0];
		R[0][2] = -1 * R[2][0];
	}

    return R;
}

// Return a scale matrix
MAT4 Scale(const float x, const float y, const float z)
{
    MAT4 S;
	S[0][0] = x;
	S[1][1] = y;
	S[2][2] = z;
    return S;
}

// Return a translation matrix
MAT4 Translate(const float x, const float y, const float z)
{
    MAT4 T;
	T[0][3] = x;
	T[1][3] = y;
	T[2][3] = z;
    return T;
}

// Returns a perspective projection matrix
MAT4 Perspective(const float rx, const float ry,
             const float front, const float back)
{
    MAT4 R;

	R[0][0] = 1.f / rx;
	R[1][1] = 1.f / ry;
	R[2][2] = (back + front) / (front - back);
	R[2][3] = (2.f * back*front) / (front - back);
	R[3][2] = -1.f;
	R[3][3] = 0.f;

    return R;
}

// Multiplies two 4x4 matrices
MAT4 operator* (const MAT4 A, const MAT4 B)
{  
    MAT4 M;
	M[0][0] = 0;
	M[1][1] = 0;
	M[2][2] = 0;
	M[3][3] = 0;

	for(int i=0;i<4;i++)
	{
		for (int j = 0; j < 4; j++)
		{
			for (int k = 0; k < 4; k++)
			{
				M[i][j] += A[i][k] * B[k][j];
			}
		}
	}

    return M;
}


MAT4 LookAt(float eyeX, float eyeY, float eyeZ, float centerX, float centerY, float centerZ, float upX, float upY, float upZ)
{
	MAT4 out;

	float aX, aY, aZ, bX, bY, bZ, vX, vY, vZ, aLength, vLength, bLength;



	vX = centerX - eyeX;
	vY = centerY - eyeY;
	vZ = centerZ - eyeZ;

	vLength = sqrtf((vX*vX) + (vY*vY) + (vZ*vZ));

	vX = vX/ vLength;
	vY =vY/ vLength;
	vZ =vZ/ vLength;

	aX = (vY*upZ) - (vZ*upY);
	aY = (vZ*upX) - (vX*upZ);
	aZ = (vX*upY) - (vY*upX);

	aLength = sqrtf((aX*aX + aY*aY + aZ*aZ));

	aX = aX / aLength;
	aY = aY/aLength;
	aZ = aZ /aLength;

	bX = (aY*vZ) - (aZ*vY);
	bY = (aZ*vX) - (aX*vZ);
	bZ = (aX*vY) - (aY*vX);

	bLength = sqrtf((bX*bX + bY*bY + bZ*bZ));
	bX =bX/ bLength;
	bY =bY/ bLength;
	bZ =bZ/ bLength;

	out[0][0] = aX;
	out[0][1] = aY;
	out[0][2] = aZ;
	out[0][3] = (-1 * aX * eyeX) - (aY*eyeY) - (aZ*eyeZ);

	out[1][0] = bX;
	out[1][1] = bY;
	out[1][2] = bZ;
	out[1][3] = (-1 * bX * eyeX) - (bY*eyeY) - (bZ*eyeZ);

	out[2][0] = vX*-1;
	out[2][1] = vY*-1;
	out[2][2] = vZ*-1;
	out[2][3] = ( vX * eyeX) + (vY*eyeY) + (vZ*eyeZ);

	out[3][0] = 0.f;
	out[3][1] = 0.f;
	out[3][2] = 0.f;
	out[3][3] = 1.f;

	return out;

}


////////////////////////////////////////////////////////////////////////
// Calculates the inverse of a matrix by performing the gaussian
// matrix reduction with partial pivoting followed by
// back/substitution with the loops manually unrolled.
//
// Taken from Mesa implementation of OpenGL:  http://mesa3d.sourceforge.net/
////////////////////////////////////////////////////////////////////////
#define MAT(m,r,c) ((*m)[r][c])
#define SWAP_ROWS(a, b) { double *_tmp = a; (a)=(b); (b)=_tmp; }

int invert(const MAT4* mat, MAT4* inv)
{
   double wtmp[4][8];
   double m0, m1, m2, m3, s;
   double *r0, *r1, *r2, *r3;

   r0 = wtmp[0], r1 = wtmp[1], r2 = wtmp[2], r3 = wtmp[3];

   r0[0] = MAT(mat,0,0);
   r0[1] = MAT(mat,0,1);
   r0[2] = MAT(mat,0,2);
   r0[3] = MAT(mat,0,3);
   r0[4] = 1.0;
   r0[5] = r0[6] = r0[7] = 0.0;
   
   r1[0] = MAT(mat,1,0);
   r1[1] = MAT(mat,1,1);
   r1[2] = MAT(mat,1,2);
   r1[3] = MAT(mat,1,3);
   r1[5] = 1.0, r1[4] = r1[6] = r1[7] = 0.0;

   r2[0] = MAT(mat,2,0);
   r2[1] = MAT(mat,2,1);
   r2[2] = MAT(mat,2,2);
   r2[3] = MAT(mat,2,3);
   r2[6] = 1.0, r2[4] = r2[5] = r2[7] = 0.0;

   r3[0] = MAT(mat,3,0);
   r3[1] = MAT(mat,3,1);
   r3[2] = MAT(mat,3,2);
   r3[3] = MAT(mat,3,3);
   r3[7] = 1.0, r3[4] = r3[5] = r3[6] = 0.0;

   /* choose pivot - or die */
   if (fabs(r3[0])>fabs(r2[0])) SWAP_ROWS(r3, r2);
   if (fabs(r2[0])>fabs(r1[0])) SWAP_ROWS(r2, r1);
   if (fabs(r1[0])>fabs(r0[0])) SWAP_ROWS(r1, r0);
   if (0.0 == r0[0])  return 0;

   /* eliminate first variable     */
   m1 = r1[0]/r0[0]; m2 = r2[0]/r0[0]; m3 = r3[0]/r0[0];
   s = r0[1]; r1[1] -= m1 * s; r2[1] -= m2 * s; r3[1] -= m3 * s;
   s = r0[2]; r1[2] -= m1 * s; r2[2] -= m2 * s; r3[2] -= m3 * s;
   s = r0[3]; r1[3] -= m1 * s; r2[3] -= m2 * s; r3[3] -= m3 * s;
   s = r0[4];
   if (s != 0.0) { r1[4] -= m1 * s; r2[4] -= m2 * s; r3[4] -= m3 * s; }
   s = r0[5];
   if (s != 0.0) { r1[5] -= m1 * s; r2[5] -= m2 * s; r3[5] -= m3 * s; }
   s = r0[6];
   if (s != 0.0) { r1[6] -= m1 * s; r2[6] -= m2 * s; r3[6] -= m3 * s; }
   s = r0[7];
   if (s != 0.0) { r1[7] -= m1 * s; r2[7] -= m2 * s; r3[7] -= m3 * s; }

   /* choose pivot - or die */
   if (fabs(r3[1])>fabs(r2[1])) SWAP_ROWS(r3, r2);
   if (fabs(r2[1])>fabs(r1[1])) SWAP_ROWS(r2, r1);
   if (0.0 == r1[1])  return 0;

   /* eliminate second variable */
   m2 = r2[1]/r1[1]; m3 = r3[1]/r1[1];
   r2[2] -= m2 * r1[2]; r3[2] -= m3 * r1[2];
   r2[3] -= m2 * r1[3]; r3[3] -= m3 * r1[3];
   s = r1[4]; if (0.0 != s) { r2[4] -= m2 * s; r3[4] -= m3 * s; }
   s = r1[5]; if (0.0 != s) { r2[5] -= m2 * s; r3[5] -= m3 * s; }
   s = r1[6]; if (0.0 != s) { r2[6] -= m2 * s; r3[6] -= m3 * s; }
   s = r1[7]; if (0.0 != s) { r2[7] -= m2 * s; r3[7] -= m3 * s; }

   /* choose pivot - or die */
   if (fabs(r3[2])>fabs(r2[2])) SWAP_ROWS(r3, r2);
   if (0.0 == r2[2])  return 0;

   /* eliminate third variable */
   m3 = r3[2]/r2[2];
   r3[3] -= m3 * r2[3];
   r3[4] -= m3 * r2[4];
   r3[5] -= m3 * r2[5];
   r3[6] -= m3 * r2[6];
   r3[7] -= m3 * r2[7];

   /* last check */
   if (0.0 == r3[3]) return 0;

   s = 1.0F/r3[3];             /* now back substitute row 3 */
   r3[4] *= s; r3[5] *= s; r3[6] *= s; r3[7] *= s;

   m2 = r2[3];                 /* now back substitute row 2 */
   s  = 1.0F/r2[2];
   r2[4] = s * (r2[4] - r3[4] * m2);
   r2[5] = s * (r2[5] - r3[5] * m2);
   r2[6] = s * (r2[6] - r3[6] * m2);
   r2[7] = s * (r2[7] - r3[7] * m2);
   m1 = r1[3];
   r1[4] -= r3[4] * m1;
   r1[5] -= r3[5] * m1;
   r1[6] -= r3[6] * m1;
   r1[7] -= r3[7] * m1;
   m0 = r0[3];
   r0[4] -= r3[4] * m0;
   r0[5] -= r3[5] * m0;
   r0[6] -= r3[6] * m0;
   r0[7] -= r3[7] * m0;

   m1 = r1[2];                 /* now back substitute row 1 */
   s  = 1.0F/r1[1];
   r1[4] = s * (r1[4] - r2[4] * m1);
   r1[5] = s * (r1[5] - r2[5] * m1);
   r1[6] = s * (r1[6] - r2[6] * m1);
   r1[7] = s * (r1[7] - r2[7] * m1);
   m0 = r0[2];
   r0[4] -= r2[4] * m0;
   r0[5] -= r2[5] * m0;
   r0[6] -= r2[6] * m0;
   r0[7] -= r2[7] * m0;

   m0 = r0[1];                 /* now back substitute row 0 */
   s  = 1.0F/r0[0];
   r0[4] = s * (r0[4] - r1[4] * m0);
   r0[5] = s * (r0[5] - r1[5] * m0);
   r0[6] = s * (r0[6] - r1[6] * m0);
   r0[7] = s * (r0[7] - r1[7] * m0);

   MAT(inv,0,0) = r0[4];
   MAT(inv,0,1) = r0[5],
   MAT(inv,0,2) = r0[6];
   MAT(inv,0,3) = r0[7],
   MAT(inv,1,0) = r1[4];
   MAT(inv,1,1) = r1[5],
   MAT(inv,1,2) = r1[6];
   MAT(inv,1,3) = r1[7],
   MAT(inv,2,0) = r2[4];
   MAT(inv,2,1) = r2[5],
   MAT(inv,2,2) = r2[6];
   MAT(inv,2,3) = r2[7],
   MAT(inv,3,0) = r3[4];
   MAT(inv,3,1) = r3[5],
   MAT(inv,3,2) = r3[6];
   MAT(inv,3,3) = r3[7];

   return 1;
}
