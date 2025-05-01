#include "seeding_prob.h"
#include <cmath>
#include <iomanip>
#include "DEIntegrator.h"
#include <chrono>
#include <ctime>   
#include <random> 

#define PI 3.14159265
#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))
#define EPS 1e-7
float alpha;
int checktime = 100;
float threshold = 0.65;
float total = 0.95;
int repeatnum = 100;
float caltime = 0.9;
bool *isLimiter;
int **randmatrix;

 
 
bool seeding_prob::isEqual(double a, double b) {
	if (fabs(a - b) < EPS) {
		return true;
	}
	return false;
}

double seeding_prob::function(double x) {
	return x * x;
}

double seeding_prob::qql(float x, float y)
{
	float result = (1 / (24 * PI*alpha*alpha))*exp(-sqrt(sqrt(x *x + y * y) / alpha));
	return result;
}

double seeding_prob::fx(double alpha, double x)
{
	return (1 / (2 * alpha)*exp(-sqrt(x / alpha)));
}


double seeding_prob::caculate_integration(double a, double b, int precision) {
	double len = b - a;
	double interval = 1.0 / precision;
	unsigned long N = (unsigned long)len / interval;
	double result = 0;
	for (unsigned long i = 0; i < N; ++i) {
		result += fx(alpha, a + i * interval)*interval;
	}
	return result;
}


unsigned long seeding_prob::factorial(int n) {
	if (n == 0 || n == 1) return 1; // 0! 和 1! 都为1
	unsigned long long result = 1;
	for (int i = 2; i <= n; ++i) {
		result *= i;
	}
	return result;
}

// 函数来计算 S_k = n(n + 1)(n + 2)...(n + (k - 1)) / k!
double seeding_prob::S(int n, int k) {
	if (k <= 0) return 0; // k 必须为正数
	double numerator = 1.0; // 分子的初始化
	for (int i = 0; i < k; ++i) {
		numerator *= (n + i); // 计算 n(n + 1)(n + 2)...(n + (k - 1))
	}
	unsigned long long denominator = factorial(k); // 计算 k!
	return numerator / denominator; // 返回 S_k = 分子 / 分母
}


seeding_prob::seeding_prob()
{
	prob4square = NULL;
	cdf4species = NULL;
	minprob4spe = NULL;
	hilmt4spec = NULL;
	numcell_all_list = NULL;
	numLimiters = NULL;
}


void seeding_prob::cal_seeding_prob(const int cell_size_in, const int specNum_in, const int* speces_maxD, const int* specesefctD, const int snr_in, const int snc_in,const int* specesmaturity, const int timestep)
{
	assert(cell_size_in > 0);
	cell_size = cell_size_in;

	snr = snr_in;
	snc = snc_in;
	specNum = specNum_in;

	prob4square = new double**[specNum]();
	cdf4species = new double*[specNum]();
	minprob4spe = new double[specNum]();
	numcell_all_list = new int[specNum]();
	hilmt4spec = new int[specNum]();

	int num_cell = snr * snc;
	numLimiters = new int**[specNum];
	 
	isLimiter = new bool[specNum]();
	randmatrix = new int*[specNum]();
	timecount = new int[specNum]();

	for (int i = 0; i < specNum; i++)
	{
		 
	     const int numcellside = init4each(speces_maxD[i], specesefctD[i], i);

		numLimiters[i] = new int*[snr_in];
		numLimiters[i][0] = new int[num_cell]();
		 
		randmatrix[i] = new int[100];

		int count = 0;

		if (cell_size_in > speces_maxD[i])
		{
			//count = floor(cell_size_in / speces_maxD[i]) * specesmaturity[i]/timestep+1;
			count = ceil(cell_size_in / speces_maxD[i]) * specesmaturity[i] / timestep ;
		}

		timecount[i] = count;
		printf("\n Calculate Limiter cell_size =%d maxD=%d, count=%d \n", cell_size_in, speces_maxD[i], timecount[i]);

		for (int j = 0; j < snr; j++)
		{
			numLimiters[i][j] = new int[snc]();

			for (int k = 0; k < snc; k++)
			{
				numLimiters[i][j][k] = -2;
				//if (count == 0)
				//{
				//	numLimiters[i][j][k] = count;
				//}
				//else
				//{
				//	int min = 0, max = count;
				//	random_device seed;//硬件生成随机数种子
				//	ranlux48 engine(seed());//利用种子生成随机数引擎
				//	uniform_int_distribution<> distrib(min, max);//设置随机数范围，并为均匀分布
				//	int random = distrib(engine);//随机数

				//	numLimiters[i][j][k] = 0;
				//	if (j == snr - 1 && k == 0)
				//	{
				//		numLimiters[i][j][k] = ceil(count/2);
				//		printf("\n Calculate timecount i =%d j=%d k=%d,  timecount=%d ", i, j, k, numLimiters[i][j][k]);
				//	}
				//}
			}
		}

		isLimiter[i] = false;

		if (cell_size < speces_maxD[i] && cell_size * 2 > speces_maxD[i])
		{
			isLimiter[i] = true;

			int kk = (speces_maxD[i] - cell_size) * 100 / cell_size;

			for (int jj = 0; jj < kk; jj++)
			{
				randmatrix[i][jj] = 1;
			}

			for (int jj = kk; jj < 100; jj++)
			{
				randmatrix[i][jj] = 0;
			}

		}
 
	}


}




