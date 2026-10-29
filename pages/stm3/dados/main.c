/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* ---------- Sensor ---------- */
#define QTR_TIPO_RC          1     /* 1 = QTR tipo RC (mede tempo de descarga)
                                      0 = placa com saida digital direta (comparador) */
#define LINHA_BRANCA_INICIAL 1     /* 1 = linha branca em fundo preto (sua pista)
                                      0 = linha preta em fundo branco
                                      (a tecla 0 do controle troca durante o uso) */
#define QTR_TIMEOUT_US       2500  /* tempo maximo de leitura por sensor (modo RC) */
#define QTR_LIMIAR_RUIDO     50    /* valores normalizados (0..1000) abaixo disso sao ignorados */
#define QTR_LIMIAR_LINHA     200   /* algum sensor acima disso = linha encontrada (igual a QTRSensors) */

/* ---------- Motores ---------- */
#define PWM_MAX              3599  /* = Counter Period do TIM4 */
/* Valores convertidos do codigo Arduino que funcionava (escala 0..255 -> 0..3599, x14,1) */
#define VEL_BASE_INICIAL     850   /* = 60 no Arduino (velocidadeLenta)  */
#define VEL_PASSO            140   /* = 10 no Arduino; teclas * e #     */
#define VEL_MIN_CURVA        700   /* velocidade minima ao reduzir nas curvas */
#define ERRO_CURVA           1500  /* acima desse erro a velocidade base e reduzida */
#define VEL_MANUAL           1800  /* velocidade no controle manual */
#define VEL_TESTE            1500  /* velocidade no teste de motores */
#define VEL_CALIBRA          1100  /* velocidade girando durante a calibracao */
#define INVERTE_MOTOR_A      0     /* mude para 1 se o motor A girar ao contrario */
#define INVERTE_MOTOR_B      1     /* mude para 1 se o motor B girar ao contrario */

/* ---------- Controle PD ---------- */
#define KP_X100              56    /* = Kp 0,04 do Arduino  (x14,1) */
#define KD_X100              1440  /* = Kd 1,02 do Arduino  (x14,1) */
#define KD_PASSO             70    /* = 0,05 no Arduino; teclas 7 e 9 */
#define SENTIDO_CORRECAO     1     /* troque para -1 se o robo virar para o lado errado */

/* ---------- Controle remoto IR (protocolo NEC) ---------- */
#define IR_TIMEOUT_MANUAL_MS 200   /* soltou a seta por esse tempo -> para */

/* Codigos usuais do controle de 17 teclas (confira na serial!) */
#define IR_1      0x45
#define IR_2      0x46
#define IR_3      0x47
#define IR_4      0x44
#define IR_5      0x40
#define IR_6      0x43
#define IR_7      0x07
#define IR_9      0x09
#define IR_0      0x19
#define IR_OK     0x1C
#define IR_CIMA   0x18
#define IR_BAIXO  0x52
#define IR_ESQ    0x08
#define IR_DIR    0x5A
#define IR_AST    0x16   /* tecla *  */
#define IR_CERQ   0x0D   /* tecla #  */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
typedef enum {
  MODO_PARADO = 0,
  MODO_SENSORES,
  MODO_TESTE_MOTOR,
  MODO_SEGUIR,
  MODO_CALIBRAR,
  MODO_MANUAL,
  MODO_FRENTE,
  MODO_RE
} Modo;

static const char *nome_modo[] = {
  "PARADO", "SENSORES", "TESTE MOTORES", "SEGUIR LINHA", "CALIBRAR", "MANUAL",
  "FRENTE", "RE"
};

typedef struct { GPIO_TypeDef *porta; uint8_t pino; } PinoQTR;

/* sensor 1 (PA2) ... sensor 8 (PB1) */
static const PinoQTR qtr[8] = {
  {GPIOA, 2}, {GPIOA, 3}, {GPIOA, 4}, {GPIOA, 5},
  {GPIOA, 6}, {GPIOA, 7}, {GPIOB, 0}, {GPIOB, 1}
};

static uint16_t qtr_bruto[8];
static uint16_t qtr_norm[8];
static uint16_t qtr_min[8];
static uint16_t qtr_max[8];
static uint8_t  calibrado  = 0;
static int32_t  ultima_pos = 3500;

