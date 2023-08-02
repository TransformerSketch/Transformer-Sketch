#ifndef _STINGY_H
#define _STINGY_H

#include "params.h"
#include <string.h>
#include <iostream>
//#include "MurmurHash.h"
#include "BOBHash.h"
#include <random>
#include <mmintrin.h>
#include <algorithm>
#define Min(a,b) 	((a) < (b) ? (a) : (b))
#define THR		(0x0f)
using namespace std;
class StingyCM_Base{
private:
	char  *counter;
	uint64_t w, d, hash_seed, logL,wdivd;
	//uint64_t (*hash_func)(const void*, int32_t, uint32_t);
	BOBHash * bobhash[5];
	uint32_t hash_value[5];
public:
	StingyCM_Base(uint32_t _w, uint32_t _d, uint32_t _hash_seed = 1000) {
		d=_d;
		w=_w;
		logL=floor(log2(w/d));
		wdivd=w/d;
		//hash_func = MurmurHash32;
		for (int i = 0; i < d; i++) {
			bobhash[i] = new BOBHash(i + 750);
		}
		hash_seed = _hash_seed;
		counter = new char[_w]();
	}
	void Carry(uint32_t loc) {
		counter[loc] += 0x40;
		if(counter[loc] & 0xc0) return;
		counter[loc] += 0x40;
		uint32_t tmp=loc & (-loc);
		Carry((loc | (tmp << 1)) ^ tmp);
	}
	void Insert(const char* str, uint key_len=0) {
		uint final_key_len = (key_len == 0)? strlen(str): key_len;
		for (int i=0; i<d; i++)
			hash_value[i] = bobhash[i]->run(str, final_key_len);
		for(char i = 0;i < d;i++){
			//uint32_t hash_value = bobhash[i]->run(str, final_key_len);
			//int addr =  (i << logL) + hash_func(str, KEY_LEN, hash_seed+i)%wdivd;
			int addr =  (i << logL) + hash_value[i]%wdivd;
			if((++counter[addr]) & 0x3f)continue;
			counter[addr] -= 0x40;
			Carry(addr|1);
		}
	}
	int Calculate(uint32_t loc) {
		uint32_t query = counter[loc] & 0xc0;
		if (query == 0) return 0;
		uint32_t tmp = loc & (-loc);
		return (query >> 6) + Calculate((loc | (tmp << 1)) ^ tmp) * 0x03;
	}
	int Query(const char* str, uint key_len=0) {
		uint final_key_len = (key_len == 0)? strlen(str): key_len;
		uint32_t query = UINT32_MAX;
		for (int i=0; i<d; i++)
			hash_value[i] = bobhash[i]->run(str, final_key_len);	
		for(char i = 0;i < d;i++){
			//uint32_t hash_value = bobhash[i]->run(str, final_key_len);
			//int addr =  (i << logL) + hash_func(str, KEY_LEN, hash_seed+i)%wdivd;
			int addr =  (i << logL) + hash_value[i]%wdivd;
			query = Min((counter[addr] & 0x3f) + (Calculate(addr | 1) << 6), query);
		}
		return query;
	}
	~StingyCM_Base(){
		 delete[] counter;
	}
};
#endif//_STINGY_H


