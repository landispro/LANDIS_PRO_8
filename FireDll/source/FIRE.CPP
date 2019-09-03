#include <string.h>

#include <stdlib.h>

#include <stdio.h>

#include <math.h>

#include <direct.h>

#include "defines.h"

#include "system1.h"

#include "error.h"

#include "point.h"

#include "pile.h"

#include "landu.h"

#include "specatt.h"

#include "specie.h"

#include "map8.h"

#include "time.h"

#include "FIRE.H"

#include "float.h"

#include <iostream>

#include <cstdio>

#include <stdio.h>

#include "gdal_priv.h"

#include "cpl_conv.h" // for CPLMalloc()

#include "cpl_string.h"

#include "ogr_spatialref.h"

//#include <vld.h>


#define PI 3.1415926

#define __DEBUG 0

//add by houbin chen 10/4/2017
#define DTN 0

#define DTNBA 1

#define DTNBIO 2

#define COUNT 3
//add by houbin chen 10/4/2017


static const float  lifespan[5]={(float)0.0,(float).2,(float).4,(float).7,(float).85};

static int red[maxLeg]={0,0,100,150,200,0,0,0,150,0,150,255,80,150,255};

static int green[maxLeg]={0,0,0,0,0,100,150,255,0,150,150,255,80,150,255};

static int blue[maxLeg]={0,150,0,0,0,0,0,0,150,150,0,0,80,150,255};





//////////////////////////////////////////////////////////////////////////////

//			    ASSOCIATED FIRE FUNCTIONS                                      //

//////////////////////////////////////////////////////////////////////////////

static PILE pile;



//////////////////////////////////////////////////////////////////////

// Construction/Destruction

//////////////////////////////////////////////////////////////////////



CFIRE::CFIRE()

{
	memset(m_strTSLF, 0, 255);
	memset(m_strFireOutputDirectory, 0, 255);

}



CFIRE::CFIRE(char*	strfn, int mode, SITES* outsites, LANDUNITS* outlus, 
	SPECIESATTRS*  outsa, PDP* ppdp, int nFNOI, char * strOutput,int randSeed)
{
	GDALDataset  *fpImg; 

	double        adfGeoTransform[6]; 

	GDALAllRegister();

	double wAdfGeoTransform[6];

	FILE * fp = LDfopen(strfn,1);

	if (fp==NULL) 
		errorSys("FIRE: FIRE parameter file not found.",STOP);

	ReadParam(fp);

	LDfclose(fp);	

	//add the main output directory in front of the subdirectory

	char s[1024];

	sprintf(s,"%s\\Fire",strOutput);

	_mkdir(s);

	if (ppdp->TrackFlag == 1)
	{
		char outputdir_spec_dtn[100];
		char outputdir_spec_dtnba[100];
		char outputdir_spec_dtnbio[100];
		char outputdir_agerange_spec_dtn_all[100];

		sprintf(outputdir_spec_dtn, "%s\\%s", s, "DTN");
		sprintf(outputdir_spec_dtnba, "%s\\%s", s, "DTNBA");
		sprintf(outputdir_spec_dtnbio, "%s\\%s", s, "DTNBIO");
		sprintf(outputdir_agerange_spec_dtn_all, "%s\\%s", s, "SPEC_AGERANGE");

		if (_mkdir(outputdir_spec_dtn) != 0)
		{
			if (errno != EEXIST)
				errorSys("Fire: Can't create the dead tree number in diff spec direcory", STOP);
		}
		if (_mkdir(outputdir_spec_dtnba) != 0)
		{
			if (errno != EEXIST)
				errorSys("Fire: Can't create the dead tree basalarea in diff spec direcory", STOP);
		}
		if (_mkdir(outputdir_spec_dtnbio) != 0)
		{
			if (errno != EEXIST)
				errorSys("Fire: Can't create the dead tree biomass in diff spec direcory", STOP);
		}
		if (_mkdir(outputdir_agerange_spec_dtn_all) != 0)
		{
			if (errno != EEXIST)
				errorSys("Fire: Can't create the dead tree number&biomass&basalarea in diff spec&agerange direcory", STOP);
		}

	}

	sprintf(s,"%s\\Fire\\%s",strOutput,m_fireParam.logFn);

	sprintf(m_fireParam.logFn,"%s",s);

	sprintf(s,"%s\\Fire\\%s",strOutput,m_fireParam.yearlyFn);

	sprintf(m_fireParam.yearlyFn,"%s",s);

	sprintf(s,"%s\\Fire\\%s",strOutput,m_fireParam.finalFn);

	sprintf(m_fireParam.finalFn,"%s",s);

	sprintf(m_strTSLF,"%s\\Fire\\TSLF",strOutput);

	sprintf(m_strFireOutputDirectory,"%s\\Fire\\",strOutput);



	//data initilization

	m_lptrIgnitionOccurance = NULL;

	m_iptrValidLandUnits = NULL;


	if (randSeed)
		m_pStochastic = new StochasticLib(randSeed);
	else
		m_pStochastic = new StochasticLib(time(0));
	

	m_pPDP = ppdp;

	m_pSPECIESATTRS = outsa;

	m_pLAND = outsites;

	m_fireParam.m_nFNOI = nFNOI;

	m_DLLMode = mode;

	m_fireParam.rows = outsites->numRows();

	m_fireParam.cols = outsites->numColumns();

	//FireSites

	m_pFireSites.init(m_fireParam.rows, m_fireParam.cols);


	if (m_fireParam.iDEMFlag) 
		ReadDEM(m_fireParam.DEMDataFn);

	//FireRegimeUnits

	m_pFireRegimeUnits.init(MAX_LANDUNITS,m_pStochastic);

	fp = LDfopen(m_fireParam.fireRegimeAttrFn,1);

	if (fp==NULL) 
		errorSys("FIRE: fire regime attribute file not found.",STOP);

	m_pFireRegimeUnits.read(fp);

	LDfclose(fp);

	m_pFireRegimeUnits.attach(&m_pFireSites);	

	if (m_fireParam.iFireRegimeFlag != 0)

	{
		if ((fpImg = (GDALDataset *)GDALOpen(m_fireParam.fireRegimeDataFn, GA_ReadOnly)) == NULL) //* landtype.img

			errorSys("landtype img map input file not found.", STOP);

		if (fpImg->GetGeoTransform(adfGeoTransform) == CE_None)
		{
			for (int i = 0; i < 6; i++)
				wAdfGeoTransform[i] = adfGeoTransform[i];
		} 

		m_pFireRegimeUnits.readFireRegimeIMG(fpImg);

		GDALClose((GDALDatasetH)fpImg); //*

	}

	else 

	{

		AttachLandUnitGIS(outsites);

	}

	m_pFireRegimeUnits.dispatch();

	m_pFireRegimeUnits.updateIGDensity(m_fireParam.cellSize);



	

	//initilize fire info in public data pool	
	for (int i = 1; i <=outsites->numRows(); i++)
	{
		for (int j = 1; j <=outsites->numColumns(); j++)
		{
			const int tempID = m_pFireSites(i, j).FRUIndex;

			m_pPDP->sTSLFire[i][j] = m_pFireRegimeUnits.operator ()(tempID)->initialLastFire;	
		}
	}

	m_cummInitiationMap.dim(outsites->numRows(),outsites->numColumns());

	//write TSLF
	WriteTSLF(outsites->numRows(), outsites->numColumns(), 0, wAdfGeoTransform);

	m_FinneyDebugOutput = 0;
}



CFIRE::~CFIRE()

{

	//do I need to delete m_pFireRegimeUnits here

	//Notice that in my deconstruction of CFireRegimeUnits, I have a delete statement also.

	delete m_pStochastic;

}





