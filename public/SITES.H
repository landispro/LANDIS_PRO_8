#ifndef SITES_H
#define SITES_H



#include <stdio.h>
#include <cassert>
#include <math.h>
#include <vector>
#include <list>

#include "site.h"
#include "landus.h"
#include "seeding_prob.h"
#include "PDP.h"


#define NumTypes70Output 8 //add DBH output option on Oct 19 2010
#define NumTypesAgeRange70 8
#define NumTypes70OutputUsing 7
#define NumTypesAgeRange70Using 6




#ifdef LANDIS40CORE_EXPORTS
	#define DLLCLASS   __declspec( dllexport )
#else
	#define DLLCLASS   __declspec( dllimport )
#endif


template class DLLCLASS std::allocator<float *>;
template class DLLCLASS std::vector<float *>;
template class DLLCLASS std::vector<SITE *>;

typedef std::vector<SITE *>  SortedSITE;




struct SiteDynmParam
{
	int RDflag, row, col;
	unsigned long long cpu_id;
	PDP* Ppdp;
};


struct SiteDynParmVec
{
	unsigned long long cpu_id;
	PDP* Ppdp;
	std::vector<int> RDflags;
	std::vector<int> rows;
	std::vector<int> cols;	
};


struct DispersalParam
{
	unsigned long long spec_id;
	std::vector<int> rows;
	std::vector<int> cols;
	std::vector<unsigned long> lc_numseeds;

	int cpu_id;
	unsigned long** gl_numseeds;
};




class DLLCLASS SITES
{
public:

	//The first parameter is the number of species in the model.  
	//The other parameters dimension the number of rows and columns on the map.
	SITES(int, int, int);             

	SITES();
	//Initial values must be set later by the dim method.  
	//This is useful so that a SITES object may be declared statically but set up dynamically.

	~SITES();

	void dim(int, int, int);            //This will set the number of species,
	void set_speces_maxD(const int* speces_maxD_in);
	void init_seeding_prob4square();

	//rows and columns in the map.

	//This will return a pointer to a singular site. 
	// It is referrenced by row and column number.
	SITE*  operator() (int, int);

	void read(FILE*);                 //Read in all site data. //need to change by Qia Oct 07 2008

	void write(FILE*);                //Write out all site data. //need to change by Qia Oct 07 2008

	void dump();                      //Dump set of site data to the CRT.



	// Compare two sites to see the relation between them
	// return 0:equal; return 1: site1 is bigger; return 2: site2 is bigger; -1: error
	int SITE_compare(SITE *site1, SITE *site2);
	int SITE_compare(int site1_x, int site1_y, int site2_x, int site2_y);

	void SITE_sort();				  // sort the pointers to sites



	void SITE_insert(int pos_sortIndex, SITE *site, int i, int j);					// insert a site in to list



	int SITE_delete(int pos_sortIndex, SITE *site, int i, int j);				// remove a site from list

	int SITE_LocateinSortIndex(SITE *site, int *pos, int *ifexist);


	void fillinSitePt(int i, int j, SITE *site);

	SITE * locateSitePt(int i, int j);

	void fillinLanduPt(int i, int j, LANDUNIT *landUnitPt);

	LANDUNIT * locateLanduPt(int i, int j);

	void BefStChg(int i, int j);

	void AftStChg(int i, int j);

	void setHeader(unsigned long[]); //Set the header info for sites

	unsigned long* getHeader();		 //Copy the header info of sites.

	int number();			  				 //This will return the total number of sites.

	int numRows();                    //Returns number of rows of sites.

	int numColumns();                 //Returns number of columns of sites.

	int numActive();                  //Returns number of active sites on the map.

	int inBounds(int r, int c);

	void SiteDynamics(SiteDynmParam* param);
	//void SiteDynamics(const int RDflag, const int Row, const int Col);

	void GetRDofSite(int Row, int Col);

	void MaxAgeofSite(SITE *siteptr);

	// void GetSeedNumberOnSite_type1(int Row, int Col, int species_id);
	void GetSeedNumberOnSite_type2(int Row, int Col, int species_id);

	void NaturalMortality(SITE *siteptr, int Row, int Col, int StartAge, const PDP* ppdp);

