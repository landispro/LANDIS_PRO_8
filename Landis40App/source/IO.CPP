#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <vector>
#include <direct.h>
#include <errno.h>
#include <windows.h>

#include "defines.h"
#include "system1.h"
#include "error.h"
#include "species.h"
#include "site.h"
#include "map8.h"
#include "reclass.h"
#include "reclass2.h"
#include "fire.h"
#include "wind.h"
#include "landis.h"
#include "io.h"
#include "GlobalVariables.h"
#include "GlobalFunctions.h"
#include "BDAExport.h"
#include "gdal_priv.h"
#include "cpl_conv.h" // for CPLMalloc()
#include "cpl_string.h"
#include "ogr_spatialref.h"

//#include <vld.h>

#define TPA		0
#define BA		1
#define Bio		2
#define Car		3
#define IV		4
#define Seeds	5
#define RDensity	6
#define DBH 7

#define COUNT 3			//add by houbin 10/25/2017



int reclassMethods = 0, numAgeMaps = 0;

static int red[maxLeg] = { 0, 0, 100, 150, 200, 0, 0, 0, 150, 0, 150, 255, 80, 150, 255 };
static int green[maxLeg] = { 0, 0, 0, 0, 0, 100, 150, 255, 0, 150, 150, 255, 80, 150, 255 };
static int blue[maxLeg] = { 0, 150, 0, 0, 0, 0, 0, 0, 150, 150, 0, 0, 80, 150, 255 };

static int red2[maxLeg] = { 0, 70, 0, 0, 0, 0, 0, 0, 200, 100, 255, 150, 200, 200, 255, 255 };
static int green2[maxLeg] = { 0, 70, 0, 0, 0, 150, 200, 255, 30, 200, 50, 50, 200, 0, 255 };
static int blue2[maxLeg] = { 0, 70, 125, 200, 255, 0, 100, 0, 30, 50, 50, 0, 0, 255 };

static int red3[maxLeg] = { 0, 70, 0, 0, 0, 0, 0, 0, 200, 100, 255, 150, 200, 200, 255, 255 };
static int green3[maxLeg] = { 0, 70, 0, 0, 0, 150, 200, 255, 30, 200, 50, 50, 200, 0, 255 };
static int blue3[maxLeg] = { 0, 70, 125, 200, 255, 0, 100, 0, 30, 50, 50, 0, 0, 255 };

static int red4[maxLeg] = { 0, 70, 0, 0, 0, 0, 0, 0, 200, 100, 255, 150, 200, 200, 255, 255 };
static int green4[maxLeg] = { 0, 70, 0, 0, 0, 150, 200, 255, 30, 200, 50, 50, 200, 0, 255 };
static int blue4[maxLeg] = { 0, 70, 125, 200, 255, 0, 100, 0, 30, 50, 50, 0, 0, 255 };




extern int gDLLMode;

extern HINSTANCE	  gLibBDA;


vector<string> BioMassFileNames;
vector<string> BasalFileNames;
vector<string> TreesFileNames;
vector<string> IVFileNames; //add April 05 2010
vector<string> DBHFileNames; //add By Qia on April 09 2010
vector<string> RDFileNames;// Add by Qia on Nov 03 2010
vector<string> SeedsFileNames;//Add By Qia on Nov 03 2010

// int flagoutputBiomass;
// int flagoutputBasal;
// int flagoutputTrees;
// int flagoutputIV;
int flagoutputDBH;


double *AgeDistOutputBuffer_TPA = NULL;
double *AgeDistOutputBuffer_BA = NULL;
double *AgeDistOutputBuffer_TPA_landtype = NULL;
double *AgeDistOutputBuffer_BA_landtype = NULL;



void AgeDistOutputBufferInitialize(int specNum, int LandTypeNum)
{
	if (sites.Flag_AgeDistStat == 0)
		return;

	const int consterm = 500 / sites.TimeStep;
	const int leng = specNum * consterm * consterm;

	AgeDistOutputBuffer_TPA = new double[leng]();
	AgeDistOutputBuffer_BA = new double[leng]();

	AgeDistOutputBuffer_TPA_landtype = new double[leng * LandTypeNum]();
	AgeDistOutputBuffer_BA_landtype = new double[leng * LandTypeNum]();
}




void AgeDistOutputBufferRelease()
{
	if (sites.Flag_AgeDistStat == 0)
		return;

	if (AgeDistOutputBuffer_TPA)
		delete AgeDistOutputBuffer_TPA;

	if (AgeDistOutputBuffer_BA)
		delete AgeDistOutputBuffer_BA;

	if (AgeDistOutputBuffer_TPA_landtype)
		delete AgeDistOutputBuffer_TPA_landtype;

	if (AgeDistOutputBuffer_BA_landtype)
		delete AgeDistOutputBuffer_BA_landtype;
}




void SetAgeDistoutputBuffer(int type, int specIndex, int ageIndex, int yearIndex, double value)
{
	if (sites.Flag_AgeDistStat == 0)
		return;

	const int dimension = 500 / sites.TimeStep;
	const int dimen_sqr = dimension * dimension;

	const int index = (specIndex - 1) * dimen_sqr + (ageIndex - 1) * dimension + yearIndex - 1;

	if (type == BA)
		AgeDistOutputBuffer_BA[index] = value;

	if (type == TPA)
		AgeDistOutputBuffer_TPA[index] = value;
}




double GetAgeDistoutputBuffer(int type, int specIndex, int ageIndex, int yearIndex)
{
	const int dimension = 500 / sites.TimeStep;
	const int dimen_sqr = dimension * dimension;

	const int index = (specIndex - 1) * dimen_sqr + (ageIndex - 1) * dimension + yearIndex - 1;

	if (type == BA)
		return AgeDistOutputBuffer_BA[index];
	else
	{
		assert(type == TPA);
		return AgeDistOutputBuffer_TPA[index];
	}
}




void SetAgeDistoutputBuffer_Landtype(int type, int specIndex, int ageIndex, int yearIndex, int landtypeIndex, double value)
{
	if (sites.Flag_AgeDistStat == 0)
		return;

	const int dimension = 500 / sites.TimeStep;
	const int dimen_sqr = dimension * dimension;

	const int index = landtypeIndex * sites.specNum * dimen_sqr + (specIndex - 1) * dimen_sqr + (ageIndex - 1) * dimension + yearIndex - 1;

	if (type == BA)
		AgeDistOutputBuffer_BA_landtype[index] = value;

	if (type == TPA)
		AgeDistOutputBuffer_TPA_landtype[index] = value;
}







double GetAgeDistoutputBuffer_Landtype(int type, int specIndex, int ageIndex, int yearIndex, int landtypeIndex)
{
	const int dimension = 500 / sites.TimeStep;
	const int dimen_sqr = dimension * dimension;

	int index = landtypeIndex * sites.specNum * dimen_sqr + (specIndex - 1) * dimen_sqr + (ageIndex - 1) * dimension + yearIndex - 1;

	if (type == BA)
		return AgeDistOutputBuffer_BA_landtype[index];
	else
	{
		assert(type == TPA);
		return AgeDistOutputBuffer_TPA_landtype[index];
	}
}







//This function is to look up the sites with same species structure
//for future delete of redundant site
//To make sure all sites are different from each other during run time
void lookupredundant(int *combineMatrix, int numCovers)
{
	for (int i = 0; i < numCovers - 1; i++)
	{
		if (i == combineMatrix[i])
		{
			SITE* siteI = sites.SortedIndex.at(i);

			for (int j = i + 1; j < numCovers; j++)
			{
				SITE* siteJ = sites.SortedIndex.at(j);

				if (j == combineMatrix[j])
				{
					if (sites.SITE_compare(siteI, siteJ) == 0)
						combineMatrix[j] = i;
				}
			}
		}
	}
}





//after looking up, this function is to delete the redunt sites 
void deleteRedundantInitial(int *combineMatrix, int numCovers)
{
	for (int i = numCovers - 1; i >= 0; i--)
	{
		if (i != combineMatrix[i])
		{
			SITE* temp = sites.SortedIndex.at(i);

			delete temp;

			sites.SortedIndex.erase(sites.SortedIndex.begin() + i);
		}
	}
}





//This will convert HSV colors to RGB.
void HSV_to_RGB(float h, float s, float v, int &red, int &green, int &blue)
{
	float f, p, q, t, r, g, b;

	if (h == 360.0)
		h = 0.0;

	h = h / (float)60.0;

	int i = (int)h;

	f = h - i;

	p = v * (1 - s);
	q = v * (1 - (s * f));
	t = v * (1 - (s * (1 - f)));

	switch (i)
	{
		case 0:   r = v; g = t; b = p; break;
		case 1:   r = q; g = v; b = p; break;
		case 2:   r = p; g = v; b = t; break;
		case 3:   r = p; g = q; b = v; break;
		case 4:   r = t; g = p; b = v; break;
		case 5:   r = v; g = p; b = q; break;
		default:  printf("Exit from HSV_to_RGB\n"); exit(0);
	}

	red = (int)(r*255.0);   //Nim: cast to (int)

	green = (int)(g*255.0); //Nim: cast to (int)

	blue = (int)(b*255.0);  //Nim: cast to (int)

}



void printMessage(const char *mssg)
{
	printf("%s\n", mssg);
	fflush(stdout);
}





void outputFileheader(FILE *fp)
{
	fprintf(fp, "ncols  %d\n", sites.numColumns());
	fprintf(fp, "nrows  %d\n", sites.numRows());
	fprintf(fp, "xllcorner  %.f\n", sites.xLLCorner - *(((float*)sites.getHeader()) + 30) / 2);
	fprintf(fp, "yllcorner  %.f\n", sites.yLLCorner - *(((float*)sites.getHeader()) + 30)*sites.numRows() + *(((float*)sites.getHeader()) + 30) / 2);
	fprintf(fp, "cellsize  %.f\n", *(((float*)sites.getHeader()) + 30));
	fprintf(fp, "NODATA_value  -9999\n");
}



