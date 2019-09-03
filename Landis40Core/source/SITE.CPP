#ifdef __HARVEST__

#include <assert.h>

#include "agelist.h"

#endif

#include <stdlib.h>

#include <stdio.h>

#include "system1.h"

#include "error.h"

#include "species.h"

#include "landus.h"

#include "site.h"




SITE::SITE(int n) : SPECIES(n) 
{ numofsites = 0; }


SITE::SITE() : SPECIES()
{ numofsites = 0; }



SITE::~SITE()  {}




//Read in site data.
void SITE::read(FILE *infile)
{
	SPECIES::read(infile);
}



//Write out site data.
void SITE::write(FILE *outfile)
{
	SPECIES::write(outfile);

	printf("\n\n");
}


//Dump site data to the CRT screen.
void SITE::dump()
{
	SPECIES::dump();

	printf("\n\n");
}

