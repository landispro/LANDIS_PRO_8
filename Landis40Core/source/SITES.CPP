#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <list>

#include "system1.h"
#include "error.h"
#include "species.h"
#include "landus.h"
#include "site.h"
#include "sites.h"



#define MaximumAvailableSeeds 100000000



#define TPA		0
#define BA		1
#define Bio		2
#define Car		3
#define IV		4
#define Seeds	5
#define RDensity	6
#define DBH 7


#define DTN 0
#define DTNBA 1
#define DTNBIO 2
#define COUNT 3




//Constructor.  This will dimension the size of the map to i rows and j
SITES::SITES(int species, int i, int j)
{
	SPECIES junk(species);

	Pro0or401 = 1;

	map70 = new SITE[i * j]; 

	rows = i;

	columns = j;
}




//Constructor.  The maximum number of species will be set to species.
//The map dimensions will can be set by a call to SITES::dim after construction.
SITES::SITES()
{
	map70 = NULL;
	map_landtype = NULL;
	SeedRain = NULL;
	GrowthRates = NULL;
	MortalityRates = NULL;
	Volume = NULL;
	BiomassFireCost = NULL;
	CarbonFireCost = NULL;
	BiomassHarvestCost = NULL;
	CarbonHarvestCost = NULL;
	BiomassFuelCost = NULL;
	CarbonFuelCost = NULL;
	BiomassBDACost = NULL;
	CarbonBDACost = NULL;
	BiomassWindCost = NULL;
	CarbonWindCost = NULL;
	OutputGeneralFlagArray = NULL;
	OutputAgerangeFlagArray = NULL;
	SpeciesAgerangeArray = NULL;
	AgeDistStat_Year = NULL;
	AgeDistStat_AgeRange = NULL;
	speces_maxD = NULL;
	specesefctD = NULL;

	OutputModuleFlagArray = NULL;
	OutputAgeRangeModuleFlagArray = NULL;//add by houbin 10/18/2017

	Pro0or401 = 1;
	rows = 0;
	columns = 0;
	Fireflag = 0;
	Harvestflag = 0;
	Fuelflag = 0;
	BDAflag = 0;
	Windflag = 0;
	flagAgeRangeOutput = 0;
}




//Destructor.
SITES::~SITES()
{
	if (map70)
		delete[] map70;

	if (map_landtype)
		delete[] map_landtype;

	if (SeedRain)
		delete[] SeedRain;

	if(speces_maxD)
		delete[] speces_maxD;

	if(specesefctD)
		delete[] specesefctD;

	if (GrowthRates)
		delete[] GrowthRates;

	if (MortalityRates)
		delete[] MortalityRates;

	for (int i = 0; i < GrowthRates_file.size(); i++)
		delete[] GrowthRates_file.at(i);
	
	for (int i = 0; i < MortalityRates_file.size(); i++)
		delete[] MortalityRates_file.at(i);
	
	for (int i = 0; i < Volume_file.size(); i++)
		delete[] Volume_file.at(i);

	if (Volume)
		delete[] Volume;

	if (OutputGeneralFlagArray)
		delete OutputGeneralFlagArray;

	if (OutputAgerangeFlagArray)
		delete OutputAgerangeFlagArray;

	if (SpeciesAgerangeArray)
		delete SpeciesAgerangeArray;

	if (AgeDistStat_AgeRange)
		delete AgeDistStat_AgeRange;

	if (AgeDistStat_Year)
		delete AgeDistStat_Year;


	if (BiomassFireCost)
		delete BiomassFireCost;

	if (CarbonFireCost)
		delete CarbonFireCost;

	if (BiomassHarvestCost)
		delete BiomassHarvestCost;

	if (CarbonHarvestCost)
		delete CarbonHarvestCost;

	if (BiomassFuelCost)
		delete BiomassFuelCost;

	if (CarbonFuelCost)
		delete CarbonFuelCost;

	if (BiomassBDACost)
		delete BiomassBDACost;

	if (CarbonBDACost)
		delete CarbonBDACost;

	if (BiomassWindCost)
		delete BiomassWindCost;

	if (CarbonWindCost)
		delete CarbonWindCost;

	if (sitetouse)
		delete sitetouse;

	if (biomassData)
		delete biomassData;

	if (OutputModuleFlagArray)
		delete OutputModuleFlagArray;

	if (OutputAgeRangeModuleFlagArray)
		delete OutputAgeRangeModuleFlagArray;

	for (int i = 0; i < SortedIndex.size(); i++)
	{
		SITE *temp = SortedIndex.at(i);;
		delete temp;
	}
}






void SITES::Wind70outputdim()
{
	Windflag = 1;

	const int num = number();

	BiomassWindCost = new double[num]();
	CarbonWindCost  = new double[num]();
}


void SITES::Harvest70outputdim()
{
	Harvestflag = 1;

	const int num = number();
	
	BiomassHarvestCost = new double[num]();
	CarbonHarvestCost  = new double[num]();
}

void SITES::Fire70outputdim()
{
	Fireflag = 1;

	const int num = number();

	BiomassFireCost = new double[num]();
	CarbonFireCost  = new double[num]();
}

void SITES::Fuel70outputdim()
{
	Fuelflag = 1;

	const int num = number();
	
	BiomassFuelCost = new double[num]();
	CarbonFuelCost  = new double[num]();
}


void SITES::BDA70outputdim()
{
	BDAflag = 1;

	const int num = number();
	
	BiomassBDACost = new double[num]();
	CarbonBDACost  = new double[num]();
}



double SITES::Wind70outputGetBiomassvalue(int i, int j)
{
	if (i <= 0 || i > rows || j <= 0 || j > columns)
	{
		char err[80];
		sprintf(err,"SITES::operator() (int,int)-> (%d, %d) are illegal map coordinates",i,j);		
		errorSys(err,STOP);
	}

	const int x = (i - 1) * columns + j - 1;

	return BiomassWindCost[x];

}




double SITES::Harvest70outputGetBiomassvalue(int i, int j)
{
	if (i <= 0 || i > rows || j <= 0 || j > columns)
	{
		char err[80];
		sprintf(err,"SITES::operator() (int,int)-> (%d, %d) are illegal map coordinates",i,j);		
		errorSys(err,STOP);
	}

	const int x = (i - 1) * columns + j - 1;

	return BiomassHarvestCost[x];
}



double SITES::Fire70outputGetBiomassvalue(int i, int j)
{
	if (i <= 0 || i > rows || j <= 0 || j > columns)
	{
		char err[80];
		sprintf(err,"SITES::operator() (int,int)-> (%d, %d) are illegal map coordinates",i,j);		
		errorSys(err,STOP);
	}

	const int x = (i - 1) * columns + j - 1;

	return BiomassFireCost[x];
}




double SITES::Fuel70outputGetBiomassvalue(int i, int j)
{
	if (i <= 0 || i > rows || j <= 0 || j > columns)
	{
		char err[80];
		sprintf(err,"SITES::operator() (int,int)-> (%d, %d) are illegal map coordinates",i,j);		
		errorSys(err,STOP);
	}

	const int x = (i - 1) * columns + j - 1;

	return BiomassFuelCost[x];
}




double SITES::BDA70outputGetBiomassvalue(int i, int j)
{
	if (i <= 0 || i > rows || j <= 0 || j > columns)
	{
		char err[80];
		sprintf(err,"SITES::operator() (int,int)-> (%d, %d) are illegal map coordinates",i,j);		
		errorSys(err,STOP);
	}

	const int x = (i - 1) * columns + j - 1;

	return BiomassBDACost[x];
}




double SITES::Wind70outputGetCarbonvalue(int i, int j)
{
	if (i <= 0 || i > rows || j <= 0 || j > columns)
	{
		char err[80];
		sprintf(err,"SITES::operator() (int,int)-> (%d, %d) are illegal map coordinates",i,j);		
		errorSys(err,STOP);
	}

	const int x = (i - 1) * columns + j - 1;

	return CarbonWindCost[x];
}




double SITES::Harvest70outputGetCarbonvalue(int i, int j)
{
	if (i <= 0 || i > rows || j <= 0 || j > columns)
	{
		char err[80];
		sprintf(err,"SITES::operator() (int,int)-> (%d, %d) are illegal map coordinates",i,j);		
		errorSys(err,STOP);
	}

	const int x = (i - 1) * columns + j - 1;

	return CarbonHarvestCost[x];
}



double SITES::Fire70outputGetCarbonvalue(int i, int j)
{
	if (i <= 0 || i > rows || j <= 0 || j > columns)
	{
		char err[80];
		sprintf(err,"SITES::operator() (int,int)-> (%d, %d) are illegal map coordinates",i,j);		
		errorSys(err,STOP);
	}

	const int x = (i - 1) * columns + j - 1;

	return CarbonFireCost[x];
}



double SITES::Fuel70outputGetCarbonvalue(int i, int j)
{
	if (i <= 0 || i > rows || j <= 0 || j > columns)
	{
		char err[80];
		sprintf(err,"SITES::operator() (int,int)-> (%d, %d) are illegal map coordinates",i,j);		
		errorSys(err,STOP);
	}

	const int x = (i - 1) * columns + j - 1;

	return CarbonFuelCost[x];
}



double SITES::BDA70outputGetCarbonvalue(int i, int j)
{
	if (i <= 0 || i > rows || j <= 0 || j > columns)
	{
		char err[80];
		sprintf(err,"SITES::operator() (int,int)-> (%d, %d) are illegal map coordinates",i,j);		
		errorSys(err,STOP);
	}

	const int x = (i - 1) * columns + j - 1;

	return CarbonBDACost[x];
}






void SITES::Wind70outputIncreaseBiomassvalue(int i, int j, double value)
{
	if (i <= 0 || i > rows || j <= 0 || j > columns)
	{
		char err[80];
		sprintf(err,"SITES::operator() (int,int)-> (%d, %d) are illegal map coordinates",i,j);		
		errorSys(err,STOP);
	}

	const int x = (i - 1) * columns + j - 1;

	BiomassWindCost[x] += value;
}




void SITES::Harvest70outputIncreaseBiomassvalue(int i, int j, double value)
{
	if (i <= 0 || i > rows || j <= 0 || j > columns)
	{
		char err[80];
		sprintf(err,"SITES::operator() (int,int)-> (%d, %d) are illegal map coordinates",i,j);		
		errorSys(err,STOP);
	}

	const int x = (i - 1) * columns + j - 1;

	BiomassHarvestCost[x] += value;
}




void SITES::Fire70outputIncreaseBiomassvalue(int i, int j, double value)
{
	if (i <= 0 || i > rows || j <= 0 || j > columns)
	{
		char err[80];
		sprintf(err,"SITES::operator() (int,int)-> (%d, %d) are illegal map coordinates",i,j);		
		errorSys(err,STOP);
	}

	const int x = (i - 1) * columns + j - 1;

	BiomassFireCost[x] += value;
}




void SITES::Fuel70outputIncreaseBiomassvalue(int i, int j, double value)
{
	if (i <= 0 || i > rows || j <= 0 || j > columns)
	{
		char err[80];
		sprintf(err,"SITES::operator() (int,int)-> (%d, %d) are illegal map coordinates",i,j);		
		errorSys(err,STOP);
	}

	const int x = (i - 1) * columns + j - 1;

	BiomassFuelCost[x] += value;
}





void SITES::BDA70outputIncreaseBiomassvalue(int i, int j, double value)
{
	if (i <= 0 || i > rows || j <= 0 || j > columns)
	{
		char err[80];
		sprintf(err,"SITES::operator() (int,int)-> (%d, %d) are illegal map coordinates",i,j);		
		errorSys(err,STOP);
	}

	const int x = (i - 1) * columns + j - 1;

	BiomassBDACost[x] += value;
}




void SITES::Wind70outputIncreaseCarbonvalue(int i, int j, double value)
{
	if (i <= 0 || i > rows || j <= 0 || j > columns)
	{
		char err[80];
		sprintf(err,"SITES::operator() (int,int)-> (%d, %d) are illegal map coordinates",i,j);		
		errorSys(err,STOP);
	}

	const int x = (i - 1) * columns + j - 1;

	CarbonWindCost[x] += value;
}





void SITES::Harvest70outputIncreaseCarbonvalue(int i, int j, double value)
{
	if (i <= 0 || i > rows || j <= 0 || j > columns)
	{
		char err[80];
		sprintf(err,"SITES::operator() (int,int)-> (%d, %d) are illegal map coordinates",i,j);		
		errorSys(err,STOP);
	}

	const int x = (i - 1) * columns + j - 1;

	CarbonHarvestCost[x] += value;
}




void SITES::Fire70outputIncreaseCarbonvalue(int i, int j, double value)
{
	if (i <= 0 || i > rows || j <= 0 || j > columns)
	{
		char err[80];
		sprintf(err,"SITES::operator() (int,int)-> (%d, %d) are illegal map coordinates",i,j);		
		errorSys(err,STOP);
	}

	const int x = (i - 1) * columns + j - 1;

	CarbonFireCost[x] += value;
}




void SITES::Fuel70outputIncreaseCarbonvalue(int i, int j, double value)
{
	if (i <= 0 || i > rows || j <= 0 || j > columns)
	{
		char err[80];
		sprintf(err,"SITES::operator() (int,int)-> (%d, %d) are illegal map coordinates",i,j);		
		errorSys(err,STOP);
	}

	const int x = (i - 1) * columns + j - 1;

	CarbonFuelCost[x] += value;
}



void SITES::BDA70outputIncreaseCarbonvalue(int i, int j, double value)
{
	if (i <= 0 || i > rows || j <= 0 || j > columns)
	{
		char err[80];
		sprintf(err,"SITES::operator() (int,int)-> (%d, %d) are illegal map coordinates",i,j);		
		errorSys(err,STOP);
	}

	const int x = (i - 1) * columns + j - 1;

	CarbonBDACost[x];
}



void SITES::set_all_speces_Dist(int* speces_maxD_in, int* specesefctD_in)	
{
	speces_maxD = new int [specNum];
	specesefctD = new int [specNum];

	for(int i=0; i<specNum; i++)
	{
		speces_maxD[i] = speces_maxD_in[i];
		specesefctD[i] = specesefctD_in[i];
	}
}



//This will dimension the size of the map to i rows and j columns, and it
//will initialize the SPECIES class to species..
void SITES::dim(int species_num, int i, int j)
{
	if (map70 != NULL)
		delete[] map70;

	if (map_landtype != NULL)
		delete[] map_landtype;


	rows = i;
	columns = j;


	SITE temp;
	temp.setNumber(species_num);

	specNum = species_num;

	Pro0or401 = 1;

	map70 = new SITE [i * j];
	
	map_landtype = new LANDUNIT* [i * j];
	
	
	const int maxD_d_cell = MaxDistofAllSpec / CellSize;

	// SeedRain = new float* [species_num];
	// for (int i = 0; i < species_num; i++)
	// 	SeedRain[i] = new float[maxD_d_cell * maxD_d_cell];
	

	const int arr_size1 = species_num * (320 / TimeStep + 1);
	
	MortalityRates = new float[arr_size1];
	GrowthRates    = new float[arr_size1];
	Volume 		   = new float[arr_size1];

	const int arr_size2 = (species_num + 1) * NumTypes70Output;

	OutputGeneralFlagArray  = new int[arr_size2];
	OutputAgerangeFlagArray = new int[arr_size2];
	OutputModuleFlagArray   = new int[arr_size2];

	const int arr_size3 = species_num * 500 / TimeStep;

	OutputAgeRangeModuleFlagArray = new int[arr_size3];
	SpeciesAgerangeArray = new int[arr_size3];
	AgeDistStat_AgeRange = new int[arr_size3];
	AgeDistStat_Year     = new int[arr_size3];
	
	countWenjuanDebug = 0;
}





//This will referrence a site map by row i and column j.
SITE* SITES::operator() (int i, int j) //bsl
{
	if (i <= 0 || i > rows || j <= 0 || j > columns)
	{
		char err[80];
		sprintf(err,"SITES::operator() (int,int)-> (%d, %d) are illegal map coordinates",i,j);		
		errorSys(err,STOP);
	}

	const int x = (i - 1) * columns + j - 1;

	return &map70[x];
}





//This will read in a set of sites from infile.
void SITES::read(FILE* infile)
{
	SITE *p = map70;

	const int num = number();

	for (int i = 1; i <= num; i++)
	{
		if (feof(infile))
			errorSys("SITES::read(FILE*)-> Premature end of file.", STOP);

		p->read(infile);

		p++;
	}
}




