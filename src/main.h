//#include <Arduino.h>
//#include "timer-api.h"
//#include "wiring_private.h"
#include <avr/eeprom.h>
//#include <AceButton.h>
#include <avr/io.h>
//#include <util/delay.h>
#include <avr/interrupt.h>

// Атомарные операции
#define ATOMIC_BLOCK(statement) do { cli(); statement; sei(); } while(0)
#define ATOMIC_READ(src, dest) ATOMIC_BLOCK(dest = src)
#define ATOMIC_RESET(var) ATOMIC_BLOCK(var = 0)

// Управление пинами
#define PIN_SET(pin)    (PORTB |= _BV(pin))
#define PIN_CLEAR(pin)  (PORTB &= ~_BV(pin))
#define PIN_TOGGLE(pin) (PORTB ^= _BV(pin))
#define PIN_READ(pin)   (PINB & _BV(pin))

// Управление флагами
#define FLAG_SET(flag)      (flag = 1)
#define FLAG_CLEAR(flag)    (flag = 0)
#define FLAG_TOGGLE(flag)   (flag = !flag)
#define FLAG_IS_SET(flag)   (flag == 1)
#define FLAG_IS_CLEAR(flag) (flag == 0)
