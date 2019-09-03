#include "seeding_prob.h"
#include <cmath>
#include <iomanip>
#include "DEIntegrator.h"


#define PI 3.14159265
#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))



seeding_prob::seeding_prob()
{
	prob4square = NULL;
	cdf4species = NULL;
	minprob4spe = NULL;
	hilmt4spec  = NULL;
	numcell_all_list = NULL;
}




void seeding_prob::cal_seeding_prob(const int cell_size_in, const int specNum_in, const int* speces_maxD, const int* specesefctD, const int snr_in, const int snc_in)
{
	assert(cell_size_in > 0);

	cell_size = cell_size_in; 
	snr = snr_in;
	snc = snc_in;
	specNum = specNum_in;

	prob4square = new double**[specNum]();
	cdf4species = new double* [specNum]();
	minprob4spe = new double  [specNum]();
	numcell_all_list = new int[specNum]();
	hilmt4spec 		 = new int[specNum]();
	
	for(int i=0; i<specNum; i++)
	{
		const int numcellside = init4each(speces_maxD[i], specesefctD[i], i);
		cal_prob4square(i, numcellside);
	}	
}




double seeding_prob::get_seed_prob(const int spec_id, const int relative_row, const int relative_col)
{
	const int hilmt = hilmt4spec[spec_id];
	const int lwlmt = -hilmt;

	const double ret_val = prob4square[spec_id][relative_row - lwlmt][hilmt - relative_col];
	
	return ret_val;
}



/*
[0,0], [0,1],..., [0,2*hilmt+1]
[1,0], [1,1],..., [1,2*hilmt+1]
...
[2*hilmt+1,0], [2*hilmt+1,1],..., [2*hilmt+1,2*hilmt+1]
*/
void seeding_prob::cal_prob4square(const int spec_id, const int numcellside)
{
	wghts = new int [prob_n]();
	wghts[0] = 1;

	for(int i=prob_n-1; i>=1; i--)	
		wghts[prob_n-i] = wghts[prob_n-i-1] * i;

	//================================================================

	const int numcell_all = numcellside * numcellside;

	const int hilmt = hilmt4spec[spec_id];
	const int lwlmt = -hilmt;

	double sum = 0;

	for(int relative_col=hilmt; relative_col>=lwlmt; relative_col--)
	{
		const float y_cor1 = (relative_col - 0.5f) * cell_size;
		const float y_cor2 = (relative_col + 0.5f) * cell_size;

		for(int relative_row=lwlmt; relative_row<=hilmt; relative_row++)
		{
			const float x_cor1 = (relative_row - 0.5f) * cell_size;
			const float x_cor2 = (relative_row + 0.5f) * cell_size;

			if (prob4square[spec_id][relative_row - lwlmt][hilmt - relative_col] < 1.0e-14)
				prob4square[spec_id][relative_row - lwlmt][hilmt - relative_col] = cal_probability(x_cor1, x_cor2, y_cor1, y_cor2, relative_row, relative_col);

			if (prob4square[spec_id][relative_row - lwlmt][hilmt + relative_col] < 1.0e-14)
				prob4square[spec_id][relative_row - lwlmt][hilmt + relative_col] = prob4square[spec_id][relative_row - lwlmt][hilmt - relative_col];

			if (prob4square[spec_id][hilmt - relative_col][relative_row - lwlmt] < 1.0e-14)
				prob4square[spec_id][hilmt - relative_col][relative_row - lwlmt] = prob4square[spec_id][relative_row - lwlmt][hilmt - relative_col];

			if (prob4square[spec_id][hilmt + relative_col][relative_row - lwlmt] < 1.0e-14)
				prob4square[spec_id][hilmt + relative_col][relative_row - lwlmt] = prob4square[spec_id][relative_row - lwlmt][hilmt - relative_col];

			sum += prob4square[spec_id][relative_row - lwlmt][hilmt - relative_col];
		}
	}

	// FILE* fp = fopen("o.txt", "w");

	// for(int relative_col=hilmt; relative_col>=lwlmt; relative_col--)
	// {
	// 	const float y_cor1 = (relative_col - 0.5f) * cell_size;
	// 	const float y_cor2 = (relative_col + 0.5f) * cell_size;

	// 	for(int relative_row=lwlmt; relative_row<=hilmt; relative_row++)
	// 	{
	// 		const float x_cor1 = (relative_row - 0.5f) * cell_size;
	// 		const float x_cor2 = (relative_row + 0.5f) * cell_size;

	// 		fprintf(fp, "%8.3lf", prob4square[spec_id][relative_row - lwlmt][hilmt - relative_col] * 1000);
	// 	}

	// 	fprintf(fp,"\n");
	// }

	// fclose(fp);

	const double minprob = prob4square[spec_id][0][0];
	const double maxprob = prob4square[spec_id][hilmt][hilmt];

	minprob4spe[spec_id] = minprob;
	// printf("min = %e, max = %e sum = %lf\n", minprob, maxprob, sum);
	
	for(int i=0; i<numcell_all; i++)
	{
		assert(minprob <= prob4square[spec_id][0][i]);
		assert(maxprob >= prob4square[spec_id][0][i]);
	}

	delete[] wghts;
	wghts = NULL;
}






