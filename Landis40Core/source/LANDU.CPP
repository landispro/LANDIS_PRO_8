#define _CRT_SECURE_NO_WARNINGS

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "defines.h"
#include "system1.h"
#include "error.h"
#include "specatt.h"
#include "specatts.h"
#include "landu.h"


LANDUNIT::LANDUNIT()
{
	name = NULL;
	probReproduction = NULL;
	probReproductionOriginalBackup = NULL;
	speciesAttrs = NULL;

	ltID = 0;
	minShade = 0;
}



LANDUNIT::~LANDUNIT()
{
	if (name)
		delete[] name;

	if (probReproduction)
		delete[] probReproduction;

	if (probReproductionOriginalBackup)
		delete[] probReproductionOriginalBackup;
}



void LANDUNIT::attach(SPECIESATTRS *s)
{
	speciesAttrs = s;
}



//Read a land unit from a file.
void LANDUNIT::read(FILE* infile)
{
	if (speciesAttrs == NULL)
		errorSys("LANDUNIT::read(FILE*)-> No attaced species attributes.", STOP);

	char instring[25];

	if (fscanc(infile, "%s", instring) != 1)
		errorSys("Error reading in name from landtype file.", STOP);

	if (name != NULL)
		delete [] name;

	name = new char[strlen(instring) + 1];

	strcpy(name, instring);


	if (fscanc(infile, "%d", &minShade) != 1)
		errorSys("Error reading in minShade from landtpye file.", STOP);

	if (fscanc(infile, "%d", &initialLastWind) != 1)
		errorSys("Error reading in initialLastWind from landtype file.", STOP);

	if (fscanc(infile, "%f", &MaxRDArray[0]) != 1)
		errorSys("Error reading in maxRD from landtype file.", STOP);

	if (fscanc(infile, "%f", &MaxRDArray[1]) != 1)
		errorSys("Error reading in maxRD from landtype file.", STOP);

	if (fscanc(infile, "%f", &MaxRDArray[2]) != 1)
		errorSys("Error reading in maxRD from landtype file.", STOP);

	if (fscanc(infile, "%f", &MaxRDArray[3]) != 1)
		errorSys("Error reading in maxRD from landtype file.", STOP);

	MaxRD = MaxRDArray[3];

	const int specAtNum = speciesAttrs->number();

	if (probReproduction != NULL)
		delete [] probReproduction;
	probReproduction = new float[specAtNum];

	
	if (probReproductionOriginalBackup != NULL)
		delete[] probReproductionOriginalBackup;
	probReproductionOriginalBackup = new float[specAtNum];

	

	for (int i = 0; i < specAtNum; i++)  
	{
		if (fscanc(infile, "%f", &probReproduction[i]) != 1)
			errorSys("Error reading in probReproduction from landtype file.", STOP);
	}

	if (_stricmp(name, "empty") == 0 || _stricmp(name, "road") == 0)
		status = PASSIVE;
	else if (_stricmp(name, "water") == 0)
		status = WATER;
	else if (_stricmp(name, "wetland") == 0)
		status = WETLAND;
	else if (_stricmp(name, "bog") == 0)
		status = BOG;
	else if (_stricmp(name, "lowland") == 0)
		status = LOWLAND;
	else if (_stricmp(name, "nonforest") == 0)
		status = NONFOREST;
	else if (_stricmp(name, "grassland") == 0)
		status = GRASSLAND;
	else
		status = ACTIVE;
}




//Write a land unit to a file.
void LANDUNIT::write(FILE* outfile)
{
	if (speciesAttrs == NULL)
		errorSys("LANDUNIT::write(FILE*)-> No attaced species attributes.", STOP);

	const int specAtNum = speciesAttrs->number();

	fprintf(outfile, "%s ", minShade, name);

	for (int i = 0; i < specAtNum; i++)
		fprintf(outfile, "%2.2f ", probReproduction[i]);

	fprintf(outfile, "\n\n");
}




//Dump a land unit to the CRT.
void LANDUNIT::dump()
{
	if (speciesAttrs == NULL)
		errorSys("LANDUNIT::dump(FILE*)-> No attaced species attributes.", STOP);

	const int specAtNum = speciesAttrs->number();

	printf("Name:          %s\n", name);

	for (int i = 0; i < specAtNum; i++)
		printf("%s: %2.2f\n", (*speciesAttrs)(i + 1)->name, probReproduction[i]);
}






//Returns the probability of reproduction of the given species on the given land unit.  
//Species is referrenced by number.
float LANDUNIT::probRepro(int s)
{
	if (s > speciesAttrs->number() || s <= 0)
		errorSys("LANDUNIT::probRepro(int)-> Array bounds error.", STOP);

	return probReproduction[s - 1];
}



//Returns the probability of reproduction of the given species on the given land unit.  
//Species is referrenced by name.
float LANDUNIT::probRepro(char *s)
{
	const int specAtNum = speciesAttrs->number();

	for (int i = 0; i < specAtNum; i++)
	{
		if (strcmp((*speciesAttrs)(i + 1)->name, s) == 0)
			return probReproduction[i];
	}

	errorSys("LANDUNIT::probRepro(char*)-> Illegal species name.", STOP);

	return 0.0;
}



//Returns the probability of reproduction of the given species on the given
//land unit.  Species is referrenced by species record.
float LANDUNIT::probRepro(SPECIESATTR *s)
{
	const int specAtNum = speciesAttrs->number();

	for (int i = 0; i < specAtNum; i++)
	{
		if (strcmp((*speciesAttrs)(i + 1)->name, s->name) == 0)
			return probReproduction[i];
	}

	errorSys("LANDUNIT::probRepro(SPECIESATTR*)-> Illegal spec. attr.", STOP);

	return 0.0;
}



int LANDUNIT::Get_status()  { return status; }


int LANDUNIT::active()
{
	if (status == ACTIVE)
		return true;
	else
		return false;
}



int LANDUNIT::water()
{
	if (status == WATER)
		return true;
	else
		return false;
}





int LANDUNIT::lowland()
{
	if (status == WETLAND || status == BOG || status == LOWLAND || status == NONFOREST)
		return true;
	else
		return false;
}