//This will write out a set of sites to outfile.
void SITES::write(FILE* outfile)
{
	SITE *p = map70;
	
	const int num = number();

	for (int i = 1; i <= num; i++)
	{
		p->write(outfile);

		fprintf(outfile, "\n");

		p++;
	}
}




//This will dump a set of sites to the CRT screen.
void SITES::dump()
{
	for (int i = rows; i >= 1; i--) 
	{
		for (int j = 1; j <= columns; j++) 
		{
			// printf("site(%d,%d) =\n", i, j);

			(*this)(i, j)->dump();
		}
	}
}





//This will return the number of rows in the sites structure.
int SITES::numRows()  { return rows; }


//This will return the number of columns in the sites structure.
int SITES::numColumns()  { return columns; }


//This will return the total number of sites.
int SITES::number()
{
	static const int num = rows * columns;
	return num;
}


//This will return the number of sites with active land units on the map.
int SITES::numActive()
{	
	const int num = number() - 1;

	int count = 0;
	
	for (int i = 0; i <= num; i++)
	{
		if (map_landtype[i]->active())
			count++;
	}

	return count;
}



void SITES::setHeader(unsigned long dest[])
{
	for (int i = 0; i < 32; i++)
		header[i] = dest[i];

	xLLCorner = *(((float*)header) + 28);
	yLLCorner = *(((float*)header) + 29);
}



unsigned long* SITES::getHeader()
{
	return &header[0];
}




int SITES::inBounds(int r, int c) 
{
	if (r >= 1 && r <= rows && c >= 1 && c <= columns)
		return 1;
	else
		return 0;
}



int SITES::SITE_compare(int site1_x, int site1_y, int site2_x, int site2_y)
{
	const int x = (site1_x - 1) * columns + site1_y - 1;
	const int y = (site2_x - 1) * columns + site2_y - 1;

	SITE* site1 = map[x];
	SITE* site2 = map[y];

	const int result = SITE_compare(site1, site2);

	return result;
}




int SITES::SITE_compare(SITE* site1, SITE* site2)
{
	SPECIE *specie1 = site1->first();
	SPECIE *specie2 = site2->first();

	const int num = specie1->getAgeVectorNum();

	while (specie1 != NULL && specie2 != NULL)
	{
		const short vegPropagules1 = specie1->vegPropagules;
		const short vegPropagules2 = specie2->vegPropagules;

		if (vegPropagules1 > vegPropagules2)
			return 1;
		else if (vegPropagules1 < vegPropagules2)
			return 2;

		const short disPropagules1 = specie1->disPropagules;
		const short disPropagules2 = specie2->disPropagules;

		if (disPropagules1 > disPropagules2)
			return 1;
		else if (disPropagules1 < disPropagules2)
			return 2;

		for (int i = 0; i < num; i++)
		{
			const int AgeVector1 = specie1->getAgeVector(i);
			const int AgeVector2 = specie2->getAgeVector(i);

			if (AgeVector1 > AgeVector2)
				return 1;
			else if (AgeVector1 < AgeVector2)
				return 2;
		}

		specie1 = site1->next();
		specie2 = site2->next();
	}

	return 0;

}



void SITES::fillinSitePt(int i, int j, SITE *site)
{
	const int x = (i - 1) * columns + j - 1;

	map[x] = site;
}





SITE* SITES::locateSitePt(int i, int j)
{
	const int x = (i - 1) * columns + j - 1;

	return map[x];
}





void SITES::fillinLanduPt(int i, int j, LANDUNIT *landUnitPt)
{
	assert(i >= 1 && i <= rows && j >= 1 && j <= columns);

	const int x = (i - 1) * columns + j - 1;

	map_landtype[x] = landUnitPt;
}





//find the landtype
LANDUNIT* SITES::locateLanduPt(int i, int j)
{
	assert(i >= 1 && i <= rows && j >= 1 && j <= columns);

	const int x = (i - 1) * columns + j - 1;

	return map_landtype[x];
}




//When a site disappears, delete it
int SITES::SITE_delete(int pos_sortIndex, SITE *site, int i, int j)
{
	std::vector<SITE *>::iterator temp_sitePtr;

	const int x = (i - 1) * columns + j - 1;

	if (site != SortedIndex.at(pos_sortIndex))
		return 0;

	if (site != map[x])
		return 0;

	delete site;

	temp_sitePtr = SortedIndex.begin();

	SortedIndex.erase(temp_sitePtr + pos_sortIndex);

	return 1;
}





//use babble algorithm to sort the initial site list array
void SITES::SITE_sort()
{
	for (size_t i = SortedIndex.size() - 1; i > 0; i--)
	{
		for (int j = 0; j <= i - 1; j++)
		{
			SITE* site1 = SortedIndex.at(j);
			SITE* site2 = SortedIndex.at(j + 1);

			if (SITE_compare(site1, site2) == 1)
			{
				SITE* temp = SortedIndex.at(j);
				SortedIndex.at(j) = SortedIndex.at(j + 1);
				SortedIndex.at(j + 1) = temp;
			}
		}
	}
}







//According to shade-rule and RD-rule determine species succession on sites, No Return Value
// void SITES::SiteDynamics(const int RDflag, const int Row, const int Col)
void SITES::SiteDynamics(SiteDynmParam* param)
{
	const int RDflag = param->RDflag;
	const int Row = param->row;
	const int Col = param->col;
	const PDP* ppdp = param->Ppdp;
	
	SITE* siteptr = SITES::operator()(Row, Col);

	LANDUNIT* l = SITES::locateLanduPt(Row, Col);


	if(RDflag == 4)
	{
		SITES::NaturalMortality(siteptr, Row, Col, 0, ppdp);//kill all ages of trees

		GetRDofSite(Row, Col);
		
		SITES::Selfthinning(siteptr, l, Row, Col, ppdp);
	}
	else
	{
		SeedGermination(siteptr, l, RDflag);

		GetRDofSite(Row, Col);

		if (RDflag >= 2)
			SITES::NaturalMortality(siteptr, Row, Col, 0, ppdp);//kill all ages of trees
		else
			SITES::NaturalMortality(siteptr, Row, Col, 1, ppdp);//kill the youngest of trees
	}

	GetRDofSite(Row, Col);
}






//Bubble sort function 
void SITES::ListbubbleSort(vector<double>& AreaList, vector<int>& SpecIndexArr, vector<int>& AgeIndexArr)
{
	for (size_t i = AreaList.size() - 1; i > 0; i--)
	{
		for (int j = 1; j <= i; j++)
		{
			if (AreaList.at(j - 1) > AreaList.at(j))
			{
				std::swap(    AreaList[j - 1],     AreaList[j]);
				std::swap(SpecIndexArr[j - 1], SpecIndexArr[j]);
				std::swap( AgeIndexArr[j - 1],  AgeIndexArr[j]);
			}
		}
	}
}