double seeding_prob::get_seed_prob(const int spec_id, const int relative_row, const int relative_col)
{
	const int hilmt = hilmt4spec[spec_id];
	const int lwlmt = -hilmt;

	const double ret_val = prob4square[spec_id][relative_row - lwlmt][hilmt - relative_col];

	return ret_val;
}


int seeding_prob::init4each(const float max_dist, const float specesefctD, const int spec_id)
{
	assert(max_dist > 0);

	//static const double alpha_x[9] = { 34,30,26,22,18,14,10,6,2 };

	//for (int alpha_index = 0; alpha_index < 9; alpha_index++)
	//{
	//	alpha = alpha_x[alpha_index];
	//	double edfx = caculate_integration(0, specesefctD, 1000);
	//	double mdfx = caculate_integration(0, max_dist, 1000);

	//	if ((edfx / mdfx) > threshold)
	//		break;
	//}


	alpha = 16.92438 - 0.04860*specesefctD + 0.01644*max_dist;

	if (alpha < 5) alpha = 5;
	if (alpha > 40) alpha = 40;
	checktime = (int)(cell_size*caltime);
	if (checktime > 500)
		checktime = 500;
	repeatnum = checktime;

	int tmp_offset = ceil((double)max_dist / cell_size);

	if (tmp_offset == 0)
	{
		tmp_offset = 1;
	}
	const int offset = tmp_offset >= 1 ? tmp_offset : 1;
	const int numcellside = 2 * offset + 1;
	const int numcell_all = numcellside * numcellside;

	numcell_all_list[spec_id] = numcell_all;

	hilmt4spec[spec_id] = offset;
	int lnum = 1000;


	int h = tmp_offset + 1;
	float c = 0.5;
	int numsize = tmp_offset / c + 1;
	float* spread_col = new float[numsize];
	float* spread_row = new float[numsize];
	spread_col[0] = -tmp_offset - c;
	spread_row[0] = tmp_offset + c;
	for (int i = 1; i < numsize; i++)
	{
		spread_col[i] = spread_col[i - 1] + 1;
		spread_row[i] = spread_row[i - 1] - 1;
	}


	//sampling============================================================================
	unsigned seed;
	std::default_random_engine random_engine(std::random_device{}());
	float min = -0.5* cell_size * 1000;
	float max = 0.5* cell_size * 1000;

	float* parent_col = new float[checktime];
	float* parent_row = new float[checktime];

	std::uniform_int_distribution<int> dist(min, max);
	for (int i = 0; i < checktime; i++)
	{
		parent_col[i] = (float)dist(random_engine) / 1000;

	}

	for (int i = 0; i < checktime; i++)
	{
		parent_row[i] = (float)dist(random_engine) / 1000;

	}

	double*** prob4squareTmp = new double**[checktime]();

	for (int k = 0; k < checktime; k++)
	{
		prob4squareTmp[k] = new double*[numcellside];
		for (int i = 0; i < numcellside; i++)
		{
			prob4squareTmp[k][i] = new double[numcellside]();
			for (int j = 0; j < numcellside; j++)
				prob4squareTmp[k][i][j] = 0;
		}
	}


	prob4square[spec_id] = new double*[numcellside];
	for (int i = 0; i < numcellside; i++)
	{
		prob4square[spec_id][i] = new double[numcellside]();
		for (int j = 0; j < numcellside; j++)
			prob4square[spec_id][i][j] = 0;
	}

	float* column = new float[repeatnum + 1];
	float* row = new float[repeatnum + 1];


	for (int m = 0; m < checktime; m++)
	{
		for (int rowindex = 0; rowindex < numsize; rowindex++)
		{
			for (int colindex = 0; colindex < numsize; colindex++)
			{

				column[0] = (spread_row[rowindex] - 1)*cell_size;
				float stepindex = cell_size * 1.0 / repeatnum;

				row[0] = spread_col[colindex] * cell_size;

				for (int i = 1; i <= repeatnum; i++)
					for (int j = 1; j <= repeatnum; j++)
					{
						column[i] = column[i - 1] + stepindex;
						row[i] = row[i - 1] + stepindex;
					}

				float central = 0;

				for (int k = 0; k <= repeatnum; k++)
					for (int l = 0; l <= repeatnum; l++)
					{
						central = central + qql(abs(row[l] - parent_row[m]), abs(column[k] - parent_col[m]));
					}

				int x = (int)(-spread_row[rowindex] + tmp_offset + 1.5 - 1);
				int y = (int)(spread_col[colindex] + tmp_offset + 1.5 - 1);
				prob4squareTmp[m][x][y] = central / (repeatnum*repeatnum)*cell_size*cell_size;
			}

		}
	}

	for (int m = 0; m < checktime; m++)
	{
		for (int i = 0; i < numcellside; i++)
		{
			for (int j = 0; j < numcellside; j++)
			{

				prob4square[spec_id][i][j] += prob4squareTmp[m][i][j];

			}

		}
	}


	float sum = 0;

	for (int i = 0; i < numcellside; i++)
	{
		for (int j = 0; j < numcellside; j++)
		{
			prob4square[spec_id][i][j] = prob4square[spec_id][i][j] / checktime;

			sum += prob4square[spec_id][i][j];
		}
	}


	//updated by Xianghua 9/9/2022
	//for (int i = 0; i < numcellside; i++)
	//{
	//	for (int j = 0; j < numcellside; j++)
	//	{
	//		prob4square[spec_id][i][j] = prob4square[spec_id][i][j] * total / sum;
 //		 
	//	}
	//}

	prob4square[spec_id][offset][offset] = total - sum + prob4square[spec_id][offset][offset];

	sum = 0;

	string filename = "prob_" + to_string(spec_id+1) +"_"+ to_string((int)(caltime*100)) + ".txt";
	FILE* fp = fopen(filename.c_str(), "w");

	fprintf(fp, " tmp_offset=%d numcellside=%d maxD= %8.3lf  effD= %8.3lf   alpha=%8.3lf cellsize= %8.3lf, checktime =%d, repeatnum=%d caltime=%8.3lf", tmp_offset, numcellside, max_dist, specesefctD,alpha, cell_size, checktime, repeatnum, caltime);
	fprintf(fp, "\n");
	for (int i = 0; i < numcellside; i++)
	{
		for (int j = 0; j < numcellside; j++)
		{
			prob4square[spec_id][i][j] = prob4square[spec_id][i][j];
			fprintf(fp, " %8.3lf", prob4square[spec_id][i][j] * 1000);
			sum += prob4square[spec_id][i][j];
		}
		fprintf(fp, "\n");
	}

	fprintf(fp, " sum= %8.3lf", sum);
	fprintf(fp, "\n");

	fclose(fp);
	const int hilmt = hilmt4spec[spec_id];
	const double minprob = prob4square[spec_id][0][0];
	const double maxprob = prob4square[spec_id][hilmt][hilmt];

	minprob4spe[spec_id] = minprob;
	cdf4species[spec_id] = new double[numcell_all];

	return numcellside;
}



