/***********************************************

Name: FuelGlobalFuncs.cpp







************************************************/

#include <stdio.h>

#include "system1.h"

#include "error.h"

#include "FuelGlobalFuncs.h"

#include "FuelGlobalVariables.h"

#include "FuelDefines.h"

#include "Reclass4.h"



#ifdef __BORLANDC__

#include <dir.h>

#endif

#include <direct.h>

#ifdef __UNIX__

#include <sys/stat.h>

#endif



#include <ctype.h>

#include <string.h>

#include "defines.h"

//add by houbin 9/28/2017
#define DTN 0

#define DTNBA 1

#define DTNBIO 2

#define COUNT 3
//add by houbin 9/28/2017


static int red2[maxLeg]={0,70,0,0,0,0,0,0,200,100,255,150,200,200,255,255};

static int green2[maxLeg]={0,70,0,0,0,150,200,255,30,200,50,50,200,0,255};

static int blue2[maxLeg]={0, 70,125,200,255,0,100,0,30,50,50,0,0,255};



FILE*	fuelout;	

FILE*	fuelManageFile;

FILE*	fuelManOutStand;



void getFuelInput(char* strfuelInputFile, int specAttrsNo, int luNo) 

{



	int i=0;

	FILE *fuelFile;	

	if(gDLLMode & G_FUEL) 

	{

		if ((fuelFile=LDfopen(strfuelInputFile,1))==NULL)

			errorSys("Fuel input file not found.",STOP);



//		printf("Begin reading in fuel data\n");

		fuelsites.init(specAttrsNo, luNo);		

		fuelsites.read(fuelFile, strfuelInputFile);

		fuelsites.initFuelSite(pfuelcoresites->numRows()+1,pfuelcoresites->numColumns()+1);

	}



}



void getFuelManage(char* strfuelManFile) 

{

	int i=0;

	FILE *fuelFile;	



	if(gDLLMode & G_FUELMANAGEMENT) 

	{

		if ((fuelFile=LDfopen(strfuelManFile,1))==NULL)

			errorSys("Fuel input file not found.",STOP);



//		printf("Begin reading in fuel management data\n");

		fuelManager.read(fuelFile);

	}

}





void getFuelRule(char* strfuelRuleFile) 

{

	FILE *fuelRuleFile;	

	if(gDLLMode & G_FUEL) 

	{

		if ((fuelRuleFile=LDfopen(strfuelRuleFile,1))==NULL)

			errorSys("Fuel Rule file not found.",STOP);		

		fuelrule.read(fuelRuleFile);

	}

}



//added by Wei Li

//void FuelInit(PDP* ppdp, char* strfuelInitFile, int specAttrsNo, int luNo, char* stroutputDir, SITES* s, int iDllMode, char* pmgtAreaMapFile, char* pstandMapFile)

int FuelInit(PDP* ppdp, char* strfuelInitFile, int specAttrsNo, int luNo, char* stroutputDir, SITES* s, int iDllMode)

