#include "immintrin.h"
#include "murmur3.h"
#include "params.h"
#include <cstring>
#include <random>
#include <set>
#include <cstdio>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <iostream>
#include <bitset>

#define MAX_LAYER 6
#define MAXN 32 // max bit in a bucket
#define MAXLEN 1<<22
#define Max(a,b) ((a) > (b) ? (a) : (b))
#define Min(a,b) ((a) < (b) ? (a) : (b))
const int counter_size[6]={0,2,4,8,16,0};
const int flag_size[6]={0,8,12,14,15};
const uint32_t Mask[6]={3u,15u,255u,65535u,4294967295u};
class sketch_v1
//2 bit start limit layers' max size and add flag
{
public:
    uint32_t **counters;
    uint32_t counter_len[10];
    int counter_layer;
    uint32_t actual_len,actual_totlen;
    uint32_t seeds[MAX_LAYER];
    uint32_t *flag;
    int offset[MAX_LAYER];//used for flag
    uint32_t index[MAX_HASH_NUM];
    sketch_v1(){}
    sketch_v1(uint32_t len,uint32_t layer,uint32_t random_seed,double *layer_ratio) 
    //len must be mutiply of 512
    {
        double total_ratio=0.0,ret1=4.0;
        for(int i=1;i<layer;i++)
        {
            total_ratio+=layer_ratio[i]/ret1;
            ret1=ret1*2;
        }
        uint32_t bitscnt=(int)((double)len/(total_ratio+1.0));
        bitscnt=(bitscnt/32+1)*32;
        counter_layer=layer;
        std::set<int>seedset;
        uint32_t seed;
        uint32_t offset_cnt=0;
        counters=new uint32_t* [layer];
        for(int i=0;i<layer;i++)
        {
            counter_len[i]=(uint)(layer_ratio[i]*bitscnt);
            counter_len[i]=((counter_len[i]-1)/32)*32+1;
            counters[i]=new uint32_t[counter_len[i]/16];
            memset(counters[i],0,sizeof(uint32_t)*(counter_len[i]/16));
            offset[i]=offset_cnt;
            if(i)offset_cnt+=counter_len[i]>>i;
            seed=rand()%MAX_PRIME32;
            while(seedset.find(seed)!=seedset.end())
            {
                seed=rand()%MAX_PRIME32;
            }
            seeds[i]=seed;
            seedset.insert(seed);
            
        }

        flag=new uint32_t[offset_cnt];
        memset(flag,0,sizeof(uint32_t)*(offset_cnt));
    }
    ~sketch_v1(){clear();}
    void clear()
    {
        for(int i=0;i<counter_layer;i++)
        {
            delete [] counters[i];
        }
        delete [] counters;
        delete [] flag;
        for(int i=0;i<counter_layer;i++)
        {  
            seeds[i]=0;
        }
    }
    uint32_t merge(uint32_t value,uint32_t posx,uint32_t posy,uint32_t merge_cnt)
    //no use
    {
        uint32_t cnt_x=(value>>(posx<<1u))&((1u<<(1u<<(merge_cnt+1u)))-1u);
        uint32_t cnt_y=(value>>(posy<<1u))&((1u<<(1u<<(merge_cnt+1u)))-1u);
        return Max(cnt_x,cnt_y);
    }
    uint32_t query_flag(int layer,uint32_t pos)
    {
        if(layer==0)return 0;
        int offsetx=offset[layer];
        uint32_t x=(offsetx+(pos>>layer))/32,y=(offsetx+(pos>>layer))%32;
        return (flag[x]>>y)&1;
    }
    void modify_flag(int layer,uint32_t pos)
    //use int to store,can be optimized to bool
    {
        int offsetx=offset[layer];
        uint32_t ret=offsetx+(pos>>layer);
        uint32_t x=ret>>5u,y=ret&31u;
        flag[x]|=1u<<y;
    }
    void Insert(const char *key, uint key_len=0)
    //CM_version
    {
        uint final_key_len = (key_len == 0)? strlen(key): key_len;
        uint32_t min_cnt=UINT32_MAX-1u;
        for (uint32_t i=0; i<counter_layer; i++){
            index[i] = MurmurHash3_x86_32(key, final_key_len, seeds[i])%counter_len[i];
        }
        for(uint32_t i=0;i<counter_layer;i++)
        {
            uint32_t ret = index[i];
            //uint32_t ret=MurmurHash3_x86_32(key, final_key_len, seeds[i])%counter_len;
            uint32_t pos_div=ret>>4u,pos=ret&15u;
            uint32_t counter_value=counters[i][pos_div];
            uint32_t merge_cnt=query_flag(i,ret)+(i==0?0:i-1);
            uint32_t pos_real=pos&(~((1ll<<merge_cnt)-1ll));//pos after merge
            uint32_t true_cnt=(counter_value>>(pos_real<<1ll))&Mask[merge_cnt];  // true cnt in pos
            uint32_t mask=Mask[merge_cnt]; //max in counter
            uint32_t now_min_cnt=min_cnt;
            if(now_min_cnt>true_cnt)now_min_cnt=true_cnt;
            if(now_min_cnt==true_cnt)true_cnt++;
            uint32_t pos_xor;
            if(true_cnt>mask)
            {
                if(merge_cnt<i)
                {
                    //merge
                    pos_xor=pos_real^(1u<<(merge_cnt));
                    modify_flag(i,ret);
                    merge_cnt++;
                    pos_real=Min(pos_real,pos_xor);
                }
                else
                {
                    continue;
                }
            }
            min_cnt=now_min_cnt;
            uint32_t ret_fornew=~(Mask[merge_cnt]<<(pos_real<<1u));
            uint32_t new_counter_value=(counter_value&ret_fornew)|(true_cnt<<(pos_real<<1u));
            counters[i][pos_div]=new_counter_value;
        }
    }
    int Query(const char *key,uint key_len=0)
    {
        uint32_t x=UINT32_MAX-1u;
        uint final_key_len = (key_len == 0)? strlen(key): key_len;
        for(uint32_t i=0;i<counter_layer;i++)
        {
            uint32_t ret=MurmurHash3_x86_32(key, final_key_len, seeds[i])%counter_len[i];
            uint32_t pos_div=ret>>4u,pos=ret&15u;
            uint32_t counter_value=counters[i][pos_div];
            uint32_t merge_cnt=query_flag(i,ret)+(i==0?0:i-1);
            uint32_t pos_real=pos&(~((1u<<merge_cnt)-1u));//pos after merge
            uint32_t true_cnt=(counter_value>>(pos_real<<1u))&Mask[merge_cnt];
            uint32_t mask=Mask[i]; //max in counter
            if(true_cnt==mask)continue;
            x=true_cnt<x?true_cnt:x;
        }
        return x;
    }
};