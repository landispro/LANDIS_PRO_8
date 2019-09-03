#ifndef SPECIE_H
#define SPECIE_H


#include <stdio.h>

#include "agelist.h"



#ifdef LANDIS40CORE_EXPORTS
	#define DLLCLASS   __declspec( dllexport )
#else
	#define DLLCLASS   __declspec( dllimport )
#endif



class DLLCLASS SPECIE : public AGELIST
{
	friend class RECLASS;
	friend int reclassificationValue(SPECIE*);

public:
	short vegPropagules;      //Number of years of vegetative propagules present.
	short disPropagules;      //Number of years of dispersed propagules present.

	unsigned long AvailableSeed;

	unsigned int TreesFromVeg;
	unsigned int MatureTree;

public:

	void updateDispropagules(int maturity);

	void SetMatureTreeValue(int value);

	int GetMatureTreeValue();

	SPECIE();

	~SPECIE();

	void clear();             //Clear all values.

	void kill(int, int);      //Kill trees in a given age range.

	//Kill trees in a given range if and only if the ages are present in a mask were the mask is represented by type AGELIST.
	void kill(int, int, AGELIST);  

	void read(FILE*);         //Read a specie from a file.

	void readTreeNum(FILE*, int specIndex);

	void write(FILE*);        //Write a specie to a file.

	void dump();              //Dump specie data to the CRT.

	void initilizeDisPropagules(int maturity);

	void birth();         //Adds trees to age ten.

	void kill(int i);  //Kills trees of a given age class.

};



#undef DLLCLASS

#endif
