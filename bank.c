//SSOO-P3 2022-2023

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stddef.h>
#include <sys/stat.h>
#include <pthread.h>
#include "queue.h"
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>

/**
 * Entry point
 * @param argc
 * @param argv
 * @return
 */





int client_numop = 0;
int bank_numop = 0;
int global_balance = 0;
int * saldo_cuenta;

pthread_mutex_t m;
pthread_cond_t no_lleno;
pthread_cond_t no_vacio;
queue* q;
struct element* list_client_ops;
int max_operaciones;
int max_cuentas;

int funcion_cajero()
{
    
    while(1)
    {
        if(pthread_mutex_lock(&m) != 0)
        {
            perror("Error en el lock");
            return -1;
        }
        
        // Mientras el buffer esté lleno esperan
        while(queue_full(q))
        {
            if(pthread_cond_wait(&no_lleno,&m) != 0)
            {
                perror("Error en el wait");
                return -1;
            }
        }
        // Sí los hilos han terminado con el array terminan
        if (client_numop >= max_operaciones)
        {
            if(pthread_cond_broadcast(&no_vacio) != 0)
            {
                perror("Error en el broadcast");
                return -1;
            }
            if(pthread_mutex_unlock(&m) != 0)
            {
                perror("Error en el unlock");
                return -1;
            }
            pthread_exit(0);
        }
        
        queue_put(q, &list_client_ops[client_numop]);
        client_numop ++;
        // Cuando metan un elemento en el buffer despierta a los
        if(pthread_cond_signal(&no_vacio) != 0)
        {
            perror("Error en el signal");
            return -1;
        }
        
        if(pthread_mutex_unlock(&m) != 0)
        {
            perror("Error en el unlock");
            return -1;
        }
    }
    return -1;
    
}

int Crear(struct element* operacion)
{
    // Si el numero de cuenta está dentro del rango de cuentas se realiza la creación
    if(0<operacion->numero_cuenta && operacion->numero_cuenta<=max_cuentas)
    {
        saldo_cuenta[operacion->numero_cuenta] = 0;
        printf("%i %s %i SALDO=%i TOTAL=%i\n",bank_numop,operacion->tipo,operacion->numero_cuenta,saldo_cuenta[operacion->numero_cuenta],global_balance);
        return 1;
    }
    else
    {
        perror("Esa cuenta excede el número máximo de cuentas");
        return -1;
    }
}

void Ingresar(struct element* operacion)
{
    // Se cambia el dinero de la cuenta y del banco
    saldo_cuenta[operacion->numero_cuenta] = saldo_cuenta[operacion->numero_cuenta] + operacion->dinero;
    global_balance = global_balance + operacion->dinero;
    printf("%i %s %i %i SALDO=%i TOTAL=%i\n",bank_numop,operacion->tipo,operacion->numero_cuenta,operacion->dinero,saldo_cuenta[operacion->numero_cuenta],global_balance);
}

void Retirar(struct element* operacion)
{
    // Se cambia el dinero de la cuenta y del banco
    saldo_cuenta[operacion->numero_cuenta] = saldo_cuenta[operacion->numero_cuenta] - operacion->dinero;
    global_balance = global_balance - operacion->dinero;
    printf("%i %s %i %i SALDO=%i TOTAL=%i\n",bank_numop,operacion->tipo,operacion->numero_cuenta,operacion->dinero,saldo_cuenta[operacion->numero_cuenta],global_balance);
}

void Saldo(struct element* operacion)
{   
    // Se imprime el dinero de la cuenta
    printf("%i %s %i SALDO=%i TOTAL=%i\n",bank_numop,operacion->tipo,operacion->numero_cuenta,saldo_cuenta[operacion->numero_cuenta],global_balance);
}

void Traspasar(struct element* operacion)
{
    // Se cambia el dinero de la cuenta que recibe y la que da el dinero  
    saldo_cuenta[operacion->numero_cuenta] = saldo_cuenta[operacion->numero_cuenta] - operacion->dinero;
    saldo_cuenta[operacion->cuenta_a_traspasar] = saldo_cuenta[operacion->cuenta_a_traspasar] + operacion->dinero;
printf("%i %s %i %i %i SALDO=%i TOTAL=%i\n",bank_numop,operacion->tipo,operacion->numero_cuenta,operacion->cuenta_a_traspasar,operacion->dinero,saldo_cuenta[operacion->cuenta_a_traspasar],global_balance);
}

