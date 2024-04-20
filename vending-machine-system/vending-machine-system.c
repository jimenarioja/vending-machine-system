// AUTOR: Jimena Rioja Olmedillas

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>

#define TAM_BUFFER 5000
#define MAX_PROVEEDORES 7
#define MAX_CONSUMIDORES 1000
#define MAX_PRODUCTOS 10

// Estructura para los productos
struct stProducto{
    char c;
    int nproveedor;
};
typedef struct stProducto Producto;

// Estructura para el nodo de la lista enlazada
struct stNodo {
    int id;
    int contadorProveedores[MAX_PROVEEDORES];
    int contadorProductos[MAX_PRODUCTOS];
    int totalProductos;
    struct stNodo* siguiente;
};
typedef struct stNodo Nodo;

int T, P, C, ip, ic, nc, np;
FILE *salida;
char *path, *ruta_salida;
Producto *buffer;
Nodo *p, *f;
sem_t hay_dato, hay_hueco, sc_ip, sc_ic, sc_fichero, lista_mutex, mutex_salida, despertar_facturador, sc_np;


void* proveedor(void* arg);
void* consumidor(void* arg);
void* facturador(void* arg);


int main(int argc, char* argv[]) {

    int *idProveedor;
    int *idConsumidor;
    int i;
    pthread_t *proveedores;
    pthread_t *consumidores;
    pthread_t facturadorThread;

    // Comprobamos que el numero de argumentos es correcto
    if (argc != 6) {
        fprintf(stderr, "Error. Hay que recibir 5 argumentos.\n");
        exit(-1);
    }


    // Verificacion del parametro correspondiente al tamaño del buffer circular
    if(sscanf(argv[3], "%d", &T) != 1 || T < 1 || T > TAM_BUFFER){
        printf("Error, el tama�o del buffer circular tiene que estar comprendido entre 1 y 5000\n");
        exit(-1);
    }

    // Verificacion del parametro correspondiente al numero de proveedores
    if(sscanf(argv[4], "%d", &P) != 1 || P < 1 || P > MAX_PROVEEDORES){
        printf("Error, el numero de proveedores tiene que estar comprendido entre 1 y 7\n");
        exit(-1);
    }

    // Verificacion del parametro correspondiente al numero de clientes consumidores
    if(sscanf(argv[5], "%d", &C) != 1 || C < 1 || C > MAX_CONSUMIDORES){
        printf("Error, el numero de clientes consumidores tiene que estar comprendido entre 1 y 1000\n");
        exit(-1);
    }

    // Incializamos las variables
    ip=0;
    ic=0;
    path = argv[1];
    ruta_salida = argv[2];
    nc = C;
    np = P;
    p=NULL;
    f=NULL;

    // Inicializamos los semaforos
    sem_init(&hay_dato, 0, 0);
    sem_init(&hay_hueco, 0, T);
    sem_init(&sc_ip, 0, 1);
    sem_init(&sc_ic, 0, 1);
    sem_init(&sc_fichero, 0, 1);
    sem_init(&lista_mutex, 0, 1);
    sem_init(&despertar_facturador,0,0);
    sem_init(&mutex_salida,0,1);
    sem_init(&sc_np, 0, 1);


    // Reserva de memoria dinamica de buffer
    if((buffer=(Producto *)malloc(sizeof(Producto)*T))==NULL){
        fprintf(stderr, "Error al reservar memoria.\n");
        exit(-1);
    }

    // Reserva de memoria dinamica de proveedores
    if((proveedores =(pthread_t *)malloc(sizeof(pthread_t)*P))==NULL){
        fprintf(stderr, "Error al reservar memoria de proveedores pthreadt.\n");
        exit(-1);
    }

    // Reserva de memoria dinamica de consumidores
    if((consumidores=(pthread_t *)malloc(sizeof(pthread_t)*C))==NULL){
        fprintf(stderr,"Error al reservar memoria de consumidores pthreadt.\n");
    }

    // Comprobamos si el fichero de salida ya existe. En tal caso, lanzamos un aviso por pantalla de que lo vamos a sobreescribir
    if((salida = fopen(ruta_salida, "r")) != NULL){
        fprintf(stdout,"AVISO: borrar� el fichero de resultados:%s\n\n", ruta_salida);
        fclose(salida);
    }

    // Comprobamos si el fichero se puede abrir correctamente para escritura
    if((salida = fopen(ruta_salida, "w")) == NULL){
        fprintf(stderr,"Error: no se pudo abrir el archivo de salida.\n");
        exit(-1);
    }

    // Reservamos memoria para los idProveedor
    if((idProveedor = (int *)malloc((sizeof(int))*P))==NULL){
        fprintf(stderr,"Error al reservar memoria\n");
        exit(-1);
    }

    // Reservamos memoria para los idConsumidor
    if((idConsumidor= (int *)malloc((sizeof(int))*C))==NULL){
        fprintf(stderr,"Error al reservar memoria\n");
        exit(-1);
    }

    // Creamos los hilos proveedores
    for (i = 0; i < P; i++) {
        idProveedor[i] = i;
        pthread_create(&proveedores[i], NULL, proveedor, (void* ) &idProveedor[i]);
    }

    // Creamos los hilos consumidores
    for (i = 0; i < C; i++) {
        idConsumidor[i] = i;
        pthread_create(&consumidores[i], NULL, consumidor, (void* ) &idConsumidor[i]);
    }

    // Creamos el hilo facturador
    pthread_create(&facturadorThread, NULL, facturador, (void*) salida);

    // Esperamos a que terminen hilos proveedores
    for (i = 0; i < P; i++) {
        pthread_join(proveedores[i], NULL);
    }

    // Esperamos a que terminen los hilos consumidores
    for (i = 0; i < C; i++) {
        pthread_join(consumidores[i], NULL);
    }

    // Esperamos a que termine el hilo facturador
    pthread_join(facturadorThread, NULL);

    // cerramos el fichero de salida
    fclose(salida);

    // destruimos los semaforos
    sem_destroy(&hay_dato);
    sem_destroy(&hay_hueco);
    sem_destroy(&sc_ip);
    sem_destroy(&sc_ic);
    sem_destroy(&sc_np);
    sem_destroy(&sc_fichero);
    sem_destroy(&lista_mutex);
    sem_destroy(&mutex_salida);
    sem_destroy(&despertar_facturador);

    // liberamos memoria
    free(buffer);
    free(idProveedor);
    free(idConsumidor);
    free(proveedores);
    free(consumidores);

    Nodo *nodo = p;
    while(nodo!=NULL) {
        Nodo* aux=nodo;
        nodo = nodo->siguiente;
        free(aux);
    }

    return 0;
}


