
// https://github.com/helen-fornazier/CG_4c/blob/master/CG_4c.ino

// ultima atualização 24/10/2022
//                    28/12/2022
//                    29/12/2022  // final

#define SCALE_8000
//#define SCALE_6000

#if defined(SCALE_8000)
    #define SET_MAX_RPM 8000 // ao mudar este valor, atualizar POS_RPM_RATE abaixo
    //#define POS_RPM_RATE ((SCALE_STEPS/SET_MAX_RPM))
    #define POS_RPM_RATE (0.212) // compilador não está colaborando, fazendo a conta na mão
                                 // fator de calibração fundo de escala.
                                 //  é o resultado da divisão 1710/8000 = 0.213
                                 // .212 melhor fator calibrado na pratica
                                 // calibrado a 500 rpm e 7500 rpm.

#elif defined(SCALE_6000)
    #define SET_MAX_RPM 6000 // ao mudar este valor, atualizar POS_RPM_RATE abaixo
    //#define POS_RPM_RATE ((SCALE_STEPS/SET_MAX_RPM))
    #define POS_RPM_RATE (0.283) // compilador não está colaborando, fazendo a conta na mão
                                 // fator de calibração fundo de escala.
                                 // é o resultado da divisão 1710/6000 = .285
                                 // .283 melhor fator calibrado na pratica
                                 // calibrado a 500 rpm e 5500 rpm
#endif

#define SCALE_STEPS 1710 // ao mudar este valor, atualizar POS_RPM_RATE
#define CALIB_STEPS (SCALE_STEPS + 320) // valor a mais de varredura de calibração
#define CALIB_STEPS_NEG 70 // O quanto pra baixo do zero a calibracao afunda antes de voltar no zero
#define PHASE_RESOLUTION 256
#define CALIB_STEP_DELAY 360

#define CORRECT_CLOCK 5      // complemento da calibração da função Mills para trabalhar com outros
                             // valores de PWM
#define micros() (micros() >> CORRECT_CLOCK)
#define millis() (millis() >> CORRECT_CLOCK)

void fixDelay(uint32_t ms) {
  delay(ms << CORRECT_CLOCK);
}

static inline unsigned int POS(int pos) {
   if (pos < 0) return 0;
   if (pos > SCALE_STEPS) return SCALE_STEPS;
   return pos;
}
// Configura os pinos do Motor de Passo
int PWM1_PIN = 3;  // verde
int PWM2_PIN = 5;  // azul
int PHASE1_PIN = 6; //amarelo
int PHASE2_PIN = 7; // roxo

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*(a)))

volatile unsigned int g_current_pos = 0;
volatile unsigned int g_read_rpm = 0;
volatile unsigned long g_last_rpm_time = 0;