int  funcion_trabajador()
{
    struct element* operacion;
    // Bucle infinito hasta que terminan de procesar todas las operaciones
    while(1)
    {   
        // Para ocuparnos de la concurrencia hacemos un lock del mutex
        // para proteger el buffer y las variables globales
        if(pthread_mutex_lock(&m) != 0)
        {
            perror("Error en el lock");
            return -1;
        }
        
        if (bank_numop >= max_operaciones)
        // Si el contador de los trabajadores es igual o supera el máximo de operaciones
        // los trabajadores comienzan a morirse y despiertan a los cajeros
        {
            if(pthread_cond_broadcast(&no_lleno) != 0)
            {
                perror("Error en el broadcast");
                return -1;
            }
            if(pthread_mutex_unlock(&m) != 0)
            {
                perror("Error en el unlock");
                return -1;
            }
            pthread_exit(0);
        }
        while(queue_empty(q))
        // Si está el buffer vacío el trabajador se queda dormido
        {
            if(pthread_cond_wait(&no_vacio,&m) != 0)
            {
                perror("Error en el wait");
                return -1;
            }
            if (bank_numop >= max_operaciones)
            {
                if(pthread_mutex_unlock(&m) != 0)
                {
                    perror("Error en el unlock");
                    return -1;
                }
                pthread_exit(0);
            }
            
        }
        
        bank_numop ++;
        operacion = queue_get(q);

        // Tras sacar la operación, la clasificamos y según la clasificación se procesa
        if (strcmp(operacion->tipo, "CREAR") == 0)
        {
            Crear(operacion);
        }
        
        else if (strcmp(operacion->tipo, "INGRESAR") == 0)
        {
            Ingresar(operacion);
        }
        
        else if (strcmp(operacion->tipo, "RETIRAR") == 0)
        {
            Retirar(operacion);
        }
        
        else if (strcmp(operacion->tipo, "SALDO") == 0)
        {
            Saldo(operacion);
        }
        else if (strcmp(operacion->tipo, "TRASPASAR") == 0)
        {
            Traspasar(operacion);
        }
        // Tras procesar liberamos la operacion
        free((void*)operacion);
        // Como el hilo ha quitado un elemento del buffer despierta a algún trabajador
        if(pthread_cond_signal(&no_lleno) != 0)
        {
            perror("Error en el signal");
            return -1;
        }
        if(pthread_mutex_unlock(&m) != 0)
        {
            perror("Error en el unlock");
            return -1;
        }
    }
    return -1; 
}