void CFIRE::Activate(int itr, int freq[], double wAdfGeoTransform[])
{
	double probForSite;    //This is the probability of firethr. init. on a site.       

	char 	 str[256] = {0};        //Character string. 

	int 	 i, j, k;  //rows and coloums 

	long   amtdamaged=0;

 
	for (int i = 0; i< 42; i++)
		m_iWindEventsLog[i] = 0;

	m_itr = itr;

	//Fill map.

	const int snr = m_pFireSites.numRows();
	const int snc = m_pFireSites.numColumns();

	m_iMapRow = snr;
	m_iMapColumn = snc;


	if(m_fireParam.iFuelFlag==2 || m_fireParam.iFuelFlag==3)
		FinneyInitilization();


	m_Map.dim(snr,snc);

	for (int i=1;i<=snr;i++)
	{
		for (int j=1;j<=snc;j++)
		{
			int tempID = m_pFireSites(i,j).FRUIndex;

			if (m_pFireRegimeUnits.operator ()(tempID)->active())
				m_Map(i,j)=1;
			else
				m_Map(i,j)=0;
		}
	}


	if (itr == 1)
		m_cummMap = m_Map;


	m_InitiationMap.dim(snr,snc);

	FILE*  logfile;        //fire log file.

	if (itr == 1)
		logfile = fopen(m_fireParam.logFn, "w");
	else
		logfile = fopen(m_fireParam.logFn, "a");

	if (logfile==NULL)
		errorSys("Error opening fire log file",STOP);

	m_LogFP = logfile;

	long lIgChecking=0;

	m_iNumLU = m_pFireRegimeUnits.number();

	m_lptrIgnitionOccurance = (long *) calloc(m_iNumLU, sizeof(long));

	m_iptrValidLandUnits = (int *)calloc(m_iNumLU, sizeof(int));

	m_pIgnitionStatusArray = (char *)calloc(snr*snc + 1, sizeof(char));

	m_pFireSites.create_FireRegimeUnitsListByIndex();	

	j = m_iNumLU;	

	for (k=0,i=0;i<j;i++)
	{
		float IgDensity = m_pFireRegimeUnits.operator () (i)->m_fIgPoisson/10*m_pLAND->TimeStep_Fire;

		m_FRUAvailableCells[i] = m_pFireRegimeUnits.NumOfSites[i];

		if (IgDensity >0)
			m_lptrIgnitionOccurance[i] = m_pStochastic->Poisson(IgDensity);
		else
			m_lptrIgnitionOccurance[i] = 0;

		if (m_lptrIgnitionOccurance[i]>0) 
			m_iptrValidLandUnits[k++] = i;
		else 
			m_iNumLU --;
	}


	

	//add by houbin 3/4/2017
	int specnum = m_pSPECIESATTRS->number();
	if (m_pPDP->TrackFlag == 1)
	{
		for (int iii = 0; iii < specnum; iii++)
		{
			for (int jjj = 0; jjj < snr; jjj++)
			{
				for (int kkk = 0; kkk < snc; kkk++)
				{
					m_pPDP->cDeadTreeBA[jjj][kkk][iii] = 0;
					m_pPDP->cDeadTreeBio[jjj][kkk][iii] = 0;
					m_pPDP->cDeadTreeNum[jjj][kkk][iii] = 0;
				}
			}
		}//clear the dead tree number, dead tree biomass and dead tree basal area information of last fireing year 
		//add by houbin 3/4/2017
		for (int iii = 0; iii < snr; iii++)
		{
			for (int jjj = 0; jjj < snc; jjj++)
			{
				for (int kkk = 0; kkk < m_pPDP->interval_number*m_pPDP->specnum; kkk++)
				{
					m_pPDP->cDeadTreeNum_spec_agerange[iii][jjj][kkk] = 0;
					m_pPDP->cDeadTreeBA_spec_agerange[iii][jjj][kkk] = 0;
					m_pPDP->cDeadTreeBio_spec_agerange[iii][jjj][kkk] = 0;
				}
			}
		}
	}
	

	LDPOINT p1;

	while (m_iNumLU>0) //Dead Lock in this loop //Qia on May 1st 2009
	{
		lIgChecking++;	

		//randomly select k from 0 to iNumLU-1 //J.Yang use stochastic.uniform instead?
		k=irand(0,m_iNumLU-1);

		k = (int) m_pStochastic->IRandom(0, m_iNumLU-1);



		p1 = Retrieve(m_iptrValidLandUnits[k]);			


		m_lptrIgnitionOccurance[m_iptrValidLandUnits[k]]--;

		if (m_lptrIgnitionOccurance[m_iptrValidLandUnits[k]]==0)

		{

			for (i=k;i<m_iNumLU-1;i++)

				m_iptrValidLandUnits[i]= m_iptrValidLandUnits[i+1];

			m_iNumLU--;

		}

		//check and perform disturbance

		const int p1y = p1.y;
		const int p1x = p1.x;

		int tempID = m_pFireSites(p1y,p1x).FRUIndex;

		if (m_pFireRegimeUnits.operator ()(tempID)->active())
		{
			//Calculating fire initiation probability

			if ( !(m_DLLMode& G_FUEL)) //using old fire initiation routine

			{
				 //mean fire return interval
				int fireINTERV=m_pFireRegimeUnits.operator ()(tempID)->fireInterval;

				if (fireINTERV == 0) 
					probForSite = 1;
				else
					probForSite= exp((double) m_pPDP->sTSLFire[p1y][p1x]*((double) -1/fireINTERV));
			}

			else

			{

				//using FF loading class

				//FUEL module has to be turned on to use fine fuel

				i = (int) m_pPDP->cFineFuel[p1y][p1x];

				if (i>=1 && i <=5)

					probForSite = (1 - m_fireParam.fInitiationProb[i-1]);//10*m_pLAND->TimeStep;// Bu, Rencang April 28, 2009

				else

					probForSite = 1;

				// 1 success means no fire till this time

				// 0 failure means fire occure till now

			}

			m_fWind = 0.0;

			m_fSlope = 0.0;

			m_dFSFactor = 0.0;

			if(probForSite > 1) 

			{

				printf("1 - fire initiation probability (probForSite: %f) is larger than 1",probForSite);

				printf(" at site %d %d \n", p1y,p1x);

				fflush(stdout); 

				probForSite = 1;

			}

			if(probForSite < 0) 
			{
				printf("1 - fire initiation probability (probForSite: %f) is less than 0",probForSite);

				printf(" at site %d %d \n", p1y,p1x);

				fflush(stdout); 

				probForSite = 0;
			}



			if (m_pStochastic->Bernoulli(probForSite)==0)
	 		{
	  			// start a fire		
				//update initiation map

				m_InitiationMap(p1y, p1x) = 1;

				m_cummInitiationMap(p1y, p1x)++;
				
				if(m_fireParam.iFuelFlag == 1)
				{
					//percolation method
					amtdamaged =  fireSpread(p1y, p1x);
				}
				else if(m_fireParam.iFuelFlag==2 || m_fireParam.iFuelFlag==3)
				{
					m_iOriginRow = p1y;

					m_iOriginColumn = p1x;

					amtdamaged = FinneySpread();
				}				 
				else
				{
					int tempID = m_pFireSites(p1y, p1x).FRUIndex;

					float tempMFS = m_pFireRegimeUnits.operator ()(tempID)->m_fMFS;

					float tempSTD = m_pFireRegimeUnits.operator ()(tempID)->m_fFireSTD;

					long arealeft = fireSize(tempMFS,tempSTD); 	  		

					amtdamaged=(long) disturb(p1y,p1x,(int)arealeft,m_Map,logfile,probForSite,itr);	  		

				}
				//printf("damage finished\n\n");			
	 		}

		}

	}



	//add by houbin 10/26/2017
	SITE * siteptr = (*m_pLAND) (2, 2);
	if (m_pPDP->TrackFlag == 1)
	{
		if (m_pLAND->OutputAgeRangeFireFlag == 1)
		{
			for (int i = 0; i < specnum; i++)
			{
				if (m_pLAND->GetOutputAgeRangeModuleFlagArray(i, DTN))
				{
					for (int j = 1; j <= m_pLAND->GetOutputAgeRangeModuleFlagArray(i, COUNT); j++)
					{
						MAP8 map_spec_agerange_dtn;
						map_spec_agerange_dtn.dim(snr, snc);
						for (int jjj = 1; jjj <= snr; jjj++)
						{
							for (int kkk = 1; kkk <= snc; kkk++)
							{
								//printf("s%d ", jjj + kkk);
								map_spec_agerange_dtn(jjj, kkk) = m_pPDP->cDeadTreeNum_spec_agerange[jjj - 1][kkk - 1][(j - 1)*m_pPDP->specnum + i];
							}
						}

						char dirDTN_spec_agerange[255];
						char strDTN_spec_agerange[255];
						int low = m_pLAND->GetOutputAgeRangeModuleFlagArray(i, j, 0);
						int high = m_pLAND->GetOutputAgeRangeModuleFlagArray(i, j, 1);
						//printf("dtn(%d,%d) ", i, j);
						sprintf(dirDTN_spec_agerange, "%s/SPEC_AGERANGE", m_strFireOutputDirectory);
						sprintf(strDTN_spec_agerange, "%s/%s_%d_%d_TreeNum_fire_%d", dirDTN_spec_agerange, siteptr->specAtt(i + 1)->name, low, high, itr*m_pLAND->TimeStep_Fire);
						map_spec_agerange_dtn.setCellSize(m_fireParam.cellSize);
						map_spec_agerange_dtn.write(strDTN_spec_agerange, red, green, blue, wAdfGeoTransform);
						map_spec_agerange_dtn.~MAP8();
					}
				}





				if (m_pLAND->GetOutputAgeRangeModuleFlagArray(i, DTNBA))
				{
					for (int j = 1; j <= m_pLAND->GetOutputAgeRangeModuleFlagArray(i, COUNT); j++)
					{
						MAP8 map_spec_agerange_dtnba;
						map_spec_agerange_dtnba.dim(snr, snc);

						for (int jjj = 1; jjj <= snr; jjj++)
						{
							for (int kkk = 1; kkk <= snc; kkk++)
							{
								//printf("s%d ", jjj + kkk);
								int x = ((jjj - 1)*snc) + kkk - 1;
								map_spec_agerange_dtnba.data_bio[x] = m_pPDP->cDeadTreeBA_spec_agerange[jjj - 1][kkk - 1][(j - 1)*m_pPDP->specnum + i];
							}
						}

						char dirDTNBA_spec_agerange[255];
						char strDTNBA_spec_agerange[255];
						int low = m_pLAND->GetOutputAgeRangeModuleFlagArray(i, j, 0);
						int high = m_pLAND->GetOutputAgeRangeModuleFlagArray(i, j, 1);

						sprintf(dirDTNBA_spec_agerange, "%s/SPEC_AGERANGE", m_strFireOutputDirectory);
						sprintf(strDTNBA_spec_agerange, "%s/%s_%d_%d_TreeBA_fire_%d", dirDTNBA_spec_agerange, siteptr->specAtt(i + 1)->name, low, high, itr*m_pLAND->TimeStep_Fire);
						map_spec_agerange_dtnba.setCellSize(m_fireParam.cellSize);
						map_spec_agerange_dtnba.write(strDTNBA_spec_agerange, red, green, blue, wAdfGeoTransform);

					}
				}
				if (m_pLAND->GetOutputAgeRangeModuleFlagArray(i, DTNBIO))
				{
					for (int j = 1; j <= m_pLAND->GetOutputAgeRangeModuleFlagArray(i, COUNT); j++)
					{
						MAP8 map_spec_agerange_dtnbio;
						map_spec_agerange_dtnbio.dim(snr, snc);
						for (int jjj = 1; jjj <= snr; jjj++)
						{
							for (int kkk = 1; kkk <= snc; kkk++)
							{
								//printf("s%d ", jjj + kkk);
								map_spec_agerange_dtnbio(jjj, kkk) = m_pPDP->cDeadTreeBio_spec_agerange[jjj - 1][kkk - 1][(j - 1)*m_pPDP->specnum + i];
							}
						}

						char dirDTNBIO_spec_agerange[255];
						char strDTNBIO_spec_agerange[255];
						int low = m_pLAND->GetOutputAgeRangeModuleFlagArray(i, j, 0);
						int high = m_pLAND->GetOutputAgeRangeModuleFlagArray(i, j, 1);
						//printf("(%d,%d) ", i, j);
						sprintf(dirDTNBIO_spec_agerange, "%s/SPEC_AGERANGE", m_strFireOutputDirectory);
						sprintf(strDTNBIO_spec_agerange, "%s/%s_%d_%d_TreeBio_fire_%d", dirDTNBIO_spec_agerange, siteptr->specAtt(i + 1)->name, low, high, itr*m_pLAND->TimeStep_Fire);
						map_spec_agerange_dtnbio.setCellSize(m_fireParam.cellSize);
						map_spec_agerange_dtnbio.write(strDTNBIO_spec_agerange, red, green, blue, wAdfGeoTransform);

					}
				}
			}
		}


		if (m_pLAND->OutputFireFlag == 1)
		{
			for (int iii = 0; iii < specnum; iii++)
			{
				//printf("spec%d:\n", iii + 1);
				SITE * siteptr = (*m_pLAND) (2, 2);
				if (m_pLAND->GetOutputModuleFlagArray(iii, DTN))
				{
					if (strcmp(siteptr->specAtt(iii + 1)->name, "N/A") != 0)
					{
						MAP8 map_dtn;
						map_dtn.dim(snr, snc);
						for (int jjj = 1; jjj <= snr; jjj++)
						{
							for (int kkk = 1; kkk <= snc; kkk++)
							{
								map_dtn(jjj, kkk) = m_pPDP->cDeadTreeNum[jjj - 1][kkk - 1][iii];
							}
						}
						char dirDTN[255];
						char strDTN[255];
						sprintf(dirDTN, "%s/DTN", m_strFireOutputDirectory);
						sprintf(strDTN, "%s/%s_TreeNum_fire_%d", dirDTN, siteptr->specAtt(iii + 1)->name, itr*m_pLAND->TimeStep_Fire);
						map_dtn.setCellSize(m_fireParam.cellSize);
						map_dtn.write(strDTN, red, green, blue, wAdfGeoTransform);
					}
				}
				if (m_pLAND->GetOutputModuleFlagArray(iii, DTNBA))
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
								map_dtn_ba.data_bio[x] = m_pPDP->cDeadTreeBA[jjj - 1][kkk - 1][iii];
							}
						}
						char dirDTNBA[255];
						char strDTNBA[255];
						sprintf(dirDTNBA, "%s/DTNBA", m_strFireOutputDirectory);
						sprintf(strDTNBA, "%s/%s_BA_fire_%d", dirDTNBA, siteptr->specAtt(iii + 1)->name, itr*m_pLAND->TimeStep_Fire);
						map_dtn_ba.setCellSize(m_fireParam.cellSize);
						map_dtn_ba.write(strDTNBA, red, green, blue, wAdfGeoTransform);
					}
				}
				if (m_pLAND->GetOutputModuleFlagArray(iii, DTNBIO))
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
								map_dtn_bio.data_bio[x] = m_pPDP->cDeadTreeBio[jjj - 1][kkk - 1][iii];
							}
						}
						char dirDTNBIO[255];
						char strDTNBIO[255];
						sprintf(dirDTNBIO, "%s/DTNBIO", m_strFireOutputDirectory);
						sprintf(strDTNBIO, "%s/%s_BIO_fire_%d", dirDTNBIO, siteptr->specAtt(iii + 1)->name, itr*m_pLAND->TimeStep_Fire);
						map_dtn_bio.setCellSize(m_fireParam.cellSize);
						map_dtn_bio.write(strDTNBIO, red, green, blue, wAdfGeoTransform);
					}
				}
			}
		}
	}
	
	//add by houbin 10/26/2017

	//printf("testing:\n");
	
	


	//Add data to cummMap

	//and change TSLF

	for (i=1;i<=snr;i++)

		for (j=1;j<=snc;j++)

		{

			if (m_Map(i,j)>=2)

			{

				m_cummMap(i,j)=max(m_Map(i,j),m_cummMap(i,j)); 

					//J.Yang should be max(m_cummMap(i,j), m_Map(i,j))

				m_pPDP->sTSLFire[i][j] = 0;

			}

			else

				m_pPDP->sTSLFire[i][j] += m_pLAND->TimeStep_Fire;

		}


	if (((itr%freq[1])==0)&&(freq[1]<=m_pLAND->TimeStep_Fire)||(itr*m_pLAND->TimeStep_Fire==freq[1])&&(freq[1]>=0)) 

	{                                                                     

		//Write map output file.

		sprintf(str,"fire damage assessment for year %d.",itr*m_pLAND->TimeStep_Fire);        

		m_Map.setHeader(m_pLAND->getHeader());

		m_Map.rename(str);

		for(i=0;i<maxLeg;i++)

		{

			m_Map.assignLeg(i,"");

		}

		m_Map.assignLeg(0,"NonActive");

		m_Map.assignLeg(1,"No Fires");

		m_Map.assignLeg(2,"Class 1");

		m_Map.assignLeg(3,"Class 2");

		m_Map.assignLeg(4,"Class 3");

		m_Map.assignLeg(5,"Class 4");

		m_Map.assignLeg(6,"Class 5");

		sprintf(str,"%s%d",m_fireParam.yearlyFn,itr*m_pLAND->TimeStep_Fire);

		m_Map.setCellSize(m_fireParam.cellSize);

		//double wAdfGeoTransform[6] = { 0.00, m_fireParam.cellSize, 0.00, 600.00, 0.00, -m_fireParam.cellSize };//*

		float nodata = 0;

		m_Map.write(str, red, green, blue, wAdfGeoTransform);



		WriteInitiationMap(snr, snc, itr, wAdfGeoTransform);

		//write time-since-last-fire map

		WriteTSLF(snr, snc, itr, wAdfGeoTransform);



	}



	LDfclose(logfile);

	if (itr==m_fireParam.m_nFNOI)

	{

		//Write cumulative map output file.

		sprintf(str,"Cumulative fire damage assessment."); 

		m_cummMap.setHeader(m_pLAND->getHeader());

		m_cummMap.rename(str);

		for(i=0;i<maxLeg;i++)

			m_cummMap.assignLeg(i,"");

		m_cummMap.assignLeg(0,"NonActive");

		m_cummMap.assignLeg(1,"No Fires");

		m_cummMap.assignLeg(2,"Class 1");

		m_cummMap.assignLeg(3,"Class 2");

		m_cummMap.assignLeg(4,"Class 3");

		m_cummMap.assignLeg(5,"Class 4");

		m_cummMap.assignLeg(6,"Class 5");

		sprintf(str,"%s",m_fireParam.finalFn);

		m_cummMap.setCellSize(m_fireParam.cellSize);

		//double wAdfGeoTransform[6] = { 0.00, m_fireParam.cellSize, 0.00, 600.00, 0.00, -m_fireParam.cellSize };//*

		float nodata = 0;

		m_cummMap.write(str, red, green, blue, wAdfGeoTransform);

		WriteCummInitiationMap(snr, snc, itr, wAdfGeoTransform);

	}

	PrintWindLog();

	if(m_fireParam.iFuelFlag==2 || m_fireParam.iFuelFlag == 3)

		FinneyFreeMemory();

	free(m_pIgnitionStatusArray);

	free(m_lptrIgnitionOccurance);

	free(m_iptrValidLandUnits);		



}







void CFIRE::ReadParam(FILE *infile)

