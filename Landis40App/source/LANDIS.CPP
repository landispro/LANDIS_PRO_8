/****************************************************************************

* LANDIS model design:                                                      *

*        David J. Mladenoff, Joel E. Boeder, Hong S. He, George E. Host     *

* LANDIS 4.0  Dynamic Linked Libraries :                                    *

*        Wei Li, Hong S. He													*

* Contact :

*						                                                    *

*        Hong S. He, Forestry Department, University of Missouri at			*

*        Columbia,                       *

*        heh@missouri.edu													*

*                                                                           *

* History:                                                                  *

*        Name           Date                  General Modification          *

* ----------------  ------------  ----------------------------------------- *

*  Jeol E. Boeder       09/93            original coding                    *

* Chris Heim           ??/94            upgrade from LANDIS 1.0 to 1.5      *

* Hong S. He         10/94-2003       upgrade to LANDIS 3.6 (VC++6.0)       *

* Wei Li			 05/2003-08/2008   LANDIS 4.0							*

* Qia Wang		08/2008-current	  LANDIS Pro

*-----------------  ------------  ----------------------------------------- *

* Last changes were made by Qia Wang 2009.									*

*                                                                           *

* Version: Pro																*

* Description:                                                              *

*        LANDIS is a raster based landscape model simulating forest         *

*        succession, wind and fire disturbances, forest management, and     *

*        the interaction among them over long period of time.               *

* Documentations:                                                           *

*																			*

*                                                                           *

****************************************************************************/

#define _CRTDBG_MAP_ALLOC
#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <crtdbg.h>
#include <vector>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <afxwin.h> 
#include <string.h>
#include <omp.h>

#include "defines.h"
#include "system1.h"
#include "error.h"
#include "params.h"
#include "landu.h"
#include "landus.h"
#include "specatt.h"
#include "specatts.h"
#include "specie.h"
#include "site.h"
#include "sites.h"
#include "fire.h"
#include "wind.h"
#include "io.h"
#include "map8.h"
#include "reclass3.h"
#include "gdal_priv.h"
#include "cpl_conv.h" // for CPLMalloc()
#include "cpl_string.h"
#include "ogr_spatialref.h"
#include "GlobalFunctions.h"
#include "PDP.h"
#include "FuelGlobalFuncs.h"
#include "BDAExport.h"
#include "FireExport.h"
#include "GlobalVariables.h"


using namespace std;


#define DTN 0
#define DTNBA 1
#define DTNBIO 2
#define COUNT 3

void singularLandisIteration(int itr, int snr, int snc, double wAdfGeoTransform[], PDP& pdp);

void Succession_Landis70(int snr, int snc, PDP& pdp);

void initiateRDofSite_Landis70(int snr, int snc);

void KillTrees(int snr, int snc, PDP& pdp);

void Succesion_output_Mortality_bio_ba(int snr, int snc, int itr, PDP& pdp, double wAdfGeoTransform[]);

static int red2[maxLeg] = { 0, 70, 0, 0, 0, 0, 0, 0, 200, 100, 255, 150, 200, 200, 255, 255 };
static int green2[maxLeg] = { 0, 70, 0, 0, 0, 150, 200, 255, 30, 200, 50, 50, 200, 0, 255 };
static int blue2[maxLeg] = { 0, 70, 125, 200, 255, 0, 100, 0, 30, 50, 50, 0, 0, 255 };

PARAMETERS    parameters;

SPECIESATTRS  speciesAttrs(MAX_SPECIES);

LANDUNITS     landUnits(MAX_LANDUNITS); 

SITES         sites;
timestep	  time_step;

BDAS*		  pBDAS = NULL;
WIND*		  pWINDdll = NULL;
CFIRE*		  pFire = NULL;

HINSTANCE	  gLibBDA;
HINSTANCE	  gLibWind;
HINSTANCE	  gLibFuel;
HINSTANCE	  gLibFire;

vector<string> SEC_landtypefiles;
vector<string> SEC_gisfiles;
vector<string> Growthratesfiles;		//add by houbin 3/9/2017

int freq[6];

int currentDecade;

int numberOfSpecies;

unsigned short currentHarvestEventId = 0;


FILE *fpforTimeBU;
FILE *fpLogFileSEC;


MAP16 visitationMap;



char reMethods[MAX_RECLASS][80], ageMaps[MAX_SPECIES][80];

int gDLLMode;


// static const int max_thread_num = omp_get_max_threads() - 1;
static const int max_thread_num = omp_get_max_threads();
int* gl_cpuid = new int [max_thread_num]();

void putHarvestOutput(int itr, double wAdfGeoTransform[], int trackflag)
{
	MAP8 m(sites.getHeader());

	char str[200], str1[200];

	char str_htyp[200], str_htyp1[200], str_dec[200], str_dec1[200];

	writeStandReport();


	if (itr * freq[3] <= parameters.numIter)
	{
		sprintf(str_htyp1, "%s/%s/htyp%d.img", parameters.outputDir, "Harvest", itr*freq[3] * sites.TimeStep_Harvest);

		sprintf(str_dec1, "%s/%s/hdec%d.img", parameters.outputDir, "Harvest", itr*freq[3] * sites.TimeStep_Harvest);

		output_harvest_Dec_Type(itr, str_htyp, str_htyp1, str_dec, str_dec1, wAdfGeoTransform);
		if (trackflag == 1)
		{
			if (sites.OutputHarvestFlag == 1)
			{
				for (int i = 0; i < sites.specNum; i++)
				{
					if (sites.GetOutputModuleFlagArray(i, DTNBA))
					{
						sprintf(str1, "%s/%s/%s/%s_BACut%d.img", parameters.outputDir, "Harvest", "DTNBA", speciesAttrs(i + 1)->name, itr*freq[3] * sites.TimeStep_Harvest);
						PutOutput_harvestBACut_spec(str, str1, i, wAdfGeoTransform);
					}

					if (sites.GetOutputModuleFlagArray(i, DTNBIO))
					{
						sprintf(str1, "%s/%s/%s/%s_BIOCut%d.img", parameters.outputDir, "Harvest", "DTNBIO", speciesAttrs(i + 1)->name, itr*freq[3] * sites.TimeStep_Harvest);
						PutOutput_harvestBioCut_spec(str, str1, i, wAdfGeoTransform);
					}

					if (sites.GetOutputModuleFlagArray(i, DTN))
					{
						sprintf(str1, "%s/%s/%s/%s_NumCut%d.img", parameters.outputDir, "Harvest", "DTN", speciesAttrs(i + 1)->name, itr*freq[3] * sites.TimeStep_Harvest);
						PutOutput_harvestNumCut_spec(str, str1, i, wAdfGeoTransform);
					}
				}
			}

			if (sites.OutputAgeRangeHarvestFlag == 1)
			{
				for (int i = 0; i < sites.specNum; i++)
				{
					if (sites.GetOutputAgeRangeModuleFlagArray(i, DTN))
					{
						for (int j = 1; j <= sites.GetOutputAgeRangeModuleFlagArray(i, COUNT); j++)
						{
							int low = sites.GetOutputAgeRangeModuleFlagArray(i, j, 0);
							int high = sites.GetOutputAgeRangeModuleFlagArray(i, j, 1);
							sprintf(str1, "%s/%s/%s/%s_%d_%d_NumCut%d.img", parameters.outputDir, "Harvest", "SPEC_AGERANGE", speciesAttrs(i + 1)->name, low, high, itr*sites.TimeStep_Harvest);
							PutOutput_harvestNumCut_spec_interval(str, str1, i, j - 1, wAdfGeoTransform);
						}
					}

					if (sites.GetOutputAgeRangeModuleFlagArray(i, DTNBA))
					{
						for (int j = 1; j <= sites.GetOutputAgeRangeModuleFlagArray(i, COUNT); j++)
						{
							int low = sites.GetOutputAgeRangeModuleFlagArray(i, j, 0);
							int high = sites.GetOutputAgeRangeModuleFlagArray(i, j, 1);
							sprintf(str1, "%s/%s/%s/%s_%d_%d_BACut%d.img", parameters.outputDir, "Harvest", "SPEC_AGERANGE", speciesAttrs(i + 1)->name, low, high, itr*sites.TimeStep_Harvest);
							PutOutput_harvestBACut_spec_interval(str, str1, i, j - 1, wAdfGeoTransform);
						}
					}
					if (sites.GetOutputAgeRangeModuleFlagArray(i, DTNBIO))
					{
						for (int j = 1; j <= sites.GetOutputAgeRangeModuleFlagArray(i, COUNT); j++)
						{
							int low = sites.GetOutputAgeRangeModuleFlagArray(i, j, 0);
							int high = sites.GetOutputAgeRangeModuleFlagArray(i, j, 1);
							sprintf(str1, "%s/%s/%s/%s_%d_%d_BIOCut%d.img", parameters.outputDir, "Harvest", "SPEC_AGERANGE", speciesAttrs(i + 1)->name, low, high, itr*sites.TimeStep_Harvest);
							PutOutput_harvestBIOCut_spec_interval(str, str1, i, j - 1, wAdfGeoTransform);
						}
					}
				}
			}

			Clear_NUM_BIO_BA_spec_interval();
		}
		
	}


	if (parameters.numIter % freq[3] != 0)
	{
		sprintf(str_htyp1, "%s/%s/htyp%d.img", parameters.outputDir, "Harvest", parameters.numIter* sites.TimeStep_Harvest);

		sprintf(str_dec1, "%s/%s/hdec%d.img", parameters.outputDir, "Harvest", parameters.numIter * sites.TimeStep_Harvest);

		output_harvest_Dec_Type(itr, str_htyp, str_htyp1, str_dec, str_dec1, wAdfGeoTransform);
		if (trackflag == 1)
		{
			if (sites.OutputHarvestFlag == 1)
			{
				for (int i = 0; i < sites.specNum; i++)
				{
					if (sites.GetOutputModuleFlagArray(i, DTNBA))
					{
						sprintf(str1, "%s/%s/%s/%s_BACut%d.img", parameters.outputDir, "Harvest", "DTNBA", speciesAttrs(i + 1)->name, parameters.numIter* sites.TimeStep_Harvest);
						PutOutput_harvestBACut_spec(str, str1, i, wAdfGeoTransform);
					}

					if (sites.GetOutputModuleFlagArray(i, DTNBIO))
					{
						sprintf(str1, "%s/%s/%s/%s_BIOCut%d.img", parameters.outputDir, "Harvest", "DTNBIO", speciesAttrs(i + 1)->name, parameters.numIter* sites.TimeStep_Harvest);
						PutOutput_harvestBioCut_spec(str, str1, i, wAdfGeoTransform);
					}

					if (sites.GetOutputModuleFlagArray(i, DTN))
					{
						sprintf(str1, "%s/%s/%s/%s_NumCut%d.img", parameters.outputDir, "Harvest", "DTN", speciesAttrs(i + 1)->name, parameters.numIter* sites.TimeStep_Harvest);
						PutOutput_harvestNumCut_spec(str, str1, i, wAdfGeoTransform);
					}
				}
			}

			if (sites.OutputAgeRangeHarvestFlag == 1)
			{
				for (int i = 0; i < sites.specNum; i++)
				{
					if (sites.GetOutputAgeRangeModuleFlagArray(i, DTN))
					{
						for (int j = 1; j <= sites.GetOutputAgeRangeModuleFlagArray(i, COUNT); j++)
						{
							int low = sites.GetOutputAgeRangeModuleFlagArray(i, j, 0);
							int high = sites.GetOutputAgeRangeModuleFlagArray(i, j, 1);
							sprintf(str1, "%s/%s/%s/%s_%d_%d_NumCut%d.img", parameters.outputDir, "Harvest", "SPEC_AGERANGE", speciesAttrs(i + 1)->name, low, high, itr*sites.TimeStep_Harvest);
							PutOutput_harvestNumCut_spec_interval(str, str1, i, j - 1, wAdfGeoTransform);
						}
					}

					if (sites.GetOutputAgeRangeModuleFlagArray(i, DTNBA))
					{
						for (int j = 1; j <= sites.GetOutputAgeRangeModuleFlagArray(i, COUNT); j++)
						{
							int low = sites.GetOutputAgeRangeModuleFlagArray(i, j, 0);
							int high = sites.GetOutputAgeRangeModuleFlagArray(i, j, 1);
							sprintf(str1, "%s/%s/%s/%s_%d_%d_BACut%d.img", parameters.outputDir, "Harvest", "SPEC_AGERANGE", speciesAttrs(i + 1)->name, low, high, itr*sites.TimeStep_Harvest);
							PutOutput_harvestBACut_spec_interval(str, str1, i, j - 1, wAdfGeoTransform);
						}
					}
					if (sites.GetOutputAgeRangeModuleFlagArray(i, DTNBIO))
					{
						for (int j = 1; j <= sites.GetOutputAgeRangeModuleFlagArray(i, COUNT); j++)
						{
							int low = sites.GetOutputAgeRangeModuleFlagArray(i, j, 0);
							int high = sites.GetOutputAgeRangeModuleFlagArray(i, j, 1);
							sprintf(str1, "%s/%s/%s/%s_%d_%d_BIOCut%d.img", parameters.outputDir, "Harvest", "SPEC_AGERANGE", speciesAttrs(i + 1)->name, low, high, itr*sites.TimeStep_Harvest);
							PutOutput_harvestBIOCut_spec_interval(str, str1, i, j - 1, wAdfGeoTransform);
						}
					}
				}
			}

			Clear_NUM_BIO_BA_spec_interval();
		}

		
		

		
	}


	sprintf(str1, "%s/%s/BACut%d.img", parameters.outputDir, "Harvest", parameters.numIter * sites.TimeStep_Harvest);

	PutOutput_harvestBACut(str, str1, wAdfGeoTransform);

	
}