seeding_prob::~seeding_prob()
{
	if (prob4square != NULL)
	{
		for (int i = 0; i < specNum; i++)
		{
			delete[] prob4square[i][0];
			delete[] prob4square[i];
		}

		delete[] prob4square;
		prob4square = NULL;
	}


	if (cdf4species != NULL)
	{
		for (int i = 0; i < specNum; i++)
			delete[] cdf4species[i];

		delete[] cdf4species;
		cdf4species = NULL;
	}


	if (hilmt4spec != NULL)
		delete[] hilmt4spec;
	hilmt4spec = NULL;

	if (numcell_all_list != NULL)
		delete[] numcell_all_list;
	numcell_all_list = NULL;

	if (minprob4spe != NULL)
		delete[] minprob4spe;
	minprob4spe = NULL;
}

//unsigned long long factorial(int n) {
//	if (n == 0 || n == 1) return 1; // 0! 和 1! 都为1
//	unsigned long long result = 1;
//	for (int i = 2; i <= n; ++i) {
//		result *= i;
//	}
//	return result;
//}
//
// 函数来计算 S_k = n(n + 1)(n + 2)...(n + (k - 1)) / k!
//double S(int n, int k) {
//	if (k <= 0) return 0; // k 必须为正数
//	double numerator = 1.0; // 分子的初始化
//	for (int i = 0; i < k; ++i) {
//		numerator *= (n + i); // 计算 n(n + 1)(n + 2)...(n + (k - 1))
//	}
//	unsigned long long denominator = factorial(k); // 计算 k!
//	return numerator / denominator; // 返回 S_k = 分子 / 分母
//}



