#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_WORKERS 50
#define MAX_STRING_DIM 100

// Tag-uri utile
#define FIRST_CONTACT_TAG 10000
#define COORDINATORS_TAG 10001
#define TOPO_TAG 10002
#define VECT_DIM_TAG 10003
#define VECT_VALS_TAG 10004
#define PARTIAL_RESULT_TAG 100000


int min(const int a, const int b) {
	return (a < b) ? a : b;
}

/**
 * @brief Afiseaza topologia pe care o cunoaste un anumit proces
 * 
 * @param rank rank-ul procesului din care se face afisarea
 * @param workers matricea de workeri pentru fiecare coordonator(0/1/2)
 * @param numWorkers vector ce contine numarul de workeri ai fiecarui
 *                   coordonator(0/1/2)
 */
void printTopology(int rank, int workers[3][MAX_WORKERS], int numWorkers[3]) {
    char outputBuf[MAX_STRING_DIM];
    char tempBuf[MAX_STRING_DIM];
    sprintf(outputBuf, "%d -> ", rank);
    for (int i = 0; i < 3; i++) {
        sprintf(tempBuf, "%d:", i);
        strcat(outputBuf, tempBuf);
        for (int j = 0; j < numWorkers[i]; j++) {
            sprintf(tempBuf, "%d", workers[i][j]);
            strcat(outputBuf, tempBuf);
            if (j != numWorkers[i] - 1) {
                strcat(outputBuf, ",");
            } else {
                strcat(outputBuf, " ");
            }
        }
    }
    printf("%s\n", outputBuf);
}

/**
 * @brief Fa schimb de workeri cu un alt proces coordonator.
 *        Procesul mai intai va trimite workwrii lui, apoi ii va astepta
 *        pe cei ai celuilalt proces coordonator(de aici numele de Prioritary).
 *        Actualizeaza matricea workers si vectorul numWorkers.
 * 
 * @param myRank rank-ul procesului curent
 * @param rRank rank-ul celuilalt proces coordonator
 * @param workers matricea de workeri pentru fiecare coordonator(0/1/2)
 * @param numWorkers vector ce contine numarul de workeri ai fiecarui
 *                   coordonator(0/1/2)
 */
void sendMyWorkersPrioritary(int myRank, int rRank, int workers[3][MAX_WORKERS], int *numWorkers) {
    MPI_Status status;
    int recvBuf[MAX_WORKERS];
    
    // ->
    // Trimite numarul de workersi catre rRank
    MPI_Send(&numWorkers[myRank], 1, MPI_INT, rRank, COORDINATORS_TAG, MPI_COMM_WORLD);
    printf("M(%d,%d)\n", myRank, rRank);
    // Trimite workersii catre rRank
    MPI_Send(&workers[myRank], numWorkers[myRank], MPI_INT, rRank,
              COORDINATORS_TAG, MPI_COMM_WORLD);
    printf("M(%d,%d)\n", myRank, rRank);

    // <- 
    // Primeste numarul de workersi de la rRank
    MPI_Recv(&numWorkers[rRank], 1, MPI_INT, rRank, COORDINATORS_TAG, MPI_COMM_WORLD , &status);
    // Primeste workersii de la rRank
    MPI_Recv(&recvBuf, numWorkers[rRank], MPI_INT, rRank, COORDINATORS_TAG,
              MPI_COMM_WORLD , &status);
    
    // Copiaza workersii in matricea workers
    for (int i = 0; i < numWorkers[rRank]; i++) {
        workers[rRank][i] = recvBuf[i];
    }
}

/**
 * @brief Fa schimb de workeri cu un alt proces coordonator.
 *        Procesul mai intai va primii workweii celuilalt proces coordonator,
 *        apoi ii va trimite pe ai lui.
 *        Actualizeaza matricea workers si vectorul numWorkers.
 * 
 * @param myRank rank-ul procesului curent
 * @param rRank rank-ul celuilalt proces coordonator
 * @param workers matricea de workeri pentru fiecare coordonator(0/1/2)
 * @param numWorkers vector ce contine numarul de workeri ai fiecarui
 *                   coordonator(0/1/2) 
 */