{

	

	int i,j;

	if (fscanc(infile, "%d", &m_fireParam.iFuelFlag) != 1) 

		errorSys("Error in reading new-fire-algorithms-flag.", STOP);

	if (fscanc(infile, "%d", &m_fireParam.iWindFlag) != 1) 

		errorSys("Error in reading wind flag.", STOP);

	if (fscanc(infile, "%d", &m_fireParam.iDEMFlag) != 1) 

		errorSys("Error in reading DEM flag.", STOP);

	if (fscanc(infile, "%d", &m_fireParam.iFireRegimeFlag) != 1) 

		errorSys("Error in reading fire regime flag.", STOP);

	if (fscanc(infile, "%d", &m_fireParam.iTSLFFlag) != 1) 

		errorSys("Error in reading TSLF flag.", STOP);

	for (i=0;i<5;i++)  {

		if (fscanc(infile,"%f",&m_fireParam.fInitiationProb[i])!=1)

			errorSys("Error reading in Initiation Probability.",STOP);

		//m_fireParam.fInitiationProb[i]=m_fireParam.fInitiationProb[i]/10*m_pLAND->TimeStep;

		}

	

	if (fscanc(infile, "%f", &m_fireParam.fSpreadProb) != 1) 

		errorSys("Error in reading SpreadProbability.", STOP);

	if (fscanc(infile, "%f", &m_fireParam.fCoeff1) != 1) 

		errorSys("Error in reading Wind coefficient.", STOP);

	if (fscanc(infile, "%f", &m_fireParam.fCoeff2) != 1) 

		errorSys("Error in reading Topography coefficient.", STOP);

	if (fscanc(infile, "%f", &m_fireParam.fCoeff3) != 1) 

		errorSys("Error in reading Predefined fire size distribution coefficient.", STOP);



	/* Reading prevaling fire regime

	*/

	if (fscanc(infile, "%d", &m_fireParam.iNonWindPercent) != 1) 

		errorSys("Error in reading Percentage of Non wind events.", STOP);

	for (i=0;i<40;i++)  

		if (fscanc(infile,"%d",&m_fireParam.iCummWindClass[i]) !=1)

			errorSys("Error reading in Cummulative probabilities of wind classes.",STOP);

	



	if (fscanc(infile, "%s", m_fireParam.fireRegimeAttrFn) != 1) 

		errorSys("Error in reading file name for fire Regime Attributes", STOP);

	if (fscanc(infile, "%s", m_fireParam.fireRegimeDataFn) != 1) 

		errorSys("Error in reading file name for fire Regime data", STOP);

	if (fscanc(infile, "%s", m_fireParam.DEMDataFn) != 1) 

		errorSys("Error in reading file name for DEM data", STOP);
	
	if (fscanc(infile, "%s", m_fireParam.File_regime_change) != 1) 

		errorSys("Error in reading file name for DEM data", STOP);

	if (!strcmp (m_fireParam.File_regime_change, "N/A")){
		flag_regime_update=0;
	}
	else if(!strcmp (m_fireParam.File_regime_change, "0")){
		flag_regime_update=0;
	}
	else{
		flag_regime_update=1;
	}

	if (fscanc(infile, "%s", m_fireParam.yearlyFn) != 1) 

		errorSys("Error in reading file name for iterationaly fire information output", STOP);

	if (fscanc(infile, "%s", m_fireParam.finalFn) != 1) 

		errorSys("Error in reading file name for final fire information output", STOP);

	if (fscanc(infile, "%s", m_fireParam.logFn) != 1) 

		errorSys("Error in reading file name for log fire information output", STOP);

	



	if (fscanc(infile, "%d", &m_fireParam.cellSize) != 1) 

		errorSys("Error in reading cell size.", STOP);

	for(i = 0;i<5;i++){
		float temp;
		for(j=0;j<3;j++){
			if (fscanc(infile, "%f", &temp) != 1) 
				errorSys("Error in reading values.", STOP);
			m_fireParam.fire_betavalues[i][j] = temp;
		}
		if (fscanc(infile, "%f", &temp) != 1) 
			errorSys("Error in reading values.", STOP);
		m_fireParam.fire_X2values[i] = temp;
	}

	if (2 == m_fireParam.iFuelFlag || 3 == m_fireParam.iFuelFlag) //using Finney spread method 2 duration. 3 fire size

	{

		//use Finney spread method, it needs another file for parameters used in Finney method

		if (fscanc(infile, "%s", m_fireParam.FinneyParamFN) != 1) 

			errorSys("Error in reading parameter file name for using Finney spread method",STOP);

		//read in finney parameter space

		FILE * fp;

		fp = LDfopen(m_fireParam.FinneyParamFN,1);

		if (fp==NULL) 

		errorSys("FIRE: FIRE Finney parameter file not found.",STOP);

		for (i=0;i<6;i++)

		{

			for (j=0;j<6;j++)

			{				

				if (fscanc(fp,"%f",&m_finneyParam.spreadRate[i][j]) !=1)

					errorSys("Error reading in fire spread rate for using Finney spread method.",STOP);

			}

		}



		for (i=0;i<6;i++)

			if (fscanc(fp,"%f",&m_finneyParam.ellipseAxisRatio[i])!=1)

				errorSys("Error reading in ellipse axis ratio for using Finney spread method.",STOP);

		if (fscanc(fp,"%f",&m_finneyParam.fuelWeight)!=1)

			errorSys("Error reading in fuel weight for using Finney spread method.",STOP);

		if (fscanc(fp,"%f",&m_finneyParam.windWeight)!=1)

			errorSys("Error reading in wind weight for using Finney spread method.",STOP);

		if (fscanc(fp,"%f",&m_finneyParam.TopoWeight)!=1)

			errorSys("Error reading in topo weight for using Finney spread method.",STOP);



		if (1 == m_fireParam.iDEMFlag) 

		{

			for (i=0;i<6;i++)

			{

				for (j=0;j<6;j++)

				{				

					if (fscanc(fp,"%f",&m_finneyParam.ROS_low[i][j]) !=1)

						errorSys("Error reading in ROS for low steepness (slope class 1) area.",STOP);

				}

			}



			for (i=0;i<6;i++)

			{

				for (j=0;j<6;j++)

				{				

					if (fscanc(fp,"%f",&m_finneyParam.ROS_moderate[i][j]) !=1)

						errorSys("Error reading in ROS for moderate steepness (slope class 2) area.",STOP);

				}

			}



			for (i=0;i<6;i++)

			{

				for (j=0;j<6;j++)

				{				

					if (fscanc(fp,"%f",&m_finneyParam.ROS_high[i][j]) !=1)

						errorSys("Error reading in ROS for high steepness (slope class 3) area.",STOP);

				}

			}



			for (i=0;i<6;i++)

			{

				for (j=0;j<6;j++)

				{				

					if (fscanc(fp,"%f",&m_finneyParam.ROS_extreme[i][j]) !=1)

						errorSys("Error reading in ROS for extreme steepness (slope class 4) area.",STOP);

				}

			}





		}



		

	LDfclose(fp);

	}

	

}





void CFIRE::AttachLandUnitGIS(SITES* outsites)

{

	int numFRU = m_pFireRegimeUnits.number();



	for (int i=outsites->numRows();i>0;i--)

	{

		for (int j=1;j<=outsites->numColumns();j++)

		{

   			int tempID = outsites->locateLanduPt(i,j)->ltID;

			//original landis4.0: int tempID = outsites->operator ()(i,j)->landUnit->ltID;

			//Changed By Qia on Oct 13 2008

			if ( tempID <= numFRU)
			{   
			    m_pFireSites.BefStChg(i,j);

				m_pFireSites(i,j).FRUIndex = tempID;

			    m_pFireSites.AftStChg(i,j);

			}
			else

				errorSys("fire regime unit is not consistent with the land unit.",STOP);
		}
	}	

}



long CFIRE::fireSize(float MFS,float STD)

{

	//generate random fire size based on lognormal distribution

	double size,VAR;	

	long  numSites,       //Number of sites.		  

		  siteSize;       //Square meters on a singular site.



	//if x is fire size following lognormal distribution with mean MFS and variance as VAR

	//then log(x) follows normal distribution with

	//mean = 2logMFS - 1/2log(VAR+MFS square)

	//variance = log(VAR+MFS square) - 2logMFS

	double mean,var,std;

	

	numSites=m_pFireSites.numRows()*m_pFireSites.numColumns();

	siteSize=m_fireParam.cellSize*m_fireParam.cellSize;	

	VAR = STD * STD;



	mean = 2.0 * log(MFS) - 0.5 * log(VAR + MFS*MFS);

	var = log(1.0*(VAR + MFS*MFS)) - 2.0 * log(MFS);

	std = sqrt(var);	



	while (1)  //standard disturbance

	{

		size = m_pStochastic->Normal(mean,std);

		if(_isnan(size)){

			size=mean;

			break;

			}

		size = exp(size);	//unit: hectare. 1 ha = 10,000 squre meters

		size = (long) 10000*size/siteSize;

		if((size <= numSites)&&(size >=1))	break;				

	}	

	return (long) size;

}



int CFIRE::disturb(int row, int col, int totArea, class MAP8 &m, FILE *logfile,

double x, int itr)

//This will activate a fire disturbance at row r and col c.  The size of

//the fire will be totArea where totArea is in number of pixels.

//The output map is contained in m and the output file is logfile.

//area is the total area consumed by fire

{

	LDPOINT p1, p2;

	MAP8 dist;						//This will be marked true if an area is disturbed.

	int      numSites=0,			//Number of sites damaged.

			 numCohorts=0,			//Number of cohorts damaged.

			 fireClass,				//Fire class (1-5).

			singleTry=0,

			stopFlag = 0,

			fireINTERV,			//fire interval

			numLU;					//number of landunits

	long 	 land[MAX_LANDUNITS];	//fire on different landunits	

	double siteProb; //verified new probability on each site

	



	long FS[MAX_LANDUNITS]; // fire size for each landunits

	int landOnFire[MAX_LANDUNITS]; // if fire on this landunits, it's 1, otherwise, 0

	int j,k;



	double dTSLF;//TimeSinceLastFire

	int nCKilled;


	numLU = m_pFireRegimeUnits.number();

	for (int t = 0; t<numLU; t++)

	{

		land[t]=0;

		landOnFire[t] = 0;

		FS[t] = 0;

	}





	pile.reset();          

	dist.dim(m_pFireSites.numRows(), m_pFireSites.numColumns());

	dist.fill(false);



	dist(row,col)=true;

	p1.x=col;

	p1.y=row;

	numCohorts = damage(p1, fireClass);

	m(row,col)=(unsigned char)(1+fireClass); 

		//J.Yang put the fireClass into the map



	int tempID = m_pFireSites(p1.y,p1.x).FRUIndex;



	land[tempID]++;

	landOnFire[tempID] = 1;

	FS[tempID] = fireSize(m_pFireRegimeUnits.operator ()(tempID)->m_fMFS,

			m_pFireRegimeUnits.operator ()(tempID)->m_fFireSTD);

	totArea = FS[tempID];

	numSites = 1;

	//now totArea means that the firesize for the 1st landtype(start point)



	if (p1.x-1>0 && !dist(p1.y,p1.x-1))

	{

		p2.x=p1.x-1;

		p2.y=p1.y;

		dist(p2.y,p2.x)=true;

		tempID = m_pFireSites(p2.y,p2.x).FRUIndex;

		if (m_pFireRegimeUnits.operator ()(tempID)->active())

			pile.push(p2);

	}



	if (p1.x+1<=m_pLAND->numColumns() && !dist(p1.y,p1.x+1))

	{

		p2.x=p1.x+1;

		p2.y=p1.y;

		dist(p2.y,p2.x)=true;

		tempID = m_pFireSites(p2.y,p2.x).FRUIndex;

		if (m_pFireRegimeUnits.operator ()(tempID)->active())

			pile.push(p2);

	}



	if (p1.y-1>0 && !dist(p1.y-1,p1.x))

	{

		p2.x=p1.x;

		p2.y=p1.y-1;

		dist(p2.y,p2.x)=true;

		tempID = m_pFireSites(p2.y,p2.x).FRUIndex;

		if (m_pFireRegimeUnits.operator ()(tempID)->active())

			pile.push(p2);

	}



	if (p1.y+1<=m_pLAND->numRows() && !dist(p1.y+1,p1.x))

	{

		p2.x=p1.x;

		p2.y=p1.y+1;

		dist(p2.y,p2.x)=true;

		tempID = m_pFireSites(p2.y,p2.x).FRUIndex;

		if (m_pFireRegimeUnits.operator ()(tempID)->active())

			pile.push(p2);

	}



	

	while (!pile.isEmpty() && ! stopFlag)

	{

		p1=pile.pull();

		singleTry ++;

		nCKilled = 0;	

		

		j = tempID = m_pFireSites(p1.y,p1.x).FRUIndex;



		fireINTERV= m_pFireRegimeUnits.operator ()(tempID)->fireInterval;	

		dTSLF = m_pPDP->sTSLFire[p1.y][p1.x];

		if (fireINTERV == 0)

			siteProb = 1;

		else

			siteProb= exp(dTSLF *((double) -1/fireINTERV));//10*m_pLAND->TimeStep; Bu, Rencang, April 18, 2009

		//modified siteProbability based on how much sites already burned

		if (landOnFire[j] == 1) //it's fire spreading

			//siteProb = exp(log(FS[j]- land[j] + 1)*log(siteProb));

			if (FS[j] > land[j]) siteProb = 0; else siteProb = 1;		

		else

		{

			//if ignitionChecking is out, siteProb = 1. it means that fire cannot be ignited

			if (m_lptrIgnitionOccurance[j] == 0) 

				siteProb = 1;

			else

			{	//consume one ignition

				//decrease m_lptrIgnitionOccurance[j] by 1 and related work			

				m_lptrIgnitionOccurance[j]-- ;			

				if (m_lptrIgnitionOccurance[j]==0) {

					for(k=0;k<m_iNumLU;k++)

						if (m_iptrValidLandUnits[k] == j) break;

					for (int i=k;i<m_iNumLU-1;i++)

						m_iptrValidLandUnits[i]= m_iptrValidLandUnits[i+1];

					m_iNumLU--;

				}

			}

		}



		if(siteProb > 1) 

		{

			printf("1 - fire spread probability (siteProb: %f) is larger than 1",siteProb);

			fflush(stdout); 

			siteProb = 1;

		}

		if(siteProb < 0) 

		{

			printf("1 - fire spread probability (siteProb: %f) is less than 0",siteProb);

			fflush(stdout); 

			siteProb = 0;

		}



		if (m_pStochastic->Bernoulli(siteProb)==0) 

		{// either fire spreading or fire initiation		

			nCKilled= damage(p1, fireClass);

			m(p1.y,p1.x)=(unsigned char)(1+fireClass); 

				//J.Yang put fireClass into the map



			land[j] ++;

			numSites ++;

			if (landOnFire[j] == 0) 

			{// an initiation				

				landOnFire[j] = 1;

				FS[j] = fireSize(m_pFireRegimeUnits.operator ()(j)->m_fMFS,

					m_pFireRegimeUnits.operator ()(j)->m_fFireSTD);								

			}

		}

		else pile.push(p1);

		numCohorts += nCKilled;

		if (nCKilled)

		{

			singleTry = 0;  		

			m(p1.y,p1.x)= (unsigned char)(1+fireClass);



			if (p1.x-1>0 && !dist(p1.y,p1.x-1))

			{

				p2.x=p1.x-1;

				p2.y=p1.y;

				dist(p2.y,p2.x)=true;

				tempID = m_pFireSites(p2.y,p2.x).FRUIndex;

				if (m_pFireRegimeUnits.operator ()(tempID)->active())

					pile.push(p2);

			}

			if (p1.x+1<=m_pLAND->numColumns() && !dist(p1.y,p1.x+1))

			{

				p2.x=p1.x+1;

				p2.y=p1.y;

				dist(p2.y,p2.x)=true;

				tempID = m_pFireSites(p2.y,p2.x).FRUIndex;

				if (m_pFireRegimeUnits.operator ()(tempID)->active())

					pile.push(p2);

			}

			if (p1.y-1>0 && !dist(p1.y-1,p1.x))

			{

				p2.x=p1.x;

				p2.y=p1.y-1;

				dist(p2.y,p2.x)=true;

				tempID = m_pFireSites(p2.y,p2.x).FRUIndex;

				if (m_pFireRegimeUnits.operator ()(tempID)->active())

					pile.push(p2);

			}

			if (p1.y+1<=m_pLAND->numRows() && !dist(p1.y+1,p1.x))

			{

				p2.x=p1.x;

				p2.y=p1.y+1;

				dist(p2.y,p2.x)=true;

				tempID = m_pFireSites(p2.y,p2.x).FRUIndex;

				if (m_pFireRegimeUnits.operator ()(tempID)->active())

					pile.push(p2);

			}    		

		}//end if nCKilled    

		stopFlag = 1;	

		for (k=0; k<numLU; k++)	

		{

			if(FS[k] > land[k]) {stopFlag = 0; break;}

		}

		if (singleTry > MAX_TRY) {stopFlag = 2; break;}

	}//end while



	//add to fire log file	

	LDfprintf0(logfile,"%d, ",itr*m_pLAND->TimeStep_Fire); 

	fflush(logfile);

	LDfprintf0(logfile,"%d,%d, %d, ",col,row,totArea);

	fflush(logfile);

	LDfprintf0(logfile, "%d, %d",numSites, numCohorts);

	for(int i = 0; i<numLU;i++)

		LDfprintf0(logfile, "%s %d", ",",land[i]);

	LDfprintf0(logfile, "%s", "\n");	

	fflush(logfile);



#ifdef __FIREDEBUG__

	LDfprintf0(txtFile,"%d\n",stopFlag);		

	LDfclose(txtFile);

#endif

	return numSites;

}