void updateLandtypeImg8(GDALDataset* ltimgFile)
{
	static const int nCols = ltimgFile->GetRasterXSize();
	static const int nRows = ltimgFile->GetRasterYSize();

	static const int xDim = sites.numColumns();
	static const int yDim = sites.numRows();

	if (nCols != xDim || nRows != yDim)
		errorSys("landtype map and species map do not match.", STOP);

	static const int total_size = sizeof(float) * xDim * yDim;
	
	float* pafScanline = (float*)CPLMalloc(total_size);

	GDALRasterBand* poBand = ltimgFile->GetRasterBand(1);

	poBand->RasterIO(GF_Read, 0, 0, xDim, yDim, pafScanline, xDim, yDim, GDT_Float32, 0, 0);

	for (int i = yDim; i > 0; i--)
	{
		const int cover_index_part = (yDim - i) * xDim;

		for (int j = 1; j <= xDim; j++)
		{
			const int coverType = (int) *(pafScanline + cover_index_part + j - 1);

			if (coverType < 0)
				errorSys("illegel landtype class found\n", STOP);

			sites.fillinLanduPt(i, j, landUnits(coverType));			
		}
	}
}





void envUpdate(int i)
{
	double wAdfGeoTransform[6];

	char iterString[5] = {0};
	itoa(i, iterString, 10);
	
	GDALAllRegister();

	//update land type attribute
	//file name is landtype.dat1, landtype.dat10 etc
	char str[255] = {0};
	strcpy(str, parameters.landUnitFile);
	strcat(str, iterString);

	FILE* luFile;
	if ((luFile = LDfopen(str, 1)) == NULL)
	{
		char tempStr[255];
		sprintf(tempStr, "Landtype attribute file %s not found.\n", str);
		errorSys(tempStr, STOP);
	}
	else
	{
		//update landtype attribute
		landUnits.read(luFile);
		fclose(luFile);
	}

	//update land type GIS
	//file name is landtype.gis1, landtype.gis10 etc

	strcpy(str, parameters.landImgMapFile);
	strcat(str, iterString);

	GDALDataset* simgFile;

	if ((simgFile = (GDALDataset *)GDALOpen(parameters.landImgMapFile, GA_ReadOnly)) == NULL) 
	{
		char tempStr[255];
		sprintf(tempStr, "Landtype img file %s not found.\n", str);
		errorSys(tempStr, STOP);
	}
	else
	{
		double adfGeoTransform[6] = {0};

		//update landtype img
		if (simgFile->GetGeoTransform(adfGeoTransform) == CE_None)
		{
			for (int i = 0; i < 6; i++)
				wAdfGeoTransform[i] = adfGeoTransform[i];
		}

		updateLandtypeImg8(simgFile);

		GDALClose((GDALDatasetH)simgFile);
	}

}






