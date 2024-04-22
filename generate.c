#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main()
{
    int n = 100000000;
    const char *filename = "array1.txt";
    FILE *file = fopen(filename, "w");

    if (file == NULL)
    {
        printf("Error opening file.\n");
        return 1;
    }

    srand(time(NULL));

    for (int i = 0; i < n; i++)
    {
        int num = rand() % (n + 1);
        fprintf(file, "%d", num);

        if (i < n - 1)
        {
            fprintf(file, ", ");
        }
    }

    fclose(file);
    printf("File '%s' created successfully.\n", filename);

    return 0;
}