/*
Hilo productor. Se encarga de abrir y procesar los ficheros de entrada de productos. Ademas, de los productos
 * que lea, solo procesara los productos validos, es decir, aquellos cuyos caracteres estan entre 'a' y 'j'.
 * Procesara los productos, añadiendolos a un bufer circular, junto con su identificador para que los consumidores
 * puedan tener la informacion de quien fue el proveedor de cada producto
*/
void* proveedor(void* arg) {

    int numLeidos=0;
    int leidosValidos=0;
    int numTipos[MAX_PRODUCTOS];
    int il;
    int idProveedor = *((int*)arg);
    FILE* fichero;
    char nombre[100];
    int c;
    int i;

    if(path[strlen(path)-1]!='/'){
        sprintf(nombre,"%s/proveedor%i.dat",path,idProveedor);
    }
    else{
        sprintf(nombre,"%sproveedor%i.dat",path,idProveedor);
    }

    // abrimos el fichero de entrada para leer los productos
    if((fichero = fopen(nombre, "r"))==NULL){
        fprintf(stderr,"Error: fichero de entrada no existe o no se puede leer.\n");
        exit(-1);
    }

    // leemos caracter a caracter hasta llegar al final del archivo
    while ((c = fgetc(fichero)) != EOF) {
        // si es un producto valido lo procesamos
        if ('a' <= c && c <= 'j') {
            sem_wait(&hay_hueco);
            sem_wait(&sc_ip);
	        il = ip;
            ip = (ip + 1) % T;
            sem_post(&sc_ip);

            // guardamos el producto en el buffer
            buffer[il].c = c;
            buffer[il].nproveedor = idProveedor;
            sem_post(&hay_dato);

            numTipos[c-'a']++;
            leidosValidos++;
        }
        numLeidos++;
    }

    fclose(fichero);
    sem_wait(&sc_np);
    np--;

    if(np==0){ // en caso de ser el ultimo proveedor, insertamos un caracter especial que represente el final
	    sem_wait(&hay_hueco);
        sem_wait(&sc_ip);
	    il = ip;
        ip = (ip + 1) % T;
        sem_post(&sc_ip);
        buffer[il].c='F';
        buffer[il].nproveedor=idProveedor;
        sem_post(&hay_dato);
    }

    sem_post(&sc_np);

    // escribimos en el fichero de salida los datos correspondientes a los proveedores y los productos
    sem_wait(&mutex_salida);

    fprintf(salida,"\nProveedor: %i\n",idProveedor);
    fprintf(salida,"Productos procesados: %i\n",numLeidos);
    fprintf(salida,"Productos invalidos: %i\n",(numLeidos-leidosValidos));
    fprintf(salida,"Productos validos: %i. De los cuales se han insertado:\n",leidosValidos);

    for(i=0;i<10;i++){
        fprintf(salida,"%i de tipo %c.\n",numTipos[i],'a'+i);
    }
    sem_post(&mutex_salida);

    pthread_exit(NULL); // finalizacion del hilo
}