void SITES::Selfthinning(SITE* siteptr, LANDUNIT *l, int row, int col, const PDP* ppdp)
{
	const float TargetRD = siteptr->RD - l->MaxRD;

	if (TargetRD <= 0) 
		return;
	
	double Area_tobeThin = TargetRD * CellSize * CellSize;

	const int num_specie = siteptr->number();

	int* quaterPercent = new int[specNum * 5];

	for (int i = 1; i <= num_specie; i++)
	{
		const int index = (i - 1) * 5;
		const int consterm = siteptr->specAtt(i)->longevity / TimeStep;

		quaterPercent[index + 0] = 0;
		quaterPercent[index + 1] = int(consterm * 0.25);
		quaterPercent[index + 2] = int(consterm * 0.50);
		quaterPercent[index + 3] = int(consterm * 0.75);
		quaterPercent[index + 4] = consterm;
	}

	vector<double> AreaList;
	vector<int> SpecIndexArray;
	vector<int> AgeIndexArray;

	static const float thin_percent[5] = {0.95f, 0.90f, 0.85f, 0.80f, 0.75f};

	for (int j = 0; j < 4; j++)
	{
		AreaList.clear();
		SpecIndexArray.clear();
		AgeIndexArray.clear();


		if (j == 0)
		{
			double subArea_tobeThin = 0.0;

			for (int spec_i = 1; spec_i <= num_specie; spec_i++)
			{
				SPECIESATTR* specatt_i = siteptr->specAtt(spec_i);
				SPECIE* 	 lc_specie = siteptr->SpecieIndex(spec_i);

				const int lc_index = (spec_i - 1) * 5 + j;
				const int tolrance = specatt_i->shadeTolerance;

				const float max_area = specatt_i->MaxAreaOfSTDTree;

				for (int age_i = quaterPercent[lc_index]+1; age_i <= quaterPercent[lc_index + 1]; age_i++)
				{
					const float lc_growrate = SITES::GetGrowthRates(spec_i, age_i, l->ltID);
					const unsigned int lc_treenum  = lc_specie->getTreeNum(age_i, spec_i);

					const double consterm = pow((lc_growrate / 25.4), 1.605) * max_area;

					subArea_tobeThin += thin_percent[tolrance - 1] * consterm * lc_treenum;

					AreaList.push_back(consterm);
					SpecIndexArray.push_back(spec_i);
					AgeIndexArray.push_back(age_i);
				}
			}

			ListbubbleSort(AreaList, SpecIndexArray, AgeIndexArray);

			if (subArea_tobeThin >= Area_tobeThin)
			{
				for (int i = 0; i < AreaList.size(); i++)
				{
					const int spec_i = SpecIndexArray.at(i);
					const int ages_i =  AgeIndexArray.at(i);

					SPECIE* lc_specie = siteptr->SpecieIndex(spec_i);

					const unsigned int lc_treenum = lc_specie->getTreeNum(ages_i, spec_i);

					if (lc_treenum > 0)
					{
						const float lc_thin_percent = thin_percent[siteptr->specAtt(spec_i)->shadeTolerance - 1];

						const double tempAreaInvolveTreeNum = AreaList.at(i) * lc_treenum * lc_thin_percent;

						if (tempAreaInvolveTreeNum > Area_tobeThin)
						{
							const int treesToremove = (int)min(lc_treenum, Area_tobeThin / AreaList.at(i));

							const double treesLeft = lc_treenum - treesToremove;

							assert(treesLeft >= 0 && treesLeft <= UINT_MAX);

							const float lc_growrate = GetGrowthRates(spec_i, ages_i, l->ltID);
							const int lc_biomasscof = siteptr->specAtt(spec_i)->BioMassCoef;

							const float tmp_DTNBA = lc_growrate * lc_growrate / 40000 * 3.1415926 * treesToremove;
							const float tmp_DTNBIO = exp(GetBiomassData(lc_biomasscof, 1) + GetBiomassData(lc_biomasscof, 2) * log(lc_growrate)) * treesToremove / 1000.00;
							
							lc_specie->setTreeNum(ages_i, spec_i, (unsigned int)treesLeft);

							if (ppdp->TrackFlag == 1)
							{
								ppdp->cDeadTreeNum[row - 1][col - 1][spec_i - 1] += treesToremove;
								ppdp->cDeadTreeBA[row - 1][col - 1][spec_i - 1] += tmp_DTNBA;
								ppdp->cDeadTreeBio[row - 1][col - 1][spec_i - 1] += tmp_DTNBIO;

								const int lc_output_agerange_limit = GetOutputAgeRangeModuleFlagArray(spec_i - 1, COUNT);
								const int agei_m_timestep = ages_i * TimeStep;

								for (int agerange_index = 1; agerange_index <= lc_output_agerange_limit; agerange_index++)					//add by houbin 11/3/2017
								{
									const int lc_output_agerange_lowlimit = GetOutputAgeRangeModuleFlagArray(spec_i - 1, agerange_index, 0);
									const int lc_output_agerange_uprlimit = GetOutputAgeRangeModuleFlagArray(spec_i - 1, agerange_index, 1);

									if (lc_output_agerange_lowlimit < agei_m_timestep && agei_m_timestep <= lc_output_agerange_uprlimit)
									{
										const int lc_third_id = (agerange_index - 1) * specNum + spec_i - 1;

										ppdp->cDeadTreeNum_spec_agerange[row - 1][col - 1][lc_third_id] += treesToremove;
										ppdp->cDeadTreeBA_spec_agerange[row - 1][col - 1][lc_third_id] += tmp_DTNBA;
										ppdp->cDeadTreeBio_spec_agerange[row - 1][col - 1][lc_third_id] += tmp_DTNBIO;
									}
								}
							}
							

							

							Area_tobeThin -= tempAreaInvolveTreeNum;							

							delete[] quaterPercent;

							return;
						}
						else
						{
							const double treesLeft = lc_treenum * (1 - lc_thin_percent);
							assert(treesLeft >= 0 && treesLeft <= UINT_MAX);

							const int treesToremove = (int)(lc_treenum - treesLeft);

							const float lc_growrate = GetGrowthRates(spec_i, ages_i, l->ltID);
							const int lc_biomasscof = siteptr->specAtt(spec_i)->BioMassCoef;

							const float tmp_DTNBA = lc_growrate * lc_growrate / 40000 * 3.1415926 * treesToremove;
							const float tmp_DTNBIO = exp(GetBiomassData(lc_biomasscof, 1) + GetBiomassData(lc_biomasscof, 2) * log(lc_growrate)) * treesToremove / 1000.00;

							lc_specie->setTreeNum(ages_i, spec_i, (unsigned int)treesLeft);

							if (ppdp->TrackFlag == 1)
							{
								ppdp->cDeadTreeNum[row - 1][col - 1][spec_i - 1] += treesToremove;
								ppdp->cDeadTreeBA[row - 1][col - 1][spec_i - 1] += tmp_DTNBA;
								ppdp->cDeadTreeBio[row - 1][col - 1][spec_i - 1] += tmp_DTNBIO;

								const int lc_output_agerange_limit = GetOutputAgeRangeModuleFlagArray(spec_i - 1, COUNT);
								const int agei_m_timestep = ages_i * TimeStep;

								for (int agerange_index = 1; agerange_index <= lc_output_agerange_limit; agerange_index++)					//add by houbin 11/3/2017
								{
									const int lc_output_agerange_lowlimit = GetOutputAgeRangeModuleFlagArray(spec_i - 1, agerange_index, 0);
									const int lc_output_agerange_uprlimit = GetOutputAgeRangeModuleFlagArray(spec_i - 1, agerange_index, 1);

									if (lc_output_agerange_lowlimit < agei_m_timestep && agei_m_timestep <= lc_output_agerange_uprlimit)
									{
										const int lc_third_id = (agerange_index - 1) * specNum + spec_i - 1;

										ppdp->cDeadTreeNum_spec_agerange[row - 1][col - 1][lc_third_id] += treesToremove;
										ppdp->cDeadTreeBA_spec_agerange[row - 1][col - 1][lc_third_id] += tmp_DTNBA;
										ppdp->cDeadTreeBio_spec_agerange[row - 1][col - 1][lc_third_id] += tmp_DTNBIO;
									}
								}
							}

							

							Area_tobeThin -= tempAreaInvolveTreeNum;
						}
					}
				}
			}
			else
			{
				for (int i = 0; i < AreaList.size(); i++)
				{
					const int spec_i = SpecIndexArray.at(i);
					const int ages_i =  AgeIndexArray.at(i);

					SPECIE* lc_specie = siteptr->SpecieIndex(spec_i);

					const unsigned int lc_treenum = lc_specie->getTreeNum(ages_i, spec_i);

					if (lc_treenum > 0)
					{
						const float lc_thin_percent = thin_percent[siteptr->specAtt(spec_i)->shadeTolerance - 1];

						const double tempAreaInvolveTreeNum = AreaList.at(i) * lc_treenum * lc_thin_percent;

						const double treesLeft = lc_treenum * (1 - lc_thin_percent);
						assert(treesLeft >= 0 && treesLeft <= UINT_MAX);

						const int treesToremove = (int)(lc_treenum - treesLeft);

						const float lc_growrate = GetGrowthRates(spec_i, ages_i, l->ltID);
						const int lc_biomasscof = siteptr->specAtt(spec_i)->BioMassCoef;

						const float tmp_DTNBA = lc_growrate * lc_growrate / 40000 * 3.1415926 * treesToremove;
						const float tmp_DTNBIO = exp(GetBiomassData(lc_biomasscof, 1) + GetBiomassData(lc_biomasscof, 2) * log(lc_growrate)) * treesToremove / 1000.00;

						lc_specie->setTreeNum(ages_i, spec_i, (unsigned int)treesLeft);

						if (ppdp->TrackFlag==1)
						{
							ppdp->cDeadTreeNum[row - 1][col - 1][spec_i - 1] += treesToremove;
							ppdp->cDeadTreeBA[row - 1][col - 1][spec_i - 1] += tmp_DTNBA;
							ppdp->cDeadTreeBio[row - 1][col - 1][spec_i - 1] += tmp_DTNBIO;

							const int lc_output_agerange_limit = GetOutputAgeRangeModuleFlagArray(spec_i - 1, COUNT);
							const int agei_m_timestep = ages_i * TimeStep;

							for (int agerange_index = 1; agerange_index <= lc_output_agerange_limit; agerange_index++)					//add by houbin 11/3/2017
							{
								const int lc_output_agerange_lowlimit = GetOutputAgeRangeModuleFlagArray(spec_i - 1, agerange_index, 0);
								const int lc_output_agerange_uprlimit = GetOutputAgeRangeModuleFlagArray(spec_i - 1, agerange_index, 1);

								if (lc_output_agerange_lowlimit < agei_m_timestep && agei_m_timestep <= lc_output_agerange_uprlimit)
								{
									const int lc_third_id = (agerange_index - 1) * specNum + spec_i - 1;

									ppdp->cDeadTreeNum_spec_agerange[row - 1][col - 1][lc_third_id] += treesToremove;
									ppdp->cDeadTreeBA_spec_agerange[row - 1][col - 1][lc_third_id] += tmp_DTNBA;
									ppdp->cDeadTreeBio_spec_agerange[row - 1][col - 1][lc_third_id] += tmp_DTNBIO;
								}
							}
						}
						

						Area_tobeThin -= tempAreaInvolveTreeNum;
					}
				}
			}
		}

		AreaList.clear();
		SpecIndexArray.clear();
		AgeIndexArray.clear();


		if (j == 1)
		{
			double subArea_tobeThin = 0.0;

			for (int spec_i = 1; spec_i <= num_specie; spec_i++)
			{
				SPECIESATTR* specatt_i = siteptr->specAtt(spec_i);
				SPECIE* 	 lc_specie = siteptr->SpecieIndex(spec_i);

				const int lc_index = (spec_i - 1) * 5 + j;
				const int tolrance = specatt_i->shadeTolerance;

				const float max_area = specatt_i->MaxAreaOfSTDTree;

				for (int age_i = quaterPercent[lc_index]+1; age_i <= quaterPercent[lc_index + 1]; age_i++)
				{
					const float lc_growrate = GetGrowthRates(spec_i, age_i, l->ltID);
					const unsigned int lc_treenum  = lc_specie->getTreeNum(age_i, spec_i);

					const double consterm = pow((lc_growrate / 25.4), 1.605) * max_area;

					subArea_tobeThin += thin_percent[tolrance - 1] * consterm * lc_treenum;

					AreaList.push_back(consterm);
					SpecIndexArray.push_back(spec_i);
					AgeIndexArray.push_back(age_i);
				}
			}

			ListbubbleSort(AreaList, SpecIndexArray, AgeIndexArray);

			if (subArea_tobeThin >= Area_tobeThin)
			{
				for (int i = 0; i < AreaList.size(); i++)
				{
					const int spec_i = SpecIndexArray.at(i);
					const int ages_i =  AgeIndexArray.at(i);

					SPECIE* lc_specie = siteptr->SpecieIndex(spec_i);

					const unsigned int lc_treenum = lc_specie->getTreeNum(ages_i, spec_i);

					if (lc_treenum > 0)
					{
						const float lc_thin_percent = thin_percent[siteptr->specAtt(spec_i)->shadeTolerance - 1];

						const double tempAreaInvolveTreeNum = AreaList.at(i) * lc_treenum * lc_thin_percent / 2;

						if (tempAreaInvolveTreeNum > Area_tobeThin)
						{
							const int treesToremove = (int)min(lc_treenum, Area_tobeThin / AreaList.at(i));

							const double treesLeft = lc_treenum - treesToremove;

							assert(treesLeft >= 0 && treesLeft <= UINT_MAX);

							const float lc_growrate = GetGrowthRates(spec_i, ages_i, l->ltID);
							const int lc_biomasscof = siteptr->specAtt(spec_i)->BioMassCoef;

							const float tmp_DTNBA = lc_growrate * lc_growrate / 40000 * 3.1415926 * treesToremove;
							const float tmp_DTNBIO = exp(GetBiomassData(lc_biomasscof, 1) + GetBiomassData(lc_biomasscof, 2) * log(lc_growrate)) * treesToremove / 1000.00;

							lc_specie->setTreeNum(ages_i, spec_i, (unsigned int)treesLeft);

							if (ppdp->TrackFlag==1)
							{
								ppdp->cDeadTreeNum[row - 1][col - 1][spec_i - 1] += treesToremove;
								ppdp->cDeadTreeBA[row - 1][col - 1][spec_i - 1] += tmp_DTNBA;
								ppdp->cDeadTreeBio[row - 1][col - 1][spec_i - 1] += tmp_DTNBIO;

								const int lc_output_agerange_limit = GetOutputAgeRangeModuleFlagArray(spec_i - 1, COUNT);
								const int agei_m_timestep = ages_i * TimeStep;

								for (int agerange_index = 1; agerange_index <= lc_output_agerange_limit; agerange_index++)					//add by houbin 11/3/2017
								{
									const int lc_output_agerange_lowlimit = GetOutputAgeRangeModuleFlagArray(spec_i - 1, agerange_index, 0);
									const int lc_output_agerange_uprlimit = GetOutputAgeRangeModuleFlagArray(spec_i - 1, agerange_index, 1);

									if (lc_output_agerange_lowlimit < agei_m_timestep && agei_m_timestep <= lc_output_agerange_uprlimit)
									{
										const int lc_third_id = (agerange_index - 1) * specNum + spec_i - 1;

										ppdp->cDeadTreeNum_spec_agerange[row - 1][col - 1][lc_third_id] += treesToremove;
										ppdp->cDeadTreeBA_spec_agerange[row - 1][col - 1][lc_third_id] += tmp_DTNBA;
										ppdp->cDeadTreeBio_spec_agerange[row - 1][col - 1][lc_third_id] += tmp_DTNBIO;
									}
								}
							}
							

							Area_tobeThin -= tempAreaInvolveTreeNum;

							delete[] quaterPercent;

							return;
						}
						else
						{
							const double treesLeft = lc_treenum * (1 - lc_thin_percent / 2);

							assert(treesLeft >= 0 && treesLeft <= UINT_MAX);

							const int treesToremove = (int)(lc_treenum - treesLeft);

							const float lc_growrate = GetGrowthRates(spec_i, ages_i, l->ltID);
							const int lc_biomasscof = siteptr->specAtt(spec_i)->BioMassCoef;

							const float tmp_DTNBA = lc_growrate * lc_growrate / 40000 * 3.1415926 * treesToremove;
							const float tmp_DTNBIO = exp(GetBiomassData(lc_biomasscof, 1) + GetBiomassData(lc_biomasscof, 2) * log(lc_growrate)) * treesToremove / 1000.00;

							lc_specie->setTreeNum(ages_i, spec_i, (unsigned int)treesLeft);

							if (ppdp->TrackFlag == 1)
							{
								ppdp->cDeadTreeNum[row - 1][col - 1][spec_i - 1] += treesToremove;
								ppdp->cDeadTreeBA[row - 1][col - 1][spec_i - 1] += tmp_DTNBA;
								ppdp->cDeadTreeBio[row - 1][col - 1][spec_i - 1] += tmp_DTNBIO;

								const int lc_output_agerange_limit = GetOutputAgeRangeModuleFlagArray(spec_i - 1, COUNT);
								const int agei_m_timestep = ages_i * TimeStep;

								for (int agerange_index = 1; agerange_index <= lc_output_agerange_limit; agerange_index++)					//add by houbin 11/3/2017
								{
									const int lc_output_agerange_lowlimit = GetOutputAgeRangeModuleFlagArray(spec_i - 1, agerange_index, 0);
									const int lc_output_agerange_uprlimit = GetOutputAgeRangeModuleFlagArray(spec_i - 1, agerange_index, 1);

									if (lc_output_agerange_lowlimit < agei_m_timestep && agei_m_timestep <= lc_output_agerange_uprlimit)
									{
										const int lc_third_id = (agerange_index - 1) * specNum + spec_i - 1;

										ppdp->cDeadTreeNum_spec_agerange[row - 1][col - 1][lc_third_id] += treesToremove;
										ppdp->cDeadTreeBA_spec_agerange[row - 1][col - 1][lc_third_id] += tmp_DTNBA;
										ppdp->cDeadTreeBio_spec_agerange[row - 1][col - 1][lc_third_id] += tmp_DTNBIO;
									}
								}
							}
							

							Area_tobeThin -= tempAreaInvolveTreeNum;
						}
					}
				}
			}
			else
			{
				for (int i = 0; i < AreaList.size(); i++)
				{
					const int spec_i = SpecIndexArray.at(i);
					const int ages_i =  AgeIndexArray.at(i);

					SPECIE* lc_specie = siteptr->SpecieIndex(spec_i);

					const unsigned int lc_treenum = lc_specie->getTreeNum(ages_i, spec_i);

					if (lc_treenum > 0)
					{
						const float lc_thin_percent = thin_percent[siteptr->specAtt(spec_i)->shadeTolerance - 1];

						const double tempAreaInvolveTreeNum = AreaList.at(i) * lc_treenum * lc_thin_percent / 2;

						const double treesLeft = lc_treenum * (1 - lc_thin_percent / 2);

						assert(treesLeft >= 0 && treesLeft <= UINT_MAX);

						const int treesToremove = (int)(lc_treenum - treesLeft);

						const float lc_growrate = GetGrowthRates(spec_i, ages_i, l->ltID);
						const int lc_biomasscof = siteptr->specAtt(spec_i)->BioMassCoef;

						const float tmp_DTNBA = lc_growrate * lc_growrate / 40000 * 3.1415926 * treesToremove;
						const float tmp_DTNBIO = exp(GetBiomassData(lc_biomasscof, 1) + GetBiomassData(lc_biomasscof, 2) * log(lc_growrate)) * treesToremove / 1000.00;

						lc_specie->setTreeNum(ages_i, spec_i, (unsigned int)treesLeft);

						if (ppdp->TrackFlag == 1)
						{
							ppdp->cDeadTreeNum[row - 1][col - 1][spec_i - 1] += treesToremove;
							ppdp->cDeadTreeBA[row - 1][col - 1][spec_i - 1] += tmp_DTNBA;
							ppdp->cDeadTreeBio[row - 1][col - 1][spec_i - 1] += tmp_DTNBIO;

							const int lc_output_agerange_limit = GetOutputAgeRangeModuleFlagArray(spec_i - 1, COUNT);
							const int agei_m_timestep = ages_i * TimeStep;

							for (int agerange_index = 1; agerange_index <= lc_output_agerange_limit; agerange_index++)					//add by houbin 11/3/2017
							{
								const int lc_output_agerange_lowlimit = GetOutputAgeRangeModuleFlagArray(spec_i - 1, agerange_index, 0);
								const int lc_output_agerange_uprlimit = GetOutputAgeRangeModuleFlagArray(spec_i - 1, agerange_index, 1);

								if (lc_output_agerange_lowlimit < agei_m_timestep && agei_m_timestep <= lc_output_agerange_uprlimit)
								{
									const int lc_third_id = (agerange_index - 1) * specNum + spec_i - 1;

									ppdp->cDeadTreeNum_spec_agerange[row - 1][col - 1][lc_third_id] += treesToremove;
									ppdp->cDeadTreeBA_spec_agerange[row - 1][col - 1][lc_third_id] += tmp_DTNBA;
									ppdp->cDeadTreeBio_spec_agerange[row - 1][col - 1][lc_third_id] += tmp_DTNBIO;
								}
							}
						}
						

						Area_tobeThin -= tempAreaInvolveTreeNum;
					}
				}
			}
		}

		AreaList.clear();
		SpecIndexArray.clear();
		AgeIndexArray.clear();


		if (j == 2)
		{
			double subArea_tobeThin = 0.0;

			for (int spec_i = 1; spec_i <= num_specie; spec_i++)
			{
				SPECIESATTR* specatt_i = siteptr->specAtt(spec_i);
				SPECIE* 	 lc_specie = siteptr->SpecieIndex(spec_i);

				const int lc_index = (spec_i - 1) * 5 + j;
				const int tolrance = specatt_i->shadeTolerance;

				const float max_area = specatt_i->MaxAreaOfSTDTree;


				for (int age_i = quaterPercent[lc_index]+1; age_i <= quaterPercent[lc_index + 1]; age_i++)
				{
					const float lc_growrate = GetGrowthRates(spec_i, age_i, l->ltID);
					const unsigned int lc_treenum  = lc_specie->getTreeNum(age_i, spec_i);

					const double consterm = pow((lc_growrate / 25.4), 1.605) * max_area;

					subArea_tobeThin += thin_percent[tolrance - 1] * consterm * lc_treenum;

					AreaList.push_back(consterm);
					SpecIndexArray.push_back(spec_i);
					AgeIndexArray.push_back(age_i);
				}

			}


			ListbubbleSort(AreaList, SpecIndexArray, AgeIndexArray);

			if (subArea_tobeThin >= Area_tobeThin)
			{
				for (int i = 0; i < AreaList.size(); i++)
				{
					const int spec_i = SpecIndexArray.at(i);
					const int ages_i =  AgeIndexArray.at(i);

					SPECIE* lc_specie = siteptr->SpecieIndex(spec_i);

					const unsigned int lc_treenum = lc_specie->getTreeNum(ages_i, spec_i);

					if (lc_treenum > 0)
					{
						const float lc_thin_percent = thin_percent[siteptr->specAtt(spec_i)->shadeTolerance - 1];

						const double tempAreaInvolveTreeNum = AreaList.at(i) * lc_treenum * lc_thin_percent / 4;

						if (tempAreaInvolveTreeNum > Area_tobeThin)
						{
							const int treesToremove = (int)min(lc_treenum, Area_tobeThin / AreaList.at(i));
							const double treesLeft = lc_treenum - treesToremove;

							assert(treesLeft >= 0 && treesLeft <= UINT_MAX);

							const float lc_growrate = GetGrowthRates(spec_i, ages_i, l->ltID);
							const int lc_biomasscof = siteptr->specAtt(spec_i)->BioMassCoef;

							const float tmp_DTNBA = lc_growrate * lc_growrate / 40000 * 3.1415926 * treesToremove;
							const float tmp_DTNBIO = exp(GetBiomassData(lc_biomasscof, 1) + GetBiomassData(lc_biomasscof, 2) * log(lc_growrate)) * treesToremove / 1000.00;

							lc_specie->setTreeNum(ages_i, spec_i, (unsigned int)max(0, treesLeft));

							if (ppdp->TrackFlag==1)
							{
								ppdp->cDeadTreeNum[row - 1][col - 1][spec_i - 1] += treesToremove;
								ppdp->cDeadTreeBA[row - 1][col - 1][spec_i - 1] += tmp_DTNBA;
								ppdp->cDeadTreeBio[row - 1][col - 1][spec_i - 1] += tmp_DTNBIO;

								const int lc_output_agerange_limit = GetOutputAgeRangeModuleFlagArray(spec_i - 1, COUNT);
								const int agei_m_timestep = ages_i * TimeStep;

								for (int agerange_index = 1; agerange_index <= lc_output_agerange_limit; agerange_index++)					//add by houbin 11/3/2017
								{
									const int lc_output_agerange_lowlimit = GetOutputAgeRangeModuleFlagArray(spec_i - 1, agerange_index, 0);
									const int lc_output_agerange_uprlimit = GetOutputAgeRangeModuleFlagArray(spec_i - 1, agerange_index, 1);

									if (lc_output_agerange_lowlimit < agei_m_timestep && agei_m_timestep <= lc_output_agerange_uprlimit)
									{
										const int lc_third_id = (agerange_index - 1) * specNum + spec_i - 1;

										ppdp->cDeadTreeNum_spec_agerange[row - 1][col - 1][lc_third_id] += treesToremove;
										ppdp->cDeadTreeBA_spec_agerange[row - 1][col - 1][lc_third_id] += tmp_DTNBA;
										ppdp->cDeadTreeBio_spec_agerange[row - 1][col - 1][lc_third_id] += tmp_DTNBIO;
									}
								}
							}
							

							Area_tobeThin -= tempAreaInvolveTreeNum;

							delete[] quaterPercent;
							
							return;
						}
						else
						{
							const double treesLeft = lc_treenum * (1 - lc_thin_percent / 4);

							assert(treesLeft >= 0 && treesLeft <= UINT_MAX);

							const int treesToremove = (int)(lc_treenum - treesLeft);

							const float lc_growrate = GetGrowthRates(spec_i, ages_i, l->ltID);
							const int lc_biomasscof = siteptr->specAtt(spec_i)->BioMassCoef;

							const float tmp_DTNBA = lc_growrate * lc_growrate / 40000 * 3.1415926 * treesToremove;
							const float tmp_DTNBIO = exp(GetBiomassData(lc_biomasscof, 1) + GetBiomassData(lc_biomasscof, 2) * log(lc_growrate)) * treesToremove / 1000.00;
							
							lc_specie->setTreeNum(ages_i, spec_i, (unsigned int)treesLeft);

							if (ppdp->TrackFlag==1)
							{
								ppdp->cDeadTreeNum[row - 1][col - 1][spec_i - 1] += treesToremove;
								ppdp->cDeadTreeBA[row - 1][col - 1][spec_i - 1] += tmp_DTNBA;
								ppdp->cDeadTreeBio[row - 1][col - 1][spec_i - 1] += tmp_DTNBIO;

								const int lc_output_agerange_limit = GetOutputAgeRangeModuleFlagArray(spec_i - 1, COUNT);
								const int agei_m_timestep = ages_i * TimeStep;

								for (int agerange_index = 1; agerange_index <= lc_output_agerange_limit; agerange_index++)					//add by houbin 11/3/2017
								{
									const int lc_output_agerange_lowlimit = GetOutputAgeRangeModuleFlagArray(spec_i - 1, agerange_index, 0);
									const int lc_output_agerange_uprlimit = GetOutputAgeRangeModuleFlagArray(spec_i - 1, agerange_index, 1);

									if (lc_output_agerange_lowlimit < agei_m_timestep && agei_m_timestep <= lc_output_agerange_uprlimit)
									{
										const int lc_third_id = (agerange_index - 1) * specNum + spec_i - 1;

										ppdp->cDeadTreeNum_spec_agerange[row - 1][col - 1][lc_third_id] += treesToremove;
										ppdp->cDeadTreeBA_spec_agerange[row - 1][col - 1][lc_third_id] += tmp_DTNBA;
										ppdp->cDeadTreeBio_spec_agerange[row - 1][col - 1][lc_third_id] += tmp_DTNBIO;
									}
								}
							}
							

							Area_tobeThin -= tempAreaInvolveTreeNum;
						}
					}
				}
			}
			else
			{
				for (int i = 0; i < AreaList.size(); i++)
				{
					const int spec_i = SpecIndexArray.at(i);
					const int ages_i =  AgeIndexArray.at(i);

					SPECIE* lc_specie = siteptr->SpecieIndex(spec_i);

					const unsigned int lc_treenum = lc_specie->getTreeNum(ages_i, spec_i);

					if (lc_treenum > 0)
					{
						const float lc_thin_percent = thin_percent[siteptr->specAtt(spec_i)->shadeTolerance - 1];

						const double tempAreaInvolveTreeNum = AreaList.at(i) * lc_treenum * lc_thin_percent / 4;

						const double treesLeft = lc_treenum * (1 - lc_thin_percent / 4);

						assert(treesLeft >= 0 && treesLeft <= UINT_MAX);

						const int treesToremove = (int)(lc_treenum - treesLeft);

						const float lc_growrate = GetGrowthRates(spec_i, ages_i, l->ltID);
						const int lc_biomasscof = siteptr->specAtt(spec_i)->BioMassCoef;

						const float tmp_DTNBA = lc_growrate * lc_growrate / 40000 * 3.1415926 * treesToremove;
						const float tmp_DTNBIO = exp(GetBiomassData(lc_biomasscof, 1) + GetBiomassData(lc_biomasscof, 2) * log(lc_growrate)) * treesToremove / 1000.00;

						lc_specie->setTreeNum(ages_i, spec_i, (unsigned int)treesLeft);

						if (ppdp->TrackFlag == 1)
						{
							ppdp->cDeadTreeNum[row - 1][col - 1][spec_i - 1] += treesToremove;
							ppdp->cDeadTreeBA[row - 1][col - 1][spec_i - 1] += tmp_DTNBA;
							ppdp->cDeadTreeBio[row - 1][col - 1][spec_i - 1] += tmp_DTNBIO;

							const int lc_output_agerange_limit = GetOutputAgeRangeModuleFlagArray(spec_i - 1, COUNT);
							const int agei_m_timestep = ages_i * TimeStep;

							for (int agerange_index = 1; agerange_index <= lc_output_agerange_limit; agerange_index++)					//add by houbin 11/3/2017
							{
								const int lc_output_agerange_lowlimit = GetOutputAgeRangeModuleFlagArray(spec_i - 1, agerange_index, 0);
								const int lc_output_agerange_uprlimit = GetOutputAgeRangeModuleFlagArray(spec_i - 1, agerange_index, 1);

								if (lc_output_agerange_lowlimit < agei_m_timestep && agei_m_timestep <= lc_output_agerange_uprlimit)
								{
									const int lc_third_id = (agerange_index - 1) * specNum + spec_i - 1;

									ppdp->cDeadTreeNum_spec_agerange[row - 1][col - 1][lc_third_id] += treesToremove;
									ppdp->cDeadTreeBA_spec_agerange[row - 1][col - 1][lc_third_id] += tmp_DTNBA;
									ppdp->cDeadTreeBio_spec_agerange[row - 1][col - 1][lc_third_id] += tmp_DTNBIO;
								}
							}
						}
						

						Area_tobeThin -= tempAreaInvolveTreeNum;
					}
				}
			}
		}


		AreaList.clear();
		SpecIndexArray.clear();
		AgeIndexArray.clear();


		if (j == 3)
		{
			double subArea_tobeThin = 0.0;

			for (int spec_i = 1; spec_i <= num_specie; spec_i++)
			{
				SPECIESATTR* specatt_i = siteptr->specAtt(spec_i);
				SPECIE* 	 lc_specie = siteptr->SpecieIndex(spec_i);

				const int lc_index = (spec_i - 1) * 5 + j;
				const int tolrance = specatt_i->shadeTolerance;

				const float max_area = specatt_i->MaxAreaOfSTDTree;


				for (int age_i = quaterPercent[lc_index]+1; age_i <= quaterPercent[lc_index + 1]; age_i++)
				{
					const float lc_growrate = GetGrowthRates(spec_i, age_i, l->ltID);
					const unsigned int lc_treenum  = lc_specie->getTreeNum(age_i, spec_i);

					const double consterm = pow((lc_growrate / 25.4), 1.605) * max_area;

					subArea_tobeThin += thin_percent[tolrance - 1] * consterm * lc_treenum;

					AreaList.push_back(consterm);
					SpecIndexArray.push_back(spec_i);
					AgeIndexArray.push_back(age_i);
				}
			}


			ListbubbleSort(AreaList, SpecIndexArray, AgeIndexArray);

			if (subArea_tobeThin >= Area_tobeThin)
			{
				for (int i = 0; i < AreaList.size(); i++)
				{
					const int spec_i = SpecIndexArray.at(i);
					const int ages_i =  AgeIndexArray.at(i);

					SPECIE* lc_specie = siteptr->SpecieIndex(spec_i);

					const unsigned int lc_treenum = lc_specie->getTreeNum(ages_i, spec_i);

					if (lc_treenum > 0)
					{
						const float lc_thin_percent = thin_percent[siteptr->specAtt(spec_i)->shadeTolerance - 1];

						const double tempAreaInvolveTreeNum = AreaList.at(i) * lc_treenum * lc_thin_percent / 8;

						if (tempAreaInvolveTreeNum > Area_tobeThin)
						{
							const int treesToremove = (int)min(lc_treenum, Area_tobeThin / AreaList.at(i));
							const double treesLeft = lc_treenum - treesToremove;

							assert(treesLeft >= 0 && treesLeft <= UINT_MAX);

							const float lc_growrate = GetGrowthRates(spec_i, ages_i, l->ltID);
							const int lc_biomasscof = siteptr->specAtt(spec_i)->BioMassCoef;

							const float tmp_DTNBA = lc_growrate * lc_growrate / 40000 * 3.1415926 * treesToremove;
							const float tmp_DTNBIO = exp(GetBiomassData(lc_biomasscof, 1) + GetBiomassData(lc_biomasscof, 2) * log(lc_growrate)) * treesToremove / 1000.00;

							lc_specie->setTreeNum(ages_i, spec_i, (unsigned int)max(0, treesLeft));

							if (ppdp->TrackFlag == 1)
							{
								ppdp->cDeadTreeNum[row - 1][col - 1][spec_i - 1] += treesToremove;
								ppdp->cDeadTreeBA[row - 1][col - 1][spec_i - 1] += tmp_DTNBA;
								ppdp->cDeadTreeBio[row - 1][col - 1][spec_i - 1] += tmp_DTNBIO;

								const int lc_output_agerange_limit = GetOutputAgeRangeModuleFlagArray(spec_i - 1, COUNT);
								const int agei_m_timestep = ages_i * TimeStep;

								for (int agerange_index = 1; agerange_index <= lc_output_agerange_limit; agerange_index++)					//add by houbin 11/3/2017
								{
									const int lc_output_agerange_lowlimit = GetOutputAgeRangeModuleFlagArray(spec_i - 1, agerange_index, 0);
									const int lc_output_agerange_uprlimit = GetOutputAgeRangeModuleFlagArray(spec_i - 1, agerange_index, 1);

									if (lc_output_agerange_lowlimit < agei_m_timestep && agei_m_timestep <= lc_output_agerange_uprlimit)
									{
										const int lc_third_id = (agerange_index - 1) * specNum + spec_i - 1;

										ppdp->cDeadTreeNum_spec_agerange[row - 1][col - 1][lc_third_id] += treesToremove;
										ppdp->cDeadTreeBA_spec_agerange[row - 1][col - 1][lc_third_id] += tmp_DTNBA;
										ppdp->cDeadTreeBio_spec_agerange[row - 1][col - 1][lc_third_id] += tmp_DTNBIO;
									}
								}
							}
							

							Area_tobeThin -= tempAreaInvolveTreeNum;

							delete[] quaterPercent;

							return;
						}
						else
						{
							const double treesLeft = lc_treenum * (1 - lc_thin_percent / 8);

							assert(treesLeft >= 0 && treesLeft <= UINT_MAX);

							const int treesToremove = (int)(lc_treenum - treesLeft);

							const float lc_growrate = GetGrowthRates(spec_i, ages_i, l->ltID);
							const int lc_biomasscof = siteptr->specAtt(spec_i)->BioMassCoef;

							const float tmp_DTNBA = lc_growrate * lc_growrate / 40000 * 3.1415926 * treesToremove;
							const float tmp_DTNBIO = exp(GetBiomassData(lc_biomasscof, 1) + GetBiomassData(lc_biomasscof, 2) * log(lc_growrate)) * treesToremove / 1000.00;

							lc_specie->setTreeNum(ages_i, spec_i, (unsigned int)treesLeft);

							if (ppdp->TrackFlag == 1)
							{
								ppdp->cDeadTreeNum[row - 1][col - 1][spec_i - 1] += treesToremove;
								ppdp->cDeadTreeBA[row - 1][col - 1][spec_i - 1] += tmp_DTNBA;
								ppdp->cDeadTreeBio[row - 1][col - 1][spec_i - 1] += tmp_DTNBIO;

								const int lc_output_agerange_limit = GetOutputAgeRangeModuleFlagArray(spec_i - 1, COUNT);
								const int agei_m_timestep = ages_i * TimeStep;

								for (int agerange_index = 1; agerange_index <= lc_output_agerange_limit; agerange_index++)					//add by houbin 11/3/2017
								{
									const int lc_output_agerange_lowlimit = GetOutputAgeRangeModuleFlagArray(spec_i - 1, agerange_index, 0);
									const int lc_output_agerange_uprlimit = GetOutputAgeRangeModuleFlagArray(spec_i - 1, agerange_index, 1);

									if (lc_output_agerange_lowlimit < agei_m_timestep && agei_m_timestep <= lc_output_agerange_uprlimit)
									{
										const int lc_third_id = (agerange_index - 1) * specNum + spec_i - 1;

										ppdp->cDeadTreeNum_spec_agerange[row - 1][col - 1][lc_third_id] += treesToremove;
										ppdp->cDeadTreeBA_spec_agerange[row - 1][col - 1][lc_third_id] += tmp_DTNBA;
										ppdp->cDeadTreeBio_spec_agerange[row - 1][col - 1][lc_third_id] += tmp_DTNBIO;
									}
								}
							}
							

							Area_tobeThin -= tempAreaInvolveTreeNum;
						}
					}
				}
			}
			else
			{
				for (int i = 0; i < AreaList.size(); i++)
				{
					const int spec_i = SpecIndexArray.at(i);
					const int ages_i =  AgeIndexArray.at(i);

					SPECIE* lc_specie = siteptr->SpecieIndex(spec_i);

					const unsigned int lc_treenum = lc_specie->getTreeNum(ages_i, spec_i);


					if (lc_treenum > 0)
					{
						const float lc_thin_percent = thin_percent[siteptr->specAtt(spec_i)->shadeTolerance - 1];

						const double tempAreaInvolveTreeNum = AreaList.at(i) * lc_treenum * lc_thin_percent / 8;

						const double treesLeft = lc_treenum * (1 - lc_thin_percent / 8);

						assert(treesLeft >= 0 && treesLeft <= UINT_MAX);

						const int treesToremove = (int)(lc_treenum - treesLeft);

						const float lc_growrate = GetGrowthRates(spec_i, ages_i, l->ltID);
						const int lc_biomasscof = siteptr->specAtt(spec_i)->BioMassCoef;

						const float tmp_DTNBA = lc_growrate * lc_growrate / 40000 * 3.1415926 * treesToremove;
						const float tmp_DTNBIO = exp(GetBiomassData(lc_biomasscof, 1) + GetBiomassData(lc_biomasscof, 2) * log(lc_growrate)) * treesToremove / 1000.00;

						lc_specie->setTreeNum(ages_i, spec_i, (unsigned int)treesLeft);

						if (ppdp->TrackFlag == 1)
						{
							ppdp->cDeadTreeNum[row - 1][col - 1][spec_i - 1] += treesToremove;
							ppdp->cDeadTreeBA[row - 1][col - 1][spec_i - 1] += tmp_DTNBA;
							ppdp->cDeadTreeBio[row - 1][col - 1][spec_i - 1] += tmp_DTNBIO;

							const int lc_output_agerange_limit = GetOutputAgeRangeModuleFlagArray(spec_i - 1, COUNT);
							const int agei_m_timestep = ages_i * TimeStep;

							for (int agerange_index = 1; agerange_index <= lc_output_agerange_limit; agerange_index++)					//add by houbin 11/3/2017
							{
								const int lc_output_agerange_lowlimit = GetOutputAgeRangeModuleFlagArray(spec_i - 1, agerange_index, 0);
								const int lc_output_agerange_uprlimit = GetOutputAgeRangeModuleFlagArray(spec_i - 1, agerange_index, 1);

								if (lc_output_agerange_lowlimit < agei_m_timestep && agei_m_timestep <= lc_output_agerange_uprlimit)
								{
									const int lc_third_id = (agerange_index - 1) * specNum + spec_i - 1;

									ppdp->cDeadTreeNum_spec_agerange[row - 1][col - 1][lc_third_id] += treesToremove;
									ppdp->cDeadTreeBA_spec_agerange[row - 1][col - 1][lc_third_id] += tmp_DTNBA;
									ppdp->cDeadTreeBio_spec_agerange[row - 1][col - 1][lc_third_id] += tmp_DTNBIO;
								}
							}
						}
						

						Area_tobeThin -= tempAreaInvolveTreeNum;
					}
				}
			}
		}
	}


	delete[] quaterPercent;

}










