#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define min(a,b) (((a)<(b))?(a):(b))

void copyVector(int *v, int *p, int size) {
    int i;
    for(i = 0;i < size; i++) {
        p[i] = v[i];
    }
}

typedef struct Process {
    int coord;
    int count0;
    int count1;
    int count2;
    int *topology[3];
    int rankP;
    int start;
    int end;
}Process;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        perror("./tema3 N ERROR\n");
    }

    int rank, procs;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int nrProcese0, nrProcese1, nrProcese2;
    int *procese0, *procese1, *procese2;
    int coord;
    int val = 0;
    struct Process *procese;
    procese = malloc(procs * sizeof(struct Process));
    if (procese == NULL) {
        fprintf(stderr, "Unable to allocate the buffer.\n");
        return 0;
    }

    procese[rank].rankP = rank;
    procese[rank].count0 = 0;
    procese[rank].count1 = 0;
    procese[rank].count2 = 0;
    //citire date cluster0
    if(rank == 0) {
        procese[rank].coord = -1;
        procese[rank].rankP = rank;
        int *work1, *work2;
        //citire fisier
        FILE * cluster0File = fopen("cluster0.txt", "rt");
        fscanf(cluster0File, "%d", &nrProcese0);
        procese0 = malloc(nrProcese0 * sizeof(int));
        if (procese0 == NULL) {
        fprintf(stderr, "Unable to allocate the buffer.\n");
        return 0;
        }
        
        for(int i= 0; i < nrProcese0; i++) {
            fscanf(cluster0File, "%d", &procese0[i]);
        }
        procese[rank].count0 = nrProcese0;
        //trimiter mesaj coordonator
        for(int i = 0; i < nrProcese0; i++) {
            MPI_Send(&rank, 1, MPI_INT, procese0[i], MPI_TAG_UB, MPI_COMM_WORLD);
             printf("M(%d,%d)\n", rank, procese0[i]);
        }
        //trimiter nr procese cluster
        for(int i = 0; i < nrProcese0; i++) {
            MPI_Send(&nrProcese0, 1, MPI_INT, procese0[i], 1, MPI_COMM_WORLD);
             printf("M(%d,%d)\n", rank, procese0[i]);
        }
         //trimitere vector procese din cluster propriu
        for(int i = 0; i < nrProcese0; i++) {
            MPI_Send(procese0, nrProcese0, MPI_INT, procese0[i], 2, MPI_COMM_WORLD);
             printf("M(%d,%d)\n", rank, procese0[i]);
        }
        //trimitem nr de procese lui 1 si 2
        MPI_Send(&nrProcese0, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, 1);
        MPI_Recv(&procese[rank].count2, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Send(&nrProcese0,1,MPI_INT,2,0,MPI_COMM_WORLD);
        MPI_Recv(&procese[rank].count1, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("M(%d,%d)\n", rank, 2);

        procese[rank].topology[0] = calloc(nrProcese0, sizeof(int));
        procese[rank].topology[1] =  calloc(procese[rank].count1, sizeof(int));
        procese[rank].topology[2] = calloc(procese[rank].count2, sizeof(int));
        work1 = calloc(procese[rank].count1, sizeof(int));
        work2 = calloc(procese[rank].count2, sizeof(int));

        for(int i = 0; i < nrProcese0; i++) {
            procese[rank].topology[rank][i] = procese0[i];
        }
        //trimite proceser lui coord 1
        for(int i = 0;i < nrProcese0; i++) {
            MPI_Send(&procese0[i], 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, 1);
        }
        //trimiter procese lui coord 2
        for(int i = 0; i < nrProcese0; i++) {
            MPI_Send(&procese0[i], 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, 2);
        }

        //primire procese de la coord 1
        for(int i = 0; i < procese[rank].count1; i++) {
            MPI_Recv(&procese[rank].topology[1][i], 1 , MPI_INT, 1, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        }

        //primire procese de la coord 2
        for(int i = 0; i < procese[rank].count2; i++) {
            MPI_Recv(&procese[rank].topology[2][i], 1 ,MPI_INT, 2, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        }
       //afisare dupa ce a aflat topologia
        printf("%d -> 0:%d", rank, procese[rank].topology[0][0]);
        for(int i = 1 ; i < procese[rank].count0; i++) {
            printf(",%d", procese[rank].topology[0][i]);
        }
        printf(" 1:%d", procese[rank].topology[1][0]);
        for(int i = 1; i < procese[rank].count1; i++) {
            printf(",%d", procese[rank].topology[1][i]);
        }
        printf(" 2:%d",procese[rank].topology[2][0]);
        for(int i = 1; i < procese[rank].count2; i++) {
            printf(",%d", procese[rank].topology[2][i]);
        }
        printf("\n");

        //trimitere nr procese din cluster 1,2 la procesele din cluster 0
        for(int i = 0;i < nrProcese0; i++) {
            MPI_Send(&procese[rank].count1, 1 , MPI_INT, procese0[i], 1, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, procese0[i]);
        }

        for(int i = 0 ;i < nrProcese0; i++) {
            MPI_Send(&procese[rank].count2 ,1 , MPI_INT, procese0[i], 2, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, procese0[i]);
        }

        //trimitere toate procesele la workerii din cluster 0
        for(int i = 0; i < nrProcese0; i++) {
            
            MPI_Send(procese[rank].topology[1], procese[rank].count1, MPI_INT, procese0[i], 11, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, procese0[i]);
            MPI_Send(procese[rank].topology[2], procese[rank].count2, MPI_INT, procese0[i], 22, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, procese0[i]);
        }

        //generare vector
        int N = atoi(argv[1]);
        int auxdim = N;
        int nr = procese[rank].count0 + procese[rank].count1 + procese[rank].count2;
        while((auxdim % nr) != 0) {
           auxdim++;
        }
        int *V = calloc(auxdim, sizeof(int));

        //impartire la nr de procese
        int CHUNK_SIZE = auxdim / nr;
        for(int i = 0;i < auxdim; i++) {
            if(i < N) {
                V[i] = i;
            } else {
                V[i] = 0;
            }
        }
        
        //trimitem la  workerii din cluster propriu start,end,auxdim si vectorul
        //trimitere dimensiune
        for(int i = 0;i < nrProcese0; i++) {
            MPI_Send(&auxdim, 1, MPI_INT, procese0[i], 1, MPI_COMM_WORLD);
             printf("M(%d,%d)\n", rank, procese0[i]);
        }
        //trimitere la workerii din cluster propriu start si end
        //retinere pozitii pe care le modifica
        procese[rank].start = 0 * (double)auxdim / nr;
        
        for(int i = 0; i < nrProcese0; i++) {
            int start = i * (double)auxdim / nr;
            int end = min((i + 1) * (double)auxdim / nr, auxdim);
            MPI_Send(&start, 1, MPI_INT, procese0[i], 11, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, procese0[i]);
            MPI_Send(&end, 1, MPI_INT, procese0[i], 110, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, procese0[i]);
             procese[rank].end = end;
        }
        //trimiter la workerii din cluster propriu vectorul
        for(int i = 0;i < nrProcese0; i++) {
            MPI_Send(V, auxdim, MPI_INT, procese0[i], 111, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, procese0[i]);
        }

        //trimitere size si vector la 1 si 2
        MPI_Send(&auxdim, 1, MPI_INT, 1, 1, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, 1);
        MPI_Send(V, auxdim, MPI_INT, 1, 2, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, 1);

        MPI_Send(&auxdim ,1 ,MPI_INT, 2, 1, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, 2);
        MPI_Send(V, auxdim, MPI_INT, 2, 2, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, 2);
        

        //primire vector modificat de la workeri
        int *ref = calloc(auxdim, sizeof(int));
        for(int i = 0; i < nrProcese0; i++) {
            int s, e;
            MPI_Recv(&s ,1 ,MPI_INT, procese0[i], 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&e, 1, MPI_INT, procese0[i], 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(ref, auxdim, MPI_INT, procese0[i], 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for(int k = s; k < e;k++) {
                V[k] = ref[k];
            }
        }

        //primire date de la 2
        int s2,e2;
        MPI_Recv(&s2,1,MPI_INT,2,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        MPI_Recv(&e2,1,MPI_INT,2,1,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        int *altVector = calloc(auxdim, sizeof(int));
        MPI_Recv(altVector,auxdim,MPI_INT,2,2,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        for(int i = s2; i < e2;i++) {
            V[i] = altVector[i];
        }

        //primire date de la 1
        int s1,e1;
        MPI_Recv(&s1,1,MPI_INT,1,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        MPI_Recv(&e1,1,MPI_INT,1,1,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        int *vect = calloc(auxdim, sizeof(int));
        MPI_Recv(vect,auxdim,MPI_INT,1,2,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        for(int i=s1; i < e1;i++) {
            V[i] = vect[i];
        }
        //afisare rezultat final
        printf("Rezultat: ");
        for(int i = 0; i < N; i++) {
            printf("%d ",V[i]);
        }
        printf("\n");
    }
    //citire date cluster1
    if(rank == 1) {
        procese[rank].coord = -1;
        procese[rank].rankP = rank;
        int *work0, *work2;
        FILE * cluster1File = fopen("cluster1.txt", "rt");
        fscanf(cluster1File, "%d", &nrProcese1);
        procese1 = malloc(nrProcese1 * sizeof(int));
        if (procese1 == NULL) {
        fprintf(stderr, "Unable to allocate the buffer.\n");
        return 0;
        }
        procese[rank].count1 = nrProcese1;
        for(int i= 0; i < nrProcese1; i++) {
            fscanf(cluster1File, "%d", &procese1[i]);
        }
        //trimiter mesaj coordonator
        for(int i = 0; i < nrProcese1; i++) {
            MPI_Send(&rank,1,MPI_INT,procese1[i], MPI_TAG_UB, MPI_COMM_WORLD);
             printf("M(%d,%d)\n", rank, procese1[i]);
        }
        //trimiter nr procese cluster
        for(int i = 0; i < nrProcese1; i++) {
            MPI_Send(&nrProcese1,1,MPI_INT,procese1[i], 1, MPI_COMM_WORLD);
             printf("M(%d,%d)\n", rank, procese1[i]);
        }
         //trimitere vector procese din cluster propriu
        for(int i = 0; i < nrProcese1; i++) {
            MPI_Send(procese1,nrProcese1,MPI_INT,procese1[i], 2, MPI_COMM_WORLD);
             printf("M(%d,%d)\n", rank, procese1[i]);
        }

        //trimitem nr de procese lui 0 si 2
        MPI_Recv(&procese[rank].count0,1,MPI_INT,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        MPI_Send(&nrProcese1,1,MPI_INT,2,0,MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, 0);
        MPI_Recv(&procese[rank].count2,1,MPI_INT,2,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        MPI_Send(&nrProcese1,1,MPI_INT,0,0,MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, 2);
        
        procese[rank].topology[0] = calloc(procese[rank].count0, sizeof(int));
        procese[rank].topology[1] =  calloc(nrProcese1, sizeof(int));
        procese[rank].topology[2] = calloc(procese[rank].count2, sizeof(int));
        work0 = calloc(procese[rank].count0, sizeof(int));
        work2 = calloc(procese[rank].count2, sizeof(int));
        for(int i=0;i< nrProcese1;i++) {
            procese[rank].topology[rank][i] = procese1[i];
        }
        //trimite proceser lui coord 0
        for(int i=0;i < nrProcese1;i++) {
            MPI_Send(&procese1[i],1,MPI_INT,0,0,MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, 0);
        }
        //trimiter procese lui coord 2
        for(int i=0;i < nrProcese1;i++) {
            MPI_Send(&procese1[i],1,MPI_INT,2,0,MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, 2);
        }

        //primire procese de la coord 0
        for(int i=0; i < procese[rank].count0;i++) {
            MPI_Recv(&procese[rank].topology[0][i],1,MPI_INT,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        }

        //primire procese de la coord 2
        for(int i=0; i < procese[rank].count2;i++) {
            MPI_Recv(&procese[rank].topology[2][i],1,MPI_INT,2,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        }

        //afisare dupa ce a aflat topologia
        printf("%d -> 0:%d", rank, procese[rank].topology[0][0]);
        for(int i=1; i < procese[rank].count0;i++) {
            printf(",%d", procese[rank].topology[0][i]);
        }
        printf(" 1:%d", procese[rank].topology[1][0]);
        for(int i=1; i < procese[rank].count1;i++) {
            printf(",%d", procese[rank].topology[1][i]);
        }
        printf(" 2:%d",procese[rank].topology[2][0]);
        for(int i=1; i < procese[rank].count2;i++) {
            printf(",%d", procese[rank].topology[2][i]);
        }
        printf("\n");

        //trimitere nr procese din cluster 0,2 la procesele din cluster 1
        for(int i=0;i < nrProcese1;i++) {
            MPI_Send(&procese[rank].count0,1,MPI_INT,procese1[i],0,MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, procese1[i]);
        }

        for(int i=0;i < nrProcese1;i++) {
            MPI_Send(&procese[rank].count2,1,MPI_INT,procese1[i],2,MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, procese1[i]);
        }

        //trimitere toate procesele la workerii din cluster 1
        for(int i=0; i<nrProcese1;i++) {
            
            MPI_Send(procese[rank].topology[0],procese[rank].count0,MPI_INT,procese1[i],0,MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, procese1[i]);
            MPI_Send(procese[rank].topology[2],procese[rank].count2,MPI_INT,procese1[i],22,MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, procese1[i]);
        }

        //primire dim si vector de la 0
        int *V;
        int auxdim;
        MPI_Recv(&auxdim,1,MPI_INT,0,1,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        V = calloc(auxdim, sizeof(int));
        MPI_Recv(V,auxdim,MPI_INT,0,2,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        procese[rank].start = -1;
        procese[rank].end = -1;
        int nr = procese[rank].count0 + procese[rank].count1 + procese[rank].count2;
        
        //trimitere dimensiune la workerii din 1
        for(int i=0;i<nrProcese1;i++) {
            MPI_Send(&auxdim,1,MPI_INT,procese1[i],0,MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, procese1[i]);
        }
        //trimitere start,end la workerii din cluster propriu
        procese[rank].start = procese[rank].count0 * (double)auxdim / nr;
         for(int i = procese[rank].count0; i<(nrProcese1 + procese[rank].count0);i++) {
            int start = i * (double)auxdim / nr;
            int end = min((i + 1) * (double)auxdim / nr, auxdim);
            MPI_Send(&start,1,MPI_INT,procese1[i - procese[rank].count0],1,MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, procese1[i - procese[rank].count0]);
            MPI_Send(&end,1,MPI_INT,procese1[i - procese[rank].count0],2,MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, procese1[i - procese[rank].count0]);
            procese[rank].end = end;
         }
         //trimitere tot vectorul la workerii din cluster 1
         for(int i=0;i<nrProcese1;i++) {
             MPI_Send(V,auxdim,MPI_INT,procese1[i],3,MPI_COMM_WORLD);
             printf("M(%d,%d)\n", rank, procese1[i]);
         }

         //primire vector modificat de la workeri
        int *ref = calloc(auxdim, sizeof(int));
        for(int i =0; i<nrProcese1;i++) {
            int s, e;
            MPI_Recv(&s,1,MPI_INT,procese1[i],0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            MPI_Recv(&e,1,MPI_INT,procese1[i],1,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            MPI_Recv(ref,auxdim,MPI_INT,procese1[i],2,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            for(int k = s; k < e;k++) {
                V[k] = ref[k];
            }
        }

         //trimitere pozitiile finale si vect de modificat catre 0
        MPI_Send(&procese[rank].start,1,MPI_INT,0,0,MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, 0);
        MPI_Send(&procese[rank].end,1,MPI_INT,0,1,MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, 0);
        MPI_Send(V,auxdim,MPI_INT,0,2,MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, 0);

    }
    //citire date cluster2
    if(rank == 2) {
        procese[rank].coord = -1;
        procese[rank].rankP = rank;
        int *work0, *work1;
        FILE * cluster2File = fopen("cluster2.txt", "rt");
        fscanf(cluster2File, "%d", &nrProcese2);
        procese2 = malloc(nrProcese2 * sizeof(int));
        if (procese2 == NULL) {
        fprintf(stderr, "Unable to allocate the buffer.\n");
        return 0;
        }

        for(int i= 0; i < nrProcese2; i++) {
            fscanf(cluster2File, "%d", &procese2[i]);
        }
        
        //trimiter mesaj coordonator
        for(int i = 0; i < nrProcese2; i++) {
            MPI_Send(&rank,1,MPI_INT,procese2[i], MPI_TAG_UB, MPI_COMM_WORLD);
             printf("M(%d,%d)\n", rank, procese2[i]);
        }
        //trimiter nr procese cluster
        for(int i = 0; i < nrProcese2; i++) {
            MPI_Send(&nrProcese2,1,MPI_INT,procese2[i], 1, MPI_COMM_WORLD);
             printf("M(%d,%d)\n", rank, procese2[i]);
        }
        //trimitere vector procese din cluster propriu
        for(int i = 0; i < nrProcese2; i++) {
            MPI_Send(procese2,nrProcese2,MPI_INT,procese2[i], 2, MPI_COMM_WORLD);
             printf("M(%d,%d)\n", rank, procese2[i]);
        }
        
        //trimitem nr de procese lui 0 si 2
        MPI_Recv(&procese[rank].count1,1,MPI_INT,1,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        MPI_Send(&nrProcese2,1,MPI_INT,0,0,MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, 0);
        MPI_Recv(&procese[rank].count0,1,MPI_INT,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        MPI_Send(&nrProcese2,1,MPI_INT,1,0,MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, 1);

        procese[rank].topology[0] = calloc(procese[rank].count0, sizeof(int));
        procese[rank].topology[1] =  calloc(procese[rank].count1, sizeof(int));
        procese[rank].topology[2] = calloc(nrProcese2, sizeof(int));
        work0 = calloc(procese[rank].count0, sizeof(int));
        work1 = calloc(procese[rank].count1, sizeof(int));
        procese[rank].count2 = nrProcese2;
        for(int i=0;i < nrProcese2;i++) {
            procese[rank].topology[rank][i] = procese2[i];
        }
        //trimite proceser lui coord 1
        for(int i=0;i < nrProcese2;i++) {
            MPI_Send(&procese2[i],1,MPI_INT,1,0,MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, 1);
        }
        //trimiter procese lui coord 0
        for(int i=0;i < nrProcese2;i++) {
            MPI_Send(&procese2[i],1,MPI_INT,0,0,MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, 0);
        }

        //primire procese de la coord 1
        for(int i=0; i < procese[rank].count1;i++) {
            MPI_Recv(&procese[rank].topology[1][i],1,MPI_INT,1,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        }

        //primire procese de la coord 0
        for(int i=0; i < procese[rank].count0;i++) {
            MPI_Recv(&procese[rank].topology[0][i],1,MPI_INT,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        }
        
        //afisare dupa ce a aflat topologia
        printf("%d -> 0:%d", rank, procese[rank].topology[0][0]);
        for(int i=1; i < procese[rank].count0;i++) {
            printf(",%d", procese[rank].topology[0][i]);
        }
        printf(" 1:%d", procese[rank].topology[1][0]);
        for(int i=1; i < procese[rank].count1;i++) {
            printf(",%d", procese[rank].topology[1][i]);
        }
        printf(" 2:%d",procese[rank].topology[2][0]);
        for(int i=1; i < procese[rank].count2;i++) {
            printf(",%d", procese[rank].topology[2][i]);
        }
        printf("\n");

        //trimitere nr procese din cluster 0,1 la procesele din cluster 2
        for(int i=0;i < nrProcese2;i++) {
            MPI_Send(&procese[rank].count0,1,MPI_INT,procese2[i],0,MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, procese2[i]);
        }

        for(int i=0;i < nrProcese2;i++) {
            MPI_Send(&procese[rank].count1,1,MPI_INT,procese2[i],1,MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, procese2[i]);
        }

        //trimitere toate procesele la workerii din cluster 2
        for(int i=0; i<nrProcese2;i++) {
            
            MPI_Send(procese[rank].topology[0],procese[rank].count0,MPI_INT,procese2[i],0,MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, procese2[i]);
            MPI_Send(procese[rank].topology[1],procese[rank].count1,MPI_INT,procese2[i],11,MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, procese2[i]);
        }

        //primire dim si vector de la 0
        int *V;
        int auxdim;
        MPI_Recv(&auxdim,1,MPI_INT,0,1,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        V = calloc(auxdim, sizeof(int));
        MPI_Recv(V,auxdim,MPI_INT,0,2,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        procese[rank].start = -1;
        procese[rank].end = -1;
        int nr = procese[rank].count0 + procese[rank].count1 + procese[rank].count2;
        int k = procese[rank].count0 + procese[rank].count1;
        //trimitere dimensiune la workerii din 1
        for(int i=0;i<nrProcese2;i++) {
            MPI_Send(&auxdim,1,MPI_INT,procese2[i],0,MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, procese2[i]);
        }
        procese[rank].start = k * (double)auxdim / nr;
        //trimitere start,end la workerii din cluster propriu
         for(int i = k; i<(nrProcese2 + k);i++) {
            int start = i * (double)auxdim / nr;
            int end = min((i + 1) * (double)auxdim / nr, auxdim);
            MPI_Send(&start,1,MPI_INT,procese2[i - k],1,MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, procese2[i - k]);
            MPI_Send(&end,1,MPI_INT,procese2[i - k],2,MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, procese2[i - k]);
            procese[rank].end = end;
         }
         //trimitere tot vectorul la workerii din cluster 1
         for(int i=0;i<nrProcese2;i++) {
             MPI_Send(V,auxdim,MPI_INT,procese2[i],3,MPI_COMM_WORLD);
             printf("M(%d,%d)\n", rank, procese2[i]);
         }

         //primire vector modificat de la workeri
        int *ref = calloc(auxdim, sizeof(int));
        for(int i =0; i<nrProcese2;i++) {
            int s, e;
            MPI_Recv(&s,1,MPI_INT,procese2[i],0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            MPI_Recv(&e,1,MPI_INT,procese2[i],1,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            MPI_Recv(ref,auxdim,MPI_INT,procese2[i],2,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            for(int k = s; k < e;k++) {
                V[k] = ref[k];
            }
        }

        //trimitere pozitiile finale si vect de modificat catre 0
        MPI_Send(&procese[rank].start,1,MPI_INT,0,0,MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, 0);
        MPI_Send(&procese[rank].end,1,MPI_INT,0,1,MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, 0);
        MPI_Send(V,auxdim,MPI_INT,0,2,MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, 0);
    }

    //trimiter mesaje cu coordonatorul la procesele din clusterul fiecaruia
    if(rank != 1 && rank != 0 && rank != 2) {
        MPI_Status status;
        MPI_Recv(&coord, 1,MPI_INT,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
        procese[rank].coord = coord;
        printf("M(%d,%d)\n",status.MPI_SOURCE, procese[rank].rankP);
    }
    //primire nr de procese din clusterul din care face parte procesul si alocare memorie pentru matricea de procese
    if(rank != 1 && rank != 0 && rank != 2) {
        MPI_Status status;
        MPI_Recv(&val, 1,MPI_INT,MPI_ANY_SOURCE,1,MPI_COMM_WORLD,&status);
        if(status.MPI_SOURCE == 0) {
            procese[rank].count0 = val;
            procese[rank].topology[status.MPI_SOURCE] = calloc(val, sizeof(int));
        } else if(status.MPI_SOURCE == 1) {
            procese[rank].count1 = val;
             procese[rank].topology[status.MPI_SOURCE] = calloc(val, sizeof(int));
        } else {
            procese[rank].count2 = val;
             procese[rank].topology[status.MPI_SOURCE] = calloc(val, sizeof(int));
        }
        printf("M(%d,%d)\n",status.MPI_SOURCE, procese[rank].rankP);
    }
    
    if(rank != 1 && rank != 0 && rank != 2) {
        MPI_Status status;
       if(procese[rank].coord == 0) {
           int *v = calloc(procese[rank].count0, sizeof(int));
           MPI_Recv(v, procese[rank].count0, MPI_INT,MPI_ANY_SOURCE,2,MPI_COMM_WORLD,&status);
           copyVector(v, procese[rank].topology[status.MPI_SOURCE], procese[rank].count0);
           free(v);
       } else if(procese[rank].coord == 1) {
           int *v = calloc(procese[rank].count1, sizeof(int));
           MPI_Recv(v, procese[rank].count1, MPI_INT,MPI_ANY_SOURCE,2,MPI_COMM_WORLD,&status);
           copyVector(v, procese[rank].topology[status.MPI_SOURCE], procese[rank].count1);
           free(v);
       } else {
           int *v = calloc(procese[rank].count2, sizeof(int));
           MPI_Recv(v, procese[rank].count2, MPI_INT,MPI_ANY_SOURCE,2,MPI_COMM_WORLD,&status);
           copyVector(v, procese[rank].topology[status.MPI_SOURCE], procese[rank].count2);
           free(v);
       }
        printf("M(%d,%d)\n",status.MPI_SOURCE, procese[rank].rankP);
    }

    //primire nr procese din celelalte clusteri
    if(rank != 0 && rank != 1 && rank != 2) {
        if(procese[rank].coord == 0) {
            MPI_Recv(&procese[rank].count1,1,MPI_INT,0,1,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            procese[rank].topology[1] = calloc(procese[rank].count1, sizeof(int));
            MPI_Recv(&procese[rank].count2,1,MPI_INT,0,2,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            procese[rank].topology[2] = calloc(procese[rank].count2, sizeof(int));
        }
        if(procese[rank].coord == 1) {
            MPI_Recv(&procese[rank].count0,1,MPI_INT,1,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            procese[rank].topology[0] = calloc(procese[rank].count0, sizeof(int));
            MPI_Recv(&procese[rank].count2,1,MPI_INT,1,2,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            procese[rank].topology[2] = calloc(procese[rank].count2, sizeof(int));
        }
        if(procese[rank].coord == 2) {
            MPI_Recv(&procese[rank].count0,1,MPI_INT,2,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            procese[rank].topology[0] = calloc(procese[rank].count0, sizeof(int));
            MPI_Recv(&procese[rank].count1,1,MPI_INT,2,1,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            procese[rank].topology[1] = calloc(procese[rank].count1, sizeof(int));
        }
    }
    //primire a tuturor celorlalti workeri si afisarea topologiei
    if(rank != 0 && rank != 1 && rank != 2) {
        if(procese[rank].coord == 0) {
            int *v = calloc(procese[rank].count1, sizeof(int));
            MPI_Recv(v, procese[rank].count1,MPI_INT,0,11,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            copyVector(v, procese[rank].topology[1], procese[rank].count1);
            free(v);
            int *m = calloc(procese[rank].count2, sizeof(int));
            MPI_Recv(m, procese[rank].count2,MPI_INT,0,22,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            copyVector(m, procese[rank].topology[2], procese[rank].count2);
            free(m);
            //afisare dupa ce a aflat topologia
            printf("%d -> 0:%d", rank, procese[rank].topology[0][0]);
            for(int i=1; i < procese[rank].count0;i++) {
                printf(",%d", procese[rank].topology[0][i]);
            }
            printf(" 1:%d", procese[rank].topology[1][0]);
            for(int i=1; i < procese[rank].count1;i++) {
                printf(",%d", procese[rank].topology[1][i]);
            }
            printf(" 2:%d",procese[rank].topology[2][0]);
            for(int i=1; i < procese[rank].count2;i++) {
                printf(",%d", procese[rank].topology[2][i]);
            }
            printf("\n");
        }

        if(procese[rank].coord == 1) {
            int *v = calloc(procese[rank].count0, sizeof(int));
            MPI_Recv(v, procese[rank].count0,MPI_INT,1,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            copyVector(v, procese[rank].topology[0], procese[rank].count0);
            free(v);
            int *m = calloc(procese[rank].count2, sizeof(int));
            MPI_Recv(m, procese[rank].count2,MPI_INT,1,22,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            copyVector(m, procese[rank].topology[2], procese[rank].count2);
            free(m);
            //afisare dupa ce a aflat topologia
            printf("%d -> 0:%d", rank, procese[rank].topology[0][0]);
            for(int i=1; i < procese[rank].count0;i++) {
                printf(",%d", procese[rank].topology[0][i]);
            }
            printf(" 1:%d", procese[rank].topology[1][0]);
            for(int i=1; i < procese[rank].count1;i++) {
                printf(",%d", procese[rank].topology[1][i]);
            }
            printf(" 2:%d",procese[rank].topology[2][0]);
            for(int i=1; i < procese[rank].count2;i++) {
                printf(",%d", procese[rank].topology[2][i]);
            }
            printf("\n");
        }

        if(procese[rank].coord == 2) {
            int *v = calloc(procese[rank].count0, sizeof(int));
            MPI_Recv(v, procese[rank].count0,MPI_INT,2,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            copyVector(v, procese[rank].topology[0], procese[rank].count0);
            free(v);
            int *m = calloc(procese[rank].count1, sizeof(int));
            MPI_Recv(m, procese[rank].count1,MPI_INT,2,11,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            copyVector(m, procese[rank].topology[1], procese[rank].count1);
            free(m);
            //afisare dupa ce a aflat topologia
            printf("%d -> 0:%d", rank, procese[rank].topology[0][0]);
            for(int i=1; i < procese[rank].count0;i++) {
                printf(",%d", procese[rank].topology[0][i]);
            }
            printf(" 1:%d", procese[rank].topology[1][0]);
            for(int i=1; i < procese[rank].count1;i++) {
                printf(",%d", procese[rank].topology[1][i]);
            }
            printf(" 2:%d",procese[rank].topology[2][0]);
            for(int i=1; i < procese[rank].count2;i++) {
                printf(",%d", procese[rank].topology[2][i]);
            }
            printf("\n");
        }
    }

    if(rank != 0 && rank != 1 && rank != 2) {
        if(procese[rank].coord == 0) {
            int auxdim;
            MPI_Recv(&auxdim,1,MPI_INT,0,1,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            MPI_Recv(&procese[rank].start,1,MPI_INT,0,11,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            MPI_Recv(&procese[rank].end,1,MPI_INT,0,110,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            int *v_recv = calloc(auxdim, sizeof(int));
            MPI_Recv(v_recv, auxdim,MPI_INT,0,111,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            for(int i = procese[rank].start; i < procese[rank].end;i++) {
                v_recv[i] = v_recv[i] * 2;
            }
            //trimitem start si end
            MPI_Send(&procese[rank].start,1,MPI_INT,0,0,MPI_COMM_WORLD);
            printf("M(%d,%d)\n",rank, procese[rank].coord);
            MPI_Send(&procese[rank].end,1,MPI_INT,0,1,MPI_COMM_WORLD);
            printf("M(%d,%d)\n",rank, procese[rank].coord);
            //trimitem vectorul inapoi la coord
            MPI_Send(v_recv,auxdim,MPI_INT,0,2,MPI_COMM_WORLD);
             printf("M(%d,%d)\n",rank, procese[rank].coord);
             free(v_recv);
        }
        if(procese[rank].coord == 1) {
            int auxdim;
            MPI_Recv(&auxdim,1,MPI_INT,1,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            MPI_Recv(&procese[rank].start,1,MPI_INT,1,1,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            MPI_Recv(&procese[rank].end,1,MPI_INT,1,2,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            int *v_recv = calloc(auxdim, sizeof(int));
            MPI_Recv(v_recv, auxdim,MPI_INT,1,3,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            for(int i = procese[rank].start; i < procese[rank].end;i++) {
                v_recv[i] = v_recv[i] * 2;
            }
           //trimitem start si end
            MPI_Send(&procese[rank].start,1,MPI_INT,1,0,MPI_COMM_WORLD);
            printf("M(%d,%d)\n",rank, procese[rank].coord);
            MPI_Send(&procese[rank].end,1,MPI_INT,1,1,MPI_COMM_WORLD);
            printf("M(%d,%d)\n",rank, procese[rank].coord);
            //trimitem vectorul inapoi la coord
            MPI_Send(v_recv,auxdim,MPI_INT,1,2,MPI_COMM_WORLD);
             printf("M(%d,%d)\n",rank, procese[rank].coord);
             free(v_recv);
             
        }
        if(procese[rank].coord == 2) {
            int auxdim;
            MPI_Recv(&auxdim,1,MPI_INT,2,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            MPI_Recv(&procese[rank].start,1,MPI_INT,2,1,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            MPI_Recv(&procese[rank].end,1,MPI_INT,2,2,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            int *v_recv = calloc(auxdim, sizeof(int));
            MPI_Recv(v_recv, auxdim,MPI_INT,2,3,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            for(int i = procese[rank].start; i < procese[rank].end;i++) {
                v_recv[i] = v_recv[i] * 2;
            }
            //trimitem start si end
            MPI_Send(&procese[rank].start,1,MPI_INT,2,0,MPI_COMM_WORLD);
            printf("M(%d,%d)\n",rank, procese[rank].coord);
            MPI_Send(&procese[rank].end,1,MPI_INT,2,1,MPI_COMM_WORLD);
            printf("M(%d,%d)\n",rank, procese[rank].coord);
            //trimitem vectorul inapoi la coord
            MPI_Send(v_recv,auxdim,MPI_INT,2,2,MPI_COMM_WORLD);
             printf("M(%d,%d)\n",rank, procese[rank].coord);
             free(v_recv);
             
        }
    }

    
   
    
    
    MPI_Finalize();
}