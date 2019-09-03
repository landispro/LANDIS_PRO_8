// FuelManEvent.cpp: implementation of the FuelManEvent class.

//

//////////////////////////////////////////////////////////////////////

#include "FuelManEvent.h"

#include "error.h"

#include "system1.h"

//#include "landis.h"

//#include "GlobalVariables.h"

//#include "GlobalFunctions.h"

#include "StandIterator.h"

#include "FuelStand.h"



#include "FuelGlobalVariables.h"

#include "FuelDefines.h"

#define COUNT 3

//////////////////////////////////////////////////////////////////////

// Construction/Destruction

//////////////////////////////////////////////////////////////////////



FuelManEvent::FuelManEvent()

{

	//  FineFuelManagement =0,

	//  CoarseFuelManagement = 0;

	//  Initialization

	FineFuelManagement = new int [FUELCATEGORY+1];

	CoarseFuelManagement = new int [FUELCATEGORY+1];

	Label = new char[FNSIZE];

	manageAreaIdentify = 0;

	rankAlgorithm = 0;

	entryDecade = 0;

	finalDecade = 0;

	reentryInterval = 0;

	minClass = 10;

	itRankAlgorithm = NULL;

	FFTreat = new int[6];

	CFTreat = new int[6];

//	useFireForTreatment = 0;



}



// Read in the fuel management event one by one

void FuelManEvent::read (FILE *inFile) {

	

	int temp;

	int i = 0;

	

	// Read in the Event label

	if (!fscanc(inFile,"%s",Label))

		errorSys("Error when read event label in fuel management file.",STOP);

	//printf("Event label: %s\n", Label);



	// Read in the Management area identify 0, 1, 2, 3...

	if (!fscanc(inFile, "%d", &manageAreaIdentify) || manageAreaIdentify<=0 || manageAreaIdentify>255)

		errorSys("Error when read management area identifier in fuel management file.",STOP);

	//printf("Management area identify: %d\n", manageAreaIdentify);

	

	// Read in the ranking algorithm

	if (!fscanc(inFile, "%d", &rankAlgorithm) || rankAlgorithm <=0 || rankAlgorithm >255)

		errorSys("Error when read rankAlgorithm in fuel management file.",STOP);

	//printf("rankAlgorithm: %d\n", rankAlgorithm);

	

	// Read in the entry decade
	/////////////////////////////
	//int hcb57;
	//fscanc(inFile, "%d",&hcb57);
	////////////////////////////

	if (!fscanc(inFile, "%d", &entryDecade) || entryDecade<0 || entryDecade>9990)

		errorSys("Error when read entryDecade in fuel management file.",STOP);

	       if (entryDecade < pfuelcoresites->TimeStep_Fuel) entryDecade=1;//Bu, Rencang April 28, 2009

		else entryDecade=entryDecade/pfuelcoresites->TimeStep_Fuel;

	//printf("entryDecade: %d\n", entryDecade);

	

	// REad in the final decade

	if (!fscanc(inFile, "%d", &finalDecade) || finalDecade <0 || finalDecade > 9900)

		errorSys("Error when read finalDecade in fuel management file.",STOP);

	       if (finalDecade < pfuelcoresites->TimeStep_Fuel) finalDecade=1;//Bu, Rencang April 28, 2009

		else finalDecade=finalDecade/pfuelcoresites->TimeStep_Fuel;

	//printf("finalDecade: %d\n", finalDecade);

	

	// REad in the re-entry interval

	if (!fscanc(inFile, "%d", &reentryInterval) || reentryInterval<0 || reentryInterval>9000)

		errorSys("Error when read in reentryInterval in fuel management file.",STOP);

	       if (reentryInterval < pfuelcoresites->TimeStep_Fuel) reentryInterval=1;//Bu, Rencang April 28, 2009

		else reentryInterval=reentryInterval/pfuelcoresites->TimeStep_Fuel;

	//printf("reentryInterval: %d\n", reentryInterval);

	

	// Read in the proportion for treatment

	if (!fscanc(inFile, "%f", &proportion) || proportion <0 || proportion >1.0)

		errorSys("Error when read in proportion in fuel management file.",STOP);

	//printf("proportion: %f\n", proportion);



	// Read in the minmum class of fire risk, if fire risk is higher than this class, then treat

	if (!fscanc(inFile, "%d", &minClass) || minClass <0 || minClass >5)

		errorSys("Error when read in minClass in fuel management file.",STOP);

	//printf("minClass: %d\n", minClass);

		/*

	if (fscanc(inFile,"%d",&temp)!=1)

		errorSys("Error reading in fire modifer from fuel.dat file.",STOP);

	useFireForTreatment = temp;

	//printf("Use fire or not %d\n",useFireForTreatment);

	*/

	

	// allocate a new ranking algorithm

	if(rankAlgorithm > 0 && minClass >= 0)

		itRankAlgorithm = new FuelTreatmentRank(manageAreaIdentify,rankAlgorithm, minClass);



	// Read in the treatment prescription on fine fuel	

	for (i=0; i<=FUELCATEGORY; i++) {

		if (fscanc(inFile,"%d",&temp)!=1)

			errorSys("Error when read in fire modifer in fuel management file.",STOP);

		FineFuelManagement[i] = temp;

	}



	// Read in the treatment prescription on coarse fuel

	for (i=0; i<=FUELCATEGORY; i++) {

		if (fscanc(inFile,"%d",&temp)!=1)

			errorSys("Error when read in fire modifer in fuel management file.",STOP);

		CoarseFuelManagement[i] = temp;

	}

}



