#include <TimerThree.h>
#include <IRremote.h>
#include <IRremoteInt.h>
#include <SPI.h>
#include <gamma.h>
#include <RGBmatrixPanel.h>
#include <Adafruit_GFX.h>
#include <Ethernet.h>

#define IRPIN  3
#define ON 4034314555
#define OFF 4287727287
#define BUP 2878444831
#define BDOWN 1373912347
#define RED 3855596927
#define GREEN 2351064443
#define BLUE 713627999
#define WHITE 3577243675
#define FLASH 900285023
#define STROBE 1541889663
#define FADE 2259740311
#define SMOOTH 2388475579

#define CLOCK 0
#define PONG 1
#define COUNT 2
#define COLOURS 3
#define RESET 4

#define SCRNORMAL 0
#define SCRPONG 1
#define SCRMENU 2
#define SCRCOUNT 3
#define SCRCOLOURS 4

#define OE  31
#define LAT 32
#define A   A8
#define B   A9
#define C   A10
#define CLK 11 


const unsigned int numbers[10][5][3] = {
   {
    {1,1,1},
    {1,0,1},
    {1,0,1},
    {1,0,1},
    {1,1,1},
   },
   {
    {1,1,0},
    {0,1,0},
    {0,1,0},
    {0,1,0},
    {1,1,1},
   },
  {
    {1,1,1},
    {0,0,1},
    {1,1,1},
    {1,0,0},
    {1,1,1}
  },
  {
    {1,1,1},
    {0,0,1},
    {1,1,1},
    {0,0,1},
    {1,1,1}
  },
  {
    {1,0,0},
    {1,0,0},
    {1,1,1},
    {0,1,0},
    {0,1,0}
  },
  {
    {1,1,1},
    {1,0,0},
    {1,1,1},
    {0,0,1},
    {1,1,1}
  },
  {
    {1,1,1},
    {1,0,0},
    {1,1,1},
    {1,0,1},
    {1,1,1}
  },
  {
    {1,1,1},
    {0,0,1},
    {0,0,1},
    {0,0,1},
    {0,0,1}
  },
  {
    {1,1,1},
    {1,0,1},
    {1,1,1},
    {1,0,1},
    {1,1,1}
  },
  {
    {1,1,1},
    {1,0,1},
    {1,1,1},
    {0,0,1},
    {0,0,1}
  }
};

RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, true);

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0D, 0xC0, 0xE5 }; // Use your MAC address

String menuItems[] = {"CLOCK", "PONG", "COUNT", "RGB", "RESET"};

EthernetClient client;
IRrecv irrecv(IRPIN);
decode_results results;
int timer1_counter;   
char buffer[7];

//Pong physics
unsigned int racketHPos = 0;
unsigned int racketMPos = 0;
int ballX = 1;
int ballY = 1;
int ballTX = 1;
int ballTY = 1;
unsigned int HScore = 0;
unsigned int MScore = 0;

//Time keeping
unsigned int days = 0;
unsigned int hour = 13;
unsigned int minute = 61;
unsigned int second = 61;

//GUI
unsigned int brightness = 1;
unsigned int openScreen = 0;
unsigned int menuIndex = 0;
unsigned int screenColour = matrix.Color333(brightness, brightness, brightness);



void setup() {
  Ethernet.begin(mac);
  
  matrix.begin();
  matrix.setTextWrap(false);
  setTime();  
  
  irrecv.enableIRIn();	   
  

    
  Timer3.initialize(1000000);
  Timer3.pwm(2,511);
  Timer3.attachInterrupt(timerClock);
  
}

void timerClock(){  
  second++;
}


void loop() {
  doControl(); 
  switch(openScreen){
    default: doClock(); break;
    case SCRNORMAL: doClock(); break;
    case SCRCOUNT: break;
    case SCRMENU: doMenu(); break;  
    case SCRCOLOURS: doRGB(); break;
    case SCRPONG: doPong(); break;
  }
  doTick();

}