	//int Get_scsssd_size(int k);

	void GetSeedDispersalProbability(int snr, int snc);

	void GetSeedDispersalonSite(DispersalParam& param);

	void GetSpeciesGrowthRates(char* fileGrowthRates, int growthrateflag);

	void GetSpeciesMortalityRates(char* fileMortalityRates, int mortalityrateflag);

	void GetVolumeRead(char*, int);

	float GetSeedRain(int spec, int Distance);

	float GetGrowthRates(int spec, int year, int landtype_index);

	float GetMortalityRates(int spec, int year, int landtype_index);

	float GetVolume(int spec, int year, int landtype_index);

	void SetSeedRain(int spec, int Distance, float value);

	void SetGrowthRates(int flag, int spec, int year, float value, int index_landtype);

	void SetMortalityRates(int flag, int spec, int year, float value, int index_landtype);

	void SetVolume(int growthrate_flag, int spec, int year, float value, int index_landtyp);


	float GetBiomassData(int i, int j);

	void SetBiomassData(int i, int j, float value);

	void SetBiomassNum(int num);

	void SetBiomassThreshold(float num);

	float GetBiomassThreshold();

	void BiomassRead(FILE *fp);

	void GetMatureTree();//Add on July 10 2009

	int GetMaxShadeTolerance();

	void Selfthinning(SITE* siteptr, LANDUNIT *l, int row, int col, const PDP* ppdp);

	void SeedGermination(SITE* siteptr, LANDUNIT *l, int RDFlag);

	void NaturalMortality_killbytargetRD(SITE * siteptr, int Row, int Col, double targetRD);


	void set_all_speces_Dist(int* speces_maxD_in, int* specesefctD_in);

	void SetAgeDistStat_YearVal(int specindex, int count, int value1);



	int GetAgeDistStat_YearVal(int specindex, int count);





	int GetAgeDistStat_YearCount(int specindex);



	void SetAgeDistStat_YearValCount(int specindex, int count);



	void SetAgeDistStat_AgeRangeVal(int specindex, int count, int value1, int value2);

	void GetAgeDistStat_AgeRangeVal(int specindex, int count, int& value1, int& value2);



	int GetAgeDistStat_AgeRangeCount(int specindex);



	void SetAgeDistStat_AgeRangeCount(int specindex, int count);


	void Wind70outputdim();

	void Harvest70outputdim();

	void Fire70outputdim();

	void Fuel70outputdim();

	void BDA70outputdim();



	double Wind70outputGetBiomassvalue(int i, int j);

	double Harvest70outputGetBiomassvalue(int i, int j);

	double Fire70outputGetBiomassvalue(int i, int j);

	double Fuel70outputGetBiomassvalue(int i, int j);

	double BDA70outputGetBiomassvalue(int i, int j);



	double Wind70outputGetCarbonvalue(int i, int j);

	double Harvest70outputGetCarbonvalue(int i, int j);

	double Fire70outputGetCarbonvalue(int i, int j);

	double Fuel70outputGetCarbonvalue(int i, int j);

	double BDA70outputGetCarbonvalue(int i, int j);



	void Wind70outputIncreaseBiomassvalue(int i, int j, double value);

	void Harvest70outputIncreaseBiomassvalue(int i, int j, double value);

	void Fire70outputIncreaseBiomassvalue(int i, int j, double value);

	void Fuel70outputIncreaseBiomassvalue(int i, int j, double value);

	void BDA70outputIncreaseBiomassvalue(int i, int j, double value);



	void Wind70outputIncreaseCarbonvalue(int i, int j, double value);

	void Harvest70outputIncreaseCarbonvalue(int i, int j, double value);

	void Fire70outputIncreaseCarbonvalue(int i, int j, double value);

	void Fuel70outputIncreaseCarbonvalue(int i, int j, double value);

	void BDA70outputIncreaseCarbonvalue(int i, int j, double value);

	void SetOutputGeneralFlagArray(int i, int j, int value);

	int GetOutputGeneralFlagArray(int i, int j);



	void SetflagAgeOutput(int value);

	int GetflagAgeOutput();