//Main program.  This contains start and shut down procedures as well as the main iteration loop.
int main(int argc, char *argv[])
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	char str[100];

	int numbOfIter, snr, snc, specAtNum;

	int numSitesActive, envOn = 0, reclYear = 0;
	
	GDALDataset  *simgFile, *ltimgFile;

	double        adfGeoTransform[6];

	GDALAllRegister();

	
	time_t ltime, ltimeTemp;

	time(&ltime);

	printf("Starting LANDIS PRO 7.1 at %s", ctime(&ltime));



	if ((argc != 2) && (argc != 3) && (argc != 4))  { argHelp(); exit(255); }

	if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "-H") == 0 || strcmp(argv[1], "-p") == 0 || strcmp(argv[1], "-P") == 0)   
		{ argHelp(); exit(255); }


	printf("%s\n", argv[1]);
	printf("%s\n", argv[2]);

	
	FILE *infile;

	////////////////////////////// Merge Part ////////////////////////////
	if ((argc != 2) && (argc != 3) && (argc != 4))
	{
		argHelp();
		exit(255);
	}
	if ((strcmp(argv[1], "-h") == 0) || (strcmp(argv[1], "-H") == 0))
	{
		argHelp();
		exit(255);
	}
	else if ((strcmp(argv[1], "-p") == 0) || (strcmp(argv[1], "-P") == 0))
	{
		help();
		exit(255);
	}
	//////////////////////////////////////////////////////////////////////

	if ((strcmp(argv[1], "-e") == 0) || (strcmp(argv[1], "-E") == 0))
	{
		if (argc != 4)
			errorSys("Syntax error. For help enter: landis -h", STOP);

		envOn = atoi(argv[2]);

		if (!envOn)
			errorSys("Invalid year value", STOP);
		else if ((infile = LDfopen(argv[3], 1)) == NULL)
			errorSys("Input file not found here. For help enter: landis -h ", STOP);
	}
	else if ((strcmp(argv[1], "-r") == 0) || (strcmp(argv[1], "-R") == 0))
	{
		if (argc != 4)
			errorSys("Syntax error. For help enter: landis -h", STOP);

		reclYear = atoi(argv[2]);

		if (!(reclYear%sites.TimeStep))
			errorSys("Invalid year number", STOP);
		else if ((infile = LDfopen(argv[3], 1)) == NULL)
			errorSys("Input file not found here0. For help enter: landis -h", STOP);
	}
	else if (((infile = LDfopen(argv[1], 1)) == NULL))
		errorSys("Input file not found. For help enter: landis -h", STOP);


	if (argc == 2)
		printf("input: %s\n", argv[1]);
	else
		printf("input: %s\n", argv[3]);

	printf("Beginning Landis 7.1 Pro Run.\n");

	if (envOn > 0)
		printf("Environment will be updated every %d iteration\n", envOn);


	int	 BDANo;

	gDLLMode = parameters.read(infile, &BDANo);

	time_step.settimestep(parameters.timestep);

	sites.stocking_x_value = parameters.stocking_x_value;
	sites.stocking_y_value = parameters.stocking_y_value;
	sites.stocking_z_value = parameters.stocking_z_value;


	sites.TimeStep         = parameters.timestep;
	sites.TimeStep_BDA 	   = parameters.timestep_BDA;
	sites.TimeStep_Fire    = parameters.timestep_Fire;
	sites.TimeStep_Fuel    = parameters.timestep_Fuel;
	sites.TimeStep_Harvest = parameters.timestep_Harvest;
	sites.TimeStep_Wind    = parameters.timestep_Wind;
	sites.CellSize 		   = parameters.cellSize;



	for (int x = 0; x < 5; x++)
		freq[x] = 1;

	///////////////// Merge Part /////////////////
	// if ((gDLLMode & G_HARVEST) != 0)
	// 	freq[5] = 1;
	if ((gDLLMode & G_HARVEST) != 0)		
		for (int x = 0; x < 6; x++) freq[x] = 1;
	else
		for (int x = 0; x < 5; x++) freq[x] = 1;
	//////////////////////////////////////////////

	if (gDLLMode & G_BDA)
		printf("BDA ");

	if (gDLLMode & G_WIND)
		printf("Wind ");

	if (gDLLMode & G_HARVEST)
		printf("Harvest ");

	if (gDLLMode & G_FUEL)
		printf("Fuel ");

	if (gDLLMode & G_FUELMANAGEMENT)
		printf("Fuel management ");

	if (gDLLMode & G_FIRE)
		printf("Fire ");

	if (gDLLMode != 0)
		printf("are(is) on\n");

	printf("------------------------------------------------------------\n\n");
	

	double wAdfGeoTransform[6] = {0};
	PDP pdp;
	getInput(infile, freq, ageMaps, pdp, BDANo, wAdfGeoTransform);

	
	if (pdp.TrackFlag == 1)
	{
		char Succ_outputdir[200];
		sprintf(Succ_outputdir, "%s/%s", parameters.outputDir, "Succession");
		if (_mkdir(Succ_outputdir) != 0)
		{
			if (errno != EEXIST)
				errorSys("Succession: Can't create the succession output direcory", STOP);
		}

		char outputdir_spec_dtn[100];
		char outputdir_spec_dtnba[100];
		char outputdir_spec_dtnbio[100];
		char outputdir_agerange_spec_dtn_all[100];

		sprintf(outputdir_spec_dtn, "%s/%s", Succ_outputdir, "DTN");
		sprintf(outputdir_spec_dtnba, "%s/%s", Succ_outputdir, "DTNBA");
		sprintf(outputdir_spec_dtnbio, "%s/%s", Succ_outputdir, "DTNBIO");
		sprintf(outputdir_agerange_spec_dtn_all, "%s/%s", Succ_outputdir, "SPEC_AGERANGE");

		if (_mkdir(outputdir_spec_dtn) != 0)
		{
			if (errno != EEXIST)
				errorSys("Succession: Can't create the dead tree number in diff spec direcory", STOP);
		}
		if (_mkdir(outputdir_spec_dtnba) != 0)
		{
			if (errno != EEXIST)
				errorSys("Succession: Can't create the dead tree basalarea in diff spec direcory", STOP);
		}
		if (_mkdir(outputdir_spec_dtnbio) != 0)
		{
			if (errno != EEXIST)
				errorSys("Succession: Can't create the dead tree biomass in diff spec direcory", STOP);
		}
		if (_mkdir(outputdir_agerange_spec_dtn_all) != 0)
		{
			if (errno != EEXIST)
				errorSys("Succession: Can't create the dead tree number&biomass&basalarea in diff spec&agerange direcory", STOP);
		}

	}

	if ((gDLLMode & G_HARVEST) != 0)
	{
		printf("Harvest Dll loaded in...\n");

		HarvestPass(&sites, &speciesAttrs);

		sites.Harvest70outputdim();
	}



	if ((gDLLMode & G_WIND) != 0)
	{
		printf("Enabling WindThrow\n");

		typedef WIND* (*GetWINDTYPE)(char*, char*, int);

		gLibWind = LoadLibrary("WindDll.dll");

		FARPROC  pnProc = GetProcAddress(gLibWind, "GetWind");

		if (!pnProc)
		{
			DWORD m = GetLastError();
			errorSys("Wind: Error in load in Wind DLL, check if there is a Wind dll.", STOP);
		}

		GetWINDTYPE GetWind = (GetWINDTYPE)pnProc;

		pWINDdll = GetWind(parameters.strWindInitName, parameters.outputDir, gDLLMode);

		pWINDdll->m_pPDP = &pdp;

		sites.Wind70outputdim();
	}


	if ((gDLLMode & G_WIND) != 0)
		pWINDdll->m_pPDP = &pdp;



	if ((gDLLMode & G_BDA) != 0)
	{
		typedef BDAS* (*GetBDASTYPE)(char* str, SITES* s, int i, char*, LANDUNITS* plu, SPECIESATTRS* ps, PDP*, int, int);

		printf("Enabling BDA\n");

		gLibBDA = LoadLibrary("BDADll.dll");

		if (gLibBDA == NULL)
			printf("Error in Loading BDADLL\n");

		FARPROC  pnProc = GetProcAddress(gLibBDA, "GetBDAS");

		if (!pnProc)
		{
			DWORD m = GetLastError();
			errorSys("BDA: Error in load in BDA DLL, check if there is a BDA dll.", STOP);
		}

		GetBDASTYPE GetBDAS = (GetBDASTYPE)pnProc;

		pBDAS = GetBDAS(parameters.strBDAInitName, &sites, parameters.cellSize, parameters.outputDir, &landUnits, &speciesAttrs, &pdp, parameters.randSeed, gDLLMode);

		printf("BDA Dll loaded in...\n");

		sites.BDA70outputdim();
	}


	if ((gDLLMode & G_FUEL) != 0)
	{
		HINSTANCE gLibFuel;

		printf("Enabling Fuel\n");

		typedef int(*GetFUELTYPE)(PDP*, char*, int, int, char*, SITES*, int);

		gLibFuel = LoadLibrary("FuelDll.dll");

		FARPROC pnProc = GetProcAddress(gLibFuel, "FuelInit");

		if (!pnProc)
		{
			DWORD m = GetLastError();
			errorSys("Fuel: Error in load in Fuel DLL, check if there is a Fuel dll.", STOP);
		}

		GetFUELTYPE FuelInit = (GetFUELTYPE)pnProc;

		gDLLMode = FuelInit(&pdp, parameters.strFuelInitName, speciesAttrs.number(), landUnits.number(), parameters.outputDir, &sites, gDLLMode);

		sites.Fuel70outputdim();
	}


	if ((gDLLMode & G_FIRE) != 0)
	{
		printf("Start fire (version 4.3) initialization \n");
		
		/////////////// Merge Part ///////////////
		fflush(stdout);
		//////////////////////////////////////////

		typedef CFIRE* (*GetFIRETYPE)(char*, int, SITES*, LANDUNITS*, SPECIESATTRS*, PDP*, int, char*, int);

		gLibFire = LoadLibrary("FireDll.dll");

		FARPROC pnProcfire = GetProcAddress(gLibFire, "GetFire");

		if (!pnProcfire)
		{
			DWORD m = GetLastError();
			errorSys("Fire: Error in load in Fire DLL, check if there is a Fire dll.", STOP);
		}

		GetFIRETYPE GetFire = (GetFIRETYPE)pnProcfire;

		pFire = GetFire(parameters.strFireInitName, gDLLMode, &sites, &landUnits, &speciesAttrs, &pdp, parameters.numIter, parameters.outputDir, parameters.randSeed);

		printf("Finish Fire initiliation \n");
		
		/////////////// Merge Part ///////////////
		fflush(stdout);
		//////////////////////////////////////////

		sites.Fire70outputdim();
	}


	printf("Finish getting input\n");


	OutputScenario();

	initiateOutput_landis70Pro();

	snr = sites.numRows();
	snc = sites.numColumns();

	specAtNum = speciesAttrs.number();

	////////////////// Merge Part /////////////////
	numSitesActive = sites.numActive();
	///////////////////////////////////////////////

	numbOfIter = parameters.numIter;

	sites.GetSeedDispersalProbability(snr, snc);
	// sites.GetSeedDispersalProbability(parameters.SeedRainFile, parameters.SeedRainFlag);



	if (parameters.GrowthFlag == 0)
	{
		sites.GetSpeciesGrowthRates(parameters.GrowthFlagFile, parameters.GrowthFlag);
	}
	else
	{
		FILE *fp;

		if ((fp = LDfopen(parameters.GrowthFlagFile, 1)) == NULL)
			errorSys("Specie Growth Rates file not found.", STOP);

		int num_of_GRfiles;

		fscanf(fp, "%d",&num_of_GRfiles);
		
		char Growthratesfile[100];

		while (!LDeof(fp))
		{
			if (fscanc(fp, "%s", Growthratesfile ) != 1)
				errorSys("Error reading in Growth rates file.", STOP);

			Growthratesfiles.push_back(string(Growthratesfile));
		}

		LDfclose(fp);

		sprintf(Growthratesfile, "%s", Growthratesfiles.at(0).c_str());

		sites.GetSpeciesGrowthRates(Growthratesfile, parameters.GrowthFlag);
	}

	// sites.GetSpeciesGrowthRates(parameters.GrowthFlagFile, parameters.GrowthFlag);

	sites.GetSpeciesMortalityRates(parameters.MortalityFile, parameters.MortalityFlag);

	sites.GetVolumeRead(parameters.VolumeFile, parameters.VolumeFlag);

	initiateRDofSite_Landis70(snr, snc);




	if (reclYear)
	{
		const int lc_year_num = reclYear / sites.TimeStep;

		reclassify(reclYear, ageMaps);

		putOutput(lc_year_num, lc_year_num, freq, wAdfGeoTransform);

		putOutput_Landis70Pro(lc_year_num, lc_year_num, freq, wAdfGeoTransform);

		putOutput_AgeDistStat(lc_year_num);

		fclose(infile);

		return 0;
	}
	else 
	{
		putOutput(0, 0, freq, wAdfGeoTransform);

		putOutput_Landis70Pro(0, 0, freq, wAdfGeoTransform);

		putOutput_AgeDistStat(0);
	}



	if ((gDLLMode & G_FUEL) != 0)
		FuelputFuelOutput(reclYear / sites.TimeStep_Fuel, parameters.outputDir, parameters.cellSize, wAdfGeoTransform);


	if (envOn > numbOfIter)
		errorSys("Invalid year of interpretation for updating environment", STOP);



	if (parameters.randSeed == 0)
	{
		srand((unsigned)time(&ltime));
		parameters.randSeed = ltime;
	}

	srand(parameters.randSeed);




	char strfilenamefortime[100];

	sprintf(strfilenamefortime, "%s/%s", parameters.outputDir, "Running Time Stat.txt");

	fpforTimeBU = fopen(strfilenamefortime, "w");

	sprintf(strfilenamefortime, "%s/%s", parameters.outputDir, "SECLog.txt");

	fpLogFileSEC = fopen(strfilenamefortime, "w");


	time(&ltimeTemp);

	printf("\nFinish the initilization at %s\n", ctime(&ltimeTemp));

	long ltimeDiff = (long)ltimeTemp - ltime;

	printf("it took %ld seconds\n", ltimeDiff);



	landUnits.initiateVariableVector(parameters.numIter, parameters.timestep, specAtNum, parameters.flagforSECFile);

	fprintf(fpforTimeBU, "Initilization took: %ld seconds\n", ltimeDiff);


	//Simulation loops////////////////////////////////////////////////
	for (int i = 1; i <= numbOfIter * sites.TimeStep; i++)
	{
		if (i % sites.TimeStep == 0)
		{
			if ((envOn > 0) && (i % envOn == 0) && i > 1 && 0)
			{
				//performing environment change
				//should also affect fireRegimeUnit GIS and Attr for fire and fuel module
				//update landtype GIS and landtype Attr if there are any
				//let fire module update fire regime unit GIS and Attr if there are any

				envUpdate(i);

				printf("landtype attribute and gis has been updated at iteration %d\n", i);
				


				if ((gDLLMode & G_FIRE) != 0)
				{
					//update fire regime unit attr and fire regime GIS
					pFire->updateFRU(i);

					printf("fire regime unit attribute and gis has been updated at iteration %d\n", i);
					
				}

			}

			if ((gDLLMode & G_FIRE) != 0 && pFire->flag_regime_update)
			{
				pFire->updateFire_Regime_Map(i);
				//printf("fire regime unit attribute and gis has been updated at iteration %d\n",i);
			}


			if (parameters.flagforSECFile == 3)
			{
				char SECfileNametemp[100];

				char SECfileMapGIS[100];

				if (i / sites.TimeStep == 1)
				{
					SEC_landtypefiles.clear();
					SEC_gisfiles.clear();

					if ((parameters.FpSECfile = LDfopen(parameters.varianceSECFile, 1)) == NULL)
						errorSys("Can not open SEC variance file", STOP);
					
					int num_of_files;
					fscanf(parameters.FpSECfile, "%d", &num_of_files);

					for (int ii_count_num = 0; ii_count_num < num_of_files; ii_count_num++)
					{
						if (fscanc(parameters.FpSECfile, "%s %s", SECfileMapGIS, SECfileNametemp) != 1)
							errorSys("Error reading in Landunit from SEC variance file.", STOP);

						SEC_landtypefiles.push_back(string(SECfileNametemp));

						SEC_gisfiles.push_back(string(SECfileMapGIS));
					}


					fclose(parameters.FpSECfile);

					int index = i / sites.TimeStep - 1;

					if (index < SEC_landtypefiles.size())
					{
						sprintf(SECfileNametemp, "%s", SEC_landtypefiles.at(index).c_str());
						sprintf(SECfileMapGIS, "%s", SEC_gisfiles.at(index).c_str());

						FILE *luFile;

						strcpy(parameters.landUnitFile, SECfileNametemp);

						if ((luFile = LDfopen(parameters.landUnitFile, 1)) == NULL)
						{
							printf("Landtype attribute file %s not found.\n", parameters.landUnitFile);
							
							errorSys(parameters.landUnitFile, STOP);
						}
						else
						{
							//update landtype attribute
							landUnits.read(luFile);

							fclose(luFile);
						}

						printf("\nEnvironment parameter Updated.\n");

						strcpy(parameters.landImgMapFile, SECfileMapGIS);


						if ((simgFile = (GDALDataset *)GDALOpen(parameters.landImgMapFile, GA_ReadOnly)) == NULL) //* landtype.img
						{
							printf("Land Map Img file %s not found.\n", parameters.landImgMapFile);
							

							errorSys(parameters.landImgMapFile, STOP);
						}
						else
						{
							if (simgFile->GetGeoTransform(adfGeoTransform) == CE_None)
							{
								for (int i = 0; i < 6; i++)
									wAdfGeoTransform[i] = adfGeoTransform[i];
							}

							updateLandtypeImg8(simgFile);

							GDALClose((GDALDatasetH)simgFile);
						}


						printf("\nEnvironment map Updated.\n");
						


						LANDUNIT*	SECLog_use = landUnits.first();

						int ii_count = 0;

						fprintf(fpLogFileSEC, "Year: %d\n", i);

						for (SECLog_use = landUnits.first(); ii_count < landUnits.number(); ii_count++){

							fprintf(fpLogFileSEC, "Landtype%d:\n", ii_count);

							for (int jj_count = 1; jj_count <= speciesAttrs.number(); jj_count++){

								fprintf(fpLogFileSEC, "spec%d: %f, ", jj_count, SECLog_use->probRepro(jj_count));
							}

							SECLog_use = landUnits.next();

							fprintf(fpLogFileSEC, "\n");
						}
					}
				}



				if (i / sites.TimeStep > 1)
				{
					const int index = i / sites.TimeStep - 1;

					if (index < SEC_landtypefiles.size())
					{
						sprintf(SECfileNametemp, "%s", SEC_landtypefiles.at(index).c_str());

						sprintf(SECfileMapGIS, "%s", SEC_gisfiles.at(index).c_str());						

						strcpy(parameters.landUnitFile, SECfileNametemp);

						FILE *luFile;
						if ((luFile = LDfopen(parameters.landUnitFile, 1)) == NULL)
						{
							printf("Landtype attribute file %s not found.\n", parameters.landUnitFile);
							

							errorSys(parameters.landUnitFile, STOP);
						}
						else
						{
							//update landtype attribute
							landUnits.read(luFile);

							fclose(luFile);
						}

						printf("\nEnvironment parameter Updated.\n");
						


						strcpy(parameters.landImgMapFile, SECfileMapGIS);

						if ((ltimgFile = (GDALDataset *)GDALOpen(parameters.landImgMapFile, GA_ReadOnly)) == NULL) //* landtype.img
						{
							printf("Land Map Img file %s not found.\n", parameters.landImgMapFile);

							errorSys(parameters.landImgMapFile, STOP);
						}
						else
						{
							if (ltimgFile->GetGeoTransform(adfGeoTransform) == CE_None)
							{
								for (int i = 0; i < 6; i++)
									wAdfGeoTransform[i] = adfGeoTransform[i];
							}

							updateLandtypeImg8(ltimgFile);

							GDALClose((GDALDatasetH)ltimgFile); 
						}

						printf("\nEnvironment map Updated.\n");


						LANDUNIT*	SECLog_use = landUnits.first();

						int ii_count = 0;

						fprintf(fpLogFileSEC, "Year: %d\n", i);

						for (SECLog_use = landUnits.first(); ii_count < landUnits.number(); ii_count++)
						{
							fprintf(fpLogFileSEC, "Landtype%d:\n", ii_count);

							for (int jj_count = 1; jj_count <= speciesAttrs.number(); jj_count++)
								fprintf(fpLogFileSEC, "spec%d: %f, ", jj_count, SECLog_use->probRepro(jj_count));

							SECLog_use = landUnits.next();

							fprintf(fpLogFileSEC, "\n");
						}
					}

				}
			}


			if (parameters.GrowthFlag == 1)
			{
				const int index = i / sites.TimeStep;

				char Growthratesfile[100] = {0};
				
				if (index < Growthratesfiles.size())
				{
					sprintf(Growthratesfile, "%s", Growthratesfiles.at(index).c_str());

					sites.GetSpeciesGrowthRates(Growthratesfile, parameters.GrowthFlag);
				}
				else
				{
					sprintf(Growthratesfile, "%s", Growthratesfiles.at(Growthratesfiles.size()-1).c_str());

					sites.GetSpeciesGrowthRates(Growthratesfile, parameters.GrowthFlag);
				}

				printf("\nSpecies Growth Rates Updated\n");
				
			}
		}



		if ((gDLLMode & G_HARVEST) != 0 && i%sites.TimeStep_Harvest == 0)
		{
			HarvestPassCurrentDecade(i);

			for (int r = 1; r <= snr; r++)
				for (int c = 1; c <= snc; c++)
					setUpdateFlags(r, c);
		}


		// printf("Processing Year %d\n", i);
		//End Harvest Init 2-----------------------------------------------//

		singularLandisIteration(i, snr, snc, wAdfGeoTransform, pdp);


		const int timestep_m_iter = sites.TimeStep * parameters.numIter;

		if ((i % sites.TimeStep == 0) || (i == timestep_m_iter))
		{
			const int i_d_timestep = i / sites.TimeStep;

			int frequency[6] = { 1, 1, 1, 1, 1, 1 };

			if (i % (sites.TimeStep * freq[0]) == 0 && i_d_timestep < parameters.numIter)
				putOutput_Landis70Pro(0, i_d_timestep, freq, wAdfGeoTransform);
			
			if (i == timestep_m_iter)
				putOutput_Landis70Pro(0, parameters.numIter, frequency, wAdfGeoTransform);			

			if (i % (sites.TimeStep * freq[4]) == 0 && i_d_timestep < parameters.numIter)
				putOutput(0, i_d_timestep, freq, wAdfGeoTransform);
			
			if (i == timestep_m_iter)
				putOutput(0, parameters.numIter, frequency, wAdfGeoTransform);			

			putOutput_AgeDistStat(i_d_timestep);
		}

		///////////////////////////////////////////// Merge Part //////////////////////////////////
		if ((gDLLMode & G_FUEL) != 0 && i%sites.TimeStep_Fuel == 0)
			FuelputFuelOutput(i / sites.TimeStep_Fuel, parameters.outputDir, parameters.cellSize, wAdfGeoTransform);
		///////////////////////////////////////////////////////////////////////////////////////////
	}




	//Simulation loops end/////////////////////////////////////////////////

	AgeDistOutputFromBufferToFile();


	if ((gDLLMode & G_HARVEST) != 0)
		CleanHarvest();

	fclose(infile);
	fclose(fpforTimeBU);
	fclose(fpLogFileSEC);

	time(&ltimeTemp);

	ltimeDiff = (long)ltimeTemp - ltime;

	printf("The whole program is running for %ld seconds\n", ltimeDiff);
	printf("End of simulation.\n");

	delete[] gl_cpuid;

	return 0;
}




