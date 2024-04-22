#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cuda_runtime.h>

#define THREADS_PER_BLOCK 256

__device__ void merge(int *arr, int *temp, int l, int m, int r)
{
    int i = l, j = m + 1, k = l;

    while (i <= m && j <= r)
    {
        if (arr[i] <= arr[j])
            temp[k++] = arr[i++];
        else
            temp[k++] = arr[j++];
    }

    while (i <= m)
        temp[k++] = arr[i++];
    while (j <= r)
        temp[k++] = arr[j++];

    for (i = l; i <= r; i++)
        arr[i] = temp[i];
}
__global__ void mergeSort(int *arr, int *temp, int n)
{
    extern __shared__ int shared_mem[];

    int tid = threadIdx.x;
    int block_size = blockDim.x;
    int block_start = blockIdx.x * block_size;

    if (tid < block_size && block_start + tid < n)
    {
        shared_mem[tid] = arr[block_start + tid];
    }
    __syncthreads();

    for (int curr_size = 1; curr_size < block_size; curr_size *= 2)
    {
        for (int left_start = 0; left_start < block_size; left_start += 2 * curr_size)
        {
            int mid = left_start + curr_size - 1;
            int right_end = min(left_start + 2 * curr_size - 1, block_size - 1);

            if (mid < right_end)
            {
                merge(shared_mem, temp, left_start, mid, right_end);
            }
        }
        __syncthreads();
    }

    if (tid < block_size && block_start + tid < n)
    {
        arr[block_start + tid] = shared_mem[tid];
    }
}
void merge2(int *arr, int *temp, int l, int m, int r)
{
    int i = l, j = m + 1, k = l;

    while (i <= m && j <= r)
    {
        if (arr[i] <= arr[j])
            temp[k++] = arr[i++];
        else
            temp[k++] = arr[j++];
    }

    while (i <= m)
        temp[k++] = arr[i++];
    while (j <= r)
        temp[k++] = arr[j++];

    for (i = l; i <= r; i++)
        arr[i] = temp[i];
}
void mergeSortedBlocks(int *arr, int n, int block_size)
{
    int *temp = (int *)malloc(n * sizeof(int));

    for (int size = block_size; size < n; size *= 2)
    {
        for (int left_start = 0; left_start < n; left_start += size * 2)
        {
            int mid = left_start + size - 1;
            int right_end = min(left_start + size * 2 - 1, n - 1);

            if (mid < right_end)
            {
                merge2(arr, temp, left_start, mid, right_end);
            }
        }
    }

    free(temp);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <array_size>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    if (n <= 0)
    {
        printf("Invalid array size\n");
        return 1;
    }

    int *arr, *d_arr, *temp;
    arr = (int *)malloc(n * sizeof(int));
    temp = (int *)malloc(n * sizeof(int));

    if (arr == NULL || temp == NULL)
    {
        printf("Memory allocation failed\n");
        return 1;
    }

    FILE *input_file = fopen("array1.txt", "r");
    if (input_file == NULL)
    {
        printf("Error opening file: array1.txt\n");
        free(arr);
        free(temp);
        return 1;
    }

    for (int i = 0; i < n; i++)
    {
        if (fscanf(input_file, "%d, ", &arr[i]) != 1)
        {
            printf("Error reading from file\n");
            fclose(input_file);
            free(arr);
            free(temp);
            return 1;
        }
    }
    cudaMalloc(&d_arr, n * sizeof(int));
    cudaMalloc(&temp, n * sizeof(int));

    int num_threads = THREADS_PER_BLOCK;
    int num_blocks = (n + num_threads - 1) / num_threads;

    clock_t total_time = 0;
    int num_trials = 5;

    for (int t = 0; t < num_trials; t++)
    {
        cudaMemcpy(d_arr, arr, n * sizeof(int), cudaMemcpyHostToDevice);

        clock_t start = clock();

        mergeSort<<<num_blocks, num_threads, num_threads * sizeof(int)>>>(d_arr, temp, n);

        cudaDeviceSynchronize();

        clock_t end = clock();
        total_time += (end - start);
        double elapsed_time = ((double)(end - start) / CLOCKS_PER_SEC) * 1000.0;

        // printf("Trial %d: Sorting time - %.2f milliseconds\n", t + 1, elapsed_time);
    }
    cudaMemcpy(arr, d_arr, n * sizeof(int), cudaMemcpyDeviceToHost);
    mergeSortedBlocks(arr, n, THREADS_PER_BLOCK);
    double average_time_ms = ((double)total_time / num_trials) / CLOCKS_PER_SEC * 1000.0;
    printf("Average sorting time over %d trials: %.2f milliseconds\n", num_trials, average_time_ms);

    cudaFree(d_arr);
    cudaFree(temp);
    free(arr);

    return 0;
}
