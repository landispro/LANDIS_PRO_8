#define _CRT_SECURE_NO_WARNINGS

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#include "system1.h"
#include "error.h"
#include "map8.h"
#include "gdal_priv.h"
#include "cpl_conv.h" // for CPLMalloc()
#include "cpl_string.h"
#include "ogr_spatialref.h"

//#include <vld.h>

MAP8::MAP8()
{
	strcpy(title, "NO TITLE");

	for (int i = 0; i < MapmaxValue; i++)
		sprintf(legend[i], "%d", i);

	largeCell = 0;

	data = NULL;

	data_bio = NULL;

	for (int i = 0; i < 32; i++)
		header[i] = 0;
}




//Constructor, No dimensions but sets the coordinates.
MAP8::MAP8(unsigned long* dest)
{
	strcpy(title, "NO TITLE");

	for (int i = 0; i < MapmaxValue; i++)
		sprintf(legend[i], "%d", i);

	largeCell = 0;

	data = NULL;

	data_bio = NULL;

	for (int i = 0; i < 32; i++)
		header[i] = dest[i];
}




//Constructor, Dimensions given.
MAP8::MAP8(int r, int c)
{
	strcpy(title, "NO TITLE");

	for (int i = 0; i < MapmaxValue; i++)
		sprintf(legend[i], "<%d>", i);

	largeCell = 0;

	data = NULL;

	data_bio = NULL;

	for (int i = 0; i < 32; i++)
		header[i] = 0;

	dim(r, c);
}



MAP8::~MAP8()
{
	if (data)     {delete[] data; 	  data = NULL;}
	if (data_bio) {delete[] data_bio; data_bio = NULL;}
}



//Read legend from a file.  First line of the file is a title.  All remaining
//lines are legends for up to fifteen classes.
int MAP8::readLegend(char* fn)
{
	FILE *fp;
	if ((fp = fopen(fn, "r")) == NULL)
		return 1;

	fgets(title, 60, fp);

	*(title + strlen(title) - 1) = '\0';



	int i = 0;

	while (!feof(fp))
	{
		fgets(legend[i], 60, fp);

		*(legend[i] + strlen(legend[i]) - 1) = '\0';

		i++;
	}

	fclose(fp);

	return 0;
}




//Read map from a file.   Input file is an ERDAS GIS File with no extension.
int MAP8::read(const char *fn)
{
	char str[100];
	sprintf(str, "%s", fn);

	FILE *fp;
	if ((fp = fopen(str, "rb")) == NULL)   { printf("open %s error\n", str); return 1; }


	float temp;
	fread((char*)header, 4, 32, fp);	
	fread((char*)&temp,  4, 1, fp);
	
	cellSize = temp;
	
	fread((char*)&temp, 4, 1, fp);

	cellSize *= temp;


	const int xDim = header[4];
	const int yDim = header[5];


	dim(yDim, xDim);


	for (int i = yDim; i > 0; i--)
	{
		for (int j = 1; j <= xDim; j++)
		{
			unsigned char c;

			const size_t numread = fread((char*)(&c), 1, 1, fp);

			unsigned short intm = (unsigned short)c;
			
			(*this)(i, j) = intm;

			if (numread > 0 && intm > largeCell)			
				largeCell = intm;
		}
	}

	fclose(fp);



	/*TRAILER FILE*/

	sprintf(str, "%s.trl", fn);

	if ((fp = fopen(str, "rb")) == NULL)
		errorSys("TRAILER FILE NOT FOUND.\n", CONTINUE);


	long dest[32];
	fread((char*)dest, 1, 128, fp);        /*Header*/
	
	fread((char*)green, 1, 128, fp);       /*Green*/
	fread((char*)&green[128], 1, 128, fp); /*Green*/
	
	fread((char*)red, 1, 128, fp);         /*Red*/
	fread((char*)&red[128], 1, 128, fp);   /*Red*/
	
	fread((char*)blue, 1, 128, fp);        /*Blue*/
	fread((char*)&blue, 1, 128, fp);       /*Blue*/
	
	fread((char*)dest, 1, 128, fp);        /*Empty Record*/

	for (int i = 9; i <= 16; i++)                  /*Histogram*/
		fread((char*)dest, 1, 128, fp);


	for (int i = 0; i <= largeCell; i++)
	{
		fread(legend[i], 1, 32, fp);

		int j = 0;

		for (; j <= 31; j++)
		{
			if (legend[i][j] == '~')
				break;
		}

		legend[i][j] = '\0';
	}

	fclose(fp);

	return 0;
}





