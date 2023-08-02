#include <stdio.h>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <string.h>
#include <ctime>
#include <time.h>
#include <iterator>
#include <math.h>
#include <vector>
#include <map>

#include "CUSketch.h"
//#include "ASketch.h"
#include "PCUSketch.h"
//#include "NitroSketch.h"
//#include "ElasticSketch.h"
//#include "MVSketch.h"
//#include "BitMatcher.h"
//#include "dms4.h"
#include "SalsaCU.h"
#include "Dripsketch_v6CU.h"
#include "tower.h"
#include "StingyCM_Base.h"
#include "StingyCU_Base.h"


using namespace std;


char * filename_stream = "../data/1.dat";


char insert[60000000 + 10000000 / 2][200];
char query[60000000 + 10000000 / 2][200];


unordered_map<string, int> unmp;

#define testcycles 3


int main(int argc, char** argv)
{
    double memory = 0.1;	//MB
    double p=0.88;
    double precent=0.0;
    bool CM_flag= true;
    if(argc >= 2)
    {
        filename_stream = argv[1];
    }
    if (argc >= 3)
    {
    	memory = stod(argv[2]);
    }
    if(argc >= 4)
    {
        p=stod(argv[3])/100.0;
    }

    //cout<<p<<endl;
    unmp.clear();
    int val;

    int memory_ = memory * 1000;//KB
    int word_size = 64;


    int w = memory * 1000 * 1024 * 8.0 / COUNTER_SIZE;	//how many counter (CM,CU);
    int w_drip = memory * 1024 * 1024 * 8.0 / 2 ; //how many 2-bit counter (drip);

    int w_p = memory * 1024 * 1024 * 8.0 / (word_size * 2);
    //int m1 = memory * 1024 * 1024 * 1.0/4 / 8 / 12;
    //int m2 = memory * 1024 * 1024 * 3.0/4 / 2 / 1;
    //int m2_mv = memory * 1024 * 1024 / 4 / 4;
    //int w_dhs = memory * 1000 * 1024 / 16;
    int w_salsa = memory * 1000 * 1024;     

    printf("\n******************************************************************************\n");
    printf("Evaluation starts!\n\n");

    
    CUSketch *cusketch;
    //ASketch *asketch;
    
    //BitMatcher *bmatcher;
    //Nitrosketch *nitrosketch;
    //Elasticsketch *elasticsketch;
    SalsaCM *salsacu;
    StingyCU_Base *stingycu;
    TowerSketchCU *towersketch;
    PCUSketch *pcusketch;
    sketch_v1 *dripsketch;


    char _temp[200], temp2[200];
    int t = 0;

    int package_num = 0;

    char timestamp[8];

    FILE *file_stream = fopen(filename_stream, "r");
    while( fread(insert[package_num], 1, KEY_LEN, file_stream)==KEY_LEN ) //for the rest
    {
        string str = string(insert[package_num], KEY_LEN);
        unmp[str]++;
        package_num++;

        if(package_num == MAX_INSERT_PACKAGE)
            break;
    }
    fclose(file_stream);

    printf("memory = %dKB\n", memory_);
    printf("dataset name: %s\n", filename_stream);
    printf("total stream size = %d\n", package_num);
    printf("distinct item number = %d\n", unmp.size());
  
    int max_freq = 0;
    unordered_map<string, int>::iterator it = unmp.begin();

    for(int i = 0; i < unmp.size(); i++, it++)
    {
        memcpy(query[i], it->first.c_str(), KEY_LEN);

        int temp2 = it->second;
        max_freq = max_freq > temp2 ? max_freq : temp2;
    }
    printf("max_freq = %d\n", max_freq);
    
    printf("*************************************\n");


/*************************************************************************/
/*************************************************************************/
/******************************** Insert *********************************/
/*************************************************************************/
/*************************************************************************/

    timespec time1, time2;
    long long resns;


    //printf("%lf\n", memory);



    clock_gettime(CLOCK_MONOTONIC, &time1);
    for(int t = 0; t < testcycles; t++)
    {
        cusketch = new CUSketch(w / LOW_HASH_NUM, LOW_HASH_NUM);
        for(int i = 0; i < package_num; i++)
        {
            cusketch->Insert(insert[i], KEY_LEN);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    double throughput_cm = (double)1000.0 * testcycles * package_num / resns;
    //printf("throughput of CM (insert): %.6lf Mips\n", throughput_cm);
    printf("%.6lf ", throughput_cm);
    

    clock_gettime(CLOCK_MONOTONIC, &time1);
    for(int t = 0; t < testcycles; t++)
    {
        salsacu = new SalsaCM(w_salsa, 5, 100);
        for(int i = 0; i < package_num; i++)
        {
            salsacu->Insert(insert[i], KEY_LEN);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    double throughput_salsacu = (double)1000.0 * testcycles * package_num / resns;
    //printf("throughput of SALSA (insert): %.6lf Mips\n", throughput_salsacu);
    printf("%.6lf ", throughput_salsacu);

    
    
    clock_gettime(CLOCK_MONOTONIC, &time1);
    for(int t = 0; t < testcycles; t++)
    {
        stingycu = new StingyCU_Base(w_salsa, 5, 100);
        for(int i = 0; i < package_num; i++)
        {
            stingycu->Insert(insert[i], KEY_LEN);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    double throughput_stingy = (double)1000.0 * testcycles * package_num / resns;
    //printf("throughput of SALSA (insert): %.6lf Mips\n", throughput_salsacu);
    printf("%.6lf ", throughput_stingy);

    

    clock_gettime(CLOCK_MONOTONIC, &time1);
    for(int t = 0; t < testcycles; t++)
    {
        towersketch = new TowerSketchCU(w / LOW_HASH_NUM / 2);   //32bit per counter
        for(int i = 0; i < package_num; i++)
        {
            towersketch->Insert(insert[i], KEY_LEN);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    double throughput_towersketch = (double)1000.0 * testcycles * package_num / resns;
    //printf("throughput of Tower (insert): %.6lf Mips\n", throughput_towersketch);
    printf("%.6lf ", throughput_towersketch);

    clock_gettime(CLOCK_MONOTONIC, &time1);
    for(int t = 0; t < testcycles; t++)
    {
        pcusketch = new PCUSketch(w_p, 4, word_size);
        for(int i = 0; i < package_num; i++)
        {
            pcusketch->Insert(insert[i], KEY_LEN);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    double throughput_pcusketch = (double)1000.0 * testcycles * package_num / resns;
    //printf("throughput of PCU (insert): %.6lf Mips\n", throughput_pcusketch);
    printf("%.6lf ", throughput_pcusketch);

    

    clock_gettime(CLOCK_MONOTONIC, &time1);
    double ratio[5]={0.11,0.18,0.27,0.29,0.15};//0 to 4
    for(int t = 0; t < testcycles; t++)
    {
        dripsketch = new sketch_v1(w_drip, LOW_HASH_NUM, 19260817,ratio);
        for(int i = 0; i < package_num; i++)
        {
            dripsketch->Insert(insert[i], KEY_LEN);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    double throughput_dripsketch = (double)1000.0 * testcycles * package_num / resns;
    //printf("throughput of Drip (insert): %.6lf Mips\n", throughput_dripsketch); 
    printf("%.6lf\n", throughput_dripsketch);

    

//return 0;


    printf("*************************************\n");

/*************************************************************************/
/*************************************************************************/
/********************************* Query *********************************/
/*************************************************************************/
/*************************************************************************/



    double res_tmp=0;
	//double query_temp = 0;
    int flow_num = unmp.size();

    double sum = 0;


    clock_gettime(CLOCK_MONOTONIC, &time1);
    for(int t = 0; t < testcycles; t++)
    {
        for(int i = 0; i < flow_num; i++)
        {
            res_tmp = cusketch->Query(query[i], KEY_LEN);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    throughput_cm = (double)1000.0 * testcycles * flow_num / resns;
    //printf("throughput of CM (query): %.6lf Mips\n", throughput_cm);
    sum += res_tmp;
    printf("%.6lf ", throughput_cm);

    
    clock_gettime(CLOCK_MONOTONIC, &time1);
    for(int t = 0; t < testcycles; t++)
    {
        for(int i = 0; i < flow_num; i++)
        {
            res_tmp = salsacu->Query(query[i], KEY_LEN);  
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    throughput_salsacu = (double)1000.0 * testcycles * flow_num / resns;
    //printf("throughput of SALSA (query): %.6lf Mips\n", throughput_salsacu);
    sum += res_tmp;
    printf("%.6lf ", throughput_salsacu);


    clock_gettime(CLOCK_MONOTONIC, &time1);
    for(int t = 0; t < testcycles; t++)
    {
        for(int i = 0; i < flow_num; i++)
        {
            res_tmp = stingycu->Query(query[i], KEY_LEN);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    throughput_stingy = (double)1000.0 * testcycles * flow_num / resns;
    //printf("throughput of CU (query): %.6lf Mips\n", throughput_cu);
    sum += res_tmp;
    printf("%.6lf ", throughput_stingy);



    clock_gettime(CLOCK_MONOTONIC, &time1);
    for(int t = 0; t < testcycles; t++)
    {
        for(int i = 0; i < flow_num; i++)
        {
            res_tmp = towersketch->Query(query[i], KEY_LEN);  
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    throughput_towersketch = (double)1000.0 * testcycles * flow_num / resns;
    //printf("throughput of Tower (query): %.6lf Mips\n", throughput_towersketch);
    printf("%.6lf ", throughput_towersketch);
    sum += res_tmp;


    clock_gettime(CLOCK_MONOTONIC, &time1);
    for(int t = 0; t < testcycles; t++)
    {
        for(int i = 0; i < flow_num; i++)
        {
            res_tmp = pcusketch->Query(query[i], KEY_LEN);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    throughput_pcusketch = (double)1000.0 * testcycles * flow_num / resns;
    //printf("throughput of PCU (query): %.6lf Mips\n", throughput_pcusketch);   
    printf("%.6lf ", throughput_pcusketch);
    sum += res_tmp;




    clock_gettime(CLOCK_MONOTONIC, &time1);
    for(int t = 0; t < testcycles; t++)
    {
        for(int i = 0; i < flow_num; i++)
        {
            res_tmp = dripsketch->Query(query[i], KEY_LEN);  
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    throughput_dripsketch = (double)1000.0 * testcycles * flow_num / resns;
    //printf("throughput of Drip (query): %.6lf Mips\n", throughput_dripsketch);
    sum += res_tmp;
    printf("%.6lf\n", throughput_dripsketch);


    
/********************************************************************************************/
    printf("*************************************\n");

    //avoid the over-optimize of the compiler! 
    if(sum == (1 << 30))
        return 0;

    char temp[200];

    double re_cm = 0.0, re_stingy = 0.0,  re_a = 0.0,  re_pcusketch = 0.0, re_pcsketch = 0.0, re_bmatcher = 0.0, re_elastic=0.0, re_nitro=0.0, re_salsacu=0.0, re_drip=0.0, re_tower=0.0;
    double re_cm_sum = 0.0, re_stingy_sum = 0.0,  re_a_sum = 0.0,  re_pcusketch_sum = 0.0, re_bmatcher_sum = 0.0, re_elastic_sum=0.0, re_nitro_sum=0.0, re_salsacu_sum=0.0, re_drip_sum=0.0, re_tower_sum=0.0;
    
    double ae_cm = 0.0, ae_stingy = 0.0,  ae_a = 0.0,  ae_pcusketch = 0.0, ae_bmatcher = 0.0, ae_elastic=0.0, ae_nitro=0.0, ae_salsacu=0.0, ae_drip=0.0, ae_tower=0.0;
    double ae_cm_sum = 0.0, ae_stingy_sum = 0.0,  ae_a_sum = 0.0,  ae_pcusketch_sum = 0.0, ae_bmatcher_sum = 0.0, ae_elastic_sum=0.0, ae_nitro_sum=0.0, ae_salsacu_sum=0.0, ae_drip_sum=0.0, ae_tower_sum=0.0;

    double val_cm = 0.0, val_stingy = 0.0,  val_a = 0.0,  val_pcusketch = 0.0, val_bmatcher = 0.0, val_elastic=0.0, val_nitro=0.0, val_salsacu=0.0, val_drip=0.0, val_tower=0.0;
    double erro_cm = 0.0, erro_stingy = 0.0, erro_a = 0.0,  erro_pcusketch = 0.0, erro_bmatcher = 0.0, erro_elastic=0.0, erro_nitro=0.0, erro_salsacu=0.0, erro_drip=0.0, erro_tower=0.0;
    double mem_cc = 0.0, mem_cc_sum = 0.0;

    //double mark_cm=0, mark_el=0, mark_cc=0, mark_pcu=0, mark_nitro=0, mark_mv=0;

    for(unordered_map<string, int>::iterator it = unmp.begin(); it != unmp.end(); it++)
    {
        memcpy(temp, (it->first).c_str(), KEY_LEN);
        val = it->second;
        //if(val==0)continue;

        val_cm = cusketch->Query(temp, KEY_LEN)*1.0;
        val_stingy = stingycu->Query(temp, KEY_LEN)*1.0;    
	    val_salsacu = salsacu->Query(temp, KEY_LEN)*1.0;
        val_drip = dripsketch->Query(temp, KEY_LEN)*1.0;
        val_tower = towersketch->Query(temp, KEY_LEN)*1.0; 
        val_pcusketch = pcusketch->Query(temp, KEY_LEN)*1.0;
        
        re_cm = fabs(val_cm - val) / (max(val,1) * 1.0);
        re_stingy = fabs(val_stingy - val) / (max(val,1) * 1.0);
	    re_salsacu = fabs(val_salsacu - val) / (max(val,1) * 1.0);
        re_drip = fabs(val_drip - val) / (max(val,1) * 1.0);
        re_tower = fabs(val_tower - val) / (max(val,1) * 1.0);
        re_pcusketch = fabs(val_pcusketch - val) / (max(val,1) * 1.0);

        ae_cm = fabs(val_cm - val);
        ae_stingy = fabs(val_stingy - val);       
	    ae_salsacu = fabs(val_salsacu-val);
        ae_drip = fabs(val_drip - val); 
        ae_tower = fabs(val_tower - val);  
        ae_pcusketch = fabs(val_pcusketch - val); 
        //if(ae_drip>1.0)printf("(%.lf %.lf %.lf %.lf)\n",val_drip,val,ae_drip,re_drip);
        
        re_cm_sum += re_cm;
        re_stingy_sum += re_stingy;          
	    re_salsacu_sum += re_salsacu;
        re_drip_sum += re_drip;
        re_tower_sum += re_tower;  
        re_pcusketch_sum += re_pcusketch; 

        ae_cm_sum += ae_cm;
        ae_stingy_sum += ae_stingy;      
	    ae_salsacu_sum += ae_salsacu;
        ae_drip_sum += ae_drip;  
        ae_tower_sum += ae_tower;
        ae_pcusketch_sum += ae_pcusketch;
    }
//return 0;
    double a = package_num * 1.0;
    double b = unmp.size() * 1.0;

	printf("*************************************\n");
    /*printf("aae_cm = %lf\n", ae_cm_sum / b);
    printf("aae_cu = %lf\n", ae_cu_sum / b);
	printf("aae_SALSA = %lf\n", ae_salsacu_sum / b);
    //printf("aae_tower = %lf\n", ae_tower_sum / b);
    printf("aae_drip = %lf\n", ae_drip_sum / b); */ 

    printf("%lf ", ae_cm_sum / b);
    printf("%lf ", ae_stingy_sum / b);
	printf("%lf ", ae_salsacu_sum / b);
    printf("%lf ", ae_tower_sum / b);
    printf("%lf ", ae_pcusketch_sum / b);
    printf("%lf\n", ae_drip_sum / b); 
    //return 0;
    printf("%lf ", re_cm_sum / b);
    printf("%lf ", re_stingy_sum / b);
	printf("%lf ", re_salsacu_sum / b);
    printf("%lf ", re_tower_sum / b);
    printf("%lf ", re_pcusketch_sum / b);
    printf("%lf\n\n\n", re_drip_sum / b); 
    return 0;
}