void sendMyWorkers(int myRank, int rRank, int workers[3][MAX_WORKERS], int *numWorkers) {
    MPI_Status status;
    int recvBuf[MAX_WORKERS];
    
    // <- 
    // Primeste numarul de workersi de la rRank
    MPI_Recv(&numWorkers[rRank], 1, MPI_INT, rRank, COORDINATORS_TAG, MPI_COMM_WORLD , &status);
    // Primeste workersii de la rRank
    MPI_Recv(&recvBuf, numWorkers[rRank], MPI_INT, rRank, COORDINATORS_TAG,
              MPI_COMM_WORLD , &status);
    
    // Copiaza workersii in matricea workers
    for (int i = 0; i < numWorkers[rRank]; i++) {
        workers[rRank][i] = recvBuf[i];
    }
    
    // ->
    // Trimite numarul de workersi catre rRank
    MPI_Send(&numWorkers[myRank], 1, MPI_INT, rRank, COORDINATORS_TAG, MPI_COMM_WORLD);
    printf("M(%d,%d)\n", myRank, rRank);
    // Trimite workersii catre rRank
    MPI_Send(&workers[myRank], numWorkers[myRank], MPI_INT, rRank,
              COORDINATORS_TAG, MPI_COMM_WORLD);
    printf("M(%d,%d)\n", myRank, rRank);

}

/**
 * @brief Trimite topologia unui proces coordonator catre un worker.
 * 
 * @param myRank rank-ul procesului coordonator curent
 * @param workerRank rank-ul unui worker asociat acestui proces coordonator
 * @param workers matricea de workeri pentru fiecare coordonator(0/1/2)
 * @param numWorkers vector ce contine numarul de workeri ai fiecarui
 *                   coordonator(0/1/2)  
 */
void sendTopologyToWorker(int myRank, int workerRank, int workers[3][MAX_WORKERS],
                          int *numWorkers) {
    for (int i = 0; i < 3; i++) {
        // ->
        // Trimite numarul de workersi catre workerRank
        MPI_Send(&numWorkers[i], 1, MPI_INT, workerRank, TOPO_TAG, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", myRank, workerRank);
        // Trimite workersii catre rRank
        MPI_Send(&workers[i], numWorkers[i], MPI_INT, workerRank, TOPO_TAG, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", myRank, workerRank);
    }
}

/**
 * @brief Primeste topologia de la un proces coordonator in calitate de worker.
 * 
 * @param myRank rank-ul workerului curent
 * @param coordRank rank-ul coordonatorului
 * @param workers matricea de workeri pentru fiecare coordonator(0/1/2)
 * @param numWorkers vector ce contine numarul de workeri ai fiecarui
 *                   coordonator(0/1/2)  
 */
void recvTopologyFromCoordinator(int myRank, int coordRank, int workers[3][MAX_WORKERS],
                                 int *numWorkers) {
    MPI_Status status;
    int recvBuf[MAX_WORKERS];

    for (int i = 0; i < 3; i++) {
        // <- 
        // Primeste numarul de workersi de la coordRank
        MPI_Recv(&numWorkers[i], 1, MPI_INT, coordRank, TOPO_TAG, MPI_COMM_WORLD , &status);
        // Primeste workersii de la rRank
        MPI_Recv(&recvBuf, numWorkers[i], MPI_INT, coordRank, TOPO_TAG, MPI_COMM_WORLD , &status);
        
        // Copiaza workersii in matricea workers
        for (int j = 0; j < numWorkers[i]; j++) {
            workers[i][j] = recvBuf[j];
        }
    }
}

/**
 * @brief Pe baza informatiilor din matricea workers, creaza un vector
 *        parents in care pe pozitia i se va gasi rank-ul procesului
 *        coordonator workerului i.
 * 
 * @param workers matricea de workeri pentru fiecare coordonator(0/1/2)
 * @param numWorkers vector ce contine numarul de workeri ai fiecarui
 *                   coordonator(0/1/2)  
 * @param parents vectorul rezultat
 */
void computeParents(int workers[3][MAX_WORKERS], int *numWorkers, int *parents) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < numWorkers[i]; j++) {
            parents[workers[i][j]] = i;
        }
    }
}

