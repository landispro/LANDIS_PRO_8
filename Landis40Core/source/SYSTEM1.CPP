#define _CRT_SECURE_NO_WARNINGS

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdarg.h>

#include "error.h"
#include "system1.h"
#include "defines.h"



#define DELIMIT    '#'
#define PI         3.141592654



static int stringCommentFlag = 0;
static int charCommentFlag = 0;



// Given a string, if it is part of comment return TRUE, otherwise FALSE
int isStringComment(char *s)
{
	int count = 0;

	for (s; *s; s++)
	{
		if (*s == DELIMIT)
		{
			count += 1;

			stringCommentFlag = (stringCommentFlag + 1) % 2;
		}
	}

	if (stringCommentFlag == 1 || count > 0)
		return true;
	else
		return false;
}





// Given a charactor, if it is part of comment retunn TRUE, otherwise FALSE
int isCharComment(char ch)
{
	if (ch == DELIMIT)
		charCommentFlag = (charCommentFlag + 1) % 2;


	if (charCommentFlag == 1 || ch == DELIMIT)
		return true;
	else
		return false;
}



// fscanc: fscanf function with one more feature: jump the comment delimited
// by # pairs. It'll return TRUE, if success, otherwise FALSE.
int fscanc(FILE *fp, char *fmt, ...)
{
	va_list ap;                 // points to each unnamed arg in turn 

	char *p, *sval, temp[256];  //Nim: changed 80 to 255

	int* ival;       //Nim: changed int to int*

	long* lval;      //Nim: changed long to long*

	float* fval;    //Nim: added this line

	double* dval;    //Nim: changed double to double*





	va_start(ap, fmt);          // make ap point to 1st unnamed arg 

	for (p = fmt; *p; p++)
	{
		if (*p != '%') 
			continue;

		while (fscanf(fp, "%s", temp), isStringComment(temp))
			;


		switch (*++p)
		{

		case 'd':

			ival = va_arg(ap, int*);   //Nim: changed int to int*

			sscanf(temp, "%d", ival);

			break;

		case 'f':

			fval = va_arg(ap, float*); //Nim: changed dval to fval and double to float*

			sscanf(temp, "%f", fval);  //Nim: changed dval to fval

			break;

		case 's':

			sval = va_arg(ap, char*);

			strcpy(sval, temp);

			break;

		case 'l':

			switch (*++p)

			{

			case 'd':

				lval = va_arg(ap, long*); //Nim: changed long to long*

				sscanf(temp, "%ld", lval);

				break;

			case 'f':

				dval = va_arg(ap, double*); //Nim: changed double to double*

				sscanf(temp, "%lf", dval);   //Nim: changed %f to %lf

				break;

			default:

				break;

			}

			break;

		default:

			break;

		};

	}

	va_end(ap);             // clean up when done  

	return true;
}





//modified fscanc function, to read line by line and could contain space in the return string
int mfscanc(FILE *fp, char *fmt, ...)
{

	va_list ap;                 /* points to each unnamed arg in turn  */

	char *p, *sval, temp[256];

	int* ival;

	long* lval;

	float* fval;

	double* dval;

	char line[256];



	va_start(ap, fmt);          /* make ap point to 1st unnamed arg */

	for (p = fmt; *p; p++)
	{
		if (*p != '%')
			continue;

		while (!feof(fp))
		{
			strcpy(line, "");

			fgets(line, 256, fp);

			if (!strcmp(line, "")) 
				break;
		}



		if (strcmp(line, ""))
		{
			perror("Could not get string from infile");

			return false;

		}



		char seps[] = "#";

		char *token;

		token = strtok(line, seps);

		if (token != NULL)
			strcpy(temp, token);
		else
			strcpy(temp, line);



		switch (*++p)
		{

		case 'd':

			ival = va_arg(ap, int*);   //Nim: changed int to int*

			sscanf(temp, "%d", ival);

			break;

		case 'f':

			fval = va_arg(ap, float*); //Nim: changed dval to fval and double to float*

			sscanf(temp, "%f", fval);  //Nim: changed dval to fval

			break;

		case 's':

			sval = va_arg(ap, char*);

			strcpy(sval, temp);

			break;

		case 'l':

			switch (*++p)

			{

			case 'd':

				lval = va_arg(ap, long*); //Nim: changed long to long*

				sscanf(temp, "%ld", lval);

				break;

			case 'f':

				dval = va_arg(ap, double*); //Nim: changed double to double*

				sscanf(temp, "%lf", dval);   //Nim: changed %f to %lf

				break;

			default:

				break;

			}

		default:

			break;

		}

	}

	va_end(ap);             /* clean up when done    */

	return true;

}




//This will skip all white space and the comment delimited by # pairs in infile.
void skipblanks(FILE* infile)
{
	char ch;

	while (((ch = (char)getc(infile)) != EOF) && ((isspace(ch)) || (isCharComment(ch))))
	;


	if (!feof(infile))
		ungetc(ch, infile);
}