/*
Hilo consumidor. Se encarga de consumir los productos almacenados en el bufer circular. Cada vez que se consume
un producto, el hilo consumidor aumenta el contador. Una vez que se consumen todos los productos que tiene a su
disposicion de todos los proveedores termina y guarda la informacion obtenida en una lista enlazada. La informacion
guardada consta de: numero de productos consumidos, numero de productos consumidos de cada tipo y de que proveedor
provenian.
*/
void* consumidor(void* arg) {

    int sigue, il;
    int *idConsumidor = ((int*)arg);
    Nodo *l;

    if((l = (Nodo*)calloc(1, sizeof(Nodo)))==NULL){
        fprintf(stderr,"Error al asignar memoria.\n");
        exit(-1);
    }

    l->siguiente=NULL;
    sigue=1;
    while(sigue) {
        sem_wait(&hay_dato);
        sem_wait(&sc_ic);

        if(buffer[ic].c=='F'){  // si el caracter es F, significa que ya no hay mas productos por consumir
            sem_post(&sc_ic);
            sem_post(&hay_dato);
            sigue=0;
        }
        else {
            il = ic;
            ic = (ic + 1) % T;
            sem_post(&sc_ic);
            sem_post(&hay_hueco);
            // consumimos el dato
            l->contadorProveedores[buffer[il].nproveedor]++;
            l->contadorProductos[buffer[il].c - 'a']++;
            l->totalProductos++;
            l->id=*idConsumidor;

        }
    }

    sem_wait(&lista_mutex);

    if (f == NULL) {
        p = l;
        f = p;
    } else {
        f->siguiente = l;
        f = l;
    }

    sem_post(&lista_mutex);
    sem_post(&despertar_facturador); // inciamos el facturador

    pthread_exit(NULL); // finalizacion del hilo

}

/*
Hilo facturador. Su funcion es recoger la informacion de los resultados de los clientes consumidores que se
encuentra en la lista enlazada. Añade al fichero de salida los datos de cada cliente consumidor y mostrara cuantos
productos ha consumido cada cliente consumidor asi como el identificador del consumidor. Tambien indica el numero
de productos validos que proceso cada proveedor asi como la suma total de los productos consumidos, los productos
consumidos de cada proveedor y el consumidor que mas productos ha consumido
*/
void* facturador(void* arg) {
    int total;
    int i,j;
    int maxConsumidor;
    int maximo;
    Nodo *nodo;
    int *totalProveedores;

    maxConsumidor=-1;
    maximo=-1;
    total=0;
    if((totalProveedores=(int *)calloc(P,sizeof(int)))==NULL){
        fprintf(stderr,"Error al reservar memoria.\n");
        exit(-1);
    }

    for (i = 0; i < C; i++) {
        sem_wait(&despertar_facturador);

        // hacemos las operaciones correspondientes para obtener los datos que tenemos que escribir como resultado
        if (i==0){
            nodo=p;
        }
        else{
            nodo= nodo->siguiente;
        }

        if(nodo->totalProductos>maximo){
            maximo=nodo->totalProductos;
            maxConsumidor=nodo->id;
        }

        total+=nodo->totalProductos;

        for(j=0;j<P;j++){
            totalProveedores[j] += nodo->contadorProveedores[j];
        }

        // escribimos los resultados en el fichero de salida
        sem_wait(&mutex_salida);
        fprintf(salida, "Cliente consumidor %d:\n", nodo->id);
        fprintf(salida, "Productos consumidos: %d. De los cuales:\n",nodo->totalProductos);
        for(j=0;j<MAX_PRODUCTOS;j++){
            fprintf(salida, "Producto tipo '%c': %d\n",('a'+j),nodo->contadorProductos[j]);
        }
        sem_post(&mutex_salida);
    }

    // escribimos los resultados en el fichero de salida
    sem_wait(&mutex_salida);
    fprintf(salida, "Total de productos consumidos: %d\n",total);
    for(j=0;j<P;j++){
        if(totalProveedores[j]>0){
            fprintf(salida,"%d del proveedor %d.\n",totalProveedores[j],j);
        }
    }
    fprintf(salida, "Cliente consumidor que mas ha consumido: %d\n",maxConsumidor);
    sem_post(&mutex_salida);

    free(totalProveedores); // liberamos memoria

    pthread_exit(NULL); // finaliza el hilo

}
