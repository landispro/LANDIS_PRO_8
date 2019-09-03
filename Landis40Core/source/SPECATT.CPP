#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "system1.h"
#include "error.h"
#include "specatt.h"


SPECIESATTR::SPECIESATTR()
{
	name = NULL;

	longevity = 0;
	maturity = 0;
	shadeTolerance = 0;
	fireTolerance = 0;
	effectiveD = 0;
	maxD = 0;
	alpha = 0.0;
	vegProb = 0.0;
	maxSproutAge = 0;
	reclassCoef = 0.0;
}



SPECIESATTR::~SPECIESATTR()
{
	if (name)
		delete[] name;
}



//Read species attributes from a file.
void SPECIESATTR::read(FILE* infile, int cellSize)
{
	char instring[25];

	if (fscanc(infile, "%s", instring) != 1)
		errorSys("Read error in spec. attr. file.", STOP);


	name = new char[strlen(instring) + 1];

	strcpy(name, instring);



	if (fscanc(infile, "%d", &longevity) != 1)
		errorSys("Error reading in longevity from spec. attr. file.", STOP);

	if (fscanc(infile, "%d", &maturity) != 1)
		errorSys("Error reading in maturity from spec. attr. file.", STOP);

	if (fscanc(infile, "%d", &shadeTolerance) != 1)
		errorSys("Error reading in shadeTolerance from spec. attr. file.", STOP);

	if (fscanc(infile, "%d", &fireTolerance) != 1)
		errorSys("Error reading in fireTolerance from spec. attr. file.", STOP);

	if (fscanc(infile, "%d", &effectiveD) != 1)
		errorSys("Error reading in effectiveD from spec. attr. file.", STOP);

	if (fscanc(infile, "%d", &maxD) != 1)
		errorSys("Error reading in maxD from spec. attr. file.", STOP);

	if (fscanc(infile, "%f", &alpha) != 1)
		errorSys("Error reading in vegClass from spec. attr. file.", STOP);

	if (fscanc(infile, "%f", &vegProb) != 1)
		errorSys("Error reading in vegClass from spec. attr. file.", STOP);

	if (fscanc(infile, "%d", &MinSproutAge) != 1)
		errorSys("Error reading in MinSproutAge from spec. attr. file.", STOP);

	if (fscanc(infile, "%d", &maxSproutAge) != 1)
		errorSys("Error reading in maxSproutAge from spec. attr. file.", STOP);

	if (fscanc(infile, "%f", &reclassCoef) != 1)
		errorSys("Error reading in reclassCoef from spec. attr. file.", STOP);

	if (fscanc(infile, "%d", &SpType) != 1)
		errorSys("Error reading in SpType from spec. attr. file.", STOP);

	if (fscanc(infile, "%d", &BioMassCoef) != 1)
		errorSys("Error reading in BioMassCoef from spec. attr. file.", STOP);

	if (fscanc(infile, "%d", &MaxDQ) != 1)
		errorSys("Error reading in MaxDQ from spec. attr. file.", STOP);

	if (fscanc(infile, "%d", &SDImax) != 1)
		errorSys("Error reading in SDImax from spec. attr. file.", STOP);


	MaxAreaOfSTDTree = 10000.0f / SDImax;


	if (fscanc(infile, "%d", &TotalSeed) != 1)
		errorSys("Error reading in TotalSeed from spec. attr. file.", STOP);

	if (fscanc(infile, "%f", &CarbonCoEfficient) != 1)
		errorSys("Error reading in CarbonCoEfficient from spec. attr. file.", STOP);
}





//Write species attributes to a file.
void SPECIESATTR::write(FILE* outfile)
{
	fprintf(outfile, "%s\t", name);

	fprintf(outfile, "%d\t", longevity);

	fprintf(outfile, "%d\t", maturity);

	fprintf(outfile, "%d\t", shadeTolerance);

	fprintf(outfile, "%d\t", fireTolerance);

	fprintf(outfile, "%d\t", effectiveD);

	fprintf(outfile, "%d\t", maxD);

	fprintf(outfile, "%f\t", alpha);

	fprintf(outfile, "%f\t", vegProb);

	fprintf(outfile, "%d\t", maxSproutAge);

	fprintf(outfile, "%f\n", reclassCoef);
}


//deprecated
//Given a distance this will return a seeding probability for a species.
//float SPECIESATTR::prob(float x)
//{
//	double a = .95;
//	double b = 0.001;
//	double md = maxD;
//	double ed = effectiveD;
//
//	if (x <= ed) 
//		return (float)a;
//	else if (x > md) 
//		return (float)b;
//
//	const double alpha = log(a / b) / (md - ed);
//	const double prob = a * exp(-1 * alpha * (x - ed));
//
//	return (float)prob;
//}




void SPECIESATTR::dump()
{
	printf("name:           %s\n", name);

	printf("longevity:      %d\n", longevity);

	printf("maturity:       %d\n", maturity);

	printf("shadeTolerance: %d\n", shadeTolerance);

	printf("fireTolerance:  %d\n", fireTolerance);

	printf("effectiveD:     %d\n", effectiveD);

	printf("maxD:           %d\n", maxD);

	printf("alpha:        %f\n", alpha);

	printf("vegProb:        %f\n", vegProb);

	printf("maxSproutAge:   %d\n", maxSproutAge);

	printf("reclassCoef:    %f\n", reclassCoef);
}

