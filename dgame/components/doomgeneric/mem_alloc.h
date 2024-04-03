// The real memory allocator
#pragma once

void* mem_alloc( int size );
void* mem_calloc( int count, int size );

void mem_free( void* p );