int CFIRE::damage(LDPOINT p, int &fireClass)

//This will cause damage from the fire at POINT p.  It will return the

//actual number of cohorts killed.  Class is the return value of the fire

//class.

//Note p.y is row, p.x is col



{
	//printf("(%d,%d)damage ",p.y,p.x);
	int i, j, count70;

	double tmpBiomass, tmpCarbon;

	int specAtNum = m_pSPECIESATTRS->number();

	int tempID = m_pFireSites(p.y, p.x).FRUIndex;

	//<Add By Qia on Aug 03 2009>

	float tmpDQ, TmpMortality, DeadTree, tmpBio, tmpBA, Total_Bio_onespec_allage, Total_BA_onespec_allage;
	int tmpDeadTree, Total_DeadTree_onespec_allage;

	SITE * siteptr = (*m_pLAND) (p.y, p.x);
	LANDUNIT *l;
	l = m_pLAND->locateLanduPt(p.y, p.x);
	tmpDQ = 0;

	/*for(i=1;i<=siteptr->number();i++)

		{

		if(siteptr->specAtt(i)->SpType>=0)

		for(j=1;j<=siteptr->specAtt(i)->longevity/m_pLAND->TimeStep;j++)

		{

		tmpDQ+=m_pLAND->GetGrowthRates(i,j)/30.48*m_pLAND->GetGrowthRates(i,j)/30.48/4*3.1415926/10000*m_pLAND->CellSize*m_pLAND->CellSize*siteptr->SpecieIndex(i)->getTreeNum(j,i);

		//tmpDQ+=0.005454*GetGrowthRates(i,j)/30.48*GetGrowthRates(i,j)/30.48*10000/CellSize/CellSize*siteptr->SpecieIndex(i)->getTreeNum(j,i);

		}

		}*/

	//</Add By Qia on Aug 03 2009>

	if (!m_pFireRegimeUnits.operator ()(tempID)->active())

		return 0;

	CFireRegimeUnit * pFRU = m_pFireRegimeUnits.operator ()(tempID);



	//Determine fire class.

	if (!m_fireParam.iFuelFlag || !(m_DLLMode& G_FUEL))

	{

		fireClass = 0;

		for (i = 4; i >= 0; i--)

		{

			if (pFRU->fireCurve[i] <= m_pPDP->sTSLFire[p.y][p.x])

				fireClass = max(fireClass, pFRU->fireClass[i]);

		}

	}

	else

	{

		fireClass = (int)m_pPDP->cFireIntensityClass[p.y][p.x];

	}

	// J.Yang need to consider when windDLL is turned on

	//how to simulate wind effect on fire damage	

	if (m_DLLMode & G_WIND)

	{

		if (m_pPDP->sTSLFire[p.y][p.x] > m_pPDP->sTSLWind[p.y][p.x])

		{

			for (i = 4; i >= 0; i--)

			{

				if (pFRU->windCurve[i] <= m_pPDP->sTSLWind[p.y][p.x])

					fireClass = max(fireClass, pFRU->windClass[i]);

			}

		}

	}

	if (fireClass <= 0)

		return 0;



	//Perform fire damage.   

	int numCohorts = 0;

	int shade = 0;

	//<Add By Qia on Nov 24 2008>

	m_pLAND->BefStChg(p.y, p.x);

	//</Add By Qia on Nov 24 2008>

	for (i = 1; i <= specAtNum; i++)

	{
		//printf("spec%d(%d,%d)\n", i,p.y,p.x);
		//<Add By Qia on Aug 03 2009>

		//siteptr->SpecieIndex(i)->TreesFromVeg=0;

		if (siteptr->specAtt(i)->SpType >= 0){
			Total_BA_onespec_allage = 0;
			Total_Bio_onespec_allage = 0;
			Total_DeadTree_onespec_allage = 0;

			for (j = 1; j <= siteptr->specAtt(i)->longevity / m_pLAND->TimeStep; j++){

				for (int k = 1; k <= m_pLAND->TimeStep; k++){

					//TmpMortality=0.01/(1+exp(3.25309-0.00072647*tmpDQ+0.01668809*m_pLAND->GetGrowthRates(i,j)/2.54));

					//DeadTree=siteptr->SpecieIndex(i)->getTreeNum(j,i)*TmpMortality/siteptr->specAtt(i)->shadeTolerance;
					int tolerance_index = siteptr->specAtt(i)->fireTolerance;

					int severity_index = fireClass;
					if (tolerance_index < 1 || tolerance_index>5){
						errorSys("fire tolerance index error.", STOP);
					}
					if (severity_index < 1 || severity_index>5){
						errorSys("fire severity index error.", STOP);
					}
					float beta1 = m_fireParam.fire_betavalues[tolerance_index - 1][0];
					float beta2 = m_fireParam.fire_betavalues[tolerance_index - 1][1];
					float beta3 = m_fireParam.fire_betavalues[tolerance_index - 1][2];
					float X_value = m_fireParam.fire_X2values[severity_index - 1];
					double prob_burn_param = beta1 + beta2*m_pLAND->GetGrowthRates(i, j, l->ltID) + beta3*X_value;
					double prob_burn = 1.0 / (1.0 + exp(0.0 - prob_burn_param));
					int tree_num_agecohort = siteptr->SpecieIndex(i)->getTreeNum(j, i);
					DeadTree = tree_num_agecohort*prob_burn;

					///////////////////////////
					/*tmpBiomass = exp(pfuelcoresites->GetBiomassData(siteptr->specAtt(i)->BioMassCoef, 1) + pfuelcoresites->GetBiomassData(siteptr->specAtt(i)->BioMassCoef, 2)
						*log(pfuelcoresites->GetGrowthRates(i, j, l->ltID)))*DeadTree / 1000.00;
						tmpBA = pfuelcoresites->GetGrowthRates(i, j, l->ltID) * pfuelcoresites->GetGrowthRates(i, j, l->ltID) / 40000 * 3.1415926* DeadTree;*/
					///////////////////////////

					//tmpDeadTree = tree_num_agecohort*prob_burn;
					int rest = tree_num_agecohort - DeadTree;
					tmpDeadTree = tree_num_agecohort - rest;
					tmpBA = m_pLAND->GetGrowthRates(i, j, l->ltID)*m_pLAND->GetGrowthRates(i, j, l->ltID) / 40000 * 3.1415926 *tmpDeadTree;
					tmpBio = exp(m_pLAND->GetBiomassData(siteptr->specAtt(i)->BioMassCoef, 1) + m_pLAND->GetBiomassData(siteptr->specAtt(i)->BioMassCoef, 2)*log(m_pLAND->GetGrowthRates(i, j, l->ltID)))*tmpDeadTree / 1000.00;

					Total_DeadTree_onespec_allage += tmpDeadTree;
					Total_BA_onespec_allage += tmpBA;
					Total_Bio_onespec_allage += tmpBio;

					//add by houbin 10/26/2017
					if (m_pPDP->TrackFlag == 1)
					{
						for (int agerange_index = 1; agerange_index <= m_pLAND->GetOutputAgeRangeModuleFlagArray(i - 1, COUNT); agerange_index++)
						{
							if (m_pLAND->GetOutputAgeRangeModuleFlagArray(i - 1, agerange_index, 0) < j*m_pLAND->TimeStep + k - 1 && j*m_pLAND->TimeStep + k - 1 <= m_pLAND->GetOutputAgeRangeModuleFlagArray(i - 1, agerange_index, 1))
							{
								m_pPDP->cDeadTreeNum_spec_agerange[p.y - 1][p.x - 1][(agerange_index - 1)*specAtNum + i - 1] += tmpDeadTree;
								m_pPDP->cDeadTreeBA_spec_agerange[p.y - 1][p.x - 1][(agerange_index - 1)*specAtNum + i - 1] += tmpBA;
								m_pPDP->cDeadTreeBio_spec_agerange[p.y - 1][p.x - 1][(agerange_index - 1)*specAtNum + i - 1] += tmpBio;
							}
						}
					}
					
					//add by houbin 10/26/2017

					siteptr->SpecieIndex(i)->setTreeNum(j, i, tree_num_agecohort - DeadTree);
					
					//printf("treenumber:%d %d %f %d\n", tree_num_agecohort, tmpDeadTree, DeadTree, rest);
					if (siteptr->specAtt(i)->maxSproutAge / m_pLAND->TimeStep >= j && siteptr->specAtt(i)->MinSproutAge / m_pLAND->TimeStep <= j)

					{

						siteptr->SpecieIndex(i)->TreesFromVeg += (int)DeadTree;

					}

				}
			}

		}
		if (m_pPDP->TrackFlag == 1)
		{
			m_pPDP->cDeadTreeNum[p.y - 1][p.x - 1][i - 1] = Total_DeadTree_onespec_allage;
			m_pPDP->cDeadTreeBA[p.y - 1][p.x - 1][i - 1] = Total_BA_onespec_allage;
			m_pPDP->cDeadTreeBio[p.y - 1][p.x - 1][i - 1] = Total_Bio_onespec_allage;
		}
			
			
		}

		m_pLAND->AftStChg(p.y, p.x);

		if (numCohorts)
			m_pPDP->sTSLFire[p.y][p.x] = 0;

		return numCohorts;

}



void CFIRE::ReadDEM(char *fileName)

{

	FILE * fp;

	fp = fopen(fileName, "r");

	int temp;

	if (fp==NULL) 

		errorSys("FIRE: DEM file not found.",STOP);

	for (int i = m_fireParam.rows; i>= 1; i--)

		for (int j = 1; j <= m_fireParam.cols; j++)

		{

			if (fscanc(fp,"%d",&temp) != 1) 

			{

				errorSys("FIRE: Error reading in topo value.",STOP);

			}

			
			    m_pFireSites.BefStChg(i,j);
				
				m_pFireSites(i,j).DEM = temp;

			    m_pFireSites.AftStChg(i,j);

		}

	



}



//Fire spread from ignition point

//Return the size of burnt area (number of sites burned)