//initiating Landis70 RD values
void initiateRDofSite_Landis70(int snr, int snc) //bsl
{
	clock_t gl_start = clock();

	for (int i = 1; i <= snr; i++)
		for (int j = 1; j <= snc; j++)
			sites.GetRDofSite(i, j);

	clock_t gl_stop = clock();

	printf("calculating initiateRDofSite_Landis70 takes %d seconds\n", (gl_stop - gl_start) / 1000);
}






unsigned int __stdcall Cal_SiteDynamics(void *p_this)
{
	SiteDynParmVec* param = static_cast<SiteDynParmVec*>(p_this);

	const int num = (param -> cols).size();

	for(int i=0; i<num; i++)
	{
		SiteDynmParam sdp;
		sdp.row = (param -> rows)[i];
		sdp.col = (param -> cols)[i];
		sdp.RDflag = (param -> RDflags)[i];
		sdp.Ppdp   = param -> Ppdp;
		sites.SiteDynamics(&sdp);
	}

	return 0;
}



void startSiteDynamicsThread(SiteDynParmVec& param, HANDLE& lHandle)
{
	//if you use more than 64 logical cores, cpu_id need to be changed
	//otherwise, some threads may not be assigned to a logical core
	const unsigned long long cpu_id = param.cpu_id;

	void* pointer = static_cast<void*>(&param);

	lHandle = (HANDLE)_beginthreadex(0, 0, &Cal_SiteDynamics, pointer, 0, 0);

	SetThreadAffinityMask(lHandle, 1 << cpu_id);
}





