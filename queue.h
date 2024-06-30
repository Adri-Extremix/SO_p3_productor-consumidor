#ifndef HEADER_FILE
#define HEADER_FILE


struct element {
	// Define the struct yourself
	char tipo[20];
	int numero_cuenta;
	int dinero;
	int cuenta_a_traspasar;
};

typedef struct queue {
	// Define the struct yourself
	// inicio es el nodo con algun elemento
	int inicio;
	// fin es el primer nodo sin elemento
	int fin;
	int size;
	struct element** array_elements;
	// last_acc es 1 si la última acción fue insertar y es 0 si la ultima accion fue extraer
	int last_acc;
	
	
}queue;

queue* queue_init (int size);
int queue_destroy (queue *q);
int queue_put (queue *q, struct element* elem);
struct element * queue_get(queue *q);
int queue_empty (queue *q);
int queue_full(queue *q);

#endif
