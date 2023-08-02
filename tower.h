#ifndef _TOWER_H
#define _TOWER_H

#include <cstring>
#include "params.h"
#include "murmur3.h"
#include "BOBHash.h"

class TowerSketch
{
protected:
	uint32_t w[d];
	uint32_t* A[d];
	uint32_t hashseed[d];
	int idx[d];
	BOBHash * bobhash[MAX_HASH_NUM];

public:
	TowerSketch() {}
	TowerSketch(uint32_t w_d) { init(w_d); }
	~TowerSketch() { clear(); }

	void init(uint32_t w_d)
	{
		for (int i = 0; i < d; ++i)
		{
			w[i] = w_d << d - i - 1;
			A[i] = new uint32_t[w_d];
			memset(A[i], 0, w_d * sizeof(uint32_t));
			//hashseed[i] = rand() % MAX_PRIME32;
			bobhash[i] = new BOBHash(i + 1000);
		}
	}

	void clear()
	{
		for (int i = 0; i < d; ++i)
			delete[]A[i];
	}

	void Insert(const char* key, uint16_t key_len=0)
	{
		uint final_key_len = (key_len == 0)? strlen(key): key_len;
		for (int i = 0; i < d; ++i)
			idx[i] = (bobhash[i]->run(key, final_key_len)) % w[i];
			//idx[i] = MurmurHash3_x86_32(key, final_key_len, hashseed[i]) % w[i];
		for (int i = 0; i < d; ++i)
		{
			uint32_t& a = A[i][idx[i] >> cpw[i]];
			uint32_t shift = (idx[i] & lo[i]) << cs[i];
			uint32_t val = (a >> shift) & mask[i];
			a += (val < mask[i]) ? (1 << shift) : 0;
		}
	}

	uint32_t Query(const char* key, uint16_t key_len=0)
	{
		uint final_key_len = (key_len == 0)? strlen(key): key_len;
		uint32_t ret = UINT32_MAX;
		for (int i = 0; i < d; ++i)
			idx[i] = (bobhash[i]->run(key, final_key_len)) % w[i];
		for (int i = 0; i < d; ++i)
		{
			//uint32_t idx = MurmurHash3_x86_32(key, final_key_len, hashseed[i]) % w[i];
			uint32_t a = A[i][idx[i] >> cpw[i]];
			uint32_t shift = (idx[i] & lo[i]) << cs[i];
			uint32_t val = (a >> shift) & mask[i];
			ret = (val < mask[i] && val < ret) ? val : ret;
		}
		return ret;
	}
};

class TowerSketchCU : public TowerSketch
{
public:
	TowerSketchCU() {}
	TowerSketchCU(uint32_t w_d) { init(w_d); }
	~TowerSketchCU() {}

	void Insert(const char* key, uint16_t key_len)
	{
		uint32_t min_val = UINT32_MAX;
		for (int i = 0; i < d; ++i)
			idx[i] = (bobhash[i]->run(key, key_len)) % w[i];
			//idx[i] = MurmurHash3_x86_32(key, key_len, hashseed[i]) % w[i];
		for (int i = 0; i < d; ++i)
		{
			uint32_t a = A[i][idx[i] >> cpw[i]];
			uint32_t shift = (idx[i] & lo[i]) << cs[i];
			uint32_t val = (a >> shift) & mask[i];
			min_val = (val < mask[i] && val < min_val) ? val : min_val;
		}
		for (int i = 0; i < d; ++i)
		{
			uint32_t& a = A[i][idx[i] >> cpw[i]];
			uint32_t shift = (idx[i] & lo[i]) << cs[i];
			uint32_t val = (a >> shift) & mask[i];
			a += (val < mask[i] && val == min_val) ? (1 << shift) : 0;
		}
	}
};

#endif