static Modo     modo       = MODO_PARADO;
static uint32_t modo_t0    = 0;
static uint8_t  passo      = 0;
static int32_t  vel_base   = VEL_BASE_INICIAL;
static int32_t  erro_ant   = 0;
static int32_t  manual_a   = 0;
static int32_t  manual_b   = 0;
static uint8_t  linha_branca   = LINHA_BRANCA_INICIAL;
static int32_t  kp_x100        = KP_X100;
static int32_t  kd_x100        = KD_X100;
static uint8_t  primeiro_ciclo = 1;

/* Variaveis do IR (alteradas dentro da interrupcao) */
static volatile uint32_t ir_ult_borda   = 0;   /* em ciclos de clock */
static volatile uint32_t ir_dados       = 0;
static volatile uint8_t  ir_bits        = 0;
static volatile uint8_t  ir_recebendo   = 0;
static volatile uint8_t  ir_novo        = 0;
static volatile uint8_t  ir_cmd         = 0;
static volatile uint8_t  ir_end         = 0;
static volatile uint8_t  ir_teve_codigo = 0;
static volatile uint32_t ir_ult_ms      = 0;   /* ultimo codigo ou repeticao */

static uint32_t ciclos_por_us = 72;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI2_Init(void);
static void MX_TIM4_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* ---------- printf pela USART1 ---------- */
int _write(int file, char *ptr, int len)
{
  (void)file;
  HAL_UART_Transmit(&huart1, (uint8_t *)ptr, (uint16_t)len, HAL_MAX_DELAY);
  return len;
}

/* ---------- Tempo em microssegundos usando o contador de ciclos (DWT) ---------- */
static void dwt_iniciar(void)
{
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;
  ciclos_por_us = SystemCoreClock / 1000000U;
}

static inline uint32_t us_desde(uint32_t ciclo_inicio)
{
  return (DWT->CYCCNT - ciclo_inicio) / ciclos_por_us;
}

static void atraso_us(uint32_t us)
{
  uint32_t ini = DWT->CYCCNT;
  uint32_t n   = us * ciclos_por_us;
  while ((DWT->CYCCNT - ini) < n) { }
}