void init_cpu_id()
{
	for(int i=0; i<max_thread_num; i++)
		gl_cpuid[i] = 0;	
}



//Processor affinity
int get_cpu_id()
{
	unsigned int count = 0;

	while(1)
	{
		for(int i=0; i<max_thread_num; i++)
		{
			if (gl_cpuid[i] == 0)
			{
				gl_cpuid[i] = 1;
				return i;
			}
		}
		
		++count;

		if (count % 100 == 0)
		{
			Sleep(1000);  //sleep 1 second

			if (count > 2147483646)
				count = 0;
		}
	}
}




unsigned int __stdcall Distribute_seeds(void *p_this)
{
	DispersalParam param = *(static_cast<DispersalParam*>(p_this));
	
	sites.GetSeedDispersalonSite(param);

	gl_cpuid[param.cpu_id] = 0; //this cpu is available now

	return 0;
}



void startDispersalThread(DispersalParam& param, HANDLE& lHandle)
{
	const unsigned long long cpu_id = param.cpu_id;
	
	void* pointer = static_cast<void*>(&param);
	
	lHandle = (HANDLE)_beginthreadex(0, 0, &Distribute_seeds, pointer, 0, 0);
	
	SetThreadAffinityMask(lHandle, 1 << cpu_id);
}





// According to shade-rule and RD-rule determine species succession on sites, main function 
void Succession_Landis70(int snr, int snc, PDP& pdp) //bsl
{
	sites.GetMatureTree();

	const int num_speciesAttrs = speciesAttrs.number();

	//GrowTree
	//increase ages
	for (int i = 1; i <= snr; i++)
	{
		for (int j = 1; j <= snc; j++)
		{
			pdp.sTSLMortality[i][j] += (short)sites.TimeStep;

			if ((gDLLMode & G_WIND) != 0)
				pdp.sTSLWind[i][j] += (short)sites.TimeStep;

			if ((gDLLMode & G_HARVEST) != 0)
				pdp.sTSLHarvest[i][j] += (short)sites.TimeStep;

			//Define Land unit.
			LANDUNIT* l = sites.locateLanduPt(i, j);

			SITE* lc_site = sites(i, j);

			if (l != NULL && l->active())
			{
				for (int k = 1; k <= num_speciesAttrs; k++)
					lc_site->SpecieIndex(k)->GrowTree();

				KillTrees(i, j, pdp);
			}
		}
	}
	
	
	clock_t gl_start = clock();

	const static int num_cell = snr * snc;

	unsigned long*** numseeds = new unsigned long** [num_speciesAttrs];

	for(int k=0; k<num_speciesAttrs; k++)
	{
		numseeds[k]    = new unsigned long* [snr];
		numseeds[k][0] = new unsigned long  [num_cell]();

		for(int i =1; i<snr; i++)
			numseeds[k][i] = numseeds[k][i-1] + snc;
	}

	HANDLE* lc_handle = new HANDLE [num_speciesAttrs]();
	DispersalParam* disp_param = new DispersalParam[num_speciesAttrs]();

	init_cpu_id();

	//get seed number
	for (int k = 1; k <= num_speciesAttrs; k++)
	{
		assert(speciesAttrs(k)->SpType >= 0);

		//data preparation
		disp_param[k-1].spec_id = k;
		disp_param[k-1].gl_numseeds = numseeds[k-1];
		disp_param[k-1].cpu_id = get_cpu_id();

		for (int i = 1; i <= snr; i++)
		{
			for (int j = 1; j <= snc; j++)
			{
				LANDUNIT* l = sites.locateLanduPt(i, j);

				if (l != NULL && l->active())
				{
					SITE* lc_site = sites(i, j);

					const int lc_mature_treenum = lc_site->SpecieIndex(k)->GetMatureTreeValue();

					const int lc_totalseed = lc_site->specAtt(k)->TotalSeed;

					unsigned long lc_numseed = lc_mature_treenum * lc_totalseed * sites.TimeStep;

					assert(lc_numseed >= 0);

					if (lc_numseed > 0)
					{
						disp_param[k-1].rows.push_back(i);
						disp_param[k-1].cols.push_back(j);
						disp_param[k-1].lc_numseeds.push_back(lc_numseed);
					}
				}
			}
		}

		startDispersalThread(disp_param[k-1], lc_handle[k-1]);
		// sites.GetSeedDispersalonSite(disp_param[k-1]);
	}

	WaitForMultipleObjects(num_speciesAttrs, lc_handle, TRUE, INFINITE);

	for (int i = 0; i < num_speciesAttrs; ++i)
		CloseHandle(lc_handle[i]);

	delete[] lc_handle;
	delete[] disp_param;
	
	for (int k = 1; k <= num_speciesAttrs; k++)
	{
		for (int i = 1; i <= snr; i++)
		{
			for (int j = 1; j <= snc; j++)
			{
				LANDUNIT* l = sites.locateLanduPt(i, j);

				if (l != NULL && l->active())
				{
					SITE* siteptr = sites(i, j);
					const float local_rd = siteptr->RD;

					if (local_rd <= l->MaxRDArray[3])
					{
						const float lc_probrepro_k = l->probRepro(k);
						assert(lc_probrepro_k >= 0);
						if (lc_probrepro_k == 0)
							continue;
					
						// sites.GetSeedNumberOnSite_type2(i, j, k);

						SPECIE* specie_k = siteptr->SpecieIndex(k);
						assert(numseeds[k-1][i-1][j-1] <= ULONG_MAX);					
						specie_k->AvailableSeed = numseeds[k-1][i-1][j-1];
					}
				}
			}
		}
	}


	for(int k=0; k<num_speciesAttrs; k++)
	{
		delete[] numseeds[k][0];
		delete[] numseeds[k];
	}

	delete[] numseeds;

	clock_t gl_stop = clock();

	printf("calculating seed number takes %d seconds\n", (gl_stop - gl_start) / 1000);
		
	//seed dispersal
	initiateRDofSite_Landis70(snr, snc);
	
	printf("Start Seed Dispersal:\n");

	int active_num = 0;

	for (int i = 1; i <= snr; i++)
	{
		for (int j = 1; j <= snc; j++)
		{
			LANDUNIT* l = sites.locateLanduPt(i, j);
			
			if (l != NULL && l->active())
				active_num++;
		}
	}

	const int thread_avgsites = active_num / max_thread_num;

	printf("row = %d, col = %d, %d threads used in Seed Dispersal: %d\n", snr, snc, max_thread_num, thread_avgsites);
	
	PDP* ppdp = &pdp;
	
	if (thread_avgsites >= 10) //if (thread_avgsites >= 12000)
	{
		printf("multi-threads are used in calculating SiteDynamics\n");

		HANDLE* handle = new HANDLE [max_thread_num]();

		SiteDynParmVec* param = new SiteDynParmVec[max_thread_num]();

		SiteDynParmVec SP;
		SP.cpu_id = 0;
		SP.Ppdp = ppdp;
		int activenum = 0;

		for (int i = 1; i <= snr; i++)
		{
			for (int j = 1; j <= snc; j++)
			{
				LANDUNIT* l = sites.locateLanduPt(i, j);

				if (l != NULL && l->active())
				{
					const float local_rd = sites(i, j)->RD;

					int RDflag = -1;

					if (local_rd < l->MaxRDArray[0])
						RDflag = 0;
					else if (local_rd >= l->MaxRDArray[0] && local_rd < l->MaxRDArray[1])
						RDflag = 1;
					else if (local_rd >= l->MaxRDArray[1] && local_rd <= l->MaxRDArray[2])
						RDflag = 2;
					else if (local_rd > l->MaxRDArray[2] && local_rd <= l->MaxRDArray[3])
						RDflag = 3;
					else
						RDflag = 4;

					SP.rows.push_back(i);
					SP.cols.push_back(j);
					SP.RDflags.push_back(RDflag);

					activenum++;

					if(activenum == thread_avgsites && SP.cpu_id < max_thread_num - 1)
					{
						param[SP.cpu_id] = SP;
						startSiteDynamicsThread(param[SP.cpu_id], handle[SP.cpu_id]);

						SP.cpu_id++;
						SP.rows.clear();
						SP.cols.clear();
						SP.RDflags.clear();
						activenum = 0;
					}
				}
			}
		}

		assert(SP.cpu_id == max_thread_num - 1);
		param[SP.cpu_id] = SP;
		startSiteDynamicsThread(param[SP.cpu_id], handle[SP.cpu_id]);

		WaitForMultipleObjects(max_thread_num, handle, TRUE, INFINITE);

		for (int i = 0; i < max_thread_num; ++i)
			CloseHandle(handle[i]);

		delete[] handle;
		delete[] param;
	}
	else
	{
		printf("single thread is used in calculating SiteDynamics\n");

		for (int i = 1; i <= snr; i++)
		{
			printf("\r%3d%%", 100 * i / snr);

			for (int j = 1; j <= snc; j++)
			{
				LANDUNIT* l = sites.locateLanduPt(i, j);
				
				if (l != NULL && l->active())
				{
					const float local_rd = sites(i, j)->RD;

					int RDflag = -1;

					if (local_rd < l->MaxRDArray[0])
						RDflag = 0;
					else if (local_rd >= l->MaxRDArray[0] && local_rd < l->MaxRDArray[1])
						RDflag = 1;
					else if (local_rd >= l->MaxRDArray[1] && local_rd <= l->MaxRDArray[2])
						RDflag = 2;
					else if (local_rd > l->MaxRDArray[2] && local_rd <= l->MaxRDArray[3])
						RDflag = 3;
					else
						RDflag = 4;


					SiteDynmParam tmp;
					tmp.RDflag = RDflag;
					tmp.row = i;
					tmp.col = j;
					tmp.cpu_id = 0;
					tmp.Ppdp = ppdp;

					sites.SiteDynamics(&tmp);
				}
			}
		}
	}

	printf("\nSeed Dispersal Done\n");
}







