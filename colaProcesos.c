#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_P 5
#define NUM_RCPU 2
#define QUANTO 4
#define MOSTRAR 1
#define CLEAR 0 //1 para "clear" / 0 para "cls"

/***Estructuras**/
typedef struct proceso{
    int id, llegada;
    int rCPU[NUM_RCPU];
    int rES[NUM_RCPU];
    int tVuelta,tEspera,tRespuesta,tFinalizado;
    int indiceCPU, indiceES, contador;
}PROCESO;

typedef struct data{
    PROCESO P[NUM_P];       //Lista de los procesos
    PROCESO *cCPU[NUM_P];   //Cola de prosesos para uso de CPU
    PROCESO *cES[NUM_P];    //Cola de procesos en E/S
    PROCESO *usandoCPU;     //Proceso en uso actual del CPU
    int q;                  //Contador para administrar el quanto
    PROCESO *Terminados[NUM_P];
}DATA;

/***Prototipos de funcion***/

//Inicializacion
void inicializar(DATA *d);

//Funciones Funcionalidad
void pasarDelTiempo(DATA *d, int t);
void ingresarColaCPU(DATA *d, PROCESO *p);
void ingresarColaES(DATA *d, PROCESO *p);
void ingresarTerminados(DATA *d, PROCESO *p, int t);
void avanzaCPU(DATA *d);
void ajustarColaES(DATA *d);
void aumentarContadores(DATA *d);
void calculos(DATA *d);

//Funciones Imprimir / Mostrar Pasos
void mostrarPasos(DATA *d, int t); 
void printUsoCPU(DATA *d);
void printColaCPU(DATA *d);
void printColaES(DATA *d);
void printTerminados(DATA *d);
void printInfoProcesos(DATA *d);


/***Main***/
int main (){
    DATA d;

    inicializar(&d);
    if(!MOSTRAR) printInfoProcesos(&d);
    for(int t=0; !d.Terminados[NUM_P-1]; t++){
        pasarDelTiempo(&d, t);
        if(MOSTRAR) mostrarPasos(&d, t);
    }
    printTerminados(&d);
    calculos(&d);
    return 0;
}


/***********Funciones************/

/***Inicializacion***/
void inicializar(DATA *d){
    /*Necesario para la funci�n random*/
    time_t t;
    srand((unsigned)time(&t));
    
    
    /*Inicializando PROCESOS*/
    for(int i=0; i<NUM_P; i++){
        d->P[i].id = i+1;
        d->P[i].llegada = rand() % 21; //Hora de llegada (0-20)
        for(int j=0; j<NUM_RCPU; j++){
            d->P[i].rCPU[j] = (rand() % 14) + 2; //Rafaga CPU (2-15)
        }
        for(int j=0; j<NUM_RCPU-1; j++){
            //Rafaga E/S (entre 2 y 3 veces mayor a Rafaga CPU);
            d->P[i].rES[j] = (rand() % (d->P[i].rCPU[j]*3 - d->P[i].rCPU[j]*2)) + d->P[i].rCPU[j]*2 + 1; 
        }
        d->P[i].tVuelta = 0;
        d->P[i].tEspera = 0;
        d->P[i].indiceCPU = 0;
        d->P[i].indiceES = 0;
        d->P[i].contador = 0;

        /*Inicializando LISTAS*/
        for(int i=0; i<NUM_P; i++){
            d->cCPU[i] = NULL;
            d->cES[i] = NULL;
            d->Terminados[i] = NULL;
        }

        d->usandoCPU = NULL; //Proceso en uso actual del CPU
        d->q = 0;            //Contador del quanto
    }
}

/***Funciones Funcionalidad***/

void pasarDelTiempo(DATA *d, int t){
    aumentarContadores(d);
    //<Llegada>
    for(int i=0; i<NUM_P; i++){
        if(d->P[i].llegada == t){
            ingresarColaCPU(d, &(d->P[i]));
        }
    }
    //</Llegada>

    //<Cola E/S>
    for(int i=0; i<NUM_P && d->cES[i] != NULL; i++){
        d->cES[i]->contador--;
        if(d->cES[i]->contador == 0){
            d->cES[i]->indiceES++;
            ingresarColaCPU(d, d->cES[i]);
            
            d->cES[i] = NULL;
        }
    }
    ajustarColaES(d);
    //</Cola E/S>

    //<Uso de CPU>
    if(d->usandoCPU){
        d->usandoCPU->contador--;
        d->q++;
        if(d->usandoCPU->contador == 0){
            d->usandoCPU->indiceCPU++;
            if(d->usandoCPU->indiceCPU == NUM_RCPU) ingresarTerminados(d, d->usandoCPU, t);
            else ingresarColaES(d, d->usandoCPU);
            d->usandoCPU = NULL;
        }
        else if(d->q == QUANTO){
            ingresarColaCPU(d, d->usandoCPU);
            d->usandoCPU = NULL;
        }
    }

    if(d->usandoCPU == NULL && d->cCPU[0]){
        d->usandoCPU = d->cCPU[0];
        if(d->usandoCPU->indiceCPU == 0 && d->usandoCPU->contador == d->usandoCPU->rCPU[0]){
            d->usandoCPU->tRespuesta = t - d->usandoCPU->llegada;
        }
        avanzaCPU(d);
        d->q = 0;
    }
    //</Uso de CPU>
}

