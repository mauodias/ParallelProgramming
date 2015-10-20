#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mpi.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE 80      // trabalho final com o valores 10.000, 100.000, 1.000.000
#endif

#define ARRAY_TAG 1
#define SORTED_TAG 2

#ifndef DELTA
#define DELTA 10
#endif

int array[ARRAY_SIZE];
int aux_array[ARRAY_SIZE];
double t1,t2;
void init_vector(int rank)
{
    int i;
    for (i = 0; i < ARRAY_SIZE; ++i)
    {
        if (!rank)
            array[i] = ARRAY_SIZE - i;
        else
            array[i] = 0;
    }
}

void bs(int n, int * vetor)
{
    int c = 0, d, troca, trocou = 1;

    while (c < (n - 1) & trocou )
    {
        trocou = 0;
        for (d = 0 ; d < n - c - 1; d++)
            if (vetor[d] > vetor[d + 1])
            {
                troca      = vetor[d];
                vetor[d]   = vetor[d + 1];
                vetor[d + 1] = troca;
                trocou = 1;
            }
        c++;
    }
}

void interleaving(int * vetor, int tam)
{
    int vetor_auxiliar[ARRAY_SIZE];
    int i1, i, i2, i_aux;

    i1 = 0;
    i2 = tam / 2;

    for (i_aux = 0; i_aux < tam; i_aux++) {
        if (((vetor[i1] <= vetor[i2]) && (i1 < (tam / 2)))
                || (i2 == tam))
            vetor_auxiliar[i_aux] = vetor[i1++];
        else
            vetor_auxiliar[i_aux] = vetor[i2++];
    }

    for (i = 0; i<tam; i++)
        array[i] = vetor_auxiliar[i];
}

int main(int argc, char** argv)
{
    int my_rank;
    int proc_n;
    int current_length = ARRAY_SIZE;
    int array_pos = 0;
    int parent;

    MPI_Status status;
    MPI_Init (&argc, & argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &proc_n);

    init_vector(my_rank);

    // Define left and right children
    int left_child = my_rank * 2 + 1;
    int right_child = my_rank * 2 + 2;

    printf("%d", my_rank);

    if (my_rank != 0)
    {
        // Get parent rank
        if (my_rank % 2)
            parent = (my_rank - 1) / 2;
        else
            parent = (my_rank - 2) / 2;

        MPI_Recv (&array, ARRAY_SIZE, MPI_INT, parent, ARRAY_TAG, MPI_COMM_WORLD, &status);
        MPI_Get_count(&status, MPI_INT, &current_length);
    }
    else
    {
        int i;
        for (i = 0; i < ARRAY_SIZE; ++i)
            printf("%d ", array[i]);
        printf("\n");
        t1 = MPI_Wtime();
    }
    if (current_length > DELTA)
    {
        // Send
        MPI_Send (&array, current_length / 2, MPI_INT, left_child, ARRAY_TAG, MPI_COMM_WORLD);
        MPI_Send (&array[current_length / 2], current_length / 2, MPI_INT, right_child, ARRAY_TAG, MPI_COMM_WORLD);

        // Recv
        MPI_Recv(&array, current_length / 2, MPI_INT, left_child, SORTED_TAG, MPI_COMM_WORLD, &status);
        MPI_Recv(&array[current_length / 2], current_length / 2, MPI_INT, right_child, SORTED_TAG, MPI_COMM_WORLD, &status);

        // interleave
        interleaving(array, current_length);
        //memcpy(array, aux_array, ARRAY_SIZE);
    }
    else
    {
        bs(current_length, array);
    }

    // send to papa
    if(my_rank != 0)
    {
        MPI_Send(&array, current_length, MPI_INT, parent, SORTED_TAG, MPI_COMM_WORLD);
    }
    else
    {
        interleaving(array, current_length);
        t2 = MPI_Wtime();
        printf("\nTempo de execucao: %f\n\n", t2-t1);
        int i;
        for (i = 0; i < ARRAY_SIZE; ++i)
            printf("%d ", array[i]);
        printf("\n");
    }

    MPI_Finalize();

    return 0;
}