{

    char	str1[100];

	char*	strfuelInputFile;	//Fuel input file name

	char*	strfuelRuleFile;	//Fuel ruls file name

	char*	strfuelManFile;		//Fuel management file name

	char*	strfuelOutputFile;	//Fuel output file

	char*	strfuelManOutputFile;	//Fuel management output file

	char*	strfuelManOutNameForStand;	//Fuel management output file, record for each stand

	FILE*	pfFUELInit;



	char*	strpmgtAreaMapFile;

	char*	strpstandMapFile;



	// Shang add 04/04

	char*	strFireRegimeAttr;	//Fire regime attribute file

	char*	strFireRegimeMap;	//Fire regime map file



	pfuelcoresites = s;

	gDLLMode = iDllMode;

	pPDP = ppdp;



	strfuelInputFile = new char[FNSIZE];	//Fuel input file name

	strfuelRuleFile = new char[FNSIZE];	//Fuel ruls file name

	strfuelManFile = new char[FNSIZE];		//Fuel management file name

	strfuelOutputFile = new char[FNSIZE];	//Fuel output file

	strfuelManOutputFile = new char[FNSIZE];	//Fuel management output file

	strfuelManOutNameForStand = new char[FNSIZE];	//Fuel management output for eadh stand

	strpmgtAreaMapFile = new char[FNSIZE];

	strpstandMapFile = new char[FNSIZE];



	strFireRegimeAttr = new char[FNSIZE];	//Fire regime attribute file

	strFireRegimeMap = new char[FNSIZE];	//Fire regime map file



	if ((pfFUELInit = LDfopen(strfuelInitFile, 1)) == NULL)

		errorSys("FUEL: FUELInit file not found.",STOP);

	

	if (fscanc(pfFUELInit,"%s",strfuelInputFile)!=1)

		 errorSys("Error reading in fuelInputFile from parameter file.",STOP);

	if (fscanc(pfFUELInit,"%s",strfuelRuleFile)!=1)

		 errorSys("Error reading in fuelRuleFile from parameter file.",STOP);



	// Shang add 04/04

	if (fscanc(pfFUELInit,"%s",strFireRegimeAttr)!=1)

		 errorSys("Error reading in strFireRegimeAttr from parameter file.",STOP);

	if (fscanc(pfFUELInit,"%s",strFireRegimeMap)!=1)

		 errorSys("Error reading in strFireRegimeMap from parameter file.",STOP);



	if (fscanc(pfFUELInit,"%s",strfuelManFile)!=1)

		 errorSys("Error reading in fuelManFile from parameter file.",STOP);



	if (fscanc(pfFUELInit,"%s",strfuelOutputFile)!=1)

		 errorSys("Error reading in fuelOutputFile from parameter file.",STOP);

	if (fscanc(pfFUELInit,"%s",strfuelManOutputFile)!=1)

		 errorSys("Error reading in fuelManOutputFile from parameter file.",STOP);

	if (fscanc(pfFUELInit,"%s",strfuelManOutNameForStand)!=1)

		 errorSys("Error reading in fuelManOutNameForStand from parameter file.",STOP);

	if (fscanc(pfFUELInit,"%s",strpstandMapFile)!=1)

		 errorSys("Error reading in fuelManOutNameForStand from parameter file.",STOP);

	if (fscanc(pfFUELInit,"%s",strpmgtAreaMapFile)!=1)

		 errorSys("Error reading in fuelManOutputFile from parameter file.",STOP);



	if (strcmp (strfuelManFile, "N/A")) {

		gDLLMode = gDLLMode | G_FUELMANAGEMENT;

		fuelsites.initFuelManSite();

	}

	getFuelInput(strfuelInputFile, specAttrsNo, luNo);	// readin input of fuel



	// Shang read in the fire regime attribute

	fuelsites.readFireRegimeAttr(strFireRegimeAttr);

	fuelsites.readFireRegimeMap(strFireRegimeMap);



	sprintf(str1,"%s/%s",stroutputDir, "Fuel");



#ifdef __UNIX__

_mkdir(str1, 493);

#else

_mkdir(str1);

#endif


if (ppdp->TrackFlag == 1)
{
	char outputdir_spec_dtn[100];
	char outputdir_spec_dtnba[100];
	char outputdir_spec_dtnbio[100];
	char outputdir_agerange_spec_dtn_all[100];

	sprintf(outputdir_spec_dtn, "%s/%s", str1, "DTN");
	sprintf(outputdir_spec_dtnba, "%s/%s", str1, "DTNBA");
	sprintf(outputdir_spec_dtnbio, "%s/%s", str1, "DTNBIO");
	sprintf(outputdir_agerange_spec_dtn_all, "%s/%s", str1, "SPEC_AGERANGE");

	if (_mkdir(outputdir_spec_dtn) != 0)
	{
		if (errno != EEXIST)
			errorSys("Fuel: Can't create the dead tree number in diff spec direcory", STOP);
	}
	if (_mkdir(outputdir_spec_dtnba) != 0)
	{
		if (errno != EEXIST)
			errorSys("Fuel: Can't create the dead tree basalarea in diff spec direcory", STOP);
	}
	if (_mkdir(outputdir_spec_dtnbio) != 0)
	{
		if (errno != EEXIST)
			errorSys("Fuel: Can't create the dead tree biomass in diff spec direcory", STOP);
	}
	if (_mkdir(outputdir_agerange_spec_dtn_all) != 0)
	{
		if (errno != EEXIST)
			errorSys("Fuel: Can't create the dead tree number&biomass&basalarea in diff spec&agerange direcory", STOP);
	}

}



	sprintf(str1,"%s/%s/%s",stroutputDir, "fuel",strfuelOutputFile);

	//if ((fuelout=LDfopen(str1,3))==NULL)

	//	  errorSys("Can not open fuelout file for write\n",STOP);

	//if ((diFile=LDfopen(strn,1))==NULL)

	  //errorSys("Wind input file not found.",STOP);



	

	if ((fuelout=fopen(str1,"w"))==NULL)

		errorSys("Can not open fuelout file for write\n",STOP);

	

	// write the title into fuel output file

		for(int t=0; t<5;t++)			

	//		LDfprintf0(fuelout,"Year        ITEM          Water       No-active   Background  0           1           2           3           4           5   Average  ");

	//	LDfprintf0(fuelout,"\n");



			fprintf(fuelout,"Year        ITEM          Water       No-active   Background  0           1           2           3           4           5   Average  ");

		fprintf(fuelout,"\n");

	

	char *str;

	str = new char[FNSIZE];

	getFuelRule(strfuelRuleFile);			// get fuel rule

	fueltrack.FFTrack();	// track fine fuel

	fueltrack.CFInit();		// track coarse fuel

//	printf("Fuel: Begin calculate potential fire probility.\n\n");

//	PotentialFireProbility();	// calculate the potential fire probability

	fueltrack.FireRiskTrack();		// track fire risk



	if ((gDLLMode & G_FUELMANAGEMENT) != 0)

	{

		 // FILE* pffuelManageFile;

		  //FILE* pffuelManOutStand;		

		//<Add By Qia on Nov 20 2008>

	int filenamelength;

	char fileextensive[10];

	filenamelength=strlen(strpstandMapFile);

	fileextensive[0]=strpstandMapFile[filenamelength-4];

	fileextensive[1]=strpstandMapFile[filenamelength-3];

	fileextensive[2]=strpstandMapFile[filenamelength-2];

	fileextensive[3]=strpstandMapFile[filenamelength-1];

	

	fileextensive[4]='\0';

	if(strcmp(fileextensive,".asc")==0){

		

	

    	standMap.readtxt(strpstandMapFile);

		}

	else{

			standMap.read(strpstandMapFile);

		}

	  //</Add By Qia on Nov 20 2008>  

	    managementAreaMap.read(strpmgtAreaMapFile);

		fuelpstands = new FUELSTANDS(); //change by Qia on Nov 20 2008

		fuelpstands->construct(); //change by Qia on Nov 20 2008

	    managementAreas.construct();



		

		// readin fuel management events

		sprintf(str,"%s/%s/%s",stroutputDir,"fuel",strfuelManOutputFile);

		if ((fuelManageFile=fopen(str,"w"))==NULL)

			errorSys("Can not open fuel management file for write\n.",STOP);		

		sprintf(str,"%s/%s/%s",stroutputDir,"fuel",strfuelManOutNameForStand);

		if ((fuelManOutStand=fopen(str,"w"))==NULL)

			errorSys("Can not open fuelManagenet Output for Stand file for write\n.",STOP);

		fprintf(fuelManOutStand, "Iteration   ManagementArea   Stand   ManageStie  FireRiskBeforeTreatment FireRiskAfterTreatment\n");

		getFuelManage(strfuelManFile);



		//<Add By Qia on Nov 20 2008>

		managementAreaMap.freeMAPdata();

	//</Add By Qia on Nov 20 2008>

		//fclose(pffuelManageFile);

		//fclose(pffuelManOutStand);

	}



	return gDLLMode;

}





