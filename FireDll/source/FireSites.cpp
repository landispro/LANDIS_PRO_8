/********************************************

Name:			FireSites.cpp 

Description:	FireSites

Input:			

Output:			

Date:			Feb. 18, 2004

Last Mod:		Feb. 18

*******************************************/



#include <stdlib.h>

#include <string.h>

#include "system1.h"

#include "error.h"

#include "FireSites.h"



//<Add By Qia on Nov 24 2008>

#include <vector>

#include <list>



CFireSites::CFireSites()
{
	m_iRows = 0;
	m_iCols = 0;
	map = NULL;
}



CFireSites::CFireSites(int i, int j)
{
	init(i, j);
}




void CFireSites::init(int i, int j)
{
	m_iRows = i;

	m_iCols = j;

	map = new FIRESITE [i * j];//Add By Qia on Nov 24 2008

	printf("LandisPro fire map allocated\n");

	FIRESITE temp;

	temp.FRUIndex = 0;
	temp.DEM = 0;
	temp.numofsites = i * j;

	SortedIndex.push_back(temp);

	for (int i = 1; i <= m_iRows; i++)
	{
		for (int j = 1; j <= m_iCols; j++)
		{
			const int x = (i - 1) * m_iCols + j - 1;
			map[x] = temp;
		}
	}
}




CFireSites::~CFireSites()
{
	if(map) delete [] map;
}



FIRESITE& CFireSites::operator() (int i, int j)
{
	if (i<=0 || i> m_iRows || j<=0 || j> m_iCols)
	{
		char err[80];

		sprintf(err,"CFireSites::operator() (int,int)-> (%d, %d) are illegal map\
                  coordinates",i,j);

		errorSys(err,STOP);
    }

	int x = (i - 1) * m_iCols + j - 1;

	return map[x];
}






void CFireSites::create_FireRegimeUnitsListByIndex()
{
	for (int i=1;i<=m_iRows;i++)
	{
		for (int j=1;j<=m_iCols;j++)
		{	
			int x=(i - 1) * m_iCols;
			x=x + j - 1;
			LDPOINT p(j,i);
			FireRegimeUnitsList[map[x].FRUIndex].push_back(p);
		}
	}

	for(int i=0;i<70000;i++)
		FireRegimeCurrentIndex[i] = 0;
}



LDPOINT CFireSites::return_a_point_from_index(int index, int cellNo)
{
	if(cellNo>FireRegimeUnitsList[index].size()){
		errorSys("Fire index error, contact development team\n",STOP);
	}
	cellNo--;
	LDPOINT p = FireRegimeUnitsList[index].at(cellNo);
	std::vector<LDPOINT>::iterator sitePtr;
	sitePtr=FireRegimeUnitsList[index].begin();
	FireRegimeUnitsList[index].erase(sitePtr+cellNo);
	
	FireRegimeCurrentIndex[index] = (FireRegimeCurrentIndex[index]+1)%70000;
	return p;
}


int	CFireSites::numColumns() { return m_iCols; }

int	CFireSites::numRows()  { return m_iRows; }

//This will return the total number of sites.
int CFireSites::number() { return m_iRows * m_iCols; }




int CFireSites::SITE_compare(int site1_x,int site1_y,int site2_x,int site2_y)
{
	int x = (site1_x - 1) * m_iCols + site1_y - 1;
	int y = (site2_x - 1) * m_iCols + site2_y - 1;

	FIRESITE site1 = map[x];
	FIRESITE site2 = map[y];

	int result = SITE_compare(site1, site2);

	return result;
}




int CFireSites::SITE_compare(FIRESITE site1,FIRESITE site2)

{

	if(site1.FRUIndex>site2.FRUIndex)

		return 1;

	if(site1.FRUIndex<site2.FRUIndex)

		return 2;

	if(site1.DEM>site2.DEM)

		return 1;

	if(site1.DEM<site2.DEM)

		return 2;

	return 0;

}

void CFireSites::fillinSitePt(int i, int j, FIRESITE site)
{
	int x = (i - 1) * m_iCols + j - 1;
	
	map[x] = site;
}



FIRESITE CFireSites::locateSitePt(int i, int j)
{
	int x = (i - 1) * m_iCols + j - 1;

	return map[x];
}



int CFireSites::compare_FIRESITE(FIRESITE site1, FIRESITE site2)
{
	if (site1.DEM == site2.DEM && site1.FRUIndex == site2.FRUIndex && site1.numofsites == site2.numofsites)
		return 0;
	else
		return -1;
}