void SITES::SeedGermination(SITE* siteptr, LANDUNIT *l, int RDFlag)
{
	static const unsigned int cellsize_square = CellSize * CellSize;

	const int num_specie = siteptr->number();
	
	double* Seedling = new double[num_specie]();
	double* IndexRD  = new double[num_specie]();
	
	const float lc_rd_diff = l->MaxRD - siteptr->RD;

	const double rddiff_m_cellsizesquare = lc_rd_diff * cellsize_square;

	const float lc_maxage = siteptr->MaxAge;
	const int lc_minshade = l->minShade;

	// static unsigned long record_availseed = 0;

	if (RDFlag == 0 || RDFlag == 1)
	{
		double RDTotal = 0.0;

		for (int i = 1; i <= num_specie; i++)
		{
			const double term1 = pow((SITES::GetGrowthRates(i, 1, l->ltID) / 25.4), 1.605);
			const float  term2 = siteptr->specAtt(i)->MaxAreaOfSTDTree;

			IndexRD[i - 1] = term1 * term2 * siteptr->SpecieIndex(i)->AvailableSeed / cellsize_square;

			RDTotal += IndexRD[i - 1];
		}


		if (RDTotal >= 0.0)
		{
			if (RDTotal <= lc_rd_diff)
			{
				for (int i = 1; i <= num_specie; i++)
				{
					SPECIESATTR* specatt_i = siteptr->specAtt(i);

					const int lc_shade_tolern = specatt_i->shadeTolerance;
					
					if (lc_shade_tolern <= 4 || (lc_shade_tolern == 5 && lc_maxage >= lc_minshade))
					{
						SPECIE* specie_i = siteptr->SpecieIndex(i);

						const long temp = specie_i->TreesFromVeg * specatt_i->vegProb;

						specie_i->TreesFromVeg = 0;

						Seedling[i - 1] = specie_i->AvailableSeed * l->probRepro(i) + temp;

						const int ran = rand() % 10 + 1; // ran in the range 1 to 10

						if (ran <= 1)
							Seedling[i - 1] += 1;
					}

				}

			}

			else
			{
				for (int i = 1; i <= num_specie; i++)
				{
					SPECIESATTR* specatt_i = siteptr->specAtt(i);
					const int lc_shade_tolern = specatt_i->shadeTolerance;

					if (lc_shade_tolern <= 4 || (lc_shade_tolern == 5 && lc_maxage >= lc_minshade))
					{
						SPECIE* specie_i = siteptr->SpecieIndex(i);

						const long temp = specie_i->TreesFromVeg * specatt_i->vegProb;

						specie_i->TreesFromVeg = 0;

						const double term1 = pow((SITES::GetGrowthRates(i, 1, l->ltID) / 25.4), 1.605);
						const float  term2 = siteptr->specAtt(i)->MaxAreaOfSTDTree;

						const double seedlingTemp = IndexRD[i - 1] * rddiff_m_cellsizesquare / (RDTotal * term1 * term2);

						Seedling[i - 1] = seedlingTemp * l->probRepro(i) + temp;

						const int ran = rand() % 10 + 1; // ran in the range 1 to 10

						if (ran <= 1)
							Seedling[i - 1] += 1;
					}

				}

			}

		}
	}
	else if (RDFlag == 2)
	{
		double RDTotal = 0.0;

		for (int i = 1; i <= num_specie; i++)
		{
			if (siteptr->specAtt(i)->shadeTolerance >= siteptr->HighestShadeTolerance)
			{
				const double term1 = pow((SITES::GetGrowthRates(i, 1, l->ltID) / 25.4), 1.605);
				const float  term2 = siteptr->specAtt(i)->MaxAreaOfSTDTree;

				IndexRD[i - 1] = term1 * term2 * siteptr->SpecieIndex(i)->AvailableSeed / cellsize_square;

				RDTotal += IndexRD[i - 1];
			}
			else
			{
				Seedling[i - 1] = 0;
			}

		}


		if (RDTotal >= 0.0)
		{
			if (RDTotal <= lc_rd_diff)
			{
				for (int i = 1; i <= num_specie; i++)
				{
					SPECIESATTR* specatt_i = siteptr->specAtt(i);

					if (specatt_i->shadeTolerance >= siteptr->HighestShadeTolerance)
					{
						SPECIE* specie_i = siteptr->SpecieIndex(i);

						const long temp = specie_i->TreesFromVeg * specatt_i->vegProb;

						specie_i->TreesFromVeg = 0;

						Seedling[i - 1] = specie_i->AvailableSeed * l->probRepro(i) + temp;

						const int ran = rand() % 10 + 1; // ran in the range 1 to 10

						if (ran <= 1)
							Seedling[i - 1] += 1;
					}
				}
			}
			else
			{
				for (int i = 1; i <= num_specie; i++)
				{
					SPECIESATTR* specatt_i = siteptr->specAtt(i);

					if (specatt_i->shadeTolerance >= siteptr->HighestShadeTolerance)
					{
						SPECIE* specie_i = siteptr->SpecieIndex(i);

						const long temp = specie_i->TreesFromVeg * specatt_i->vegProb;

						specie_i->TreesFromVeg = 0;

						const double term1 = pow((SITES::GetGrowthRates(i, 1, l->ltID) / 25.4), 1.605);
						const float  term2 = siteptr->specAtt(i)->MaxAreaOfSTDTree;

						const double seedlingTemp = IndexRD[i - 1] * rddiff_m_cellsizesquare / (RDTotal * term1 * term2);

						Seedling[i - 1] = seedlingTemp * l->probRepro(i) + temp;

						const int ran = rand() % 10 + 1; // ran in the range 1 to 10

						if (ran <= 1)
							Seedling[i - 1] += 1;
					}
				}
			}
		}		

	}

	else if (RDFlag == 3)
	{
		double RDTotal = 0.0;

		for (int i = 1; i <= num_specie; i++)
		{
			if (siteptr->specAtt(i)->shadeTolerance == MaxShadeTolerance && lc_maxage >= lc_minshade)
			{
				const double term1 = pow((SITES::GetGrowthRates(i, 1, l->ltID) / 25.4), 1.605);
				const float  term2 = siteptr->specAtt(i)->MaxAreaOfSTDTree;

				IndexRD[i - 1] = term1 * term2 * siteptr->SpecieIndex(i)->AvailableSeed / cellsize_square;

				RDTotal += IndexRD[i - 1];
			}
			else
			{
				Seedling[i - 1] = 0;
			}
		}


		if (RDTotal >= 0.0)
		{
			if (RDTotal <= lc_rd_diff)
			{
				for (int i = 1; i <= num_specie; i++)
				{
					SPECIESATTR* specatt_i = siteptr->specAtt(i);

					if (specatt_i->shadeTolerance == MaxShadeTolerance && lc_maxage >= lc_minshade)
					{
						SPECIE* specie_i = siteptr->SpecieIndex(i);

						const long temp = specie_i->TreesFromVeg * specatt_i->vegProb;

						specie_i->TreesFromVeg = 0;

						Seedling[i - 1] = specie_i->AvailableSeed * l->probRepro(i) + temp;
						
						const int ran = rand() % 10 + 1; // ran in the range 1 to 10

						if (ran <= 1)
							Seedling[i - 1] += 1;
					}
				}

			}

			else
			{
				for (int i = 1; i <= num_specie; i++)
				{
					SPECIESATTR* specatt_i = siteptr->specAtt(i);
					
					if (specatt_i->shadeTolerance == MaxShadeTolerance && lc_maxage >= lc_minshade)
					{
						SPECIE* specie_i = siteptr->SpecieIndex(i);

						const long temp = specie_i->TreesFromVeg * specatt_i->vegProb;

						specie_i->TreesFromVeg = 0;

						const double term1 = pow((SITES::GetGrowthRates(i, 1, l->ltID) / 25.4), 1.605);
						const float  term2 = siteptr->specAtt(i)->MaxAreaOfSTDTree;

						const double seedlingTemp = IndexRD[i - 1] * rddiff_m_cellsizesquare / (RDTotal * term1 * term2);

						Seedling[i - 1] = seedlingTemp * l->probRepro(i) + temp;

						const int ran = rand() % 10 + 1; // ran in the range 1 to 10

						if (ran <= 1)
							Seedling[i - 1] += 1;
					}
				}

			}

		}
	}
	else
	{
		for (int i = 1; i <= num_specie; i++)
			Seedling[i - 1] = 0;
	}


	for (int i = 1; i <= num_specie; i++)
	{
		if (Seedling[i - 1] > 0)
		{
			assert(Seedling[i - 1] < UINT_MAX + 1.0);
			siteptr->SpecieIndex(i)->setTreeNum(1, i, (unsigned int)Seedling[i - 1]);
		}
		else
			siteptr->SpecieIndex(i)->setTreeNum(1, i, 0);
	}

	delete[] IndexRD;
	delete[] Seedling;



	// SPECIE* lspecie_i = siteptr->SpecieIndex(1);
	// if(lspecie_i->AvailableSeed != 0)
	// {
	// 	record_availseed += Seedling[0];
	// 	// printf("Avail_seed = %lu, record_availseed = %lu\n", lspecie_i->AvailableSeed, record_availseed);
	// 	printf("%lu\n", record_availseed);
	// }
}