//This will write the map to a file.  It will use the default color scheme.
int MAP8::write(const char *fn)
{
	double wAdfGeoTransform[6] = { 0.00, cellSize, 0.00, 600.00, 0.00, -cellSize };

	const int ret_val = write(fn, red, green, blue, wAdfGeoTransform);
	
	return ret_val;
}



//Write map to a file.  There is no extension on the file name.  Three
//arrays of colors cooresponding to the palette (RGB) are also included.
int MAP8::write(const char *fn, int *red, int *green, int *blue, double wAdfGeoTransform[])
{
	unsigned char ured[maxLeg], ugreen[maxLeg], ublue[maxLeg];

	/// 27JUL94_CSH
	for (int i = 0; i < maxLeg; i++)
	{
		ured[i]   = (unsigned char)red[i];
		ugreen[i] = (unsigned char)green[i];
		ublue[i]  = (unsigned char)blue[i];
	}

	const int ret_val = write(fn, &ured[0], &ugreen[0], &ublue[0], wAdfGeoTransform);

	return ret_val;
}






//Write map to a file.  There is no extension on the file name.  Three
//arrays of colors cooresponding to the palette (RGB) are also included.
int MAP8::write(const char *fn, unsigned char *red, unsigned char *green, unsigned char *blue, double wAdfGeoTransform[])
{
	static const int lcsize = numRows * numCols;
	static const char *pszFormat = "HFA";

	GDALDriver *poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);

	if (poDriver == NULL) { printf("podriver\n"); exit(1); }

	char ** papszMetadata = poDriver->GetMetadata();


	OGRSpatialReference oSRS;

	oSRS.SetUTM(11, TRUE);

	oSRS.SetWellKnownGeogCS("HEAD74");//*NAD27

	char *pszSRS_WKT = NULL;
	oSRS.exportToWkt(&pszSRS_WKT);

	CPLFree(pszSRS_WKT);

	char str[100];
	sprintf(str, "%s.img", fn);

	unsigned int* pintScanline = (unsigned int *)CPLMalloc(sizeof(unsigned int) * lcsize);

	float* pafScanline = (float *)CPLMalloc(sizeof(float)*(numCols*numRows));

	char** papszOptions = NULL;
	GDALDataset *poDstDS = NULL;

	if (strstr(str, "BA") || strstr(str, "BIO"))
		poDstDS = poDriver->Create(str, numCols, numRows, 1,GDT_CFloat32, papszOptions);
	else
		poDstDS = poDriver->Create(str, numCols, numRows, 1, GDT_UInt32, papszOptions);

	if (poDstDS == NULL) { printf("could not create file for img file output"); return 1; }

	poDstDS->SetGeoTransform(wAdfGeoTransform);

	GDALRasterBand* outPoBand = poDstDS->GetRasterBand(1);


	for (int i = numRows; i > 0; i--)
	{
		for (int j = 1; j <= numCols; j++)
		{
			if (strstr(str, "BA") || strstr(str, "BIO"))
			{
				const int xx = ((i - 1) * numCols) + j - 1;
				*(pintScanline + (numRows - i) * numCols + j - 1) = (*this).data_bio[xx];
			}
			else
				*(pintScanline + (numRows - i) * numCols + j - 1) = (*this)(i, j);
		}
	}


	if (strstr(str, "BA") || strstr(str, "BIO"))
		outPoBand->RasterIO(GF_Write, 0, 0, numCols, numRows, pafScanline, numCols, numRows, GDT_Float32, 0, 0);	
	else
		outPoBand->RasterIO(GF_Write, 0, 0, numCols, numRows, pintScanline, numCols, numRows, GDT_UInt32, 0, 0);


	if (poDstDS != NULL)
		GDALClose((GDALDatasetH)poDstDS);

	CPLFree(pintScanline);
	CSLDestroy(papszOptions);

	return 0;
}





//This will copy the contents of one map into another.  
//This includes legends, titles, and map dimensions.
void MAP8::copy(const MAP8 &n)
{
	largeCell = n.largeCell;

	dim(n.numRows, n.numCols);

	strcpy(title, n.title);

	for (int i = 0; i < 22; i++)
		strcpy(legend[i], n.legend[i]);

	static const int lcsize = numRows * numCols;

	for (int i = 0; i < lcsize; i++)
	{
		data[i] = n.data[i];
		data_bio[i] = n.data_bio[i];
	}
}





