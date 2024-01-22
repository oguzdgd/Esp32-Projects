#include <LiquidCrystal_I2C.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <WiFi.h> 
#include <ArduinoJson.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);  //I2C led 


WiFiMulti wifiMulti;
String sunucuAdresi;

int buzzerPin = 5;  //Buzzer'ın bağlı olduğu pin (artı uç) (eksi uç toprağa bağlanır)
int greenLed = 32;     
int redLed = 33;

int buttonUpPin = 2;  
int buttonDownPin = 14;
int buttonOnayPin = 4;

int currentColumn = 1;  // leddeki sütun kontrolü için
int can = 3;            // oyundaki can hakkı

int sayi = 0;
String pass = "0000"; 
int password[4];


int melodi[] = {262, 294, 330, 349, 392, 440, 494, 523};  // Oyun sonu tebrikler mesajı icin


void setup() {
  Serial.begin(9600);

  pinMode(buzzerPin, OUTPUT);   // Buzzer pin
  pinMode(greenLed, OUTPUT);   // Yeşil led, kullanıcı doğru tahmin ettiğinde yanması için
  pinMode(redLed, OUTPUT);    // Kırmızı led, kullanıcı yanlış tahmin ettiğinde yanması için

  pinMode(buttonUpPin, INPUT_PULLUP);
  pinMode(buttonDownPin, INPUT_PULLUP);
  pinMode(buttonOnayPin, INPUT_PULLUP);

  lcd.init();       // lcd ekranı başlatır
  lcd.backlight();  // lcd ekranın arka plan ışığını açar.

  lcd.print("OYUN BASLADI");
  lcd.clear();

  lcd.setCursor(1, 0); //setCursor lcd ekrandaki sütün ve satırı belirtir. Sol parametre sütun, sağ parametre satırı belirler
  lcd.print("****");
  lcd.setCursor(currentColumn, 1);
  lcd.print("_");

  lcd.setCursor(6, 0);
  lcd.print("Can: " + String(can));
 
  wifiMulti.addAP("Redmi", "oguz0123");  // wifi'ye bağlanmak icin

    if ((wifiMulti.run() == WL_CONNECTED)) {
    Serial.print("Wifi'ye baglandi\n");
    delay(1000);

    sunucuAdresi = "http://192.168.113.237:5000/get_random_number"; // 
    Serial.println("Adres: " + sunucuAdresi);
    delay(5000);

    HTTPClient http;
    
    http.begin(sunucuAdresi);  
    http.setConnectTimeout(30000);
    http.setTimeout(30000);
    int httpCode = http.GET();
    
    if (httpCode > 0) {
      String jsonString = http.getString();
      Serial.print("Response: ");
      Serial.println(jsonString);

      
      const size_t capacity = JSON_OBJECT_SIZE(1) + 20;
      DynamicJsonDocument doc(capacity); // DynamicJsonDocument nesnesi oluşturularak JSON verisi ayrıştırılır. 
      deserializeJson(doc, jsonString);  // Bu, ArduinoJSON kütüphanesini kullanarak gerçekleştirilir.

    
      int randomNumber = doc["randomNumber"]; //JSON dokümanından rasgele sayı (randomNumber) çıkartılır.
      
    
      for (int i = 0; i < 4; i++) { // for döngüsü ile bu rasgele sayının her bir basamağı ayrı ayrı password dizisine atanır.
          password[i] = (randomNumber / int(pow(10, 3 - i))) % 10;
      }

       delay(5000);
      } else {
        Serial.println("HTTP GET islemi başarisiz, hata: %s\n", http.errorToString(httpCode).c_str());
        Serial.println("HTTP Cevabi: " + http.getString());  //Eğer hata varsa ekrana yazdırılır.
      }
      http.end(); // HTTP bağlantısı kapatılır.
    } 
}

void loop() {
    if (Serial.available() > 0) { // seri bağlantı üzerinde bir veri mevcut mu diye kontrol edilir. 
    char incomingChar = Serial.read(); //Serial read, seri bağlantıdan bir karakter okur ve onu incomingChar değerine atar.
   processCommand(incomingChar);
  }
}

void processCommand(char command) {
  
  // Gelen komutu işle
  switch (command) {

    case 'W':
       lcd.setCursor(currentColumn, 1);
        sayi = sayi + 1;
        Serial.println("Sayi 1 artirildi.");
        if (sayi > 9) {
          sayi = 9;
        }
        lcd.print(sayi);
        Serial.println("Yeni sayi :" + String(sayi));
        lcd.setCursor(6, 0);
        lcd.print("Can: " + String(can));
        break;
    case 'S':
        lcd.setCursor(currentColumn, 1);
        sayi = sayi - 1;
        Serial.println("Sayi 1 azaltildi.");
        if (sayi < 0) {
          sayi = 0;
        }
        lcd.print(String(sayi));
        Serial.println("Yeni sayi :" + String(sayi));
        lcd.setCursor(6, 0);
        lcd.print("Can: " + String(can));
      break;

    case 'D':
      if (sayi == password[currentColumn - 1]) {
        Serial.println("Dogru Tahmin!");
        digitalWrite(greenLed, HIGH); // Doğru tahminde yeşil led yanar
        delay(1000);
        digitalWrite(greenLed, LOW);
        delay(1000);
        lcd.setCursor(currentColumn, 1);
        lcd.print(" ");
        currentColumn = currentColumn + 1;
        lcd.setCursor(currentColumn, 1);
        lcd.print("_");
        lcd.setCursor(6, 0);
        lcd.print("Can: " + String(can));
      } else {
        Serial.println("Yanlis Tahmin!");
        digitalWrite(redLed, HIGH); // yanlış tahminde kırmızı led yanar.
        delay(1000);
        digitalWrite(redLed, LOW);
        delay(1000);

        if (sayi > password[currentColumn - 1]) {
          tone(buzzerPin, 1400); // tahmin edilen index, şifredeki indexten büyükse buzzer çalışır.
          delay(300); // buzzer çalışma süresi
          noTone(buzzerPin); // buzzerı kapatır
        
        }

        if (sayi < password[currentColumn - 1]) {
          tone(buzzerPin, 700);
          delay(300);
          noTone(buzzerPin);
        
        }

        can = can - 1;
        lcd.setCursor(6, 0);
        lcd.print("Can: " + String(can));

        if (can == 0) {
          lcd.clear(); // lcd ekranı temizler
          lcd.setCursor(1, 1);
          lcd.print("Tekrar Dene");
          delay(2000);
        }
      }
      break;
      }
  lcdrender();
}

void lcdrender() { 
  lcd.setCursor(1, 0);
  if (currentColumn == 1) {
    lcd.print("****");
  }

  if (currentColumn == 2) {
    lcd.print(String(password[0]) + "***");
  }

  if (currentColumn == 3) {
    lcd.print(String(password[0]) + String(password[1]) + "**");
  }

  if (currentColumn == 4) {
    lcd.print(String(password[0]) + String(password[1]) + String(password[2]) + "*");
  }

  if (currentColumn == 5) {
    lcd.clear();
    lcd.setCursor(0, 0);
    for (int i = 0; i < 4; i++) {
      lcd.print(password[i]);  
    }
    lcd.setCursor(1, 1);
    lcd.print("TEBRIKLER");
    playMelody();
    delay(3000);
  }
}

void playMelody() { // bu fonksiyon kullanıcı şifreyi doğru tahmin ettiği zaman çalan bir melodi
  for (int i = 0; i < 8; i++) {
    tone(buzzerPin, melodi[i]);
    delay(200);
    noTone(buzzerPin);
    delay(50);
  }
}
