#include <Arduino.h>
#include <TwoWayESP.h>
#include <Wire.h>


// Пины для аналоговых энкодеров
#define ENC1_PIN 34
#define ENC2_PIN 35
#define ENC3_PIN 32
#define ENC4_PIN 33

// Кнопки
#define BUTTON_PIN 21 // старт запись
#define BUTTON_PIN2 15 // старт воспроизведение
#define BUTTON_PIN3 22 // старт трансляции
#define BUTTON_PIN4 23 // сброс фингер

#define BUTTON_PIN5 34 // торс вправо
#define BUTTON_PIN6 2 // торс влево

// Остальные переменные
#define Coff 8
#define defm 26



String alltext;
int x, y;
//EC:64:C9:91:BE:58
uint8_t broadcastAddress[] = {0xEC, 0x64, 0xC9, 0x91, 0xBE, 0x58};

typedef struct ServoAngles {
  uint16_t id;
  uint16_t Serv1; // кистю
  uint16_t Serv2; // локоль
  uint16_t Serv3; //бицепс
  uint16_t Serv4; // плечо
   uint16_t buttRightHand; // правая клешня
  uint16_t Butt1; // Запись
  uint16_t Butt2; // Повтор
   uint16_t Butt3; // трансляция
} ServoAngles;

ServoAngles myData;


void setup() {


  // Настройка кнопок
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUTTON_PIN2, INPUT_PULLUP);
  pinMode(BUTTON_PIN3, INPUT_PULLUP);
  pinMode(BUTTON_PIN4, INPUT_PULLUP);
  pinMode(BUTTON_PIN5, INPUT);
  pinMode(BUTTON_PIN6, INPUT);



  // Пины аналоговых энкодеров
  pinMode(ENC1_PIN, INPUT);
  pinMode(ENC2_PIN, INPUT);
  pinMode(ENC3_PIN, INPUT);
  pinMode(ENC4_PIN, INPUT);

  // Запуск последовательного порта
  Serial.begin(9600);
  TwoWayESP::Begin(broadcastAddress);
  Serial.println("rukaout");
}

void loop() {
  // Считывание значений с энкодеров
  int enc1_value = analogRead(ENC1_PIN);
  int enc2_value = analogRead(ENC2_PIN);
  int enc3_value = analogRead(ENC3_PIN);
  int enc4_value = analogRead(ENC4_PIN);

  // Конвертация значений энкодеров в углы
  myData.Serv1 = map(enc1_value, 0, 4095, 0, 180);
  myData.Serv2 = map(enc2_value, 0, 4095, 0, 180);
  myData.Serv3 = map(enc3_value, 0, 4095, 0, 180);
  myData.Serv4 = map(enc4_value, 0, 4095, 0, 180);

  // Кнопки
  if (digitalRead(BUTTON_PIN) == LOW) myData.Butt1 = 1; else myData.Butt1 = 0;
  if (digitalRead(BUTTON_PIN2) == LOW) myData.Butt2 = 1; else myData.Butt2 = 0;
  if (digitalRead(BUTTON_PIN3) == LOW) myData.Butt3 = 1; else myData.Butt3 = 0;
    if (digitalRead(BUTTON_PIN4) == LOW)   myData.buttRightHand = 1; else   myData.buttRightHand = 0;





  // Вывод данных
  Serial.print("Enc1: "); Serial.print(myData.Serv1);
  Serial.print(" Enc2: "); Serial.print(myData.Serv2);
  Serial.print(" Enc3: "); Serial.print(myData.Serv3);
  Serial.print(" Enc4: "); Serial.println(myData.Serv4);

  // Отправка данных через ESP
   TwoWayESP::SendBytes(&myData, sizeof(ServoAngles));

  delay(200);
}