void initiateOutput_landis70Pro()
{
	const int species_num = sites.specNum;

	char filename[256];

	if (strcmp(reMethods[0], "N/A") != 0)
	{
		for (int i = 1; i <= species_num; i++)
		{
			sprintf(filename, "%s_Bio", speciesAttrs(i)->name);

			BioMassFileNames.push_back(string(filename));
		}

		// flagoutputBiomass = 1;
	}
	else
	{
		// flagoutputBiomass = 0;
	}



	if (strcmp(reMethods[1], "N/A") != 0)
	{
		for (int i = 1; i <= species_num; i++)
		{
			sprintf(filename, "%s_BA", speciesAttrs(i)->name);

			BasalFileNames.push_back(string(filename));
		}

		// flagoutputBasal = 1;
	}
	else
	{
		// flagoutputBasal = 0;
	}



	if (strcmp(reMethods[2], "N/A") != 0)
	{
		for (int i = 1; i <= species_num; i++)
		{
			sprintf(filename, "%s_TreeNum", speciesAttrs(i)->name);

			TreesFileNames.push_back(string(filename));
		}

		// flagoutputTrees = 1;
	}
	else
	{
		// flagoutputTrees = 0;
	}


	for (int i = 1; i <= species_num; i++)
	{
		const char* lc_name = speciesAttrs(i)->name;

		sprintf(filename, "%s_IV", lc_name);
		IVFileNames.push_back(string(filename));

		sprintf(filename, "%s_AvailableSeed", lc_name);
		SeedsFileNames.push_back(string(filename));

		sprintf(filename, "%s_RelativeDensity", lc_name);
		RDFileNames.push_back(string(filename));

		sprintf(filename, "%s_DBH", lc_name);
		DBHFileNames.push_back(string(filename));
	}

	// flagoutputIV = 1;
	flagoutputDBH = 1;

	AgeDistOutputBufferInitialize(species_num, landUnits.number());

}




void AgeDistOutputFromBufferToFile()
{
	if (sites.Flag_AgeDistStat == 0)
		return;

	printf("Output BA and TPA stat:\n");

	char str[300] = {0};
	sprintf(str, "%s/BA_TPADist.txt", parameters.outputDir);
	
	FILE *output;
	if ((output = LDfopen(str, 3)) == NULL)
		errorSys("can not open output file 1", STOP);

	for (int k = 1; k <= sites.specNum; k++)
	{
		fprintf(output, "%s:\n", speciesAttrs(k)->name);
		fprintf(output, "AgeRange\\Year: ");

		const int year_end = sites.GetAgeDistStat_YearCount(k - 1);

		for (int count_year = 1; count_year <= year_end; count_year++)
		{
			int year = sites.GetAgeDistStat_YearVal(k - 1, count_year);

			fprintf(output, "TPA_%d ", year);
		}

		for (int count_year = 1; count_year <= year_end; count_year++)
		{
			int year = sites.GetAgeDistStat_YearVal(k - 1, count_year);

			fprintf(output, "BA_%d ", year);
		}

		fprintf(output, "\n");

		const int age_end = sites.GetAgeDistStat_AgeRangeCount(k - 1);

		for (int count_age = 1; count_age <= age_end; count_age++)
		{
			int age1, age2;

			sites.GetAgeDistStat_AgeRangeVal(k - 1, count_age, age1, age2);

			fprintf(output, "Age%d-%d ", age1, age2);

			for (int count_year = 1; count_year <= year_end; count_year++)
			{
				double value = GetAgeDistoutputBuffer(TPA, k, count_age, count_year);

				fprintf(output, "%lf ", value);
			}

			for (int count_year = 1; count_year <= year_end; count_year++)
			{
				double value = GetAgeDistoutputBuffer(BA, k, count_age, count_year);

				fprintf(output, "%lf ", value);
			}

			fprintf(output, "\n");
		}
	}

	fprintf(output, "\n\n");

	const int landunit_num = landUnits.number();

	for (int count_landtype = 0; count_landtype < landunit_num; count_landtype++)
	{
		fprintf(output, "Landtype:%d\n", count_landtype);

		for (int k = 1; k <= sites.specNum; k++)
		{
			fprintf(output, "%s:\n", speciesAttrs(k)->name);
			fprintf(output, "AgeRange\\Year: ");

			const int year_end = sites.GetAgeDistStat_YearCount(k - 1);

			for (int count_year = 1; count_year <= year_end; count_year++)
			{
				int year = sites.GetAgeDistStat_YearVal(k - 1, count_year);

				fprintf(output, "TPA_%d ", year);
			}

			for (int count_year = 1; count_year <= year_end; count_year++)
			{
				int year = sites.GetAgeDistStat_YearVal(k - 1, count_year);

				fprintf(output, "BA_%d ", year);
			}

			fprintf(output, "\n");

			const int age_end = sites.GetAgeDistStat_AgeRangeCount(k - 1);

			for (int count_age = 1; count_age <= age_end; count_age++)
			{
				int age1, age2;

				sites.GetAgeDistStat_AgeRangeVal(k - 1, count_age, age1, age2);

				fprintf(output, "Age%d-%d ", age1, age2);

				for (int count_year = 1; count_year <= year_end; count_year++)
				{
					double value = GetAgeDistoutputBuffer_Landtype(TPA, k, count_age, count_year, count_landtype);

					fprintf(output, "%lf ", value);
				}

				for (int count_year = 1; count_year <= year_end; count_year++)
				{
					double value = GetAgeDistoutputBuffer_Landtype(BA, k, count_age, count_year, count_landtype);

					fprintf(output, "%lf ", value);
				}

				fprintf(output, "\n");
			}
		}
	}


	AgeDistOutputBufferRelease();

	fclose(output);

}







void putOutput_AgeDistStat(int itr)
{
	if (sites.Flag_AgeDistStat == 0)
		return;

	const int num_species = sites.specNum;
	const double local_const = 3.1415926 / (4 * 10000.00);
	const int itr_m_timestep = itr * sites.TimeStep;

	const int numrow = sites.numRows();
	const int numcol = sites.numColumns();

	//Below is for every species
	for (int k = 1; k <= num_species; k++)
	{
		const int end_age = sites.GetAgeDistStat_AgeRangeCount(k - 1);
		const int endyear = sites.GetAgeDistStat_YearCount(k - 1);
		const int lc_long = speciesAttrs(k)->longevity;

		for (int count_age = 1; count_age <= end_age; count_age++)
		{
			int age1 = 0, age2 = 0, year = 0;

			sites.GetAgeDistStat_AgeRangeVal(k - 1, count_age, age1, age2);

			const int m_beg = min(lc_long, age1) / sites.TimeStep;
			const int m_end = min(lc_long, age2) / sites.TimeStep;

			for (int count_year = 1; count_year <= endyear; count_year++)
			{
				year = sites.GetAgeDistStat_YearVal(k - 1, count_year);

				if (itr_m_timestep == year)
				{
					double TmpBasalAreaS = 0;
					int TmpTreesS = 0;

					for (int i = numrow; i > 0; i--)
					{
						for (int j = 1; j <= numcol; j++)
						{
							LANDUNIT *l = sites.locateLanduPt(i, j);
							SPECIE* specie_k = sites(i, j)->SpecieIndex(k);

							for (int m = m_beg; m <= m_end; m++)
							{
								const float lc_growrate = sites.GetGrowthRates(k, m, l->ltID);
								const int   lc_tree_num = specie_k->getTreeNum(m, k);

								TmpBasalAreaS += lc_growrate * lc_growrate * lc_tree_num * local_const;
								TmpTreesS += lc_tree_num;
							}
						}
					}

					SetAgeDistoutputBuffer(BA, k, count_age, count_year, TmpBasalAreaS);
					SetAgeDistoutputBuffer(TPA, k, count_age, count_year, TmpTreesS);
				}
			}
		}
	}



	const int num_landtype = landUnits.number();

	for (int count_landtype = 0; count_landtype < num_landtype; count_landtype++)
	{
		for (int k = 1; k <= num_species; k++)
		{
			const int end_age = sites.GetAgeDistStat_AgeRangeCount(k - 1);
			const int endyear = sites.GetAgeDistStat_YearCount(k - 1);
			const int lc_long = speciesAttrs(k)->longevity;

			for (int count_age = 1; count_age <= end_age; count_age++)
			{
				int age1 = 0, age2 = 0, year = 0;

				sites.GetAgeDistStat_AgeRangeVal(k - 1, count_age, age1, age2);

				const int m_beg = min(lc_long, age1) / sites.TimeStep;
				const int m_end = min(lc_long, age2) / sites.TimeStep;

				for (int count_year = 1; count_year <= endyear; count_year++)
				{
					year = sites.GetAgeDistStat_YearVal(k - 1, count_year);

					if (itr_m_timestep == year)
					{
						double TmpBasalAreaS = 0;
						int TmpTreesS = 0;

						for (int i = numrow; i > 0; i--)
						{
							for (int j = 1; j <= numcol; j++)
							{
								LANDUNIT *l = sites.locateLanduPt(i, j);

								if (l == landUnits(count_landtype))
								{
									SPECIE* specie_k = sites(i, j)->SpecieIndex(k);

									for (int m = m_beg; m <= m_end; m++)
									{
										const float lc_growrate = sites.GetGrowthRates(k, m, l->ltID);
										const int   lc_tree_num = specie_k->getTreeNum(m, k);

										TmpBasalAreaS += lc_growrate * lc_growrate * lc_tree_num * local_const;
										TmpTreesS += lc_tree_num;
									}
								}
							}
						}

						SetAgeDistoutputBuffer_Landtype(BA, k, count_age, count_year, count_landtype, TmpBasalAreaS);
						SetAgeDistoutputBuffer_Landtype(TPA, k, count_age, count_year, count_landtype, TmpTreesS);
					}
				}
			}
		}
	}
}







