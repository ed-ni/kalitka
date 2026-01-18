#include "main.h"

void coil(void);   // coil on/off
void ledpin(void); // led on/off
void button(void); // button short/long press

#define buzzPin PB3 // зуммер
#define ledPin PB2  // светодиод
#define buttInnerMISO PB1 // кнопка
#define buttOuterMOSI PB0
#define coilPin PB4 // катушка
#define INTERV_DEBOUNCE 9  // 0.01s x 9 = 0.09s debounce
#define INTERV_LED 9       // 0.01s x 9 = 0.09s fast blink period
#define INTERV_LONG 29     // 0.1s x 30 = 3.0s long press
#define COIL_DURATION 399  // 0.01s x 399 = 3.99s (~4s) opening duration
#define LED_SLOW_MAX 20    // 0.1s x 20 = 2.0s slow blink period
#define MODE_SWITCH_SIGNAL_DURATION 399 // 4s signal duration

volatile bool press100ms = 0; // short press flag
volatile bool press3s = 1;    // mode flag: 1=P1 (open), 0=P2 (locked)
volatile bool modeSwitchSignal = 0; // mode switch signal active flag

volatile bool timer1_fl = 0, timer2_fl = 1; // timer enable flags
unsigned int buttCntr, led_slow_cntr;       // counters
volatile unsigned int modeSwitchCntr = 0;   // mode switch signal counter (incremented in ISR)
bool coil_fl = 1, led_fl = 0;               // state flags: coil_fl=1=locked, 0=unlocked
volatile unsigned long int timer1_cntr, timer1_counter;
volatile unsigned long int timer2_cntr, timer2_counter;
volatile unsigned long int timer4_cntr, timer4_counter;

ISR(TIM0_COMPA_vect)
{
  if (timer1_fl)
    timer1_cntr++; // Timer1: coil opening duration
  
  if (timer2_fl)
    timer2_cntr++; // Timer2: button debounce
  
  timer4_cntr++;   // Timer4: LED/buzzer control
  
  if (modeSwitchSignal)
    modeSwitchCntr++; // Mode switch signal duration counter
}

void setup()
{
  // Initialize pins
  DDRB |= _BV(ledPin) | _BV(buzzPin) | _BV(coilPin) | _BV(buttOuterMOSI);
  PORTB &= ~_BV(ledPin) & ~_BV(buzzPin) & ~_BV(coilPin) & ~_BV(buttOuterMOSI); // buttOuterMOSI = virtual GND
  PORTB |= _BV(buttInnerMISO); // pullup on input
  
  // Initialize state: P1 mode (open), coil off
  press3s = 1;           // P1 mode
  coil_fl = 0;           // coil unlocked (open)
  PORTB &= ~_BV(coilPin); // coil off at startup
  led_fl = 0;
  led_slow_cntr = 0;
  buttCntr = 0;
  modeSwitchCntr = 0;
  modeSwitchSignal = 0;
  timer1_cntr = 0;
  timer2_cntr = 0;
  timer4_cntr = 0;

  // Timer0 setup: CTC mode, interrupt every ~10ms
  TCCR0B |= _BV(CS02) | _BV(CS00); // prescaler 1024: 1200000/1024 = 1171.875 Hz
  TCCR0A |= _BV(WGM01);            // CTC mode
  TIMSK0 |= _BV(OCIE0A);           // enable interrupt
  OCR0A = 11;                      // 853us * 12 = ~10.236ms
  sei();
}

int main(void)
{
  setup();

  while (1)
  {
    button();
    ledpin();
    coil();
  }
}

// Coil control function (timer1)
// P1 mode: coil always off (unlocked)
// P2 mode: coil on (locked) in idle, off (unlocked) during opening
void coil(void)
{
  // P1 mode: coil always off
  if (press3s)
  {
    if (timer1_fl)
    {
      cli();
      timer1_cntr = 0;
      timer1_fl = 0;
      sei();
    }
    coil_fl = 0;
    PORTB &= ~_BV(coilPin); // coil off (unlocked)
    return;
  }

  // P2 mode
  if (!timer1_fl && !coil_fl && press100ms) // Start opening: button pressed, coil locked
  {
    cli();
    timer1_cntr = 0;
    timer1_fl = 1;
    sei();
    coil_fl = 0;
    PORTB &= ~_BV(coilPin); // unlock (coil off)
  }
  else if (timer1_fl) // Timer running (opening in progress)
  {
    cli();
    timer1_counter = timer1_cntr;
    sei();
    
    if (timer1_counter >= COIL_DURATION) // Opening duration expired
    {
      cli();
      timer1_cntr = 0;
      timer1_fl = 0;
      sei();
      coil_fl = 1;
      PORTB |= _BV(coilPin); // lock (coil on)
    }
  }
  else if (!coil_fl) // P2 idle but coil off (shouldn't happen normally)
  {
    coil_fl = 1;
    PORTB |= _BV(coilPin); // ensure locked in P2 idle
  }
}

