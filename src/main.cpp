#include "main.h"

void coil(void);   //coil on/off
void ledpin(void); //led on/off
void button(void); //button short/long pression
//void buzzer(unsigned int interval); //функция звук (инт-л)

// Вспомогательные функции
static inline unsigned long atomic_read_timer(volatile unsigned long *timer);
static inline void atomic_reset_timer(volatile unsigned long *timer);
static inline bool timer_expired(volatile unsigned long *timer, unsigned long threshold);
static inline void timer_start(volatile bool *timer_flag);
static inline void timer_stop(volatile bool *timer_flag, volatile unsigned long *timer_counter);
static inline void set_coil(bool state);
static inline void set_led_buzzer(bool led_state, bool buzzer_state);

#define buzzPin PB3 //зуммер
#define ledPin PB2  // светодиод
#define buttInnerMISO PB1 // кнопка
#define buttOuterMOSI PB0
#define coilPin PB4 // катушка
#define INTERV_DEBOUNCE 9 //0.01s x 10 = 0.1 s debounce BUZZ_DURATION
#define INTERV_LED 9      //x 0.01s = 0.1 s led blink on duration
#define INTERV_LONG 29    //0.01s x INTERV_DEBOUNCE x 30 = 3 s
//#define BUZZ_DURATION 4   //buzzer sound duration 0.05 s
#define COIL_DURATION 399 //4 s
#define LED_SLOW_MAX 20   //led blink dutycicle when coil func off

volatile bool press100ms = 0; //turn off coil on 6 s
volatile bool press3s = 1;    //turn off coil
//bool press;                   //press button event

volatile bool timer1_fl = 1, timer2_fl; //переменная вкл/выкл таймера
// volatile bool timer3_fl, timer4_fl;
unsigned int buttCntr, led_slow_cntr; //debounced/short durations counter
bool coil_fl = 0, led_fl = 0;         //переменная для хранения состояния
// bool buzzer_fl;
volatile unsigned long int timer1_cntr, timer1_counter; //переменные подсчета мс и
volatile unsigned long int timer2_cntr, timer2_counter; //переменные подсчета мс и
//volatile unsigned long int timer3_cntr, timer3_counter;       //переменные подсчета мс и
volatile unsigned long int timer4_cntr, timer4_counter; //переменные подсчета мс и

// Реализации вспомогательных функций
static inline unsigned long atomic_read_timer(volatile unsigned long *timer) {
    unsigned long value;
    ATOMIC_READ(*timer, value);
    return value;
}

static inline void atomic_reset_timer(volatile unsigned long *timer) {
    ATOMIC_RESET(*timer);
}

static inline bool timer_expired(volatile unsigned long *timer, unsigned long threshold) {
    return atomic_read_timer(timer) >= threshold;
}

static inline void timer_start(volatile bool *timer_flag) {
    FLAG_SET(*timer_flag);
}

static inline void timer_stop(volatile bool *timer_flag, volatile unsigned long *timer_counter) {
    FLAG_CLEAR(*timer_flag);
    atomic_reset_timer(timer_counter);
}

static inline void set_coil(bool state) {
    if (state) {
        PIN_SET(coilPin);
    } else {
        PIN_CLEAR(coilPin);
    }
}

static inline void set_led_buzzer(bool led_state, bool buzzer_state) {
    if (led_state) {
        PIN_CLEAR(ledPin);
    } else {
        PIN_SET(ledPin);
    }
    if (buzzer_state) {
        PIN_SET(buzzPin);
    } else {
        PIN_CLEAR(buzzPin);
    }
}

ISR(TIM0_COMPA_vect)
{
  if (timer1_fl)   //если включен миллисекудный таймер для coil
    timer1_cntr++; //инкремент переменной таймера (+1)

  if (timer2_fl)   //если включен миллисекудный таймер для button
    timer2_cntr++; //инкремент переменной таймера (+1)

  // if (timer3_fl)   //если включен миллисекудный таймер для buzzer
  //   timer3_cntr++; //инкремент переменной таймера (+1)

  // if (timer4_fl)   //если включен миллисекудный таймер для led
  timer4_cntr++; //инкремент переменной таймера (+1)
}

void setup()
{
  //  INIT PINs
  //press3s = 1; //turn off coil
  DDRB |= _BV(ledPin) | _BV(buzzPin) | _BV(coilPin) | _BV(buttOuterMOSI);
  PORTB &= ~_BV(ledPin) & ~_BV(buzzPin) & ~_BV(coilPin) & ~_BV(buttOuterMOSI);//buttOuterMOSI became virtual GND
  PORTB |= _BV(buttInnerMISO);//set pullup on input

  TCCR0B |= _BV(CS02) | _BV(CS00); // 1200000 дел. 1024 = 853 мкс
  TCCR0A |= _BV(WGM01);            //set  CTC mode
  TIMSK0 |= _BV(OCIE0A);           // разрешение прерываний по совпадению т.рег.А
  OCR0A = 11;                      // 853us*12 =~ 0.01 сек.
  sei();
}

int main(void)
{
  setup();

  while (1)
  {
    button();
    //  buzzer(600);
    ledpin();
    coil();
  }
}

