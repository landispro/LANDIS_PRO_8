#ifndef SYSTEM1_H

#define SYSTEM1_H



#include <stdio.h>
#include <cassert>


#undef min

#undef max



#ifdef LANDIS40CORE_EXPORTS
	#define DLLFUNC   __declspec( dllexport )
#else
	#define DLLFUNC   __declspec( dllimport )
#endif



#ifdef __cplusplus

extern "C" {  // only need to export C interface if

              // used by C++ source code

#endif



#define max(a,b) (((a)>(b)) ? (a):(b))
#define min(a,b) (((a)<(b)) ? (a):(b))



DLLFUNC	FILE* LDfopen(char * strFName, int);

DLLFUNC void LDfclose(FILE*);

DLLFUNC int fscanc(FILE *fp, char *fmt, ...);

DLLFUNC int LDfprintf0(FILE * fp,const char * fmt, ...);

DLLFUNC int LDprintf(const char *fmt, ...);


DLLFUNC int mfscanc(FILE *fp, char *fmt, ...);

DLLFUNC void skipblanks(FILE*);


DLLFUNC float frand1();


//This will return a random integer between a and b inclusive.
DLLFUNC int irand(int a, int b);




//This returns a random number normally distributed around 0.
DLLFUNC float frandNorm();



DLLFUNC int factorial(int k);



DLLFUNC void swap(float&, float&);



DLLFUNC int LDeof(FILE*);

//This will return TRUE if eof is encountered and FALSE otherwise.  It is

//different from the feof library function in that it skips all white space

//before detecting the end of file condition.




#ifdef __cplusplus

}

#endif



#ifdef LANDIS40CORE_EXPORTS
#endif
#undef DLLFUNC
#endif

