#include <string.h>

#include <stdlib.h>

#include <stdio.h>

#include "system1.h"

#include "error.h"

#include "landu.h"

#include "specatts.h"

#include "landus.h"



//Constructor.  N is equal to maximum array size for number of species.
LANDUNITS::LANDUNITS(int n)
{
	numLU = 0;
	currentLU = 0;
	maxLU = n;

	landUnits = new LANDUNIT[n];
	VectorIteration = NULL;
}



LANDUNITS::~LANDUNITS()
{
	if (landUnits)
		delete[] landUnits;

	if (VectorIteration)
		delete[] VectorIteration;
}



//Read set of land units from a file.
void LANDUNITS::read(FILE* infile)
{
	numLU = 0;

	while (!LDeof(infile))
	{
		assert(numLU < maxLU);
		
		landUnits[numLU++].read(infile);

		landUnits[numLU].ltID = numLU;
	}

	printf("Number of landUnits: %d\n", numLU);
}





//Write set of land units to a file.
void LANDUNITS::write(FILE* outfile)
{
	for (int i = 0; i < numLU; i++)
		landUnits[i].write(outfile);
}




//Dump set of land units to the CRT.
void LANDUNITS::dump()
{
	for (int i = 0; i < numLU; i++)
	{
		landUnits[i].dump();
		printf("===================================\n");
	}
}



//Attaches a set of species attributes to every land unit.  This must be performed following construction.
void LANDUNITS::attach(SPECIESATTRS *s)
{
	for (int i = 0; i < maxLU; i++)
		landUnits[i].attach(s);
}





//Referrence a land unit by land unit name.
LANDUNIT* LANDUNITS::operator () (char *name)
{
	for (int i = 0; i < numLU; i++)
	{
		if (strcmp(name, landUnits[i].name) == 0)
			return &landUnits[i];
	}

	return NULL;
}




//Referrence a land unit by land unit number.
LANDUNIT* LANDUNITS::operator () (int n)
{
	if (n > numLU || n < 0)
		return NULL;
	else
		return &landUnits[n];
}



//Referrence first land unit attribute.
LANDUNIT* LANDUNITS::first()
{
	currentLU = 0;

	if (numLU == 0)
		return NULL;
	else
		return &landUnits[0];
}


//Referrence next land unit attribute.
LANDUNIT* LANDUNITS::next()
{
	currentLU++;

	if (currentLU >= numLU)
		return NULL;
	else
		return &landUnits[currentLU];
}



int LANDUNITS::number() { return numLU; }




void LANDUNITS::initiateVariableVector(int NumofIter, int timestep_in, int num, int flag)
{
	timestep = timestep_in;

	Totaliteration = NumofIter;

	numSpecies = num;

	flagforSECFile = flag;

	ReprodBackup();
	
	VectorIteration = new char[NumofIter]();

	if (flag == 1)
	{
		if (timestep < 5)
		{
			int onceVarianceLast = 0;

			int flagForVariance;

			if (frand1() > 0.5)
				flagForVariance = -1;
			else
				flagForVariance = 1;

			for (int i = 0; i < Totaliteration;)
			{
				if (onceVarianceLast == 0)
				{
					const int randnumber = rand() % 2 + 3;

					onceVarianceLast = randnumber / timestep;

					if (onceVarianceLast == 0)
						onceVarianceLast = 1;

					flagForVariance = flagForVariance * (-1);
				}
				else
				{
					VectorIteration[i] = flagForVariance;

					i++;

					onceVarianceLast--;
				}
			}
		}
	}
	else if (flag == 2)
	{
		for (int i = 0; i < Totaliteration; i++)
		{
			if (frand1() > 0.5)
				VectorIteration[i] = 1;
			else
				VectorIteration[i] = -1;
		}
	}
	
}




void LANDUNITS::ReprodBackup()
{
	for (int i = 0; i < numLU; i++)
		for (int j = 0; j < numSpecies; j++)
			landUnits[i].probReproductionOriginalBackup[j] = landUnits[i].probReproduction[j];
}





void LANDUNITS::ReprodUpdate(int year)
{
	if (flagforSECFile == 3 || flagforSECFile == 0)
		return;

	const int consterm = 1 + VectorIteration[year - 1];
	const float fconst = consterm * timestep / 10.0f;

	for (int i = 0; i < numLU; i++)
		for (int j = 0; j < numSpecies; j++)		
			landUnits[i].probReproduction[j] = landUnits[i].probReproductionOriginalBackup[j] * fconst;
}
