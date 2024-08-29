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

uint16_t sTelaAtual = TELA1;

/* =============ESTRUTURAS DE ARMAZENAMENTO DE DADOS============== */

dataset xVelLinearAtual;
dataset xPosicaoAtual;
dataset xVelAngularAtual;
dataset xCorrenteAtual;

/* ===========================HANDLERS============================ */

TaskHandle_t xHandlerDisplayManager = NULL;

QueueHandle_t xQueueCorrente = NULL;
QueueHandle_t xQueueVelAngular = NULL;
QueueHandle_t xQueuePosicao = NULL;

SemaphoreHandle_t xMutexCorrenteAtual = NULL;
SemaphoreHandle_t xMutexVelLinearAtual = NULL;
SemaphoreHandle_t xMutexVelAngularAtual = NULL;
SemaphoreHandle_t xMutexPosicaoAtual = NULL;


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


/* =====================INICIALIZAÇÃO DO RTOS===================== */

// Função de inicialização
void userRTOS(void){

	xMutexVelLinearAtual = xSemaphoreCreateMutex();
	xMutexVelAngularAtual = xSemaphoreCreateMutex();
	xMutexPosicaoAtual = xSemaphoreCreateMutex();

	xQueueCorrente = xQueueCreate(50, sizeof(dataset));
	xQueueVelAngular = xQueueCreate(30, sizeof(dataset));
	xQueuePosicao = xQueueCreate(5, sizeof(dataset));

	xTaskCreate(vDisplayManager,
				"displayManager",
				2048,
				(void*) 0,
				1,
				&xHandlerDisplayManager);

	xTaskCreate(vTaskGerarQueueCorrente,
				"gerarQueueCorrente",
				128,
				(void*) 0,
				5,
				NULL);

	xTaskCreate(vTaskGerarQueueVelAngular,
				"gerarQueueVelAngular",
				128,
				(void*) 0,
				4,
				NULL);

	xTaskCreate(vTaskGerarQueuePosicao,
				"gerarQueuePosicao",
				128,
				(void*) 0,
				3,
				NULL);

	xTaskCreate(vTaskQueueCorrenteReader,
				"queueCorrenteReader",
				256,
				(void*) 0,
				2,
				NULL);

	xTaskCreate(vTaskQueueVelAngularReader,
				"queueVelAngularReader",
				256,
				(void*) 0,
				2,
				NULL);

	xTaskCreate(vTaskQueuePosicaoReader,
				"queuePosicaoReader",
				256,
				(void*) 0,
				2,
				NULL);

	vTaskStartScheduler();

    while(1);
}


/* =========================TASKS DO RTOS========================= */

// Gerenciamento da tela
void vDisplayManager(void *p){
	TickType_t xLastWakeTime;;
	uint16_t sIndice = 0;
	while(1){
		xLastWakeTime = xTaskGetTickCount();
		if(sIndice >= 9){
			switch(sTelaAtual) {
    		    case TELA1:
    		        sTelaAtual = TELA2;
    		        break;
    		    case TELA2:
    		        sTelaAtual = TELA3;
    		        break;
    		    case TELA3:
    		       	sTelaAtual = TELA1;
    		        break;
    		    default:
    		        break;
    		}
			baseTela(sTelaAtual);
			sIndice = 0;
		}else{
			sIndice++;
		}
		dadosTela(sTelaAtual);
		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(REFRESH_TELA));
	}
}

// Geração de dados de corrente e envio para queue
void vTaskGerarQueueCorrente(void *p) {
	TickType_t xLastWakeTime;
	dataset correntes;
	uint16_t sIndice = 0;
	while(1) {
		xLastWakeTime = xTaskGetTickCount();
		correntes.x = vetorCorrenteX[sIndice];
		correntes.y = vetorCorrenteY[sIndice];
		correntes.z = vetorCorrenteZ[sIndice];
		correntes.timestamp = xLastWakeTime;
		if(sIndice >= LENGTH_LUT - 1){
			sIndice = 0;
		}else{
			sIndice++;
		}
		if(xQueueSendToBack(xQueueCorrente, &correntes, 0) == errQUEUE_FULL){
			HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, 0);
		}
		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1));
	}
}