int main (int argc, const char * argv[] ) {
    if (argc != 6)
    {
        perror("Argumentos insuficientes");
        return -1;
    }
    
    else
    {
        int valores_incorrecto = 0;
        // Comprobación de digitos
        for (int i = 2;i <=5; i++)
        {   
            //Si es negativo
            if (argv[i][0] == '-')
            {
                valores_incorrecto = 1;    
            }
        }
        if (valores_incorrecto == 1)
        {
            perror("Argumentos no válidos");
            return -1;
        }
        else
        {
            FILE * archivo = fopen(argv[1],"r");
            char tipo_operacion[20];
            // Lo primero es leer el numero maximo de operaiones
            fscanf(archivo,"%d",&max_operaciones);
            if (max_operaciones >200)
            {
                perror("Superado el número máximo");
                return -1;
            }
            list_client_ops = (struct element*)malloc(sizeof(struct element) * max_operaciones);
            
            int i = 0;
            while(fscanf(archivo,"%s",tipo_operacion) != EOF)
            // Iteramos con fscanf hasta que termine el fichero y clasificamos cada linea
            // al clasificar nos guardamos en el struct element los datos necesarios
            {
                if (strcmp(tipo_operacion,"CREAR") == 0)
                {
                    strcpy(list_client_ops[i].tipo,tipo_operacion);
                    fscanf(archivo,"%d",&list_client_ops[i].numero_cuenta);
                }
                else if (strcmp(tipo_operacion,"INGRESAR") == 0)
                {
                    strcpy(list_client_ops[i].tipo,tipo_operacion);
                    fscanf(archivo,"%d",&list_client_ops[i].numero_cuenta);
                    fscanf(archivo,"%d",&list_client_ops[i].dinero);
                    
                }
                else if(strcmp(tipo_operacion,"RETIRAR") == 0)
                {
                    strcpy(list_client_ops[i].tipo,tipo_operacion);
                    fscanf(archivo,"%d",&list_client_ops[i].numero_cuenta);
                    fscanf(archivo,"%d",&list_client_ops[i].dinero);
                }
                else if(strcmp(tipo_operacion,"SALDO") == 0)
                {
                    strcpy(list_client_ops[i].tipo,tipo_operacion);
                    fscanf(archivo,"%d",&list_client_ops[i].numero_cuenta);
                }
                else if(strcmp(tipo_operacion,"TRASPASAR") == 0)
                {
                    strcpy(list_client_ops[i].tipo,tipo_operacion); 
                    fscanf(archivo,"%d",&list_client_ops[i].numero_cuenta);
                    fscanf(archivo,"%d",&list_client_ops[i].cuenta_a_traspasar);
                    fscanf(archivo,"%d",&list_client_ops[i].dinero);
                }
                
                else
                // Si no es ninguna clasificada la operacion no es valida
                {
                    perror("Operación no válida");
                    return -1;
                }
                i ++;
            }
            if (i != max_operaciones)
            // Si al terminar hemos contado más o menos operaciones de las indicadas damos un error
            {
                perror("El número de operaciones no coincide con max_operaciones");
                return -1;
            }
            max_cuentas = atoi(argv[4]) +1;
            saldo_cuenta = (int*)malloc(sizeof(int) * max_cuentas);
            // Creamos los mutex, cond e hilos 
            pthread_t cajeros[atoi(argv[2])];
            pthread_t trabajadores[atoi(argv[3])];
            
            if(pthread_mutex_init(&m,NULL) != 0)
            {
                perror("Error en la creación del mutex");
                return -1;
            }
            if(pthread_cond_init(&no_lleno,NULL) != 0)
            {
                perror("Error en la creación del cond");
                return -1;
            }
            if(pthread_cond_init(&no_vacio,NULL) != 0)
            {
                perror("Error en la creación del cond");
                return -1;
            }
            
            int tamano_buffer = atoi(argv[5]);
            
            q = queue_init(tamano_buffer);
            
            for (int n_trab = 0; n_trab < atoi(argv[3]); n_trab++)
            {
                if(pthread_create(&trabajadores[n_trab],NULL,(void *)funcion_trabajador,NULL) != 0)
                {
                    perror("Error en la creación de un hilo");
                    return -1;
                }
            }
            
            for (int n_caj = 0; n_caj < atoi(argv[2]); n_caj++)
            {
                if(pthread_create(&cajeros[n_caj],NULL,(void *)funcion_cajero,NULL) != 0)
                {
                    perror("Error en la creación de un hilo");
                    return -1;
                }
            }
            
            
            
            
            
            for (int n_trab = 0; n_trab < atoi(argv[3]); n_trab++)
            {
                if(pthread_join(trabajadores[n_trab],NULL) != 0)
                {
                    perror("Error en la espera de un hilo");
                    return -1;
                }
            }
            
            
            
            for (int n_caj = 0; n_caj < atoi(argv[2]); n_caj++)
            {
                if(pthread_join(cajeros[n_caj],NULL) != 0)
                {
                    perror("Error en la espera de un hilo");
                    return -1;
                }
            }
            
            if(pthread_mutex_destroy(&m) != 0)
            {
                perror("Error en la destrucción del mutex");
                return -1;
            }
            if(pthread_cond_destroy(&no_lleno) != 0)
            {
                perror("Error en la destrucción del cond");
                return -1;
            }
            if(pthread_cond_destroy(&no_vacio) != 0)
            {
                perror("Error en la destrucción del cond");
                return -1;
            }
	        queue_destroy(q);
            // Liberamos los mallocs
            free((void*)list_client_ops);
            free((void*)saldo_cuenta);
            
            
            return 0;
        }
    }
}