void ingresarColaCPU(DATA *d, PROCESO *p){
    for(int i=0; i<NUM_P; i++){
        if(d->cCPU[i] == NULL){
            d->cCPU[i] = p;
            if(p->contador == 0) p->contador = p->rCPU[p->indiceCPU];
            return;
        }
    }
}

void ingresarColaES(DATA *d, PROCESO *p){
    for(int i=0; i<NUM_P; i++){
        if(d->cES[i] == NULL){
            d->cES[i] = p;
            p->contador = p->rES[p->indiceES];
            return;
        }
    }
}

void ingresarTerminados(DATA *d, PROCESO *p, int t){
    for(int i=0; i<NUM_P; i++){
        if(d->Terminados[i] == NULL){
            d->Terminados[i] = p;
            p->tFinalizado = t;
            return;
        }
    }
}

void avanzaCPU(DATA *d){
    int i;
    for(i=1; i < NUM_P && d->cCPU[i] != NULL; i++){
        d->cCPU[i-1] = d->cCPU[i];
    }
    d->cCPU[i-1] = NULL;
}

void ajustarColaES(DATA *d){
    int j;
    for(int i=0; i<NUM_P; i++){
        if(i>0 && d->cES[i]!=NULL && d->cES[i-1]==NULL){
            for(j=i-1; j>=0 && d->cES[j]==NULL; j--);
            d->cES[j+1] = d->cES[i];
            d->cES[i] = NULL;
        }
    }
}

void aumentarContadores(DATA *d){
    if(d->usandoCPU){
        d->usandoCPU->tVuelta++;
    }

    for(int i=0; i<NUM_P && d->cCPU[i]; i++){
        d->cCPU[i]->tVuelta++;
        d->cCPU[i]->tEspera++;
    }

    for(int i=0; i<NUM_P && d->cES[i]; i++){
        d->cES[i]->tVuelta++;
    }
}

void calculos(DATA *d){
    float tpVuelta=0, tpEspera=0, tpRespuesta=0, tazaSalida;
    for(int i=0; i<NUM_P; i++){
        tpVuelta += d->P[i].tVuelta;
        tpEspera += d->P[i].tEspera;
        tpRespuesta += d->P[i].tRespuesta;
    }

    tpVuelta /= (float)NUM_P;
    tpEspera /= (float)NUM_P;
    tpRespuesta /= (float)NUM_P;
    tazaSalida = d->Terminados[NUM_P-1]->tFinalizado / (float)NUM_P;

    printf("Calculos:\n");
    printf("Tiempo promedio de Vuelta: %.2f\n", tpVuelta);
    printf("Tiempo promedio de Espera: %.2f\n", tpEspera);
    printf("Tiempo promedio de Respuesta: %.2f\n", tpRespuesta);
    printf("Taza de Salida: %.2f\n", tazaSalida);
}


/***Funciones Imprimir / Mostrar Pasos***/

void mostrarPasos(DATA *d, int t){
    printInfoProcesos(d);
    printf("T%d: ", t);
    printUsoCPU(d);
    printColaCPU(d);
    printColaES(d);
    printTerminados(d);
    getchar();

    if(CLEAR) system("clear");
    else      system("cls");
}

void printUsoCPU(DATA *d){
    if(d->usandoCPU) printf(" [P%d(%d) - %d]\n", d->usandoCPU->id, d->usandoCPU->contador, QUANTO - d->q);
    else printf(" [--]\n");
}

void printColaCPU(DATA *d){
    printf("Cola CPU:\n");
    for(int i=0; i<NUM_P; i++){
        if(d->cCPU[i] == NULL)  printf("- ");
        else  printf("P%d(%d) ", d->cCPU[i]->id, d->cCPU[i]->contador);
    }
    printf("\n\n");
}

void printColaES(DATA *d){
    printf("Procesos en E/S:\n");
    for(int i=0; i<NUM_P; i++){
        if(d->cES[i] == NULL) //break;
        printf("- ");
        else
        printf("P%d(%d) ", d->cES[i]->id, d->cES[i]->contador);
    }
    printf("\n\n");
}

void printTerminados(DATA *d){
    if(d->Terminados[0] == NULL){
        return;
    }

    printf("TERMINADOS:\n");
    for(int i=0; i<NUM_P; i++){
        if(d->Terminados[i] != NULL) 
        printf("P%d t(%d) v(%d) e(%d) r(%d)\n", d->Terminados[i]->id, d->Terminados[i]->tFinalizado, d->Terminados[i]->tVuelta, d->Terminados[i]->tEspera, d->Terminados[i]->tRespuesta);
    }
    printf("\n\n");
}

void printInfoProcesos(DATA *d){
    int j;
    printf("P\tLlegada\t\tRafagas CPU - Rafaga E/S\n");
    for(int i=0; i<NUM_P; i++){
        printf("%d\t%d\t\t", d->P[i].id, d->P[i].llegada);
        for(j=0; j<NUM_RCPU-1; j++){
            printf("%.2d [%.2d] ", d->P[i].rCPU[j], d->P[i].rES[j]);
        }
        printf("%.2d\n", d->P[i].rCPU[j]);
    }
    printf("\n");
}