// Geração de dados de velocidade angular e envio para queue
void vTaskGerarQueueVelAngular(void *p) {
	TickType_t xLastWakeTime;
	dataset velAngular;
	uint16_t sIndice = 0;
	while(1) {
		xLastWakeTime = xTaskGetTickCount();
		velAngular.x = vetorVelAngX[sIndice];
		velAngular.y = vetorVelAngY[sIndice];
		velAngular.z = vetorVelAngZ[sIndice];
		velAngular.timestamp = xLastWakeTime;
		if(sIndice >= LENGTH_LUT - 1){
			sIndice = 0;
		}else{
			sIndice++;
		}
		if(xQueueSendToBack(xQueueVelAngular, &velAngular, 0) == errQUEUE_FULL){
			HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, 0);
		}
		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(10));
	}
}

// Geração de dados de GPS e envio para queue
void vTaskGerarQueuePosicao(void *p) {
	TickType_t xLastWakeTime;
	dataset posicao;
	uint16_t sIndice = 0;
	while(1) {
		xLastWakeTime = xTaskGetTickCount();
		posicao.x = vetorPosicaoX[sIndice];
		posicao.y = vetorPosicaoY[sIndice];
		posicao.z = vetorPosicaoZ[sIndice];
		posicao.timestamp = xLastWakeTime;
		if(sIndice >= LENGTH_LUT - 1){
			sIndice = 0;
		}else{
			sIndice++;
		}
		if(xQueueSendToBack(xQueuePosicao, &posicao, 0) == errQUEUE_FULL){
			HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, 0);
		}
		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(100));
	}
}

// Leitura de dados de corrente da queue
void vTaskQueueCorrenteReader(void *p) {
	const TickType_t xMaxMutexDelay = pdMS_TO_TICKS(1);
	TickType_t xLastWakeTime;
	dataset corrente;
	while (1) {
		xLastWakeTime = xTaskGetTickCount();
		while(xQueueReceive(xQueueCorrente, &corrente, 0) != errQUEUE_EMPTY){
			if(xSemaphoreTake(xMutexCorrenteAtual, xMaxMutexDelay) == pdPASS){
				xCorrenteAtual = corrente;
				xSemaphoreGive(xMutexCorrenteAtual);
			}
		}
		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(125));
	}
}

// Leitura de dados de velocidade angular da queue
void vTaskQueueVelAngularReader(void *p) {
	const TickType_t xMaxMutexDelay = pdMS_TO_TICKS(1);
	const uint16_t L_cm = 20;
	const uint16_t r_cm = 15;
	const float alpha1 = 0;
	const float alpha2 = 2*M_PI/3;
	const float alpha3 = 4*M_PI/3;
	const float sin_alpha1 = sin(alpha1);
	const float sin_alpha2 = sin(alpha2);
	const float sin_alpha3 = sin(alpha3);
	const float cos_alpha1 = cos(alpha1);
	const float cos_alpha2 = cos(alpha2);
	const float cos_alpha3 = cos(alpha3);
	TickType_t xLastWakeTime;
	float linearVX, linearVY, linearW;
	dataset velAngular, velLinear;
	while (1) {
		xLastWakeTime = xTaskGetTickCount();

		while(xQueueReceive(xQueueVelAngular, &velAngular, 0) != errQUEUE_EMPTY) {
			linearVX = r_cm*(2.0/3.0)*(-(sin_alpha1*velAngular.x)-(sin_alpha2*velAngular.y)-(sin_alpha3*velAngular.z));
			linearVY = r_cm*(2.0/3.0)*((cos_alpha1*velAngular.x)+(cos_alpha2*velAngular.y)+(cos_alpha3*velAngular.z));
			linearW = (r_cm*(velAngular.x+velAngular.y+velAngular.z))/(3*L_cm);
			velLinear.x = linearVX;
			velLinear.y = linearVY;
			velLinear.z = linearW;

			if(xSemaphoreTake(xMutexVelLinearAtual, xMaxMutexDelay) == pdPASS){
				xVelLinearAtual = velLinear;
				xSemaphoreGive(xMutexVelLinearAtual);
			}

			if(xSemaphoreTake(xMutexVelAngularAtual, xMaxMutexDelay) == pdPASS){
				xVelAngularAtual = velAngular;
				xSemaphoreGive(xMutexVelAngularAtual);
			}
		}

		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(100));
	}
}