FuelManEvent::~FuelManEvent()

{

	// Clear the memory

	if (FineFuelManagement) delete [] FineFuelManagement;

	if (CoarseFuelManagement) delete [] CoarseFuelManagement;

	if (Label)	delete [] Label;



}





int FuelManEvent::FineFuelManager(int finefuel) {

	// This function return the fine fuel loads after treatment

	//if (finefuel < 0)	return FineFuelManagement;

	//else return min (FUELCATEGORY, finefuel + FineFuelManagement);	

	

	int ff = finefuel;

	if (finefuel <0)	finefuel = 0;	

	else if (finefuel > FUELCATEGORY)

		finefuel = FUELCATEGORY;

	ff = FineFuelManagement[finefuel];

	if (ff <0)	ff = 0;

	else if (ff >FUELCATEGORY)		ff = FUELCATEGORY;	

	return ff;

	

}





int FuelManEvent::CoarseFuelManager(int coarsefuel) {

	// This function return the coarse fuel loads after treatment



	//if (coarsefuel < 0)	return CoarseFuelManagement;

	//else return min (FUELCATEGORY, coarsefuel + CoarseFuelManagement);

	

	int cf = coarsefuel;

	if (coarsefuel <0)	coarsefuel = 0;

	else if (coarsefuel > FUELCATEGORY)

		coarsefuel = FUELCATEGORY;

	cf = CoarseFuelManagement[coarsefuel];

	if (cf <0)	cf = 0;

	else if (cf >FUELCATEGORY)		cf = FUELCATEGORY;

	return cf;

	

}

//<Add By Qia on May 19 2009>

