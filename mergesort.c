#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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
            arr[k] = L[i];
            i++;
        }
        else
        {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    while (i < n1)
    {
        arr[k] = L[i];
        i++;
        k++;
    }

    while (j < n2)
    {
        arr[k] = R[j];
        j++;
        k++;
    }
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

void printArray(int arr[], int size)
{
    for (int i = 0; i < size; i++)
        printf("%d ", arr[i]);
    printf("\n");
}

int main(int argc, char *argv[])
{
    int n;

    if (argc != 2)
    {
        printf("Usage: %s <array_size>\n", argv[0]);
        return 1;
    }

    n = atoi(argv[1]);
    if (n <= 0)
    {
        printf("Invalid array size\n");
        return 1;
    }
    int *arr = malloc(n * sizeof(int));
    int *temp = malloc(n * sizeof(int));
    if (arr == NULL || temp == NULL)
    {
        printf("Memory allocation failed\n");
        return 1;
    }
    FILE *input_file = fopen("array1.txt", "r");
    if (input_file == NULL)
    {
        printf("Error opening file: input.txt\n");
        return 1;
    }

    for (int i = 0; i < n; i++)
    {
        if (fscanf(input_file, "%d, ", &arr[i]) != 1)
        {
            printf("Error reading from file\n");
            fclose(input_file);
            return 1;
        }
    }

    fclose(input_file);

    clock_t total_time = 0;
    int num_trials = 100;

    for (int t = 0; t < num_trials; t++)
    {
        for (int i = 0; i < n; i++)
        {
            temp[i] = arr[i]; // Copy original array to temp array
        }

        clock_t start = clock();
        mergeSort(temp, 0, n - 1);
        clock_t end = clock();

        double elapsed_time = ((double)(end - start) / CLOCKS_PER_SEC) * 1000.0;
        total_time += elapsed_time;

        // printf("Trial %d: Sorting time - %.2f milliseconds\n", t + 1, elapsed_time);
    }

    // Calculate average time
    double average_time = (double)total_time / num_trials;
    printf("\nAverage sorting time over %d trials: %.2f milliseconds\n", num_trials, average_time);
    free(arr);
    free(temp);
    return 0;
}