//start killing trees gradually at the 80 % longevity until they reach their longevity
void KillTrees(int snr, int snc, PDP& pdp)
{
	const float e0 = 0.00000001f;             //for 32 bit system

	const int spec_num = speciesAttrs.number();

	int DTN_onespec_allage = 0;
	float DTNBIO_onespec_allage = 0;
	float DTNBA_onespec_allage = 0;				//add by houbin 3/15/2017

	float tmpBA = 0;
	float tmpBIO = 0;							//add by houbin 3/15/2017

	LANDUNIT *l = sites.locateLanduPt(snr, snc);

	for (int k = 1; k <= spec_num; k++) //sites.specNum
	{
		DTN_onespec_allage = 0;
		DTNBIO_onespec_allage = 0;
		DTNBA_onespec_allage = 0;

		float chanceDeath = 0.2f;

		const int longev = speciesAttrs(k)->longevity;
		const int numYears = longev / 5;

		const float chanceMod = 0.8f / (numYears + e0);
		
		SPECIE* lc_specie = sites(snr, snc)->SpecieIndex(k);


		const int m_beg = (longev - numYears) / sites.TimeStep;
		const int m_end = longev 			  / sites.TimeStep;
		
		for (int m = m_beg; m <= m_end; m++)
		{
			const unsigned int tmpTreeNum = lc_specie->getTreeNum(m, k);			

			unsigned int tmpMortality = 0;

			if (tmpTreeNum > 0)
			{
				const float consterm = chanceDeath * sites.TimeStep / 10;
				
				for (int x = 1; x <= tmpTreeNum; x++)
				{
					if (frand1() < consterm)
						tmpMortality += 1;
				}

				lc_specie->setTreeNum(m, k, max(0, tmpTreeNum - tmpMortality));
			}

			chanceDeath += chanceMod;

			const float lc_growrate = sites.GetGrowthRates(k, m, l->ltID);

			tmpBA = lc_growrate * lc_growrate / 40000 * 3.1415926 *tmpMortality;
			tmpBIO = exp(sites.GetBiomassData(speciesAttrs(k)->BioMassCoef, 1) + sites.GetBiomassData(speciesAttrs(k)->BioMassCoef, 2)*log(lc_growrate))*tmpMortality / 1000.00;

			DTN_onespec_allage += tmpMortality;
			DTNBA_onespec_allage += tmpBA;
			DTNBIO_onespec_allage += tmpBIO;
			if (pdp.TrackFlag == 1)
			{
				for (int agerange_index = 1; agerange_index <= sites.GetOutputAgeRangeModuleFlagArray(k - 1, COUNT); agerange_index++)
				{
					if (sites.GetOutputAgeRangeModuleFlagArray(k - 1, agerange_index, 0) < m * sites.TimeStep && m * sites.TimeStep <= sites.GetOutputAgeRangeModuleFlagArray(k - 1, agerange_index, 1))
					{
						const int const_id = (agerange_index - 1) * spec_num + k - 1;

						pdp.cDeadTreeNum_spec_agerange[snr - 1][snc - 1][const_id] += tmpMortality;
						pdp.cDeadTreeBA_spec_agerange[snr - 1][snc - 1][const_id] += tmpBA;
						pdp.cDeadTreeBio_spec_agerange[snr - 1][snc - 1][const_id] += tmpBIO;
					}
				}
			}

			
		}

		if (pdp.TrackFlag == 1)
		{
			pdp.cDeadTreeNum[snr - 1][snc - 1][k - 1] += DTN_onespec_allage;
			pdp.cDeadTreeBA[snr - 1][snc - 1][k - 1] += DTNBA_onespec_allage;
			pdp.cDeadTreeBio[snr - 1][snc - 1][k - 1] += DTNBIO_onespec_allage;
		}
	}

}