// Leitura de dados de GPS da queue
void vTaskQueuePosicaoReader(void *p) {
	const TickType_t xMaxMutexDelay = pdMS_TO_TICKS(1);
	TickType_t xLastWakeTime;
	dataset posicao;
	while (1) {
		xLastWakeTime = xTaskGetTickCount();
		while(xQueueReceive(xQueuePosicao, &posicao, 0) != errQUEUE_EMPTY){
			if(xSemaphoreTake(xMutexPosicaoAtual, xMaxMutexDelay) == pdPASS){
				xPosicaoAtual = posicao;
				xSemaphoreGive(xMutexPosicaoAtual);
			}
		}
		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(125));
	}
}

/* ======================FUNÇÕES AUXILIARES======================= */

// Inicialização da tela executada antes da inicialização do RTOS
void inicializar(void){
	ILI9341_Init();
	ILI9341_SetRotation(SCREEN_HORIZONTAL_2);
	baseTela(sTelaAtual);
}

// Geração de escala do eixo X
void funcScaleXGraph(TickType_t tempoInicio, TickType_t tempoFim){
	float tmp_value = 0;
	char textBuffer[20];

	for(int i = 0; i < 5; i++){
		tmp_value = (tempoInicio+((tempoFim - tempoInicio)*i/4.0))/1000.0;
		sprintf(textBuffer, "%.2f  ", tmp_value);
		ILI9341_DrawText(textBuffer, FONT1, 31+(i*59), 202, WHITE, BLACK);
	}
}

// Base da Tela 1
void funcBaseTela1(void){
	ILI9341_DrawText("Vel. Linear e Posicao", FONT4, 25, 11, WHITE, NAVY);
	ILI9341_DrawText("Vel. X (cm/s):", FONT3, 25, 60, LIGHTBLUE, BLACK);
	ILI9341_DrawText("Vel. Y (cm/s)", FONT3, 25, 120, MAGENTA, BLACK);
	ILI9341_DrawText("Pos. X (cm):", FONT3, 165, 60, GREEN, BLACK);
	ILI9341_DrawText("Pos. Y (cm):", FONT3, 165, 120, DARKORANGE, BLACK);
}

// Base da Tela 2
void funcBaseTela2(void){
	ILI9341_DrawText("Velocidade Angular", FONT4, 25, 11, WHITE, NAVY);
	ILI9341_DrawText("Motor 1:", FONT3, 25, 60, LIGHTBLUE, BLACK);
	ILI9341_DrawText("Motor 2:", FONT3, 25, 120, MAGENTA, BLACK);
	ILI9341_DrawText("Motor 3:", FONT3, 165, 60, GREEN, BLACK);
	ILI9341_DrawText("W:", FONT3, 165, 120, DARKORANGE, BLACK);
}

// Base da Tela 3
void funcBaseTela3(void){
	ILI9341_DrawText("Velocidade Angular", FONT4, 25, 11, WHITE, NAVY);
	ILI9341_DrawText("Motor 1:", FONT3, 25, 60, LIGHTBLUE, BLACK);
	ILI9341_DrawText("Motor 2:", FONT3, 25, 120, MAGENTA, BLACK);
	ILI9341_DrawText("Motor 3:", FONT3, 165, 60, GREEN, BLACK);
}

// Seleção de base de tela
void baseTela(uint16_t sNumTela){
	ILI9341_DrawRectangle(0, 36, 320, 204, BLACK);
	ILI9341_DrawRectangle(0, 0, 320, 36, NAVY);
	switch(sNumTela){
		case TELA1:
			funcBaseTela1();
			break;
		case TELA2:
			funcBaseTela2();
			break;
		case TELA3:
			funcBaseTela3();
			break;
		default:
			break;
	}
}

