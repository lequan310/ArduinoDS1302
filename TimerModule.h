#ifndef _TIMERMODULE_H_
#define _TIMERMODULE_H_

// define pin
#define RST 3 // B3, rst = ce
#define CLK 5 // B5
#define IO 4 // B4

// define value
#define sec_w 0x80
#define sec_r 0x81
#define min_w 0x82
#define min_r 0x83
#define hour_w 0x84
#define hour_r 0x85
#define burst_w 0xBE
#define burst_r 0xBF

void initTM();
void startComm();
void endComm();
void generateCLKSignal();
void delayCLK();
void sendByte(int data);
int receiveByte();
void burstRead(int myByte[]);
void burstWrite(int myByte[]);
void getTime(int myByte[], int currentTime[]);
void setTime(int myByte[], int currentTime[]);

#endif