const uint8_t phase[PHASE_RESOLUTION][4] = {
                           {0,0,1,0},
                           {0,2,1,1},
                           {0,4,1,1},
                           {0,7,1,1},
                           {0,9,1,1},
                           {0,12,1,1},
                           {0,14,1,2},
                           {0,17,1,2},
                           {0,19,1,2},
                           {0,21,1,3},
                           {0,24,1,3},
                           {0,26,1,4},
                           {0,29,1,5},
                           {0,31,1,6},
                           {0,33,1,6},
                           {0,35,1,7},
                           {0,38,1,8},
                           {0,40,1,9},
                           {0,42,1,10},
                           {0,44,1,11},
                           {0,47,1,12},
                           {0,49,1,13},
                           {0,51,1,15},
                           {0,53,1,16},
                           {0,55,1,17},
                           {0,57,1,19},
                           {0,59,1,20},
                           {0,61,1,22},
                           {0,63,1,23},
                           {0,65,1,25},
                           {0,67,1,26},
                           {0,68,1,28},
                           {0,70,1,30},
                           {0,72,1,32},
                           {0,74,1,33},
                           {0,75,1,35},
                           {0,77,1,37},
                           {0,78,1,39},
                           {0,80,1,41},
                           {0,81,1,43},
                           {0,83,1,45},
                           {0,84,1,47},
                           {0,85,1,49},
                           {0,87,1,51},
                           {0,88,1,53},
                           {0,89,1,56},
                           {0,90,1,58},
                           {0,91,1,60},
                           {0,92,1,62},
                           {0,93,1,65},
                           {0,94,1,67},
                           {0,94,1,69},
                           {0,95,1,71},
                           {0,96,1,74},
                           {0,97,1,76},
                           {0,97,1,79},
                           {0,98,1,81},
                           {0,98,1,83},
                           {0,98,1,86},
                           {0,99,1,88},
                           {0,99,1,91},
                           {0,99,1,93},
                           {0,99,1,96},
                           {0,99,1,98},
                           {0,100,0,0},
                           {0,99,0,2},
                           {0,99,0,4},
                           {0,99,0,7},
                           {0,99,0,9},
                           {0,99,0,12},
                           {0,98,0,14},
                           {0,98,0,17},
                           {0,98,0,19},
                           {0,97,0,21},
                           {0,97,0,24},
                           {0,96,0,26},
                           {0,95,0,29},
                           {0,94,0,31},
                           {0,94,0,33},
                           {0,93,0,35},
                           {0,92,0,38},
                           {0,91,0,40},
                           {0,90,0,42},
                           {0,89,0,44},
                           {0,88,0,47},
                           {0,87,0,49},
                           {0,85,0,51},
                           {0,84,0,53},
                           {0,83,0,55},
                           {0,81,0,57},
                           {0,80,0,59},
                           {0,78,0,61},
                           {0,77,0,63},
                           {0,75,0,65},
                           {0,74,0,67},
                           {0,72,0,68},
                           {0,70,0,70},
                           {0,68,0,72},
                           {0,67,0,74},
                           {0,65,0,75},
                           {0,63,0,77},
                           {0,61,0,78},
                           {0,59,0,80},
                           {0,57,0,81},
                           {0,55,0,83},
                           {0,53,0,84},
                           {0,51,0,85},
                           {0,49,0,87},
                           {0,47,0,88},
                           {0,44,0,89},
                           {0,42,0,90},
                           {0,40,0,91},
                           {0,38,0,92},
                           {0,35,0,93},
                           {0,33,0,94},
                           {0,31,0,94},
                           {0,29,0,95},
                           {0,26,0,96},
                           {0,24,0,97},
                           {0,21,0,97},
                           {0,19,0,98},
                           {0,17,0,98},
                           {0,14,0,98},
                           {0,12,0,99},
                           {0,9,0,99},
                           {0,7,0,99},
                           {0,4,0,99},
                           {0,2,0,99},
                           {0,0,0,100},
                           {1,98,0,99},
                           {1,96,0,99},
                           {1,93,0,99},
                           {1,91,0,99},
                           {1,88,0,99},
                           {1,86,0,98},
                           {1,83,0,98},
                           {1,81,0,98},
                           {1,79,0,97},
                           {1,76,0,97},
                           {1,74,0,96},
                           {1,71,0,95},
                           {1,69,0,94},
                           {1,67,0,94},
                           {1,65,0,93},
                           {1,62,0,92},
                           {1,60,0,91},
                           {1,58,0,90},
                           {1,56,0,89},
                           {1,53,0,88},
                           {1,51,0,87},
                           {1,49,0,85},
                           {1,47,0,84},
                           {1,45,0,83},
                           {1,43,0,81},
                           {1,41,0,80},
                           {1,39,0,78},
                           {1,37,0,77},
                           {1,35,0,75},
                           {1,33,0,74},
                           {1,32,0,72},
                           {1,30,0,70},
                           {1,28,0,68},
                           {1,26,0,67},
                           {1,25,0,65},
                           {1,23,0,63},
                           {1,22,0,61},
                           {1,20,0,59},
                           {1,19,0,57},
                           {1,17,0,55},
                           {1,16,0,53},
                           {1,15,0,51},
                           {1,13,0,49},
                           {1,12,0,47},
                           {1,11,0,44},
                           {1,10,0,42},
                           {1,9,0,40},
                           {1,8,0,38},
                           {1,7,0,35},
                           {1,6,0,33},
                           {1,6,0,31},
                           {1,5,0,29},
                           {1,4,0,26},
                           {1,3,0,24},
                           {1,3,0,21},
                           {1,2,0,19},
                           {1,2,0,17},
                           {1,2,0,14},
                           {1,1,0,12},
                           {1,1,0,9},
                           {1,1,0,7},
                           {1,1,0,4},
                           {1,1,0,2},
                           {1,0,0,0},
                           {1,1,1,100},
                           {1,1,1,96},
                           {1,1,1,93},
                           {1,1,1,91},
                           {1,1,1,88},
                           {1,2,1,86},
                           {1,2,1,83},
                           {1,2,1,81},
                           {1,3,1,79},
                           {1,3,1,76},
                           {1,4,1,74},
                           {1,5,1,71},
                           {1,6,1,69},
                           {1,6,1,67},
                           {1,7,1,65},
                           {1,8,1,62},
                           {1,9,1,60},
                           {1,10,1,58},
                           {1,11,1,56},
                           {1,12,1,53},
                           {1,13,1,51},
                           {1,15,1,49},
                           {1,16,1,47},
                           {1,17,1,45},
                           {1,19,1,43},
                           {1,20,1,41},
                           {1,22,1,39},
                           {1,23,1,37},
                           {1,25,1,35},
                           {1,26,1,33},
                           {1,28,1,32},
                           {1,30,1,30},
                           {1,32,1,28},
                           {1,33,1,26},
                           {1,35,1,25},
                           {1,37,1,23},
                           {1,39,1,22},
                           {1,41,1,20},
                           {1,43,1,19},
                           {1,45,1,17},
                           {1,47,1,16},
                           {1,49,1,15},
                           {1,51,1,13},
                           {1,53,1,12},
                           {1,56,1,11},
                           {1,58,1,10},
                           {1,60,1,9},
                           {1,62,1,8},
                           {1,65,1,7},
                           {1,67,1,6},
                           {1,69,1,6},
                           {1,71,1,5},
                           {1,74,1,4},
                           {1,76,1,3},
                           {1,79,1,3},
                           {1,81,1,2},
                           {1,83,1,2},
                           {1,86,1,2},
                           {1,88,1,1},
                           {1,91,1,1},
                           {1,93,1,1},
                           {1,95,1,1},
                           {1,100,1,1}};

