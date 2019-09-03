#define _CRT_SECURE_NO_WARNINGS

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "system1.h"
#include "map8.h"
#include "specie.h"
#include "site.h"
#include "landis.h"
#include "reclass.h"
#include "error.h"




//This will perform a reclassification based on the underlying map 
//land units.  It will perform the reclassification on the global 
//object sites and place the results in m.
void luReclass(MAP8 &m)
{
	const int snr = sites.numRows();
	const int snc = sites.numColumns();

	m.dim(snr, snc);

	m.rename("Landtype");

	const int lun = landUnits.number();


	for (int k = 0; k < lun; k++)
		landUnits(k)->index = k;


	for (int i = snr; i >= 1; i--)
	{
		for (int j = 1; j <= snc; j++)
			m(i, j) = sites.locateLanduPt(i, j)->index;
	}

	if (lun < maxLeg)
	{
		for (int i = 0; i < lun; i++)  
			m.assignLeg(i, landUnits(i)->name);
	}
	else
	{
		for (int i = 0; i < maxLeg; i++)
			m.assignLeg(i, landUnits(i)->name);
	}
}





void ageReclassYoungest(MAP8 &m)
{
	const int snr = sites.numRows();
	const int snc = sites.numColumns();

	m.dim(snr, snc);

	m.rename("Age class representation");

	for (int j = 1; j < MapmaxValue; j++)
		m.assignLeg(j, "");

	char str[200];
  	//J.Yang maxLeg is defined as 256 in map8.h, therefore, maximum age cohorts it can output is 254 
	for (int i = 1; i < MaxValueforLegend - 4; i++)
	{
		const int lc_val = (i - 1) * sites.TimeStep;

		sprintf(str, "%3d - %3d yr", lc_val + 1, lc_val + sites.TimeStep);

		m.assignLeg(i, str);
	}

	
	m.assignLeg(0, "NoSpecies");
	m.assignLeg(MaxValueforLegend - 1, "N/A");
	m.assignLeg(MaxValueforLegend - 2, "Water");
	m.assignLeg(MaxValueforLegend - 3, "NonForest");

	sprintf(str, "	  >%3d yr", (MaxValueforLegend - 4 - 1) * sites.TimeStep);
	m.assignLeg(MaxValueforLegend - 4, str);

	
	for (int i = snr; i >= 1; i--)
	{
		for (int j = 1; j <= snc; j++)
		{
			LANDUNIT* pLanduPt = sites.locateLanduPt(i, j);
			SITE* lc_site = sites(i, j);

			if (pLanduPt->active())
			{
				m(i, j) = 0;

				int myage = MapmaxValue;

				SPECIE *s = lc_site->first();

				int temp;

				while (s != NULL)
				{
					if ((temp = s->youngest()) < myage && s->youngest() > 0)
						myage = temp;

					s = lc_site->next();
				}

				if (myage == MapmaxValue)
					myage = 0;
				else
					myage = myage / sites.TimeStep;

				m(i, j) = myage;
			}
			else if (pLanduPt->lowland())
				m(i, j) = MaxValueforLegend - 3;
			else if (pLanduPt->water())
				m(i, j) = MaxValueforLegend - 2;
			else
				m(i, j) = MaxValueforLegend - 1;

		}

	}

}





//This will perform a reclassification based upon the oldest cohort upon a landis stand.  
void ageReclass(MAP8 &m)
{
	const int snr = sites.numRows();
	const int snc = sites.numColumns();

	m.dim(snr, snc);
	m.rename("Age class representation");

	for (int j = 1; j < MapmaxValue; j++)
		m.assignLeg(j, "");


	char str[200];
	//J.Yang hard coding changing itr*sites.TimeStep to itr
	//J.Yang maxLeg is defined as 256 in map8.h, therefore, maximum age cohorts it can output is 254 
	for (int i = 1; i < MaxValueforLegend - 4; i++)  
	{
		const int lc_val = (i - 1) * sites.TimeStep;

		sprintf(str, "%3d - %3d yr", lc_val + 1, lc_val + sites.TimeStep);

		m.assignLeg(i, str);
	}

	m.assignLeg(0, "NoSpecies");
	m.assignLeg(MaxValueforLegend - 1, "N/A");
	m.assignLeg(MaxValueforLegend - 2, "Water");
	m.assignLeg(MaxValueforLegend - 3, "NonForest");

	sprintf(str, "	  >%3d yr", (MaxValueforLegend - 4 - 1) * sites.TimeStep);
	m.assignLeg(MaxValueforLegend - 4, str);


	for (int i = snr; i >= 1; i--)
	{
		for (int j = 1; j <= snc; j++)
		{
			LANDUNIT* pLanduPt = sites.locateLanduPt(i, j);
			SITE* lc_site = sites(i, j);

			if (pLanduPt->active())
			{
				m(i, j) = 0;

				int myage = 0;

				SPECIE *s = lc_site->first();

				int temp;

				while (s != NULL)
				{
					if ((temp = s->oldest()) > myage)
						myage = temp;

					s = lc_site->next();
				}

				myage = myage / sites.TimeStep;

				m(i, j) = myage;
			}
			else if (pLanduPt->lowland())
				m(i, j) = MaxValueforLegend - 3;
			else if (pLanduPt->water())
				m(i, j) = MaxValueforLegend - 2;
			else
				m(i, j) = MaxValueforLegend - 1;

		}

	}

}




//This will output age at 10 year step for each specified species.
//The cohorts can be up to 50 age classes, 0-500 years.
void speciesAgeMap(MAP8 &m, char *ageFile)
{
	const int snr = sites.numRows();
	const int snc = sites.numColumns();

	const int curSp = speciesAttrs.current(ageFile);

	m.dim(snr, snc);

	char str[200];
	sprintf(str, "%s", ageFile);
	m.rename(str);

	for (int i = 1; i < maxLeg - 4; i++)
	{
		const int lc_val = (i - 1) * sites.TimeStep;

		sprintf(str, "%3d - %3d yr", lc_val + 1, lc_val + sites.TimeStep);

		m.assignLeg(i, str);
	}


	m.assignLeg(0, "NotPresent");
	m.assignLeg(MaxValueforLegend - 1, "N/A");
	m.assignLeg(MaxValueforLegend - 2, "Water");
	m.assignLeg(MaxValueforLegend - 3, "NonForest");

	sprintf(str, "	  >%3d yr", (maxLeg - 4 - 1) * sites.TimeStep);
	m.assignLeg(MaxValueforLegend - 4, str);


	for (int i = snr; i >= 1; i--)
	{
		for (int j = 1; j <= snc; j++)
		{
			LANDUNIT* pLanduPt = sites.locateLanduPt(i, j);

			if (pLanduPt == NULL)
				errorSys("Invalid landunit error\n", STOP);

			if (pLanduPt->active())
			{
				m(i, j) = 0;       //where species not presents

				if (sites(i, j) == NULL)
					errorSys("No site\n", STOP);

				SPECIE *s = sites(i, j)->current(curSp);

				if (s == NULL)
					errorSys("No Species\n", STOP);

				if (s->query())
				{
					m(i, j) = (unsigned short)(s->oldest() / sites.TimeStep);

					//maximum longevity is 640 years; Notice 66 means 640 years
					if (m(i, j) > MaxValueforLegend - 4)
						m(i, j) = MaxValueforLegend - 4;
				}
			}
			else if (pLanduPt->water())
				m(i, j) = MaxValueforLegend - 2;
			else if (pLanduPt->lowland())
				m(i, j) = MaxValueforLegend - 3;
			else
				m(i, j) = MaxValueforLegend - 1;
		}
	}
}
