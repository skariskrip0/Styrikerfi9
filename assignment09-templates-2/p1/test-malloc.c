#include "testlib.h"
#include "malloc.h"
#include <pthread.h>
#include <string.h>

#define NUM_THREADS 4
#define NUM_ITERATIONS 1000
#define MAX_ALLOC_SIZE 1024

/* Structure to pass data to threads */
typedef struct {
	int thread_id;
	void **allocations;
	int *allocation_sizes;
} ThreadData;

/* Thread function that performs allocations and frees */
void* thread_function(void* arg) {
	ThreadData* data = (ThreadData*)arg;
	void **allocations = data->allocations;
	int *allocation_sizes = data->allocation_sizes;
	
	for (int i = 0; i < NUM_ITERATIONS; i++) {
		// Allocate memory
		allocation_sizes[i] = (rand() % MAX_ALLOC_SIZE) + 1;
		allocations[i] = my_malloc(allocation_sizes[i]);
		
		// Verify allocation
		if (allocations[i] != NULL) {
			// Write some data to verify memory is accessible
			memset(allocations[i], 0xFF, allocation_sizes[i]);
		}
		
		// Randomly free some allocations
		if (rand() % 2 == 0 && i > 0) {
			int free_index = rand() % i;
			if (allocations[free_index] != NULL) {
				my_free(allocations[free_index]);
				allocations[free_index] = NULL;
			}
		}
	}
	
	return NULL;
}

/* Return true (1): all ok, false (0): something is wrong */
int allocator_test()
{
	initAllocator();
	
	// Enable debug mode
	enable_debug(1);
	
	// Create thread data
	pthread_t threads[NUM_THREADS];
	ThreadData thread_data[NUM_THREADS];
	void *allocations[NUM_THREADS][NUM_ITERATIONS];
	int allocation_sizes[NUM_THREADS][NUM_ITERATIONS];
	
	// Initialize thread data
	for (int i = 0; i < NUM_THREADS; i++) {
		thread_data[i].thread_id = i;
		thread_data[i].allocations = allocations[i];
		thread_data[i].allocation_sizes = allocation_sizes[i];
	}
	
	// Create and start threads
	for (int i = 0; i < NUM_THREADS; i++) {
		pthread_create(&threads[i], NULL, thread_function, &thread_data[i]);
	}
	
	// Wait for all threads to complete
	for (int i = 0; i < NUM_THREADS; i++) {
		pthread_join(threads[i], NULL);
	}
	
	// Free all remaining allocations
	for (int i = 0; i < NUM_THREADS; i++) {
		for (int j = 0; j < NUM_ITERATIONS; j++) {
			if (allocations[i][j] != NULL) {
				my_free(allocations[i][j]);
			}
		}
	}
	
	// Print final state
	printf("Final heap state:\n");
	dumpAllocator();
	
	return 1;
}