void setTime(){
  matrix.setTextSize(1);
  matrix.setTextColor(matrix.Color333(brightness, brightness, brightness));
  matrix.fillScreen(0);
  matrix.println("WAIT");
  matrix.swapBuffers(true);
  if (client.connect("unacceptableuse.com", 80)) {
    client.println("GET /wowsosecret/time.php HTTP/1.1");
    client.println("Host: unacceptableuse.com");
    client.println("Connection: close");
    client.println();
  }else{
    if(hour == 13){
      delay(1000);
       setTime();
    }
     
  }
  
  delay(1000);
  
  String out = "";
  
  while (client.available()) {
    char c = client.read();
    out = out+c;
    if(out.endsWith("Content-Type: text/html\r\n\r\n"))
    {
      out="";
    }
  }
  //01234567
  //hh:mm:ss
  hour = out.substring(0,2).toInt();
  minute = out.substring(3,5).toInt();
  second = out.substring(6,8).toInt();
 
  if (!client.connected()) {
    client.stop();
  }
}

void setCountdown(){
  matrix.setTextSize(1);
  matrix.setTextColor(matrix.Color333(brightness, brightness, brightness));
  matrix.fillScreen(0);
  matrix.println("WAIT");
  matrix.swapBuffers(true);
  if (client.connect("unacceptableuse.com", 80)) {
    client.println("GET /wowsosecret/count.php HTTP/1.1");
    client.println("Host: unacceptableuse.com");
    client.println("Connection: close");
    client.println();
  }else{
    delay(1000);
    setCountdown();
  }
  
  delay(1000);
  
  String out = "";
  
  while (client.available()) {
    char c = client.read();
    out = out+c;
    if(out.endsWith("Content-Type: text/html\r\n\r\n"))
    {
      out="";
    }
  }
  days = out.toInt(); 
  matrix.setTextSize(2);
}


void doClock(){
  matrix.setCursor(1,4);
  matrix.fillScreen(0);
  sprintf(buffer, "%s%d%s%s%d",hour < 10 ? "0" : NULL, hour, second % 2 == 1 ? ":" : " ",minute < 10 ? "0" : NULL, minute);
  matrix.print(buffer);
  matrix.swapBuffers(true);
}

void doCountdown(){
  matrix.setCursor(5,0);
  matrix.fillScreen(0);
  matrix.print(days);
  matrix.swapBuffers(true);
}

void doMenu(){
  matrix.setCursor(0,0);
  matrix.fillScreen(0);
  matrix.setTextColor(matrix.Color333(brightness,0,0));
  matrix.println(menuItems[menuIndex]);
  matrix.setTextColor(matrix.Color333(brightness,brightness,brightness));
  matrix.println(menuIndex > 3 ? menuItems[0] : menuItems[menuIndex+1]);
  matrix.swapBuffers(true);
}

void initRGB(){
  matrix.fillScreen(0);
  
  matrix.setCursor(1,0);
  matrix.setTextColor(matrix.Color333(brightness,0,0));
  matrix.print("R");
  
  matrix.setCursor(6,0);
  matrix.setTextColor(matrix.Color333(0,brightness,0));
  matrix.print("G");
  
  matrix.setCursor(12,0);
  matrix.setTextColor(matrix.Color333(0,0,brightness));
  matrix.print("B");
 
  matrix.swapBuffers(true);
}

void doRGB(){
  

}

void doPong(){
  matrix.fillScreen(0);
  
  //Center line
  matrix.drawLine(15,0,15,16, screenColour);
  
  //Hour paddle
  matrix.fillRect(0, racketHPos, 2, 6, screenColour);
  
  //Minute paddle
  matrix.fillRect(30, racketMPos, 2, 6, screenColour);
 
  //Ball
  matrix.drawPixel(ballX, ballY, screenColour);
  
  smallHour(HScore, 11, 0);
  smallMinute(MScore, 17, 0);
  
  matrix.swapBuffers(true);
 
  pongTick(); 
}

