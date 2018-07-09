#ifndef DS_UTIL_H_
#define DS_UTIL_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

/* comment or uncomment to timed/or not, functions run */
#define __TIME_RUN__


/* swap using xor */
#define _XOR_SWAP(a,b) 								\
	{												\
		bsize = sizeof(a[0]);						\
		size /= bsize;								\
		do {										\
			*a ^= *b;								\
			*b ^= *a;								\
			*a++ ^= *b++;							\
		}											\
		while(--size > 0);							\
	}

/* swap using a temp variable */
#define _SWAP(a,b,t)							 	\
	{												\
		bsize = sizeof(a[0]);						\
		size /= bsize;								\
		do {										\
			t c = *a;								\
			*a = *b;								\
			*b++ = c;								\
			a++; 									\
		}											\
		while(--size > 0);							\
	}


/* 
 * swap 2 values
 *
 * @param a - value 1
 * @param b - value 2
 * @param size - sizeof a and b (bytes)
 *
 * NOTE:
 * size % sizeof(type) - ensures size is a multiple of the given type
 * size this avoid reading/writing over the allocated memory
 * which would cause a segmentation fault
 */

static inline void swap(void* a, void* b, int size)
{
	/* get data in different formats */
	union {
		const void *data;
		uint8_t  *_u8;
		uint16_t *_u16;
		uint32_t *_u32;
		uint64_t *_u64;
	} a1 = {a}, b1 = {b};

#ifdef __TIME_RUN__
	clock_t bc = clock();
#endif 
	int bsize = 0;
	/* if size is multiple of 8 */
	if(size >= sizeof(uint64_t) && (size % sizeof(uint64_t) == 0))
		_XOR_SWAP(a1._u64,b1._u64)

	/* if size is multiple of 4 */
	else if(size >= sizeof(uint32_t) && (size % sizeof(uint32_t) == 0))
		_XOR_SWAP(a1._u32,b1._u32)

	/* if size is multiple of 2 */
	else if(size >= sizeof(uint16_t) && (size % sizeof(uint16_t) == 0))
		_XOR_SWAP(a1._u16,b1._u16)

	/* swap byte by byte */
	else
		_SWAP(a1._u8,b1._u8, uint8_t)

#ifdef __TIME_RUN__
	clock_t ec = clock();
	printf("swap %d byte(s) at a time\n", bsize);
	printf("(swap) takes %lf secs\n",(double)(ec - bc) / CLOCKS_PER_SEC);
#endif
}



/* copy template macro */
#define _COPY(a, b, t) {				\
	size /= (bsize = sizeof(b[0]));		\
	t *ptr = a;							\
	do {								\
		*ptr++  = *b++;					\
	} while(--size > 0);				\
}

/*
 * make a copy of src
 *
 * @param src - what to copy
 * @param size - total size of src
 * @return void* to new copy of src 
 */

static inline void* copy(const void* src, int size)
{
	/* not enough memory, we don't want to use swap */
	if (size / sysconf(_SC_PAGESIZE) > (size_t) sysconf (_SC_PHYS_PAGES))
		return NULL;

	/* data */
	union {
		const void * data;
		uint8_t  *_u8;
		uint16_t *_u16;
		uint32_t *_u32;
		uint64_t *_u64;
	} b = {src}, a;

	a.data = malloc(size);
	if(a.data == NULL)
		return NULL;

#ifdef __TIME_RUN__
	clock_t bc = clock();
#endif 

	int bsize = 0; /* block size & temp var*/

	/* if size is multiple of 8 */
	if(size >= sizeof(uint64_t) && (size % sizeof(uint64_t) == 0))
		_COPY(a._u64, b._u64, uint64_t)

	/* if size is multiple of 4 */
	else if(size >= sizeof(uint32_t) && (size % sizeof(uint32_t) == 0))
		_COPY(a._u32, b._u32, uint32_t)

	/* if size is multiple of 2 */
	else if(size >= sizeof(uint16_t) && (size % sizeof(uint16_t) == 0))
		_COPY(a._u16, b._u16, uint16_t)

	/* copy 1 byte at a time */
	else _COPY(a._u8, b._u8, uint8_t)

#ifdef __TIME_RUN__
	clock_t ec = clock();
	printf("copy %d byte(s) at a time\n", bsize);
	printf("(copy) takes %lf secs\n",(double)(ec - bc) / CLOCKS_PER_SEC);
#endif
	return (void*)a.data;
}

/*
 * compare to values
 *
 * @param a - value 1
 * @param b - value 2
 * @param sizea - size of a
 * @param sizeb - size of b
 * @return 0 - a=b, >0 - a>b, <0 - a<b
 *
 */

#define _COMPARE(a,b)			 					\
	{												\
		bsize = sizeof(a[0]);						\
		size /= bsize;								\
		do {										\
			if(*a++ != *b++)  						\
				return (*(a-1) - *(b-1));			\
		}											\
		while(--size > 0);							\
		return 0; /* a == b	*/						\
	}

static inline int compare(const void *a, int sizea, const void *b, int sizeb)
{
	int diff = 0,  /* result of a - b */
		bsize = 0, /* block size */
		size = (sizea <= sizeb)? sizea : sizeb;

	/* data */
	union {
		const void *data;
		uint8_t  *_u8;
		uint16_t *_u16;
		uint32_t *_u32;
		uint64_t *_u64;
	} a1 = {a}, b1 = {b};

#ifdef __TIME_RUN__
	clock_t bc = clock();
#endif 

	/*compare 64bits block */
	if(size >= sizeof(uint64_t) && (size % sizeof(uint64_t) == 0))
		_COMPARE(a1._u64,b1._u64)

	/*compare 32bits block*/
	else if(size >= sizeof(uint32_t) && (size % sizeof(uint32_t) == 0))
		_COMPARE(a1._u32,b1._u32)

	/* compare 16bits blocks */
	else if(size >= sizeof(uint16_t) && (size % sizeof(uint16_t) == 0))
		_COMPARE(a1._u16,b1._u16)

	/* byte by byte comparison*/
	else
		_COMPARE(a1._u8,b1._u8)

#ifdef __TIME_RUN__
	clock_t ec = clock();
	printf("compare %d byte(s) at a time\n", bsize);
	printf("(compare) takes %lf secs\n",(double)(ec - bc) / CLOCKS_PER_SEC);
#endif
	return diff;
}


#endif /* DS_UTIL_H_ */