//When a site disappears, delete it
int CFireSites::SITE_delete(int pos_sortIndex, FIRESITE site, int i, int j)
{
	int x =(i - 1) * m_iCols + j - 1;

	//site != SortedIndex.at(pos_sortIndex)
	if(compare_FIRESITE(site, SortedIndex.at(pos_sortIndex)))
		return 0;

	//if(site != map[x])
	if(compare_FIRESITE(site, map[x]))
		return 0;

	SortedIndex.erase(SortedIndex.begin() + pos_sortIndex);

	return 1;
}





//when there is a new site during succession or whatever, we need to 
//check if the new site already exists, if yes combine with existing one
//if not insert to the position according to sort
int CFireSites::SITE_insert(int pos_sortIndex, FIRESITE site, int i, int j)
{
	int ifexist=0, pos=0;

	int x=(i - 1) * m_iCols + j - 1;

	SITE_LocateinSortIndex(site, &pos,&ifexist);

	if(ifexist)
	{
		map[x] = SortedIndex.at(pos);
		
		map[x].numofsites++;		
	}
	else
	{	FIRESITE temp;

		temp.FRUIndex=site.FRUIndex;

		temp.DEM=site.DEM;

		temp.numofsites=1;

		map[x]=temp;

		SortedIndex.insert(SortedIndex.begin() + pos, temp);
	}

	return 1;

}





//Find if a new site exists in sorted list

//If a new site exists, find its location and set *ifexist as 1

//if this no site matches this one, find location before which new site pointer should be inserted
int CFireSites::SITE_LocateinSortIndex(FIRESITE site, int *pos, int *ifexist)
{

	int begin, end, mid;

	int temp_flag;

	*ifexist=0;

	begin=0;

	end=SortedIndex.size();

	if(end==0) {printf("No site at all wrong wrong wrong\n"); return -1;}

	end--;

	mid=(begin+end)/2;

	FIRESITE temp = SortedIndex.at(mid);

	while(begin<end)
	{
		temp_flag=SITE_compare(site,temp);

		if(temp_flag==0)
		{
			*ifexist=1;
			*pos=mid;
			return 1;
		}

		else if(temp_flag==1)

			{

				begin=mid+1;

				mid=(begin+end)/2;

			}

		else if(temp_flag==2)

			{

				end=mid-1;

				mid=(begin+end)/2;

			}

		else

			{

				return -1;

			}

		temp=SortedIndex.at(mid);

	}

	temp_flag = SITE_compare(site,temp);

	if(temp_flag==0)
	{
		*ifexist=1;

		*pos=mid;

		return 1;
	}
	else if(temp_flag==2)
	{
		*ifexist=0;

		*pos=mid;

		return 0;
	}
	else if(temp_flag==1)
	{
		*ifexist=0;

		*pos=mid+1;

		return 0;
	}
	else
	{
		return -1;
	}
}




//use babble algorithm to sort the initial site list array
int CFireSites::SITE_sort()
{
	const int size=SortedIndex.size();

	for(int i=size-1; i>0; i--)
	{
		for(int j=0; j<=i-1; j++)
		{
			FIRESITE site1 = SortedIndex.at(j);
			FIRESITE site2 = SortedIndex.at(j+1);

			if(SITE_compare(site1, site2)==1)
			{
				FIRESITE temp=SortedIndex.at(j);

				SortedIndex.at(j)=SortedIndex.at(j+1);

				SortedIndex.at(j+1)=temp;
			}
		}
	}

	return 1;
}





//Before Site Change
//This function back up a site and following changes are based on this seprated site
//sort vector is not touched here
void CFireSites::BefStChg(int i, int j)
{
	FIRESITE temp = locateSitePt(i,j);

	sitetouse.FRUIndex = temp.FRUIndex;
	sitetouse.DEM = temp.DEM;

	if(temp.numofsites==1)
	{
		int pos, ifexist=0;

		SITE_LocateinSortIndex(sitetouse,&pos,&ifexist);

		if(ifexist)
		{
			SortedIndex.erase(SortedIndex.begin() + pos);
		}
		else
		{
			printf("num of vectors %d\n",SortedIndex.size());

			printf("ERROR ERROR ERROR ERROR!!~~~%d\n",pos);				
		}
	}
	else if(temp.numofsites<=0)
		printf("FIRESITE NO NO NO NO NO\n");
	else
		temp.numofsites--;

	fillinSitePt(i,j, sitetouse);
}





//After Site Change
//This function does combination and delete of the seprated site made by BefStChg(int i, int j)
//insert this site to the sorted vector
void CFireSites::AftStChg(int i,int j)
{
	SITE_insert(0,sitetouse,i,j);
}

