#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE 40      // trabalho final com o valores 10.000, 100.000, 1.000.000
#endif

#define LENGTH_TAG 1
#define ARRAY_TAG 2
#define SORTED_TAG 3

#ifndef DELTA
#define DELTA 10
#endif

int array[ARRAY_SIZE];
int aux_array[ARRAY_SIZE];

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

int *interleaving(int vetor[], int tam)
{
    int *vetor_auxiliar;
    int i1, i2, i_aux;

    vetor_auxiliar = (int *)malloc(sizeof(int) * tam);

    i1 = 0;
    i2 = tam / 2;

    for (i_aux = 0; i_aux < tam; i_aux++) {
        if (((vetor[i1] <= vetor[i2]) && (i1 < (tam / 2)))
                || (i2 == tam))
            vetor_auxiliar[i_aux] = vetor[i1++];
        else
            vetor_auxiliar[i_aux] = vetor[i2++];
    }

    return vetor_auxiliar;
}

void init_vector()
{
	int i;
    for (i = 0; i < ARRAY_SIZE; ++i)
    {
        array[i] = ARRAY_SIZE - i;
    }
}

int main(int argc, char** argv)
{
    int my_rank;  /* Identificador do processo */
    int proc_n;   /* Número de processos */
    int current_length = ARRAY_SIZE;
    int array_pos = 0;
    int parent;

    init_vector(); // Inicializa vetor principal em ordem decrescente

    MPI_Status status; /* Status de retorno */
    MPI_Init (&argc , & argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &proc_n);

    if (my_rank != 0) // Se o nodo atual não for raiz
    {
        if (my_rank % 2 == 1) 
            parent = (my_rank - 1) / 2;
        else
            parent = (my_rank - 2) / 2;
#ifdef DEBUG
        printf("Current Length before: %d on %d\n",my_rank, current_length);
#endif
        MPI_Recv(&array_pos, 1, MPI_INT, parent, ARRAY_TAG, MPI_COMM_WORLD, &status); // Recebe a posição inicial do array
        MPI_Recv(&current_length, 1, MPI_INT, parent, LENGTH_TAG, MPI_COMM_WORLD, &status); // Recebe o tamanho do array
#ifdef DEBUG
        printf("Current Length after: %d\n",current_length);
#endif
    }
    if (current_length > DELTA) // Se o tamanho recebido for maior que o delta
    {
        current_length /= 2;
        int left_child = my_rank * 2 + 1;
        int array_left = array_pos;
        int array_right = array_pos+current_length;
        int right_child = my_rank * 2 + 2;
#ifdef DEBUG
        printf("%d %d %d %d %d %d %d\n", my_rank, parent, left_child, right_child, current_length, array_left, array_right);
#endif
        MPI_Send(&array_left, 1, MPI_INT, left_child, ARRAY_TAG, MPI_COMM_WORLD); // Envia a posição inicial para o primeiro filho
        MPI_Send(&current_length, 1, MPI_INT, left_child, LENGTH_TAG, MPI_COMM_WORLD); // Envia a metade do tamanho para o primeiro filho
        MPI_Send(&array_right, 1, MPI_INT, right_child, ARRAY_TAG, MPI_COMM_WORLD); // Envia a posição inicial mais o tamanho para o segundo filho (metade do array)
        MPI_Send(&current_length, 1, MPI_INT, right_child, LENGTH_TAG, MPI_COMM_WORLD); // Envia a metade do tamanho para o primeiro filho
        MPI_Recv(&aux_array[array_left], current_length, MPI_INT, left_child, SORTED_TAG, MPI_COMM_WORLD, &status); // Recebe o array ordenado do primeiro filho e armazena no começo do auxiliar
        MPI_Recv(&aux_array[array_right], current_length, MPI_INT, right_child, SORTED_TAG, MPI_COMM_WORLD, &status); // Recebe o array ordenado do segundo filho e armazena no final do auxiliar
        int *other_aux;
        other_aux = interleaving(&aux_array[array_pos], current_length*2);
#ifdef DEBUG
        /*
        printf("array %d:\n", my_rank);
        int i;
        for (i = 0; i < ARRAY_SIZE; ++i) {
            printf("[%d: %d ] ", i,array[i]);
        }
        printf("\n");
        printf("other_aux %d:\n", my_rank);
        for (i = 0; i < current_length * 2; ++i) {
            printf("[%d: %d ] ", i,other_aux[i]);
        }
        printf("\n");
        */
#endif
    }

    bs(current_length*2, &array[array_pos]);
    if(my_rank != 0)
    {
    	MPI_Send(&array[array_pos], 1, MPI_INT, parent, SORTED_TAG, MPI_COMM_WORLD); // Envia a posição inicial para o primeiro filho
    }

    MPI_Finalize();
    return 0;
}