#include "smp.h"
#include <stdlib.h>

int
thread_loop(void* arg)
{
	ThreadData* tdata = arg;
	mtx_t mutex;
	rassert(mtx_init(&mutex, mtx_plain) == thrd_success, "failed to create mutex");
	mtx_lock(&mutex);
	
	while(1)
	{	
		cnd_wait(tdata->wait_cvar, &mutex);
		if(atomic_load(tdata->quit_bool))
			break;

		(tdata->fn)(tdata->arg);

		atomic_store(&tdata->done_bool, 1);
	}

	mtx_unlock(&mutex);
	mtx_destroy(&mutex);

	return 0;
}



ThreadPool* 
smp_create_thread_pool(const int32_t thread_count)
{
	ThreadPool* pool = calloc(1, sizeof(*pool));
	pool->thread_count = thread_count;
	pool->thread_array = calloc(thread_count, sizeof(*pool->thread_array));
	rassert(cnd_init(&pool->wait_cvar) == thrd_success,"failed to init cvar!");
	atomic_init(&pool->quit_bool, 0);

	for(ThreadData* tdata = pool->thread_array; tdata != pool->thread_array + thread_count; tdata++)
	{
		atomic_init(&tdata->done_bool, 1);
		tdata->wait_cvar = &pool->wait_cvar;
		tdata->quit_bool = &pool->quit_bool;
		rassert(thrd_create(&tdata->handle, &thread_loop, tdata) == thrd_success, "failed to create thread!");
	}

	return pool;
}

void 
smp_destroy_thread_pool(ThreadPool* pool)
{
	smp_wait_completion(pool);
	atomic_store(&pool->quit_bool, 1);
	rassert(cnd_broadcast(&pool->wait_cvar) == thrd_success, "failed to broadcast thread wait!");

	for(int32_t i = 0; i < pool->thread_count; i++)
	{
		ThreadData* tdata = pool->thread_array + i; 
		rassert(thrd_join(tdata->handle, NULL) == thrd_success, "failed to join thread");
	}
	free(pool->thread_array);

	cnd_destroy(&pool->wait_cvar);
	free(pool);
}

void 
smp_submit_work(ThreadPool* pool, const thrd_start_t fn, void** const arg_array)
{
	for(int32_t i = 0; i < pool->thread_count; i++)
	{
		ThreadData* tdata = pool->thread_array + i; 
		rassert(atomic_load(&tdata->done_bool), "submitted work to busy thead pool!");
		atomic_store(&tdata->done_bool, 0);

		tdata->fn = fn;
		tdata->arg = arg_array[i];
	}

	rassert(cnd_broadcast(&pool->wait_cvar) == thrd_success, "failed to broadcast thread wait!");

}

void 
smp_wait_completion(ThreadPool* pool)
{
	int waiting = 1;
	while(waiting)
	{
		thrd_yield();
		waiting = 0;
		for(int32_t i = 0; i < pool->thread_count; i++)
		{
			ThreadData* tdata = pool->thread_array + i; 
			if(!atomic_load(&tdata->done_bool)) //busy
			{
				waiting = 1;
				break;
			}
		}
	}
}