MAP8& MAP8::operator=(const MAP8 &n)
{
	largeCell = n.largeCell;
	cellSize = n.cellSize;

	dim(n.numRows, n.numCols);

	strcpy(title, n.title);

	for (int i = 0; i < maxLeg; i++)
		strcpy(legend[i], n.legend[i]);

	static const int lcsize = numRows * numCols;

	for (int i = 0; i < lcsize; i++)
	{
		data[i] = n.data[i];
		data_bio[i] = n.data_bio[i];
	}

	// for(int i=0; i<32; i++)
	// 	header[i] = n.header[i];

	// for(int i=0; i<maxLeg; i++)
	// {
	// 	red[i] = n.red[i];
	// 	green[i] = n.green[i];
	// 	blue[i] = n.blue[i];
	// }

	return *this;
}






//This will dimension or redimension the map size.
void MAP8::dim(int r, int c)
{
	if (data != NULL)
		delete[] data;

	if (data_bio != NULL)
		delete[] data_bio;

	numCols = c;
	numRows = r;

	data = new unsigned short[numCols * numRows]();
	data_bio = new float[numCols * numRows]();

	if (data == NULL || data_bio == NULL)
		errorSys("Memory is not enough", STOP);
}




int MAP8::cols()  { return numCols; }
int MAP8::rows() { return numRows; }




//This will return a single map element.
unsigned short& MAP8::operator()(int r, int c)
{
	if (r <= 0 || r > numRows || c <= 0 || c > numCols)
	{
		char err[80];
		sprintf(err, "MAP8::operator() (int,int)-> (%d, %d) are illegal map coordinates", r, c);
		errorSys(err, STOP);
	}

	const int x = ((r - 1) * numCols) + c - 1;

	return data[x];
}




//This will rename a map.
void MAP8::rename(char *c)
{
	strncpy(title, c, 45);
}





//This will assign a legend element to a map.
void MAP8::assignLeg(int pos, char *c)
{
	strncpy(legend[pos], c, 50);

	if ((int)largeCell < pos)
		largeCell = (unsigned char)(pos + 1);
}



//This will fill a map with a single value.
void MAP8::fill(const unsigned short c)
{
	static const int lcsize = numRows * numCols;

	for (int i = 0; i < lcsize; i++)
		data[i] = c;
}




//This will set the size of a single cell.
void MAP8::setCellSize(int size)  { cellSize = size; }



//This will return the highest value present in the map.
int MAP8::high() { return largeCell; }



//This will return the cell size of a map.
float MAP8::sizeOfCells() { return cellSize; }



//This will return the contents of a legend item.
char* MAP8::legendItem(int pos)
{	
	return strdup(legend[pos]); //Note: strdup uses malloc.
}



//sets the map coordinates.
void MAP8::setHeader(unsigned long* dest)
{
	for (int i = 0; i < 32; i++) 
		header[i] = dest[i];
}



//=========================================================================

MAP16::MAP16()
{
	largeCell = 0;

	data = NULL;

	for (int i = 0; i < 32; i++)
		header[i] = 0;
}




MAP16::MAP16(int r, int c)
{
	largeCell = 0;

	data = NULL;

	for (int i = 0; i < 32; i++)
		header[i] = 0;

	dim(r, c);
}



MAP16::~MAP16()
{
	if (data)
		delete[] data;

	if (data32)
		delete[] data32;
}




//Read map from a file.   Input file is an 16-bit ERDAS GIS File with no extension.
int MAP16::read(const char *fn)
{
	flag16or32 = 16;

	char str[100];
	sprintf(str, "%s", fn); //Nim: changed %s.gis to %s

	FILE *fp;
	if ((fp = fopen(str, "rb")) == NULL)  { printf("open %s error\n", str); return 1; }


	fread((char*)header, 4, 30, fp);


	char	b16or8;   //true: 16, false 8 bit

	if ((header[1] & 0xff0000) == 0x020000)
		b16or8 = 16;
	else if ((header[1] & 0xff0000) == 0)
		b16or8 = 8;
	else
		errorSys("Error: IO: Landtype map is niether 16 bit or 8 bit.", STOP);


	float temp;
	fread((char*)&temp, 4, 1, fp);

	cellSize = temp;

	fread((char*)&temp, 4, 1, fp);

	cellSize *= temp;


	const int xDim = header[4];
	const int yDim = header[5];


	dim(yDim, xDim);


	for (int i = yDim; i > 0; i--)
	{
		for (int j = 1; j <= xDim; j++)
		{
			size_t numread = 0;
			unsigned short mapValue;

			if (b16or8 == 8)
			{
				unsigned char c8bit;

				numread = fread((char*)(&c8bit), 1, 1, fp);

				mapValue = c8bit;
			}
			else			
			{
				assert(b16or8 == 16);

				unsigned short c16bit;

				numread = fread((char*)(&c16bit), 2, 1, fp);

				mapValue = c16bit;
			}

			
			(*this).set_data(i, j, mapValue);

			if (numread > 0 && mapValue > largeCell)
				largeCell = mapValue;
		}
	}

	fclose(fp);

	return 0;
}





