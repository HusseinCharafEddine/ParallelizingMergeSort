#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

void merge(int arr[], int l, int m, int r)
{
    int i, j, k;
    int n1 = m - l + 1;
    int n2 = r - m;

    int *L = malloc(n1 * sizeof(int));
    int *R = malloc(n2 * sizeof(int));

    for (i = 0; i < n1; i++)
        L[i] = arr[l + i];
    for (j = 0; j < n2; j++)
        R[j] = arr[m + 1 + j];

    i = 0;
    j = 0;
    k = l;
    while (i < n1 && j < n2)
    {
        if (L[i] <= R[j])
        {
            arr[k++] = L[i++];
        }
        else
        {
            arr[k++] = R[j++];
        }
    }

    while (i < n1)
    {
        arr[k++] = L[i++];
    }

    while (j < n2)
    {
        arr[k++] = R[j++];
    }

    free(L);
    free(R);
}

void mergeSort(int arr[], int l, int r)
{
    if (l < r)
    {
        int m = l + (r - l) / 2;
        mergeSort(arr, l, m);
        mergeSort(arr, m + 1, r);
        merge(arr, l, m, r);
    }
}

void mergeSortedSubarrays(int *arr, int num_elements, int num_procs)
{
    int subarray_size = num_elements / num_procs;
    for (int i = 0; i < num_procs; i++)
    {
        merge(arr, i * subarray_size, (i + 1) * subarray_size - 1, num_elements - 1);
        printf("%d \n", i);
    }
    merge(arr, 0, num_elements / 2, num_elements);
}

int main(int argc, char *argv[])
{
    int rank, size;
    int n;
    double startTime, endTime;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 2)
    {
        if (rank == 0)
        {
            printf("Usage: %s <array_size>\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }

    n = atoi(argv[1]);
    if (n <= 0)
    {
        if (rank == 0)
        {
            printf("Invalid array size\n");
        }
        MPI_Finalize();
        return 1;
    }

    int *arr = NULL;
    int local_size = n / size;
    int *local_arr = (int *)malloc(local_size * sizeof(int));

    if (rank == 0)
    {
        arr = (int *)malloc(n * sizeof(int));
        FILE *input_file = fopen("array1.txt", "r");
        if (input_file == NULL)
        {
            printf("Error opening file: array1.txt\n");
            MPI_Finalize();
            return 1;
        }
        for (int i = 0; i < n; i++)
        {
            if (fscanf(input_file, "%d,", &arr[i]) != 1)
            {
                printf("Error reading from file\n");
                fclose(input_file);
                MPI_Finalize();
                return 1;
            }
        }
        fclose(input_file);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    int num_trials = 5;
    double total_time = 0.0;

    for (int trial = 0; trial < num_trials; trial++)
    {
        MPI_Barrier(MPI_COMM_WORLD);

        startTime = MPI_Wtime();

        MPI_Scatter(arr, local_size, MPI_INT, local_arr, local_size, MPI_INT, 0, MPI_COMM_WORLD);

        mergeSort(local_arr, 0, local_size - 1);

        MPI_Gather(local_arr, local_size, MPI_INT, arr, local_size, MPI_INT, 0, MPI_COMM_WORLD);

        endTime = MPI_Wtime();

        if (rank == 0)
        {
            mergeSortedSubarrays(arr, n, size);
            double trial_time = (endTime - startTime) * 1000.0;
            total_time += trial_time;
        }
    }

    free(local_arr);

    if (rank == 0)
    {
        printf("Average sorting time over %d trials: %.2f milliseconds\n", num_trials, total_time / num_trials);

        FILE *output_file = fopen("sorted.txt", "w");
        if (output_file == NULL)
        {
            printf("Error opening file: sorted.txt\n");
            free(arr);
            MPI_Finalize();
            return 1;
        }

        for (int i = 0; i < n; i++)
        {
            fprintf(output_file, "%d, ", arr[i]);
        }

        fclose(output_file);
        free(arr);
    }

    MPI_Finalize();
    return 0;
}
