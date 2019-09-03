// Reclass4.h: interface for the Reclass4 class.

//

//////////////////////////////////////////////////////////////////////

#include "map8.h"

#include "List.h"





//class Reclass4  

//{

//public:

//	Reclass4();

//	~Reclass4();

	void FuelReclass(MAP8&, int, FILE*);				// output the fine & coarse fule into maps

	void FuelReclass_DTN(MAP8&, int, FILE*);			// output the dead tree number in different spec into maps(cf/ffload is 2 dimensional, deadtreenum is 3)

	void FuelReclass_DTNBIO(MAP8&, int, FILE*);			// output the dead tree biomass into maps(cf/ffload is 2 dimensional, deadtreenum is 3)
	
	void FuelReclass_DTNBA(MAP8&, int, FILE*);			// output the dead tree basal area into maps(cf/ffload is 2 dimensional, deadtreenum is 3)

	//void FuelReclass_DTN_age_interval(MAP8&, int, FILE*);// output the dead tree number int different age interval into maps(cf/ffload is 2 dimensional, deadtreenum is 3)

	void FuelReclass_DTN_spec_agerange(MAP8&, int,int, FILE*);//output the dead tree number area different spec&agerange into maps

	void FuelReclass_DTNBA_spec_agerange(MAP8&, int, int, FILE*);//output the dead tree basal area different spec&agerange into maps

	void FuelReclass_DTNBIO_spec_agerange(MAP8&, int, int, FILE*);//output the dead tree biomass different spec&agerange into maps
	
	void fireReclass(MAP8&, MAP8&, MAP8&, FILE *, int);	// output the fire classes

//	void fireReclass(MAP8&, MAP8&, MAP8&);

	int fireProbilityCategory();						// output the fire probility class

//	void PotentialFireProbility();						// output the fire probability

//	int AscendOrderSort(List<float>);

//};