//assume the logFire is a data memember of CFIRE

//Notice the value of row: 1 ...... rows

//				   of col: 1 ...... cols

long CFIRE::fireSpread(int row, int col)
{
	int numSites=1,		//Number of sites damaged.

		numCohorts=0,		//Number of cohorts damaged.

		numFRU= m_pFireRegimeUnits.number(), //Number of fire regime unit

		fireClass;		//Fire class (1-5).

	long* land = new long [numFRU]();	//fire on different landunits	
	long* FS = new long [numFRU](); // predefined fire size for each landunits
	int*  landOnFire = new int[numFRU](); // if fire on this landunits, it's 1, otherwise, 0

	int i,j,k;


	fireFront * frontTemp;//Add By Qia on Feb 10 2009

	long lLength1,lLength2;	

	long l;

	long MemoryAllocNum = 500;

	fireFront* front1 = new fireFront [MemoryAllocNum];
	fireFront* front2 = new fireFront [MemoryAllocNum];

	//the initiation point
	LDPOINT point;

	point.x = col;
	point.y = row;

	int tempID = m_pFireSites(row,col).FRUIndex;

	j = tempID;

	land[j]++;

	landOnFire[j] = 1;

	FS[j] = fireSize(m_pFireRegimeUnits.operator ()(j)->m_fMFS,

			m_pFireRegimeUnits.operator ()(j)->m_fFireSTD);			

	const int totArea = FS[j]; //Predefined fire size 

	DrawWindEvent();


	lLength1 = 1;

	front1[0].burnningTime = 1;

	front1[0].burnningLimit = 1;

	front1[0].row = row;

	front1[0].col = col;

	

	int tempRow,tempCol;

	lLength2 = 0;

	

	while(lLength1>0) 
	{
		for (long l =0;l<lLength1;l++) 
		{
			if (front1[l].burnningTime < front1[l].burnningLimit ) 
			{
				front2[lLength2].burnningLimit = front1[l].burnningLimit;

				front2[lLength2].burnningTime = front1[l].burnningTime+1;

				front2[lLength2].row = front1[l].row;

				front2[lLength2].col = front1[l].col;

				lLength2 ++;

				if(lLength2>=MemoryAllocNum)
				{

					printf("Fire memory size reallocate %d\n",MemoryAllocNum);

					frontTemp= new fireFront [MemoryAllocNum];

					memcpy(frontTemp,front2,sizeof(front2));

					delete [] front2;

					front2 = new fireFront [MemoryAllocNum*2];

					if(front2==NULL)

						errorSys("fire memory allocation fail\n",STOP);

					memcpy(front2,frontTemp,sizeof(frontTemp));



					memcpy(frontTemp,front1,sizeof(frontTemp));

					delete [] front1;

					front1 = new fireFront [MemoryAllocNum*2];

					if(front1==NULL)

						errorSys("fire memory allocation fail\n",STOP);

					memcpy(front1,frontTemp,sizeof(frontTemp));



					MemoryAllocNum=MemoryAllocNum*2;

					}

				//</Add By Qia on Feb 10 2009>

			}

			else 
			{
				//burnt out

				tempRow = front1[l].row; tempCol = front1[l].col;

				point.y = tempRow;

				point.x = tempCol;

				numCohorts += damage(point,fireClass);

				m_Map(tempRow,tempCol)=(unsigned char)(1+fireClass);


				for (int i =0; i<8; i++) 
				{
					switch (i) 
					{
						case 0: //SW

							tempRow = front1[l].row-1;tempCol = front1[l].col-1;							

							break;

						case 1: //W

							tempRow = front1[l].row;tempCol = front1[l].col-1;

							break;

						case 2: //NW

							tempRow = front1[l].row +1;tempCol = front1[l].col-1;

							break;

						case 3: //N

							tempRow = front1[l].row+1;tempCol = front1[l].col;

							break;

						case 4: //NE

							tempRow = front1[l].row+1;tempCol = front1[l].col+1;

							break;

						case 5:  //E

							tempRow = front1[l].row;tempCol = front1[l].col+1;

							break;

						case 6:  //SE

							tempRow = front1[l].row-1;tempCol = front1[l].col+1;

							break;

						case 7:  //S

							tempRow = front1[l].row-1;tempCol = front1[l].col;

							break;

						default:

							break;

					}					

				    //check its neighbors

					double fireSpreadProb;

					//isValid means the site is in the bound

					//and it is in an active site

					//and it is unburnt site

					//if the site is in a new fire regime unit even it is active

					//check the limit of fire occurrences for this FRU

					if (isValid(tempRow,tempCol,landOnFire)) 

					{						



						j = m_pFireSites(tempRow,tempCol).FRUIndex;

						CalculateWind(i);

						CalculateSlope(front1[l].row, front1[l].col,tempRow,tempCol);

						CalculateFSFactor(land[j],FS[j]);						



						fireSpreadProb = modifiedSpreadProb(tempRow,tempCol);//10*m_pLAND->TimeStep; Bu, Rencang, April 28, 2009;

						if (fireSpreadProb > 1) 

						{

							printf("fire spread probability (fireSpreadProb: %f) is larger than 1",fireSpreadProb);

							printf(" at site %d %d \n", tempRow,tempCol);

							fflush(stdout); 

							fireSpreadProb = 1;

						}

						if (fireSpreadProb <0) 

						{

							printf("fire spread probability (fireSpreadProb: %f) is less than 0",fireSpreadProb);

							printf(" at site %d %d \n", tempRow,tempCol);

							fflush(stdout); 

							fireSpreadProb = 0;

						}

						if(m_pStochastic->Bernoulli(fireSpreadProb) ==1) {

							//burnning site

							front2[lLength2].row = tempRow;

							front2[lLength2].col = tempCol;

							front2[lLength2].burnningTime = 1;

							front2[lLength2].burnningLimit = 1;

							lLength2 ++;

							//<Add By Qia on Feb 10 2009>

							if(lLength2>=MemoryAllocNum){

								printf("Fire memory size reallocate %d\n",MemoryAllocNum);

								frontTemp= new fireFront [MemoryAllocNum];

								memcpy(frontTemp,front2,sizeof(front2));

								delete [] front2;

								front2 = new fireFront [MemoryAllocNum*2];

								if(front2==NULL)

									errorSys("fire memory allocation fail\n",STOP);

								memcpy(front2,frontTemp,sizeof(frontTemp));



								memcpy(frontTemp,front1,sizeof(frontTemp));

								delete [] front1;

								front1 = new fireFront [MemoryAllocNum*2];

								if(front1==NULL)

									errorSys("fire memory allocation fail\n",STOP);

								memcpy(front1,frontTemp,sizeof(frontTemp));



								MemoryAllocNum=MemoryAllocNum*2;

								}

							//</Add By Qia on Feb 10 2009>

							//change map value for this site

							m_Map(tempRow,tempCol) = 2; //0 not active; 1 no fire ; 2 fire class = 1 (lowest)

							//increase fire size (numSites) w.r.t. fire regime unit

							land[j] ++;					

							numSites ++;

							if (landOnFire[j] == 0) 

							{// an initiation				

								landOnFire[j] = 1;

								FS[j] = fireSize(m_pFireRegimeUnits.operator ()(j)->m_fMFS,

									m_pFireRegimeUnits.operator ()(j)->m_fFireSTD);								

								m_lptrIgnitionOccurance[j]-- ;			

								if (m_lptrIgnitionOccurance[j]==0)

								{

									for(k=0;k<m_iNumLU;k++)

										if (m_iptrValidLandUnits[k] == j) break;

									for (int i=k;i<m_iNumLU-1;i++)

										m_iptrValidLandUnits[i]= m_iptrValidLandUnits[i+1];

									m_iNumLU--;
								}	
							}//end of an initiation
						}

					} // end one neighbor

				}// end for looping in 8 neighbors		

			} //end else
		} //end for (looping in the front1)


		lLength1 = lLength2;

		if(lLength2 >0) 
		{						

			//copy elt from front2 to front 1;

			for(long l=0;l<lLength2;l++) 
			{

				front1[l].burnningLimit = front2[l].burnningLimit;

				front1[l].burnningTime = front2[l].burnningTime;

				front1[l].row = front2[l].row;

				front1[l].col = front2[l].col;

			}

			lLength2 =0;

		 }



	} //end while

	delete front1;

	delete front2;



	//add to fire log file	

	LDfprintf0(m_LogFP,"%d, ",m_itr*m_pLAND->TimeStep_Fire);

	LDfprintf0(m_LogFP,"%d, %d, %d, ",col,row,totArea);

	LDfprintf0(m_LogFP, "%d, %d",numSites, numCohorts);

	for(i = 0; i<numFRU;i++)
		LDfprintf0(m_LogFP, "%s %d", ",",land[i]);

	LDfprintf0(m_LogFP, "%s", "\n");	

	fflush(m_LogFP);


	delete[] land;
	delete[] FS;
	delete[] landOnFire;

	return numSites;
}



//Notice the value of row: 1 ...... rows

//				   of col: 1 ...... cols

int CFIRE::isValid(int row, int col, int * landOnFire)

{

	//check map boundary

	//check map value (0 not active, 1 no fires yet, 2 burnning or fire class is 1)

	//check the site is in an active FRU (Fire Regime Unit)

	//check the site is in a new FRU which still allows new initiations

	

	if (row <1 || row > m_fireParam.rows || col < 1 || col > m_fireParam.cols) 

		return 0;

	if (m_Map(row,col) > 1)

		return 0;

	//if (!m_pFireSites(row,col)->pFireRegimeUnit->active())

	//	return 0;

	

	

	int j = m_pFireSites(row,col).FRUIndex;

	if (landOnFire[j] == 0) //there is no fire spreading at this FRU		

		if (m_lptrIgnitionOccurance[j] == 0) 

				return 0;

		

	return 1;



}



double CFIRE::modifiedSpreadProb(int row, int col)

{

	double ret = 0.0;

	double k = 0.0; 

	k = log(1-m_fireParam.fSpreadProb)/(-3);

	if (m_DLLMode & G_FUEL)

	{

		ret = 1-exp( -1 * 

			pow( (1.0f + m_fireParam.fCoeff1), m_fWind) *

			pow( (1.0f + m_fireParam.fCoeff2), m_fSlope) * 

			pow( (double)(1.0f + m_fireParam.fCoeff3), m_dFSFactor) * 

			k * (int) m_pPDP->cFireIntensityClass[row][col]

			);

	}

	else

	{

		ret = 1-exp( -1 * 

			pow( (1 + m_fireParam.fCoeff1), m_fWind) *

			pow( (1 + m_fireParam.fCoeff2), m_fSlope) * 

			pow( (double)(1 + m_fireParam.fCoeff3), m_dFSFactor) * 

			k *1

			);		

	}

	return ret;



}





/************** The Coordination System in LANDIS ***************/

/* (500,1) (500,2) (500,3) ..........(500,500)

/* (499,1) (499,2) (499,3) ..........(499,500)

/* ...........................................

/* ...........................................

/* (1,1)   (1,2)   (1,3) .............(1,500)

For a 500 x 500 map

*/



void CFIRE::CalculateWind(int index)

{

	//index is the direction along fire is spreading into

	//prevailing wind index and prevailing wind intensity is

	//	reserved in m_fireParam

	//simulated wind index and intensity will be done later



	m_fWind = 0.0f;

	if (m_fireParam.iWindFlag == 0)

		return;



	/* draw a wind event

	*/

	//comment it, move it to the begining of fire spread

	//otherwise, wind direction and speed is actually 

	//simulated at each cell burning time rather than 

	//at each fire ignition time.

	//DrawWindEvent();

	if (m_iWindIntensity == 0)

		return;

	int diff;

	diff = abs(m_iWindDirectionIndex - index);

	switch (diff) {

	case 0:

		m_fWind = 0.2f;

		break;

	case 1:

		m_fWind = 0.1f;

		break;

	case 7:

		m_fWind = 0.1f;

		break;

	case 2:

		m_fWind = 0.0f;

		break;

	case 6:

		m_fWind = 0.0f;

		break;

	case 3:

		m_fWind = -0.1f;

		break;

	case 5:

		m_fWind = -0.1f;

		break;

	case 4:

		m_fWind = -0.2f;

		break;

	default:

		m_fWind = 0.0f;

		break;

	}

	m_fWind = m_fWind * m_iWindIntensity;



}





void CFIRE::CalculateSlope(int row1, int col1, int row2, int col2)
{
	m_fSlope = 0.0;

	if(m_fireParam.iDEMFlag)
	{
		double diff = m_pFireSites(row2,col2).DEM - m_pFireSites(row1,col1).DEM;

		m_fSlope = (float) atan(diff/m_fireParam.cellSize);
	}
}



void CFIRE::CalculateFSFactor(long lCurrentFS, long lFS)

{

	if (lFS > 0)

		m_dFSFactor = 1 - (double) 2 * lCurrentFS/lFS;

	else

		m_dFSFactor = 0;

}