void putOutput_Landis70Pro(int rep, int itr, int freq[], double wAdfGeoTransform[])
{
	time_t ltime, ltimeTemp;

	const char *pszFormat = "HFA";

	GDALDriver *poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);

	if (poDriver == NULL)
		exit(1);

	char** papszMetadata = poDriver->GetMetadata();

	char** papszOptions = NULL;

	float *pafScanline1;
	float *pafScanline2;
	float *pafScanline3;
	float *pafScanline4;
	float *pafScanline5;
	unsigned int *pintScanline;

	char *pszSRS_WKT = NULL;

	OGRSpatialReference oSRS;

	oSRS.SetUTM(11, TRUE);

	oSRS.SetWellKnownGeogCS("HEAD74");

	oSRS.exportToWkt(&pszSRS_WKT);

	CPLFree(pszSRS_WKT);

	GDALRasterBand *outPoBand;
	GDALRasterBand *outPoBand1;
	GDALRasterBand *outPoBand2;
	GDALRasterBand *outPoBand3;
	GDALRasterBand *outPoBand4;
	GDALRasterBand *outPoBand5;

	GDALDataset *poDstDS;
	GDALDataset *poDstDS1;
	GDALDataset *poDstDS2;
	GDALDataset *poDstDS3;
	GDALDataset *poDstDS4;
	GDALDataset *poDstDS5;

	time(&ltime);

	printf("Start 7.0 Style writing output at %s", ctime(&ltime));

	const int numrow = sites.numRows();
	const int numcol = sites.numColumns();

	const int total_size = numrow * numcol;

	const int itr_m_timestep = itr * sites.TimeStep;

	const float biomass_threshold = sites.GetBiomassThreshold();

	const double local_const = 3.1415926 / (4 * 10000.00);

	const int cellsize_square = sites.CellSize * sites.CellSize;

	float* pafScanline = (float *)CPLMalloc(sizeof(float)* total_size);

	//Below is for every species
	
	//1.Bio
	for (int k = 1; k <= sites.specNum; k++)
	{
		const int m_end = speciesAttrs(k)->longevity / sites.TimeStep;

		if (sites.GetOutputGeneralFlagArray(k - 1, Bio))
		{
			if (strcmp(BioMassFileNames[k - 1].c_str(), "N/A") != 0)
			{
				char str1[100];
				sprintf(str1, "%s/%s_%d.img", parameters.outputDir, BioMassFileNames[k - 1].c_str(), itr_m_timestep);//*

				poDstDS = poDriver->Create(str1, numcol, numrow, 1, GDT_Float32, papszOptions);//*

				if (poDstDS == NULL)
					errorSys("Img file not be created.", STOP);

				poDstDS->SetGeoTransform(wAdfGeoTransform);//*

				outPoBand = poDstDS->GetRasterBand(1);

				const float biomass1 = sites.GetBiomassData(speciesAttrs(k)->BioMassCoef, 1);
				const float biomass2 = sites.GetBiomassData(speciesAttrs(k)->BioMassCoef, 2);


				for (int i = numrow; i > 0; i--)
				{
					const int part_index = (numrow - i) * numcol - 1;

					for (int j = 1; j <= numcol; j++)
					{
						LANDUNIT *l = sites.locateLanduPt(i, j);
						double TmpBiomassS = 0;

						SPECIE* specie_k = sites(i, j)->SpecieIndex(k);

						for (int m = 1; m <= m_end; m++)
						{
							const float lc_growrate = sites.GetGrowthRates(k, m, l->ltID);

							if (lc_growrate >= biomass_threshold)
								TmpBiomassS += exp(biomass1 + biomass2 * log(lc_growrate)) * specie_k->getTreeNum(m, k) / 1000.00;
						}

						*(pafScanline + part_index + j) = TmpBiomassS;
					}
				}


				outPoBand->RasterIO(GF_Write, 0, 0, numcol, numrow, pafScanline, numcol, numrow, GDT_Float32, 0, 0);

				if (poDstDS != NULL)
					GDALClose((GDALDatasetH)poDstDS);
			}

		}



		//2.BA
		if (sites.GetOutputGeneralFlagArray(k - 1, BA))
		{
			if (strcmp(BasalFileNames[k - 1].c_str(), "N/A") != 0)
			{
				char str1[100];
				sprintf(str1, "%s/%s_%d.img", parameters.outputDir, BasalFileNames[k - 1].c_str(), itr_m_timestep);//* change

				poDstDS = poDriver->Create(str1, numcol, numrow, 1, GDT_Float32, papszOptions);

				if (poDstDS == NULL)
					errorSys("Img file not be created.", STOP);

				poDstDS->SetGeoTransform(wAdfGeoTransform);

				outPoBand = poDstDS->GetRasterBand(1);

				for (int i = numrow; i > 0; i--)
				{
					const int part_index = (numrow - i) * numcol - 1;

					for (int j = 1; j <= numcol; j++)
					{
						LANDUNIT *l = sites.locateLanduPt(i, j);
						double TmpBasalAreaS = 0;

						SPECIE* specie_k = sites(i, j)->SpecieIndex(k);

						for (int m = 1; m <= m_end; m++)
						{
							const float lc_growrate = sites.GetGrowthRates(k, m, l->ltID);

							TmpBasalAreaS += lc_growrate * lc_growrate * local_const * specie_k->getTreeNum(m, k);
						}

						*(pafScanline + part_index + j) = TmpBasalAreaS;
					}
				}

				outPoBand->RasterIO(GF_Write, 0, 0, numcol, numrow, pafScanline, numcol, numrow, GDT_Float32, 0, 0);

				if (poDstDS != NULL)
					GDALClose((GDALDatasetH)poDstDS);
			}

		}



		//3.TPA  TreeNumber
		pintScanline = (unsigned int *)CPLMalloc(sizeof(unsigned int)* total_size);

		if (sites.GetOutputGeneralFlagArray(k - 1, TPA))
		{
			if (strcmp(TreesFileNames[k - 1].c_str(), "N/A") != 0)
			{
				char str1[100];
				sprintf(str1, "%s/%s_%d.img", parameters.outputDir, TreesFileNames[k - 1].c_str(), itr_m_timestep);//* change

				poDstDS = poDriver->Create(str1, numcol, numrow, 1, GDT_UInt32, papszOptions);

				poDstDS->SetGeoTransform(wAdfGeoTransform);

				outPoBand = poDstDS->GetRasterBand(1);


				if (poDstDS == NULL)
					errorSys("Img file not be created.", STOP);

				
				for (int i = numrow; i > 0; i--)
				{
					const int part_index = (numrow - i) * numcol - 1;

					for (int j = 1; j <= numcol; j++)
					{
						SPECIE* specie_k = sites(i, j)->SpecieIndex(k);

						int TmpTreesS = 0;

						for (int m = 1; m <= m_end; m++)
						{
							TmpTreesS += specie_k->getTreeNum(m, k);
						}

						*(pintScanline + part_index + j) = TmpTreesS;
					}
				}				

				outPoBand->RasterIO(GF_Write, 0, 0, numcol, numrow, pintScanline, numcol, numrow, GDT_UInt32, 0, 0);//*

				if (poDstDS != NULL)
					GDALClose((GDALDatasetH)poDstDS);

			}

		}



		//4.IV
		if (sites.GetOutputGeneralFlagArray(k - 1, IV))
		{
			if (strcmp(IVFileNames[k - 1].c_str(), "N/A") != 0)
			{
				char str1[100];
				sprintf(str1, "%s/%s_%d.img", parameters.outputDir, IVFileNames[k - 1].c_str(), itr_m_timestep);//* change

				poDstDS = poDriver->Create(str1, numcol, numrow, 1, GDT_Float32, papszOptions);

				if (poDstDS == NULL)
					errorSys("Img file not be created.", STOP);

				poDstDS->SetGeoTransform(wAdfGeoTransform);

				outPoBand = poDstDS->GetRasterBand(1);

				for (int i = numrow; i > 0; i--)
				{
					const int part_index = (numrow - i) * numcol - 1;

					for (int j = 1; j <= numcol; j++)
					{
						LANDUNIT *l = sites.locateLanduPt(i, j);
						double TmpBasalAreaT = 0;
						int TmpTreeT = 0;

						for (int kk = 1; kk <= sites.specNum; kk++)
						{
							SPECIE* specie_k = sites(i, j)->SpecieIndex(kk);

							if (speciesAttrs(kk)->SpType >= 0)
							{
								const int m_kk_end = speciesAttrs(kk)->longevity / sites.TimeStep;

								for (int m = 1; m <= m_kk_end; m++)
								{
									const float lc_growrate = sites.GetGrowthRates(kk, m, l->ltID);
									const int 	lc_tree_num = specie_k->getTreeNum(m, kk);

									TmpBasalAreaT += lc_growrate * lc_growrate * local_const * lc_tree_num;

									TmpTreeT += lc_tree_num;
								}
							}
						}


						SPECIE* specie_k = sites(i, j)->SpecieIndex(k);

						double TmpBasalAreaS = 0;
						int TmpTreesS = 0;

						for (int m = 1; m <= m_end; m++)
						{
							const float lc_growrate = sites.GetGrowthRates(k, m, l->ltID);
							const int 	lc_tree_num = specie_k->getTreeNum(m, k);

							TmpBasalAreaS += lc_growrate * lc_growrate * local_const * lc_tree_num;

							TmpTreesS += lc_tree_num;
						}


						if (TmpTreeT == 0 || TmpBasalAreaT < 0.0001)
							*(pafScanline + part_index + j) = 0;
						else
							*(pafScanline + part_index + j) = (double)TmpTreesS / TmpTreeT + TmpBasalAreaS / TmpBasalAreaT;
					}

				}

				outPoBand->RasterIO(GF_Write, 0, 0, numcol, numrow, pafScanline, numcol, numrow, GDT_Float32, 0, 0);//*

				if (poDstDS != NULL)
					GDALClose((GDALDatasetH)poDstDS);
			}

		}




		//5. Seeds
		if (sites.GetOutputGeneralFlagArray(k - 1, Seeds))
		{
			if (strcmp(SeedsFileNames[k - 1].c_str(), "N/A") != 0)
			{
				char str1[100];
				sprintf(str1, "%s/%s_%d.img", parameters.outputDir, SeedsFileNames[k - 1].c_str(), itr_m_timestep);//* change

				poDstDS = poDriver->Create(str1, numcol, numrow, 1, GDT_UInt32, papszOptions);

				if (poDstDS == NULL)
					errorSys("Img file not be created.", STOP);

				outPoBand = poDstDS->GetRasterBand(1);

				poDstDS->SetGeoTransform(wAdfGeoTransform);

				for (int i = numrow; i > 0; i--)
				{
					//printf("\r%3d%%",100*i/numrow);

					for (int j = 1; j <= numcol; j++)
					{
						*(pintScanline + (numrow - i)*numcol + j - 1) = sites(i, j)->SpecieIndex(k)->AvailableSeed;
					}

				}

				outPoBand->RasterIO(GF_Write, 0, 0, numcol, numrow, pintScanline, numcol, numrow, GDT_UInt32, 0, 0);//*

				if (poDstDS != NULL)
					GDALClose((GDALDatasetH)poDstDS);
			}

		}



		//6.RDensity
		if (sites.GetOutputGeneralFlagArray(k - 1, RDensity))//sites.GetOutputGeneralFlagArray(k-1,DBH)&&
		{
			if (strcmp(RDFileNames[k - 1].c_str(), "N/A") != 0)
			{
				char str1[100];
				sprintf(str1, "%s/%s_%d.img", parameters.outputDir, RDFileNames[k - 1].c_str(), itr_m_timestep);//* change

				poDstDS = poDriver->Create(str1, numcol, numrow, 1, GDT_Float32, papszOptions);

				if (poDstDS == NULL)
					errorSys("Img file not be created.", STOP);

				outPoBand = poDstDS->GetRasterBand(1);

				poDstDS->SetGeoTransform(wAdfGeoTransform);

				for (int i = numrow; i > 0; i--)
				{
					const int part_index = (numrow - i) * numcol - 1;

					for (int j = 1; j <= numcol; j++)
					{
						LANDUNIT *l = sites.locateLanduPt(i, j);
						SPECIE* specie_k = sites(i, j)->SpecieIndex(k);
						SPECIESATTR* specAtt_k = sites(i, j)->specAtt(k);

						if (specAtt_k->SpType >= 0)
						{
							float temp = 0.0;
							const int jj_end = specAtt_k->longevity / sites.TimeStep;
							const float lc_maxarea = specAtt_k->MaxAreaOfSTDTree;

							for (int jj = 1; jj <= jj_end; jj++)
							{
								const float lc_growrate = sites.GetGrowthRates(k, jj, l->ltID);
								const int 	lc_tree_num = specie_k->getTreeNum(jj, k);

								temp += pow((lc_growrate / 25.4), 1.605) * lc_maxarea * lc_tree_num / cellsize_square;
							}

							*(pafScanline + part_index + j) = temp;
						}
						else
						{
							*(pafScanline + part_index + j) = 0.0;
						}

					}

				}

				outPoBand->RasterIO(GF_Write, 0, 0, numcol, numrow, pafScanline, numcol, numrow, GDT_Float32, 0, 0);

				if (poDstDS != NULL)
					GDALClose((GDALDatasetH)poDstDS);
			}

		}




		//7.DBH
		if (flagoutputDBH && 0)
		{
			if (strcmp(DBHFileNames[k - 1].c_str(), "N/A") != 0)
			{
				if (itr * freq[0] <= parameters.numIter)
				{
					char str1[100];
					sprintf(str1, "%s/%s_%d.img", parameters.outputDir, DBHFileNames[k - 1].c_str(), itr_m_timestep);

					poDstDS = poDriver->Create(str1, numcol, numrow, 1, GDT_Float32, papszOptions);

					if (poDstDS == NULL)
						errorSys("Img file not be created.", STOP);

					outPoBand = poDstDS->GetRasterBand(1);

					poDstDS->SetGeoTransform(wAdfGeoTransform);

					for (int i = numrow; i > 0; i--)
					{
						const int part_index = (numrow - i) * numcol - 1;

						for (int j = 1; j <= numcol; j++)
						{
							LANDUNIT *l = sites.locateLanduPt(i, j);
							SPECIE* specie_k = sites(i, j)->SpecieIndex(k);

							float tempDBH = 0.0f;

							for (int m = 1; m <= m_end; m++)
							{
								if (specie_k->getTreeNum(m, k) > 0)
								{
									tempDBH = sites.GetGrowthRates(k, m, l->ltID);
									break;
								}
							}

							*(pafScanline + part_index + j) = tempDBH;
						}

					}

					outPoBand->RasterIO(GF_Write, 0, 0, numcol, numrow, pafScanline, numcol, numrow, GDT_Float32, 0, 0);//*

					if (poDstDS != NULL)
						GDALClose((GDALDatasetH)poDstDS);
				}

			}

		}

	}



	const int Bio_flag = sites.GetOutputGeneralFlagArray(sites.specNum, Bio);
	const int Car_flag = sites.GetOutputGeneralFlagArray(sites.specNum, Car);
	const int  BA_flag = sites.GetOutputGeneralFlagArray(sites.specNum, BA);
	const int RDenflag = sites.GetOutputGeneralFlagArray(sites.specNum, RDensity);
	const int TPA_flag = sites.GetOutputGeneralFlagArray(sites.specNum, TPA);

	if (Bio_flag || Car_flag || BA_flag || RDenflag || TPA_flag)
	{
		pafScanline1 = (float *)CPLMalloc(sizeof(float)* total_size);
		pafScanline2 = (float *)CPLMalloc(sizeof(float)* total_size);
		pafScanline3 = (float *)CPLMalloc(sizeof(float)* total_size);
		pafScanline4 = (float *)CPLMalloc(sizeof(float)* total_size);
		pafScanline5 = (float *)CPLMalloc(sizeof(float)* total_size);

		//1.Bio
		if (Bio_flag)
		{
			char str[100];
			sprintf(str, "%s/TotalBio_%d.img", parameters.outputDir, itr_m_timestep);

			poDstDS1 = poDriver->Create(str, numcol, numrow, 1, GDT_Float32, papszOptions);

			if (poDstDS1 == NULL)
				errorSys("Img file not be created.", STOP);

			outPoBand1 = poDstDS1->GetRasterBand(1);

			poDstDS1->SetGeoTransform(wAdfGeoTransform);
		}


		//2.Car
		if (Car_flag)
		{
			char str[100];
			sprintf(str, "%s/TotalCarbon_%d.img", parameters.outputDir, itr_m_timestep);

			poDstDS2 = poDriver->Create(str, numcol, numrow, 1, GDT_Float32, papszOptions);

			if (poDstDS2 == NULL)
				errorSys("Img file not be created.", STOP);

			outPoBand2 = poDstDS2->GetRasterBand(1);

			poDstDS2->SetGeoTransform(wAdfGeoTransform);
		}


		//3.BA
		if (BA_flag)
		{
			char str[100];
			sprintf(str, "%s/TotalBA_%d.img", parameters.outputDir, itr_m_timestep);

			poDstDS3 = poDriver->Create(str, numcol, numrow, 1, GDT_Float32, papszOptions);

			if (poDstDS3 == NULL)
				errorSys("Img file not be created.", STOP);

			outPoBand3 = poDstDS3->GetRasterBand(1);

			poDstDS3->SetGeoTransform(wAdfGeoTransform);
		}


		//4.TPA
		if (TPA_flag)
		{
			char str[100];
			sprintf(str, "%s/TotalTrees_%d.img", parameters.outputDir, itr_m_timestep);

			poDstDS4 = poDriver->Create(str, numcol, numrow, 1, GDT_UInt32, papszOptions);

			if (poDstDS4 == NULL)
				errorSys("Img file not be created.", STOP);

			outPoBand4 = poDstDS4->GetRasterBand(1);

			poDstDS4->SetGeoTransform(wAdfGeoTransform);
		}


		//5.RDensity
		if (RDenflag)
		{
			char str[100];
			sprintf(str, "%s/RelativeDensity_%d.img", parameters.outputDir, itr_m_timestep);

			poDstDS5 = poDriver->Create(str, numcol, numrow, 1, GDT_Float32, papszOptions);

			if (poDstDS5 == NULL)
				errorSys("Img file not be created.", STOP);

			outPoBand5 = poDstDS5->GetRasterBand(1);

			poDstDS5->SetGeoTransform(wAdfGeoTransform);
		}



		for (int i = numrow; i > 0; i--)
		{
			const int part_index = (numrow - i) * numcol - 1;

			for (int j = 1; j <= numcol; j++)
			{
				LANDUNIT *l = sites.locateLanduPt(i, j);
				
				double TmpBiomassT = 0;
				double TmpBasalAreaT = 0;
				double TmpCarbonTotal = 0;

				int TmpTreeT = 0;

				float temp;

				for (int k = 1; k <= sites.specNum; k++)
				{
					double TmpCarbon = 0;

					const int m_end = speciesAttrs(k)->longevity / sites.TimeStep;

					if (speciesAttrs(k)->SpType >= 0)
					{
						SPECIE* specie_k = sites(i, j)->SpecieIndex(k);

						const float biomass1 = sites.GetBiomassData(speciesAttrs(k)->BioMassCoef, 1);
						const float biomass2 = sites.GetBiomassData(speciesAttrs(k)->BioMassCoef, 2);

						for (int m = 1; m <= m_end; m++)
						{
							const float lc_growrate = sites.GetGrowthRates(k, m, l->ltID);
							const double log_lc_growrate = log(lc_growrate);
							const int lc_tree_num = specie_k->getTreeNum(m, k);

							if (lc_growrate >= biomass_threshold && Bio_flag)
								TmpBiomassT += exp(biomass1 + biomass2 * log_lc_growrate) * lc_tree_num / 1000.00;

							if (BA_flag)
								TmpBasalAreaT += lc_growrate * lc_growrate * local_const * lc_tree_num;

							if (TPA_flag)
								TmpTreeT += lc_tree_num;

							if (Car_flag)
							{
								if (lc_growrate == 0)
									temp = 0;
								else
									temp = log_lc_growrate;

								TmpCarbon += exp(biomass1 + biomass2 * temp) * lc_tree_num;
							}
						}

					}

					if (Car_flag)
						TmpCarbonTotal += TmpCarbon * speciesAttrs(k)->CarbonCoEfficient;
				}


				//1.Bio
				if (Bio_flag)
					*(pafScanline1 + part_index + j) = TmpBiomassT;

				//2.Car
				if (Car_flag)
					*(pafScanline2 + part_index + j) = TmpCarbonTotal / 1000.00;

				//3.BA
				if (BA_flag)
					*(pafScanline3 + part_index + j) = TmpBasalAreaT;

				//5.RDensity
				if (RDenflag)
					*(pafScanline5 + part_index + j) = sites(i, j)->RD;

				//4.TPA
				if (TPA_flag)
					*(pintScanline + part_index + j) = TmpTreeT;
			}

		}


		//1.Bio
		if (Bio_flag)
		{
			outPoBand1->RasterIO(GF_Write, 0, 0, numcol, numrow, pafScanline1, numcol, numrow, GDT_Float32, 0, 0);

			if (poDstDS1 != NULL)
				GDALClose((GDALDatasetH)poDstDS1);
		}



		//2.Car
		if (Car_flag)
		{
			outPoBand2->RasterIO(GF_Write, 0, 0, numcol, numrow, pafScanline2, numcol, numrow, GDT_Float32, 0, 0);

			if (poDstDS2 != NULL)
				GDALClose((GDALDatasetH)poDstDS2);
		}


		//3.BA
		if (BA_flag)
		{
			outPoBand3->RasterIO(GF_Write, 0, 0, numcol, numrow, pafScanline3, numcol, numrow, GDT_Float32, 0, 0);

			if (poDstDS3 != NULL)
				GDALClose((GDALDatasetH)poDstDS3);
		}


		//4.TPA
		if (TPA_flag)
		{
			outPoBand4->RasterIO(GF_Write, 0, 0, numcol, numrow, pintScanline, numcol, numrow, GDT_UInt32, 0, 0);

			if (poDstDS4 != NULL)
				GDALClose((GDALDatasetH)poDstDS4);
		}


		//5.RDensity
		if (RDenflag)
		{
			outPoBand5->RasterIO(GF_Write, 0, 0, numcol, numrow, pafScanline5, numcol, numrow, GDT_Float32, 0, 0);

			if (poDstDS5 != NULL)
				GDALClose((GDALDatasetH)poDstDS5);
		}
	}


	//below is for species age range
	int Agerangeage1, Agerangeage2, Agerangecount = 0, Agerangeii;

	//1.Bio
	if (sites.flagAgeRangeOutput)
	{
		for (int k = 1; k <= sites.specNum; k++)
		{
			const int specie_k_long = speciesAttrs(k)->longevity;
			const int m_end = specie_k_long / sites.TimeStep;


			if (sites.GetOutputAgerangeFlagArray(k - 1, Bio))
			{
				const float biomass1 = sites.GetBiomassData(speciesAttrs(k)->BioMassCoef, 1);
				const float biomass2 = sites.GetBiomassData(speciesAttrs(k)->BioMassCoef, 2);

				Agerangecount = sites.GetAgerangeCount(k - 1);

				for (Agerangeii = 1; Agerangeii <= Agerangecount; Agerangeii++)
				{
					sites.GetSpeciesAgerangeArray(k - 1, Agerangeii, Agerangeage1, Agerangeage2);

					const int m_beg = min(specie_k_long, Agerangeage1) / sites.TimeStep;
					const int m_end = min(specie_k_long, Agerangeage2) / sites.TimeStep;

					if (strcmp(BioMassFileNames[k - 1].c_str(), "N/A") != 0)
					{
						char str1[100];
						sprintf(str1, "%s/%s_Age%d_Age%d_%d.img", parameters.outputDir, BioMassFileNames[k - 1].c_str(), Agerangeage1, Agerangeage2, itr_m_timestep);//* change

						poDstDS = poDriver->Create(str1, numcol, numrow, 1, GDT_Float32, papszOptions);

						if (poDstDS == NULL)
							errorSys("Img file not be created.", STOP);

						outPoBand = poDstDS->GetRasterBand(1);

						poDstDS->SetGeoTransform(wAdfGeoTransform);

						for (int i = numrow; i > 0; i--)
						{
							const int part_index = (numrow - i) * numcol - 1;

							for (int j = 1; j <= numcol; j++)
							{
								LANDUNIT* l = sites.locateLanduPt(i, j);
								double TmpBiomassS = 0;

								SPECIE* specie_k = sites(i, j)->SpecieIndex(k);

								for (int m = m_beg; m <= m_end; m++)
								{
									const float lc_growrate = sites.GetGrowthRates(k, m, l->ltID);

									if (lc_growrate >= biomass_threshold)
										TmpBiomassS += exp(biomass1 + biomass2 * log(lc_growrate)) * specie_k->getTreeNum(m, k) / 1000.00;
								}

								*(pafScanline + part_index + j) = TmpBiomassS;
							}

						}

						outPoBand->RasterIO(GF_Write, 0, 0, numcol, numrow, pafScanline, numcol, numrow, GDT_Float32, 0, 0);//*

						if (poDstDS != NULL)
							GDALClose((GDALDatasetH)poDstDS);
					}

				}

			}



			//2.BA
			if (sites.GetOutputAgerangeFlagArray(k - 1, BA))
			{
				Agerangecount = sites.GetAgerangeCount(k - 1);

				for (Agerangeii = 1; Agerangeii <= Agerangecount; Agerangeii++)
				{
					sites.GetSpeciesAgerangeArray(k - 1, Agerangeii, Agerangeage1, Agerangeage2);

					const int m_beg = min(specie_k_long, Agerangeage1) / sites.TimeStep;
					const int m_end = min(specie_k_long, Agerangeage2) / sites.TimeStep;

					if (strcmp(BasalFileNames[k - 1].c_str(), "N/A") != 0)
					{
						char str1[100];
						sprintf(str1, "%s/%s_Age%d_Age%d_%d.img", parameters.outputDir, BasalFileNames[k - 1].c_str(), Agerangeage1, Agerangeage2, itr_m_timestep);//* change

						poDstDS = poDriver->Create(str1, numcol, numrow, 1, GDT_Float32, papszOptions);

						if (poDstDS == NULL)
							errorSys("Img file not be created.", STOP);

						outPoBand = poDstDS->GetRasterBand(1);

						poDstDS->SetGeoTransform(wAdfGeoTransform);

						for (int i = numrow; i > 0; i--)
						{
							const int part_index = (numrow - i) * numcol - 1;

							for (int j = 1; j <= numcol; j++)
							{
								LANDUNIT *l = sites.locateLanduPt(i, j);
								double TmpBasalAreaS = 0;

								SPECIE* specie_k = sites(i, j)->SpecieIndex(k);

								for (int m = m_beg; m <= m_end; m++)
								{
									const float lc_growrate = sites.GetGrowthRates(k, m, l->ltID);

									TmpBasalAreaS += lc_growrate * lc_growrate * local_const * specie_k->getTreeNum(m, k);
								}

								*(pafScanline + part_index + j) = TmpBasalAreaS;
							}
						}

						outPoBand->RasterIO(GF_Write, 0, 0, numcol, numrow, pafScanline, numcol, numrow, GDT_Float32, 0, 0);//*

						if (poDstDS != NULL)
							GDALClose((GDALDatasetH)poDstDS);

					}

				}

			}



			//3.TPA
			if (sites.GetOutputAgerangeFlagArray(k - 1, TPA))
			{
				Agerangecount = sites.GetAgerangeCount(k - 1);

				for (Agerangeii = 1; Agerangeii <= Agerangecount; Agerangeii++)
				{
					sites.GetSpeciesAgerangeArray(k - 1, Agerangeii, Agerangeage1, Agerangeage2);

					const int m_beg = min(specie_k_long, Agerangeage1) / sites.TimeStep;
					const int m_end = min(specie_k_long, Agerangeage2) / sites.TimeStep;

					if (strcmp(TreesFileNames[k - 1].c_str(), "N/A") != 0)
					{
						char str1[100];
						sprintf(str1, "%s/%s_Age%d_Age%d_%d.img", parameters.outputDir, TreesFileNames[k - 1].c_str(), Agerangeage1, Agerangeage2, itr_m_timestep);//* change

						poDstDS = poDriver->Create(str1, numcol, numrow, 1, GDT_UInt32, papszOptions);

						if (poDstDS == NULL)
							errorSys("Img file not be created.", STOP);

						outPoBand = poDstDS->GetRasterBand(1);

						poDstDS->SetGeoTransform(wAdfGeoTransform);

						for (int i = numrow; i > 0; i--)
						{
							const int part_index = (numrow - i) * numcol - 1;

							for (int j = 1; j <= numcol; j++)
							{
								int TmpTreesS = 0;

								SPECIE* specie_k = sites(i, j)->SpecieIndex(k);

								for (int m = m_beg; m <= m_end; m++)
									TmpTreesS += specie_k->getTreeNum(m, k);

								*(pintScanline + part_index + j) = TmpTreesS;
							}
						}

						outPoBand->RasterIO(GF_Write, 0, 0, numcol, numrow, pintScanline, numcol, numrow, GDT_UInt32, 0, 0);//*

						if (poDstDS != NULL)
							GDALClose((GDALDatasetH)poDstDS);
					}

				}

			}




			//4.IV
			if (sites.GetOutputAgerangeFlagArray(k - 1, IV))
			{
				Agerangecount = sites.GetAgerangeCount(k - 1);

				for (Agerangeii = 1; Agerangeii <= Agerangecount; Agerangeii++)
				{
					sites.GetSpeciesAgerangeArray(k - 1, Agerangeii, Agerangeage1, Agerangeage2);

					if (strcmp(IVFileNames[k - 1].c_str(), "N/A") != 0)
					{
						char str1[100];
						sprintf(str1, "%s/%s_Age%d_Age%d_%d.img", parameters.outputDir, IVFileNames[k - 1].c_str(), Agerangeage1, Agerangeage2, itr_m_timestep);//* change

						poDstDS = poDriver->Create(str1, numcol, numrow, 1, GDT_Float32, papszOptions);

						if (poDstDS == NULL)
							errorSys("Img file not be created.", STOP);

						outPoBand = poDstDS->GetRasterBand(1);

						poDstDS->SetGeoTransform(wAdfGeoTransform);

						for (int i = numrow; i > 0; i--)
						{
							const int part_index = (numrow - i) * numcol - 1;

							for (int j = 1; j <= numcol; j++)
							{
								LANDUNIT *l = sites.locateLanduPt(i, j);
								double TmpBasalAreaT = 0;

								int TmpTreeT = 0;

								for (int kk = 1; kk <= sites.specNum; kk++)
								{
									if (speciesAttrs(kk)->SpType >= 0)
									{
										const int m_kk_end = speciesAttrs(kk)->longevity / sites.TimeStep;
										SPECIE* specie_kk = sites(i, j)->SpecieIndex(kk);

										for (int m = 1; m <= m_kk_end; m++)
										{
											const float lc_growrate = sites.GetGrowthRates(kk, m, l->ltID);
											const int lc_tree_num = specie_kk->getTreeNum(m, kk);

											TmpBasalAreaT += lc_growrate * lc_growrate * local_const * lc_tree_num;
											TmpTreeT += lc_tree_num;
										}
									}
								}

								double TmpBasalAreaS = 0;
								int TmpTreesS = 0;

								SPECIE* specie_k = sites(i, j)->SpecieIndex(k);

								for (int m = 1; m <= m_end; m++)
								{
									const float lc_growrate = sites.GetGrowthRates(k, m, l->ltID);
									const int lc_tree_num = specie_k->getTreeNum(m, k);

									TmpBasalAreaS += lc_growrate * lc_growrate * local_const * lc_tree_num;
									TmpTreesS += lc_tree_num;
								}

								if (TmpTreeT == 0 || TmpBasalAreaT < 0.0001)
									*(pafScanline + part_index + j) = 0;
								else
									*(pafScanline + part_index + j) = (double)TmpTreesS / TmpTreeT + TmpBasalAreaS / TmpBasalAreaT;
							}
						}

						outPoBand->RasterIO(GF_Write, 0, 0, numcol, numrow, pafScanline, numcol, numrow, GDT_Float32, 0, 0);//*

						if (poDstDS != NULL)
							GDALClose((GDALDatasetH)poDstDS);
					}
				}
			}
		}


		//below is for age range  total 
		//1.Bio
		if (sites.GetOutputAgerangeFlagArray(sites.specNum, Bio))
		{
			char str[100];
			sprintf(str, "%s/TotalBio_AgeRange_%d.img", parameters.outputDir, itr_m_timestep);//* change

			poDstDS1 = poDriver->Create(str, numcol, numrow, 1, GDT_Float32, papszOptions);//*

			if (poDstDS1 == NULL)
				errorSys("Img file not be created.", STOP);//*

			outPoBand1 = poDstDS1->GetRasterBand(1);//*

			poDstDS1->SetGeoTransform(wAdfGeoTransform);//*

		}

		//2.TPA
		if (sites.GetOutputAgerangeFlagArray(sites.specNum, TPA))
		{
			char str[100];
			sprintf(str, "%s/TotalTrees_AgeRange_%d.img", parameters.outputDir, itr_m_timestep);//* change

			poDstDS2 = poDriver->Create(str, numcol, numrow, 1, GDT_UInt32, papszOptions);//*

			if (poDstDS2 == NULL)
				errorSys("Img file not be created.", STOP);//*

			outPoBand2 = poDstDS2->GetRasterBand(1);//*

			poDstDS2->SetGeoTransform(wAdfGeoTransform);//*


		}


		//3.BA
		if (sites.GetOutputAgerangeFlagArray(sites.specNum, BA))
		{
			char str[100];
			sprintf(str, "%s/TotalBasal_AgeRange_%d.img", parameters.outputDir, itr_m_timestep);//* change

			poDstDS3 = poDriver->Create(str, numcol, numrow, 1, GDT_Float32, papszOptions);

			if (poDstDS3 == NULL)
				errorSys("Img file not be created.", STOP);

			outPoBand3 = poDstDS3->GetRasterBand(1);

			poDstDS3->SetGeoTransform(wAdfGeoTransform);
		}


		//4.Car
		if (sites.GetOutputAgerangeFlagArray(sites.specNum, Car))
		{
			char str[100];
			sprintf(str, "%s/TotalCarbon_AgeRange_%d.img", parameters.outputDir, itr_m_timestep);//* change

			poDstDS4 = poDriver->Create(str, numcol, numrow, 1, GDT_Float32, papszOptions);//*

			if (poDstDS4 == NULL)
				errorSys("Img file not be created.", STOP);//*

			outPoBand4 = poDstDS4->GetRasterBand(1);//*

			poDstDS4->SetGeoTransform(wAdfGeoTransform);//*

		}

		//5.RDensity
		if (sites.GetOutputAgerangeFlagArray(sites.specNum, RDensity))
		{
			char str[100];
			sprintf(str, "%s/TotalRD_AgeRange_%d.img", parameters.outputDir, itr_m_timestep);//* change

			poDstDS5 = poDriver->Create(str, numcol, numrow, 1, GDT_Float32, papszOptions);

			if (poDstDS5 == NULL)
				errorSys("Img file not be created.", STOP);

			outPoBand5 = poDstDS5->GetRasterBand(1);

			poDstDS5->SetGeoTransform(wAdfGeoTransform);

		}



		for (int i = numrow; i > 0; i--)
		{
			const int part_index = (numrow - i) * numcol - 1;


			for (int j = 1; j <= numcol; j++)
			{
				LANDUNIT *l = sites.locateLanduPt(i, j);
				double TmpBiomassT = 0;

				double TmpBasalAreaT = 0;

				int TmpTreeT = 0;

				double TmpCarbonTotal = 0;

				double TmpRDTotal = 0;

				for (int k = 1; k <= sites.specNum; k++)
				{
					double TmpCarbon = 0;

					const int specie_k_long = speciesAttrs(k)->longevity;
					const int specie_sptype = speciesAttrs(k)->SpType;

					const float specie_carbon_coeff = speciesAttrs(k)->CarbonCoEfficient;

					SPECIE* specie_k = sites(i, j)->SpecieIndex(k);
					const float lc_maxarea = sites(i, j)->specAtt(k)->MaxAreaOfSTDTree;

					const float biomass1 = sites.GetBiomassData(speciesAttrs(k)->BioMassCoef, 1);
					const float biomass2 = sites.GetBiomassData(speciesAttrs(k)->BioMassCoef, 2);

					Agerangecount = sites.GetAgerangeCount(k - 1);

					for (Agerangeii = 1; Agerangeii <= Agerangecount; Agerangeii++)
					{
						sites.GetSpeciesAgerangeArray(k - 1, Agerangeii, Agerangeage1, Agerangeage2);

						const int m_beg = min(specie_k_long, Agerangeage1) / sites.TimeStep;
						const int m_end = min(specie_k_long, Agerangeage2) / sites.TimeStep;

						if (specie_sptype >= 0)
						{
							for (int m = m_beg; m <= m_end; m++)
							{
								const float lc_growrate = sites.GetGrowthRates(k, m, l->ltID);
								const double log_lc_growrate = log(lc_growrate);
								const int lc_tree_num = specie_k->getTreeNum(m, k);
								const double consterm = exp(biomass1 + biomass2 * log_lc_growrate) * lc_tree_num;

								if (lc_growrate >= biomass_threshold)
									TmpBiomassT += consterm / 1000.00;

								TmpBasalAreaT += lc_growrate * lc_growrate * local_const * lc_tree_num;

								TmpTreeT += lc_tree_num;

								TmpCarbon += consterm;

								TmpRDTotal += pow((lc_growrate / 25.4), 1.605) * lc_maxarea * lc_tree_num / cellsize_square;
							}
						}

						TmpCarbonTotal += TmpCarbon * specie_carbon_coeff;
					}
				}


				//1.Bio
				if (sites.GetOutputAgerangeFlagArray(sites.specNum, Bio))
					*(pafScanline1 + part_index + j) = TmpBiomassT;

				//4.Car
				if (sites.GetOutputAgerangeFlagArray(sites.specNum, Car))
					*(pafScanline4 + part_index + j) = TmpCarbonTotal / 1000.00;

				//3.BA
				if (sites.GetOutputAgerangeFlagArray(sites.specNum, BA))
					*(pafScanline3 + part_index + j) = TmpBasalAreaT;

				//5.RDensity
				if (sites.GetOutputAgerangeFlagArray(sites.specNum, RDensity))
					*(pafScanline5 + part_index + j) = TmpRDTotal;

				//2.TPA
				if (sites.GetOutputAgerangeFlagArray(sites.specNum, TPA))
					*(pintScanline + part_index + j) = TmpTreeT;
			}

		}


		//1.Bio
		if (sites.GetOutputAgerangeFlagArray(sites.specNum, Bio))
		{
			outPoBand1->RasterIO(GF_Write, 0, 0, numcol, numrow, pafScanline1, numcol, numrow, GDT_Float32, 0, 0);//*

			if (poDstDS1 != NULL)
				GDALClose((GDALDatasetH)poDstDS1);
		}


		//4.Car
		if (sites.GetOutputAgerangeFlagArray(sites.specNum, Car))
		{
			outPoBand4->RasterIO(GF_Write, 0, 0, numcol, numrow, pafScanline4, numcol, numrow, GDT_Float32, 0, 0);//*

			if (poDstDS4 != NULL)
				GDALClose((GDALDatasetH)poDstDS4);
		}


		//3.BA
		if (sites.GetOutputAgerangeFlagArray(sites.specNum, BA))
		{
			outPoBand3->RasterIO(GF_Write, 0, 0, numcol, numrow, pafScanline3, numcol, numrow, GDT_Float32, 0, 0);//*

			if (poDstDS3 != NULL)
				GDALClose((GDALDatasetH)poDstDS3);
		}


		//2.TPA
		if (sites.GetOutputAgerangeFlagArray(sites.specNum, TPA))
		{
			outPoBand2->RasterIO(GF_Write, 0, 0, numcol, numrow, pintScanline, numcol, numrow, GDT_UInt32, 0, 0);//*

			if (poDstDS2 != NULL)
				GDALClose((GDALDatasetH)poDstDS2);
		}


		//5.RDensity
		if (sites.GetOutputAgerangeFlagArray(sites.specNum, RDensity))
		{
			outPoBand5->RasterIO(GF_Write, 0, 0, numcol, numrow, pafScanline5, numcol, numrow, GDT_Float32, 0, 0);//*

			if (poDstDS5 != NULL)
				GDALClose((GDALDatasetH)poDstDS5);
		}
	}

	time(&ltimeTemp);

	CPLFree(pafScanline1);
	CPLFree(pafScanline2);
	CPLFree(pafScanline3);
	CPLFree(pafScanline4);
	CPLFree(pafScanline5);
	CPLFree(pintScanline);
	CPLFree(pafScanline);

	printf("Finish 7.0 Style writing output at %s\n", ctime(&ltimeTemp));
	printf("it took %ld seconds\n", (long)ltimeTemp - ltime);

	CSLDestroy(papszOptions);
}