void FuelProcessEvent()

{

	fueltrack.FFTrack();		// track fine fuel

	fueltrack.CFTrack();		// track coarse fuel

//	printf("Fuel: Begin calculate potential fire probility.\n\n");

//	PotentialFireProbility();	// track fire probability (decimal)

	fueltrack.FireRiskTrack();

}





void FuelManageProcessEvents(int itr)

{

	fuelManager.processEvents(itr);
	/*printf("\n\nhere1 %d %f\t", pPDP->cDeadTreeNum[165][148][0], pPDP->cDeadTreeBio[165][148][0]);
	printf("\n\nhere1 %d %f\n\n", pPDP->cDeadTreeNum[165][151][0], pPDP->cDeadTreeBio[165][151][0]);*/

}





void FuelputFuelOutput(int itr, char* FueloutputDir, int nCellSize, double wAdfGeoTransform[])

{

	char str[100];

	char fuelOutDir[100];
	//printf("\n\nhere3 %d %f\t", pPDP->cDeadTreeNum[165][148][0], pPDP->cDeadTreeBio[165][148][0]);
	//printf("\n\nhere3 %d %f\n\n", pPDP->cDeadTreeNum[165][151][0], pPDP->cDeadTreeBio[165][151][0]);
	//printf("specnumb=%d\n",sizeof(pPDP->cDeadTreeNum[0][0])/sizeof(pPDP->cDeadTreeNum[0][0][0]));
	//printf("itr=%d\n",itr);

	//sprintf(FueloutputDir, "%s/%s", FueloutputDir, "fuel");

	sprintf(fuelOutDir, "%s/%s", FueloutputDir, "fuel");

	if (itr != 0 && (gDLLMode & G_FUELMANAGEMENT)!=0)
	{
		/*printf("%d ", pPDP->cDeadTreeNum[0][0][0]);
		printf("%d ", pPDP->cDeadTreeNum[0][0][1]);
		printf("%d ", pPDP->cDeadTreeNum[0][0][2]);
		printf("%d\n", pPDP->cDeadTreeNum[0][0][3]);
		printf("itr=%d",itr);*/
		//getchar();
		//getchar();
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//int spec_num = sizeof(pPDP->cDeadTreeNum[0][0]) / sizeof(pPDP->cDeadTreeNum[0][0][0]);
		SITE * siteptr = (*pfuelcoresites) (2, 2);


		//add by houbin 10/25/2017
		int spec_num = pPDP->specnum;
		if (pPDP->TrackFlag == 1)
		{
			if (pfuelcoresites->OutputAgeRangeFuelFlag == 1)
			{
				for (int i = 0; i < spec_num; i++)
				{
					if (pfuelcoresites->GetOutputAgeRangeModuleFlagArray(i, DTN))
					{
						for (int j = 1; j <= pfuelcoresites->GetOutputAgeRangeModuleFlagArray(i, COUNT); j++)
						{
							int low = pfuelcoresites->GetOutputAgeRangeModuleFlagArray(i, j, 0);
							int high = pfuelcoresites->GetOutputAgeRangeModuleFlagArray(i, j, 1);
							char str_dtn_spec_ageinterval[100];
							char fuelOutDir_dtn_spec_ageintval[100];
							MAP8 map_spec_agerange(pfuelcoresites->getHeader());
							sprintf(fuelOutDir_dtn_spec_ageintval, "%s/%s", fuelOutDir, "SPEC_AGERANGE");
							sprintf(str_dtn_spec_ageinterval, "%s/%s_%d_%d_DeadTreeNum_burn_%d", fuelOutDir_dtn_spec_ageintval, siteptr->specAtt(i + 1)->name, low, high, itr*pfuelcoresites->TimeStep_Fuel);
							FuelReclass_DTN_spec_agerange(map_spec_agerange, i, j, fuelout);
							map_spec_agerange.setCellSize(nCellSize);
							map_spec_agerange.write(str_dtn_spec_ageinterval, red2, green2, blue2, wAdfGeoTransform);
						}
					}
					if (pfuelcoresites->GetOutputAgeRangeModuleFlagArray(i, DTNBA))
					{
						for (int j = 1; j <= pfuelcoresites->GetOutputAgeRangeModuleFlagArray(i, COUNT); j++)
						{
							int low = pfuelcoresites->GetOutputAgeRangeModuleFlagArray(i, j, 0);
							int high = pfuelcoresites->GetOutputAgeRangeModuleFlagArray(i, j, 1);
							char str_dtnba_spec_ageinterval[100];
							char fuelOutDir_dtnba_spec_ageintval[100];
							MAP8 map_spec_agerange(pfuelcoresites->getHeader());
							sprintf(fuelOutDir_dtnba_spec_ageintval, "%s/%s", fuelOutDir, "SPEC_AGERANGE");
							sprintf(str_dtnba_spec_ageinterval, "%s/%s_%d_%d_DeadTreeBio_burn_%d", fuelOutDir_dtnba_spec_ageintval, siteptr->specAtt(i + 1)->name, low, high, itr*pfuelcoresites->TimeStep_Fuel);
							FuelReclass_DTNBA_spec_agerange(map_spec_agerange, i, j, fuelout);
							map_spec_agerange.setCellSize(nCellSize);
							map_spec_agerange.write(str_dtnba_spec_ageinterval, red2, green2, blue2, wAdfGeoTransform);
						}
					}
					if (pfuelcoresites->GetOutputAgeRangeModuleFlagArray(i, DTNBIO))
					{
						for (int j = 1; j <= pfuelcoresites->GetOutputAgeRangeModuleFlagArray(i, COUNT); j++)
						{
							int low = pfuelcoresites->GetOutputAgeRangeModuleFlagArray(i, j, 0);
							int high = pfuelcoresites->GetOutputAgeRangeModuleFlagArray(i, j, 1);
							char str_dtnbio_spec_ageinterval[100];
							char fuelOutDir_dtnbio_spec_ageintval[100];
							MAP8 map_spec_agerange(pfuelcoresites->getHeader());
							sprintf(fuelOutDir_dtnbio_spec_ageintval, "%s/%s", fuelOutDir, "SPEC_AGERANGE");
							sprintf(str_dtnbio_spec_ageinterval, "%s/%s_%d_%d_DeadTreeBA_burn_%d", fuelOutDir_dtnbio_spec_ageintval, siteptr->specAtt(i + 1)->name, low, high, itr*pfuelcoresites->TimeStep_Fuel);
							FuelReclass_DTNBIO_spec_agerange(map_spec_agerange, i, j, fuelout);
							map_spec_agerange.setCellSize(nCellSize);
							map_spec_agerange.write(str_dtnbio_spec_ageinterval, red2, green2, blue2, wAdfGeoTransform);
						}
					}
				}
			}
			if (pfuelcoresites->OutputFuelFlag == 1)
			{
				for (int i = 0; i < spec_num; i++){
					float nodata = 0;
					if (pfuelcoresites->GetOutputModuleFlagArray(i, DTN))
					{
						if (strcmp(siteptr->specAtt(i + 1)->name, "N/A") != 0)
						{
							char str_dtn[100];
							char fuelOutDir_dtn[100];
							MAP8 map(pfuelcoresites->getHeader());
							sprintf(fuelOutDir_dtn, "%s/%s", fuelOutDir, "DTN");
							FuelReclass_DTN(map, i, fuelout);
							sprintf(str_dtn, "%s/%s_TreeNum_burn_%d", fuelOutDir_dtn, siteptr->specAtt(i + 1)->name, itr*pfuelcoresites->TimeStep_Fuel);
							map.setCellSize(nCellSize);
							map.write(str_dtn, red2, green2, blue2, wAdfGeoTransform);
						}
					}
					if (pfuelcoresites->GetOutputModuleFlagArray(i, DTNBA))
					{
						if (strcmp(siteptr->specAtt(i + 1)->name, "N/A") != 0)
						{
							char str_dtn_ba[100];
							char fuelOutDir_dtnba[100];
							MAP8 map_ba(pfuelcoresites->getHeader());
							sprintf(fuelOutDir_dtnba, "%s/%s", fuelOutDir, "DTNBA");
							FuelReclass_DTNBA(map_ba, i, fuelout);
							sprintf(str_dtn_ba, "%s/%s_BA_burn_%d", fuelOutDir_dtnba, siteptr->specAtt(i + 1)->name, itr*pfuelcoresites->TimeStep_Fuel);
							map_ba.setCellSize(nCellSize);
							map_ba.write(str_dtn_ba, red2, green2, blue2, wAdfGeoTransform);

						}
					}
					if (pfuelcoresites->GetOutputModuleFlagArray(i, DTNBIO))
					{
						if (strcmp(siteptr->specAtt(i + 1)->name, "N/A") != 0)
						{
							char str_dtn_bio[100];
							char fuelOutDir_dtnbio[100];
							MAP8 map_bio(pfuelcoresites->getHeader());
							sprintf(fuelOutDir_dtnbio, "%s/%s", fuelOutDir, "DTNBIO");
							FuelReclass_DTNBIO(map_bio, i, fuelout);
							sprintf(str_dtn_bio, "%s/%s_Bio_burn_%d", fuelOutDir_dtnbio, siteptr->specAtt(i + 1)->name, itr*pfuelcoresites->TimeStep_Fuel);
							map_bio.setCellSize(nCellSize);
							float nodata = 0;
							map_bio.write(str_dtn_bio, red2, green2, blue2, wAdfGeoTransform);
						}
					}



					/*for (int iii = 1; iii <= 349; iii++)
					{
					for (int jjj = 1; jjj <= 945; jjj++)
					{
					int xxx = ((iii - 1) * 945) + jjj - 1;
					if ((map(iii, jjj) == 0 && map_bio.data_bio[xxx] != (float)0) || (map(iii, jjj) != 0 && map_bio.data_bio[xxx] == (float)0))
					{
					printf("\n(%d,%d,%d)dtn and dtnbio different5 %d %f", iii, jjj, i, map(iii, jjj), map_bio.data_bio[xxx]);
					}
					}
					}*/
				}
			}
		}
		
		//add by houbin 10/25/2017

		/*
		//////////////////////////////////////////////////////////////////////////////////////
		int longest_longevity = 0;
		for (int iii = 1; iii <= pPDP->specnum; iii++)
		{
			if (longest_longevity < siteptr->specAtt(iii)->longevity)
			{
				longest_longevity = siteptr->specAtt(iii)->longevity;
			}
		}
		int interv_number = pPDP->interval_number;
		for (int i = 0; i < interv_number; i++)
		{
			char str_dtn_ageinterval[100];
			char fuelOutDir_dtnageintv[100];
			MAP8 map_age_intv(pfuelcoresites->getHeader());
			sprintf(fuelOutDir_dtnageintv, "%s/%s", fuelOutDir, "AGE INTERVAL");
			sprintf(str_dtn_ageinterval, "%s/%d_%d_TreeNum_burn_%d", fuelOutDir_dtnageintv, i*longest_longevity / interv_number, (i+1)*longest_longevity / interv_number, itr*pfuelcoresites->TimeStep_Fuel);
			FuelReclass_DTN_age_interval(map_age_intv, i, fuelout);
			map_age_intv.setCellSize(nCellSize);
			map_age_intv.write(str_dtn_ageinterval, red2, green2, blue2, wAdfGeoTransform);
		}
		///////////////////////////////////////////////////////////////////////////////////////
		*/

		
		
		
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	MAP8 m(pfuelcoresites->getHeader());
	

	





//	printf("Reclassfy fine fuel load!\n");

	fprintf(fuelout, "%-4d  finefuel            ",itr*pfuelcoresites->TimeStep_Fuel);



	//FuelReclass(m,0, fueloutfile);	// reclass fine fuel load	

	FuelReclass(m,0, fuelout);	// reclass fine fuel load	

	sprintf(str,"%s/FF%d",fuelOutDir,itr*pfuelcoresites->TimeStep_Fuel);

	m.setCellSize(nCellSize);
	/*printf("\nhere0\n\n");
	m.print(20);
	printf("\nhere1\n\n");*/

	//double wAdfGeoTransform[6] = { 0.00, nCellSize, 0.00, 600.00, 0.00, nCellSize };//comment by houbin 2/6/2017

	float nodata = 0;
	///////////////   houbin test 12/13/2016
	/*int itr111;
	for (itr111 = 1; itr111 < 700; itr111++)
	{
		red2[itr111] = 0;
		green2[itr111] = 0;
		blue2[itr111] = 0;
	}*/
	///////////////   houbin test 12/13/2016
	/*m.print(20);
	printf("\nhere2\n\n");*/
	m.write(str, red2, green2, blue2, wAdfGeoTransform);
	/*m.print(20);
	printf("\nhere3\n\n");*/
	//getchar();


//	printf("Reclassfy coarse fuel load\n");

	fprintf(fuelout,"%-4d  coarsefuel          ", itr*pfuelcoresites->TimeStep_Fuel);



	//FuelReclass(m,1, fueloutfile);  // reclass coarse fuel load

	FuelReclass(m,1, fuelout);  // reclass coarse fuel load

	sprintf(str,"%s/CF%d",fuelOutDir,itr*pfuelcoresites->TimeStep_Fuel);

	m.setCellSize(nCellSize);

	

	m.write(str, red2, green2, blue2, wAdfGeoTransform);

	

	// First, relcass potential fire probility into category

	if (!fuelrule.fireProbReclass)

		if (!fireProbilityCategory())

			 errorSys("Error! Can not reclassify potential fire probility into categories\n", STOP);



//	printf("Reclassfy fire probility, severity and risk\n");

	MAP8 m1(pfuelcoresites->getHeader());	// map for fire probability

	MAP8 m2(pfuelcoresites->getHeader());	// map for potential fire intensity



	fireReclass(m1,m2,m,fuelout, itr); // reclass fire probility, severity & risk

//	fireReclass(m1,m2,m); // reclass fire probility, severity & risk



	sprintf(str,"%s/FR%d",fuelOutDir,itr*pfuelcoresites->TimeStep_Fuel);	// fire risk

	m.setCellSize(nCellSize);

	

	m.write(str, red2, green2, blue2, wAdfGeoTransform);



	sprintf(str,"%s/FP%d",fuelOutDir,itr*pfuelcoresites->TimeStep_Fuel);  // fire probility

	m1.setCellSize(nCellSize);


	m1.write(str, red2, green2, blue2, wAdfGeoTransform);



	sprintf(str,"%s/FI%d",fuelOutDir,itr*pfuelcoresites->TimeStep_Fuel);  // fire severity

	m2.setCellSize(nCellSize);

	

	m2.write(str, red2, green2, blue2, wAdfGeoTransform);



}