// Exibição de valores da tela 1
void funcDadosTela1(void){
	const TickType_t xMaxMutexDelay = pdMS_TO_TICKS(1);
	dataset velLinear;
	dataset posicao;
	char textBuffer[20];

	if(xSemaphoreTake(xMutexVelLinearAtual, xMaxMutexDelay) == pdPASS){
		velLinear = xVelLinearAtual;
		xSemaphoreGive(xMutexVelLinearAtual);
	}
	if(xSemaphoreTake(xMutexPosicaoAtual, xMaxMutexDelay) == pdPASS){
		posicao = xPosicaoAtual;
		xSemaphoreGive(xMutexPosicaoAtual);
	}

	sprintf(textBuffer, "%.1f    ", velLinear.x);
	ILI9341_DrawText(textBuffer, FONT4, 25, 80, LIGHTBLUE, BLACK);
	sprintf(textBuffer, "%.1f    ", velLinear.y);
	ILI9341_DrawText(textBuffer, FONT4, 25, 140, MAGENTA, BLACK);

	sprintf(textBuffer, "%.2f    ", posicao.x);
	ILI9341_DrawText(textBuffer, FONT4, 165, 80, GREEN, BLACK);
	sprintf(textBuffer, "%.2f    ", posicao.y);
	ILI9341_DrawText(textBuffer, FONT4, 165, 140, DARKORANGE, BLACK);
}

// Exibição do gráfico da tela 2

void funcDadosTela2(void){
	const TickType_t xMaxMutexDelay = pdMS_TO_TICKS(1);
	dataset velAngular;
	dataset velLinear;
	char textBuffer[20];

	if(xSemaphoreTake(xMutexVelLinearAtual, xMaxMutexDelay) == pdPASS){
		velLinear = xVelLinearAtual;
		xSemaphoreGive(xMutexVelLinearAtual);
	}
	if(xSemaphoreTake(xMutexVelAngularAtual, xMaxMutexDelay) == pdPASS){
		velAngular = xVelAngularAtual;
		xSemaphoreGive(xMutexVelAngularAtual);
	}

	sprintf(textBuffer, "%.1f    ", velAngular.x);
	ILI9341_DrawText(textBuffer, FONT4, 25, 80, LIGHTBLUE, BLACK);
	sprintf(textBuffer, "%.1f    ", velAngular.y);
	ILI9341_DrawText(textBuffer, FONT4, 25, 140, MAGENTA, BLACK);
	sprintf(textBuffer, "%.1f    ", velAngular.z);
	ILI9341_DrawText(textBuffer, FONT4, 165, 80, YELLOW, BLACK);
	sprintf(textBuffer, "%.2f    ", velLinear.z);
	ILI9341_DrawText(textBuffer, FONT4, 165, 140, GREEN, BLACK);

}

// Exibição do gráfico da tela 3
void funcDadosTela3(void){
	const TickType_t xMaxMutexDelay = pdMS_TO_TICKS(1);
	dataset corrente;
	char textBuffer[20];

	if(xSemaphoreTake(xMutexCorrenteAtual, xMaxMutexDelay) == pdPASS){
		corrente = xCorrenteAtual;
		xSemaphoreGive(xMutexCorrenteAtual);
	}

	sprintf(textBuffer, "%.1f    ", corrente.x);
	ILI9341_DrawText(textBuffer, FONT4, 25, 80, LIGHTBLUE, BLACK);
	sprintf(textBuffer, "%.1f    ", corrente.y);
	ILI9341_DrawText(textBuffer, FONT4, 25, 140, MAGENTA, BLACK);
	sprintf(textBuffer, "%.1f    ", corrente.z);
	ILI9341_DrawText(textBuffer, FONT4, 165, 80, YELLOW, BLACK);
}

// Exibição de dados na tela
void dadosTela(uint16_t sNumTela){
	switch(sNumTela){
		case TELA1:
			funcDadosTela1();
			break;
		case TELA2:
			funcDadosTela2();
			break;
		case TELA3:
			funcDadosTela3();
			break;
		default:
			break;
	}
}