int seeding_prob::check_prob(const double x_val, const int n)
{
	int demon = 1;

	int* 	weight = new int   [n]();
	double* x_list = new double[n]();

	for(int i=n-1; i>=1; i--)
		demon *= i;

	constsca = 1 / (2 * PI * demon);
	
	weight[0]   = 1;
	x_list[n-1] = 1;

	for(int i=n-1; i>=1; i--)
	{
		weight[n-i] = weight[n-i-1] * i;
		x_list[i-1] = x_list[i] * x_val;
	}

	double term = 0;

	for(int i=0; i<n; i++)
		term += x_list[i] * weight[i];

	const double prob = 1 - exp(-x_val) * term / demon;
	
	delete[] weight;
	delete[] x_list;

	if(prob > 0.94)
		return 0;
	else
		return -1;
}



int seeding_prob::init4each(const float max_dist, const float specesefctD, const int spec_id)
{
	assert(max_dist > 0);

	static const double alpha_x[9] = {9.233413476451586, 11.228872242412663, 13.062240779188071, 14.794149222537211, 16.454745203680105, 18.061636840199071, 19.626177395384239, 21.156198165839982, 22.657373309062930};
	//we assume the probability of a seed placed in a square area: 2 * max_dist by 2 * max_dist is 0.999
	//then an important value to calculate alpha is 10. please see corresponding document why it is 10.
	for(int i=0; i<9; i++)
	{
		int n = i + 2;

		prob_n = n;

		alpha = alpha_x[i] / pow (max_dist, 1.0 / n);

		const double x_val = alpha * pow (specesefctD, 1.0 / n);

		const int ret_id = check_prob(x_val, n);

		if (ret_id == 0)
			break;

		if(i == 8 && ret_id == -1)
		{
			printf("\n\nplease check effective distance and max distance of the %d th species in SpeciesAtrributes.dat.\n");
			printf("something might be wrong.\n within the effective distance, the cumulated probability is not greater than 0.95\n");
			Sleep(10000);
		}
	}

	// printf("prob_expon = %lf max_dist = %f, specesefctD = %f\n", 1.0 / prob_n, max_dist, specesefctD);

	const int tmp_offset = max_dist / cell_size;
	const int offset = tmp_offset >= 1 ? tmp_offset : 1;
	const int numcellside = 2 * offset + 1;
	const int numcell_all = numcellside * numcellside;
	
	numcell_all_list[spec_id] = numcell_all;
	
	hilmt4spec[spec_id] = offset;

	prob4square[spec_id]    = new double* [numcellside];
	prob4square[spec_id][0] = new double  [numcell_all]();

	for(int i=1; i<numcellside; i++)
		prob4square[spec_id][i] = prob4square[spec_id][i-1] + numcellside;

	cdf4species[spec_id]    = new double [numcell_all];

	return numcellside;
}





seeding_prob::~seeding_prob()
{
	if(prob4square != NULL)
	{
		for (int i=0; i<specNum; i++)
		{
			delete [] prob4square[i][0];
			delete [] prob4square[i];
		}

		delete [] prob4square;
		prob4square = NULL;
	}


	if(cdf4species != NULL)
	{
		for (int i=0; i<specNum; i++)
			delete [] cdf4species[i];

		delete [] cdf4species;
		cdf4species = NULL;
	}


	if(hilmt4spec != NULL)
		delete [] hilmt4spec;
	hilmt4spec = NULL;

	if(numcell_all_list != NULL)
		delete [] numcell_all_list;
	numcell_all_list = NULL;

	if(minprob4spe != NULL)
		delete [] minprob4spe;
	minprob4spe = NULL;	
}



