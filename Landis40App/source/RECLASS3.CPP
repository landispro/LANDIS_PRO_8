#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>

#include "system1.h"
#include "specie.h"
#include "sites.h"
#include "landis.h"
#include "reclass3.h"



//This will reclassify sites from user defined class file and the existing age
//maps. Class file is a file containing site descriptions for a set of class.
//Age maps involved in the reclassification need to be created before.
void reclassify(int timeStep, char ageMaps[MAX_SPECIES][80])
{
	const int specAtNum = speciesAttrs.number();
	const int yDim = sites.numRows();
	const int xDim = sites.numColumns();

	FILE* inAgeMap, *inAgeIndex;


	for (int i = 0; i < specAtNum; i++)
	{
		char str[200] = {0};
		char speciesName[10] = {0};

		//read species name from ageIndex file
		sprintf(str, "%s.age", ageMaps[i]);

		if ((inAgeIndex = fopen(str, "r")) == NULL) 
			{ printf("%s%s\n", str, " file not found."); exit(255); }
		else if (fscanc(inAgeIndex, "%s", speciesName) != 1)
			{ printf("%s%s\n", "Error, reading file::", str); exit(255);}

		const int curSp = speciesAttrs.current(speciesName);

		//read age map file from output directory
		sprintf(str, "%s/%s%d.gis", parameters.outputDir, ageMaps[i], timeStep);

		if ((inAgeMap = fopen(str, "rb")) == NULL) { printf("%s%s\n", str, " file not found."); exit(255); }

		unsigned long dest[64] = {0};
		fread((char*)dest, 4, 32, inAgeMap);

		// read inAgeMap
		for (int i = yDim; i > 0; i--)
		{
			for (int j = 1; j <= xDim; j++)
			{
				unsigned char coverType;

				fread(&coverType, 1, 1, inAgeMap);

				if (coverType == 255)          //species absence
				{
					SPECIE *s = sites(i, j)->current(curSp);

					s->clear();
				}
				else if ((coverType != 255) && (coverType >= 3)) //0-empty 1-water 2-nonforest
				{
					SPECIE *s = sites(i, j)->current(curSp);

					s->clear();

					s->set((coverType - 2) * sites.TimeStep);
				}
			}
		}

		fclose(inAgeMap);
	}

	fclose(inAgeIndex);
}

