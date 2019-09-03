//#include <ctype.h>

#include "defines.h"
#include "system1.h"
#include "error.h"
#include "agelist.h"
#include "GlobalVariables.h"

//#include <vld.h>

static unsigned int times = 10;


static unsigned int *countofagevector;




//The following masks are used by the AGELIST class to process bits. 
//lowMask is a mask with the low bit set.  
//highMask has the high bit set and fullMask has all bits sets.  
//Mask is an array with 32 elements, each having their respective bit set.

static const unsigned long lowMask = 0x00000001;
static const unsigned long highMask = 0x80000000;
static const unsigned long fullMask = 0XFFFFFFFF;

static const unsigned long mask[32] =
{
	0x00000001, 0x00000002, 0x00000004, 0x00000008,
	0x00000010, 0x00000020, 0x00000040, 0x00000080,
	0x00000100, 0x00000200, 0x00000400, 0x00000800,
	0x00001000, 0x00002000, 0x00004000, 0x00008000,
	0x00010000, 0x00020000, 0x00040000, 0x00080000,
	0x00100000, 0x00200000, 0x00400000, 0x00800000,
	0x01000000, 0x02000000, 0x04000000, 0x08000000,
	0x10000000, 0x20000000, 0x40000000, 0x80000000
};






void timestep::settimestep(unsigned int time_step_in)
{
	Time_step = time_step_in;

	times = time_step_in;
}


void timestep::getSpecNum(unsigned int spec_num)
{
	SpecNum = spec_num;

	countofagevector = new unsigned int[spec_num];
}



void timestep::Setlongevity(int i, unsigned int num)
{
	if (i > SpecNum)
		errorSys("Illegal spec index in set longevity", STOP);

	countofagevector[i - 1] = num / times;
}




timestep::~timestep() { delete countofagevector; }




//===========================================================================================
//===========================================================================================
//===========================================================================================
//the following is for AGELIST


unsigned int AGELIST::getTreeNum(int n, int specIndex)
{
	if (n == 0)
		return 0;

	if (n < 0 || n > countofagevector[specIndex - 1])
	{
		printf("%d,%d,%d\n", n, countofagevector[specIndex - 1], agevector[n]);
		errorSys("Index age error in agelist 70 Pro", STOP);
	}

	return agevector[n];
}



void AGELIST::setTreeNum(int n, int specIndex, unsigned int num)
{
	if (n < 1 || n > countofagevector[specIndex - 1])
		errorSys("set age error in agelist 70 Pro", STOP);

	agevector[n] = num;
}



void AGELIST::GrowTree()
{
	for (int i = agevector[0]; i > 1; i--)
		agevector[i] = agevector[i - 1];

	agevector[1] = 0;
}




int AGELIST::getAgeVector(int i)
{
	if (i < 0)
		errorSys("AGELIST::set()-> Illegal age.", STOP);

	return agevector[i];
}



int AGELIST::getAgeVectorNum() { return agevector[0]; }

int AGELIST::getTimeStep() { return times; }


AGELIST::AGELIST() { agevector = NULL; }



void AGELIST::AGELISTAllocateVector(int index)
{
	agevector = new unsigned int[countofagevector[index] + 1]();

	agevector[0] = countofagevector[index];
}





AGELIST::AGELIST(const AGELIST& agelist)
{
	agevector = new unsigned int[agelist.agevector[0] + 1];

	for (unsigned int i = 0; i <= agelist.agevector[0]; i++)
		agevector[i] = agelist.agevector[i];
}







AGELIST& AGELIST::operator=(const AGELIST& s)
{
	if (agevector != NULL)
		delete[] agevector;

	agevector = new unsigned int[s.agevector[0] + 1];

	for (unsigned int i = 0; i <= s.agevector[0]; i++)
		agevector[i] = s.agevector[i];

	return *this;
}





AGELIST::~AGELIST()
{
	if (agevector)
		delete[] agevector;
}




//Clear agelist.
void AGELIST::clear()
{
	if (agevector == NULL)
	{
		agevector = new unsigned int[320 + 1]();

		agevector[0] = 320;
	}
	else
	{
		for (unsigned int i = 1; i <= agevector[0]; i++)
			agevector[i] = 0;
	}
}





//Set an age to true.
void AGELIST::set(int age)
{
	if (age % times == 0)
		age = age / times;
	else
		age = age / times + 1;

	if (age < 1 || age > 320 / times)
		errorSys("AGELIST::set()-> Illegal age.", STOP);

	int temp1 = (age - 1) / 32;
	int temp2 = (age - 1) % 32;

	if (age == 64)
		printf("something happend\nn");

	agevector[temp1] |= mask[temp2];
}




//Reset an age to false.
void AGELIST::reset(int age)
{
	if (age % times == 0)
		age = age / times;
	else
		age = age / times + 1;

	if (age < 1 || age > 320 / times || age > agevector[0])
		errorSys("AGELIST::reset()-> Illegal age.", STOP);

	agevector[age] = 0;
}





//Returns true if any age clases are present, false otherwise.
int AGELIST::query()
{
	int result = 0;

	for (unsigned int i = 1; i <= agevector[0]; i++)
	{
		result = result || agevector[i];
	}

	return result;
}




//Return true if an age exists in the list, false otherwise.
int AGELIST::query(int age)
{
	if (age % times == 0)
		age = age / times;
	else
		age = age / times + 1;

	if (age < 1 || age > 320 / times || age > agevector[0])
		errorSys("AGELIST::query(int)-> Illegal age.", STOP);

	return agevector[age];
}