// Tabela de aceleração do Ponteiro criada através da planilha Excell
const float g_delay_equations[][3] = {
/*
{ 42.75,  0,  15000     },
{ 106.875,  -171.539961,  22333.33333     },
{ 160.3125, -52.39766082, 9600            },
{ 213.75, -11.22807018, 3000            },
{ 427.5,  -2.105263158, 1050            },
{ 641.25, -0.4678362573,  350             },
{ 1068.75,  -0.04678362573, 80              },
{ 1496.25,  -0.04678362573, 80              },
*/
{ 19,     0,      11000   },
{ 32,    -155,    14000   },
{ 53,   -140,     13500   },
{ 106,   -56,      9000   },
{ 160,   -33,     6600    },
{ 213,   -19,     4200    },
{ 427,   -0.23,   250     },
{ 855,   -0.06,   175     },
{ 1710,  -0.03,   150     },
};
/*
const float g_delay_equations[][3] = {
{ 42.75,  0,  15000     },
{ 106.875,  -171.539961,  22333.33333     },
{ 160.3125, -52.39766082, 9600            },
{ 213.75, -11.22807018, 3000            },
{ 427.5,  -2.105263158, 1050            },
{ 641.25, -0.4678362573,  350             },
{ 1068.75,  -0.04678362573, 80              },
{ 1496.25,  -0.04678362573, 80              },
};
*/
static void set_phase(int phase_idx) {
   while (phase_idx < 0)
      phase_idx += PHASE_RESOLUTION;

   phase_idx = phase_idx % PHASE_RESOLUTION;

   //Serial.print("set_phase:");
   //Serial.println(phase_idx);

   // regra de 3, 100 -> 255, val -> x
   uint8_t pwm1_val = phase[phase_idx][1] * 255 / 100;
   analogWrite(PWM1_PIN, pwm1_val);
   digitalWrite(PHASE1_PIN, phase[phase_idx][0]);

   uint8_t pwm2_val = phase[phase_idx][3] * 255 / 100;
   analogWrite(PWM2_PIN, pwm2_val);
   digitalWrite(PHASE2_PIN, phase[phase_idx][2]);
}