//calculate the RD for a given site, No Return Value: bsl
void SITES::GetRDofSite(int Row, int Col)
{
	static const unsigned int cellsize_square = CellSize * CellSize;

	SITE* siteptr = SITES::operator()(Row, Col);

	siteptr->HighestShadeTolerance = 0;

	const int numSpec = siteptr->number();
	
	siteptr->RD = 0;

	LANDUNIT *l = SITES::locateLanduPt(Row, Col);

	for (int i = 1; i <= numSpec; i++)
	{
		SPECIESATTR* speciesattr_i = siteptr -> specAtt(i);
		SPECIE* 		  specie_i = siteptr -> SpecieIndex(i);
		const float local_val = speciesattr_i -> MaxAreaOfSTDTree / cellsize_square;
		
		if (speciesattr_i->SpType >= 0)
		{
			const int loopnumber = speciesattr_i->longevity / TimeStep;

			for (int j = 1; j <= loopnumber; j++)
			{
				const unsigned int local_tree_num = specie_i->getTreeNum(j, i);
				const float lc_growrate = GetGrowthRates(i, j, l->ltID);

				siteptr->RD += pow(lc_growrate / 25.4, 1.605) * local_tree_num * local_val;

				if (local_tree_num > 0)
				{
					if (siteptr->HighestShadeTolerance < speciesattr_i->shadeTolerance)
						siteptr->HighestShadeTolerance = speciesattr_i->shadeTolerance;
				}
			}
		}
	}
}







//calculate the MaxAge for a given site
void SITES::MaxAgeofSite(SITE *siteptr)
{
	const int numSpec = siteptr->number();

	siteptr->MaxAge = 0;
	
	for (int i = 1; i <= numSpec; i++)
	{
		const int j_end = siteptr->specAtt(i)->longevity / TimeStep;
		
		SPECIE* lc_specie = siteptr->SpecieIndex(i);

		for (int j = 1; j <= j_end; j++)
		{
			if (lc_specie->getTreeNum(j, i) > 0)
			{
				if (siteptr->MaxAge < j)
					siteptr->MaxAge = j;
			}
		}
	}
}






void SITES::GetMatureTree()
{
	for (int i = 1; i <= rows; i++)
	{
		for (int j = 1; j <= columns; j++)
		{
			SITE* siteptr = SITES::operator()(i, j);
			const int num_species = siteptr->number();

			for (int k = 1; k <= num_species; k++)
			{
				SPECIE* lc_specie = siteptr->SpecieIndex(k);

				const int m_beg = siteptr->specAtt(k)->maturity / TimeStep;
				const int m_end = siteptr->specAtt(k)->longevity / TimeStep;

				unsigned int tempvalue = 0;
				
				for (int m = m_beg; m <= m_end; m++)
					tempvalue += lc_specie->getTreeNum(m, k);

				lc_specie->SetMatureTreeValue(tempvalue);
			}
		}
	}
}










//According to total seed of each species, calculate the available seed number would disperse into other sites
void SITES::GetSeedNumberOnSite_type2(int Row, int Col, int species_id)
{
	SITE* siteptr = SITES::operator()(Row, Col);

	const int k = species_id;
	SPECIESATTR* specAtt_k = siteptr->specAtt(k);
	SPECIE* 	  specie_k = siteptr->SpecieIndex(k);

	const int local_maxD = specAtt_k->maxD;
	assert(local_maxD > 0);

	const int local_maxD_d_cellsize = local_maxD / CellSize;
	const int offset = local_maxD_d_cellsize >= 1 ? local_maxD_d_cellsize : 1;
	
	specie_k->AvailableSeed = 0;

	double Avail_Seed = specie_k->AvailableSeed;

	for (int row_offset = -offset; row_offset <= offset; row_offset++)
	{
		for (int col_offset = -offset; col_offset <= offset; col_offset++)
		{
			const int i = Row - row_offset;
			const int j = Col - col_offset;
			
			if (i >= 1 && i <= rows && j >= 1 && j <= columns)
			{
				LANDUNIT* landunit_ij = locateLanduPt(i, j);

				if (landunit_ij != NULL && landunit_ij->active())
				{
					SITE* lc_site = SITES::operator()(i, j);
					const int lc_mature_treenum = lc_site->SpecieIndex(k)->GetMatureTreeValue();
					const int lc_totalseed = lc_site->specAtt(k)->TotalSeed;

					// const float lc_seedrain = GetSeedRain(k, TempDist);
					const double lc_seedrain = seed_prob.get_seed_prob(k-1, row_offset, col_offset);

					Avail_Seed += lc_seedrain * lc_mature_treenum * lc_totalseed * TimeStep;
				}
			}
		}
	}


	if (Avail_Seed > ULONG_MAX)
		specie_k->AvailableSeed = ULONG_MAX;
	else
		specie_k->AvailableSeed = (unsigned long)Avail_Seed;
}









void SITES::NaturalMortality_killbytargetRD(SITE * siteptr, int Row, int Col, double targetRD)
{
	const int num_specie = siteptr->number();

	for (int i = 1; i <= num_specie; i++)
	{
		SPECIESATTR* lc_speciesattr_i = siteptr->specAtt(i);

		if (lc_speciesattr_i->SpType >= 0)
		{
			SPECIE* lc_specie = siteptr->SpecieIndex(i);
			
			for (int j = lc_speciesattr_i->longevity / TimeStep; j >= 1; j--)
			{
				if (lc_specie->getTreeNum(j, i) > 0)
				{
					lc_specie->setTreeNum(j, i, 0);
					
					GetRDofSite(Row, Col);
					
					if (siteptr->RD <= targetRD)
						return;
				}
			}
		}
	}
}






