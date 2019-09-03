#ifndef SITE_H
#define SITE_H

#include "species.h"
#include "landu.h"
#include "landus.h"



#ifdef LANDIS40CORE_EXPORTS
	#define DLLCLASS   __declspec( dllexport )
#else
	#define DLLCLASS   __declspec( dllimport )
#endif



class DLLCLASS SITE : public SPECIES
{
public:
	SITE(int n);                      //Constructor.      NOTE:  Apply same rules
	SITE();                           //Constructor.      as for class SPECIES.
	~SITE();                          //Destructor.

	void read(FILE*);                 //Read in all site data.
	void write(FILE*);                //Write out all site data.
	void dump();                      //Dump set of site data to the CRT.

	float RD;
	int HighestShadeTolerance;
	float MaxAge;
	int numofsites;
};



#undef DLLCLASS



#endif