void seeding_prob::process_seeds_on_site(int spec_id, int currow, int curcol, unsigned long** numseeds, const unsigned long totalnumseeds)
{

	// clock_t start_s=clock();
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();

	const int offset = hilmt4spec[spec_id];

	const int norm_currow = currow - 1;
	const int norm_curcol = curcol - 1;

 
	//	printf(" Process_seeds_on_site: spec_id =%d currow=%d curcol=%d,   \n ", spec_id, norm_currow, norm_curcol);

	//reduce 1 for each timestep
	if (numLimiters[spec_id][norm_currow][norm_curcol] <= -1)
	{
		numseeds[norm_currow][norm_curcol] += totalnumseeds;
		
		return;
	}

	if (numLimiters[spec_id][norm_currow][norm_curcol] >= 1)
	{
		numseeds[norm_currow][norm_curcol] += totalnumseeds;
		numLimiters[spec_id][norm_currow][norm_curcol] = numLimiters[spec_id][norm_currow][norm_curcol] - 1;
		return;
	}
	
 
	if (isLimiter[specNum])
	{
		int sel = rand() % 100;
		//if (norm_currow == 0 && norm_curcol == 0)
		//{
		//	printf("\n is Random select limiter =%d value=%d\n", sel, randmatrix[specNum][sel]);
		//}
		
		if (randmatrix[specNum][sel] == 0)
		{
			return;
		}
	}
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
			numseeds[spread_row][spread_col] += number;
 		}
	}

	numseeds[norm_currow][norm_curcol] += totalnumseeds - sumofbio;

	// mut_ex.Unlock();

	// clock_t finish = clock();
	// printf("%.2lf ms.\n", (finish - start_s) * 1000.0 / CLOCKS_PER_SEC);
	// fflush(stdout);
}