//According to MortalityRates, kill trees and calculate tree numbers
void SITES::NaturalMortality(SITE * siteptr, int Row, int Col, int StartAge, const PDP* ppdp)//bsl
{
	static const unsigned int cellsize_square = CellSize * CellSize;
	static const double tmpDQ_term2 = 3.1415926 / (4 * 0.0002471 * cellsize_square * 30.48 * 30.48);

	LANDUNIT *l = SITES::locateLanduPt(Row, Col);

	const int num_specie = siteptr->number();

	int DTN_onespec_allage = 0;
	float DTNBIO_onespec_allage = 0;
	float DTNBA_onespec_allage = 0;

	float tmp_BA = 0;
	float tmp_BIO = 0;
	
	if (StartAge == 0)//kill all tree, else kill youngest tree
	{		
		if (MortalityFlag == 0)
		{
			double tmpDQ = 0;

			for (int i = 1; i <= num_specie; i++)
			{
				SPECIE* lc_specie = siteptr->SpecieIndex(i);

				if (siteptr->specAtt(i)->SpType >= 0)
				{
					const int loopnum = siteptr->specAtt(i)->longevity / TimeStep;

					for (int j = 1; j <= loopnum; j++)
					{
						const float lc_growrate = GetGrowthRates(i, j, l->ltID);

						tmpDQ += lc_growrate * lc_growrate * tmpDQ_term2 * lc_specie->getTreeNum(j, i);
					}
				}
			}


			for (int i = 1; i <= num_specie; i++)
			{
				DTN_onespec_allage = 0;
				DTNBIO_onespec_allage = 0;
				DTNBA_onespec_allage = 0;

				SPECIE* lc_specie = siteptr->SpecieIndex(i);

				const int loopnum = siteptr->specAtt(i)->longevity / TimeStep;

				if (siteptr->specAtt(i)->SpType >= 0)
				{
					for (int j = 1; j <= loopnum; j++)
					{
						const float lc_growrate = GetGrowthRates(i, j, l->ltID);

						double TmpMortality = TimeStep / 10.0 / (1.0 + exp(3.25309 - 0.00072647 * tmpDQ + 0.01668809 * lc_growrate / 2.54));
						TmpMortality = min(1.0, TmpMortality);

						const unsigned int numnum = lc_specie->getTreeNum(j, i);

						const double DeadTree = numnum * TmpMortality;
						
						unsigned int DeadTreeInt = (unsigned int)DeadTree;

						if (DeadTree - DeadTreeInt >= 0.0001 && frand1() < 0.1)
							DeadTreeInt++;

						assert(numnum - DeadTreeInt <= UINT_MAX);

						lc_specie->setTreeNum(j, i, (unsigned int)max(0, numnum - DeadTreeInt));
					
						tmpDQ -= lc_growrate * lc_growrate * tmpDQ_term2 * DeadTree;

						DeadTreeInt = min(DeadTreeInt, numnum);

						const int lc_biomasscof = siteptr->specAtt(i)->BioMassCoef;

						tmp_BA = lc_growrate * lc_growrate / 40000 * 3.1415926 * DeadTreeInt;	
						tmp_BIO = exp(GetBiomassData(lc_biomasscof, 1) + GetBiomassData(lc_biomasscof, 2) * log(lc_growrate)) * DeadTreeInt / 1000.00;					

						DTN_onespec_allage += DeadTreeInt;
						DTNBA_onespec_allage += tmp_BA;
						DTNBIO_onespec_allage += tmp_BIO;

						const int lc_output_agerange_limit = GetOutputAgeRangeModuleFlagArray(i - 1, COUNT);
						const int j_m_timestep = j * TimeStep;

						if (ppdp->TrackFlag == 1)
						{
							for (int agerange_index = 1; agerange_index <= lc_output_agerange_limit; agerange_index++)					//add by houbin 11/3/2017
							{
								const int lc_output_agerange_lowlimit = GetOutputAgeRangeModuleFlagArray(i - 1, agerange_index, 0);
								const int lc_output_agerange_uprlimit = GetOutputAgeRangeModuleFlagArray(i - 1, agerange_index, 1);

								if (lc_output_agerange_lowlimit < j_m_timestep && j_m_timestep <= lc_output_agerange_uprlimit)
								{
									const int lc_third_id = (agerange_index - 1) * num_specie + i - 1;

									ppdp->cDeadTreeNum_spec_agerange[Row - 1][Col - 1][lc_third_id] += DeadTreeInt;
									ppdp->cDeadTreeBA_spec_agerange[Row - 1][Col - 1][lc_third_id] += tmp_BA;
									ppdp->cDeadTreeBio_spec_agerange[Row - 1][Col - 1][lc_third_id] += tmp_BIO;
								}
							}
						}
						
					}

					if (ppdp->TrackFlag == 1)
					{
						ppdp->cDeadTreeNum[Row - 1][Col - 1][i - 1] += DTN_onespec_allage;
						ppdp->cDeadTreeBA[Row - 1][Col - 1][i - 1] += DTNBA_onespec_allage;
						ppdp->cDeadTreeBio[Row - 1][Col - 1][i - 1] += DTNBIO_onespec_allage;
					}
				}
				else
				{
					for (int j = 1; j <= loopnum; j++)
						lc_specie->setTreeNum(j, i, cellsize_square);
				}
			}
		}
		else
		{
			for (int i = 1; i <= num_specie; i++)
			{
				DTN_onespec_allage = 0;
				DTNBIO_onespec_allage = 0;
				DTNBA_onespec_allage = 0;

				SPECIE* lc_specie = siteptr->SpecieIndex(i);

				const int loopnum = siteptr->specAtt(i)->longevity / TimeStep;

				if (siteptr->specAtt(i)->SpType >= 0)
				{
					for (int j = 1; j <= loopnum; j++)
					{
						const unsigned int numnum = lc_specie->getTreeNum(j, i);

						const double DeadTree = numnum * GetMortalityRates(i, j, l->ltID);

						unsigned int DeadTreeInt = (unsigned int)DeadTree;

						if (DeadTree - DeadTreeInt >= 0.0001 && frand1() < 0.1)
							DeadTreeInt++;

						assert(numnum - DeadTreeInt <= UINT_MAX);
						
						lc_specie->setTreeNum(j, i, (unsigned int)max(0, numnum - DeadTreeInt));

						DeadTreeInt = min(DeadTreeInt, numnum);

						const int lc_biomasscof = siteptr->specAtt(i)->BioMassCoef;

						const float lc_growrate = GetGrowthRates(i, j, l->ltID);
						tmp_BA = lc_growrate * lc_growrate / 40000 * 3.1415926 * DeadTreeInt;	
						tmp_BIO = exp(GetBiomassData(lc_biomasscof, 1) + GetBiomassData(lc_biomasscof, 2) * log(lc_growrate)) * DeadTreeInt / 1000.00;					

						DTN_onespec_allage += DeadTreeInt;
						DTNBA_onespec_allage += tmp_BA;
						DTNBIO_onespec_allage += tmp_BIO;

						const int lc_output_agerange_limit = GetOutputAgeRangeModuleFlagArray(i - 1, COUNT);
						const int j_m_timestep = j * TimeStep;

						if (ppdp->TrackFlag == 1)
						{
							for (int agerange_index = 1; agerange_index <= lc_output_agerange_limit; agerange_index++)					//add by houbin 11/3/2017
							{
								const int lc_output_agerange_lowlimit = GetOutputAgeRangeModuleFlagArray(i - 1, agerange_index, 0);
								const int lc_output_agerange_uprlimit = GetOutputAgeRangeModuleFlagArray(i - 1, agerange_index, 1);

								if (lc_output_agerange_lowlimit < j_m_timestep && j_m_timestep <= lc_output_agerange_uprlimit)
								{
									const int lc_third_id = (agerange_index - 1) * num_specie + i - 1;

									ppdp->cDeadTreeNum_spec_agerange[Row - 1][Col - 1][lc_third_id] += DeadTreeInt;
									ppdp->cDeadTreeBA_spec_agerange[Row - 1][Col - 1][lc_third_id] += tmp_BA;
									ppdp->cDeadTreeBio_spec_agerange[Row - 1][Col - 1][lc_third_id] += tmp_BIO;
								}
							}
						}
						
					}

					if (ppdp->TrackFlag == 1)
					{
						ppdp->cDeadTreeNum[Row - 1][Col - 1][i - 1] += DTN_onespec_allage;
						ppdp->cDeadTreeBA[Row - 1][Col - 1][i - 1] += DTNBA_onespec_allage;
						ppdp->cDeadTreeBio[Row - 1][Col - 1][i - 1] += DTNBIO_onespec_allage;
					}
					
				}
				else
				{
					for (int j = 1; j <= loopnum; j++)
						lc_specie->setTreeNum(j, i, cellsize_square);
				}
			}
		}
	}
	else
	{//kill all tree

		if (MortalityFlag == 0)
		{
			double tmpDQ = 0;

			for (int i = 1; i <= num_specie; i++)
			{
				if (siteptr->specAtt(i)->SpType >= 0)
				{
					SPECIE* lc_specie = siteptr->SpecieIndex(i);

					const int loopnum = siteptr->specAtt(i)->longevity / TimeStep;

					for (int j = 1; j <= loopnum; j++)
					{
						const float lc_growrate = GetGrowthRates(i, j, l->ltID);

						tmpDQ += lc_growrate * lc_growrate * tmpDQ_term2 * lc_specie->getTreeNum(j, i);
					}
				}				

			}


			for (int i = 1; i <= num_specie; i++)
			{
				DTN_onespec_allage = 0;
				DTNBIO_onespec_allage = 0;
				DTNBA_onespec_allage = 0;

				SPECIE* lc_specie = siteptr->SpecieIndex(i);

				if (siteptr->specAtt(i)->SpType >= 0)
				{
					for (int j = 1; j <= StartAge; j++)
					{
						const float lc_growrate = GetGrowthRates(i, j, l->ltID);

						double TmpMortality = TimeStep / 10.0 / (1.0 + exp(3.25309 - 0.00072647*tmpDQ + 0.01668809*lc_growrate / 2.54));
						TmpMortality = min(1.0, TmpMortality);

						const unsigned int numnum = lc_specie->getTreeNum(j, i);
						
						const double DeadTree = numnum * TmpMortality;

						unsigned int DeadTreeInt = (unsigned int)DeadTree;

						if (DeadTree - DeadTreeInt >= 0.0001 && frand1() < 0.1)
							DeadTreeInt++;

						assert(numnum - DeadTreeInt <= UINT_MAX);
						
						lc_specie->setTreeNum(j, i, (unsigned int)max(0, numnum - DeadTreeInt));
						
						tmpDQ -= lc_growrate * lc_growrate * tmpDQ_term2 * DeadTree;

						DeadTreeInt = min(DeadTreeInt, numnum);

						const int lc_biomasscof = siteptr->specAtt(i)->BioMassCoef;

						tmp_BA = lc_growrate * lc_growrate / 40000 * 3.1415926 * DeadTreeInt;	
						tmp_BIO = exp(GetBiomassData(lc_biomasscof, 1) + GetBiomassData(lc_biomasscof, 2) * log(lc_growrate)) * DeadTreeInt / 1000.00;					

						DTN_onespec_allage += DeadTreeInt;
						DTNBA_onespec_allage += tmp_BA;
						DTNBIO_onespec_allage += tmp_BIO;

						const int lc_output_agerange_limit = GetOutputAgeRangeModuleFlagArray(i - 1, COUNT);
						const int j_m_timestep = j * TimeStep;

						if (ppdp->TrackFlag == 1)
						{
							for (int agerange_index = 1; agerange_index <= lc_output_agerange_limit; agerange_index++)					//add by houbin 11/3/2017
							{
								const int lc_output_agerange_lowlimit = GetOutputAgeRangeModuleFlagArray(i - 1, agerange_index, 0);
								const int lc_output_agerange_uprlimit = GetOutputAgeRangeModuleFlagArray(i - 1, agerange_index, 1);

								if (lc_output_agerange_lowlimit < j_m_timestep && j_m_timestep <= lc_output_agerange_uprlimit)
								{
									const int lc_third_id = (agerange_index - 1) * num_specie + i - 1;

									ppdp->cDeadTreeNum_spec_agerange[Row - 1][Col - 1][lc_third_id] += DeadTreeInt;
									ppdp->cDeadTreeBA_spec_agerange[Row - 1][Col - 1][lc_third_id] += tmp_BA;
									ppdp->cDeadTreeBio_spec_agerange[Row - 1][Col - 1][lc_third_id] += tmp_BIO;
								}
							}
						}
						
					}

					if (ppdp->TrackFlag == 1)
					{
						ppdp->cDeadTreeNum[Row - 1][Col - 1][i - 1] += DTN_onespec_allage;
						ppdp->cDeadTreeBA[Row - 1][Col - 1][i - 1] += DTNBA_onespec_allage;
						ppdp->cDeadTreeBio[Row - 1][Col - 1][i - 1] += DTNBIO_onespec_allage;
					}
					
				}
				else
				{
					for (int j = 1; j <= StartAge; j++)
						lc_specie->setTreeNum(j, i, cellsize_square);
				}
			}

		}
		else
		{
			for (int i = 1; i <= num_specie; i++)
			{
				DTN_onespec_allage = 0;
				DTNBIO_onespec_allage = 0;
				DTNBA_onespec_allage = 0;

				SPECIE* lc_specie = siteptr->SpecieIndex(i);

				if (siteptr->specAtt(i)->SpType >= 0)
				{
					for (int j = 1; j <= StartAge; j++)
					{
						const unsigned int numnum = lc_specie->getTreeNum(j, i);

						const double DeadTree = numnum * GetMortalityRates(i, j, l->ltID);

						unsigned int DeadTreeInt = (unsigned int)DeadTree;

						if (DeadTree - DeadTreeInt >= 0.0001 && frand1() < 0.1)
							DeadTreeInt++;
						
						assert(numnum - DeadTreeInt <= UINT_MAX);

						lc_specie->setTreeNum(j, i, (unsigned int)max(0, numnum - DeadTreeInt));

						DeadTreeInt = min(DeadTreeInt, numnum);

						const int lc_biomasscof = siteptr->specAtt(i)->BioMassCoef;

						const float lc_growrate = GetGrowthRates(i, j, l->ltID);
						tmp_BA = lc_growrate * lc_growrate / 40000 * 3.1415926 * DeadTreeInt;	
						tmp_BIO = exp(GetBiomassData(lc_biomasscof, 1) + GetBiomassData(lc_biomasscof, 2) * log(lc_growrate)) * DeadTreeInt / 1000.00;					

						DTN_onespec_allage += DeadTreeInt;
						DTNBA_onespec_allage += tmp_BA;
						DTNBIO_onespec_allage += tmp_BIO;

						const int lc_output_agerange_limit = GetOutputAgeRangeModuleFlagArray(i - 1, COUNT);
						const int j_m_timestep = j * TimeStep;
						
						if (ppdp->TrackFlag == 1)
						{
							for (int agerange_index = 1; agerange_index <= lc_output_agerange_limit; agerange_index++)					//add by houbin 11/3/2017
							{
								const int lc_output_agerange_lowlimit = GetOutputAgeRangeModuleFlagArray(i - 1, agerange_index, 0);
								const int lc_output_agerange_uprlimit = GetOutputAgeRangeModuleFlagArray(i - 1, agerange_index, 1);

								if (lc_output_agerange_lowlimit < j_m_timestep && j_m_timestep <= lc_output_agerange_uprlimit)
								{
									const int lc_third_id = (agerange_index - 1) * num_specie + i - 1;

									ppdp->cDeadTreeNum_spec_agerange[Row - 1][Col - 1][lc_third_id] += DeadTreeInt;
									ppdp->cDeadTreeBA_spec_agerange[Row - 1][Col - 1][lc_third_id] += tmp_BA;
									ppdp->cDeadTreeBio_spec_agerange[Row - 1][Col - 1][lc_third_id] += tmp_BIO;
								}
							}
						}
						
					}

					if (ppdp->TrackFlag == 1)
					{
						ppdp->cDeadTreeNum[Row - 1][Col - 1][i - 1] += DTN_onespec_allage;
						ppdp->cDeadTreeBA[Row - 1][Col - 1][i - 1] += DTNBA_onespec_allage;
						ppdp->cDeadTreeBio[Row - 1][Col - 1][i - 1] += DTNBIO_onespec_allage;
					}
					
				}
				else
				{
					for (int j = 1; j <= StartAge; j++)
						lc_specie->setTreeNum(j, i, cellsize_square);
				}
			}
		}
	}
}






void SITES::SetOutputGeneralFlagArray(int i, int j, int value)
{
	if (i > (specNum + 1) || j > NumTypes70Output)
		errorSys("Out bound in Flag Array\n", STOP);
	
	OutputGeneralFlagArray[i*NumTypes70Output + j] = value;
}


int SITES::GetOutputGeneralFlagArray(int i, int j)
{
	if (i > (specNum + 1) || j > NumTypes70Output)
		errorSys("Out bound in Flag Array\n", STOP);
	
	return OutputGeneralFlagArray[i*NumTypes70Output + j];
}



void SITES::SetflagAgeOutput(int value)
{
	flagAgeRangeOutput = value;
}


int SITES::GetflagAgeOutput()
{
	return flagAgeRangeOutput;
}


//add by houbin 10/18/2017
void SITES::SetOutputModuleFlagArray(int i, int j, int value)
{
	if (i > specNum + 1 || j > NumTypes70Output)
		errorSys("Out bound in Flag Array\n", STOP);

	OutputModuleFlagArray[i * NumTypes70Output + j] = value;
}

int SITES::GetOutputModuleFlagArray(int i, int j)
{
	if (i > specNum + 1 || j > NumTypes70Output)
		errorSys("Out bound in Flag Array\n", STOP);

	return OutputModuleFlagArray[i * NumTypes70Output + j];
}


void SITES::SetOutputAgeRangeModuleFlagArray(int i, int j, int value)
{
	if (i > specNum + 1 || j > NumTypes70Output)
		errorSys("Out bound in Flag Array\n", STOP);

	OutputAgeRangeModuleFlagArray[i * 500 / TimeStep + j] = value;
}


void SITES::SetOutputAgeRangeModuleFlagArray(int i, int j, int value1, int value2)
{
	if (i > specNum || j > 40)
		errorSys("Out bound in species age range\n", STOP);

	const int index = i * 500 / TimeStep + j * 2 + 2;

	OutputAgeRangeModuleFlagArray[index] = value1;
	OutputAgeRangeModuleFlagArray[index + 1] = value2;
}

int SITES::GetOutputAgeRangeModuleFlagArray(int i, int j)
{
	if (i > specNum)
		errorSys("Out bound in species age range\n", STOP);
	
	return OutputAgeRangeModuleFlagArray[i * 500 / TimeStep + j];
}