float** FuelManEvent::damage_FireinFuel(LDPOINT p, int fireClass)

	//This will cause damage from the fire at POINT p.	It will return the

	//actual number of cohorts killed.	Class is the return value of the fire

	//class.

	//Note p.y is row, p.x is col

	

	{

		int i, j;

		int specAtNum= fuelsites.spNum;
		

		//Perform fire damage.	 

		int count70;

		float tmpBiomass,tmpCarbon;					//changed by houbin, double(origin),float(now)
		float tmpBA;								//add by houbin 2/22/2017

		int numCohorts=0; 

		int shade = 0;
		int DeadTree;
		
		//<Add By Qia on Nov 24 2008>

		pfuelcoresites->BefStChg(p.y,p.x);
		LANDUNIT *l;
		l=pfuelcoresites->locateLanduPt(p.y,p.x);
		//</Add By Qia on Nov 24 2008>

		//<Add By Qia on Nov 30 2012>
		SITE * siteptr=(*pfuelcoresites) (p.y,p.x);
		//printf("qia, fire burned \n");

		////////////////////////////////////////////////////////////////////////////comment for testing pfuelcoresites and siteptr
		/*for (int iii = 1; iii <= 12; iii++)
		{
			printf("%d,", siteptr->specAtt(iii)->BioMassCoef);
			printf("%d ", siteptr->specAtt(iii)->longevity);
		}
		printf("\nFUEL5, 50, 8: ");
		printf("%f ", pfuelcoresites->GetGrowthRates(5, 50, 8));

		printf("\nFUEL12, 56, 2: ");
		printf("%f ", pfuelcoresites->GetGrowthRates(12, 56, 2));

		printf("\nFUE11, 22, 0: ");
		printf("%f ", pfuelcoresites->GetGrowthRates(11, 22, 0));
		

		printf("\nFUEL8, 60, 9: ");
		printf("%f ", pfuelcoresites->GetGrowthRates(8, 60, 9));

		printf("\nFUEL9, 38, 12: ");
		printf("%f ", pfuelcoresites->GetGrowthRates(9, 38, 12));

		printf("\nFUEL10, 14, 2: ");
		printf("%f ", pfuelcoresites->GetGrowthRates(10, 14, 2));*/
		////////////////////////////////////////////////////////////////////////////comment for testing pfuelcoresites and siteptr
		//add by houbin 1/7/2017
		int* dtn_in_diff_spec;
		dtn_in_diff_spec = new int[specAtNum];
		
		float* dtnbio_in_diff_spec;
		dtnbio_in_diff_spec = new float[specAtNum];

		float* dtnba_in_diff_spec;
		dtnba_in_diff_spec = new float[specAtNum];

		float** dtn_and_bio_in_diff_spec = new float*[3];
		for (int counter = 0; counter < 3; counter++)
		{
			dtn_and_bio_in_diff_spec[counter] = new float[specAtNum];
		}


		//getchar();
		int Total_DeadTree_in_allage_onespec;
		float Total_DeadTree_bio_allage_onespec;
		float Total_DeadTree_ba_allage_onespec;
		//add by houbin 1/7/2017
		//printf("specAtNum=%d\n",specAtNum);
		int longest_longevity = 0;
		int interval_number = pPDP->interval_number;
		

		for (int iii = 1; iii <= specAtNum; iii++)
		{
			
			if (longest_longevity < siteptr->specAtt(iii)->longevity)
			{
				longest_longevity = siteptr->specAtt(iii)->longevity;
			}
		}
		

		if (l->active())
		{
			for (i = 1; i <= specAtNum; i++)
			{
				//printf("spec%d dtn allage\n",i);
				//printf("longevity=%d\n",siteptr->specAtt(i)->longevity);
				if (siteptr->specAtt(i)->SpType >= 0){
					Total_DeadTree_in_allage_onespec = 0;		//add by houbin 1/7/2017
					Total_DeadTree_bio_allage_onespec = 0;		//add by houbin 2/9/2017
					Total_DeadTree_ba_allage_onespec = 0;		//add by houbin 2/22/2017
					for (j = 1; j <= siteptr->specAtt(i)->longevity / pfuelcoresites->TimeStep; j++){

						for (int k = 1; k <= pfuelcoresites->TimeStep; k++){

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
							float beta1 = fuelsites.fire_betavalues[tolerance_index - 1][0];
							float beta2 = fuelsites.fire_betavalues[tolerance_index - 1][1];
							float beta3 = fuelsites.fire_betavalues[tolerance_index - 1][2];
							float X_value = fuelsites.fire_X2values[severity_index - 1];
							double prob_burn_param = beta1 + beta2*pfuelcoresites->GetGrowthRates(i, j, l->ltID) + beta3*X_value;
							double prob_burn = 1.0 / (1.0 + exp(0.0 - prob_burn_param));
							int tree_num_agecohort = siteptr->SpecieIndex(i)->getTreeNum(j, i);
							DeadTree = tree_num_agecohort*prob_burn;
							tmpBiomass = exp(pfuelcoresites->GetBiomassData(siteptr->specAtt(i)->BioMassCoef, 1) + pfuelcoresites->GetBiomassData(siteptr->specAtt(i)->BioMassCoef, 2)
								*log(pfuelcoresites-> GetGrowthRates(i, j, l->ltID)))*DeadTree / 1000.00;
							tmpBA = pfuelcoresites->GetGrowthRates(i, j, l->ltID) * pfuelcoresites->GetGrowthRates(i, j, l->ltID) / 40000 * 3.1415926* DeadTree;
							/*if (j == 1 || j == 2){
								printf("spec%d at age%d dtn=%d\t", i, (j - 1)*pfuelcoresites->TimeStep + k, DeadTree);
								}
								getchar();
								getchar();*/
							//printf("%d ",DeadTree);
							if (j == 1 || j == 2 || j == 3 || j == 4 || j == 5){
								//printf("specnum=%d,age=%d,deadtree=%d\n", i, j, DeadTree);//houbin 12/12/2016
							}


							siteptr->SpecieIndex(i)->setTreeNum(j, i, tree_num_agecohort - DeadTree);
							if ((p.x == 293 && p.y == 378) ||
								(p.x == 152 && p.y == 331) ||
								(p.x == 855 && p.y == 728) ||
								(p.x == 223 && p.y == 530) ||
								(p.x == 172 && p.y == 63)
								)
							{
								printf("debug fire: (x: %d y: %d): spec: %d, age: %d, prob: %lf, kill tree: %d\n", p.x, p.y, i, j, prob_burn, (int)DeadTree);
								printf("Tolerance: %d, b1: %f, b2: %f, b3: %f, growth: %f, severity: %d, X: %f, prob: %lf\n",
									tolerance_index, beta1, beta2, beta3, pfuelcoresites->GetGrowthRates(i, j, l->ltID), severity_index, X_value, prob_burn);
							}
							if (siteptr->specAtt(i)->maxSproutAge / pfuelcoresites->TimeStep >= j && siteptr->specAtt(i)->MinSproutAge / pfuelcoresites->TimeStep <= j)

							{

								siteptr->SpecieIndex(i)->TreesFromVeg += (int)DeadTree;

							}

							////////////////////////////////////////////////////////////
							/*for (int ii = 0; ii < interval_number; ii++)
							{
								if (ii*longest_longevity / interval_number<j*pfuelcoresites->TimeStep + k - 1 && j*pfuelcoresites->TimeStep + k - 1 <= (ii + 1)*longest_longevity / interval_number)
								{
									//if (DeadTree != 0)
									//{
									//	printf("%d(%d,%d)+%d  ", ii, p.y - 1, p.x - 1, DeadTree);
									//}
									
									pPDP->cDeadTreeNum_ageinterval[p.y - 1][p.x - 1][ii] += DeadTree;
								}

							}*/
							////////////////////////////////////////////////////////////////

							//add by houbin 10/25/2017
							if (pPDP->TrackFlag == 1)
							{
								for (int agerange_index = 1; agerange_index <= pfuelcoresites->GetOutputAgeRangeModuleFlagArray(i - 1, COUNT); agerange_index++)
								{
									if (pfuelcoresites->GetOutputAgeRangeModuleFlagArray(i - 1, agerange_index, 0) < j*pfuelcoresites->TimeStep + k - 1 && j*pfuelcoresites->TimeStep + k - 1 <= pfuelcoresites->GetOutputAgeRangeModuleFlagArray(i - 1, agerange_index, 1))
									{
										pPDP->cDeadTreeNum_spec_agerange[p.y - 1][p.x - 1][(agerange_index - 1)*specAtNum + i - 1] += DeadTree;
										pPDP->cDeadTreeBA_spec_agerange[p.y - 1][p.x - 1][(agerange_index - 1)*specAtNum + i - 1] += tmpBA;
										pPDP->cDeadTreeBio_spec_agerange[p.y - 1][p.x - 1][(agerange_index - 1)*specAtNum + i - 1] += tmpBiomass;
									}
								}
							}
							
							//add by houbin 10/25/2017


							Total_DeadTree_in_allage_onespec += DeadTree;
							Total_DeadTree_bio_allage_onespec += tmpBiomass;
							Total_DeadTree_ba_allage_onespec += tmpBA;
						}

					}
				}
				dtn_in_diff_spec[i - 1] = Total_DeadTree_in_allage_onespec;			//add by houbin 1/7/2017
				dtnbio_in_diff_spec[i - 1] = Total_DeadTree_bio_allage_onespec;
				dtnba_in_diff_spec[i - 1] = Total_DeadTree_ba_allage_onespec;
				
				if ((dtn_in_diff_spec[i - 1] == 0 && dtnbio_in_diff_spec[i - 1] != (float)0) || (dtn_in_diff_spec[i - 1] != 0 && dtnbio_in_diff_spec[i - 1] == (float)0))
				{
					//printf("x,y=%d,%d  ", p.x, p.y);
					printf("\ndtn and dtnbio different1\n");
				}
				//printf("spec%d dtn=%d\t",i,Total_DeadTree_in_allage_onespec);
				/*getchar();
				getchar();*/
			}
		}
		else
		{
			for (i = 1; i <= specAtNum; i++){
				dtn_in_diff_spec[i - 1] = 0;
				dtnbio_in_diff_spec[i - 1] = 0;
				dtnba_in_diff_spec[i - 1] = 0;
			}
		}

		

		/*printf("here1\n");
		for (int z = 0; z < 12; z++)
		{
			printf("%d,%f\t",dtn_in_diff_spec[z], dtnbio_in_diff_spec[z]);
		}*/
		//</Add By Qia on Nov 30 2012>

		//<Add By Qia on Nov 24 2008>

		pfuelcoresites->AftStChg(p.y,p.x);

		//</Add By Qia on Nov 24 2008>

		if (numCohorts&&(gDLLMode&G_FIRE)) 	

			pPDP->sTSLFire[p.y][p.x] = 0; 

		//return numCohorts;

		/*printf("dtn_in_diff_spec length=%d\n", sizeof(dtn_in_diff_spec) / sizeof(dtn_in_diff_spec[0]));
		printf("dtn[0]=%d\n", dtn_in_diff_spec[0]);
		getchar();
		printf("dtn[1]=%d\n", dtn_in_diff_spec[1]);
		getchar();
		printf("dtn[2]=%d\n", dtn_in_diff_spec[2]);
		getchar();
		printf("dtn[3]=%d\n", dtn_in_diff_spec[3]);
		getchar();*/
		for (int iii = 0; iii < specAtNum; iii++)
		{
			dtn_and_bio_in_diff_spec[0][iii] = (float)dtn_in_diff_spec[iii];
		}
		for (int iii = 0; iii < specAtNum; iii++)
		{
			dtn_and_bio_in_diff_spec[1][iii] = (float)dtnbio_in_diff_spec[iii];
		}
		for (int iii = 0; iii < specAtNum; iii++)
		{
			dtn_and_bio_in_diff_spec[2][iii] = (float)dtnba_in_diff_spec[iii];
		}
		//dtn_and_bio_in_diff_spec[1] = dtnbio_in_diff_spec;
		//dtn_and_bio_in_diff_spec[2] = dtnba_in_diff_spec;
		/*printf("here2\n");
		for (int iii = 0; iii < specAtNum; iii++)
		{
			printf("%d,%f\t",dtn_and_bio_in_diff_spec[0][iii],dtn_and_bio_in_diff_spec[1][iii]);
		}*/
		return dtn_and_bio_in_diff_spec;

	}