// make one step torwards position
static void go_to_pos_dir(unsigned int pos) {
   unsigned int small_pos;

   if (pos == g_current_pos)
       return;

   if (pos > g_current_pos)
      small_pos = POS(g_current_pos + 1);
   else
      small_pos = POS(g_current_pos - 1);

   set_phase(small_pos);
   g_current_pos = small_pos;
}

// Calibra o Ponteiro através da varredura.
static void calibrate() {
   int i; // Ponteiro vai para o fim de curso direita
   for (i = 0; i < CALIB_STEPS; i++) {
      delayMicroseconds(CALIB_STEP_DELAY);
      set_phase(i);
   }
   delayMicroseconds(100);
   // Ponteiro vai para o fim de curso da esquerda
   while(i-- > -CALIB_STEPS_NEG) {
      delayMicroseconds(CALIB_STEP_DELAY);
      set_phase(i);
   }

   // Volte suavemente ao zero
   while(i++) {
      delayMicroseconds(get_speed_delay(1));
      set_phase(i);
   }
}

void setup() {
  // put your setup code here, to run once:
   pinMode(PWM1_PIN, OUTPUT); // configura pino como saída
   pinMode(PWM2_PIN, OUTPUT); // configura pino como saída
   pinMode(PHASE1_PIN, OUTPUT); // configura pino como saída
   pinMode(PHASE2_PIN, OUTPUT); // configura pino como saída

   attachInterrupt(0, isr_rpm, FALLING);  //could also work with RISING

   Serial.begin(115200);
   Serial.println("CG_4c 1.0 20/10/2022");

    // Pins D5 and D6 - 16kHz
   TCCR0B = 0b00000001; // x8
   TCCR0A = 0b00000001; // fast pwm

   // Pins D3 and D11 - 16kHz
   TCCR2B = 0b00000001; // x1
   TCCR2A = 0b00000001; // phase correct

   calibrate();
}

void isr_rpm() {
  static unsigned int tick = 0;
  unsigned long current_time = millis();
  unsigned long lapsed_time = current_time - g_last_rpm_time;

  tick++;

  // wait 3 ticks, which is equivalent to 180 milli on a 500rpm
  if (tick < 3)
    return;

  g_read_rpm = ((tick * 60000)/2) / (lapsed_time);
  if (g_read_rpm > SET_MAX_RPM)
    g_read_rpm = SET_MAX_RPM;

  tick = 0;
  g_last_rpm_time = current_time;
}

// return position equivalent from 0 to SCALE_STEPS
static inline unsigned int convert_rpm_to_pos(unsigned int rpm) {
    return POS(rpm*POS_RPM_RATE);
}

static unsigned int get_target_pos() {
  if (millis() - g_last_rpm_time > 200) // if below 3 ticks time
    g_read_rpm = 0;

  return convert_rpm_to_pos(g_read_rpm);
}

float get_speed_delay(unsigned int diff) {
  unsigned int i;

  for (i = 0; i < ARRAY_SIZE(g_delay_equations); i++) {
      if (diff < g_delay_equations[i][0])
        return g_delay_equations[i][1]*diff + g_delay_equations[i][2];
  }

  // fasted speed is limited to the delay of last point on the last equation
  return g_delay_equations[i][1]*g_delay_equations[i][0] + g_delay_equations[i][2];
}

void loop() {
  //get_rpm_from_serial();
  volatile unsigned int new_pos = get_target_pos();
  unsigned int diff = new_pos > g_current_pos ? new_pos - g_current_pos :
                                                g_current_pos - new_pos;

  go_to_pos_dir(new_pos);
  float delay_us = get_speed_delay(diff);
  delayMicroseconds(delay_us);
}

/*
 * 1000 33 HZ   
2000  66 HZ   
3000  99 HZ   
4000  132 HZ    
5000  165 HZ    
6000  198 HZ    
7000  231 HZ    
8000  264 HZ    
*/