int SITES::GetOutputAgeRangeModuleFlagArray(int i, int j, int low_or_high)
{
	if (i > specNum || j > 40)
		errorSys("Out bound in species age range\n", STOP);
	
	return OutputAgeRangeModuleFlagArray[i * 500 / TimeStep + j * 2 + 2 + low_or_high];
}
//add by houbin 10/24/2017



void SITES::SetOutputAgerangeFlagArray(int i, int j, int value)
{
	if (i > (specNum + 1) || j > NumTypes70Output)
		errorSys("Out bound in Flag Array\n", STOP);
	
	OutputAgerangeFlagArray[i*NumTypes70Output + j] = value;
}


int SITES::GetOutputAgerangeFlagArray(int i, int j)
{
	if (i > (specNum + 1) || j > NumTypes70Output)
		errorSys("Out bound in Flag Array\n", STOP);
	
	return OutputAgerangeFlagArray[i*NumTypes70Output + j];
}





void SITES::SetSpeciesAgerangeArray(int specindex, int count, int value1, int value2)
{
	if (specindex > specNum || count > 40)
		errorSys("Out bound in species age range\n", STOP);

	const int index = specindex * 500 / TimeStep + count * 2;

	SpeciesAgerangeArray[index - 1] = value1;
	SpeciesAgerangeArray[index] = value2;
}



void SITES::GetSpeciesAgerangeArray(int specindex, int count, int& value1, int& value2)
{
	if (specindex > specNum || count > 40)
		errorSys("Out bound in species age range\n", STOP);

	const int index = specindex * 500 / TimeStep + count * 2;

	value1 = SpeciesAgerangeArray[index - 1];
	value2 = SpeciesAgerangeArray[index];
}



int SITES::GetAgerangeCount(int specindex)
{
	return SpeciesAgerangeArray[specindex * 500 / TimeStep];
}



void SITES::SetAgerangeCount(int specindex, int count)
{
	SpeciesAgerangeArray[specindex * 500 / TimeStep] = count;
}



void SITES::SetAgeDistStat_YearVal(int specindex, int count, int value1)
{
	if (specindex > specNum || count > 40)
		errorSys("Out bound in species age range\n", STOP);

	AgeDistStat_Year[specindex * 500 / TimeStep + count] = value1;
}



int SITES::GetAgeDistStat_YearVal(int specindex, int count)
{
	if (specindex > specNum || count > 40)
		errorSys("Out bound in species age range\n", STOP);

	return AgeDistStat_Year[specindex * 500 / TimeStep + count];
}



int SITES::GetAgeDistStat_YearCount(int specindex)
{
	return AgeDistStat_Year[specindex * 500 / TimeStep];
}



void SITES::SetAgeDistStat_YearValCount(int specindex, int count)
{
	AgeDistStat_Year[specindex * 500 / TimeStep] = count;
}



/////////////////

void SITES::SetAgeDistStat_AgeRangeVal(int specindex, int count, int value1, int value2)
{
	if (specindex > specNum || count > 40)
		errorSys("Out bound in species age range\n", STOP);

	const int index = specindex * 500 / TimeStep + count * 2;

	AgeDistStat_AgeRange[index - 1] = value1;
	AgeDistStat_AgeRange[index] = value2;
}


void SITES::GetAgeDistStat_AgeRangeVal(int specindex, int count, int& value1, int& value2)
{
	if (specindex > specNum || count > 40)
		errorSys("Out bound in species age range\n", STOP);

	const int index = specindex * 500 / TimeStep + count * 2;

	value1 = AgeDistStat_AgeRange[index - 1];
	value2 = AgeDistStat_AgeRange[index];
}



int SITES::GetAgeDistStat_AgeRangeCount(int specindex)
{
	return AgeDistStat_AgeRange[specindex * 500 / TimeStep];
}



void SITES::SetAgeDistStat_AgeRangeCount(int specindex, int count)
{
	AgeDistStat_AgeRange[specindex * 500 / TimeStep] = count;
}







void SITES::Read70OutputOption(char *FileName)
{
	char temp[100];

	printf("reading Landis 70 output customization file...\n");

	FILE *fp;
	if ((fp = LDfopen(FileName, 1)) == NULL)
		errorSys("Can not open Read70OutputOption file", STOP);

	////general every species////

	for (int i = 0; i < specNum; i++)
	{
		fscanc(fp, "%s", temp);

		//TPA
		fscanc(fp, "%s", temp);

		if (!strcmp(temp, "Y"))
			SetOutputGeneralFlagArray(i, TPA, 1);
		else
			SetOutputGeneralFlagArray(i, TPA, 0);

		// BA
		fscanc(fp, "%s", temp);

		if (!strcmp(temp, "Y"))
			SetOutputGeneralFlagArray(i, BA, 1);
		else
			SetOutputGeneralFlagArray(i, BA, 0);

		//BIO
		fscanc(fp, "%s", temp);

		if (!strcmp(temp, "Y"))
			SetOutputGeneralFlagArray(i, Bio, 1);
		else
			SetOutputGeneralFlagArray(i, Bio, 0);

		//IV
		fscanc(fp, "%s", temp);

		if (!strcmp(temp, "Y"))
			SetOutputGeneralFlagArray(i, IV, 1);
		else
			SetOutputGeneralFlagArray(i, IV, 0);

		//SEEDS
		fscanc(fp, "%s", temp);

		if (!strcmp(temp, "Y"))
			SetOutputGeneralFlagArray(i, Seeds, 1);
		else
			SetOutputGeneralFlagArray(i, Seeds, 0);
		
		//RD
		fscanc(fp, "%s", temp);

		if (!strcmp(temp, "Y"))
			SetOutputGeneralFlagArray(i, RDensity, 1);
		else
			SetOutputGeneralFlagArray(i, RDensity, 0);
	}

	///////below is for general total//////////

	fscanc(fp, "%s", temp);
	fscanc(fp, "%s", temp);

	if (!strcmp(temp, "Y"))
		SetOutputGeneralFlagArray(specNum, TPA, 1);
	else
		SetOutputGeneralFlagArray(specNum, TPA, 0);


	fscanc(fp, "%s", temp);

	if (!strcmp(temp, "Y"))
		SetOutputGeneralFlagArray(specNum, BA, 1);
	else
		SetOutputGeneralFlagArray(specNum, BA, 0);


	fscanc(fp, "%s", temp);

	if (!strcmp(temp, "Y"))
		SetOutputGeneralFlagArray(specNum, Bio, 1);
	else
		SetOutputGeneralFlagArray(specNum, Bio, 0);


	fscanc(fp, "%s", temp);

	if (!strcmp(temp, "Y"))
		SetOutputGeneralFlagArray(specNum, Car, 1);
	else
		SetOutputGeneralFlagArray(specNum, Car, 0);


	fscanc(fp, "%s", temp);

	if (!strcmp(temp, "Y"))
		SetOutputGeneralFlagArray(specNum, RDensity, 1);
	else
		SetOutputGeneralFlagArray(specNum, RDensity, 0);

	/////////////////////

	fscanc(fp, "%s", temp);

	if (!strcmp(temp, "Y"))
		SetflagAgeOutput(1);
	else
		SetflagAgeOutput(0);

	/////below is age range for every species///////

	for (int i = 0; i < specNum; i++)
	{
		fscanc(fp, "%s", temp);
		fscanc(fp, "%s", temp);

		if (!strcmp(temp, "Y"))
			SetOutputAgerangeFlagArray(i, TPA, 1);
		else
			SetOutputAgerangeFlagArray(i, TPA, 0);
		

		fscanc(fp, "%s", temp);

		if (!strcmp(temp, "Y"))
			SetOutputAgerangeFlagArray(i, BA, 1);
		else
			SetOutputAgerangeFlagArray(i, BA, 0);

		fscanc(fp, "%s", temp);

		if (!strcmp(temp, "Y"))
			SetOutputAgerangeFlagArray(i, Bio, 1);
		else
			SetOutputAgerangeFlagArray(i, Bio, 0);


		fscanc(fp, "%s", temp);

		if (!strcmp(temp, "Y"))
			SetOutputAgerangeFlagArray(i, IV, 1);
		else
			SetOutputAgerangeFlagArray(i, IV, 0);
	}

	///////below is for age range total//////////

	fscanc(fp, "%s", temp);
	fscanc(fp, "%s", temp);

	if (!strcmp(temp, "Y"))
		SetOutputAgerangeFlagArray(specNum, TPA, 1);
	else
		SetOutputAgerangeFlagArray(specNum, TPA, 0);

	fscanc(fp, "%s", temp);

	if (!strcmp(temp, "Y"))
		SetOutputAgerangeFlagArray(specNum, BA, 1);
	else
		SetOutputAgerangeFlagArray(specNum, BA, 0);

	fscanc(fp, "%s", temp);

	if (!strcmp(temp, "Y"))
		SetOutputAgerangeFlagArray(specNum, Bio, 1);
	else
		SetOutputAgerangeFlagArray(specNum, Bio, 0);

	fscanc(fp, "%s", temp);

	if (!strcmp(temp, "Y"))
		SetOutputAgerangeFlagArray(specNum, Car, 1);
	else
		SetOutputAgerangeFlagArray(specNum, Car, 0);

	fscanc(fp, "%s", temp);

	if (!strcmp(temp, "Y"))
		SetOutputAgerangeFlagArray(specNum, RDensity, 1);
	else
		SetOutputAgerangeFlagArray(specNum, RDensity, 0);

	////////////////////////

	for (int i = 0; i < specNum; i++)
	{
		int count, value1, value2;
		char ch;

		fscanc(fp, "%s", temp);
		fscanf(fp, "%d", &count);

		SetAgerangeCount(i, count);

		for (int j = 1; j <= count; j++)
		{
			fscanf(fp, "%d", &value1);
			fscanf(fp, "%c", &ch);
			fscanf(fp, "%d", &value2);

			SetSpeciesAgerangeArray(i, j, value1, value2);
		}
	}





	fscanc(fp, "%s", temp);

	if (strcmp(temp, "N/A") != 0)
	{
		Flag_AgeDistStat = 1;

		FILE *fpAgeDist;
		
		if ((fpAgeDist = LDfopen(temp, 1)) == NULL)
			errorSys("Can not open Read70OutputOption file", STOP);

		for (int i = 0; i < specNum; i++)
		{
			int count, value1;

			fscanc(fpAgeDist, "%s", temp);
			fscanf(fpAgeDist, "%d", &count);

			SetAgeDistStat_YearValCount(i, count);

			for (int j = 1; j <= count; j++)
			{
				fscanf(fpAgeDist, "%d", &value1);

				SetAgeDistStat_YearVal(i, j, value1);
			}
		}

		for (int i = 0; i < specNum; i++)
		{
			int count, value1, value2;
			char ch;

			fscanc(fpAgeDist, "%s", temp);
			fscanf(fpAgeDist, "%d", &count);

			SetAgeDistStat_AgeRangeCount(i, count);

			for (int j = 1; j <= count; j++)
			{
				fscanf(fpAgeDist, "%d", &value1);
				fscanf(fpAgeDist, "%c", &ch);
				fscanf(fpAgeDist, "%d", &value2);

				SetAgeDistStat_AgeRangeVal(i, j, value1, value2);
			}
		}

		fclose(fpAgeDist);

	}
	else
		Flag_AgeDistStat = 0;

	/////////////////////// chen houbin
	fscanc(fp, "%s", temp);
	if (!strcmp(temp, "Y"))
	{
		fscanc(fp, "%s", temp);
		if (!strcmp(temp, "Y"))
			OutputFireFlag = 1; //output mortality for fire module if it's on
		else
			OutputFireFlag = 0;

		fscanc(fp, "%s", temp);
		if (!strcmp(temp, "Y"))
		{
			OutputFuelFlag = 1;
		}
		else
		{
			OutputFuelFlag = 0;
		}
		fscanc(fp, "%s", temp);
		if (!strcmp(temp, "Y"))
		{
			OutputHarvestFlag = 1;
		}
		else
		{
			OutputHarvestFlag = 0;
		}
		fscanc(fp, "%s", temp);
		if (!strcmp(temp, "Y"))
		{
			OutputSuccessionFlag = 1;
		}
		else
		{
			OutputSuccessionFlag = 0;
		}
	}
	else
	{
		OutputFireFlag = 0;
		OutputFuelFlag = 0;
		OutputHarvestFlag = 0;
		OutputSuccessionFlag = 0;
		fscanc(fp, "%s", temp);
		fscanc(fp, "%s", temp);
		fscanc(fp, "%s", temp);
		fscanc(fp, "%s", temp);
	}

	//////////////////////module output option	add by houbin 10/19/2017
	for (int i = 0; i < specNum; i++)
	{
		fscanc(fp, "%s", temp);
		fscanc(fp, "%s", temp);

		if (!strcmp(temp, "Y"))
			SetOutputModuleFlagArray(i, DTN, 1);
		else
			SetOutputModuleFlagArray(i, DTN, 0);

		fscanc(fp, "%s", temp);
		
		if (!strcmp(temp, "Y"))
			SetOutputModuleFlagArray(i, DTNBA, 1);
		else
			SetOutputModuleFlagArray(i, DTNBA, 0);

		fscanc(fp, "%s", temp);
		if (!strcmp(temp, "Y"))
			SetOutputModuleFlagArray(i, DTNBIO, 1);
		else
			SetOutputModuleFlagArray(i, DTNBIO, 0);
	}
	//////////////////////		add by houbin 10 / 19 / 2017

	//////////////////////		module output option in age range add by houbin 10/23/2017
	fscanc(fp, "%s", temp);
	if (!strcmp(temp, "Y"))
	{
		fscanc(fp, "%s", temp);
		if (!strcmp(temp, "Y"))
		{
			OutputAgeRangeFireFlag = 1;
		}
		else
		{
			OutputAgeRangeFireFlag = 0;
		}
		fscanc(fp, "%s", temp);
		if (!strcmp(temp, "Y"))
		{
			OutputAgeRangeFuelFlag = 1;
		}
		else
		{
			OutputAgeRangeFuelFlag = 0;
		}
		fscanc(fp, "%s", temp);
		if (!strcmp(temp, "Y"))
		{
			OutputAgeRangeHarvestFlag = 1;
		}
		else
		{
			OutputAgeRangeHarvestFlag = 0;
		}
		fscanc(fp, "%s", temp);
		if (!strcmp(temp, "Y"))
		{
			OutputAgeRangeSuccessionFlag = 1;
		}
		else
		{
			OutputAgeRangeSuccessionFlag = 0;
		}
	}
	else
	{
		OutputAgeRangeFireFlag = 0;
		OutputAgeRangeFuelFlag = 0;
		OutputAgeRangeHarvestFlag = 0;
		OutputAgeRangeSuccessionFlag = 0;
		fscanc(fp, "%s", temp);
		fscanc(fp, "%s", temp);
		fscanc(fp, "%s", temp);
		fscanc(fp, "%s", temp);
	}

	//////////////////////module output option	add by houbin 10/19/2017
	for (int i = 0; i < specNum; i++)
	{
		fscanc(fp, "%s", temp);
		fscanc(fp, "%s", temp);
		
		if (!strcmp(temp, "Y"))
			SetOutputAgeRangeModuleFlagArray(i, DTN, 1);
		else
			SetOutputAgeRangeModuleFlagArray(i, DTN, 0);
		
		fscanc(fp, "%s", temp);

		if (!strcmp(temp, "Y"))
			SetOutputAgeRangeModuleFlagArray(i, DTNBA, 1);
		else
			SetOutputAgeRangeModuleFlagArray(i, DTNBA, 0);
		
		fscanc(fp, "%s", temp);

		if (!strcmp(temp, "Y"))
			SetOutputAgeRangeModuleFlagArray(i, DTNBIO, 1);
		else
			SetOutputAgeRangeModuleFlagArray(i, DTNBIO, 0);

		int count = 0;
		fscanf(fp, "%d", &count);
		SetOutputAgeRangeModuleFlagArray(i, COUNT, count);

		for (int j = 1; j <= count; j++)
		{
			int value1, value2;
			char ch;

			fscanf(fp, "%d", &value1);
			fscanf(fp, "%c", &ch);
			fscanf(fp, "%d", &value2);

			SetOutputAgeRangeModuleFlagArray(i, j, value1, value2);
		}
	}
	//////////////////////		add by houbin 10/23/2017

	fclose(fp);
}