/////////////////// func
void coil(void) //coil function, timer 1
{
  if (FLAG_IS_CLEAR(timer1_fl) && FLAG_IS_CLEAR(press3s) && FLAG_IS_SET(press100ms)) //если таймер не был запущен и разрешена coil and butt pressed
  {
    FLAG_SET(timer1_fl); //запустить таймер
  }
  else if (FLAG_IS_SET(timer1_fl) && FLAG_IS_SET(press3s)) //если таймер был запущен и запрещена coil
  {
    atomic_reset_timer(&timer1_cntr);
    FLAG_CLEAR(timer1_fl);   //запретить пополнение переменной таймера
  }
  else if (FLAG_IS_SET(timer1_fl) && FLAG_IS_CLEAR(press3s)) //если разрешена coil
  {
    timer1_counter = atomic_read_timer(&timer1_cntr);
    if (timer1_counter >= COIL_DURATION) //counter reached max
    {
      atomic_reset_timer(&timer1_cntr);
      FLAG_CLEAR(timer1_fl);    //запретить пополнение переменной таймера
      if (FLAG_IS_CLEAR(coil_fl)) //if flag was reset
      {
        FLAG_SET(coil_fl); // flag set
        set_coil(true); //coil turn on
      }
    }
    else //идет счет
    {
      if (FLAG_IS_SET(coil_fl)) //if flag was set
      {
        FLAG_CLEAR(coil_fl); //flag reset
        set_coil(false); //coil turn off
      }
    }
  }
}

void button(void) //press long/sort func, timer 2
{
  if (FLAG_IS_CLEAR(timer2_fl))
  {
    FLAG_SET(timer2_fl);
  }

  timer2_counter = atomic_read_timer(&timer2_cntr);

  if (timer2_counter >= INTERV_DEBOUNCE)
  {
    atomic_reset_timer(&timer2_cntr);
    if (!PIN_READ(buttInnerMISO)) //is button pressed?
    {
      FLAG_SET(press100ms);
      if (buttCntr >= INTERV_LONG) //long press is reached max
      {
        buttCntr = 0;       //reset debounced counter
        FLAG_TOGGLE(press3s); //flip long press flag
      }
      else
      {
        buttCntr++;
      }
    }
    else
    {
      buttCntr = 0;   //reset debounced counter
      FLAG_CLEAR(press100ms); //reset short press flag
    }
  }
}

void ledpin(void) //led control, temer4
{
  // if (timer4_fl == 0)
  // {
  //   timer4_fl = 1;
  // }

  timer4_counter = atomic_read_timer(&timer4_cntr);
  if (timer4_counter >= INTERV_LED) //end time period reach
  {
    atomic_reset_timer(&timer4_cntr);
    // if (timer1_fl && !press3s) //if coil on
    if (FLAG_IS_SET(coil_fl)) //if coil on
    {
      if (FLAG_IS_CLEAR(led_fl))
      {
        FLAG_SET(led_fl);
        set_led_buzzer(true, false); //buzz off; //led on
      }
    }
    else if (FLAG_IS_CLEAR(press3s)) //exit duration, led and buzzer is toggled
    {
      // led_fl = !led_fl;
      if (FLAG_IS_SET(led_fl))
      {
        FLAG_CLEAR(led_fl);
        set_led_buzzer(false, true); //buzz on; //led off
      }
      else
      {
        FLAG_SET(led_fl);
        set_led_buzzer(true, false); //buzz off; //led on}
      }
    }
    else // coil continuously off, buzz and led is slowly toggled
    {
      if (led_slow_cntr >= LED_SLOW_MAX)
      {
        led_slow_cntr = 0; //reset cntr
        if (FLAG_IS_CLEAR(led_fl))
        {
          set_led_buzzer(true, true); //led on, buzz on
          FLAG_SET(led_fl);
        }
      }
      else
      {
        led_slow_cntr++;
        if (FLAG_IS_SET(led_fl))
        {
          set_led_buzzer(false, false); //led off, buzz off
          FLAG_CLEAR(led_fl);
        }
      }
    }
  }
}

// void buzzer(unsigned int interval) //buzz control. timer3
// {
//   if (timer3_fl == 0)
//   {
//     timer3_fl = 1;
//   }

//   cli();
//   timer3_counter = timer3_cntr;
//   sei();
//   if (timer3_counter >= interval) //end time period reach
//   {
//     cli();
//     timer3_cntr = 0; //timer conter reset
//     sei();
//     //  buzzer_fl = 0; //flag reset
//   }
//   else if (timer3_counter > (interval - BUZZ_DURATION)) //BUZZ_DURATION time slot
//   {
//     if (buzzer_fl == 0)
//     {
//       PORTB |= _BV(buzzPin); //buzz on
//       buzzer_fl = 1;         //flag set
//     }
//   }
//   else //rest time slot
//   {
//     if (buzzer_fl == 1)
//     {
//       PORTB &= ~_BV(buzzPin); //buzz off
//       buzzer_fl = 0;          //flag reset
//     }
//   }
// }