//This will write out a Landis description for this iteration and replicate.
void putOutput(int rep, int itr, int freq[], double wAdfGeoTransform[])
{
	time_t ltime, ltimeTemp;

	time(&ltime);

	printf("Start 6.0 Style writing output at %s", ctime(&ltime));
	
	MAP8 m(sites.getHeader());

	const int itr_m_timestep = itr * sites.TimeStep;

	char str[100];

	for (int i = 0; i < 0; i++) //Nim: changed int i to i
	{
		char* pdest = strrchr(reMethods[i], '\\');

		char str1[100], strMsg[256];

		if (pdest)
		{
			pdest++;

			strcpy(str1, pdest);

			sprintf(strMsg, "reclass %s\n", pdest);
		}
		else
		{
			strcpy(str1, reMethods[i]);

			sprintf(strMsg, "reclass %s\n", reMethods[i]);
		}

		printMessage(strMsg);

		sprintf(str, "%s.rcs", reMethods[i]);

		fileReclass(m, str);

		sprintf(str, "%s/%s%d", parameters.outputDir, str1, itr);

		m.setCellSize(parameters.cellSize);

		m.write(str, red, green, blue, wAdfGeoTransform);

	}


	for (int i = 1; ((i <= sites.specNum) && (!rep)); i++) //Nim: changed int i to i
	{
		printf("creating %d %d %s %s\n", sites.specNum, rep, speciesAttrs(i)->name, "age map");

		speciesAgeMap(m, speciesAttrs(i)->name);

		sprintf(str, "%s/%s_%d", parameters.outputDir, speciesAttrs(i)->name, itr_m_timestep);

		m.setCellSize(parameters.cellSize);

		m.write(str, red2, green2, blue2, wAdfGeoTransform);
	}

	ageReclass(m);

	sprintf(str, "%s/ageOldest%d", parameters.outputDir, itr_m_timestep);

	m.setCellSize(parameters.cellSize);

	m.write(str, red2, green2, blue2, wAdfGeoTransform);


	ageReclassYoungest(m);

	sprintf(str, "%s/ageYoungest%d", parameters.outputDir, itr_m_timestep);

	m.setCellSize(parameters.cellSize);

	m.write(str, red2, green2, blue2, wAdfGeoTransform);

	time(&ltimeTemp);

	printf("Finish 6.0 Style writing output at %s\n", ctime(&ltimeTemp));
	printf("it took %ld seconds\n", (long)ltimeTemp - ltime);
}