float SITES::GetGrowthRates(int spec, int year, int landtype_index)
{
	const int consterm1 = 320 / TimeStep + 1;
	const int consterm2 = specNum * consterm1;
	const int consterm3 = (spec - 1) * consterm1 + year - 1;


	if (spec < 1 || spec > specNum || spec * year >= consterm2)
		errorSys("Out bound in GetGrowthRates\n", STOP);
	
	if (GrowthRates_file.size() == 0)
		return GrowthRates[consterm3];		
	else
	{
		if (landtype_index >= GrowthRates_file.size())
			errorSys("Out bound in GetGrowthRates\n", STOP);

		return GrowthRates_file.at(landtype_index)[consterm3];
	}
}







float SITES::GetMortalityRates(int spec, int year, int landtype_index)
{
	const int consterm1 = 320 / TimeStep + 1;
	const int consterm2 = specNum * consterm1;
	const int consterm3 = (spec - 1) * consterm1 + year - 1;

	if (spec<1 || spec>specNum || spec*year >= consterm2)
		errorSys("Out bound in GetMortalityRates\n", STOP);
	
	if (MortalityRates_file.size() == 0)
		return MortalityRates[consterm3];	
	else
	{
		if (landtype_index >= MortalityRates_file.size())
			errorSys("Out bound in MortalityRates\n", STOP);

		return MortalityRates_file.at(landtype_index)[consterm3];			
	}
}






float SITES::GetVolume(int spec, int year, int landtype_index)
{
	const int consterm1 = 320 / TimeStep + 1;
	const int consterm2 = specNum * consterm1;
	const int consterm3 = (spec - 1) * consterm1 + year - 1;

	if (spec<1 || spec>specNum || spec*year >= consterm2)
		errorSys("Out bound in GetVolume\n", STOP);
	
	if (Volume_file.size() == 0)
		return Volume[consterm3];		
	else
	{
		if (landtype_index >= Volume_file.size())
			errorSys("Out bound in Volume\n", STOP);

		return Volume_file.at(landtype_index)[consterm3];
	}
}





void SITES::SetGrowthRates(int flag, int spec, int year, float value, int index_landtype)
{
	const int consterm1 = 320 / TimeStep + 1;
	const int consterm2 = specNum * consterm1;
	const int consterm3 = (spec - 1) * consterm1 + year - 1;

	if (spec < 1 || spec > specNum || spec * year >= consterm2)
		errorSys("Out bound in SetGrowthRates\n", STOP);
	
	if (flag == 0)
	{
		GrowthRates[consterm3] = value;
	}
	else
	{
		if (index_landtype == GrowthRates_file.size())
		{
			float *temp = new float[consterm2];
			GrowthRates_file.push_back(temp);
		}
		
		GrowthRates_file.at(index_landtype)[consterm3] = value;
	}
}




void SITES::SetMortalityRates(int flag, int spec, int year, float value, int index_landtype)
{
	const int consterm1 = 320 / TimeStep + 1;
	const int consterm2 = specNum * consterm1;
	const int consterm3 = (spec - 1) * consterm1 + year - 1;

	if (spec < 1 || spec > specNum || spec * year >= consterm2)
		errorSys("Out bound in SetMortalityRates\n", STOP);
	
	if (flag == 0)
	{
		MortalityRates[consterm3] = value;
	}
	else
	{
		if (index_landtype == MortalityRates_file.size())
		{
			float *temp = new float[consterm2];
			MortalityRates_file.push_back(temp);
		}
		
		MortalityRates_file.at(index_landtype)[consterm3] = value;
	}
}




void SITES::SetVolume(int growthrate_flag, int spec, int year, float value, int index_landtype)
{
	const int consterm1 = 320 / TimeStep + 1;
	const int consterm2 = specNum * consterm1;
	const int consterm3 = (spec - 1) * consterm1 + year - 1;

	if (spec < 1 || spec > specNum || spec * year >= consterm2)
		errorSys("Out bound in Volume\n", STOP);
	
	if (growthrate_flag == 0)
	{
		Volume[consterm3] = value;
	}
	else
	{
		if (index_landtype == Volume_file.size())
		{
			float *temp = new float[consterm2];
			Volume_file.push_back(temp);
		}
		
		Volume_file.at(index_landtype)[consterm3] = value;
	}
}




void SITES::GetSeedDispersalProbability(int snr, int snc)
{
	seed_prob.cal_seeding_prob(CellSize, specNum, speces_maxD, specesefctD, snr, snc);
}




void SITES::GetSeedDispersalonSite(DispersalParam& param)
{
	const int norm_spec_id = param.spec_id - 1;
	
	unsigned long** numseeds = param.gl_numseeds;

	int vector_size = int(param.cols.size());
	assert(vector_size == param.rows.size());
	assert(vector_size == param.lc_numseeds.size());

	for(int i=0; i<vector_size; i++)
	{
		int currow = (param.rows)[i];
		int curcol = (param.cols)[i];
		unsigned long lc_numseed = (param.lc_numseeds)[i];

		seed_prob.process_seeds_on_site(norm_spec_id, currow, curcol, numseeds, lc_numseed);
	}
}







//Reads or calculates species' growth rates at corresponding ages,  no return value
//Save the DBH and Age in a two dimension matrix, GrowthRates
void SITES::GetSpeciesGrowthRates(char* fileGrowthRates, int growthrateflag)
{
	SITE *sitetmp = SITES::operator()(1, 1);

	GrowthFlag = growthrateflag;

	if (GrowthFlag == 0)
	{
		for (int i = 1; i <= specNum; i++)
		{
			SPECIESATTR* specatt_i = sitetmp->specAtt(i);
			
			const int lc_long  = specatt_i->longevity;
			const int lc_maxdq = specatt_i->MaxDQ;
			const int term1 = lc_long / TimeStep + 1;

			const double term2 = -0.088 * 100 * TimeStep / lc_long;
			const double term3 =  -0.12 * 100 * TimeStep / lc_long;


			if (specatt_i->SpType == 0)
			{
				for (int j = 1; j <= term1; j++)
				{
					const float temp = exp(-11.37 * exp(j * term2)) * lc_maxdq;

					SetGrowthRates(GrowthFlag, i, j, temp, 0);
				}
			}
			else if (specatt_i->SpType == 1)
			{
				for (int j = 1; j <= term1; j++)
				{
					const float temp = exp(-11.7 * exp(j * term3)) * lc_maxdq;

					SetGrowthRates(GrowthFlag, i, j, temp, 0);
				}
			}
			else
			{
				for (int j = 1; j <= term1; j++)
					SetGrowthRates(GrowthFlag, i, j, lc_maxdq, 0);
			}
		}
	}
	else
	{
		//Read data from file
		FILE *fp;
		if ((fp = LDfopen(fileGrowthRates, 1)) == NULL)
			errorSys("Specie Growth Rates file not found.", STOP);
		
		int numLU = 0;

		while (!LDeof(fp))
		{
			for (int i = 1; i <= specNum; i++)
			{
				const int j_end = sitetmp->specAtt(i)->longevity / TimeStep;

				for (int j = 1; j <= j_end; j++)
				{
					float temp = 0;

					if (fscanc(fp, "%f", &temp) != 1)
						errorSys("Error reading in growth rates from file.", STOP);

					SetGrowthRates(GrowthFlag, i, j, temp, numLU);
				}
			}

			numLU++;
		}

		fclose(fp);
	}
}






//Reads or calculates species' mortality rates at corresponding ages, No Return Value
//Save the DBH and Age in a two dimension matrix, MortalityRates
void SITES::GetSpeciesMortalityRates(char* fileMortalityRates, int mortalityrateflag)
{
	MortalityFlag = mortalityrateflag;

	if (MortalityFlag != 0)
	{
		//read data from file
		FILE *fp;
		if ((fp = LDfopen(fileMortalityRates, 1)) == NULL)
			errorSys("Specie Mortality Rates file not found.", STOP);

		//specific MortalityRates for different landtypes go here
		SITE *sitetmp = SITES::operator()(1, 1);

		int numLU = 0;

		while (!LDeof(fp))
		{
			for (int i = 1; i <= specNum; i++)
			{
				const int consterm = sitetmp->specAtt(i)->longevity / TimeStep;

				for (int j = 1; j <= consterm; j++)
				{
					float temp;

					if (fscanc(fp, "%f", &temp) != 1)
						errorSys("Error reading in mortality rates from file.", STOP);
					
					SetMortalityRates(MortalityFlag, i, j, temp, numLU);
				}
			}

			numLU++;
		}

		fclose(fp);
	}
}





void SITES::GetVolumeRead(char* fileVolumeFlag, int VolumeFlag_Flag)
{
	SITE *sitetmp = SITES::operator()(1, 1);

	float * VolumeTemp = new float[specNum * 6];

	VolumeFlag = VolumeFlag_Flag;

	static const double constval1 = pow(0.3048, 3.0);

	if (VolumeFlag == 0)
	{
		for (int i = 1; i <= specNum; i++)
		{
			const int consterm = sitetmp->specAtt(i)->longevity / TimeStep;

			for (int j = 1; j <= consterm; j++)
			{
				if (GrowthFlag == 0)
				{
					float temp = (-61.9 + 6.83 * GetGrowthRates(i, j, 0) / 2.54) * constval1;
					SetVolume(GrowthFlag, i, j, temp, 0);
				}
				else
				{
					const int range = GrowthRates_file.size();

					for (int i_growth = 0; i_growth < range; i_growth++)
					{
						float temp = (-61.9 + 6.83 * GetGrowthRates(i, j, i_growth) / 2.54) * constval1;
						SetVolume(GrowthFlag, i, j, temp, i_growth);
					}
				}
			}
		}
	}
	else
	{	
		//read data from file
		FILE *fp, *fpTreeheight;

		if ((fp = LDfopen(fileVolumeFlag, 1)) == NULL)
			errorSys("fileVolumeFlag file not found.", STOP);

		fscanc(fp, "%d", VolumeFlag);

		char FilenameHeights[FNSIZE];

		fscanc(fp, "%s", FilenameHeights);

		if ((fpTreeheight = LDfopen(FilenameHeights, 1)) == NULL)
			errorSys("FilenameHeights file not found.", STOP);



		if (VolumeFlag == 1)
		{
			for (int i = 1; i <= specNum; i++)
			{
				const int index_part = (i - 1) * 6;

				for (int j = 1; j <= 6; j++)			
					fscanc(fp, "%f", &VolumeTemp[index_part + j - 1]);				
			}

			for (int i = 1; i <= specNum; i++)
			{
				const int index_part = (i - 1) * 6;

				const int consterm = sitetmp->specAtt(i)->longevity / TimeStep;

				for (int j = 1; j <= consterm; j++)
				{
					float TreeHeight;
					fscanc(fpTreeheight, "%f", &TreeHeight);

					const float vol0 = VolumeTemp[index_part];
					const float vol1 = VolumeTemp[index_part + 1];
					const float vol2 = VolumeTemp[index_part + 2];
					const float vol3 = VolumeTemp[index_part + 3];
					const float vol4 = VolumeTemp[index_part + 4];
					const float vol5 = VolumeTemp[index_part + 5];

					const double constval2 = pow(TreeHeight, vol5);

					if (GrowthFlag == 0)
					{
						const float lc_growrate0 = GetGrowthRates(i, j, 0);

						float temp = vol0 + vol1 * pow(lc_growrate0, vol2) + vol3 * pow(lc_growrate0, vol4) * constval2;
						SetVolume(GrowthFlag, i, j, temp, 0);
					}
					else
					{
						const int range = GrowthRates_file.size();
						
						for (int i_growth = 0; i_growth < range; i_growth++)
						{
							const float lc_growrate = GetGrowthRates(i, j, i_growth);

							float temp = vol0 + vol1 * pow(lc_growrate, vol2) + vol3 * pow(lc_growrate, vol4) * constval2;
							SetVolume(GrowthFlag, i, j, temp, i_growth);
						}
					}
				}
			}
		}
		else
		{
			errorSys("compute volume wrong", STOP);

			for (int i = 1; i <= specNum; i++)
			{
				const int consterm = sitetmp->specAtt(i)->longevity / TimeStep;

				for (int j = 1; j <= consterm; j++)
				{
					int temp;

					if (fscanc(fp, "%d", &temp) != 1)
						errorSys("Error reading in height rates from file.", STOP);


					if (GrowthFlag == 0)
					{
						SetVolume(GrowthFlag, i, j, temp, 0);
					}
					else
					{
						for (int i_growth = 0; i_growth < GrowthRates_file.size(); i_growth++)						
							SetVolume(GrowthFlag, i, j, temp, i_growth);
					}
				}
			}
		}

		fclose(fpTreeheight);
		fclose(fp);
	}

	delete[] VolumeTemp;
}






float SITES::GetBiomassData(int i, int j)
{
	if (i > biomassNum || j < 1 || j > 2)
		errorSys("index error at GetBiomass", STOP);

	const int index = (i - 1) * 2 + j - 1;

	return biomassData[index];
}




void SITES::SetBiomassData(int i, int j, float value)
{
	if (i > biomassNum || j < 1 || j > 2)
		errorSys("index error at SetBiomass", STOP);

	const int index = (i - 1) * 2 + j - 1;

	biomassData[index] = value;
}



void SITES::SetBiomassNum(int num)
{
	biomassNum = num;

	biomassData = new float[num * 2];
}







//read biomass coefficients from a file into matrix, (float) BioMassData(int ID,2). No Return Value
//Read a ID first from the file, and ID is the size of BioMassData;
//Read the two variable in to BioMassData(v1,v2)
void SITES::BiomassRead(FILE *fp)
{
	int temp;
	fscanc(fp, "%d", &temp);

	SetBiomassNum(temp);

	float value;
	fscanc(fp, "%f", &value);

	SetBiomassThreshold(value);

	for (int i = 1; i <= temp; i++)
	{
		for (int j = 1; j <= 2; j++)
		{
			if (fscanc(fp, "%f", &value) == EOF)
				errorSys("Error reading Biomassfile content", STOP);

			SetBiomassData(i, j, value);
		}
	}
}



int SITES::GetMaxShadeTolerance()
{
	return MaxShadeTolerance;
}




void SITES::SetBiomassThreshold(float num)
{
	BiomassThreshold = num;
}


float SITES::GetBiomassThreshold()
{
	return BiomassThreshold;
}





void SITES::BefStChg(int i, int j) {}
void SITES::AftStChg(int i, int j) {}





//when there is a new site during succession or whatever, we need to 
//check if the new site already exists, if yes combine with existing one
//if not insert to the position according to sort
void SITES::SITE_insert(int pos_sortIndex, SITE *site, int i, int j)
{
	int ifexist = 0, pos = 0;

	const int x = (i - 1) * columns + j - 1;

	SITE_LocateinSortIndex(site, &pos, &ifexist);

	if (ifexist)
	{
		map[x] = SortedIndex.at(pos);

		map[x]->numofsites++;
	}
	else
	{
		SITE *temp = new SITE;

		*temp = *site;

		temp->numofsites = 1;

		map[x] = temp;

		SortedIndex.insert(SortedIndex.begin() + pos, temp);
	}
}





//Find if a new site exists in sorted list
//If a new site exists, find its location and set *ifexist as 1
//if this no site matches this one, find location before which new site pointer should be inserted
int SITES::SITE_LocateinSortIndex(SITE *site, int *pos, int *ifexist)
{
	int end = SortedIndex.size();

	if (end == 0)  { printf("No site at all wrong wrong wrong\n"); return -1; }

	end--;

	int begin = 0;
	int mid = (begin + end) / 2;

	SITE* temp = SortedIndex.at(mid);

	int temp_flag;

	*ifexist = 0;

	while (begin < end)
	{
		temp_flag = SITE_compare(site, temp);

		if (temp_flag == 0)
		{
			*ifexist = 1;

			*pos = mid;

			return 1;
		}
		else if (temp_flag == 1)
		{
			begin = mid + 1;

			mid = (begin + end) / 2;
		}
		else if (temp_flag == 2)
		{
			end = mid - 1;

			mid = (begin + end) / 2;
		}
		else
		{
			return -1;
		}

		temp = SortedIndex.at(mid);
	}

	temp_flag = SITE_compare(site, temp);

	if (temp_flag == 0)
	{
		*ifexist = 1;

		*pos = mid;

		return 1;
	}
	else if (temp_flag == 1)
	{
		*ifexist = 0;

		*pos = mid + 1;

		return 0;
	}
	else if (temp_flag == 2)
	{
		*ifexist = 0;

		*pos = mid;

		return 0;
	}
	else
	{
		return -1;
	}

}

