#define _CRT_SECURE_NO_WARNINGS

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "system1.h"
#include "map8.h"
#include "specie.h"
#include "site.h"
#include "landis.h"
#include "reclass2.h"
#include "error.h"




static int maximum[MAX_RECLASS];   //Maximum table.

static int lc_bool[MAX_RECLASS][MAX_RECLASS];  //Boolean table of class values.

static int numClasses;    //Number of output classes.



//This will reset the reclassification system.
static void reset()
{
	const int specAtnum = speciesAttrs.number();

	for (int i = 1; i <= specAtnum; i++)
	{
		//J.Yang add 0.0...01 here, avoid dividing by 0
		maximum[i - 1] = (int)(speciesAttrs(i)->longevity / (speciesAttrs(i)->reclassCoef + 0.000001f));
	}
}




//This will read in a class description file given the file name
//and the number of classes in the file (m).
void readInClassDescrip(FILE *infile)
{
	const int specAtnum = speciesAttrs.number();

	for (int i = 0; i < MAX_RECLASS; i++)     /// if this is not done the reclass maps
		for (int j = 0; j < MAX_RECLASS; j++) /// do not get reclassified _CSH
			lc_bool[i][j] = 0;

	int i = 1;
	char str[200], sub[200], *p;
	numClasses = 0;

	while (!LDeof(infile))
	{
		fgets(str, 199, infile);

		p = str;

		int numChar;

		if (sscanf(p, "%s%n", sub, &numChar) == 1)
		{
			numClasses++;

			p += numChar + 1;

			while (*p != '\0' && *p != '\n' && *(p - 1) != '\0' && *(p - 1) != '\n')
			{
				if (sscanf(p, "%s%n", sub, &numChar) != 1)
					break;

				p += numChar + 1;

				int bvalue;

				if (sub[0] == '!')
				{
					bvalue = -1;

					strcpy(sub, &sub[1]);
				}
				else
				{
					bvalue = 1;
				}

				for (int j = 1; j <= specAtnum; j++)
				{
					if (strcmp(speciesAttrs(j)->name, sub) == 0)
						lc_bool[i][j] = bvalue;
				}

			}

			i++;
		}
	}
}





//This will calculate the reclassification value given a specie list for a site.
//J.Yang the original coding is equivalent to the AGELIST::number();
//I therefore just use the function instead
int reclassificationValue(SPECIE *s)
{
	return s->number();
}




//This will reclassify a singular site.  M is the number of possible output classes.
int reclassifySite(SITE *site, int m)
{
	float sval[MAX_RECLASS] = {0};

	SPECIE* specie = site->first();

	int j = 1;

	while (specie != NULL)
	{
		float c = (float)specie->oldest() / maximum[j - 1];

		if (c > 1.0)
			c = 1.0f;

		for (int i = 1; i <= m; i++)
		{
			if (lc_bool[i][j] != 0)
			{
				(lc_bool[i][j] > 0) ? (sval[i] += c) : (sval[i] -= c);

				if (sval[i] != 0)
				{
					if (sval[i] > 1.0) 
						sval[i] = 1.0f;

					if (sval[i] < 0.0) 
						sval[i] = 0.0f;
				}
			}
		}

		j++;

		specie = site->next();
	}


	int mx = 0;
	float mxVal = 0.0f;

	for (int i = 1; i <= m; i++)
	{
		if (sval[i] > mxVal)
		{
			mxVal = sval[i];

			mx = i;
		}
	}


	if (mxVal > 0.0)
		return mx;
	else
		return m + 1;
}








//This will reclass the Landis map and return the result m.  
//The map will be reclassified using a method defined in a an output reclassification text file.
void fileReclass(MAP8 &m, const char *fname)
{
	FILE *infile;
	if ((infile = LDfopen((char*)fname, 1)) == NULL) { printf("\n%s", fname); errorSys("Can not open reclassification file", STOP); }

	reset();

	const int snr = sites.numRows();
	const int snc = sites.numColumns();

	char str[200], sub[200];
	sprintf(str, "Reclassification file: %s", fname);

	m.rename(str);
	m.dim(snr, snc);

	m.assignLeg(MaxValueforLegend - 1, "N/A");
	m.assignLeg(MaxValueforLegend - 2, "Water");
	m.assignLeg(MaxValueforLegend - 3, "NonForest");

	int n = 0;
	int i = 1;

	while (!LDeof(infile))
	{
		sprintf(str, "");

		fgets(str, 199, infile);

		if (sscanf(str, "%s", sub) == 1)
		{
			sub[strlen(sub) - 1] = '\0';

			m.assignLeg(i, sub);

			i++;
			n++;
		}
	}

	m.assignLeg(i, "Other");

	for (int j = i + 1; j < maxLeg - 3; j++)
		m.assignLeg(j, "");

	rewind(infile);

	readInClassDescrip(infile);


	for (int i = snr; i >= 1; i--)
	{
		for (int j = 1; j <= snc; j++)
		{
			LANDUNIT* pLanduPt = sites.locateLanduPt(i, j);

			if (pLanduPt->active())
				m(i, j) = (unsigned short)(reclassifySite(sites(i, j), n));
			else if (pLanduPt->lowland())
				m(i, j) = MaxValueforLegend - 3;
			else if (pLanduPt->water())
				m(i, j) = MaxValueforLegend - 2;
			else
				m(i, j) = MaxValueforLegend - 1;
		}
	}

	fclose(infile);
}