//Read map from a file.   Input file is an 16-bit ERDAS GIS File with no extension.
int MAP16::read(const char *fn, const int giRow, const int giCol)
{
	flag16or32 = 16;
	
	char str[100];
	sprintf(str, "%s", fn);

	FILE *fp;
	if ((fp = fopen(str, "rb")) == NULL)  { printf("open %s error\n", str); return 1; }

	fread((char*)header, 4, 30, fp);

	char	b16or8;   //true: 16, false 8 bit

	if ((header[1] & 0xff0000) == 0x020000)
		b16or8 = 16;
	else if ((header[1] & 0xff0000) == 0)
		b16or8 = 8;
	else
		errorSys("Error: IO: Landtype map is niether 16 bit or 8 bit.", STOP);


	float temp;
	fread((char*)&temp, 4, 1, fp);

	cellSize = temp;

	fread((char*)&temp, 4, 1, fp);

	cellSize *= temp;


	const int xDim = header[4]; //Nim: changed yDim to xDim
	const int yDim = header[5]; //Nim: changed xDim to yDim

	assert(giRow == yDim);	
	assert(giCol == xDim);

	dim(yDim, xDim);


	for (int i = yDim; i > 0; i--)
	{
		for (int j = 1; j <= xDim; j++)
		{
			size_t numread = 0;
			unsigned short  mapValue;

			if (b16or8 == 8)
			{
				unsigned char c8bit;

				numread = fread((char*)(&c8bit), 1, 1, fp);

				mapValue = c8bit;
			}
			else				
			{
				assert(b16or8 == 16);

				unsigned short c16bit;

				numread = fread((char*)(&c16bit), 2, 1, fp);
				
				mapValue = c16bit;
			}

			(*this).set_data(i, j, mapValue);

			if (numread > 0 && mapValue > largeCell)
				largeCell = mapValue;
		}

	}

	fclose(fp);

	return 0;
}






//Read map from a file.   Input file is an 16-bit ERDAS GIS File with no extension.
void MAP16::readImg(const char *fn, const int giRow, const int giCol)
{
	GDALAllRegister(); 

	GDALDataset  *simgFile;

	if ((simgFile = (GDALDataset *)GDALOpen(fn, GA_ReadOnly)) == NULL) 
		errorSys(" img map input file not found.", STOP);

	const int xDim = simgFile->GetRasterXSize();
	const int yDim = simgFile->GetRasterYSize();

	assert(giRow == yDim);
	assert(giCol == xDim);

	dim(yDim, xDim);

	float* pafScanline = (float *)CPLMalloc(sizeof(float)* (xDim * yDim));

	GDALRasterBand* poBand = simgFile->GetRasterBand(1);

	poBand->RasterIO(GF_Read, 0, 0, xDim, yDim, pafScanline, xDim, yDim, GDT_Float32, 0, 0);


	for (int i = yDim; i > 0; i--)
	{
		const int intm = (yDim - i) * xDim;

		for (int j = 1; j <= xDim; j++)
		{
			const int mapValue = (int)*(pafScanline + intm + j - 1);

			(*this).set_data(i, j, mapValue);

			if (mapValue > largeCell)
				largeCell = mapValue;
		}
	}
}




//This will dimension or redimension the map size.
void MAP16::dim(int r, int c)
{
	if (data != NULL)
		delete[] data;

	numCols = c;
	numRows = r;

	flag16or32 = 16;

	data = new unsigned int[numCols * numRows];
}





int MAP16::cols() { return numCols; }
int MAP16::rows() { return numRows; }



unsigned int MAP16::get_data(int r, int c)
{
	assert(r >= 1 && r <= numRows && c >= 1 && c <= numCols);

	assert(flag16or32 == 16);

	const int x = ((r - 1) * numCols) + c - 1;

	return data[x];
}




