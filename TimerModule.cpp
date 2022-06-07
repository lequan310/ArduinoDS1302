#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include "TimerModule.h"

int dec2bcd(int dec) {
  return ((dec / 10 * 16) + (dec % 10));
}

int bcd2dec(int bcd) {
  return ((bcd / 16 * 10) + (bcd % 16));
}

void initTM() { // Initialize timer module
  DDRB |= (1 << RST) | (1 << CLK) | (1 << IO); // Set all pin as output
  PORTB &= ~(1 << RST) & ~(1 << CLK) & ~(1 << IO); // Set all pin as low

  TCCR2B |= (1 << CS20); // Enable Timer 2
}

void startComm() { // Comm = communication between arduino and timer module DS1302
  PORTB |= (1 << RST);
}

void endComm() {
  PORTB &= ~(1 << RST);
}

void generateCLKSignal() {
  PORTB |= (1 << CLK);
  delayCLK(); // delay 2 us
  PORTB &= ~(1 << CLK);
  delayCLK();
}

void delayCLK() { 
  TCNT2 = 0;

  while (1) {
    if (TCNT2 >= 31) return;
  }
}

void sendByte(int data) {
  DDRB |= (1 << IO); // Set IO port output
  PORTB &= ~(1 << IO);

  for (int i = 0; i < 8; i++) {
    if ((data >> i) & 0x01) {
      PORTB |= (1 << IO);
    } else {
      PORTB &= ~(1 << IO);
    }

    generateCLKSignal();
  }
}

int receiveByte() {
  DDRB &= ~(1 << IO); // Set IO port input

  int result = 0;

  for (int i = 0; i < 8; i++) {
    if (PINB & (1 << IO)) {
      result |= (1 << i);
    } 

    generateCLKSignal();
  }

  return result;
}

void burstRead(int myByte[]) {
  startComm();
  sendByte(burst_r);

  for (int i = 0; i < 8; i++) {
    myByte[i] = receiveByte();
  }
  endComm();
}

void burstWrite(int myByte[]) {
  startComm();
  sendByte(burst_w);

  for (int i = 0; i < 8; i++) {
    sendByte(myByte[i]);
  }
  endComm();
}

void getTime(int myByte[], int currentTime[]) {
  burstRead(myByte);
  
  int minute = bcd2dec(myByte[1]);
  int hour = bcd2dec(myByte[2]);

  currentTime[1] = hour;
  currentTime[0] = minute;
}

void setTime(int myByte[], int currentTime[]) {
  for (int i = 0; i < 8; i++) {
    myByte[i] = 0;
  }

  myByte[2] = dec2bcd(currentTime[1]); // hour byte
  myByte[1] = dec2bcd(currentTime[0]); // min byte

  burstWrite(myByte);
}
