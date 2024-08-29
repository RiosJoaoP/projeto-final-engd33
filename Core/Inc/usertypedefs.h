#include "main.h"
#include "FreeRTOS.h"


#define TAMANHO_BUFFER 40

typedef struct {
	float x;
    float y;
    float z;
    TickType_t timestamp;
} dataset;
