#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>

#include <stdio.h>

#include <math.h>

#include <string.h>

#include <direct.h>

#include <errno.h>



#include "defines.h"

#include "system1.h"

#include "error.h"

#include "point.h"

#include "pile.h"

#include "map8.h"

#include "specie.h"

#include "specatts.h"

#include "landus.h"

#include "sites.h"

#include "Wind.h"



static const int lsclasses = 5;

static const float lifespan[5] = { 0.0, 0.2, 0.5, 0.7, 0.85 };
static const float  suscept[5] = { 0.0, 0.05, 0.50, 0.85, 1.0 };



static int red3[maxLeg] = { 0, 70, 0, 0, 0, 0, 0, 0, 200, 100, 255, 150, 200, 200, 255, 255 };

static int green3[maxLeg] = { 0, 70, 0, 0, 0, 150, 200, 255, 30, 200, 50, 50, 200, 0, 255 };

static int blue3[maxLeg] = { 0, 70, 125, 200, 255, 0, 100, 0, 30, 50, 50, 0, 0, 255 };



#define MaximumWindInterations 100000 //add By Qia on Oct 26 2009





WIND::WIND(char* strn, char* poutputDir, int gDLLMode)
{
	FILE* diFile;
	if ((diFile = LDfopen(strn, 1)) == NULL)
		errorSys("Wind input file not found.", STOP);

	while (!LDeof(diFile))
	{
		read(diFile);
		initLogFile(poutputDir);
	}

	fclose(diFile);
}



WIND::~WIND()  {}




//This will cause damage from the windthrow at POINT p.  It will return the
//actual number of cohorts killed.
int WIND::damage(LDPOINT p, int &cl)
{
	int numCohorts = 0;

	double tmpBiomass, tmpCarbon;

	float fr = frand1();

	float TmpMortality, DeadTree;
	
	LANDUNIT* l = pWindsites->locateLanduPt(p.y, p.x);
	
	float tmpDQ = 0;

	SITE* siteptr = (*pWindsites) (p.y, p.x);
	const int num_specie = siteptr->number();

	const int time_step = pWindsites->TimeStep;

	static const int cellsize_square = pWindsites->CellSize * pWindsites->CellSize;
	static const double tmpDQ_term2 = 3.1415926 / (4 * 10000 * cellsize_square * 30.48 * 30.48);

	for (int i = 1; i <= num_specie; i++)
	{
		SPECIESATTR* specatt_i = siteptr->specAtt(i);
		SPECIE*      lc_specie = siteptr->SpecieIndex(i);
		const int j_end  = specatt_i->longevity / time_step;

		if (specatt_i->SpType >= 0)
		{
			for (int j = 1; j <= j_end; j++)
			{
				const float lc_growrate = pWindsites->GetGrowthRates(i, j, l->ltID);

				tmpDQ += lc_growrate * lc_growrate * tmpDQ_term2 * lc_specie->getTreeNum(j, i);
			}
		}
	}

	m_pPDP->sTSLWind[p.y][p.x] = 0;

	pWindsites->BefStChg(p.y, p.x);

	const int numAttrs = pWindspeciesAttrs->number();

	for (int i = 1; i <= numAttrs; i++)
	{
		SPECIESATTR* specatt_i = siteptr->specAtt(i);
		SPECIE*      lc_specie = siteptr->SpecieIndex(i);
		
		const int j_end  = specatt_i->longevity / time_step;
		const int maxsproutage_d_step = specatt_i->maxSproutAge / time_step;
		
		lc_specie->TreesFromVeg = 0;

		if (specatt_i->SpType >= 0)
		{
			for (int j = 1; j <= j_end; j++)
			{
				const int local_tree_num = lc_specie->getTreeNum(j, i);
				const float lc_growrate = pWindsites->GetGrowthRates(i, j, l->ltID);
				const float TmpMortality = 0.01 / (1 + exp(3.25309 - 0.00072647 * tmpDQ + 0.01668809 * lc_growrate / 2.54));
				const int DeadTree = (int)(local_tree_num * TmpMortality * time_step / specatt_i->shadeTolerance);

				if (maxsproutage_d_step >= j)
					lc_specie->TreesFromVeg += DeadTree;
			}
		}

		SPECIE *s = (*(*pWindsites)(p.y, p.x))(i);

		int longevity = (*pWindspeciesAttrs)(i)->longevity;
		

		for (int j = pWindsites->TimeStep_Wind; j <= longevity; j += pWindsites->TimeStep_Wind)
		{
			if (s->query(j))
			{
				int k;

				for (k = lsclasses - 1; k >= 0; k--)
				{
					if (lifespan[k] < ((float)j / longevity)) 
						break;
				}


				if (k >= 0 && fr < suscept[k])
				{
					if (j < 320) 
					{
						s->kill(j);

						const float lc_biomass1 = pWindsites->GetBiomassData(specatt_i->BioMassCoef, 1);
						const float lc_biomass2 = pWindsites->GetBiomassData(specatt_i->BioMassCoef, 2);
						const float lc_growrate = pWindsites->GetGrowthRates(i, j / time_step, l->ltID);

						const double part1 = exp(lc_biomass1 + lc_biomass2 * log(lc_growrate));

						const int local_tree_num = lc_specie->getTreeNum(j / time_step, i);
						const double val = part1 * local_tree_num;

						tmpCarbon  = val;
						tmpBiomass = val / 1000.00;

						pWindsites->Wind70outputIncreaseBiomassvalue(p.y, p.x, tmpBiomass);				
						pWindsites->Wind70outputIncreaseCarbonvalue( p.y, p.x, tmpCarbon * specatt_i->CarbonCoEfficient);
					}

					numCohorts++;

					cl = max(cl, k);
				}
			}
		}
	}

	pWindsites->AftStChg(p.y, p.x);

	return numCohorts;
}






