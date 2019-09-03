#ifndef AGELIST_H
#define AGELIST_H



#include <stdio.h>



#ifdef LANDIS40CORE_EXPORTS
	#define DLLCLASS   __declspec( dllexport )
#else
	#define DLLCLASS   __declspec( dllimport )
#endif



class DLLCLASS AGELIST
{
public:
	void AGELISTAllocateVector(int index);

	AGELIST();

	AGELIST(const AGELIST& agelist);

	~AGELIST();


	void clear();           //Clear age list.

	void set(int);          //Set an age to true.

	void reset(int);        //Reset an age to false.

	int query();            //Returns true if any ages are present in age list, false otherwise.

	int query(int);         //Return true if age is in list, false otherwise.

	int query(int, int);    //Return true if an age exists in the given range bounded by two integers.

	int youngest();         //Return the youngest age.

	int oldest();           //Return the oldest age.

	int number();           //Return the number of age clases present.

	void read(FILE*);       //This shall read the age list from a file.

	void readBinary(FILE*);//Add By Qia on Feb 26 2009

	void readTreeNum(FILE*, int); //Add By Qia on July 6 2009

	void write(FILE*);      //This shall write the age list to a file.

	void dump();            //This shall dump the age list to the CRT.


	int getAgeVector(int i);

	int getAgeVectorNum();

	int getTimeStep();

	unsigned int getTreeNum(int n, int specIndex);

	void setTreeNum(int n, int specIndex, unsigned int age);

	void GrowTree();

	AGELIST& operator=(const AGELIST&);


protected:

	unsigned int* agevector;

};







class DLLCLASS timestep
{
public:

	void settimestep(unsigned int time_step_in);

	void getSpecNum(unsigned int num);

	void Setlongevity(int i, unsigned int num);

	~timestep();

	unsigned int Time_step;

	unsigned int SpecNum;

};



#undef DLLCLASS



#endif