void MAP16::set_data(int r, int c, unsigned int val)
{
	assert(flag16or32 == 16);
	assert(r >= 1 && r <= numRows && c >= 1 && c <= numCols);

	const int x = ((r - 1) * numCols) + c - 1;

	data[x] = val;
}



void MAP16::putvalue32in(int r, int c, int value)
{
	const int x = ((r - 1) * numCols) + c - 1;

	data32[x] = value;
}




int MAP16::getvalue32out(int r, int c)
{
	assert(c >= 1 && c <= numCols && r >= 1 && r <= numRows);

	const int x = ((r - 1) * numCols) + c - 1;

	if (flag16or32 == 16)
		return get_data(r, c); //return (*this).get_data(r, c);
	else
	{
		assert(flag16or32 == 32);
		return data32[x];
	}
}





//This will fill a map with a single value.
void MAP16::fill(const unsigned short c)
{
	const int size = numCols * numRows;
	
	for (int i = 0; i < size; i++)
		data[i] = c;
}





//aim to compress management area data
void MAP16::freeMAPdata()
{
	if (flag16or32 == 16)
	{
		if (data)
			delete[] data;

		data = NULL;
	}
	else if (flag16or32 == 32)
	{
		if (data32)
			delete[] data32;

		data32 = NULL;
	}
}





//This will return the highest value present in the map.
int MAP16::high()  { return (int)largeCell; }



int MAP16::inMap(int r, int c)
{
	int result = 0;

	if (r >= 1 && r <= numRows && c >= 1 && c <= numCols)
		result = 1;

	return result;
}



void MAP16::dump() 
{
	const int numrow = rows();
	const int numcol = cols();

	for (int i = numrow; i > 0; i--) 
	{
		for (int j = 1; j <= numcol; j++) 
		{
			int cellValue = (*this).get_data(i, j);

			printf(" %2d", cellValue);
		}

		printf("\n");
	}
}






void MAP16::readtxt(char *fn)
{
	flag16or32 = 32;

	char str[100];
	sprintf(str, "%s", fn); //Nim: changed %s.gis to %s

	FILE *fp;
	if ((fp = fopen(str, "r")) == NULL) { printf("open %s error\n", str); return; }

	int xDim, yDim;

	fscanf(fp, "%s", str);
	fscanf(fp, "%d", &xDim);
	fscanf(fp, "%s", str);
	fscanf(fp, "%d", &yDim);
	fscanf(fp, "%s", str);
	fscanf(fp, "%s", str);
	fscanf(fp, "%s", str);
	fscanf(fp, "%s", str);
	fscanf(fp, "%s", str);
	fscanf(fp, "%f", &cellSize);
	fscanf(fp, "%s", str);
	fscanf(fp, "%s", str);


	dim32(yDim, xDim);


	for (int i = yDim; i > 0; i--)
	{
		int mapValue;

		for (int j = 1; j <= xDim; j++)
		{
			const int numread = fscanf(fp, "%d", &mapValue);

			putvalue32in(i, j, mapValue);

			if (numread > 0 && mapValue > largeCell)
				largeCell = mapValue;
		}
	}

	fclose(fp);
}





void MAP16::readtxt(char *fn, const int giRow, const int giCol)
{
	flag16or32 = 32;
	
	char str[100];
	sprintf(str, "%s", fn);

	FILE *fp;
	if ((fp = fopen(str, "r")) == NULL) { printf("open %s error\n", str); return; }		


	int xDim, yDim;

	fscanf(fp, "%s", str);
	fscanf(fp, "%d", &xDim);
	fscanf(fp, "%s", str);
	fscanf(fp, "%d", &yDim);

	assert(giRow == yDim);
	assert(giCol == xDim);

	fscanf(fp, "%s", str);
	fscanf(fp, "%s", str);
	fscanf(fp, "%s", str);
	fscanf(fp, "%s", str);
	fscanf(fp, "%s", str);
	fscanf(fp, "%f", &cellSize);
	fscanf(fp, "%s", str);
	fscanf(fp, "%s", str);

	dim32(yDim, xDim);

	for (int i = yDim; i > 0; i--)
	{
		int mapValue;

		for (int j = 1; j <= xDim; j++)
		{
			const int numread = fscanf(fp, "%d", &mapValue);
			putvalue32in(i, j, mapValue);

			if (numread > 0 && mapValue > largeCell)
				largeCell = mapValue;
		}
	}

	fclose(fp);
}





void MAP16::dim32(int r, int c)
{
	if (data32 != NULL)
		delete[] data32;

	numCols = c;
	numRows = r;

	data32 = new unsigned int[numCols * numRows];
}