void CFIRE::updateFire_Regime_Map(int i){
	char FireRegimeNametemp[100];
	char FireMapGIS[100];
	if(i/m_pLAND->TimeStep_Fire==1){
		FILE * FpFireupdate;
		Fire_regime_files.clear();
		Fire_regime_gisfiles.clear();
		if ((FpFireupdate=LDfopen(m_fireParam.File_regime_change, 1))==NULL){
			printf("fire update file: %s\n", m_fireParam.File_regime_change);
			errorSys("Can not open fire update file",STOP);
		}
		int num_of_files;
		fscanf(FpFireupdate,"%d",&num_of_files);
		//while(!feof(FpFireupdate))
		for(int ii_count_num = 0; ii_count_num<num_of_files;ii_count_num++){
			if (fscanc(FpFireupdate,"%s %s",FireMapGIS, FireRegimeNametemp)!=1)
				errorSys("Error reading in fire regime updating file\n",STOP);
			Fire_regime_files.push_back(string(FireRegimeNametemp));
			Fire_regime_gisfiles.push_back(string(FireMapGIS));
		}
		LDfclose(FpFireupdate);

		int index = i/m_pLAND->TimeStep_Fire-1;
		if(index < Fire_regime_files.size()){
			sprintf(FireRegimeNametemp,"%s", Fire_regime_files.at(index).c_str());
			sprintf(FireMapGIS,"%s", Fire_regime_gisfiles.at(index).c_str());
			FILE *luFile;
			strcpy(m_fireParam.fireRegimeAttrFn,FireRegimeNametemp);
			if ((luFile=LDfopen(m_fireParam.fireRegimeAttrFn, 1))==NULL){
				printf("Fire regime file %s not found.\n",m_fireParam.fireRegimeAttrFn);		
				errorSys(m_fireParam.fireRegimeAttrFn,STOP);
			}
			else{
				//update landtype attribute
				m_pFireRegimeUnits.read(luFile);
				LDfclose(luFile);
			}
			
			printf("\Fire regime parameter updated.\n");
			FILE *GISmap;
			strcpy(m_fireParam.fireRegimeDataFn,FireMapGIS);
			if ((GISmap=LDfopen(m_fireParam.fireRegimeDataFn, 2))==NULL){
				printf("File regime map file %s not found.\n",m_fireParam.fireRegimeDataFn);		
				errorSys(m_fireParam.fireRegimeDataFn,STOP);
			}
			else{
				//update landtype attribute
				m_pFireRegimeUnits.readFireRegimeGIS(GISmap); 
				LDfclose(GISmap);
			}
			printf("\nFire Regime map Updated.\n");
		}
	}
	if(i/m_pLAND->TimeStep_Fire>1){
		int index = i/m_pLAND->TimeStep_Fire-1;
		if(index < Fire_regime_files.size()){
			sprintf(FireRegimeNametemp,"%s", Fire_regime_files.at(index).c_str());
			sprintf(FireMapGIS,"%s", Fire_regime_gisfiles.at(index).c_str());
			FILE *luFile;
			strcpy(m_fireParam.fireRegimeAttrFn,FireRegimeNametemp);
			if ((luFile=LDfopen(m_fireParam.fireRegimeAttrFn, 1))==NULL){
				printf("File regime map file %s not found.\n",m_fireParam.fireRegimeAttrFn);		
				errorSys(m_fireParam.fireRegimeAttrFn,STOP);
			}
			else{
				//update landtype attribute
				m_pFireRegimeUnits.read(luFile);
				LDfclose(luFile);
			}
			
			printf("\nFire regime parameter updated.\n");
			FILE *GISmap;
			strcpy(m_fireParam.fireRegimeDataFn,FireMapGIS);
			if ((GISmap=LDfopen(m_fireParam.fireRegimeDataFn, 2))==NULL){
				printf("Land Map file %s not found.\n",m_fireParam.fireRegimeDataFn);		
				errorSys(m_fireParam.fireRegimeDataFn,STOP);
			}
			else{
				//update landtype attribute
				m_pFireRegimeUnits.readFireRegimeGIS(GISmap); 
				LDfclose(GISmap);
			}
			printf("\nFire Regime map Updated.\n");		
		}
	}
}


void CFIRE::updateFRU(int itr)

{

	FILE * fp;

	char str[255],iterString[5];	

	itoa(itr,iterString,m_pLAND->TimeStep_Fire);



	strcpy(str,m_fireParam.fireRegimeAttrFn);	

	strcat(str,iterString);



	fp = LDfopen(str,1);

	if (fp==NULL) 

	{

		printf("Fire regime attribute file %s not found.\n",str);

		errorSys("FIRE: fire regime attribute can not be updated.",STOP);

	}

	m_pFireRegimeUnits.read(fp);

	LDfclose(fp);



	if (m_fireParam.iFireRegimeFlag != 0)

	{

		strcpy(str,m_fireParam.fireRegimeDataFn);	

		strcat(str,iterString);

		fp = LDfopen(str, 1);

		if (fp==NULL) 

		{

			printf("Fire regime GIS file %s not found.\n",str);

			errorSys("FIRE: fire regime GIS can not be updated.",STOP);

		}

		m_pFireRegimeUnits.readFireRegimeGIS(fp); 

		LDfclose(fp);

	}

	else 

	{

		AttachLandUnitGIS(m_pLAND);

	}

	//j.Yang check this

	m_pFireRegimeUnits.dispatch();	

	m_pFireRegimeUnits.updateIGDensity(m_fireParam.cellSize);

}



void CFIRE::DrawWindEvent()

{

	/* 

	generate a number that follows uniform distribution 

	bw. 0 -- 100, then compare it with the frqeuncy distributions

	if it is less than Percentage of NonWindEvents, then we write no wind

	otherwise, it is a wind event

	Do similar thing to decide wind class

	*/

	m_iWindEventsLog[41] ++;

	float z;

	z = (float) m_pStochastic->Random() * 100;

	if (z <= m_fireParam.iNonWindPercent)

	{

		m_iWindIntensity = 0;

		m_iWindDirectionIndex = 0; //any thing, doesn't matter

		m_iWindEventsLog[0] ++;

		return;

	}

	z = (float) m_pStochastic->Random() * 100;

	for (int i = 0; i< 40; i++)

	{

		if (z <= m_fireParam.iCummWindClass[i])

		{

			//it is i'th class 

			m_iWindDirectionIndex = (int)((float)floor((float)i/5));

			m_iWindIntensity = (i % 5) + 1;

			m_iWindEventsLog[i+1] ++;

			return;

		}

	}

	

}



void CFIRE::PrintWindLog()

{	

	

	if (m_fireParam.iWindFlag == 0)

		return;

	

	int percentage[41];

	int i;	

	if (m_iWindEventsLog[41] == 0)

		percentage[0] = 0;

	else

		percentage[0] = 100 * m_iWindEventsLog[0]/m_iWindEventsLog[41];

	int temp;

	temp = m_iWindEventsLog[41] - m_iWindEventsLog[0];	

	for (i = 1; i< 41; i++)

	{

		if (temp <= 0)

			percentage[i] = 0;

		else

			percentage[i] = 100 * m_iWindEventsLog[i] / temp;

	}	

	printf("There are %d fire events simulated in this iteraion\t",m_iWindEventsLog[41]);

	printf("Among them, there are %d (%d percent) non-wind events\n",m_iWindEventsLog[0],percentage[0]);

	printf("Number of wind class simulated and its respective percentage of wind events are:\n");

	for (i = 1; i< 41; i++)

	{

		if ( (i % 5) == 0)

		{

			printf("%5d (%2d)\n",m_iWindEventsLog[i],percentage[i]);

		}

		else

		{

			printf("%5d (%2d)\t",m_iWindEventsLog[i],percentage[i]);

		}

	}	

	fflush(stdout);	

}



void CFIRE::WriteTSLF(int snr, int snc, int itr, double wAdfGeoTransform[])

{

	

	int i,j;

	char str[255];

	if (m_fireParam.iTSLFFlag)

		{

			/*

			FILE * tslfFile;

			sprintf(str,"%s/tslf%d.txt",parameters.outputDir,itr*10);

			if ((tslfFile=fopen(str,"a"))==NULL)

				errorSys("Error opening time-since-last-fire file",STOP);

			*/

			class MAP8 m;

			m.dim(snr,snc);

			for (i=1;i<=snr;i++)

				for (j=1;j<=snc;j++)

				{

					int tempID = m_pFireSites(i,j).FRUIndex;				

					if (m_pFireRegimeUnits.operator ()(tempID)->active())			

						m(i,j)= m_pPDP->sTSLFire[i][j]/10 ;			

					else

						m(i,j)= 255;							

				

					//fprintf(tslfFile,"%d ", m(i,j));

					//if (j==snc) fprintf(tslfFile, "\n");

				}

				//fclose(tslfFile);

				sprintf(str,"Time-since-last-fire for year %d.",itr*m_pLAND->TimeStep_Fire); 

				m.setHeader(m_pLAND->getHeader());

				m.rename(str);

				for(i=0;i<maxLeg;i++)

				{

					sprintf(str,"%d year",i*m_pLAND->TimeStep_Fire);

					m.assignLeg(i,str);

				}		

				m.assignLeg(255,"Not Active");

				sprintf(str,"%s%d",m_strTSLF,itr*m_pLAND->TimeStep_Fire);

				m.setCellSize(m_fireParam.cellSize);

				m.write(str, red, green, blue, wAdfGeoTransform);

		}



}



void CFIRE::FinneyInitilization()

{

	m_fuelCostMap = (float **) new float *[m_iMapRow+1];

	m_windCostMap = (float **) new float *[m_iMapRow+1];

	m_minTimeMap = (float **) new float *[m_iMapRow+1];

	m_checkMap = (int **) new int *[m_iMapRow+1];

	for (int i=0;i<=m_iMapRow;i++) 
	{			
		m_fuelCostMap[i]= (float *) new float[m_iMapColumn+1]();		
		m_windCostMap[i]= (float *) new float[m_iMapColumn+1]();		
		m_minTimeMap[i] = (float *) new float[m_iMapColumn+1]();		
		m_checkMap[i]   = (int *)   new int  [m_iMapColumn+1]();
	}	

	m_actualROSMap = m_windCostMap; //m_actualROSMap is identical to m_windCostMap. Just different name

}



void CFIRE::FinneyFreeMemory()

{

	for (int i = 0; i <= m_iMapRow; i++) 

	{

		delete m_fuelCostMap[i];

		delete m_windCostMap[i];

		delete m_minTimeMap[i];

		delete m_checkMap[i];

	}

	delete m_fuelCostMap;

	delete m_windCostMap;

	delete m_minTimeMap;

	delete m_checkMap;

}



void CFIRE::FinneyCalculateDimensions()

{

	

	//m_fRatio = m_iWindIntensity * 0.195 + 0.885;

	m_fRatio = m_finneyParam.ellipseAxisRatio[m_iWindIntensity];



	//major axis divided by minor axis

	//Priliminary equation relating wind serverity to ellipse shape

	/*

	Ratio = Scalar(WindStr * 0.2 + 1)  	

	A = Scalar(SQRT((Area div PI) div Ratio))  // A = minor axis length

	B = Scalar (A * Ratio) // B = major axis length

	C = Scalar (SQRT(B * B - A * A))

	*/

	//fire size unit is hectare, change it to meter square. 1 hectare = 10000 meter squar

	//A is flanking spread rate meter/day assuming the fire size is reached within one day

	//B + C is forward maximum spread rate (when Theta = 0)

	//C is the offset from the center of the ellipse to the ignition point

	m_fA = m_fRatio - sqrt(m_fRatio * m_fRatio -1);

	m_fB = m_fA * m_fRatio;

	m_fC = 1 - m_fB;



}



float CFIRE::fireSizeHa(float MFS, float STD)

{

	//generate random fire size based on lognormal distribution

	double size,VAR;	



	//if x is fire size following lognormal distribution with mean MFS and variance as VAR

	//then log(x) follows normal distribution with

	//mean = 2logMFS - 1/2log(VAR+MFS square)

	//variance = log(VAR+MFS square) - 2logMFS

	double mean,var,std;		

	VAR = STD * STD;



	mean = 2.0 * log(MFS) - 0.5 * log(VAR + MFS*MFS);

	var = log(1.0*(VAR + MFS*MFS)) - 2.0 * log(MFS);

	std = sqrt(var);	

	size = m_pStochastic->Normal(mean,std);

	size = exp(size);	//unit: hectare. 1 ha = 10,000 squre meters

	return (float) size;



}



long CFIRE::FinneySpread()

{

	

	DrawWindEvent();

	int numFRU = m_pFireRegimeUnits.number();

	InitilizeBurnedCells();

	int tempID = m_pFireSites(m_iOriginRow, m_iOriginColumn).FRUIndex;

	if (m_fireParam.iFuelFlag == 2)

	{

		m_fPredefinedDuration = fireDuration(m_pFireRegimeUnits.operator ()(tempID)->m_fMFS,

			m_pFireRegimeUnits.operator ()(tempID)->m_fFireSTD); 

	}

	else if (m_fireParam.iFuelFlag == 3)

	{

		m_lPredefinedFireSize = fireSize(m_pFireRegimeUnits.operator ()(tempID)->m_fMFS,

			m_pFireRegimeUnits.operator ()(tempID)->m_fFireSTD); 

	}

	

	FinneyCalculateDimensions();

	FinneyCalculateFuelCost();

	FinneyEucDirection();

	FinneyCalculateWindCost();

	FinneyCalculateFinalCost();

	FinneyCalculateMinTime();

	



	//add to fire log file	

	LDfprintf0(m_LogFP,"%d, ",m_itr*m_pLAND->TimeStep_Fire);

	fflush(m_LogFP);

	char str[256]; 

	if (m_fireParam.iFuelFlag == 2)

		sprintf(str,"%f",m_fPredefinedDuration);

	else if (m_fireParam.iFuelFlag == 3)

		sprintf(str,"%ld",m_lPredefinedFireSize);

	LDfprintf0(m_LogFP,"%d, %d, %s, ",m_iOriginColumn,m_iOriginRow,str);

	fflush(m_LogFP);

	LDfprintf0(m_LogFP, "%d, %d",length_BurnedCells, FinneynumCohorts);

	for(int i = 0; i<numFRU;i++)

		LDfprintf0(m_LogFP, "%s %d", ",",burnedCells[i]);

	LDfprintf0(m_LogFP, "%s", "\n");	

	fflush(m_LogFP);

	return length_BurnedCells;



}



void CFIRE::FinneyCalculateFuelCost()