//</Add By Qia on May 19 2009>

void FuelManEvent::Treatment(int itr, FILE *fuelManageFile, FILE *fuelManOutStand, int FireinFuel) {

	// This function do the fuel treatment event of each iteration, and write the outputs into two files, one record fuel management info, one record stands info.

	//	printf("fuelmanageevent1111-> treatment");

	//	printf("itr = %d, finalDecade = %d, entryDecade = %d, reentryInterval = %d\n", itr, finalDecade, entryDecade, reentryInterval);


	


	int count=0;
	int clear_all_in_this_area;
	if (itr<1 || itr>finalDecade)	return;

	//	printf("itr = %d, finaldecade = %d\n", itr, finaldecade);

	//  If iteration <1 or iteration > final decade, then do nothing

	//else if(((itr-entryDecade)%reentryInterval) >0)	return;
	else if (((itr - entryDecade) % reentryInterval) > 0 || ((itr - entryDecade) % reentryInterval) < 0)			//changed by Houbin Chen 2/13/2017
	{
		List<int> rankedList;
		FUELStand* stand;
		LDPOINT pt;
		SITE* site;
		long TotalActiveCell = 0;
		int* dtn_diff_spec = new int[pfuelcoresites->specNum];
		float* dtnbio_diff_spec = new float[pfuelcoresites->specNum];
		float* dtnba_diff_spec = new float[pfuelcoresites->specNum];
		LANDUNIT *l;
		clear_all_in_this_area = 1;
		TotalActiveCell = itRankAlgorithm->rankStands(rankedList,clear_all_in_this_area);
		//printf("\nafter rank function rankedlist length=%d\n",rankedList.length());
		for (ListIterator<int> it(rankedList); it.more(); it.advance()) {
			stand = (*fuelpstands)(it.current());
			count++;
			//printf("stand %d set DTN because not interval:\t", stand->getId());
			for (StandIterator itStand(*stand); itStand.moreSites(); itStand.gotoNextSite()) {
				pt = itStand.getCurrentSite();
				site = (*pfuelcoresites)(pt.y, pt.x);
				//printf("%d,%d  ", pt.y, pt.x);
				//dtn_diff_spec = damage_FireinFuel(pt, FireinFuel);
				if (pPDP->TrackFlag == 1)
				{
					for (int i = 0; i < fuelsites.spNum; i++)
					{
						dtn_diff_spec[i] = 0;
						dtnbio_diff_spec[i] = 0;
						dtnba_diff_spec[i] = 0;
						for (int agerange_index = 1; agerange_index <= pfuelcoresites->GetOutputAgeRangeModuleFlagArray(i, COUNT); agerange_index++)
						{

							pPDP->cDeadTreeNum_spec_agerange[pt.y - 1][pt.x - 1][(agerange_index - 1)*fuelsites.spNum + i - 1] = 0;
							pPDP->cDeadTreeBA_spec_agerange[pt.y - 1][pt.x - 1][(agerange_index - 1)*fuelsites.spNum + i - 1] = 0;
							pPDP->cDeadTreeBio_spec_agerange[pt.y - 1][pt.x - 1][(agerange_index - 1)*fuelsites.spNum + i - 1] = 0;
						}
					}
					fuelsites.setDTN(dtn_diff_spec, pt.y, pt.x);
					fuelsites.setDTNBIO(dtnbio_diff_spec, pt.y, pt.x);
					fuelsites.setDTNBA(dtnba_diff_spec, pt.y, pt.x);
				}
				
				
			}
			//printf("\n");
		}
		//printf("area setDTN count=%d",count);
		return;
	}

	//  use tre-entry interval to decide whether treat or not.



	//	printf("fuelmanageevent2222-> treatment");

	//printf("come to reatment");

	List<int> rankedList;

	FUELStand* stand;

	LDPOINT pt;

	SITE* site;

	long TotalTreatCell = 0;

	long TotalActiveCell = 0;

	long budgetTreat = 0;

	float treatProportion = 0;

	int i, standTreatCells = 0;;

	char FFLoad = 0, CFLoad = 0, Intensity = 0, Probility = 0, Risk = 0;

	for (i = 0; i < 6; i++) {

		FFTreat[i] = 0;

		CFTreat[i] = 0;

	}

	int* dtn_diff_spec = new int[pfuelcoresites->specNum];
	float* dtnbio_diff_spec = new float[pfuelcoresites->specNum];
	float* dtnba_diff_spec = new float[pfuelcoresites->specNum];
	float** dtn_and_bio_diff_spec;
	dtn_and_bio_diff_spec = new float*[3];

	int daopoint1 = 0;

	for (int iii = 0; iii < 3; iii ++)
	{
		dtn_and_bio_diff_spec[iii] = new float[pfuelcoresites->specNum];
	}

	LANDUNIT *l;

	clear_all_in_this_area = 0;

	TotalActiveCell = itRankAlgorithm->rankStands(rankedList,clear_all_in_this_area);
	//printf("\nafter rank function rankedlist length=%d\n", rankedList.length());
	// get the total active cells

	budgetTreat = (long)((float)TotalActiveCell * proportion);

	// get the budget treat cells

	//	printf("fuelmanageevent444444444-> treatment");



	for (ListIterator<int> it(rankedList); it.more(); it.advance()) {

		// cycle to treat each seleted stand



		//		printf("fuelmanageevent5555555555-> treatment");



		long FFTotal = 0, CFTotal = 0, IntensityTotal = 0, RiskTotal = 0;



		//        stand = stands(it.current());

		stand = (*fuelpstands)(it.current());
		count++;
		// get the current stand from the list

		//printf("Process fuel treatment of %d \n", it.current());	

		float risk = stand->getMeanFireRisk();

		//if (stand->getId() == 6808 || stand->getId() == 7451 || stand->getId() == 6979 || stand->getId() == 6399 || stand->getId() == 7165) printf("stand%d,before risk = %d", stand->getId(), risk);

		// get the potential fire risk of the stand

		//printf("%d : Fire risk: %f   \n",it.current(), risk);


		//printf("stand %d set DTN:\t",stand->getId());
		for (StandIterator itStand(*stand); itStand.moreSites(); itStand.gotoNextSite()) {

			// cycle to treat the cells/sites of the seleted stand

			pt = itStand.getCurrentSite();

			// pt is the x and y of the current cell/site

			//		site = sites(pt.y,pt.x);

			site = (*pfuelcoresites)(pt.y, pt.x);

			//				printf("6-> treatment");



			// get the current site
			//printf("%d,%d,%d ",pt.y,pt.x,pPDP->cDeadTreeNum[165][148][0]);
			//printf("%d,%d ", pt.y, pt.x);
			if (pt.y == 166 && pt.x == 149)
			{
				daopoint1 = 1;
			}
			if (pfuelcoresites->locateLanduPt(pt.y, pt.x)->active()) {

				//original landis4.0: site->landUnit->active()

				//Changed By Qia on Oct 13 2008



				// if landtype is active, then continue

				//				l=sites(pt.y,pt.x)->landUnit;  //changed by Vera


				/*if (pt.y == 166 && pt.x == 149)
				{
					printf("point1 active\n");
				}
				if (pt.y == 166 && pt.x == 152)
				{
					printf("point2 active\n");
				}*/
				l = pfuelcoresites->locateLanduPt(pt.y, pt.x);


				//original landis4.0: (*pfuelcoresites)(pt.y,pt.x)->landUnit

				//changed By Qia on Oct 13 2008



				// get the land type



				// For fine fuel treatment

				// Shang 03/04

				//FFLoad = fuelsites(pt.y, pt.x)->FFLoad;

				FFLoad = fuelsites.getFFLoad(pt.y, pt.x);

				// get the fine fuel load of the current site

				if (FFLoad <0)		FFLoad = 0;

				else if (FFLoad> 5)	FFLoad = 5;

				// correct the wrong fuel load

				FFTreat[FFLoad]++;

				// record the treated sites for each fine fuel load
				//printf("oldffload=%d",FFLoad);
				FFLoad = FineFuelManager(FFLoad);
				//printf("newffload=%d",FFLoad);

				//printf("area%d,finefuelmanager:", stand->FuelgetManagementAreaId());
				//for (i = 1; i < 6; i++){
				//printf("%d", FineFuelManagement[i]);
				//}
				//printf("\n");
				//<Add By Qia on May 19 2009>

				if (FireinFuel > 0){
					//printf("site(%d,%d) ", pt.y, pt.x);
					//printf("\ncalculating:");
					dtn_and_bio_diff_spec = damage_FireinFuel(pt, FireinFuel);      //changed by houbin 1/10/2017



					/*printf("\nhere3");
					for (int iii = 0; iii < 12; iii++)
					{
						printf("%f,%f\t", dtn_and_bio_diff_spec[0][iii], dtn_and_bio_diff_spec[1][iii]);
					}*/



					for (int iii = 0; iii < pfuelcoresites->specNum; iii++)
					{
						dtn_diff_spec[iii] = (int)dtn_and_bio_diff_spec[0][iii];
					}


					

					//dtnbio_diff_spec = dtn_and_bio_diff_spec[1];
					//dtnba_diff_spec = dtn_and_bio_diff_spec[2];
					for (int iii = 0; iii < pfuelcoresites->specNum; iii++)
					{
						dtnbio_diff_spec[iii] = dtn_and_bio_diff_spec[1][iii];
					}
					for (int iii = 0; iii < pfuelcoresites->specNum; iii++)
					{
						dtnba_diff_spec[iii] = dtn_and_bio_diff_spec[2][iii];
					}

					for (int iii = 0; iii < pfuelcoresites->specNum; iii++)
					{
						if ((dtnbio_diff_spec[iii] == (float)0 && dtn_diff_spec[iii] != 0) || (dtnbio_diff_spec[iii] != (float)0 && dtn_diff_spec[iii] == 0))
						{
							printf("\ndtn and dtnbio different2\n");
						}
					}

					/*printf("\nhere4");
					for (int iii = 0; iii < 12; iii++)
					{
						printf("%d,%f\t",dtn_diff_spec[iii],dtnbio_diff_spec[iii]);
					}*/



					//printf("%d\n", (sizeof(dtn_diff_spec) / sizeof(dtn_diff_spec[0])));
					/*for (int iii = 0; iii <= (sizeof(dtn_diff_spec) / sizeof(dtn_diff_spec[0])); iii++){
						printf("%d ", dtn_diff_spec[iii]);
					}
					printf(" ");*/
				}

					//</Add By Qia on May 19 2009>

					// get the treat prescription for fine fuel

					// Shang 03/04

					//fuelsites(pt.y, pt.x)->FFLoad = FFLoad;

				if (pPDP->TrackFlag == 1)
				{
					fuelsites.setDTN(dtn_diff_spec, pt.y, pt.x);
					fuelsites.setDTNBIO(dtnbio_diff_spec, pt.y, pt.x);
					fuelsites.setDTNBA(dtnba_diff_spec, pt.y, pt.x);
				}
					
					/*if (daopoint1 == 1)
					{
						printf(" ");
						for (int iii = 0; iii < pfuelcoresites->specNum; iii++)
						{
							printf("%d-", pPDP->cDeadTreeNum[pt.y-1][pt.x-1][iii]);
						}
						printf("^^");
						for (int iii = 0; iii < pfuelcoresites->specNum; iii++)
						{
							printf("%d-", pPDP->cDeadTreeNum[165][148][iii]);
						}
						printf("\n");
					}
					else
					{
						if (dtn_diff_spec[0] != 0)
						{
							for (int iii = 0; iii < pfuelcoresites->specNum; iii++)
							{
								printf("%d-", pPDP->cDeadTreeNum[pt.y - 1][pt.x - 1][iii]);
							}
							printf("^^");
							for (int iii = 0; iii < pfuelcoresites->specNum; iii++)
							{
								printf("%d-", pPDP->cDeadTreeNum[165][148][iii]);
							}
							printf("\n");
						}
					}*/

					/*if (pt.y == 166 && pt.x == 149)
					{
						printf("point1 active set finish\n");
					}
					if (pt.y == 166 && pt.x == 152)
					{
						printf("point2 active set finish\n");
					}*/
					fuelsites.setFFLoad(FFLoad, pt.y, pt.x);

					// save the treated fine fuel into fuelsite

					FFTotal += FFLoad;

					// record the total fine fuel loads for stand





					// For coarse fuel treatment

					// Shang 03/04

					//CFLoad = fuelsites(pt.y, pt.x)->CFLoad;

					CFLoad = fuelsites.getCFLoad(pt.y, pt.x);

					// get the coarse fuel load of the current site

					if (CFLoad < 0)		CFLoad = 0;

					else if (CFLoad>5)	CFLoad = 5;

					// correct the wrong coarse fuel loads

					CFTreat[CFLoad]++;

					// record the treated sites for each coarse fuel load

					//printf("Before CF: %d   ",CFLoad);

					CFLoad = CoarseFuelManager(CFLoad);

					// get the treat prescription for coarse fuel

					// Shang 03/04

					//fuelsites(pt.y, pt.x)->CFLoad = CFLoad;

					fuelsites.setCFLoad(CFLoad, pt.y, pt.x);

					// save the treated coarse fuel into fuelsite

					//printf("After CF: %d   ",CFLoad);

					CFTotal += CFLoad;

					// record the total coarse fuel loads for stand

					//<Add By Qia on Nov 19 2008>

					fuelsites.BefStChg(pt.y, pt.x);

					//</Add By Qia on Nov 19 2008>

					// change beginCFDecompositionClass and time of coarse fuel accumulation

					fuelsites(pt.y, pt.x)->timeOfCFAccumulation = fuelsites.getCFAccumulationTime(l->ltID, CFLoad);

					fuelsites(pt.y, pt.x)->beginCFDecompClass = CFLoad;

					//<Add By Qia on Nov 19 2008>

					fuelsites.AftStChg(pt.y, pt.x);

					//</Add By Qia on Nov 19 2008>

					//printf("Time: %d   \n",fuelsites(pt.y,pt.x)->timeOfCFAccumulation);



					// Adujust fire intensity

					if (CFLoad > 5) CFLoad = 5;

					if (CFLoad <0) CFLoad = 0;

					if (FFLoad > 5) FFLoad = 5;

					if (FFLoad < 0) FFLoad = 0;

					Intensity = fuelrule.getFireIntensity((int)FFLoad, (int)CFLoad);

					if (Intensity>5)	Intensity = 5;

					// Shang 03/04

					//fuelsites(pt.y, pt.x)->fireIntensity = Intensity;

					fuelsites.setFireIntensityClass(Intensity, pt.y, pt.x);



					IntensityTotal += Intensity;



					// Adjust fire risk

					// Shang 03/04

					//Probility = fuelsites(pt.y, pt.x)->fireProbility;

					Probility = fuelsites.getFireProbabiltyClass(pt.y, pt.x);

					if (Probility > 5)		Probility = 5;

					if (Intensity <= 0 || Probility <= 0)	Risk = 0;

					else	Risk = (char)fuelrule.fireRisk[Probility - 1][Intensity - 1];


					//if (stand->getId() == 6808 || stand->getId() == 7451 || stand->getId() == 6979 || stand->getId() == 6399 || stand->getId() == 7165) printf("stand%d %d,%d fireprob=%d fireintens=%d cfload=%d ffload=%d\n", stand->getId(), pt.x, pt.y, Probility, Intensity, CFLoad, FFLoad);

					// Shang 03/04

					//fuelsites(pt.y, pt.x)->fireRisk = Risk;

					fuelsites.setFireRiskClass(Risk, pt.y, pt.x);

					RiskTotal += Risk;



					fuelsites.fuelManSite[(pt.y - 1)*fuelsites.cols + pt.x - 1].sTSLFUELMAN = 0;

					fuelsites.fuelManSite[(pt.y - 1)*fuelsites.cols + pt.x - 1].cFUELMANEvent = (char)manageAreaIdentify;



					//(pPDP->sTSLFUELMAN)[pt.y][pt.x] = 0;

					//(pPDP->cFUELMANEvent)[pt.y][pt.x] = (short)manageAreaIdentify;

				}
			else{
				/*if (pt.y == 166 && pt.x == 149)
				{
					printf("point1 not active\n");
				}
				if (pt.y == 166 && pt.x == 152)
				{
					printf("point2 not active\n");
				}*/
				l = pfuelcoresites->locateLanduPt(pt.y, pt.x);
				//if (FireinFuel > 0){
				dtn_and_bio_diff_spec = damage_FireinFuel(pt, FireinFuel);      //changed by houbin 1/10/2017
				for (int iii = 0; iii < pfuelcoresites->specNum; iii++)
				{
					dtn_diff_spec[iii] = (int)dtn_and_bio_diff_spec[0][iii];
				}

				/*if (daopoint1 == 1)
				{
					printf(" ");
					for (int iii = 0; iii < pfuelcoresites->specNum; iii++)
					{
						printf("%d-", dtn_diff_spec[iii]);
					}
					printf("\n");
				}*/

				for (int iii = 0; iii < pfuelcoresites->specNum; iii++)
				{
					dtnbio_diff_spec[iii] = dtn_and_bio_diff_spec[1][iii];
				}
				for (int iii = 0; iii < pfuelcoresites->specNum; iii++)
				{
					dtnba_diff_spec[iii] = dtn_and_bio_diff_spec[2][iii];
				}
				//dtnbio_diff_spec = dtn_and_bio_diff_spec[1];
				//dtnba_diff_spec = dtn_and_bio_diff_spec[2];
				//}

				for (int iii = 0; iii < 12; iii++)
				{
					if ((dtnbio_diff_spec[iii] == (float)0 && dtn_diff_spec[iii] != 0) || (dtnbio_diff_spec[iii] != (float)0 && dtn_diff_spec[iii] == 0))
					{
						printf("\ndtn and dtnbio different2 not active\n");
					}
				}

				if (pPDP->TrackFlag == 1)
				{
					fuelsites.setDTN(dtn_diff_spec, pt.y, pt.x);
					fuelsites.setDTNBIO(dtnbio_diff_spec, pt.y, pt.x);
					fuelsites.setDTNBA(dtnba_diff_spec, pt.y, pt.x);
				}
				
				/*if (pt.y == 166 && pt.x == 149)
				{
					printf("point1 not active set finish\n");
				}
				if (pt.y == 166 && pt.x == 152)
				{
					printf("point2 not active set finish\n");
				}*/
			}

			}
			//printf("\n");




			standTreatCells = stand->itsFuelTreatmentSites;

			// get the sites number of the stand

			if (standTreatCells > 0) {

				stand->itMeanFineFuel = FFTotal / standTreatCells;

				// calculate the mean fine fuel load of the stand

				stand->itMeanCoarseFuel = CFTotal / standTreatCells;

				// calculate the mean coarse fuel load of the stand

				stand->itMeanFireIntensity = IntensityTotal / standTreatCells;

				// calculate the mean fire intensity of the stand

				stand->itMeanFireRisk = risk;

				// save the mean fire risk of the stand

			}

			else {

				stand->itMeanFineFuel = 0;

				stand->itMeanCoarseFuel = 0;

				stand->itMeanFireIntensity = 0;

				stand->itMeanFireRisk = 0;

			}



			// output fuel management record for each stand

			fprintf(fuelManOutStand, "%4d %4d ", itr, manageAreaIdentify);

			// output the management area identify

			//fprintf(fuelManOutStand, "%6d %6d ", it.current(), standTreatCells, risk);
			fprintf(fuelManOutStand, "%6d %6d ", it.current(), standTreatCells);
			fflush(fuelManOutStand);

			//fprintf(fuelManOutStand, "%6.2f %6.2f ", stand->itMeanFineFuel, FFTotal/standTreatCells);

			//fprintf(fuelManOutStand, "%6.2f %6.2f ", stand->itMeanCoarseFuel, CFTotal/standTreatCells);

			//fprintf(fuelManOutStand, "%6.2f %6.2f ", stand->itMeanFireProbability, stand->itMeanFireProbability);

			//fprintf(fuelManOutStand, "%6.2f %6.2f ", stand->itMeanFireIntensity, IntensityTotal/standTreatCells);
			if ((int)standTreatCells == 0){
				fprintf(fuelManOutStand, "%6.2f %6.2f ", risk, float(0));
			}
			else{
				fprintf(fuelManOutStand, "%6.2f %6.2f ", risk, (float)RiskTotal / (float)standTreatCells);
			}





			fprintf(fuelManOutStand, "\n");



			TotalTreatCell += standTreatCells;

			if (TotalTreatCell >= budgetTreat)		break;

			// if total treated sites > budget sites. then break the cycle

		}
		//printf("area setDTN count=%d\n", count);


		// output fuel management

		fprintf(fuelManageFile, "Iteration  %d  ", itr);

		fprintf(fuelManageFile, "ManageAreaID: %d   FineFuel Treatment:  ", manageAreaIdentify);

		for (i = 0; i < 6; i++)

			fprintf(fuelManageFile, "   %d->%d   %6d   ", i, FineFuelManagement[i], FFTreat[i]);

		//fprintf(parameters.fuelManageFile,"\n");



		fprintf(fuelManageFile, "CoarseFuel Treatment:  ");

		for (i = 0; i < 6; i++)

			fprintf(fuelManageFile, "   %d->%d   %6d   ", i, CoarseFuelManagement[i], CFTreat[i]);

		treatProportion = (float)TotalTreatCell / (float)TotalActiveCell;

		fprintf(fuelManageFile, "  Total treated sites: %6d   %8.6f of total %6d  active management area\n", TotalTreatCell, treatProportion, TotalActiveCell);
		fflush(stdout);
		fflush(fuelManageFile);




	}




