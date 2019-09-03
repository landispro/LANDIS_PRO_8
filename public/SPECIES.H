#ifndef SPECIES_H
#define SPECIES_H



#include <stdio.h>

#include "specie.h"
#include "specatt.h"
#include "specatts.h"



#ifdef LANDIS40CORE_EXPORTS
	#define DLLCLASS   __declspec( dllexport )
#else
	#define DLLCLASS   __declspec( dllimport )
#endif



class DLLCLASS SPECIES
{
public:
	//This constructor can only be used on the first creation instance of class SPECIES. 
	// It sets the number of different varieties of species in the model.
	SPECIES(int n); 
	
	//Should be used on all instance constructions other than the first.  
	//This may be used on the first instance of construction but should be followed by a call to setNumber before the second construction instance.
	SPECIES(); 

	

	~SPECIES();  

	//Sets the number of species.  Must be called after the first instance of construction and before the second.
	static void setNumber(int);

	//If all instances of class SPECIES are destructed then the reset function may be called. 
	// After it has been called instances may be constructed with a different number of species present.
	static void reset();


	static void attach(SPECIESATTRS*);//This attaches a set of species attributes to all instances of species.

	static int number();              //Returns number of species.


	void read(FILE*);                 //Read in all species data.

	void write(FILE*);                //Write out all species data.

	void dump();                      //Dump set of species data to the CRT.

	SPECIE* operator() (int);         //Referrence a species by species number.

	SPECIE* first();                  //Referrence first species attribute.

	SPECIE* next();                   //Referrence next species attribute.

	SPECIE* current(int);             //Reference by current species number. 

	SPECIESATTR* specAtt();           //Referrence the current species attribute.

	SPECIESATTR* specAtt(int i);           //Referrence the specified species attribute.

	int youngest();                   //Return the youngest age.

	int oldest();                     //Return the oldest age.

	SPECIES& operator=(const SPECIES&);

	SPECIE* SpecieIndex(int n);

private:

	SPECIE *species;                  //Array holding all species information.

	unsigned char currentSpec;        //Current species number being pointed to by first and next access functions.

	static int numSpec;               //Number of species.

	static SPECIESATTRS *speciesAttrs;//Pointer to an attached set of species Attributes.
};

#undef DLLCLASS

#endif
