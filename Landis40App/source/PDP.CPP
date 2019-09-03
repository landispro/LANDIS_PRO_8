
#include <string.h>
#include <stdlib.h>


#include "sites.h"
#include "PDP.H"



PDP::PDP(int mode, int col, int row, int BDANo, int SpecNum, int interval_num, int trackflag)
{
	set_parameters(mode, col, row, BDANo, SpecNum, interval_num, trackflag);
}



void PDP::set_parameters(int mode, int col, int row, int BDANo, int SpecNum, int interval_num, int trackflag)
{
	iCols = col;
	iRows = row;
	iBDANum = BDANo;

	sTSLFire = NULL;
	cFireSeverity = NULL;
	cFineFuel = NULL;
	cCoarseFuel = NULL;
	cFireIntensityClass = NULL;				// 13.	PotentialFireIntensity
	cFireRiskClass = NULL;					// 14.	Potential FireRisk
	sTSLHarvest = NULL;
	cHarvestEvent = NULL;
	sTSLWind = NULL;
	cWindSeverity = NULL;
	sTSLMortality = NULL;
	pBDAPDP = NULL;
	TrackFlag = trackflag;


	//changed by houbin 10/25/2017
	specnum = SpecNum;
	interval_number = interval_num;
	const int all_interval_num = interval_number * specnum;
	if (TrackFlag == 1)
	{
		
		cDeadTreeNum_spec_agerange = new int**[iRows];
		cDeadTreeBio_spec_agerange = new float**[iRows];
		cDeadTreeBA_spec_agerange = new float**[iRows];

		for (int i = 0; i < iRows; i++)
		{
			cDeadTreeNum_spec_agerange[i] = new int*[iCols];
			cDeadTreeBio_spec_agerange[i] = new float*[iCols];
			cDeadTreeBA_spec_agerange[i] = new float*[iCols];
		}
		for (int i = 0; i < iRows; i++)
			for (int j = 0; j < iCols; j++)
			{
				cDeadTreeNum_spec_agerange[i][j] = new int[all_interval_num]();
				cDeadTreeBio_spec_agerange[i][j] = new float[all_interval_num]();
				cDeadTreeBA_spec_agerange[i][j] = new float[all_interval_num]();
			}


		cDeadTreeNum = new int**[iRows];
		cDeadTreeBio = new float**[iRows];
		cDeadTreeBA = new float**[iRows];
		for (int i = 0; i < iRows; i++)
		{
			cDeadTreeNum[i] = new int*[iCols];
			cDeadTreeBio[i] = new float*[iCols];
			cDeadTreeBA[i] = new float*[iCols];
		}
			
		for (int i = 0; i < iRows; i++)
			for (int j = 0; j < iCols; j++)
			{
				cDeadTreeNum[i][j] = new int[specnum]();
				cDeadTreeBio[i][j] = new float[specnum]();
				cDeadTreeBA[i][j] = new float[specnum]();
			}
				
	}
	


	sTSLMortality = Array2D(sTSLMortality, iRows, iCols);

	//if BDA is loaded, intilize according to whether Fire, Harvetst, and Wind are loaded or not
	if (mode & G_BDA)
	{
		if (mode & G_WIND)
			sTSLWind = Array2D(sTSLWind, iRows, iCols);
	}

	//Fire
	if (mode & G_FIRE)
	{
		sTSLFire = Array2D(sTSLFire, iRows, iCols);

		cFireSeverity = Array2D(cFireSeverity, iRows, iCols);
	}



	if (mode & G_FUEL)
	{
		cFineFuel = Array2D(cFineFuel, iRows, iCols);

		cCoarseFuel = Array2D(cCoarseFuel, iRows, iCols);

		cFireIntensityClass = Array2D(cFireIntensityClass, iRows, iCols);

		cFireRiskClass = Array2D(cFireRiskClass, iRows, iCols);
	}



	//Fuel
	if (mode & G_FUEL)
	{
		if (mode & G_BDA)
		{
			//allocate BDA
			pBDAPDP = new BDAPDP[iBDANum];

			for (int i = 0; i < iBDANum; i++)
			{
				pBDAPDP[i].BDAType = new char[50];

				pBDAPDP[i].cBDASeverity = Array2D(pBDAPDP[i].cBDASeverity, iRows, iCols);

				pBDAPDP[i].sTSLBDA = Array2D(pBDAPDP[i].sTSLBDA, iRows, iCols);
			}
		}

	}


	if (mode & G_WIND)
	{
		if (sTSLWind == NULL)
			sTSLWind = Array2D(sTSLWind, iRows, iCols);

		if (cWindSeverity == NULL)
			cWindSeverity = Array2D(cWindSeverity, iRows, iCols);

		sTSLWind[1][1] = 0;
	}


	if (mode & G_HARVEST)
	{
		if (sTSLHarvest == NULL)
			sTSLHarvest = Array2D(sTSLHarvest, iRows, iCols);

		if (cHarvestEvent == NULL)
			cHarvestEvent = Array2D(cHarvestEvent, iRows, iCols);
	}
}





