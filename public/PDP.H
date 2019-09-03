#ifndef PDP_H
#define PDP_H



#include "defines.h"


typedef struct
{
	short**	sTSLBDA;		//tinesince last BDA
	char**	cBDASeverity;
	char*	BDAType;
}BDAPDP;





class PDP
{
public:
	
	PDP() {};

	PDP(int, int, int, int, int, int, int);

	void set_parameters(int mode, int col, int row, int BDANo, int SpecNum, int interval_num, int trackflag);

	~PDP();

	template <class ArrayType> ArrayType** Array2D(ArrayType** A, int row, int col);
	template <class ArrayType> void DeleteArray2D(ArrayType** A, int row);

public:
	int			iCols;
	int			iRows;

	int 		specnum;				// 16.  species number Houbin 2/13/2017 
	int			interval_number;		//add by houbin 4/17/2017
	int			TrackFlag;				//add by houbin 12/8/2017

	//BDA
	int			iBDANum;
	BDAPDP*		pBDAPDP;

	//Fire
	short**		sTSLFire;
	char**		cFireSeverity;
	
	//Fuel
	char**		cFineFuel;
	char**		cCoarseFuel;
	char**		cFireIntensityClass;			// 13.	PotentialFireIntensity 
	char**		cFireRiskClass;					// 14.	Potential FireRisk

	//Harvest
	short**		sTSLHarvest;
	char**     cHarvestEvent;

	//wind
	short**		sTSLWind;
	char**		cWindSeverity;

	//Succession
	short**		sTSLMortality;


	int***		cDeadTreeNum;					// 15.	Dead tree number Houbin 1/1/2017 dim:row*col*specnum	
	float***		cDeadTreeBio;				// 17.  Dead tree Biomass Houbin 2/19/2017 dim:row*col*specnum
	float***		cDeadTreeBA;				// 18.  Dead tree basal area Houbin 2/22/2017 dim:row*col*specnum

	int***		cDeadTreeNum_spec_agerange;		//19.  Dead tree number differ spec&age range Houbin 10/25/2017 dim:row*col*(spec*age_range_count)
	float***	cDeadTreeBio_spec_agerange;		//20.  Dead tree biomass differ spec&age range Houbin 10/25/2017 dim:row*col*(spec*age_range_count)
	float***	cDeadTreeBA_spec_agerange;		//21.  Dead tree basal area differ spec&age range Houbin 10/25/2017 dim:row*col*(spec*age_range_count)
};



#endif
