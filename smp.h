#ifndef TERRAIN_SMP_H
#define TERRAIN_SMP_H
#include "defs.h"
#include <threads.h>
#include <stdatomic.h>

typedef struct
{
	thrd_t handle;
	volatile atomic_bool done_bool;

	cnd_t* wait_cvar;
	volatile atomic_bool* quit_bool;

	thrd_start_t fn;
	void* arg;
} ThreadData;

typedef struct 
{
	int32_t thread_count;
	ThreadData* thread_array;
	cnd_t wait_cvar;
	volatile atomic_bool quit_bool;
} ThreadPool;


ThreadPool* smp_create_thread_pool(const int32_t thread_count);
void smp_destroy_thread_pool(ThreadPool* pool);

void smp_submit_work(ThreadPool* pool, const thrd_start_t fn, void** const arg_array);
void smp_wait_completion(ThreadPool* pool);



#endif