//This will read in the global variable sites from class file and simgFileint file.
//Class file is a file containing site descriptions for a set of class values.
//Map file contains the corresponding  img map.  Thus every value in mapFile
//is represented by the cooresponding class descritption represented in classFile. 
void inputImgSpec(FILE* classFile, GDALDataset* simgFileint, int yDim, int xDim)
{
	GDALRasterBand  *poBand = simgFileint->GetRasterBand(1);

	float* pafScanline = (float *)CPLMalloc(sizeof(float)* (xDim * yDim));

	poBand->RasterIO(GF_Read, 0, 0, xDim, yDim, pafScanline, xDim, yDim, GDT_Float32, 0, 0);


	SITE temp;
	int  numCovers = 0;
	
	while (!LDeof(classFile))
	{
		temp.read(classFile);

		numCovers++;
	}

	fclose(classFile);


	if ((classFile = LDfopen(parameters.reclassInFile, 1)) == NULL) //This one has age list
		errorSys("Map attribute input file not found.", STOP);


	SITE* s = new SITE[numCovers];

	for (int i = 0; i < numCovers; i++)
		s[i].read(classFile);

	int* combineMatrix = NULL;

	if (sites.Pro0or401 == 0)
	{
		for (int i = 0; i < numCovers; i++)
		{
			SITE* site = new SITE;

			*site = s[i];

			sites.SortedIndex.push_back(site);
		}

		combineMatrix = new int[numCovers];

		for (int i = 0; i < numCovers; i++)		
			combineMatrix[i] = i;

		lookupredundant(combineMatrix, numCovers);
	}

	printf("Reading species composition map in inputImgSpec1. number of class is %d\n", numCovers);

	for (int i = yDim; i > 0; i--)
	{
		const int const_int = (yDim - i) * xDim;

		for (int j = 1; j <= xDim; j++)
		{
			const int coverType = (int)*(pafScanline + const_int + j - 1);

			assert(coverType >= 0 && coverType < numCovers);

			if (sites.Pro0or401 == 0)
			{
				sites.fillinSitePt(i, j, sites.SortedIndex.at(combineMatrix[coverType]));

				sites.SortedIndex.at(combineMatrix[coverType])->numofsites++;
			}
			else
			{
				*(sites(i, j)) = s[coverType];
			}
		}
	}


	if (sites.Pro0or401 == 0)
	{
		printf("releasing redundant memory\n");

		deleteRedundantInitial(combineMatrix, numCovers);

		sites.SITE_sort();

		delete[] combineMatrix;
	}

	delete[] s;
}