//Return true if an age exists in the range bounded by low and high, false otherwise.
int AGELIST::query(int low, int high)
{
	if (low % times == 0)
		low = low / times;
	else
		low = low / times + 1;

	if (low < 1 || low > 320 / times)
		errorSys("AGELIST::query(int,int)-> Illegal lower bound.", STOP);


	if (high % times == 0)
		high = high / times;
	else
		high = high / times + 1;


	if (high < 1 || high > 320 / times)
		errorSys("AGELIST::query(int,int)-> Illegal upper bound.", STOP);

	if (low > high)
		errorSys("AGELIST::query(int,int)-> Lower bound is greater than upper bound.", STOP);


	for (unsigned int i = low; i <= high; i++)
	{
		if (i < 1 || i > agevector[0])
			errorSys("Agelist Reset Problem", STOP);

		if (agevector[i])
			return true;
	}

	return false;
}





//Returns the youngest age present.
int AGELIST::youngest()
{
	for (unsigned int j = 1; j <= agevector[0]; j++)
	{
		if (agevector[j] > 0)
		{
			const int ret = j * times;
			return ret;
		}
	}

	return 0;
}



//Returns the oldest age present.
int AGELIST::oldest()
{
	for (int j = agevector[0]; j >= 1; j--)
	{
		if (agevector[j] > 0)
		{
			const int ret = j * times;
			return ret;
		}
	}

	return 0;
}




//Returns the number of age classes present.
int AGELIST::number()
{
	int  num = 0;

	for (unsigned int j = 1; j <= agevector[0]; j++)
	{
		if (agevector[j] > 0)
			num++;
	}

	return num;
}








void AGELIST::readBinary(FILE *infile)
{
	skipblanks(infile);

	const int j_end1 = 320 / times;

	char ch;

	for (int j = 0; j < j_end1 && ((ch = (char)fgetc(infile)) != EOF) && isdigit(ch); j++)
	{
		if (ch != '1' && ch != '0')
			errorSys("read map attribute age error\n", STOP);

		if (ch == '1')
		{
			const int temp1 = j / 32;
			const int temp2 = j % 32;

			agevector[temp1] = agevector[temp1] | mask[temp2];
		}
	}
}





//This shall read an age list from a file.
void AGELIST::read(FILE* infile)
{
	int numSet = 0, buffer1 = 0, buffer2 = 0, temp, barflag = 0;

	clear();

	skipblanks(infile);

	char ch;

	for (int j = 0; numSet <= 320 && ((ch = (char)fgetc(infile)) != EOF); j++)
	{
		if (isdigit(ch))
		{
			if (barflag == 0)
				buffer1 = buffer1 * 10 + ch - 48;
			else
				buffer2 = buffer2 * 10 + ch - 48;
		}
		else
		{
			if (ch == '-')
			{
				barflag = 1;
			}
			else if (ch == ' ')
			{
				if (barflag == 1)
				{
					barflag = 0;

					for (temp = buffer1; temp <= buffer2; temp = temp + times)
					{
						if (temp >= times)
						{
							agevector[temp / times] = 1;

							numSet++;
						}
					}

					buffer1 = 0;
					buffer2 = 0;
				}
				else
				{
					temp = buffer1;

					if (temp >= times)
					{
						agevector[temp / times] = 1;

						numSet++;

						buffer1 = 0;
						buffer2 = 0;
					}
				}
			}
			else
			{
				if (barflag == 1)
				{
					barflag = 0;

					for (temp = buffer1; temp <= buffer2; temp = temp + times)
					{
						if (temp >= times)
						{
							agevector[temp / times] = 1;

							numSet++;
						}
					}

					buffer1 = 0;
					buffer2 = 0;
				}
				else
				{
					temp = buffer1;

					if (temp >= times)
					{
						agevector[temp / times] = 1;

						numSet++;

						buffer1 = 0;
						buffer2 = 0;
					}
				}

				break;
			}
		}
	}
}









void AGELIST::readTreeNum(FILE* infile, int specIndex)
{
	clear();

	char ch;

	int buffer1 = 0, spaceflag = 0;

	unsigned int numSet = 0;

	skipblanks(infile);

	for (int j = 0; numSet < countofagevector[specIndex] && ((ch = (char)fgetc(infile)) != EOF); j++)
	{
		if (ch == '\n')
			break;

		if (isdigit(ch))
		{
			buffer1 = buffer1 * 10 + ch - 48;

			spaceflag = 0;
		}
		else
		{
			if (spaceflag == 0)
			{
				agevector[numSet + 1] = buffer1;

				numSet++;

				buffer1 = 0;

				spaceflag = 1;
			}
		}
	}


	if (numSet < 1 || numSet > countofagevector[specIndex])
		errorSys("specie composition error", STOP);
}





//This will write an age list to a file.
void AGELIST::write(FILE* outfile)
{
	printf("%s: \n", outfile);

	for (unsigned int j = 0; j < 320 / times; j++)
	{
		const int temp1 = j / 32;
		const int temp2 = j % 32;

		if (agevector[temp1] & mask[temp2])
			fprintf(outfile, "1");
		else
			fprintf(outfile, "0");
	}
}




//This will dump an age list to the CRT.
void AGELIST::dump()
{
	for (unsigned int j = 0; j < 320 / times; j++)
	{
		const int temp1 = j / 32;

		const int temp2 = j % 32;

		if (agevector[temp1] & mask[temp2])
			printf("%d:a", temp2);
		else
			printf("%d:b", temp2);
	}

	printf("\n");
}