int main (int argc, char *argv[])
{
    int numtasks, rank;
    // N - numarul de elemente din vectorul V
    int n, *v;
    // Rank-ul procesului coordonator unui worker
    int coordinator = -1;
    // Fiserul de intrare pentru coordonatori
    FILE *inputFile;
    // Numarul de workeri ai fiecarui coordonator
    int numWorkers[3];
    // Rank-urile workerilor fiecarui coordonator
    int workers[3][MAX_WORKERS];
    // Coordonatorii fiecarui worker
    int parents[MAX_WORKERS];
    // Index-ul de inceput si de sfarsit al fiecarui subvector
    int vectorRange[MAX_WORKERS][2];
    // Dimensiunea fiecaru subvector ce va fi trimis catre workeri
    // (folosit doar de procesul 0)
    int vectorDim[MAX_WORKERS];
    // Dimensiunea fiecaru subvector ce va fi trimis catre workeri
    // (folosit doar de procesele 1 si 2)
    int subVdims[MAX_WORKERS];

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    // Verificare numar de parametri
    if (argc < 3) {
        printf("Error: not enough parameters(Usage: ./tema3 <vect_dim> <comm_err>\n");
        MPI_Finalize();
        return 0;
    }

    // Initializare vector V;
    if (rank == 0) {
        n = atoi(argv[1]);
        v = calloc(n, sizeof(int));
        for (int i = 0; i < n; i++) {
            v[i] = i;
        }
    }
    // Stabilirea topologiei(Task 1)
    if (rank == 0 || rank == 1 || rank == 2) {
        // Formeaza numele fisierului de intrare pentru coordonatori
        char fileName[13] = "clusterX.txt";
        fileName[7] = '0' + rank;
        // Deschide fisierul cu workerii clusterului curent
        inputFile = fopen(fileName, "rt");
        // Citeste numarul de workeri ai clusterului curent
        fscanf(inputFile, "%d", &numWorkers[rank]);
        // Citeste rank-urile workeri-lor din clusterul curent
        for (int i = 0; i < numWorkers[rank]; i++) {
            fscanf(inputFile, "%d", &workers[rank][i]);
        }
        // Inchide fisierul
        fclose(inputFile);


        // Trimite rank-ul coordonatorului catre workerii lui
        for (int i = 0; i < numWorkers[rank]; i++) {
            MPI_Send(&rank, 1, MPI_INT, workers[rank][i], FIRST_CONTACT_TAG, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, workers[rank][i]);
        }

        // Trimite workerii coordonatorului curent catre ceilalti coordonatori
        // In ordine, se realizeaza schimbul intre: 0 <-> 1, 1 <-> 2, 0 <-> 2
        int recvBuf[MAX_WORKERS];
        MPI_Status status;
        if (rank == 0) {
            sendMyWorkersPrioritary(rank, 1, workers, numWorkers);
            sendMyWorkersPrioritary(rank, 2, workers, numWorkers);
        
        } else if (rank == 1) {
            sendMyWorkers(rank, 0, workers, numWorkers);
            sendMyWorkersPrioritary(rank, 2, workers, numWorkers);

        } else if (rank == 2) {
            sendMyWorkers(rank, 1, workers, numWorkers);
            sendMyWorkers(rank, 0, workers, numWorkers);
        }

        // Trimite topologia finala catre workerii acestui cluster
        for (int i = 0; i < numWorkers[rank]; i++) {
            sendTopologyToWorker(rank, workers[rank][i], workers, numWorkers);
        }

        // Afiseaza topologia
        printTopology(rank, workers, numWorkers);
    } else {
        MPI_Status status;
        MPI_Recv(&coordinator, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        // Primeste si afiseaza topologia primita de la coordonator
        recvTopologyFromCoordinator(rank, coordinator, workers, numWorkers);
        printTopology(rank, workers, numWorkers);
    }


    // Asteapta ca toate procesele sa termine stabilirea topologiei
    MPI_Barrier(MPI_COMM_WORLD);


    // Distribuire subvectori catre workeri si efectuare operatii(Task 2, Part 1)
    int totalWorkers = numWorkers[0] + numWorkers[1] + numWorkers[2];
    int *subV;
    int subVdim;
    if (rank == 0) {
        computeParents(workers, numWorkers, parents);

        for (int i = 0; i < totalWorkers; i++) {
            int dest = i + 3;
            // Determinare indecsi in vector si dimensiune pentru fiecare worker
            vectorRange[i][0] = i * (double)n / totalWorkers; 
            vectorRange[i][1] = min (n, (i + 1) * (double)n / totalWorkers);
            vectorDim[i] = vectorRange[i][1] - vectorRange[i][0];
        }

        // Trimite numar de elemente in vector
        for (int i = 0; i < numWorkers[0]; i++) {
            MPI_Send(&n, 1, MPI_INT, workers[0][i], VECT_DIM_TAG, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, workers[0][i]);
        }

        MPI_Send(&n, 1, MPI_INT, 1, VECT_DIM_TAG, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, 1);

        MPI_Send(&n, 1, MPI_INT, 2, VECT_DIM_TAG, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, 2);


        // Trimite subvectorii catre workeri
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < numWorkers[i]; j++) {
                int dest = workers[i][j];
                // Daca worker-ul curent nu este legat la coordonatorul 0,
                // trimite catre coordonatorul 1 sau 2
                // Altfel, trimite direct la worker
                if (i != 0) {
                    dest = parents[workers[i][j]];
                }
                int dim = vectorDim[workers[i][j] - 3];
                int startPos = vectorRange[workers[i][j] - 3][0];
                int startElem = v[startPos];

                MPI_Send(&v[startPos], dim, MPI_INT, dest, VECT_VALS_TAG, MPI_COMM_WORLD);
                printf("M(%d,%d)\n", rank, dest);
            }
        }


    } else if (rank == 1 || rank == 2) {
        int numElem;
        MPI_Status status;

        MPI_Recv(&numElem, 1, MPI_INT, 0, VECT_DIM_TAG, MPI_COMM_WORLD, &status);
        // Paseaza mai departe numarul de elemente din vector
        for (int i = 0; i < numWorkers[rank]; i++) {
            MPI_Send(&numElem, 1, MPI_INT, workers[rank][i], VECT_DIM_TAG, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, workers[rank][i]);
        }

        // Paseaza subvectorii catre workeri
        for (int i = 0; i < numWorkers[rank]; i++) {
            // Calculeaza dimensiunea subvectorului
            int start = (workers[rank][i] - 3) * (double)numElem / totalWorkers; 
            int end = min (numElem, ((workers[rank][i] - 3) + 1) * (double)numElem / totalWorkers);
            int dim = end - start;
            subVdims[i] = dim;

            // Primeste subvectorul
            int *recvBuf;
            recvBuf = calloc(dim, sizeof(int));
            MPI_Recv(recvBuf, dim, MPI_INT, 0, VECT_VALS_TAG, MPI_COMM_WORLD, &status);

            // Trimite subvectorul mai departe
            MPI_Send(recvBuf, dim, MPI_INT, workers[rank][i], VECT_VALS_TAG, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, workers[rank][i]);

            free(recvBuf);
        }
    } else {
        // Primire numar de elemente
        int numElem;
        MPI_Status status;

        // Primeste numarul elementelor din vector
        MPI_Recv(&numElem, 1, MPI_INT, coordinator, VECT_DIM_TAG, MPI_COMM_WORLD, &status);
        int start = (rank - 3) * (double)numElem / totalWorkers; 
        int end = min (numElem, ((rank - 3) + 1) * (double)numElem / totalWorkers);
        subVdim = end - start;

        // Primeste sub-vectorul pentru acest worker
        subV = calloc(subVdim, sizeof(int));
        MPI_Recv(subV, subVdim, MPI_INT, coordinator, VECT_VALS_TAG, MPI_COMM_WORLD, &status);

        // Dubleaza elementele din vector
        for (int i = 0; i < subVdim; i++) {
            subV[i] *= 2;
        }

    }

    // Asteapta ca toti workerii sa primeasca subvectorii si sa efectueze operatiile
    MPI_Barrier(MPI_COMM_WORLD);

    // Adunare subvectori de la workeri (Task 2, Part 2)
    if (rank == 0) {
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < numWorkers[i]; j++) {
                int *recvBuf;
                MPI_Status status;
                int src = workers[i][j];
                int dim = vectorDim[workers[i][j] - 3];
                int start = vectorRange[workers[i][j] - 3][0];
                if (i != 0) {
                    src = parents[workers[i][j]];
                }

                // Primeste rezultat partial
                recvBuf = calloc(dim, sizeof(int));
                MPI_Recv(recvBuf, dim, MPI_INT, src, PARTIAL_RESULT_TAG + workers[i][j],
                         MPI_COMM_WORLD, &status);

                // Scrie rezultatul partial in vector
                for (int i = 0; i < dim; i++) {
                    v[start + i] = recvBuf[i];
                }

                free(recvBuf);
            }
        }    
    } else if (rank == 1 || rank == 2) {
        for (int i = 0; i < numWorkers[rank]; i++) {
            // Primeste sub-vectorul pentru acest worker
            int *recvBuf;
            MPI_Status status;
            recvBuf = calloc(subVdims[i], sizeof(int));
            MPI_Recv(recvBuf, subVdims[i], MPI_INT, workers[rank][i],
                     PARTIAL_RESULT_TAG + workers[rank][i], MPI_COMM_WORLD, &status);

            // Trimite rezultatul partial catre procesul 0
            MPI_Send(recvBuf, subVdims[i], MPI_INT, 0, PARTIAL_RESULT_TAG + workers[rank][i],
                     MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, 0);

            free(recvBuf);
        }

    } else {
        // Trimite rezultat partial catre coordonator(din tag-ul mesajelor se va extrage 
        // rank-ul workerului care a livrat acest rezultat partial)
        MPI_Send(subV, subVdim, MPI_INT, coordinator, PARTIAL_RESULT_TAG + rank, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, coordinator);

    }

    // Asteapta ca toate procesele sa trimita subvectorii
    MPI_Barrier(MPI_COMM_WORLD);

    // Afiseaza rezultat
    if (rank == 0) {
        printf("Rezultat: ");
        for (int i = 0; i < n; i++) {
            printf("%d ", v[i]);
        }
        printf("\n");
        free(v);
    }

    MPI_Finalize();

}