//function name should be calculateSpreadRate() instead in this version

{

	int iRow = m_iMapRow, iCol = m_iMapColumn;

	int i, j;

	

	if (0 == m_fireParam.iDEMFlag)

	{

		for (i = 1; i <= iRow; i++)

		{

			for (j = 1; j<= iCol; j++)

			{

				int tempFuelClass;

				tempFuelClass = m_pPDP->cFireIntensityClass[i][j];

				m_fuelCostMap[i][j] = m_finneyParam.spreadRate[tempFuelClass][m_iWindIntensity];

			}	

		}

	}

	else 

	{

		int tempSlope;

		for (i = 1; i <= iRow; i++)

		{

			for (j = 1; j<= iCol; j++)

			{

				tempSlope = m_pFireSites(i,j).DEM;

				int tempFuelClass;

				tempFuelClass = m_pPDP->cFireIntensityClass[i][j];

				if (0 == tempSlope) 

					m_fuelCostMap[i][j] = m_finneyParam.spreadRate[tempFuelClass][m_iWindIntensity];

				if (1 == tempSlope) 

					m_fuelCostMap[i][j] = m_finneyParam.ROS_low[tempFuelClass][m_iWindIntensity];

				if (2 == tempSlope) 

					m_fuelCostMap[i][j] = m_finneyParam.ROS_moderate[tempFuelClass][m_iWindIntensity];

				if (3 == tempSlope) 

					m_fuelCostMap[i][j] = m_finneyParam.ROS_high[tempFuelClass][m_iWindIntensity];

				if (4 == tempSlope) 

					m_fuelCostMap[i][j] = m_finneyParam.ROS_extreme[tempFuelClass][m_iWindIntensity];
			}
		}
	}

}



/* this function is different from EucDirection in my fireTravel program

because they are using different coordination system

*/

void CFIRE::FinneyEucDirection()

{

	int iRow = m_iMapRow, iCol = m_iMapColumn;

	int i,j;



	/* transform

	(x,y) -->(x',y')

	where (x,y) is from the coordination system

	(row,1).......(row,col)

	:

	(1,1).........(1,col)

	and (x',y') is from 

	(0,0)........(0,col-1)

	:

	(row-1,0).....(row-1,col-1)



	then 

	x' = row - x; y' = y -1;

	*/



	

	int originRowTrans = iRow - m_iOriginRow;

	int originColTrans = m_iOriginColumn-1;





	for (i = 0; i < iRow; i++)

	{

		for (j = 0; j<iCol; j++)

		{

			int distRow, distColumn;

			int iTrans = iRow - i;

			int jTrans = j + 1;



			distRow = i - originRowTrans; //difference in row //note m_iOriginRow starts from 1 rather than 0

			distColumn = j - originColTrans; //difference in column 

			if (distRow != 0)

			{ 

				if (distRow > 0) //South

				{

					if (distColumn <= 0) //SW 180-270

						m_windCostMap[iTrans][jTrans] = (-1* atan( (double) distColumn/distRow)* 180/PI) + 180;

					else //SE 90-180

						m_windCostMap[iTrans][jTrans] = 180 - (atan( (double) distColumn/distRow)* 180/PI);

				}

				else //North

				{

					if (distColumn <= 0) //NW 270 -360

						m_windCostMap[iTrans][jTrans] = 360 - (atan( (double) distColumn/distRow)* 180/PI);

					else //NE 0 - 90

						m_windCostMap[iTrans][jTrans] = (-1 * atan( (double) distColumn/distRow)* 180/PI);

				}

			}

			else //distRow == 0, either E(90) or W(270)

			{

				if (distColumn < 0)

					m_windCostMap[iTrans][jTrans] = 270; //W

				if (distColumn > 0)

					m_windCostMap[iTrans][jTrans] = 90; //E

				if (distColumn == 0)

					m_windCostMap[iTrans][jTrans] = 0; //reserved for origin point

			}

		}	

	}

	/*

	for (i = iRow; i >=1 ; i--)

	{

		for (j = 1; j<= iCol; j++)

		{

			int distRow, distColumn;



			distRow = i - m_iOriginRow; //difference in row //note m_iOriginRow starts from 1 rather than 0

			distColumn = j - m_iOriginColumn; //difference in column 

			if (distRow != 0)

			{ 

				if (distRow > 0) //South

				{

					if (distColumn <= 0) //SW 180-270

						m_windCostMap[iTrans][jTrans] = (-1* atan( (double) distColumn/distRow)* 180/PI) + 180;

					else //SE 90-180

						m_windCostMap[iTrans][jTrans] = 180 - (atan( (double) distColumn/distRow)* 180/PI);

				}

				else //North

				{

					if (distColumn <= 0) //NW 270 -360

						m_windCostMap[iTrans][jTrans] = 360 - (atan( (double) distColumn/distRow)* 180/PI);

					else //NE 0 - 90

						m_windCostMap[iTrans][jTrans] = (-1 * atan( (double) distColumn/distRow)* 180/PI);

				}

			}

			else //distRow == 0, either E(90) or W(270)

			{

				if (distColumn < 0)

					m_windCostMap[iTrans][jTrans] = 270; //W

				if (distColumn > 0)

					m_windCostMap[iTrans][jTrans] = 90; //E

				if (distColumn == 0)

					m_windCostMap[iTrans][jTrans] = 0; //reserved for origin point

			}

		}	

	}

    */

	if (__DEBUG && m_FinneyDebugOutput <=3 )

	{

		//output the wind cost map to c:\temp\EucDirection.txt

		FILE * fp;

		fp = fopen("c:\\temp\\EucDirection.txt","w");

		for (i = iRow; i >= 1; i--)

		{

			for (j = 1; j<= iCol; j++)

			{

				fprintf(fp,"%3.0f ",m_windCostMap[i][j]);

			}

			fprintf(fp,"\n");

		}

		fclose(fp);

		m_FinneyDebugOutput ++;



	}



}



void CFIRE::FinneyCalculateWindCost()

{

	// if iWindServerity == 0 then all wind cost is 1

	// else update windCost for each cell

	// what about the origin cell?

	// how to incorporate wind serverity in this wind cost calculation? do we need it?

	int iRow = m_iMapRow, iCol = m_iMapColumn;

	int i,j;



	int degree;

	m_maxWindRate = 0;



	if (m_iWindIntensity != 0)

	{

		switch (m_iWindDirectionIndex)

		{

		case 0:

			//SW

			degree = 225;

			break;

		case 1:

			//W

			degree = 270;

			break;

		case 2:

			//NW

			degree = 315;

			break;

		case 3:

			//N

			degree = 360;

			break;

		case 4: //NE

			degree = 45;

			break;

		case 5:

			//E

			degree = 90;

			break;

		case 6:

			//SE

			degree = 135;

			break;

		case 7:

			degree = 180;

			break;

		default:

			degree = 0;

			break;

		}



		for (i = iRow; i >= 1; i--)

		{

			for (j = 1; j<= iCol; j++)

			{

				//int iTrans = iRow -i;

			    //int jTrans = j 1 1;



				double Beta;

				//Beta = (m_windCostMap[i][j] - 180 - m_parameterSet.iWindDirection) /360;

				/*Brian's version counts for the fact that ArcGIS EucDirection is used to 

				calculate the direction to the ignition rather than from igniton

				*/

				

				Beta = (m_windCostMap[i][j] - degree) * PI /180;



				double sqA, sqB,sqC,CosBeta,SinBeta,sqCosBeta,sqSinBeta;

				sqA = m_fA * m_fA;

				sqB = m_fB * m_fB;

				sqC = m_fC * m_fC;

				CosBeta = cos(Beta);

				SinBeta = sin(Beta);

				sqCosBeta = CosBeta * CosBeta;

				sqSinBeta = SinBeta * SinBeta;

				double Theta=0.0;

				Theta = acos( (m_fA * CosBeta * sqrt(sqA * sqCosBeta + (sqB - sqC) * sqSinBeta) 

					- (m_fB * m_fC * sqSinBeta) ) / (sqA * sqCosBeta + sqB * sqSinBeta) );

				double Rate = 1;

				Rate = (m_fA * (m_fC * cos(Theta) + m_fB)) / 

					(sqrt( (sqA * cos(Theta) * cos(Theta)) + (sqB * sin(Theta) * sin(Theta) )));			

				//  Rate is equal to distance covered in 1 burning day

				if (m_maxWindRate <Rate)

					m_maxWindRate = Rate;

				m_windCostMap[i][j] = Rate; //now it is rate, rather than cost

			}

		}

	}

	else

	{

		m_maxWindRate = 1;

		for (i = iRow; i >= 1; i--)

		{

			for (j = 1; j<= iCol; j++)

			{

				m_windCostMap[i][j] = 1;

			}

		}

	}



	if (__DEBUG && m_FinneyDebugOutput <=3 )

	{

		//output the wind cost map to c:\temp\windcostmap.txt

		FILE * fp;

		fp = fopen("c:\\temp\\WindIndexMap.txt","w");

		for (i = iRow; i > 0; i--)

		{

			for (j = 1; j<= iCol; j++)

			{

				fprintf(fp,"%f ",m_windCostMap[i][j]);

			}

			fprintf(fp,"\n");

		}

		fclose(fp);

		m_FinneyDebugOutput ++;



	}



}



//The funcion below outputs rate instead of cost now. J. Yang, 09/04/2006

void CFIRE::FinneyCalculateFinalCost()

{

	int iRow = m_iMapRow, iCol = m_iMapColumn;

	int i,j;

	float WindIndex;

	float FinalFuelRate;

	float FinalWindIndex;

	for (i = 1; i <= iRow; i++)

	{

		for (j = 1; j<= iCol; j++)

		{

			if (m_fuelCostMap[i][j] > 0)

			{

				WindIndex = m_windCostMap[i][j]/m_maxWindRate;

				FinalFuelRate = pow(m_fuelCostMap[i][j], m_finneyParam.fuelWeight);

				FinalWindIndex =  pow(WindIndex, m_finneyParam.windWeight);

				FinalFuelRate = FinalFuelRate * FinalWindIndex;

				m_fuelCostMap[i][j] = FinalFuelRate;

				m_windCostMap[i][j] = 0.0; //it will be used as the actual ROS in the CalculateMinTime() fn.

										// note that m_windCostMap and m_actualROSMap are the same



			}

			else

				m_windCostMap[i][j] = 0.0;



		}

	}

	if (__DEBUG && m_FinneyDebugOutput <= 4 )

	{

		//output final cost

		FILE * fp;

		fp = fopen("c:\\temp\\finalcost.txt","w");

		for (i = iRow; i > 0; i--)

		{

			for (j = 1; j<= iCol; j++)

			{

				if (m_fuelCostMap[i][j] != FLT_MAX) 

					fprintf(fp,"%f ",m_fuelCostMap[i][j]);

				else

					fprintf(fp,"%f ",0.0f);



			}

			fprintf(fp,"\n");

		}

		fclose(fp);

		m_FinneyDebugOutput ++;



	}

}



void CFIRE::FinneyCalculateMinTime()

{

	//initilization to the + infinite first

	int iRow, iCol;

	iRow = m_iMapRow;

	iCol = m_iMapColumn;



	int i,j;

	for (i = 1; i <= iRow; i++)

	{

		for (j = 1; j<= iCol; j++)

		{

			m_minTimeMap[i][j] = FLT_MAX;

		}

	}



	

	FinneynumCohorts=0;

	int fireClass;

	LDPOINT point;



	m_minTimeMap[m_iOriginRow][m_iOriginColumn] = 0; //m_iOriginRow starts from 1, not zero

	//damage, amtdamaged ++

	point.y = m_iOriginRow;

	point.x = m_iOriginColumn;

	FinneynumCohorts += damage(point,fireClass);

	m_Map(m_iOriginRow,m_iOriginColumn)=(unsigned char)(1+fireClass);

	

	int tempID = m_pFireSites(m_iOriginRow, m_iOriginColumn).FRUIndex;

	burnedCells[tempID] = 1;

	length_BurnedCells = 1; 


	m_FinneyCutoff = false;

	FinneyList.clear();

	FinneyInitilizeActiveCostList();	



	while (!FinneyList.empty())

	{

		

			

		//expand activePoints by one more		



		CFinneyCell tempCell;

		tempCell = FinneyList.front();

		FinneyList.pop_front();		

		m_checkMap[tempCell.row][tempCell.col] = 1;



		//break the loop if > the cut-off

		if (m_fireParam.iFuelFlag == 2) //duration approach

		{

			if (tempCell.minTime > m_fPredefinedDuration) 

				break;

		}

		else if(m_fireParam.iFuelFlag == 3) //fire size approach

		{	

			

			if ( (length_BurnedCells + 1) > m_lPredefinedFireSize)							

				break;			

		}	



		

		

		//simulate the damage

		point.y = tempCell.row;

		point.x = tempCell.col;

		FinneynumCohorts += damage(point,fireClass);

		m_Map(tempCell.row,tempCell.col)=(unsigned char)(1+fireClass);



		//bookkeeping		

		int tempID = m_pFireSites(tempCell.row, tempCell.col).FRUIndex;

		burnedCells[tempID] ++;

		length_BurnedCells ++;



		FinneyExpandCostList(tempCell.row,tempCell.col);

		



	}


}



void CFIRE::FinneyInitilizeActiveCostList()