//This processes a singular Landis iteration.  It loops through each site followed by each species.  
//For every iteration of the loop grow and kill are called. Then seed availability is checked. 
//If seed is available and shade conditions are correct birth is called.
void singularLandisIteration(int itr, int snr, int snc, double wAdfGeoTransform[], PDP& pdp)
{
	time_t ltime, ltimeTemp;

	long ltimeDiff;

	fprintf(fpforTimeBU, "\nYear: %d:\n", itr); 

	
	if ((gDLLMode & G_WIND) != 0 && itr % sites.TimeStep_Wind == 0)
	{
		printMessage("\nProcessing windthrow.\n");

		time(&ltime);

		typedef void(*GetWindACTIVETYPE)(WIND*, int, int freq[], SITES*, SPECIESATTRS*, int, int, int, int);

		FARPROC  pnProc1 = GetProcAddress(gLibWind, "GetWindACTIVATE");

		if (!pnProc1)
		{
			DWORD m = GetLastError();
			errorSys("BDA: Error in access BDAActive.", STOP);
		}

		GetWindACTIVETYPE GetWindACTIVATE = (GetWindACTIVETYPE)pnProc1;

		GetWindACTIVATE(pWINDdll, itr / sites.TimeStep_Wind, freq, &sites, &speciesAttrs, parameters.cellSize, parameters.randSeed, parameters.numIter, gDLLMode);

		time(&ltimeTemp);

		ltimeDiff = (long)ltimeTemp - ltime;

		fprintf(fpforTimeBU, "Processing windthrow: %ld seconds\n", ltimeDiff);
		
		srand(parameters.randSeed + itr * 2);
	}



	time_t time1, time2, time3, time4, time5;

	if ((gDLLMode & G_FIRE) != 0 && itr%sites.TimeStep_Fire == 0)
	{
		time(&ltime);

		printf("Start simulating fire disturbance ... at %s", ctime(&ltime));

		srand(parameters.randSeed + itr / sites.TimeStep * 1);

		time(&time1);

		typedef CFIRE* (*GetFireACTIVATETYPE)(CFIRE* pf, int itr, int freq[], double wAdfGeoTransform[]);

		gLibFire = LoadLibrary("FireDll.dll");

		time(&time2);

		FARPROC pnProc = GetProcAddress(gLibFire, "GetFireACTIVATE");

		if (!pnProc)
		{
			DWORD m = GetLastError();
			errorSys("Fire: Error in load in Fire DLL, check if there is a Fire dll.", STOP);
		}

		time(&time3);

		GetFireACTIVATETYPE GetFireACTIVATE = (GetFireACTIVATETYPE)pnProc;

		time(&time4);

		GetFireACTIVATE(pFire, itr / sites.TimeStep_Fire, freq, wAdfGeoTransform);

		time(&ltimeTemp);

		ltimeDiff = (long)ltimeTemp - ltime;

		printf("Finish simulating fire disturbance at %sit took %ld seconds\n", ctime(&ltimeTemp), ltimeDiff);

		fprintf(fpforTimeBU, "Processing fire: %ld seconds\n", ltimeDiff);
	}




	if ((gDLLMode & G_BDA) != 0 && itr%sites.TimeStep_BDA == 0)
	{
		time_t lBDAtime, lBDAtimeTemp;

		long lBDAtimeDiff;

		time(&lBDAtime);

		printf("Start BDA at %s", ctime(&lBDAtime));

		srand(parameters.randSeed + itr / sites.TimeStep * 2);

		FARPROC  pnProc1 = GetProcAddress(gLibBDA, "GetBDAACTIVE");

		if (!pnProc1)
		{
			DWORD m = GetLastError();
			errorSys("BDA: Error in access BDAActive.", STOP);
		}


		typedef void(*GetBDAACTIVETYPE)(BDAS* pBDAS, int itr);
		GetBDAACTIVETYPE GetBDAACTIVE = (GetBDAACTIVETYPE)pnProc1;


		GetBDAACTIVE(pBDAS, itr / sites.TimeStep_BDA);

		srand(parameters.randSeed+itr*4);//Commented By Qia on Dec 18 2008

		time(&lBDAtimeTemp);

		printf("End BDA at %s", ctime(&lBDAtimeTemp));

		lBDAtimeDiff = (long)(lBDAtimeTemp - lBDAtime);

		printf("The whole BDA procedure took %ld seconds\n", lBDAtimeDiff);

		fprintf(fpforTimeBU, "Processing BDA: %ld seconds\n", lBDAtimeDiff);
	}



	if ((gDLLMode & G_FUEL) != 0 && itr%sites.TimeStep_Fuel == 0)
	{
		printf("Processing Fuel\n");

		time(&ltime);

		srand(parameters.randSeed + itr / sites.TimeStep * 3);

		FuelProcessEvent();

		srand(parameters.randSeed+itr*5);

		time(&ltimeTemp);

		ltimeDiff = (long)ltimeTemp - ltime;

		fprintf(fpforTimeBU, "Processing fuel: %ld seconds\n", ltimeDiff);
	}




	if ((gDLLMode & G_HARVEST) != 0 && itr%sites.TimeStep_Harvest == 0)
	{
		printMessage("Processing harvest events.\n");

		time(&ltime);

		HarvestprocessEvents(itr / sites.TimeStep);

		srand(parameters.randSeed+itr*6);

		putHarvestOutput(itr / sites.TimeStep_Harvest, wAdfGeoTransform, pdp.TrackFlag);

		time(&ltimeTemp);

		ltimeDiff = (long)ltimeTemp - ltime;

		fprintf(fpforTimeBU, "Processing harvest: %ld seconds\n", ltimeDiff);

	}



	if ((gDLLMode & G_FUELMANAGEMENT) != 0 && itr%sites.TimeStep_Fuel == 0)
	{
		printf("Fuel: Processing fuel management.\n");

		time(&ltime);

		srand(parameters.randSeed + itr / sites.TimeStep * 5);

		FuelManageProcessEvents(itr / sites.TimeStep_Fuel);

		srand(parameters.randSeed+itr*7);//Commented By Qia on Dec 18 2008

		time(&ltimeTemp);

		ltimeDiff = (long)ltimeTemp - ltime;

		fprintf(fpforTimeBU, "Processing fuel management: %ld seconds\n", ltimeDiff);
	}


	if ((gDLLMode & G_FUEL) != 0 && itr % sites.TimeStep_Fuel == 0)
		FuelputFuelOutput(itr / sites.TimeStep_Fuel, parameters.outputDir, parameters.cellSize, wAdfGeoTransform);


	if (itr % sites.TimeStep == 0)
	{
		time(&ltime);

		printf("Start succession ... at %s\n", ctime(&ltime));

		srand(parameters.randSeed + itr / sites.TimeStep * 6);

		landUnits.ReprodUpdate(itr / sites.TimeStep);

		if (pdp.TrackFlag == 1)
		{
			for (int i = 0; i < snr; i++)
			{
				for (int j = 0; j < snc; j++)
				{
					for (int k = 0; k < pdp.specnum; k++)
					{
						pdp.cDeadTreeNum[i][j][k] = 0;
						pdp.cDeadTreeBA[i][j][k] = 0;
						pdp.cDeadTreeBio[i][j][k] = 0;
					}
				}
			}
			for (int i = 0; i < snr; i++)
				for (int j = 0; j < snc; j++)
					for (int k = 0; k < pdp.specnum*pdp.interval_number; k++)
					{
						pdp.cDeadTreeBA_spec_agerange[i][j][k] = 0;
						pdp.cDeadTreeBio_spec_agerange[i][j][k] = 0;
						pdp.cDeadTreeNum_spec_agerange[i][j][k] = 0;
					}
		}
		

		Succession_Landis70(snr, snc, pdp);

		Succesion_output_Mortality_bio_ba(snr, snc, itr / parameters.timestep, pdp, wAdfGeoTransform);

		time(&ltimeTemp);

		ltimeDiff = (long)ltimeTemp - ltime;

		printf("Finish succession at %sit took %ld seconds\n", ctime(&ltimeTemp), ltimeDiff);

		fprintf(fpforTimeBU, "Processing succession: %ld seconds\n", ltimeDiff);

	}

	srand(parameters.randSeed);
}