void inputBin16(FILE* classFile, FILE* mapFile, int yDim, int xDim)

//This will read in the global variable sites from class file and map file.

//Class file is a file containing site descriptions for a set of class values.

//Map file contains the corresponding map.  Thus every value in mapFile

//is represented by the cooresponding class descritption represented in

//classFile.  mapFile is an Erdas 16 bit gis file.  The file pointer is

//placed on the first map element.  yDim and xDim are the (x,y) dimensions

//of the Erdas map.
{
	int coverType, numRead, numCovers = 0, i;

	unsigned short c;

	SITE *s;

	SITE *site; //Add By Qia on Oct 10 2008

	int *combineMatrix; //Add By Qia on Oct 22 2008

	printf("reading age cohort in inputBin16\n");

	s = new SITE[50000]; //number is temporary //

	while (!LDeof(classFile))
	{
		s[numCovers].read(classFile);

		numCovers++;
	}

 
	//build sites and copy sites to sorted array, this array will be sorted later.

	for (i = 0; i < numCovers; i++)
	{
		site = new SITE;

		*site = s[i];

		sites.SortedIndex.push_back(site);
	}
 
	combineMatrix = new int[numCovers];

	for (i = 0; i < numCovers; i++)
		combineMatrix[i] = i;

	lookupredundant(combineMatrix, numCovers);
 
	printf("Reading species composition map\n");

	for (i = yDim; i > 0; i--)
	{
		for (int j = 1; j <= xDim; j++)
		{
			numRead = fread((char*)(&c), 2, 1, mapFile);

			coverType = (int)c;

			if (numRead > 0)
			{
				if ((coverType) < numCovers && coverType > 0)
				{
					sites.fillinSitePt(i, j, sites.SortedIndex.at(combineMatrix[coverType]));

					sites.SortedIndex.at(combineMatrix[coverType])->numofsites++;
				}
				else
				{
					sites.fillinSitePt(i, j, sites.SortedIndex.at(0));

					sites.SortedIndex.at(combineMatrix[coverType])->numofsites++;
				}
			}
		}
	}

	printf("releasing redundant memory\n");

	deleteRedundantInitial(combineMatrix, numCovers);

	sites.SITE_sort();


	delete[] s;
	delete[] combineMatrix;
}







