#include "main.h"

void coil(void);   //coil on/off
void ledpin(void); //led on/off
void button(void); //button short/long pression
//void buzzer(unsigned int interval); //функция звук (инт-л)

#define buzzPin PB3
#define ledPin PB2
#define buttInnerMISO PB1
#define buttOuterMOSI PB0
#define coilPin PB4
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
  if (!timer1_fl && !press3s && press100ms) //если таймер не был запущен и разрешена coil and butt pressed
  {
    timer1_fl = 1; //запустить таймер
  }
  else if (timer1_fl && press3s) //если таймер был запущен и запрещена coil
  {
    cli();           //остановить прерывания
    timer1_cntr = 0; //обнулить переменную таймера
    sei();           //разрешить прерывания
    timer1_fl = 0;   //запретить пополнение переменной таймера
  }
  else if (timer1_fl && !press3s) //если разрешена coil
  {
    cli();                               //остановить прерывания
    timer1_counter = timer1_cntr;        //сохранить значение переменной таймера
    sei();                               //разрешить прерывания
    if (timer1_counter >= COIL_DURATION) //counter reached max
    {
      cli();
      timer1_cntr = 0; //обнулить таймер
      sei();
      timer1_fl = 0;    //запретить пополнение переменной таймера
      if (coil_fl == 0) //if flag was reset
      {
        coil_fl = 1;           // flag set
        PORTB |= _BV(coilPin); //coil turn on
      }
    }
    else //идет счет
    {
      if (coil_fl == 1) //if flag was set
      {
        coil_fl = 0;            //flag reset
        PORTB &= ~_BV(coilPin); //coil turn off
      }
    }
  }
}

void button(void) //press long/sort func, timer 2
{
  if (timer2_fl == 0)
  {
    timer2_fl = 1;
  }

  cli();
  timer2_counter = timer2_cntr;
  sei();

  if (timer2_counter >= INTERV_DEBOUNCE)
  {
    cli();
    timer2_cntr = 0; //timer counter reset
    sei();
    if (~PINB & _BV(buttInnerMISO)) //is button pressed?
    {
      press100ms = 1;
      if (buttCntr >= INTERV_LONG) //long press is reached max
      {
        buttCntr = 0;       //reset debounced counter
        press3s = !press3s; //flip long press flag
      }
      else
      {
        buttCntr++;
      }
    }
    else
    {
      buttCntr = 0;   //reset debounced counter
      press100ms = 0; //reset short press flag
    }
  }
}

void ledpin(void) //led control, temer4
{
  // if (timer4_fl == 0)
  // {
  //   timer4_fl = 1;
  // }

  cli();
  timer4_counter = timer4_cntr;
  sei();
  if (timer4_counter >= INTERV_LED) //end time period reach
  {
    cli();
    timer4_cntr = 0; //timer conter reset
    sei();
    // if (timer1_fl && !press3s) //if coil on
    if (coil_fl) //if coil on
    {
      if (!led_fl)
      {
        led_fl = 1;
        PORTB &= ~_BV(ledPin) & ~_BV(buzzPin); //buzz off; //led on
      }
    }
    else if (!press3s) //exit duration, led and buzzer is toggled
    {
      // led_fl = !led_fl;
      if (led_fl)
      {
        led_fl = 0;
        PORTB |= _BV(ledPin) | _BV(buzzPin); //buzz on; //led off
      }
      else
      {
        led_fl = 1;
        PORTB &= ~_BV(ledPin) & ~_BV(buzzPin); //buzz off; //led on}
      }
    }
    else // coil continuously off, buzz and led is slowly toggled
    {
      if (led_slow_cntr >= LED_SLOW_MAX)
      {
        led_slow_cntr = 0; //reset cntr
        if (!led_fl)
        {
          PORTB &= ~_BV(ledPin);//led on
          PORTB |= _BV(buzzPin); //buzz on
          led_fl = 1;
        }
      }
      else
      {
        led_slow_cntr++;
        if (led_fl)
        {
          PORTB |= _BV(ledPin);//led off
          PORTB &= ~_BV(buzzPin); //buzz off
          led_fl = 0;
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
