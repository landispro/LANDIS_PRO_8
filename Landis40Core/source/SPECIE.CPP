#include <stdio.h>
#include <string.h>

#include "system1.h"
#include "agelist.h"
#include "specie.h"
#include "sites.h"




SPECIE::SPECIE()
{
	vegPropagules = 0;
	disPropagules = 0;
}


SPECIE::~SPECIE()	{}


//Clear all specie values.
void SPECIE::clear()
{
	vegPropagules = 0;
	disPropagules = 0;

	AGELIST::clear();
}


//Kill all trees bounded by l and h.
void SPECIE::kill(int l, int h)
{
	int num = getTimeStep();

	for (int i = l; i <= h; i += num)
		reset(i);
}



//Kill all trees bounded by l and h if and only if the age is present in a.
void SPECIE::kill(int l, int h, AGELIST a)
{
	int num = getTimeStep();

	for (int i = l; i <= h; i += num)
	{
		if (a.query(i))
			reset(i);
	}
}



//Read a specie from a file.
void SPECIE::read(FILE *infile)
{
	int a;

	fscanc(infile, "%d ", &a);

	vegPropagules = (short)a;

	AGELIST::read(infile);
}





void SPECIE::readTreeNum(FILE* infile, int specIndex)
{
	int a;

	fscanc(infile, "%d ", &a);

	vegPropagules = (short)a;

	AGELIST::readTreeNum(infile, specIndex);

	TreesFromVeg = 0;
}



//Write a specie to a file.
void SPECIE::write(FILE *outfile)
{
	fprintf(outfile, "%d ", vegPropagules);

	AGELIST::write(outfile);
}



//Dump a specie to the CRT.
void SPECIE::dump()
{
	printf("%d\t", vegPropagules);

	AGELIST::dump();
}



void SPECIE::initilizeDisPropagules(int maturity)
{
	if (oldest() >= maturity)
		disPropagules = 1;
}



void SPECIE::updateDispropagules(int maturity)
{
	if (oldest() >= maturity)
		disPropagules = true;
	else
		disPropagules = false;
}




//Adds trees to age ten.
void SPECIE::birth() { set(getTimeStep()); }

//Kills trees of a given age class.
void SPECIE::kill(int i) { reset(i); } 


void SPECIE::SetMatureTreeValue(int value)  { MatureTree = value; }

int SPECIE::GetMatureTreeValue() { return MatureTree; }
