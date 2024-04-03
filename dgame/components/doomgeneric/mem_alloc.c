#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "mem_alloc.h"
#include "esp_heap_caps.h"

static int total_used = 0;

void* mem_alloc( int size )
{
    total_used += size;
    printf( "[MEM] block: %d, total: %d\n", size, total_used );
    return heap_caps_malloc( size, MALLOC_CAP_32BIT | MALLOC_CAP_SPIRAM  );
}

void* mem_calloc( int count, int size )
{
    total_used += ( count * size );
    printf( "[MEM] block: %d, total: %d\n", count * size , total_used );
    void* p = mem_alloc( count * size );
    if( p ) {
        memset( p, 0, count * size );
    }
    return p;
}

void mem_free( void* p )
{
    free( p );
}