double seeding_prob::cal_probability(const float x_cor1, const float x_cor2, const float y_cor1, const float y_cor2, const int relative_row, const int relative_col) const
{
	assert(x_cor1 < x_cor2);
	assert(y_cor1 < y_cor2);

	const float xcor1 = abs(x_cor1);
	const float xcor2 = abs(x_cor2);
	const float ycor1 = abs(y_cor1);
	const float ycor2 = abs(y_cor2);

	double prob = 0;

	if (relative_row != 0 && relative_col != 0)
	{
		const float x1 = min(xcor1, xcor2);
		const float x2 = max(xcor1, xcor2);
		const float y1 = min(ycor1, ycor2);
		const float y2 = max(ycor1, ycor2);

		prob = cal_rect_prob(x2, y2) - cal_rect_prob(x1, y2) - cal_rect_prob(x2, y1) + cal_rect_prob(x1, y1);
	}
	else
	{
		if (relative_row == 0 && relative_col == 0)
		{
			prob = cal_rect_prob(x_cor2, y_cor2) * 4;
		}
		else if (relative_row == 0)
		{
			assert(relative_col != 0);

			const float y1 = min(ycor1, ycor2);
			const float y2 = max(ycor1, ycor2);

			prob = (cal_rect_prob(x_cor2, y2) - cal_rect_prob(x_cor2, y1)) * 2;
		}
		else
		{
			assert(relative_row != 0 && relative_col == 0);

			const float x1 = min(xcor1, xcor2);
			const float x2 = max(xcor1, xcor2);

			prob = (cal_rect_prob(x2, y_cor2) - cal_rect_prob(x1, y_cor2)) * 2;
		}
	}

	assert(prob >= -1.0e-14 && prob <= 1 + 1.0e-14);

	return prob;
}



//calculate rectangular probability: P(x, y) (x >= 0, y >= 0)
//the rectangular is defined by [0, 0], [x, 0], [x, y], [0, y]
double seeding_prob::cal_rect_prob(const float x, const float y) const
{
	assert(x > 0 && y > 0);

	double prob = 0.25 - constsca * prob_base(x, y);

	return prob;
}



class basefunction
{
public:
	basefunction(double alpha_in, int prob_n, int* wghts_in)
	{
		alpha = alpha_in;
		n = prob_n;
		wghts = wghts_in;

		z_list = new double[n]();		
	}

	~basefunction() { delete[] z_list; }

	void set_onecor(double one_cor_in)
	{
		one_cor = one_cor_in;
	}


	double operator()(double theta) const
	{
		const double z_val = pow(one_cor / cos(theta), 1.0 / n) * alpha;
		
		z_list[n-1] = 1;

		for(int i=n-1; i>=1; i--)
			z_list[i-1] = z_list[i] * z_val;

		double term = 0;
		for(int i=0; i<n; i++)
			term += z_list[i] * wghts[i];

		const double retval = exp(-z_val) * term;
		
		return retval;
	}

private:
	float alpha;
	float one_cor; //one coordinate: x or y
	int n;
	int* wghts;
	double* z_list;
};





double seeding_prob::prob_base(const float x, const float y) const
{
	basefunction basefunc(alpha, prob_n, wghts);
	// int evaluations;
	// double errorEstimate;

	basefunc.set_onecor(x);	
	double theta1 = atan (y / x);
	// double integra1 = DEIntegrator<basefunction>::Integrate(basefunc, 0, theta1, 1e-9, evaluations, errorEstimate);
	double integra1 = DEIntegrator<basefunction>::Integrate(basefunc, 0, theta1, 1e-8);	

	basefunc.set_onecor(y);
	double theta2 = PI / 2 - theta1;
	// double integra2 = DEIntegrator<basefunction>::Integrate(basefunc, 0, theta2, 1e-9, evaluations, errorEstimate);
	double integra2 = DEIntegrator<basefunction>::Integrate(basefunc, 0, theta2, 1e-8);

	// printf("theta1 = %lf, integra1 = %lf, theta2 = %lf, integra2 = %lf\n", theta1, integra1, theta2, integra2);

	double ret_val = integra1 + integra2;

	return ret_val;
}





void seeding_prob::process_seeds_on_site(int spec_id, int currow, int curcol, unsigned long** numseeds, const unsigned long totalnumseeds)
{
	// clock_t start_s=clock();
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();

	const int offset = hilmt4spec[spec_id];

	const int norm_currow = currow - 1;
	const int norm_curcol = curcol - 1;
	
	//sampling============================================================================
	std::default_random_engine generator(seed);
	unsigned long sumofbio = 0;

	//numseeds is the shared variable in multiple threads, if you want parallel process to change it, we need lock
	// mut_ex.Lock();
	
	for (int i = -offset; i<=offset; i++)
	{
		const int spread_row = norm_currow + i;

		if (spread_row < 0 || spread_row >= snr)
			continue;

		const int prob4square_row_id = i + offset;

		for (int j = -offset; j<=offset; j++)
		{
			const int spread_col = norm_curcol + j;

			if (spread_col < 0 || spread_col >= snc)
				continue;

			const int prob4square_col_id = j + offset;

			binomial_distribution<unsigned long> distribution(totalnumseeds, prob4square[spec_id][prob4square_row_id][prob4square_col_id]);
			const unsigned long number = distribution(generator);
			sumofbio += number;
			numseeds[spread_row][spread_col] = number;
		}
	}

	numseeds[norm_currow][norm_curcol] += totalnumseeds - sumofbio;

	// mut_ex.Unlock();

	// clock_t finish = clock();
	// printf("%.2lf ms.\n", (finish - start_s) * 1000.0 / CLOCKS_PER_SEC);
	// fflush(stdout);
}