void pongTick(){
  
  //Move the ball in its current trajectory
  if(ballTX != 0){
      ballX += ballTX;
  }

  if(ballTY != 0){
      ballY += ballTY;
  }
  
  //Detect hitting the sides
//  if(ballX >= 31 || ballX <= 1){
//    ballTX = ballTX * -1;
//  }

  if(ballX < 1){
   if(ballY < racketHPos-1 || ballY > racketHPos+6){
      pongReset(true);
   }else{
     ballTX = ballTX * -1;
   } 
  }else if(ballX > 30){
   if(ballY < racketMPos-1 || ballY > racketMPos+6){
     pongReset(false);
   }else{
      ballTX = ballTX * -1;
   }
  }
  
  //Bounce the ball off the sides
  if(ballY >= 13 || ballY <= 1){
    ballTY = ballTY * -1;
  }
  
  
  if(ballTX == -1){
    if(minute == MScore){
      if(ballY > racketHPos+5){
        racketHPos++; 
      }else if(ballY < racketHPos){
        racketHPos--; 
      }
    }
  }else{
    if(hour == HScore){
      if(ballY > racketMPos+5){
        racketMPos++;
      }else if(ballY < racketMPos){
        racketMPos--;
      }
    }
  }
  
  delay(10);
}

void pongReset(boolean hourLost){
  
 racketHPos = 2;
 racketMPos = 2;
 ballTY = 0;
 ballTX = 0;
 ballY = hourLost ? racketMPos+3 : racketHPos+3;
 ballX = 2;
 
 delay(1000);
 
 if(hourLost){
   ballTY = -1;
   ballTX = -1;
   MScore++;

 }else{
   ballTY = 1;
   ballTX = 1;
   MScore = 0;
   HScore++;
 }

}

void doTick(){
 if(second >= 60){
   second = 0;
   minute++;
 }
 
 if(minute >= 60){
    minute = 0;
    hour++; 
    setTime();
 }
 
 if(hour >= 12){
     hour = 1;
 }
}

void doControl(){
   if (irrecv.decode(&results)) {
    if(results.value == BUP){
      if(openScreen == SCRMENU){
        if(menuIndex > 0){
           menuIndex--;
        }else{
           menuIndex = 5; 
        }
      }else if(brightness < 7){
        brightness++;
        screenColour = matrix.Color333(brightness, brightness, brightness);
        matrix.setTextColor(screenColour);
      }
    }else if(results.value == BDOWN){
      if(openScreen == SCRMENU){
        if(menuIndex < 4){
           menuIndex++;
        }else{
           menuIndex = 0; 
        }
      } else{
        if(brightness > 1){
          brightness--;
          screenColour = matrix.Color333(brightness, brightness, brightness);
          matrix.setTextColor(screenColour);
        }
      }
    }else if(results.value == FLASH){
      if(openScreen == SCRMENU){
        openScreen = menuIndex;
      }else{
        matrix.setTextSize(1);
        openScreen = SCRMENU;
      }
    }else if(results.value == STROBE){
      switch(menuIndex){
       case RESET: setTime(); openScreen = SCRNORMAL; break; 
       case COUNT: setCountdown(); doCountdown(); openScreen = SCRCOUNT; break;
       case CLOCK: setTime(); openScreen = SCRNORMAL; break;
       case COLOURS: initRGB(); openScreen = SCRCOLOURS; break;
       case PONG: HScore = hour; MScore = minute; openScreen = SCRPONG;  break;
       default: openScreen = SCRNORMAL; break;
      }
    }
    irrecv.resume(); 
  }
}

void smallHour(int digit, int x, int y){
   if(digit >= 10){
    smallDigit(1, x-4, y);
    smallDigit(digit-10, x, y);
  }else{
   smallDigit(digit, x, y);
 }
}

void smallMinute(int digit, int x, int y){
  if(digit >= 10){
    smallDigit(digit/10, x, y);
    smallDigit(digit-(digit/10)*10, x+4, y);
  }else{
   smallDigit(digit, x, y);
 }
}

void smallDigit(int digit, int x, int y){
  for(int cx = 0; cx < 3; cx++){
     for(int cy = 0; cy < 5; cy++){
         matrix.drawPixel(x+cx, y+cy, numbers[digit][cy][cx] == 1 ? screenColour : 0); 
     }
   }
}
