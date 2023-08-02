#ifndef _PARAMS_H
#define _PARAMS_H

#define MAX_INSERT_PACKAGE 1000000
//_1131530

#define MAX_HASH_NUM 20

#define FILTER_SIZE 32

#define COUNTER_SIZE 16

#define LOW_HASH_NUM 5

#define MAX_MEM 400000000

#define N 400000000

#define KEY_LEN 13

typedef long long lint;
typedef unsigned int uint;

#define WORD_SIZE 64


const uint32_t d = 5;
const uint32_t cs[] = { 1, 2, 3, 4, 5 };
const uint32_t cpw[] = { 4, 3, 2, 1, 0 };
const uint32_t lo[] = { 0xf, 0x7, 0x3, 0x1, 0x0 };
const uint32_t mask[] = { 0x3, 0xf, 0xff, 0xffff, 0xffffffff };


#endif //_PARAMS_H
