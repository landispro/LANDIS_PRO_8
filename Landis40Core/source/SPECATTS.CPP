#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "system1.h"
#include "error.h"
#include "specatt.h"
#include "specatts.h"




//Constructor.  N is equal to maximum array size for number of species. 
SPECIESATTRS::SPECIESATTRS(int n)
{
	numAttrs = 0;

	specAttrs = new SPECIESATTR[n];

	speces_maxD = NULL;
	specesefctD = NULL;

	currentAttr = 0;

	maxAttrs = n;
}



SPECIESATTRS::~SPECIESATTRS()
{
	if (specAttrs)
		delete[] specAttrs;

	if (speces_maxD)
		delete[] speces_maxD;

	if (specesefctD)
		delete[] specesefctD;
}




//Read set of species attributes from a file.
void SPECIESATTRS::read(FILE* infile, int cellSize)
{
	while (!LDeof(infile))
	{
		assert(numAttrs < maxAttrs);

		specAttrs[numAttrs++].read(infile, cellSize);		
	}


	printf("number of species: %d\n", numAttrs);

	speces_maxD = new int [numAttrs];
	specesefctD = new int [numAttrs];

	MaxDistanceofAllSpecs = 0;

	for (int i = 0; i < numAttrs; i++)
	{
		speces_maxD[i] = specAttrs[i].maxD;
		specesefctD[i] = specAttrs[i].effectiveD;

		if (specAttrs[i].maxD > MaxDistanceofAllSpecs)
			MaxDistanceofAllSpecs = specAttrs[i].maxD;
	}


	MaxShadeTolerance = 0;

	for (int i = 0; i < numAttrs; i++)
	{
		if (specAttrs[i].shadeTolerance >= MaxShadeTolerance && specAttrs[i].SpType >= 0)
			MaxShadeTolerance = specAttrs[i].shadeTolerance;
	}

}






int* SPECIESATTRS::get_speces_maxD()
{
	if (speces_maxD == NULL)
		exit(1);

	return speces_maxD;
}


int* SPECIESATTRS::get_specesefctD()
{
	if (specesefctD == NULL)
		exit(1);

	return specesefctD;
}




//Write set of species attributes to a file.
void SPECIESATTRS::write(FILE* outfile)
{
	for (int i = 0; i < numAttrs; i++)
		specAttrs[i].write(outfile);
}




void SPECIESATTRS::dump()
{
	for (int i = 0; i < numAttrs; i++)
	{
		specAttrs[i].dump();
		printf("==================================\n");
	}
}




//Referrence an attribute by species name.
SPECIESATTR* SPECIESATTRS::operator ()(char* name)
{
	for (int i = 0; i < numAttrs; i++)
	{
		if (strcmp(name, specAttrs[i].name) == 0)
			return &specAttrs[i];
	}

	return NULL;
}


//Return the number of sequence of a given species name
int SPECIESATTRS::current(char* name)
{
	for (int i = 0; i < numAttrs; i++)
	{
		if (strcmp(name, specAttrs[i].name) == 0)
			return i;
	}

	return NULL;
}





//Referrence an attribute by species number.
SPECIESATTR* SPECIESATTRS::operator ()(int n)
{
	if (n > numAttrs || n == 0)
		errorSys("Specie Attributes out bound", STOP);
	
	return &specAttrs[n - 1];
}




//Referrence first species attribute.
SPECIESATTR* SPECIESATTRS::first()
{
	currentAttr = 0;

	if (numAttrs == 0)
		return NULL;
	else
		return &specAttrs[currentAttr];
}



//Referrence next species attribute.
SPECIESATTR* SPECIESATTRS::next()
{
	currentAttr++;

	if (currentAttr >= numAttrs)
		return NULL;
	else
		return &specAttrs[currentAttr];
}



//Returns number of species.
int SPECIESATTRS::number()  { return numAttrs; }