//This will activate a windthrow disturbance at row r and col c.  The size of
//the windthrow will be totArea where totArea is in number of pixels.
//The output map is contained in m and the output file is logfile.
int WIND::disturb(int row, int col, int totArea, MAP8 &m, FILE *logfile, double x, int itr)
{
	static PILE pile;

	LDPOINT p1, p2;

	MAP8 dist;       //This will be marked true if an area is disturbed.

	int cl = 0,          //Windthrow class.

		area = 0,      //Total area consumed to date.

		numSites = 0,  //Number of sites damaged.

		numCohorts = 0;//Number of cohorts damaged.

	double xx,       //verified new probability on each site

		siteProb; //probability on the site

	//Nim: next line was originally (rand()%4)+1. Note that 0 is not possible

	int windDirection = irand(1, 4); //0=none 1=N 2=E 3=S 4=W

	//Nim: next line was originally rand()%3

	//int windSpeed=irand(0,2);     //0=no wind 1=moderate 2=heavy



	int N, S, E, W;             // north/southing and east/westing

	//int snr=sites.numRows(), snc=sites.numColumns();

	int snr = pWindsites->numRows(), snc = pWindsites->numColumns();



	pile.reset();

	//dist.dim(sites.numRows(), sites.numColumns());

	dist.dim(snr, snc);

	dist.fill(false);

	if (row > snr || row == 0 || col > snc || col == 0)

	{

		printf("ILLEGAL INITIAL COORDINATE FOR WIND.\n");

		exit(0);

	}



	dist(row, col) = true;

	p1.x = col;

	p1.y = row;

	pile.push(p1);



	while (!pile.isEmpty() && area < totArea)

	{

		p2 = pile.pull();

		area++;

		int nCKilled = 0; //printf("wind line 116\n");


		if (pWindsites->locateLanduPt(p2.y, p2.x)->active())
		{
			siteProb = probCoef*(1.0 / (double)meanReturn)*(exp(1.0 / (double)meanReturn)) / 10 * pWindsites->TimeStep_Wind; //Bu, Rencang April 29, 2009

			xx = siteProb*(double)m_pPDP->sTSLWind[p2.y][p2.x] / (double)meanReturn;

			if (x <= xx)
				nCKilled = damage(p2, cl);
		}
		else
			nCKilled = 0;

		numCohorts += nCKilled; //printf("wind line 121\n");

		if (nCKilled)

		{

			numSites++;


			if (pWindsites->locateLanduPt(p2.y, p2.x)->active() ||

				pWindsites->locateLanduPt(p2.y, p2.x)->water() ||

				pWindsites->locateLanduPt(p2.y, p2.x)->lowland())

				m(p2.y, p2.x) = (unsigned char)(1 + cl);

		}


		switch (windDirection)

		{

			//   block formation to the NE SE SW NW

		case 1: N = 2; S = 1; E = 1; W = 1;  break; // North

		case 2: N = 1; S = 1; E = 2; W = 1;  break; // East

		case 3: N = 1; S = 2; E = 1; W = 1;  break; // South

		case 4: N = 1; S = 1; E = 1; W = 2;  break; // West

		}





		//// the below 4 ifs establish sites to put in a windthrow pile

		//// to be drawn at random

		if ((W != 0) && (p2.x - W > 0) && (!dist(p2.y, p2.x - W)))

		{

			p1.x = p2.x - W;

			p1.y = p2.y;

			dist(p1.y, p1.x) = true;

			pile.push(p1);

			if ((W > 1) && (!dist(p2.y, p2.x - W + 1)))

			{

				p1.x = p2.x - W + 1;

				dist(p1.y, p1.x) = true;

				pile.push(p1); //printf("wind line 149\n");

			}

		}

		if ((E != 0) && (p2.x + E <= snc) && (!dist(p2.y, p2.x + E)))

		{

			p1.x = p2.x + E;

			p1.y = p2.y;

			dist(p1.y, p1.x) = true;

			pile.push(p1);

			if ((E > 1) && (!dist(p2.y, p2.x + E - 1)))

			{

				p1.x = p2.x + E - 1;

				dist(p1.y, p1.x) = true;

				pile.push(p1); //printf("wind line 160\n");

			}

		}

		if ((S != 0) && (p2.y - S > 0) && (!dist(p2.y - S, p2.x)))

		{

			p1.x = p2.x;

			p1.y = p2.y - S;

			dist(p1.y, p1.x) = true;

			pile.push(p1);

			if ((S > 1) && (!dist(p2.y - S + 1, p2.x)))

			{

				p1.y = p2.y - S + 1;

				dist(p1.y, p1.x) = true;

				pile.push(p1); //printf("wind line 171\n");

			}

		}

		if ((N != 0) && (p2.y + N <= snr) && (!dist(p2.y + N, p2.x)))

		{

			p1.x = p2.x;

			p1.y = p2.y + N;

			dist(p1.y, p1.x) = true;

			pile.push(p1);

			if ((N > 1) && (!dist(p2.y + N - 1, p2.x)))

			{

				p1.y = p2.y + N - 1;

				dist(p1.y, p1.x) = true;

				pile.push(p1); //printf("wind line 182\n");

			}

		}

	}

	if (numCohorts)

	{

		fprintf(logfile, "%d, ", itr*pWindsites->TimeStep_Wind);

		fflush(logfile);

		fprintf(logfile, "'(%d,%d)',%d,", col, row, totArea);

		fflush(logfile);

		fprintf(logfile, "%d, %d\n", numSites, numCohorts);

		fflush(logfile);

	}

	return numSites;

}