// Button processing function (timer2)
// Short press (0.1s): open door in P2 mode
// Long press (3s): switch mode P1↔P2
void button(void)
{
  cli();
  timer2_counter = timer2_cntr;
  sei();

  if (timer2_counter >= INTERV_DEBOUNCE)
  {
    cli();
    timer2_cntr = 0;
    sei();
    
    if (~PINB & _BV(buttInnerMISO)) // button pressed
    {
      press100ms = 1;
      
      if (buttCntr >= INTERV_LONG) // long press detected (3s)
      {
        buttCntr = 0;
        
        // Mode switch: P1↔P2
        if (!modeSwitchSignal) // avoid multiple triggers
        {
          press3s = !press3s;
          modeSwitchSignal = 1;
          cli();
          modeSwitchCntr = 0;
          sei();
          
          // Change coil state according to new mode
          if (press3s) // switched to P1 (open)
          {
            coil_fl = 0;
            PORTB &= ~_BV(coilPin); // unlock
            cli();
            timer1_cntr = 0;
            timer1_fl = 0;
            sei();
          }
          else // switched to P2 (locked)
          {
            coil_fl = 1;
            PORTB |= _BV(coilPin); // lock
          }
        }
      }
      else
      {
        buttCntr++;
      }
    }
    else // button released
    {
      buttCntr = 0;
      press100ms = 0;
    }
  }
  
  // Handle mode switch signal duration (4 seconds) - counter incremented in ISR
  if (modeSwitchSignal)
  {
    cli();
    unsigned int cntr = modeSwitchCntr;
    sei();
    if (cntr >= MODE_SWITCH_SIGNAL_DURATION)
    {
      modeSwitchSignal = 0;
      cli();
      modeSwitchCntr = 0;
      sei();
    }
  }
}

// LED and buzzer control function (timer4)
// Handles all LED/buzzer states: P1 slow blink, P2 idle constant, P2 opening fast blink, mode switch signal
void ledpin(void)
{
  cli();
  timer4_counter = timer4_cntr;
  sei();
  
  if (timer4_counter >= INTERV_LED)
  {
    cli();
    timer4_cntr = 0;
    sei();
    
    // Mode switch signal: fast blink for 4 seconds (same as opening)
    if (modeSwitchSignal)
    {
      led_fl = !led_fl;
      if (led_fl)
      {
        PORTB &= ~_BV(ledPin) & ~_BV(buzzPin); // LED on, buzzer on
      }
      else
      {
        PORTB |= _BV(ledPin) | _BV(buzzPin);   // LED off, buzzer off
      }
    }
    // P2 idle: coil locked, LED constant on, buzzer off
    else if (!press3s && coil_fl)
    {
      if (!led_fl)
      {
        led_fl = 1;
        PORTB &= ~_BV(ledPin);  // LED on
        PORTB |= _BV(buzzPin);  // Buzzer off
      }
    }
    // P2 opening: coil unlocked, fast blink LED + buzzer (0.1s period)
    else if (!press3s && !coil_fl)
    {
      led_fl = !led_fl;
      if (led_fl)
      {
        PORTB &= ~_BV(ledPin) & ~_BV(buzzPin); // LED on, buzzer on
      }
      else
      {
        PORTB |= _BV(ledPin) | _BV(buzzPin);   // LED off, buzzer off
      }
    }
    // P1 mode: slow blink LED + buzzer (2.0s period)
    else // press3s == 1 (P1 mode)
    {
      if (led_slow_cntr >= LED_SLOW_MAX)
      {
        led_slow_cntr = 0;
        if (!led_fl)
        {
          PORTB &= ~_BV(ledPin) & ~_BV(buzzPin); // LED on, buzzer on
          led_fl = 1;
        }
      }
      else
      {
        led_slow_cntr++;
        if (led_fl)
        {
          PORTB |= _BV(ledPin) | _BV(buzzPin); // LED off, buzzer off
          led_fl = 0;
        }
      }
    }
  }
}