{

	int i,j,k;



	int tempRow,tempCol;

	float weight;	



	int iRow = m_iMapRow, iCol = m_iMapColumn;

	for (i = 1; i <= iRow; i++)

	{

		for (j = 1; j<= iCol; j++)

		{

			m_checkMap[i][j] = 0;

		}

	}

	m_checkMap[m_iOriginRow][m_iOriginColumn] = 1;



	i = m_iOriginRow ; //iOriginRow starts from 1

	j = m_iOriginColumn ;

	



	//LDPOINT point;

		

	/*

	int neighborPointID[8];//sorted w.r.t. fuelcost from low to high

	//i.e., neighborPointID[0] = 1 means ID 1 neighbor point has least fuel cost	

	*/

	



	for (k =0; k<8; k++) 

	{

		

		switch (k) 

		{

		case 0: 

			tempRow = i -1;

			tempCol = j -1;

			weight = sqrt(2.0f);

			break;

		case 1:

			tempRow = i;

			tempCol = j -1;

			weight = 1.0;

			break;

		case 2:

			tempRow = i +1;

			tempCol = j -1;

			weight = sqrt(2.0f);

			break;

		case 3:

			tempRow = i +1;

			tempCol = j;

			weight = 1.0;

			break;

		case 4: 

			tempRow = i +1;

			tempCol = j +1;

			weight = sqrt(2.0f);

			break;

		case 5:

			tempRow = i;

			tempCol = j +1;

			weight = 1.0;

			break;

		case 6: 

			tempRow = i -1;

			tempCol = j +1;

			weight = sqrt(2.0f);

			break;

		case 7:

			tempRow = i -1;

			tempCol = j;

			weight = 1.0;

			break;

		}





		//calculate minTimeCost from the temp point to (i,j)

		//provided it is a valid point (within boundary, not an active point)

		float tempCost =0 ;		

		if (FinneyIsValid(tempRow,tempCol))

		{

			//tempCost = m_fireParam.cellSize * (m_fuelCostMap[tempRow][tempCol] + m_fuelCostMap[i][j]) * 0.5 * weight;

			tempCost = FinneyCalculateAcceleration(i,j,tempRow,tempCol,weight);

			m_minTimeMap[tempRow][tempCol] = tempCost;	

			

			// add the stuct fireCell to the listTime	

			CFinneyCell tempCell;

			tempCell.setValue(tempRow,tempCol,tempCost);

			FinneyList.push_back(tempCell);



		}

	}

	

	FinneyList.sort(); //sort in ascending order



}



int CFIRE::FinneyIsValid(int row, int column)

{

	if (row <1 || row > m_iMapRow 

		|| column < 1 || column > m_iMapColumn)

		return 0;

	if(m_minTimeMap[row][column] < FLT_MAX) //has been updated during spread simulation

		return 0;

	if (m_fuelCostMap[row][column] == FLT_MAX) //invalid landtype

		return 0;

	if (m_Map(row,column) > 1) //has been burned  

		return 0;

	return 1;

}



void CFIRE::FinneyExpandCostList(int row, int col)

{

	//put the eight neighbor points of ignition point into the costList

	int i,j;

	i = row;

	j = col;

	int tempRow,tempCol;

	float weight;	

	

	LDPOINT point;





	

	for (int k =0; k<8; k++) 

	{

		switch (k) 

		{

		case 0: 

			tempRow = i -1;

			tempCol = j -1;

			weight = sqrt(2.0f);

			break;

		case 1:

			tempRow = i;

			tempCol = j -1;

			weight = 1.0;

			break;

		case 2:

			tempRow = i +1;

			tempCol = j -1;

			weight = sqrt(2.0f);

			break;

		case 3:

			tempRow = i +1;

			tempCol = j;

			weight = 1.0;

			break;

		case 4: 

			tempRow = i +1;

			tempCol = j +1;

			weight = sqrt(2.0f);

			break;

		case 5:

			tempRow = i;

			tempCol = j +1;

			weight = 1.0;

			break;

		case 6: 

			tempRow = i -1;

			tempCol = j +1;

			weight = sqrt(2.0f);

			break;

		case 7:

			tempRow = i -1;

			tempCol = j;

			weight = 1.0;

			break;

		}

		//calculate minTimeCost from the temp point to (i,j)

		//provided it is a valid point (within boundary, not an active point)

		float tempCost = 0;

		if (FinneyIsValid(tempRow,tempCol))

		{

			

			/*

			tempCost = m_fireParam.cellSize * (m_fuelCostMap[tempRow][tempCol] + m_fuelCostMap[i][j]) 

				* 0.5 * weight 

				+ m_minTimeMap[i][j];

			*/



			tempCost = FinneyCalculateAcceleration(i,j,tempRow,tempCol,weight)

				+ m_minTimeMap[i][j];

			m_minTimeMap[tempRow][tempCol] = tempCost;



			CFinneyCell tempCell;

			tempCell.setValue(tempRow,tempCol,tempCost);

			//insert the cell into the list

			FinneyInsertList(tempCell);			

		}



	}



}



void CFIRE::InitilizeBurnedCells()

{

	for (int i = 0; i<MAX_LANDUNITS;i++) {		

		burnedCells[i]=0;		

	}



}



LDPOINT CFIRE::Retrieve(int index)

{

	//index is the ID of FRU

	long cellNo;

	long tempNo;

	cellNo = m_pStochastic->Uniform(1,m_FRUAvailableCells[index]);

	m_FRUAvailableCells[index] --;

	int i,j;

	int count=0;

	int snr = m_pFireSites.numRows();

	int snc = m_pFireSites.numColumns(); 

	bool found = false;

	/*for (i=1;i<=snr;i++)

	{

		for (j=1;j<=snc;j++)

		{

			int tempID;

			tempID = m_pFireSites(i,j).FRUIndex;	

			tempNo = (i-1) * snc + j;

			if (tempID == index && m_pIgnitionStatusArray[tempNo] == 0)

				count ++;

			if (count == cellNo)

			{

				m_pIgnitionStatusArray[tempNo] = 1;

				found = true;

				break;

			}

		}

		if (found)

			break;

	}



	if (j > snc || i > snr)

	{

		j = snc;

		i = snr;

		errorSys("Error in generating ignition points",STOP);		

	}	*/
	
	LDPOINT p = m_pFireSites.return_a_point_from_index(index,cellNo);
	tempNo = (p.y-1) * snc + p.x;
	if (m_pIgnitionStatusArray[tempNo] == 0){
		m_pIgnitionStatusArray[tempNo] = 1;
	}


	return p;



}



float CFIRE::fireDuration(float mean, float std)

{

	

	/*

	//generate random fire duration based on lognormal distribution	

	//if x is fire size following lognormal distribution with mean MFS and variance as VAR

	//then log(x) follows normal distribution with

	//mean = 2logMFS - 1/2log(VAR+MFS square)

	//variance = log(VAR+MFS square) - 2logMFS

	double NormalMean,NormalStd, NormalVar;

	double var;	

	var = std * std;

	NormalMean = 2.0 * log(mean) - 0.5 * log(var + mean*mean);

	NormalVar = log(1.0*(var + mean*mean)) - 2.0 * log(mean);

	NormalStd = sqrt(NormalVar);	

	double duration;

	duration = m_pStochastic->Normal(NormalMean,NormalStd);

	duration = exp(duration);

	return (float) duration;

	*/

	

	

	

	//generate negative exponential distribution with input mean

	double duration;

	duration = std + m_pStochastic->Exponential(mean);

	return (float) duration;

	







}



void CFIRE::WriteInitiationMap(int snr, int snc, int itr, double wAdfGeoTransform[])

{

	

	int i;

	char str[255];	

	sprintf(str,"Initiation map for year %d.",itr*m_pLAND->TimeStep_Fire);

	m_InitiationMap.setHeader(m_pLAND->getHeader());

	m_InitiationMap.rename(str);

	for(i=0;i<maxLeg;i++)

	{

		m_InitiationMap.assignLeg(i,"");

	}		

	m_InitiationMap.assignLeg(0,"No initiations");

	m_InitiationMap.assignLeg(1,"one initiation");

	

	sprintf(str,"%sInitiation%d",m_strFireOutputDirectory,itr*m_pLAND->TimeStep_Fire);

	m_InitiationMap.setCellSize(m_fireParam.cellSize);

	m_InitiationMap.write(str,red,green,blue,wAdfGeoTransform);

}



void CFIRE::WriteCummInitiationMap(int snr, int snc, int itr, double wAdfGeoTransform[])

{

	int i;

	char str[255];	

	sprintf(str,"Cummulative Initiation map");

	m_cummInitiationMap.setHeader(m_pLAND->getHeader());

	m_cummInitiationMap.rename(str);

	for(i=0;i<maxLeg;i++)

	{

		m_cummInitiationMap.assignLeg(i,"");

	}		

	m_cummInitiationMap.assignLeg(0,"No initiations");

	m_cummInitiationMap.assignLeg(1,"one initiation");

	m_cummInitiationMap.assignLeg(2,"two initiations");

	m_cummInitiationMap.assignLeg(3,"three initiations");

	

	sprintf(str,"%sCummInit",m_strFireOutputDirectory);

	m_cummInitiationMap.setCellSize(m_fireParam.cellSize);

	m_cummInitiationMap.write(str, red, green, blue, wAdfGeoTransform);



}



void CFIRE::FinneyInsertList(CFinneyCell inCell)

{

	if (FinneyList.size() == 0)

	{

		FinneyList.push_back(inCell);

		return;

	}

	list<CFinneyCell>::iterator start, end, location;

	start = FinneyList.begin();

	end = FinneyList.end();

	location = lower_bound(start, end, inCell);

	if (location != end)

		FinneyList.insert(location,inCell);

	else

		FinneyList.push_back(inCell);





}



double CFIRE::ProductLog(double kernel)

{

	int i; 

	const double eps=4.0e-16, em1=0.3678794411714423215955237701614608; 

	double p,e,t,w;

	

	if (kernel<-em1) { 

		return -1.0;

		char pszMessage[1024];

		sprintf(pszMessage, "LambertW: bad argument %g, exiting. Code PL1\n",kernel);		

		fprintf(stderr,"LambertW: bad argument %g, exiting.\n",kernel); 

		exit(1); 

	}

	

	if (0.0==kernel) return 0.0;

	if (kernel<-em1+1e-4) { // series near -em1 in sqrt(q)

		double q=kernel+em1,r=sqrt(q),q2=q*q,q3=q2*q;

		return 

			-1.0

			+2.331643981597124203363536062168*r

			-1.812187885639363490240191647568*q

			+1.936631114492359755363277457668*r*q

			-2.353551201881614516821543561516*q2

			+3.066858901050631912893148922704*r*q2

			-4.175335600258177138854984177460*q3

			+5.858023729874774148815053846119*r*q3

			-8.401032217523977370984161688514*q3*q;  // error approx 1e-16

	}

	

	/* initial approx for iteration... */

	if (kernel<1.0) { /* series near 0 */

		p=sqrt(2.0*(2.7182818284590452353602874713526625*kernel+1.0));

		w=-1.0+p*(1.0+p*(-0.333333333333333333333+p*0.152777777777777777777777)); 

	} else 

		w=log(kernel); /* asymptotic */

	if (kernel>3.0) w-=log(w); /* useful? */

	for (i=0; i<15; i++) { /* Halley iteration */

		e=exp(w); 

		t=w*e-kernel;

		p=w+1.0;

		t/=e*p-0.5*(p+1.0)*t/p; 

		w-=t;

		if (fabs(t)<eps*(1.0+fabs(w))) return w; /* rel-abs error */

	}

	/* should never get here */

	return w;

	char pszMessage[1024];

	sprintf(pszMessage, "LambertW: bad argument %g, exiting. Code PL%g\n",kernel,w);

	fprintf(stderr,"LambertW: No convergence at kernel=%g, exiting.\n",kernel); 

	exit(1);



}



float CFIRE::FinneyCalculateAcceleration(int from_x, int from_y, int to_x, int to_y, float weight)

{

// calculate the time for fire traveling from the point (FROM) to the point (TO) and return it

// meanwhile the function will update the actualROSMap at the point (TO)



	// theApp.m_parameterSet.iCellSize * (m_fuelCostMap[to_x][to_y] + m_fuelCostMap[from_x][from_y]) * 0.5 * weight;

	double a = 0.115;

	const double eps = 0.00000001;



	float dist, y;	

	float t_alpha, t_beta;

	float ROS_boundary, ROS_beta;

	double PL_Kernel;

	

	dist = m_fireParam.cellSize * 0.5 * weight;

	if (m_fuelCostMap[from_x][from_y] > eps)

	{

		y = 1 - m_actualROSMap[from_x][from_y]/m_fuelCostMap[from_x][from_y];

		PL_Kernel = -1 * y * exp(-1 * (a * dist / m_fuelCostMap[from_x][from_y] + y));

		t_alpha = dist / m_fuelCostMap[from_x][from_y] + 1/a * y + 1/a * ProductLog(PL_Kernel);

		ROS_boundary = m_fuelCostMap[from_x][from_y] * (1 - y * exp(-1 * a * t_alpha));

	}

	else

	{

		return FLT_MAX;

	}







	if (m_fuelCostMap[to_x][to_y] > ROS_boundary && m_fuelCostMap[to_x][to_y] > eps)

	{

		y = 1 - ROS_boundary / m_fuelCostMap[to_x][to_y];

		PL_Kernel = -1 * y * exp(-1 * (a * dist / m_fuelCostMap[to_x][to_y] + y));

		t_beta = dist / m_fuelCostMap[to_x][to_y] + 1/a * y + 1/a * ProductLog(PL_Kernel);

		ROS_beta = m_fuelCostMap[to_x][to_y] * (1 - y * exp(-1 * a * t_beta));

	}

	else

	{

		if (m_fuelCostMap[to_x][to_y] > eps)

		{

			t_beta = dist / m_fuelCostMap[to_x][to_y];

			ROS_beta = m_fuelCostMap[to_x][to_y];

		}

		else

		{

			return FLT_MAX;

		}



	}

	m_actualROSMap[to_x][to_y] = ROS_beta;



	float tempCost;

	tempCost = t_alpha + t_beta;

	return tempCost;

}

