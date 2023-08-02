#ifndef _CMSKETCH_H
#define _CMSKETCH_H

#include <algorithm>
#include <cstring>
#include <string.h>
#include "params.h"
#include "BOBHash.h"
#include <iostream>

using namespace std;

class CMSketch
{	
private:
	BOBHash * bobhash[MAX_HASH_NUM];
	int index[MAX_HASH_NUM];
	int *counter[MAX_HASH_NUM];
	int w, d;
	int MAX_CNT;
	int counter_index_size;
	uint64_t hash_value;

public:
	CMSketch(int _w, int _d)
	{
		counter_index_size = 20;
		w = _w;
		d = _d;
		
		for(int i = 0; i < d; i++)	
		{
			counter[i] = new int[w];
			memset(counter[i], 0, sizeof(int) * w);
			bobhash[i] = new BOBHash(i + 1000);
		}

		//MAX_CNT = MAX_INSERT_PACKAGE;//(1 << COUNTER_SIZE) - 1;
		MAX_CNT = (1 << COUNTER_SIZE) - 1;
		
		/*for(int i = 0; i < d; i++)
		{
			
		}*/
	}
	void Insert(const char * str, uint key_len=0)
	{
		uint final_key_len = (key_len == 0)? strlen(str): key_len;
		for(int i = 0; i < d; i++)
		{
			index[i] = (bobhash[i]->run(str, final_key_len)) % w;
		}
		for(int i = 0; i < d; i++)
		{
			uint val=counter[i][index[i]];
			counter[i][index[i]] += val<MAX_CNT?1:0;
			/*if(counter[i][index[i]] != MAX_CNT)
			{
				counter[i][index[i]]++;
			}*/
		}
	}
	double Query(const char *str, uint key_len=0)
	{
		int min_value = MAX_CNT;
		int temp;
		uint final_key_len = (key_len == 0)? strlen(str): key_len;
		for(int i = 0; i < d; i++)
		{
			index[i] = (bobhash[i]->run(str, final_key_len)) % w;
		}
		for(int i = 0; i < d; i++)
		{
			//index[i] = (bobhash[i]->run(str, final_key_len)) % w;
			temp = counter[i][index[i]];
			min_value = temp < min_value ? temp : min_value;
		}
		return min_value;
	
	}
	void Delete(const char * str, uint key_len=0)
	{
		for(int i = 0; i < d; i++)
		{
			index[i] = (bobhash[i]->run(str, key_len)) % w;
			counter[i][index[i]] --;
		}
	}
	~CMSketch()
	{
		for(int i = 0; i < d; i++)	
		{
			delete []counter[i];
		}


		for(int i = 0; i < d; i++)
		{
			delete bobhash[i];
		}
	}
};
#endif//_CMSKETCH_H