/* ---------- Motores (TB6612FNG) ---------- */
static void motor_um(int32_t v,
                     GPIO_TypeDef *p1, uint16_t in1,
                     GPIO_TypeDef *p2, uint16_t in2,
                     uint32_t canal)
{
  if (v >  PWM_MAX) v =  PWM_MAX;
  if (v < -PWM_MAX) v = -PWM_MAX;

  if (v > 0) {
    HAL_GPIO_WritePin(p1, in1, GPIO_PIN_SET);
    HAL_GPIO_WritePin(p2, in2, GPIO_PIN_RESET);
  } else if (v < 0) {
    HAL_GPIO_WritePin(p1, in1, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(p2, in2, GPIO_PIN_SET);
    v = -v;
  } else {
    /* freio (IN1 = IN2 = 1) */
    HAL_GPIO_WritePin(p1, in1, GPIO_PIN_SET);
    HAL_GPIO_WritePin(p2, in2, GPIO_PIN_SET);
  }
  __HAL_TIM_SET_COMPARE(&htim4, canal, (uint32_t)v);
}

/* a = motor A (esquerdo), b = motor B (direito). Faixa: -PWM_MAX .. +PWM_MAX */
static void motores(int32_t a, int32_t b)
{
#if INVERTE_MOTOR_A
  a = -a;
#endif
#if INVERTE_MOTOR_B
  b = -b;
#endif
  motor_um(a, ain1_GPIO_Port, ain1_Pin, ain2_GPIO_Port, ain2_Pin, TIM_CHANNEL_1);
  motor_um(b, bin1_GPIO_Port, bin1_Pin, bin2_GPIO_Port, bin2_Pin, TIM_CHANNEL_2);
}

/* ---------- Sensor QTR-8 ---------- */
/* PA2..PA7 e PB0..PB1 ficam no registrador CRL (pinos 0..7 de cada porta) */
#define CFG_SAIDA_PP  0x3u   /* saida push-pull 50 MHz */
#define CFG_ENTRADA   0x4u   /* entrada flutuante      */

static inline void qtr_modo(const PinoQTR *s, uint32_t cfg)
{
  uint32_t sh = (uint32_t)s->pino * 4u;
  s->porta->CRL = (s->porta->CRL & ~(0xFu << sh)) | (cfg << sh);
}

static void qtr_ler_bruto(void)
{
#if QTR_TIPO_RC
  /* 1) carrega os capacitores: pinos como saida em nivel alto */
  for (int i = 0; i < 8; i++) {
    qtr[i].porta->BSRR = (1u << qtr[i].pino);
    qtr_modo(&qtr[i], CFG_SAIDA_PP);
  }
  atraso_us(10);

  /* 2) pinos como entrada e mede quanto tempo cada um leva para cair */
  for (int i = 0; i < 8; i++) {
    qtr_modo(&qtr[i], CFG_ENTRADA);
    qtr_bruto[i] = QTR_TIMEOUT_US;
  }

  uint32_t ini = DWT->CYCCNT;
  uint8_t pendentes = 0xFF;
  while (pendentes) {
    uint32_t t = us_desde(ini);
    if (t >= QTR_TIMEOUT_US) break;
    for (int i = 0; i < 8; i++) {
      if ((pendentes & (1u << i)) && !(qtr[i].porta->IDR & (1u << qtr[i].pino))) {
        qtr_bruto[i] = (uint16_t)t;
        pendentes &= (uint8_t)~(1u << i);
      }
    }
  }
#else
  /* placa com saida digital: 1 -> 1000, 0 -> 0 */
  for (int i = 0; i < 8; i++)
    qtr_bruto[i] = (qtr[i].porta->IDR & (1u << qtr[i].pino)) ? 1000 : 0;
#endif
}

static void calib_reset(void)
{
  for (int i = 0; i < 8; i++) { qtr_min[i] = 0xFFFF; qtr_max[i] = 0; }
  calibrado = 0;
}

/* Igual a QTRSensors::calibrate(): faz 10 leituras e so aceita um novo
 * maximo/minimo que se repita em todas elas (ignora picos de ruido). */
static void calib_atualizar(void)
{
  uint16_t lo[8], hi[8];
  for (int i = 0; i < 8; i++) { lo[i] = 0xFFFF; hi[i] = 0; }

  for (int n = 0; n < 10; n++) {
    qtr_ler_bruto();
    for (int i = 0; i < 8; i++) {
      if (qtr_bruto[i] < lo[i]) lo[i] = qtr_bruto[i];
      if (qtr_bruto[i] > hi[i]) hi[i] = qtr_bruto[i];
    }
  }
  for (int i = 0; i < 8; i++) {
    if (lo[i] > qtr_max[i]) qtr_max[i] = lo[i];
    if (hi[i] < qtr_min[i]) qtr_min[i] = hi[i];
  }
}

/* Converte para 0..1000, onde 1000 = em cima da linha */
static void qtr_normalizar(void)
{
  for (int i = 0; i < 8; i++) {
    int32_t v;
#if QTR_TIPO_RC
    int32_t faixa = (int32_t)qtr_max[i] - (int32_t)qtr_min[i];
    if (!calibrado || faixa <= 0) v = 0;
    else v = ((int32_t)qtr_bruto[i] - (int32_t)qtr_min[i]) * 1000 / faixa;
#else
    v = qtr_bruto[i];
#endif
    if (v < 0)    v = 0;
    if (v > 1000) v = 1000;
    if (linha_branca) v = 1000 - v;
    qtr_norm[i] = (uint16_t)v;
  }
}

/* Posicao da linha: 0 (sensor 1) ... 7000 (sensor 8); 3500 = centro */
static int32_t qtr_posicao(uint8_t *viu_linha)
{
  int32_t soma = 0, pond = 0;
  uint8_t achou = 0;
  for (int i = 0; i < 8; i++) {
    if (qtr_norm[i] > QTR_LIMIAR_LINHA) achou = 1;
    if (qtr_norm[i] > QTR_LIMIAR_RUIDO) {
      soma += qtr_norm[i];
      pond += (int32_t)qtr_norm[i] * i * 1000;
    }
  }
  if (!achou || soma == 0) {
    /* perdeu a linha: continua virando para o lado onde ela sumiu */
    *viu_linha = 0;
    ultima_pos = (ultima_pos < 3500) ? 0 : 7000;
    return ultima_pos;
  }
  *viu_linha = 1;
  ultima_pos = pond / soma;
  return ultima_pos;
}

/* ---------- Sensores laterais QRE (lidos como digitais) ---------- */
static inline uint8_t qre_esq(void)
{
  return HAL_GPIO_ReadPin(qre_esq_GPIO_Port, qre_esq_Pin) == GPIO_PIN_SET;
}

static inline uint8_t qre_dir(void)
{
  return HAL_GPIO_ReadPin(qre_dir_GPIO_Port, qre_dir_Pin) == GPIO_PIN_SET;
}

/* ---------- Modos ---------- */
static void mostrar_parametros(void)
{
  printf("vel_base=%ld  Kp_x100=%ld  Kd_x100=%ld  linha=%s\r\n",
         (long)vel_base, (long)kp_x100, (long)kd_x100,
         linha_branca ? "BRANCA" : "PRETA");
}

static void mudar_modo(Modo m)
{
  modo     = m;
  modo_t0  = HAL_GetTick();
  passo    = 0;
  erro_ant = 0;
  primeiro_ciclo = 1;
  motores(0, 0);

  printf("Modo: %s\r\n", nome_modo[m]);
  if (m == MODO_CALIBRAR) {
    calib_reset();
    printf("Calibrando por 3 s... deixe o robo sobre a linha\r\n");
  } else if (m == MODO_TESTE_MOTOR) {
    printf("A frente, A re, B frente, B re, ambos frente, ambos re (1 s cada)\r\n");
  } else if (m == MODO_SEGUIR) {
    mostrar_parametros();
  }
}

static void manual(int32_t a, int32_t b)
{
  if (modo != MODO_MANUAL) mudar_modo(MODO_MANUAL);
  manual_a = a;
  manual_b = b;
}

static void tratar_ir(void)
{
  if (!ir_novo) return;
  ir_novo = 0;

  uint8_t c = ir_cmd;
  printf("IR: end=0x%02X cmd=0x%02X\r\n", ir_end, c);

  switch (c) {
    case IR_OK:    mudar_modo(MODO_PARADO);      break;
    case IR_1:     mudar_modo(MODO_SENSORES);    break;
    case IR_2:     mudar_modo(MODO_TESTE_MOTOR); break;
    case IR_4:     mudar_modo(MODO_CALIBRAR);    break;
    case IR_5:     mudar_modo(MODO_FRENTE);      break;
    case IR_6:     mudar_modo(MODO_RE);          break;
    case IR_3:
      if (QTR_TIPO_RC && !calibrado) printf("Calibre primeiro (tecla 4)\r\n");
      else mudar_modo(MODO_SEGUIR);
      break;

    case IR_AST:
      vel_base -= VEL_PASSO;
      if (vel_base < 0) vel_base = 0;
      mostrar_parametros();
      break;
    case IR_CERQ:
      vel_base += VEL_PASSO;
      if (vel_base > PWM_MAX) vel_base = PWM_MAX;
      mostrar_parametros();
      break;
    case IR_7:
      kd_x100 -= KD_PASSO;
      if (kd_x100 < 0) kd_x100 = 0;
      mostrar_parametros();
      break;
    case IR_9:
      kd_x100 += KD_PASSO;
      mostrar_parametros();
      break;
    case IR_0:
      linha_branca = !linha_branca;
      mostrar_parametros();
      break;

    case IR_CIMA:  manual( VEL_MANUAL,  VEL_MANUAL); break;
    case IR_BAIXO: manual(-VEL_MANUAL, -VEL_MANUAL); break;
    case IR_ESQ:   manual(-VEL_MANUAL,  VEL_MANUAL); break;
    case IR_DIR:   manual( VEL_MANUAL, -VEL_MANUAL); break;
    default: break;
  }
}

static void executar_modo(void)
{
  static uint32_t ult_print = 0;
  uint32_t agora = HAL_GetTick();
  uint32_t dt    = agora - modo_t0;

  switch (modo) {

  case MODO_PARADO:
    break;

  case MODO_SENSORES: {
    qtr_ler_bruto();
    qtr_normalizar();
    if (agora - ult_print >= 150) {
      ult_print = agora;
      uint8_t viu;
      int32_t pos = qtr_posicao(&viu);
      printf("bruto:");
      for (int i = 0; i < 8; i++) printf(" %4u", qtr_bruto[i]);
      printf("  | norm:");
      for (int i = 0; i < 8; i++) printf(" %4u", qtr_norm[i]);
      printf("  | pos=%ld%s", (long)pos, viu ? "" : " (sem linha)");
      printf("  | QRE esq=%u dir=%u\r\n", qre_esq(), qre_dir());
    }
    break;
  }

  case MODO_TESTE_MOTOR:
    if (dt >= 1000) { passo++; modo_t0 = agora; }
    switch (passo) {
      case 0: motores( VEL_TESTE, 0);          break;
      case 1: motores(-VEL_TESTE, 0);          break;
      case 2: motores(0,  VEL_TESTE);          break;
      case 3: motores(0, -VEL_TESTE);          break;
      case 4: motores( VEL_TESTE,  VEL_TESTE); break;
      case 5: motores(-VEL_TESTE, -VEL_TESTE); break;
      default: mudar_modo(MODO_PARADO);        break;
    }
    break;

  case MODO_CALIBRAR:
    calib_atualizar();
    if (dt < 3000) {
      /* gira para um lado e para o outro, passando os sensores sobre a linha */
      if (((dt + 375) / 750) % 2 == 0) motores( VEL_CALIBRA, -VEL_CALIBRA);
      else                             motores(-VEL_CALIBRA,  VEL_CALIBRA);
    } else {
      motores(0, 0);
      calibrado = 1;
      printf("min:");
      for (int i = 0; i < 8; i++) printf(" %4u", qtr_min[i]);
      printf("\r\nmax:");
      for (int i = 0; i < 8; i++) printf(" %4u", qtr_max[i]);
      printf("\r\nCalibrado.\r\n");
      mudar_modo(MODO_PARADO);
    }
    break;

  case MODO_SEGUIR: {
    qtr_ler_bruto();
    qtr_normalizar();
    uint8_t viu;
    int32_t pos  = qtr_posicao(&viu);
    (void)viu;
    int32_t erro = (pos - 3500) * SENTIDO_CORRECAO;
    if (primeiro_ciclo) { erro_ant = erro; primeiro_ciclo = 0; }  /* evita tranco na partida */

    int32_t corr = (erro * kp_x100 + (erro - erro_ant) * kd_x100) / 100;
    erro_ant = erro;

    /* reduz a velocidade nas curvas fortes (como no codDefinitivo) */
    int32_t base = vel_base;
    int32_t abs_erro = (erro < 0) ? -erro : erro;
    if (abs_erro > ERRO_CURVA) {
      base -= abs_erro * 141 / 100;
      if (base < VEL_MIN_CURVA) base = VEL_MIN_CURVA;
      if (base > vel_base) base = vel_base;
    }

    /* como no Arduino: constrain(vel, 0, 255) -> as rodas nunca giram para tras */
    int32_t esq = base + corr;
    int32_t dir = base - corr;
    if (esq < 0) esq = 0;
    if (esq > PWM_MAX) esq = PWM_MAX;
    if (dir < 0) dir = 0;
    if (dir > PWM_MAX) dir = PWM_MAX;
    motores(esq, dir);
    break;
  }

  case MODO_MANUAL:
    if (agora - ir_ult_ms > IR_TIMEOUT_MANUAL_MS) mudar_modo(MODO_PARADO);
    else motores(manual_a, manual_b);
    break;

  case MODO_FRENTE:            /* tecla 5: as duas rodas para frente ate apertar OK */
    motores( VEL_TESTE,  VEL_TESTE);
    break;

  case MODO_RE:                /* tecla 6: as duas rodas para tras ate apertar OK */
    motores(-VEL_TESTE, -VEL_TESTE);
    break;
  }
}

static void pisca_led(void)
{
  static uint32_t ult = 0;
  if (HAL_GetTick() - ult >= 500) {
    ult = HAL_GetTick();
    HAL_GPIO_TogglePin(led_placa_GPIO_Port, led_placa_Pin);
  }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI2_Init();
  MX_TIM4_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  dwt_iniciar();

  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);

  HAL_GPIO_WritePin(ledon_GPIO_Port, ledon_Pin, GPIO_PIN_SET);   /* liga os LEDs do QTR */
  HAL_GPIO_WritePin(sd_cs_GPIO_Port, sd_cs_Pin, GPIO_PIN_SET);   /* SD desselecionado */

  motores(0, 0);
  calib_reset();

  printf("\r\n=== Robo seguidor - teste ===\r\n");
  printf("OK=parar  1=sensores  2=teste motores  3=seguir  4=calibrar\r\n");
  printf("5=frente  6=re  setas=manual  *=vel-  #=vel+\r\n");
  printf("7=Kd-  9=Kd+  0=troca cor da linha\r\n");
  mostrar_parametros();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    tratar_ir();
    executar_modo();
    pisca_led();
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 0;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 3599;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */
  HAL_TIM_MspPostInit(&htim4);

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(led_placa_GPIO_Port, led_placa_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(ledon_GPIO_Port, ledon_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, bin1_Pin|bin2_Pin|sd_cs_Pin|ain1_Pin
                          |ain2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : led_placa_Pin */
  GPIO_InitStruct.Pin = led_placa_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(led_placa_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : ledon_Pin */
  GPIO_InitStruct.Pin = ledon_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(ledon_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : qtr1_Pin qtr2_Pin qtr3_Pin qtr4_Pin
                           qtr5_Pin qtr6_Pin */
  GPIO_InitStruct.Pin = qtr1_Pin|qtr2_Pin|qtr3_Pin|qtr4_Pin
                          |qtr5_Pin|qtr6_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : qtr7_Pin qtr8_Pin */
  GPIO_InitStruct.Pin = qtr7_Pin|qtr8_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : bin1_Pin bin2_Pin sd_cs_Pin ain1_Pin
                           ain2_Pin */
  GPIO_InitStruct.Pin = bin1_Pin|bin2_Pin|sd_cs_Pin|ain1_Pin
                          |ain2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : qre_esq_Pin qre_dir_Pin */
  GPIO_InitStruct.Pin = qre_esq_Pin|qre_dir_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : ir_Pin */
  GPIO_InitStruct.Pin = ir_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(ir_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
/* Decodificador NEC: mede o tempo entre bordas de descida do receptor IR.
 *   inicio     : ~13,5 ms   (9 ms baixo + 4,5 ms alto)
 *   repeticao  : ~11,25 ms  (9 ms baixo + 2,25 ms alto)
 *   bit 0      : ~1,125 ms
 *   bit 1      : ~2,25 ms
 * Quadro: 32 bits, LSB primeiro = endereco, ~endereco, comando, ~comando */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin != ir_Pin) return;

  uint32_t agora = DWT->CYCCNT;
  uint32_t dt    = (agora - ir_ult_borda) / ciclos_por_us;
  ir_ult_borda   = agora;

  if (dt > 12500 && dt < 14500) {            /* inicio de quadro */
    ir_recebendo = 1;
    ir_bits = 0;
    ir_dados = 0;
    return;
  }
  if (dt > 10500 && dt < 12500) {            /* codigo de repeticao (tecla segurada) */
    if (ir_teve_codigo) ir_ult_ms = HAL_GetTick();
    ir_recebendo = 0;
    return;
  }
  if (!ir_recebendo) return;

  uint32_t bit;
  if      (dt >  800 && dt < 1500) bit = 0;
  else if (dt > 1800 && dt < 2700) bit = 1;
  else { ir_recebendo = 0; return; }         /* tempo invalido: descarta */

  ir_dados |= (bit << ir_bits);
  ir_bits++;

  if (ir_bits == 32) {
    ir_recebendo = 0;
    uint8_t cmd  = (uint8_t)(ir_dados >> 16);
    uint8_t ncmd = (uint8_t)(ir_dados >> 24);
    if ((uint8_t)(cmd ^ ncmd) == 0xFF) {     /* confere o comando invertido */
      ir_end  = (uint8_t)(ir_dados & 0xFF);
      ir_cmd  = cmd;
      ir_novo = 1;
      ir_teve_codigo = 1;
      ir_ult_ms = HAL_GetTick();
    }
  }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
