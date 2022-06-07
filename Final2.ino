#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include "TimerModule.h"

int numbers[][7] = { // Bits to display in 7seg display
  {0, 0, 0, 0, 0, 0, 1}, // 0
  {1, 0, 0, 1, 1, 1, 1}, // 1
  {0, 0, 1, 0, 0, 1, 0}, // 2
  {0, 0, 0, 0, 1, 1, 0}, // 3
  {1, 0, 0, 1, 1, 0, 0}, // 4
  {0, 1, 0, 0, 1, 0, 0}, // 5
  {0, 1, 0, 0, 0, 0, 0}, // 6
  {0, 0, 0, 1, 1, 1, 1}, // 7
  {0, 0, 0, 0, 0, 0, 0}, // 8
  {0, 0, 0, 0, 1, 0, 0}}; // 9

int currentTime[2]; // currentTime[1] = hour, currentTime[0] = min
int myByte[8];
static int state[4] = {0, 0, 0, 0};

const int D1 = 7, D2 = 6, D3 = 5, D4 = 4; // D1 = min1, D2 = min10, D3 = hour1, D4 = hour10
const int A = 0, B = 1, C = 2, D = 0, E = 1, F = 2, G = 3;
volatile boolean setMode = false;
volatile int selected = D1;

int main(void) {  
  DDRD &= ~(1 << 0) & ~(1 << 1) & ~(1 << 2) & ~(1 << 3); // D0 = Mode, D1 = Switch Digit, D2/D3 = Increase/Decrease
  DDRD |= (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7);
  DDRB |= (1 << 0) | (1 << 1) | (1 << 2);
  DDRC |= (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4);
  PORTC &= ~(1 << 4);
  initTM();
  sei();
  
  while(1) {
    if (setMode) {
      PORTC |= (1 << 4); // Turn on LED
      if (selected == D1 || selected == D2) displayMin(); // display 2 digit for minute if current selected digit is D1 or D2
      else displayHour(); // display 2 digit for hour otherwise
      
      // Switch to run mode
      if (debounce(0)) { // If button at pin D0 is pushed
        setTime(myByte, currentTime); // Send new time data to override current time data of DS1302
        selected = D1;
        setMode = false;
      } 
      else if (debounce(1)) { // If button at pin D1 is pushed
        if (selected == D4) selected = D1;
        else selected--;
      } 
      else if (debounce(2)) { // If button at pin D2 is pushed
        switch (selected) {
          // Increase minute digit 1
          case D1: 
            if (currentTime[0] % 10 == 9) 
              currentTime[0] = (currentTime[0] / 10) * 10;
            else 
              currentTime[0]++;
            break;
          // Increase minute digit 10
          case D2: // Min 10
            if (currentTime[0] / 10 == 5) 
              currentTime[0] = currentTime[0] % 10;
            else 
              currentTime[0] += 10;
            break;
          // Increase hour digit 1
          case D3: // Hour 1
            if (currentTime[1] / 10 != 2) {
              if (currentTime[1] % 10 == 9) 
                currentTime[1] = (currentTime[1] / 10) * 10;
              else 
                currentTime[1]++;
            }
            else {
              if (currentTime[1] % 10 == 3) 
                currentTime[1] = 20;
              else 
                currentTime[1]++;
            }
            break;
          // Increase hour digit 10
          case D4: // Hour 10
            if (currentTime[1] % 10 < 4) {
              if (currentTime[1] / 10 == 2) 
                currentTime[1] = currentTime[1] % 10;
              else 
                currentTime[1] += 10;
            } else {
              if (currentTime[1] / 10 == 1) 
                currentTime[1] = currentTime[1] % 10;
              else 
                currentTime[1] += 10;
            }
            break;
          default:
            selected = D1;
        }
      } 
      else if (debounce(3)) { // If button at pin D3 is pushed
        switch (selected) {
          // Decrease minute digit 1
          case D1:
            if (currentTime[0] % 10 == 0) 
              currentTime[0] = currentTime[0] + 9;
            else 
              currentTime[0]--;
            break;
          // Decrease minute digit 10
          case D2:
            if (currentTime[0] / 10 == 0) 
              currentTime[0] = currentTime[0] + 50;
            else 
              currentTime[0] -= 10;
            break;
          // Decrease hour digit 1
          case D3:
            if (currentTime[1] % 10 == 0) {
              if (currentTime[1] / 10 == 2) currentTime[1] = 23;
              else currentTime[1] += 9;
            }
            else
              currentTime[1]--;
            break;
          // Decrease hour digit 10
          case D4:
            if (currentTime[1] / 10 == 0) {
              if (currentTime[1] % 10 < 4) currentTime[1] += 20;
              else currentTime[1] += 10;
            } 
            else
              currentTime[1] -= 10;
            break;
          default:
            selected = D1;
        }
      }
    } 
    else if (!setMode) {
      getTime(myByte, currentTime); // Get time data from Timer Module DS1302
      displayTime(); // Display time on the 4-digit-7-segment display
      PORTC &= ~(1 << 4); // Turn off LED
      
      // Switch to set mode
      if (debounce(0)) { // If button at pin D0 is pushed
        setMode = true;
      }
    }
  }
}

void displayHour() {
  setValueDisplay(D4, currentTime[1] / 10);
  setValueDisplay(D3, currentTime[1] % 10);
}

void displayMin() {
  setValueDisplay(D2, currentTime[0] / 10);
  setValueDisplay(D1, currentTime[0] % 10);
}

void displayTime() {
  displayHour();
  displayMin();
}

void setValueDisplay(int digit, int value) {
  turnOffDigit();
  PORTD |= (1 << digit);
  
  turnOnNum();
  PORTB |= (numbers[value][0] << A) | (numbers[value][1] << B) | (numbers[value][2] << C);
  PORTC |= (numbers[value][3] << D) | (numbers[value][4] << E) | (numbers[value][5] << F) | (numbers[value][6] << G);
  _delay_ms(1);
}

void turnOffDigit() {
  PORTD &= ~(1 << D1);
  PORTD &= ~(1 << D2);
  PORTD &= ~(1 << D3);
  PORTD &= ~(1 << D4);
}

void turnOnNum() {
  PORTB &= ~(1 << A) & ~(1 << B) & ~(1 << C);
  PORTC &= ~(1 << D) & ~(1 << E) & ~(1 << F) & ~(1 << G);
}

bool debounce(int button) {
  state[button] = (state[button] << 1) | (PIND & (1 << button)) | 0xe000;
  if (state[button] == 0xf000) return true;
  return false;
}