PDP::~PDP()
{
	DeleteArray2D(sTSLFire, iRows);

	DeleteArray2D(cFireSeverity, iRows);

	DeleteArray2D(cFineFuel, iRows);

	DeleteArray2D(cCoarseFuel, iRows);

	DeleteArray2D(sTSLHarvest, iRows);

	DeleteArray2D(cHarvestEvent, iRows);

	DeleteArray2D(sTSLWind, iRows);

	DeleteArray2D(cWindSeverity, iRows);

	DeleteArray2D(sTSLMortality, iRows);

	DeleteArray2D(cFireIntensityClass, iRows);

	DeleteArray2D(cFireRiskClass, iRows);

	if (pBDAPDP != NULL)
	{
		for (int i = 0; i < iBDANum; i++)
		{
			delete[] pBDAPDP[i].BDAType;

			DeleteArray2D(pBDAPDP[i].cBDASeverity, iRows);

			DeleteArray2D(pBDAPDP[i].sTSLBDA, iRows);
		}

		delete[] pBDAPDP;
	}


	//add by houbin 10/25/2017
	if (TrackFlag == 1)
	{
		for (int i = 0; i < iRows; i++)
		{
			for (int j = 0; j < iCols; j++)
			{
				delete[] cDeadTreeNum[i][j];
				delete[] cDeadTreeBio[i][j];
				delete[] cDeadTreeBA[i][j];
			}
			delete[] cDeadTreeNum[i];
			delete[] cDeadTreeBio[i];
			delete[] cDeadTreeBA[i];
		}
		delete[] cDeadTreeNum;
		delete[] cDeadTreeBio;
		delete[] cDeadTreeBA;


		//add by houbin 10/25/2017
		for (int i = 0; i < iRows; i++)
		{
			for (int j = 0; j < iCols; j++)
			{
				delete[] cDeadTreeNum_spec_agerange[i][j];
				delete[] cDeadTreeBA_spec_agerange[i][j];
				delete[] cDeadTreeBio_spec_agerange[i][j];
			}

			delete[] cDeadTreeNum_spec_agerange[i];
			delete[] cDeadTreeBA_spec_agerange[i];
			delete[] cDeadTreeBio_spec_agerange[i];
		}

		delete[] cDeadTreeNum_spec_agerange;
		delete[] cDeadTreeBA_spec_agerange;
		delete[] cDeadTreeBio_spec_agerange;
		//add by houbin 10/25/2017
	}
	
}



template <class ArrayType> ArrayType** PDP::Array2D(ArrayType** A, int row, int col)
{
	A = new ArrayType * [row + 1];

	if (A == NULL) { printf("memory not enough new ArrayType* [row+1] %d %d\n", row, col); exit(1);}

	for (int j = 0; j < row + 1; j++)
	{
		A[j] = new ArrayType[col + 1]();

		if (A[j] == NULL) { printf("memory not enough A[j] = new ArrayType [col+1]; %d %d\n", row, col); exit(1);}
	}

	return A;
}



template <class ArrayType> void PDP::DeleteArray2D(ArrayType** A, int row)
{
	if (A != NULL)
	{
		for (int j = 0; j < row + 1; j++)
			delete[] A[j];
		
		delete[] A;
	}
}