//This will read landtype img map and associate landUnit to each site.
//ltimgFile is an 32 bit img file.  
void inputLandtypeImg(GDALDataset* ltimgFile, int xDim, int yDim, PDP& ppdp)
{
	const int nCols = ltimgFile->GetRasterXSize();
	const int nRows = ltimgFile->GetRasterYSize();

	if ((nCols != xDim) && (nRows != yDim))
		errorSys("landtype map and species map do not match.", STOP);

	float* pafScanline = (float *)CPLMalloc(sizeof(float)* (xDim * yDim));

	GDALRasterBand* poBand = ltimgFile->GetRasterBand(1);

	poBand->RasterIO(GF_Read, 0, 0, xDim, yDim, pafScanline, xDim, yDim, GDT_Float32, 0, 0);
	
	for (int i = yDim; i > 0; i--)
	{
		const int const_int = (yDim - i) * xDim;

		for (int j = 1; j <= xDim; j++)
		{
			int coverType = (int)*(pafScanline + const_int + j - 1);

			assert(coverType >= 0);
			
			sites.fillinLanduPt(i, j, landUnits(coverType));

			if (gDLLMode & G_WIND)
				(ppdp.sTSLWind)[i][j] = (short)sites.locateLanduPt(i, j)->initialLastWind;				
		}
	}
}





