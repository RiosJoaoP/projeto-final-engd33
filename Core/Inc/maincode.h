#ifndef MAINCODE_H
#define MAINCODE_H

/* ===========================INCLUDES============================ */

#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "main.h"
#include "ILI9341_GFX.h"
#include "ILI9341_STM32_Driver.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "usertypedefs.h"
#include "lookuptable.h"

/* ================DEFINES E VARIÁVEIS DE CONTROLE================ */

#define TELA1 1
#define TELA2 2
#define TELA3 3

#define REFRESH_TELA 699

extern uint16_t sTelaAtual;

/* =============ESTRUTURAS DE ARMAZENAMENTO DE DADOS============== */

extern dataset xVelLinearAtual;
extern dataset xPosicaoAtual;
extern dataset xVelAngularAtual;
extern dataset xCorrenteAtual;

/* ===========================HANDLERS============================ */

extern TaskHandle_t xHandlerDisplayManager;

extern QueueHandle_t xQueueCorrente;
extern QueueHandle_t xQueueVelAngular;
extern QueueHandle_t xQueuePosicao;

extern SemaphoreHandle_t xMutexCorrenteAtual;
extern SemaphoreHandle_t xMutexVelLinearAtual;
extern SemaphoreHandle_t xMutexVelAngularAtual;
extern SemaphoreHandle_t xMutexPosicaoAtual;

/* ==========================PROTÓTIPOS========================== */

void userRTOS(void);
void vDisplayManager(void*);
void vTaskGerarQueueCorrente(void*);
void vTaskGerarQueueVelAngular(void*);
void vTaskGerarQueuePosicao(void*);
void vTaskQueueCorrenteReader(void*);
void vTaskQueueVelAngularReader(void*);
void vTaskQueuePosicaoReader(void*);
void inicializar(void);
void funcScaleXGraph(TickType_t, TickType_t);
void funcBaseTela1(void);
void funcBaseTela2(void);
void funcBaseTela3(void);
void baseTela(uint16_t);
void funcDadosTela1(void);
void funcDadosTela2(void);
void funcDadosTela3(void);
void dadosTela(uint16_t);

#endif /* MAINCODE_H */