///////////////////////////////////////////////////////////////////////////////

//			    EXTERNAL FUNCTIONS				     //

///////////////////////////////////////////////////////////////////////////////



void WIND::activate(int itr, int freq[], SITES* pS, SPECIESATTRS*	pWA, int cellSize, int randSeed, int numberOfIterations, int gLDDllMode) //Nim: added int freq[]

//This will simulate windthrow for a singular Landis iteration.



{

	FILE *logfile;         //Windthrow log file.

	double probPerSite;    //This is the probability of windthr. init. on a site.

	int  numSitesAct,      //Number of sites.

		siteSize;         //Square meters on a singular site.

	char str[256];          //Character string.

	double x;

	long numSites;

	float size, arealeft, amtdamaged = 0;

	int i, j;

	int max_sites_examined = 0;

	int snr, snc;

	double MWS;

	int count = 0;

	pWindsites = pS;

	pWindspeciesAttrs = pWA;



	m_dllmode = gLDDllMode;



	snr = pWindsites->numRows();

	snc = pWindsites->numColumns();

	//snr = pWindsites->getnRows();

	//snc = pWindsites->getnCols();



	numSites = snr*snc;

	m.dim(snr, snc);

	for (i = 1; i <= snr; i++)

		for (j = 1; j <= snc; j++)

		{

			//		  if (sites(i,j)->landUnit->active())

			if (pWindsites->locateLanduPt(i, j)->active())

				//original landis4.0: (*pWindsites)(i,j)->landUnit->active()

				//Changed By Qia on Oct 13 2008

				m(i, j) = 1;

			else

				m(i, j) = 0;

		}

	if (itr == 1)

		cummMap = m;



	//Setup windthrow parameters.

	//numSitesAct=(int)(sites.numActive()*breakOutCoef);

	numSitesAct = (int)(pWindsites->numActive()*breakOutCoef) / 10 * pWindsites->TimeStep_Wind; //Bu, Rencang April 29, 2009;;

	if (numSitesAct <= 0)

		numSitesAct = 1;

	//siteSize=parameters.cellSize*parameters.cellSize;

	siteSize = cellSize*cellSize;

	MWS = (double)meanWindSize / siteSize;

	probPerSite = probCoef*(1.0 / (double)meanReturn)*(exp(1.0 / (double)meanReturn)) / 10 * pWindsites->TimeStep_Wind; //Bu, Rencang April 29, 2009;

	//convert the integer in constructor to double



	float minSize = minWindSize / (float)siteSize;

	float maxSize = maxWindSize / (float)siteSize;

	//srand(parameters.randSeed+itr*4);

	//srand(randSeed+itr*4);//Commented By Qia on Dec 18 2008

	//sprintf(str,"%s/%s",parameters.outputDir,logFn);

	sprintf(str, "%s/%s", outputDir, logFn);

	if ((logfile = fopen(str, "a")) == NULL)

		errorSys("Error opening wind log file", STOP);



	//Perform windthrow.



	while ((max_sites_examined < numSitesAct) && (numSitesAct))

	{

		//<Add By Qia on Oct 26 2009>

		count++;

		if (count > MaximumWindInterations)

		{

			break;

		}

		//</Add By Qia on Oct 26 2009>

		while (windmode == 1)  //whole spectrum of disturbance

		{

			size = (double)(pow(10.0f, frandNorm())*(double)MWS*(double)sizeCoef / 100.0);

			if ((size<maxSize) && (size>minSize))

			{

				arealeft = size;

				break;

			}

		}



		while (windmode == 2) //mean disturbance

		{

			size = (double)(pow(10.0f, frandNorm())*(double)MWS*sizeCoef);

			arealeft = size;

			if ((size > (float)meanLower*numSites / 100.0) && (size < (float)meanUpper*numSites / 100.0))

				break;

		}



		while (windmode == 3)  //strong disturbance

		{

			size = (float)(pow(10.0f, frandNorm())*(double)MWS*sizeCoef);

			arealeft = size;

			if (size > (float)strong*numSites / 100.0)

				break;

		}



		while (windmode == 4) //light disturbance

		{

			size = (float)(pow(10.0f, frandNorm())*(double)MWS*sizeCoef);

			arealeft = size;

			if (size < (float)light*numSites / 100.0)

				break;

		}

		i = irand(1, snr); //Nim: was (rand()%snr)+1

		j = irand(1, snc); //Nim: was (rand()%snc)+1

		max_sites_examined++;

		x = frand1();

		//	if ((sites(i,j)->landUnit->active()) && (x<probPerSite))

		if ((pWindsites->locateLanduPt(i, j)->active()) && (x < probPerSite))

			//Original landis4.0: ((*pWindsites)(i,j)->landUnit->active()) && (x<probPerSite)

			//changed By Qia on Oct 13 2008

		{

			amtdamaged = (float)disturb(i, j, (int)size, m, logfile, x, itr);

			arealeft = arealeft - amtdamaged;

		}

		if (amtdamaged)

		{

			numSitesAct = (int)((float)numSitesAct*exp(-frand1()*frand1()));

			max_sites_examined = 0;

		}

	}



	//Add data to cummMap. && Update PDP with Wind Severity

	for (i = 1; i <= snr; i++)

		for (j = 1; j <= snc; j++)

		{



			if (m(i, j) >= 2)

			{

				cummMap(i, j) = m(i, j);

				m_pPDP->cWindSeverity[i][j] = m(i, j) - 2;		//add by Wei Li, 2004 Mar. 25

			}



		}



	if (((itr%freq[2]) == 0) && (freq[2] <= pWindsites->TimeStep_Wind) || (itr*pWindsites->TimeStep_Wind == freq[2]) && (freq[2] >= 0)) //Nim: added this line

	{                                                                     //Nim: added this line

		//Write map output file.

		sprintf(str, "Windthrow damage assessment for year %d.", itr*pWindsites->TimeStep_Wind);

		//    m.setHeader(sites.getHeader());

		m.setHeader(pWindsites->getHeader());



		m.rename(str);

		for (i = 0; i < maxLeg; i++)

			m.assignLeg(i, "");

		m.assignLeg(0, "NonActive");

		m.assignLeg(1, "NoSevereWinds");

		m.assignLeg(2, "Class 1");

		m.assignLeg(3, "Class 2");

		m.assignLeg(4, "Class 3");

		m.assignLeg(5, "Class 4");

		m.assignLeg(6, "Class 5");

		//    sprintf(str,"%s/%s%d",parameters.outputDir,yearlyFn,itr*10);

		sprintf(str, "%s/%s%d", outputDir, yearlyFn, itr*pWindsites->TimeStep_Wind);

		m.setCellSize(cellSize);

		double wAdfGeoTransform[6] = { 0.00, cellSize, 0.00, 600.00, 0.00, -cellSize };//*

		float nodata = 0;

		m.write(str, (int*)red3, (int*)green3, (int*)blue3, wAdfGeoTransform);

	} //Nim: added this line



	fclose(logfile);



	if (itr == numberOfIterations)
	{
		//Write cumulative map output file.

		sprintf(str, "Cumulative windthrow damage assessment."); //Nim: changed spelling

		//     cummMap.setHeader(sites.getHeader());

		cummMap.setHeader(pWindsites->getHeader());

		cummMap.rename(str);

		for (i = 0; i < maxLeg; i++)

			cummMap.assignLeg(i, "");

		cummMap.assignLeg(0, "NonActive");

		cummMap.assignLeg(1, "NoSevereWinds");

		cummMap.assignLeg(2, "Class 1");

		cummMap.assignLeg(3, "Class 2");

		cummMap.assignLeg(4, "Class 3");

		cummMap.assignLeg(5, "Class 4");

		cummMap.assignLeg(6, "Class 5");

		//	 sprintf(str,"%s/%s",parameters.outputDir,finalFn);

		sprintf(str, "%s/%s", outputDir, finalFn);

		cummMap.setCellSize(cellSize);

		double wAdfGeoTransform[6] = { 0.00, cellSize, 0.00, 600.00, 0.00, -cellSize };//*

		float nodata = 0;

		cummMap.write(str, (int*)red3, (int*)green3, (int*)blue3, wAdfGeoTransform);
	}
}