//This will read in all LANDIS global variables.
void getInput(FILE *infile, int freq[], char ageMap[MAX_SPECIES][80], PDP& pdp, int BDANo, double wAdfGeoTransform[])
{
	GDALAllRegister();

	printMessage("Reading input.");

	if (_mkdir(parameters.outputDir) != 0)
	{
		if (errno != EEXIST)
			errorSys("BDAS: Can't create the direcory", STOP);
	}

	FILE *saFile, *luFile, *rcFile, *bioMass;
	GDALDataset  *simgFile, *ltimgFile;

	if ((saFile = LDfopen(parameters.specAttrFile, 1)) == NULL)
		errorSys("Species attribute file not found.", STOP);

	if ((luFile = LDfopen(parameters.landUnitFile, 1)) == NULL)
		errorSys("Landtype attribute file not found.", STOP);

	if ((ltimgFile = (GDALDataset *)GDALOpen(parameters.landImgMapFile, GA_ReadOnly)) == NULL) //* landtype.img
		errorSys("landtype img map input file not found.", STOP);

	if ((simgFile = (GDALDataset *)GDALOpen(parameters.siteImgFile, GA_ReadOnly)) == NULL)
		errorSys("species img map input file not found.", STOP);

	double adfGeoTransform[6];

	if (simgFile->GetGeoTransform(adfGeoTransform) == CE_None)
	{
		for (int i = 0; i < 6; i++)
			wAdfGeoTransform[i] = adfGeoTransform[i];
	}

	if ((rcFile = LDfopen(parameters.reclassInFile, 1)) == NULL) //This one has age list
		errorSys("Map attribute input file not found.", STOP);

	if ((bioMass = LDfopen(parameters.Biomassfile, 1)) == NULL)
		errorSys("BioMassfile not found.", STOP);

	speciesAttrs.read(saFile, parameters.cellSize);

	const int num_species = speciesAttrs.number();

	time_step.getSpecNum(num_species);//Add By Qia on July 5 2009

	for (int x = 1; x <= num_species; x++)
		time_step.Setlongevity(x, speciesAttrs(x)->longevity);

	if ((gDLLMode & G_HARVEST) != 0)
		numberOfSpecies = num_species;

	landUnits.attach(&speciesAttrs);

	landUnits.read(luFile);

	sites.BiomassRead(bioMass);

	SPECIES::attach(&speciesAttrs);

	const int nCols = simgFile->GetRasterXSize();
	const int nRows = simgFile->GetRasterYSize();

	sites.MaxDistofAllSpec = speciesAttrs.MaxDistanceofAllSpecs;

	sites.MaxShadeTolerance = speciesAttrs.MaxShadeTolerance;

	sites.dim(num_species, nRows, nCols);//need to change Qia Oct 06 2008

	sites.set_all_speces_Dist(speciesAttrs.get_speces_maxD(), speciesAttrs.get_specesefctD());

	sites.Read70OutputOption(parameters.OutputOption70);

	int biggest_agerange_count = 0;	//changed by houbin 10/25/2017

	for (int i = 0; i < sites.specNum; i++)
	{
		const int age_range_count = sites.GetOutputAgeRangeModuleFlagArray(i, COUNT);

		if (age_range_count > biggest_agerange_count)
			biggest_agerange_count = age_range_count;
	}

	pdp.set_parameters(gDLLMode, sites.numColumns(), sites.numRows(), BDANo, sites.specNum, biggest_agerange_count,parameters.TrackFlag);

	inputImgSpec(rcFile, simgFile, nRows, nCols);

	inputLandtypeImg(ltimgFile, nCols, nRows, pdp);

	if ((gDLLMode & G_HARVEST) != 0)
		HarvestPassInit(&sites, numberOfSpecies, parameters.outputDir, parameters.strHarvestInitName, &pdp);

	GDALClose((GDALDatasetH)simgFile);
	GDALClose((GDALDatasetH)ltimgFile);

	fclose(saFile);
	fclose(rcFile);
	fclose(bioMass);
	fclose(luFile);


	FILE *freqOfOutput;

	if ((freqOfOutput = LDfopen(parameters.freq_out_put, 1)) != NULL)
	{
		int c = 0;

		char temp[80] = {0};

		while (!LDeof(freqOfOutput))
		{
			fscanc(freqOfOutput, "%s", temp);

			freq[c] = atoi(temp);

			assert(freq[c] >= 0 && freq[c] <= parameters.numIter);

			c++;
		}

		printf("\n");

		if (freq[0] <= 1)
		{
			freq[0] = 1;

			printf("%s%i%s\n", "Species maps output every------->", freq[0] * sites.TimeStep, " years.");
			printf("%s%i%s\n", "Age maps output every----------->", freq[0] * sites.TimeStep, " years.");
		}
		else
		{
			printf("%s%i%s\n", "Species map outputs for year---->", freq[0] * sites.TimeStep, ".");
			printf("%s%i%s\n", "Age map outputs for year-------->", freq[0] * sites.TimeStep, ".");
		}

		if (freq[4] <= 1)
		{
			freq[4] = 1;

			printf("%s%i%s\n", "Age group maps output every----->", freq[4] * sites.TimeStep, " years.");
		}
		else
		{
			printf("%s%i%s\n", "Age group map outputs for year-->", freq[4] * sites.TimeStep, ".");
		}

		printf("\n");

		fclose(freqOfOutput);
	}
	else
	{
		freq[0] = 1; freq[1] = 1; freq[2] = 1; freq[3] = 1; freq[4] = 1;

		if ((gDLLMode & G_HARVEST) != 0)
			freq[5] = 1;

		printf("file <FREQ_OUT.PUT> not found\n");
		printf("creating file\n");
		printf("frequency of output unknown assuming every iteration\n");
		

		if ((freqOfOutput = LDfopen("freq_out.put", 3)) != NULL)
		{
			fprintf(freqOfOutput, "#This file establishes the number of years for output for reclass methods#\n");

			fprintf(freqOfOutput, "#fire, wind, timber, and age class. A one (1) in any field will produce maps#\n");

			fprintf(freqOfOutput, "#every iteration#\n");

			fprintf(freqOfOutput, "#output maps for reclass#  2\n");

			fprintf(freqOfOutput, "#output maps for fire# 1\n");

			fprintf(freqOfOutput, "#output maps for wind# 1\n");

			fprintf(freqOfOutput, "#output maps for timber# 1\n");

			fprintf(freqOfOutput, "#output maps for age class# 2\n");

			if ((gDLLMode & G_HARVEST) != 0)
				fprintf(freqOfOutput, "#output maps for harvest# 1\n");

			fclose(freqOfOutput);
		}
	}

	//Set age colors to a spectrum.
	if ((gDLLMode & G_WIND) != 0)
	{
		for (int i = 2; i < 16; i++)
			HSV_to_RGB((i - 2) / 14.0 * 360, 1.0, 1.0, red2[i], green2[i], blue2[i]);
	}

	//Write landtype map
	MAP8 m(sites.getHeader());

	luReclass(m); // need to change Qia Oct 06 2008

	char str[100];
	sprintf(str, "%s/lu", parameters.outputDir);

	m.setCellSize(parameters.cellSize);

	m.write(str, red, green, blue, wAdfGeoTransform);
}





//This will print the landis parameter file format on CRT.
void help()
{

	printMessage("-------------------------LANDIS PARAMETER INPUT FILE--------------------------");

	printMessage("Species Attribute File   #Species vital attribute file#");

	printMessage("Landtype Attribute File  #Attributes of each landtype class#");

	printMessage("Landtype Map File        #Input map of landtype#");

	printMessage("Species Map File         #Input map of species and their age classes#");

	printMessage("Map Attribute File       #Attributes of each map elements#");

	printMessage("Maps Indexes File        #Species name indexes for map output#");

	printMessage("Age Index File           #Species name indexes for age map output#");

	printMessage("Output Directory         #Path for Landis output#");

	printMessage("Disturbance File         #Disturbance Input File#");

	printMessage("default.plt              #Required in the parameter file directory#");

	printMessage("freq_out.put             #Setting for alternative output#");

	printMessage("Iteration Number         #Number of iterations of 10 year step#");

	printMessage("Random Number Seed       #0: real time seed, others: fixed seed#");

	printMessage("Cell Size                #Cell size in meters#");

	printMessage("Seed Dispersal Method    #The name of the seed dispersal routine#");

	printMessage("Wind Switch              #0-no, 1-standard, 2-mean, 3-strong, 4-light#");

	printMessage("Fire Switch              #0-no, 1-standard, 2-mean, 3-strong, 4-light#");

	printMessage("Harvesting Switch        #0-harvesting off, 1-harvesting on#");

	if ((gDLLMode & G_HARVEST) != 0)
	{
		printMessage("Harvest Event Switch     #0-harvesting module off, 1-harvesting moduel on#");

		printMessage("Stand Adjacency Flag     #0-off, 1-on#");

		printMessage("n                        #decade span to consider an adjacent stand recently harvested#");

		printMessage("p                        #Proportion of cells cut in the last n decades is at least p#");

		printMessage("Harvest Event File       #User defined harvest scenarios#");

		printMessage("Stand Map                #Stand identifier map#");

		printMessage("Management Area Map      #Management area identifier map#");

		printMessage("Stand Log File           #Harvest log by stands#");

		printMessage("Management Area Log File #Harvest log by management areas#");
	}

}



void argHelp()
{

	printMessage("LANDIS Version Pro 7.0 ");

	printMessage("University of Missouri--Columbia ");

	printMessage("(C) Copyright 2000-2009");

	printMessage(" ");

	printMessage("Usage: LANDIS [-argument [...]] <input file> ");

	printMessage(" ");

	printMessage("       argument:");

	printMessage("       -e: the years environmental change interpreted");

	printMessage("       -h: this help menu");

	printMessage("       -p: input file help");

	printMessage("       -r: the year for reclassification");

}






void  OutputScenario()
{
	printf("Output Landis Scenario.txt....\n\n");

	char	strScenario[2000];

	sprintf(strScenario, "%s/%s", parameters.outputDir, "Scenario.txt");

	FILE* pfScenario = fopen(strScenario, "w");

	fprintf(pfScenario, "Landis Scenario.txt\n\n");
	fprintf(pfScenario, "Landis version:		Pro 1.0\n");
	fprintf(pfScenario, "Output dir:		%s\n", parameters.outputDir);
	fprintf(pfScenario, "specAttrFile:		%s\\%s\n", parameters.outputDir, parameters.specAttrFile);
	fprintf(pfScenario, "landUnitFile:		%s\\%s\n", parameters.outputDir, parameters.landUnitFile);
	fprintf(pfScenario, "siteInFile:		%s\\%s\n", parameters.outputDir, parameters.siteInFile);
	fprintf(pfScenario, "\nMAIN PARAMETERS:----------------------------------------\n");



	if (parameters.randSeed)
		fprintf(pfScenario, "Random:			repeatable (1)\n");
	else
		fprintf(pfScenario, "Random:			NOT repeatable (0)\n");

	fprintf(pfScenario, "numIter:	%d\n", parameters.numIter);
	fprintf(pfScenario, "Map size:		Row: %d x Col: %d\n", sites.numRows(), sites.numColumns());

	fprintf(pfScenario, "\n\nDLLs----------------------------------------\n");

	if (gDLLMode & G_BDA)
		fprintf(pfScenario, "BDA ...............is turned on\n");

	if (gDLLMode & G_WIND)
		fprintf(pfScenario, "Wind ..............is turned on \n");

	if (gDLLMode & G_HARVEST)
		fprintf(pfScenario, "Harvest ...........is turned on \n");

	if (gDLLMode & G_FUEL)
		fprintf(pfScenario, "Fuel ..............is turned on \n");

	if (gDLLMode & G_FUELMANAGEMENT)
		fprintf(pfScenario, "Fuel management....is turned on \n");

	if (gDLLMode & G_FIRE)
		fprintf(pfScenario, "Fire ..............is turned on \n");

	fprintf(pfScenario, "\n");



	if (!(gDLLMode & G_BDA))
		fprintf(pfScenario, "BDA ....................  off\n");

	if (!(gDLLMode & G_WIND))
		fprintf(pfScenario, "Wind .................... off \n");

	if (!(gDLLMode & G_HARVEST))
		fprintf(pfScenario, "Harvest ..................off \n");

	if (!(gDLLMode & G_FUEL))
		fprintf(pfScenario, "Fuel .................... off\n");

	if (!(gDLLMode & G_FUELMANAGEMENT))
		fprintf(pfScenario, "Fuel management ..........off \n");

	if (!(gDLLMode & G_FIRE))
		fprintf(pfScenario, "Fire .................... off\n");

	fclose(pfScenario);

}

