#include <stdlib.h>

#include <stdio.h>

#include "system1.h"

#include "error.h"

#include "specatt.h"

#include "specatts.h"

#include "specie.h"

#include "species.h"



int SPECIES::numSpec = 0;

SPECIESATTRS *SPECIES::speciesAttrs = NULL;







//This constructor can only be used on the first creation instance of class SPECIES.  
//It sets the number of different varieties of species in the model.
SPECIES::SPECIES(int n)
{
	if (numSpec != 0)
		errorSys("SPECIES::SPECIES(int)-> Number of species may only be set once at construction.", STOP);

	species = new SPECIE[n];

	numSpec = n;

	currentSpec = 0;
}





//Constructor.  Should be used on all instance constructions other than the 
//first.  This may be used on the first instance of construction but should 
//be followed by a call to setNumber before the second construction instance.
SPECIES::SPECIES()
{
	currentSpec = 0;

	if (numSpec == 0)
	{
		species = NULL;
	}
	else
	{
		species = new SPECIE[numSpec];

		for (int i = 0; i < numSpec; i++)
			species[i].AGELISTAllocateVector(i);
	}
}






SPECIES::~SPECIES()
{
	if (species)
		delete[] species;
}




//Sets the number of species.  Must be called after the first instance of 
//construction and before the second.
void SPECIES::setNumber(int n)
{
	if (numSpec != 0)
		errorSys("SPECIES::setNumber()-> Number of species already set.", STOP);

	numSpec = n;
}





//If all instances of class SPECIES are destructed then the reset function 
//may be called.  After it has been called instances may be constructed with
//a different number of species present.
void SPECIES::reset()
{
	numSpec = 0;
}



//Read a set of species from a file.
void SPECIES::read(FILE *infile)
{
	for (int i = 0; i < numSpec; i++)
	{
		species[i].readTreeNum(infile, i);

		species[i].initilizeDisPropagules(this->specAtt(i + 1)->maturity);
	}
}



//Write a set of species to a file.
void SPECIES::write(FILE *outfile)
{
	for (int i = 0; i < numSpec; i++)
	{
		species[i].write(outfile);

		fprintf(outfile, "\n");
	}
}





//Referrences a species by number.
SPECIE* SPECIES::operator () (int n)
{
	if (n > numSpec || n < 0)
		return NULL;
	else
	{
		currentSpec = (unsigned char)(n - 1);

		return &species[n - 1];
	}
}



//Referrence first species.
SPECIE* SPECIES::first()
{
	currentSpec = 0;

	return &species[currentSpec];
}




SPECIE* SPECIES::SpecieIndex(int n)
{
	n--;

	if (n >= numSpec || n < 0)
		errorSys("Specie Index Error out Bound", STOP);

	return &species[n];
}




SPECIE* SPECIES::current(int n)
{
	if (n > numSpec || n < 0)
		return NULL;

	currentSpec = (unsigned char)n;

	return &species[n];
}



//Referrence next species.
SPECIE* SPECIES::next()
{
	currentSpec++;

	if (currentSpec >= numSpec)
		return NULL;
	else
		return &species[currentSpec];
}




//Referrence the current species attribute.
SPECIESATTR* SPECIES::specAtt()
{
	if (speciesAttrs == NULL)
		errorSys("SPECIES::specAtt()-> Species attributes not attached to SPECIES.", STOP);

	if (currentSpec >= numSpec)
		return NULL;

	return (*speciesAttrs)(currentSpec + 1);
}



//Referrence the current species attribute.
SPECIESATTR* SPECIES::specAtt(int i)
{
	if (speciesAttrs == NULL)
		errorSys("SPECIES::specAtt()-> Species attributes not attached to SPECIES.", STOP);

	if (i > numSpec || i == 0)
		errorSys("specAtt out of bound", STOP);

	return (*speciesAttrs)(i);
}




//This attaches a set of species attributes to all instances of species.
void SPECIES::attach(SPECIESATTRS *s)  { speciesAttrs = s; }


//This returns the number of species.
int SPECIES::number() { return numSpec; }




//This returns the youngest age.
int SPECIES::youngest()
{
	int y = 10000;

	for (int i = 0; i < numSpec; i++)
		y = min(y, species[i].youngest());

	return y;
}



//This returns the oldest age among the youngest.
int SPECIES::oldest()
{
	int o = 0;

	for (int i = 0; i < numSpec; i++)
		o = max(o, species[i].oldest());

	return o;
}



SPECIES& SPECIES::operator=(const SPECIES& s)
{
	for (int i = 0; i < number(); i++)
	{
		species[i] = s.species[i];
		species[i].disPropagules = s.species[i].disPropagules;
		species[i].vegPropagules = s.species[i].vegPropagules;
	}

	currentSpec = 0;

	return *this;
}



//Write a set of species to the CRT.
void SPECIES::dump()
{
	if (speciesAttrs == NULL)
		errorSys("SPECIES::dump()-> Species attributes not attached to SPECIES.", STOP);

	for (int i = 0; i < numSpec; i++)
	{
		printf("%s\t", (*speciesAttrs)(i + 1)->name);

		species[i].dump();
	}
}