//This will read windthrow parameters into the model.
void WIND::read(FILE *infile)
{
	//added by Vera

	if (fscanc(infile, "%d", &windmode) != 1)
		errorSys("Error reading in windmode from disturbance file.", STOP);

	if (fscanc(infile, "%ld", &minWindSize) != 1)
		errorSys("Error reading in minWindSize from disturbance file.", STOP);

	if (fscanc(infile, "%ld", &maxWindSize) != 1)
		errorSys("Error reading in maxWindSize from disturbance file.", STOP);

	if (fscanc(infile, "%ld", &meanWindSize) != 1)
		errorSys("Error reading in meanWindSize from disturbance file.", STOP);

	if (fscanc(infile, "%ld", &meanReturn) != 1)
		errorSys("Error reading in meanReturn from disturbance file.", STOP);

	if (fscanc(infile, "%f", &breakOutCoef) != 1)
		errorSys("Error reading in breakOutCoef from disturbance file.", STOP);

	if (fscanc(infile, "%ld", &probCoef) != 1)
		errorSys("Error reading in probCoef from disturbance file.", STOP);

	if (fscanc(infile, "%ld", &sizeCoef) != 1)
		errorSys("Error reading in probCoef from disturbance file.", STOP);

	if (fscanc(infile, "%d", &meanLower) != 1)
		errorSys("Error reading in meanLower from disturbance file.", STOP);

	if (fscanc(infile, "%d", &meanUpper) != 1)
		errorSys("Error reading in meanUpper from disturbance file.", STOP);

	if (fscanc(infile, "%d", &strong) != 1)
		errorSys("Error reading in minFireSize from disturbance file.", STOP);

	if (fscanc(infile, "%d", &light) != 1)
		errorSys("Error reading in minFireSize from disturbance file.", STOP);

	if (fscanc(infile, "%s", &yearlyFn) != 1)
		errorSys("Error reading in yearlyFn from disturbance file.", STOP);

	if (fscanc(infile, "%s", &finalFn) != 1)
		errorSys("Error reading in finalFn from disturbance file.", STOP);

	if (fscanc(infile, "%s", &logFn) != 1)
		errorSys("Error reading in logFn from disturbance file.", STOP);
}