void Succesion_output_Mortality_bio_ba(int snr, int snc, int itr, PDP& pdp, double wAdfGeoTransform[])
{
	if (pdp.TrackFlag == 1)
	{
		if (sites.OutputSuccessionFlag == 1)
		{
			for (int iii = 0; iii < pdp.specnum; iii++)
			{
				SITE* siteptr = sites(1, 1);

				if (sites.GetOutputModuleFlagArray(iii, DTN))
				{
					if (strcmp(siteptr->specAtt(iii + 1)->name, "N/A") != 0)
					{
						MAP8 map_dtn;
						map_dtn.dim(snr, snc);

						for (int jjj = 1; jjj <= snr; jjj++)
						{
							for (int kkk = 1; kkk <= snc; kkk++)
							{
								map_dtn(jjj, kkk) = pdp.cDeadTreeNum[jjj - 1][kkk - 1][iii];
							}
						}

						char dirMortality[255];
						char dirDTN[255];
						char strDTN[255];
						sprintf(dirMortality, "%s/Succession", parameters.outputDir);
						sprintf(dirDTN, "%s/DTN", dirMortality);
						sprintf(strDTN, "%s/%s_TreeNum_succ_%d", dirDTN, siteptr->specAtt(iii + 1)->name, itr*parameters.timestep);
						map_dtn.setCellSize(parameters.cellSize);
						map_dtn.write(strDTN, red2, green2, blue2, wAdfGeoTransform);
					}
				}

				if (sites.GetOutputModuleFlagArray(iii, DTNBA))
				{
					if (strcmp(siteptr->specAtt(iii + 1)->name, "N/A") != 0)
					{
						MAP8 map_dtn_ba;
						map_dtn_ba.dim(snr, snc);
						for (int jjj = 1; jjj <= snr; jjj++)
						{
							for (int kkk = 1; kkk <= snc; kkk++)
							{
								int x = ((jjj - 1)*snc) + kkk - 1;
								map_dtn_ba.data_bio[x] = pdp.cDeadTreeBA[jjj - 1][kkk - 1][iii];
							}
						}

						char dirMortality[255];
						char dirDTNBA[255];
						char strDTNBA[255];
						sprintf(dirMortality, "%s/Succession", parameters.outputDir);
						sprintf(dirDTNBA, "%s/DTNBA", dirMortality);
						sprintf(strDTNBA, "%s/%s_BA_succ_%d", dirDTNBA, siteptr->specAtt(iii + 1)->name, itr*parameters.timestep);
						map_dtn_ba.setCellSize(parameters.cellSize);
						map_dtn_ba.write(strDTNBA, red2, green2, blue2, wAdfGeoTransform);
					}
				}

				if (sites.GetOutputModuleFlagArray(iii, DTNBIO))
				{
					if (strcmp(siteptr->specAtt(iii + 1)->name, "N/A") != 0)
					{
						MAP8 map_dtn_bio;
						map_dtn_bio.dim(snr, snc);
						for (int jjj = 1; jjj <= snr; jjj++)
						{
							for (int kkk = 1; kkk <= snc; kkk++)
							{
								int x = ((jjj - 1)*snc) + kkk - 1;
								map_dtn_bio.data_bio[x] = pdp.cDeadTreeBio[jjj - 1][kkk - 1][iii];
							}
						}

						char dirMortality[255];
						char dirDTNBIO[255];
						char strDTNBIO[255];
						sprintf(dirMortality, "%s/Succession", parameters.outputDir);
						sprintf(dirDTNBIO, "%s/DTNBIO", dirMortality);
						sprintf(strDTNBIO, "%s/%s_BIO_succ_%d", dirDTNBIO, siteptr->specAtt(iii + 1)->name, itr*parameters.timestep);
						map_dtn_bio.setCellSize(parameters.cellSize);
						map_dtn_bio.write(strDTNBIO, red2, green2, blue2, wAdfGeoTransform);
					}
				}
			}
		}
		if (sites.OutputAgeRangeSuccessionFlag == 1)								//add by houbin 11/3/2017
		{
			for (int iii = 0; iii < pdp.specnum; iii++)
			{
				SITE * siteptr = sites(1, 1);
				if (sites.GetOutputAgeRangeModuleFlagArray(iii, DTN))
				{
					for (int agerange_index = 1; agerange_index <= sites.GetOutputAgeRangeModuleFlagArray(iii, COUNT); agerange_index++)
					{
						int low = sites.GetOutputAgeRangeModuleFlagArray(iii, agerange_index, 0);
						int high = sites.GetOutputAgeRangeModuleFlagArray(iii, agerange_index, 1);
						MAP8 map_dtn;
						map_dtn.dim(snr, snc);
						for (int jjj = 1; jjj <= snr; jjj++)
						{
							for (int kkk = 1; kkk <= snc; kkk++)
							{
								map_dtn(jjj, kkk) = pdp.cDeadTreeNum_spec_agerange[jjj - 1][kkk - 1][(agerange_index - 1)*pdp.specnum + iii];
							}
						}
						char dirMortality[255];
						char dirDTN[255];
						char strDTN[255];
						sprintf(dirMortality, "%s/Succession", parameters.outputDir);
						sprintf(dirDTN, "%s/SPEC_AGERANGE", dirMortality);
						sprintf(strDTN, "%s/%s_%d_%d_TreeNum_succ_%d", dirDTN, siteptr->specAtt(iii + 1)->name, low, high, itr*parameters.timestep);
						map_dtn.setCellSize(parameters.cellSize);
						map_dtn.write(strDTN, red2, green2, blue2, wAdfGeoTransform);
					}
				}
				if (sites.GetOutputAgeRangeModuleFlagArray(iii, DTNBA))
				{
					for (int agerange_index = 1; agerange_index <= sites.GetOutputAgeRangeModuleFlagArray(iii, COUNT); agerange_index++)
					{
						int low = sites.GetOutputAgeRangeModuleFlagArray(iii, agerange_index, 0);
						int high = sites.GetOutputAgeRangeModuleFlagArray(iii, agerange_index, 1);
						MAP8 map_dtn_ba;
						map_dtn_ba.dim(snr, snc);
						for (int jjj = 1; jjj <= snr; jjj++)
						{
							for (int kkk = 1; kkk <= snc; kkk++)
							{
								int x = ((jjj - 1)*snc) + kkk - 1;
								map_dtn_ba.data_bio[x] = pdp.cDeadTreeBA_spec_agerange[jjj - 1][kkk - 1][(agerange_index - 1)*pdp.specnum + iii];
								//map_dtn(jjj, kkk) = pdp.cDeadTreeBA_spec_agerange[jjj - 1][kkk - 1][(agerange_index - 1)*pdp.specnum + iii];
							}
						}
						char dirMortality[255];
						char dirDTNBA[255];
						char strDTNBA[255];
						sprintf(dirMortality, "%s/Succession", parameters.outputDir);
						sprintf(dirDTNBA, "%s/SPEC_AGERANGE", dirMortality);
						sprintf(strDTNBA, "%s/%s_%d_%d_TreeBA_succ_%d", dirDTNBA, siteptr->specAtt(iii + 1)->name, low, high, itr*parameters.timestep);
						map_dtn_ba.setCellSize(parameters.cellSize);
						map_dtn_ba.write(strDTNBA, red2, green2, blue2, wAdfGeoTransform);
					}
				}
				if (sites.GetOutputAgeRangeModuleFlagArray(iii, DTNBIO))
				{
					for (int agerange_index = 1; agerange_index <= sites.GetOutputAgeRangeModuleFlagArray(iii, COUNT); agerange_index++)
					{
						int low = sites.GetOutputAgeRangeModuleFlagArray(iii, agerange_index, 0);
						int high = sites.GetOutputAgeRangeModuleFlagArray(iii, agerange_index, 1);
						MAP8 map_dtn_bio;
						map_dtn_bio.dim(snr, snc);
						for (int jjj = 1; jjj <= snr; jjj++)
						{
							for (int kkk = 1; kkk <= snc; kkk++)
							{
								int x = ((jjj - 1)*snc) + kkk - 1;
								map_dtn_bio.data_bio[x] = pdp.cDeadTreeBio_spec_agerange[jjj - 1][kkk - 1][(agerange_index - 1)*pdp.specnum + iii];
								//map_dtn(jjj, kkk) = pdp.cDeadTreeBA_spec_agerange[jjj - 1][kkk - 1][(agerange_index - 1)*pdp.specnum + iii];
							}
						}
						char dirMortality[255];
						char dirDTNBIO[255];
						char strDTNBIO[255];
						sprintf(dirMortality, "%s/Succession", parameters.outputDir);
						sprintf(dirDTNBIO, "%s/SPEC_AGERANGE", dirMortality);
						sprintf(strDTNBIO, "%s/%s_%d_%d_TreeBIO_succ_%d", dirDTNBIO, siteptr->specAtt(iii + 1)->name, low, high, itr*parameters.timestep);
						map_dtn_bio.setCellSize(parameters.cellSize);
						map_dtn_bio.write(strDTNBIO, red2, green2, blue2, wAdfGeoTransform);
					}
				}
			}
		}
	}
	
	

	
}


