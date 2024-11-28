#include <Arduino.h>
#include <TwoWayESP.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED настройки
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1 // Не используем сброс
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Пины для аналоговых энкодеров
#define ENC1_PIN 34
#define ENC2_PIN 35
#define ENC3_PIN 32
#define ENC4_PIN 33

// Кнопки
#define BUTTON_PIN 18 // старт запись
#define BUTTON_PIN2 15 // старт воспроизведение
#define BUTTON_PIN3 19 // старт трансляции
#define BUTTON_PIN4 23 // сброс фингер

// Остальные переменные
String alltext;
uint8_t broadcastAddress[] = {0xEC, 0x64, 0xC9, 0x91, 0xBE, 0x58};

typedef struct ServoAngles {
  uint16_t id;
  uint16_t Serv1;           // кисть
  uint16_t Serv2;           // локоть
  uint16_t Serv3;           // бицепс
  uint16_t Serv4;           // плечо
  uint16_t buttRightHand;   // правая клешня
  uint16_t Butt1;           // Запись
  uint16_t Butt2;           // Повтор
  uint16_t Butt3;           // Трансляция
} ServoAngles;

ServoAngles myData;

// Состояние кнопок
bool stateButt1 = false;
bool stateButt2 = false;
bool stateButt3 = false;
bool stateButt4 = false;

void setup() {
  // Настройка кнопок
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUTTON_PIN2, INPUT_PULLUP);
  pinMode(BUTTON_PIN3, INPUT_PULLUP);
  pinMode(BUTTON_PIN4, INPUT_PULLUP);

  // Пины аналоговых энкодеров
  pinMode(ENC1_PIN, INPUT);
  pinMode(ENC2_PIN, INPUT);
  pinMode(ENC3_PIN, INPUT);
  pinMode(ENC4_PIN, INPUT);

  // Запуск последовательного порта
  Serial.begin(9600);
  TwoWayESP::Begin(broadcastAddress);
  Serial.println("rukaout");

  // Инициализация OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED initialization failed");
    for (;;);
  }
  display.clearDisplay();
  display.display();
  Serial.println("OLED ready");
}

void drawScreen() {
  // Очистка экрана
  display.clearDisplay();

  // Отображение квадратиков
  int boxSize = 10;
  int startX = 5;
  int startY = 5;
  int spacing = 15;
 display.setTextSize(1); // Увеличить текст до разумного размера
  display.setTextColor(SSD1306_WHITE);
  // Квадратик 1
  if (stateButt1) display.fillRect(startX, startY, boxSize, boxSize, SSD1306_WHITE);
  else display.drawRect(startX, startY, boxSize, boxSize, SSD1306_WHITE);
  display.setCursor(startX + spacing, startY);
  display.print("Rec");

  // Квадратик 2
  if (stateButt2) display.fillRect(startX, startY + spacing, boxSize, boxSize, SSD1306_WHITE);
  else display.drawRect(startX, startY + spacing, boxSize, boxSize, SSD1306_WHITE);
  display.setCursor(startX + spacing, startY + spacing);
  display.print("Play");

  // Квадратик 3
  if (stateButt3) display.fillRect(startX, startY + 2 * spacing, boxSize, boxSize, SSD1306_WHITE);
  else display.drawRect(startX, startY + 2 * spacing, boxSize, boxSize, SSD1306_WHITE);
  display.setCursor(startX + spacing, startY + 2 * spacing);
  display.print("Stream");

  // Квадратик 4
  if (stateButt4) display.fillRect(startX, startY + 3 * spacing, boxSize, boxSize, SSD1306_WHITE);
  else display.drawRect(startX, startY + 3 * spacing, boxSize, boxSize, SSD1306_WHITE);
  display.setCursor(startX + spacing, startY + 3 * spacing);
  display.print("Reset");

  // Отображение координат углов
  display.setCursor(60, 5);
  display.print("S1:");
  display.print(myData.Serv1);

  display.setCursor(60, 15);
  display.print("S2:");
  display.print(myData.Serv2);

  display.setCursor(60, 25);
  display.print("S3:");
  display.print(myData.Serv3);

  display.setCursor(60, 35);
  display.print("S4:");
  display.print(myData.Serv4);

  // Обновление экрана
  display.display();
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

  // Проверка кнопок и обновление их состояний
  if (digitalRead(BUTTON_PIN) == LOW) stateButt1 = !stateButt1;
  if (digitalRead(BUTTON_PIN2) == LOW) stateButt2 = !stateButt2;
  if (digitalRead(BUTTON_PIN3) == LOW) stateButt3 = !stateButt3;
  if (digitalRead(BUTTON_PIN4) == LOW) stateButt4 = !stateButt4;

  // Кнопки в структуру
  myData.Butt1 = stateButt1 ? 1 : 0;
  myData.Butt2 = stateButt2 ? 1 : 0;
  myData.Butt3 = stateButt3 ? 1 : 0;
  myData.buttRightHand = stateButt4 ? 1 : 0;

  // Отправка данных через ESP
  TwoWayESP::SendBytes(&myData, sizeof(ServoAngles));

  // Отображение состояния на OLED
  drawScreen();

  delay(200);
}