//This will write windthrow parameters out of the model.
void WIND::write(FILE *outfile)
{
	fprintf(outfile, "%ld\n", minWindSize);

	fprintf(outfile, "%ld\n", maxWindSize);

	fprintf(outfile, "%ld\n", meanWindSize);

	fprintf(outfile, "%ld\n", meanReturn);

	fprintf(outfile, "%f\n", breakOutCoef);

	fprintf(outfile, "%ld\n", probCoef);

	fprintf(outfile, "%ld\n", sizeCoef);

	fprintf(outfile, "%ld\n", meanLower);

	fprintf(outfile, "%ld\n", meanUpper);

	fprintf(outfile, "%ld\n", strong);

	fprintf(outfile, "%ld\n", light);

	fprintf(outfile, "%s\n", yearlyFn);

	fprintf(outfile, "%s\n", finalFn);

	fprintf(outfile, "%s\n", logFn);

}



void WIND::dump()

//This will dump windthrow parameters to the CRT screen.



{

	printf("%ld\n", minWindSize);

	printf("%ld\n", maxWindSize);

	printf("%ld\n", meanWindSize);

	printf("%ld\n", meanReturn);

	printf("%ld\n", breakOutCoef);

	printf("%ld\n", probCoef);

	printf("%ld\n", sizeCoef);

	printf("%ld\n", meanLower);

	printf("%ld\n", meanUpper);

	printf("%ld\n", strong);

	printf("%ld\n", light);

	printf("%s\n", yearlyFn);

	printf("%s\n", finalFn);

	printf("%s\n", logFn);

}



void WIND::initColors()

{

	red = red3;

	green = green3;

	blue = blue3;

}



void WIND::initLogFile(char* strOut)

//This will initialize the windthrow logfile.



{

	FILE *logfile;

	char str[100];



	//memcpy(outputDir, strOut, strlen(strOut)+1);

	sprintf(outputDir, "%s/%s", strOut, "Wind");

	//_mkdir(outputDir);

	//added by Vera, April 26, 2004

	if (_mkdir(outputDir) != 0)

	{

		if (errno != EEXIST)

			errorSys("Wind: Can't create the direcory", STOP);

	}



	sprintf(str, "%s/%s", outputDir, logFn);

	remove(str);

	if ((logfile = LDfopen(str, 4)) == NULL)

		errorSys("Output directory not found or error opening wind log file", STOP);

	fprintf(logfile, "Year, \"(x, y)\", Total_Area, Damag_Area, Damag_Cohorts\n");

	fclose(logfile);

}