	void SetOutputModuleFlagArray(int i, int j, int value);
	int GetOutputModuleFlagArray(int i, int j);
	void SetOutputAgeRangeModuleFlagArray(int i, int j, int value);
	void SetOutputAgeRangeModuleFlagArray(int i, int j, int value1, int value2);
	int GetOutputAgeRangeModuleFlagArray(int i, int j);
	int GetOutputAgeRangeModuleFlagArray(int i, int j, int low_or_high);



	void SetOutputAgerangeFlagArray(int i, int j, int value);

	int GetOutputAgerangeFlagArray(int i, int j);



	void SetSpeciesAgerangeArray(int specindex, int count, int value1, int value2);

	void GetSpeciesAgerangeArray(int specindex, int count, int& value1, int& value2);

	int GetAgerangeCount(int specindex);

	void SetAgerangeCount(int specindex, int count);



	void Read70OutputOption(char *FileName);

public:

	SortedSITE SortedIndex;


	double stocking_x_value;
	double stocking_y_value;
	double stocking_z_value;


	int SeedRainFlag;

	int GrowthFlag;

	int MortalityFlag;

	int VolumeFlag;

	int CellSize;

	int MaxDistofAllSpec;

	int specNum;

	int TimeStep;

	int TimeStep_Harvest;

	int TimeStep_Fuel;

	int TimeStep_BDA;

	int TimeStep_Wind;

	int TimeStep_Fire;

	int Pro0or401;

	int MaxShadeTolerance;

	int countWenjuanDebug;

	int flag_cut_GROUP_CUT[200]; //add by Qia on Nov 10 2011

	int flag_plant_GROUP_CUT[200];

	int num_TreePlant_GROUP_CUT[200];

	int flag_cut_GROUP_CUT_copy[200]; //add by Qia on June 02 2012

	int flag_plant_GROUP_CUT_copy[200];

	int num_TreePlant_GROUP_CUT_copy[200];

	float xLLCorner;

	float yLLCorner;

	int Flag_AgeDistStat;

	int *AgeDistStat_Year;

	int *AgeDistStat_AgeRange;

	double *BiomassFireCost;

	double *CarbonFireCost;

	int Fireflag;

	double *BiomassHarvestCost;

	double *CarbonHarvestCost;

	int Harvestflag;

	double *BiomassFuelCost;

	double *CarbonFuelCost;

	int Fuelflag;

	double *BiomassBDACost;

	double *CarbonBDACost;

	int BDAflag;

	double *BiomassWindCost;

	double *CarbonWindCost;

	int Windflag;

	int *OutputGeneralFlagArray;

	//add by houbin 10/18
	int OutputFireFlag;
	int OutputFuelFlag;
	int OutputHarvestFlag;
	int OutputSuccessionFlag;

	int *OutputModuleFlagArray;
	//add by houbin 10/18

	//add by houbin 10/23/2017
	int OutputAgeRangeFireFlag;
	int OutputAgeRangeFuelFlag;
	int OutputAgeRangeHarvestFlag;
	int OutputAgeRangeSuccessionFlag;

	int *OutputAgeRangeModuleFlagArray;
	//add by houbin 10/23/2017

	int flagAgeRangeOutput;

	int *OutputAgerangeFlagArray;

	int  *SpeciesAgerangeArray;

private:

	seeding_prob seed_prob;

	int* speces_maxD;
	int* specesefctD;
	float **SeedRain;

	float *GrowthRates;
	float *MortalityRates;

	
	std::vector<float *> GrowthRates_file;
	std::vector<float *> MortalityRates_file;	
	std::vector<float *> Volume_file;

	float *Volume;

	void ListbubbleSort(vector<double>& AreaList, vector<int>& SpecIndexArray, vector<int>& AgeIndexArray);

	SITE **map;

	SITE *map70;

	LANDUNIT **map_landtype;

	SITE *sitetouse;

	float *biomassData;

	int biomassNum;

	float BiomassThreshold;

	int rows, columns;                //Number of rows and columns contained in the whole map.

	unsigned long header[32];			 //Upperleft coordinates of sites

};









#undef DLLCLASS



#endif