//This will return TRUE if eof is encountered and FALSE otherwise.  It is
//different from the feof library function in that it skips all white space
//and the comment delimited by # pairs before detecting the end of file condition.
int LDeof(FILE* infile)
{
	char ch;

	while (((ch = (char)getc(infile)) != EOF) && ((isspace(ch)) || (isCharComment(ch))))
	;


	if (!feof(infile))
	{
		ungetc(ch, infile);

		return false;
	}
	else
		return true;
}



float frand1()
{
	return (float)rand() / (RAND_MAX + 1);
}



//This will return a random integer between a and b inclusive.
int irand(int a, int b)
{
	return (int)(rand() % (b - a + 1) + a);
}




//This returns a random number normally distributed around 0.0.
float frandNorm()
{
	float a1 = 0, a2 = 0;

	while (a1 == 0)
	{
		a1 = frand1();
		a2 = frand1();
	}

	const float ret_val = (float)(sqrt(-0.75 * log(a1)) * sin(PI * PI * a2)) - 0.14f;

	return ret_val;
}




//This will swap the values of a and b.
void swap(float &a, float &b)
{
	float t;

	t = a;
	a = b;
	b = t;
}



#ifdef __HARVEST__

float gasdev()
{
	static int iset = 0;

	static float gset;

	if  (iset == 0) 
	{
		float rsq,v1,v2;

		do 
		{
			v1= 2.0f * frand1() - 1.0f;
			v2= 2.0f * frand1() - 1.0f;

			rsq = v1 * v1 + v2 * v2;

		}
		while (rsq >= 1.0 || rsq == 0.0);

		float fac = (float)sqrt(-2.0f * log(rsq) / rsq);

		gset = v1 * fac;

		iset = 1;

		return v2 * fac;

	} 
	else 
	{
		iset = 0;

		return gset;
	}

}



float gasdev(float mean, float sd)
{
	float gset = gasdev() * sd + mean;

	return gset;
}

#endif



int factorial(int k)
{
	if (k = 0)
		return 1;

	int temp = 1;

	for (int i = k; i >= 1; i--)
		temp *= i;

	return temp;
}



FILE* LDfopen(char * strFName, int type)
{
	if (type == 1)
		return fopen(strFName, "r");
	else if (type == 2)
		return fopen(strFName, "rb");
	else if (type == 3)
		return fopen(strFName, "w");
	else if (type == 4)
		return fopen(strFName, "a");
	else
		return NULL;
}





void LDfclose(FILE* f)
{
	fclose(f);
}




int LDfprintf0(FILE * fp, const char * fmt, ...)

{

	va_list ap;



	char *p, *ep, *sval, cval;



	int ival;

	long lval;

	float fval;

	double dval;





	va_start(ap, fmt);          // make ap point to 1st unnamed arg 





	int n = 0;

	p = strdup(fmt);

	while (*p != '\0')
	{
		if (*p != '%')
		{
			//Not special, just write out the char

			(void)putc(*p, fp);

			++n;

			++p;

		}

		else

		{

			ep = p + 1;

			switch (*ep)

			{

			case 'c':

				cval = va_arg(ap, char);

				n += fprintf(fp, "%c", cval);

				break;

			case 'd':

				ival = va_arg(ap, int);

				n += fprintf(fp, "%d", ival);

				break;

			case 'f':

				fval = va_arg(ap, float);

				n += fprintf(fp, "%f", fval);

				break;

			case 's':

				sval = va_arg(ap, char *);

				n += fprintf(fp, "%s", sval);

				break;

			case 'l':

				ep++;

				switch (*ep)

				{

				case 'd':

					lval = va_arg(ap, long);

					n += fprintf(fp, "%ld", lval);

					break;

				case 'f':

					dval = va_arg(ap, double);

					n += fprintf(fp, "%lf", dval);

					break;

				default:

					return -1;

				}

				break;

			default:

				return -1;

			}

			p = ep + 1;

		}

	} //end of while

	va_end(ap);  // clean up when done  

	return n;
}







int LDprintf(const char * fmt, ...)
{
	va_list ap;

	char *p, *ep, *sval, cval;

	int ival;

	long lval;

	float fval;

	double dval;

	va_start(ap, fmt);          // make ap point to 1st unnamed arg 

	int n = 0;

	p = strdup(fmt);

	while (*p != '\0')
	{
		if (*p != '%')
		{		
			(void)putchar(*p);//Not special, just write out the char

			++n;

			++p;
		}
		else
		{
			ep = p + 1;

			switch (*ep)
			{
			case 'c':

				cval = va_arg(ap, char);

				n += printf("%c", cval);

				break;

			case 'd':

				ival = va_arg(ap, int);

				n += printf("%d", ival);

				break;

			case 'f':

				fval = va_arg(ap, float);

				n += printf("%f", fval);

				break;

			case 's':

				sval = va_arg(ap, char *);

				n += printf("%s", sval);

				break;

			case 'l':

				ep++;

				switch (*ep)

				{

				case 'd':

					lval = va_arg(ap, long);

					n += printf("%ld", lval);

					break;

				case 'f':

					dval = va_arg(ap, double);

					n += printf("%lf", dval);

					break;

				default:

					return -1;

				}

				break;

			default:

				return -1;

			}

			p = ep + 1;

		}

	} //end of while

	va_end(ap);  // clean up when done  

	return n;
}
