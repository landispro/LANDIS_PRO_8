#ifndef _Locate_seeds_H  
#define _Locate_seeds_H  

#include <process.h>
#include <string>
#include <cassert>
#include <queue>
#include <windows.h>
#include <iostream>
#include <chrono>
#include <random>
#include <functional>
#include <array>
#include "Lock.h"
using namespace std;


#ifdef LANDIS40CORE_EXPORTS
	#define DLLCLASS   __declspec( dllexport )
#else
	#define DLLCLASS   __declspec( dllimport )
#endif


class DLLCLASS seeding_prob
{

public:
	seeding_prob();

	~seeding_prob();

	void cal_seeding_prob(const int cell_size, const int specNum_in, const int* speces_maxD_in, const int* specesefctD_in, const int snr_in, const int snc_in);

	double get_seed_prob(const int spec_id, const int relative_row, const int relative_col);
	
	void print() const;

	void process_seeds_on_site(int k, int currow, int curcol, unsigned long** numseeds, const unsigned long lc_numseeds);

private:

	void cal_prob4square(const int spec_id, const int numcellside);

	void Gen_dist_vect(const int k, const int numcell_all, const double minprob, const double maxprob);

	//max_dist: the max distance a seed of a species can travel
	int init4each(const float max_dist, const float efctdist, const int spec_id);

	double prob_base(const float x, const float y) const;
	double cal_rect_prob(const float x, const float y) const;

	double cal_probability(const float x_cor1, const float x_cor2, const float y_cor1, const float y_cor2, const int relative_row, const int relative_col) const;
	
	int check_prob(const double x_val, const int n);

private:
	
	int specNum;
	int snr;
	int snc;
	
	float cell_size;
	float alpha;
	int prob_n;

	double constsca;

	Mutex mut_ex;
	
private:
	
	int* numcell_all_list;

	double*** prob4square;
	double**  cdf4species;
	double*	  minprob4spe;

	int* hilmt4spec; //record hilmt for each species
	int* wghts;
};

#endif  
