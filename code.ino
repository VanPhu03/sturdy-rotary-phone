#define BLYNK_TEMPLATE_ID "TMPL6mwRV_tZm"
#define BLYNK_TEMPLATE_NAME "doan"
#define BLYNK_AUTH_TOKEN "Q26n5yYhSRIRmRKC34KzInFf2qwkYgbm"

#include <DHT.h>
#include <Wire.h>
//#include <WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <BlynkSimpleEsp8266.h>
// Cấu hình LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Địa chỉ LCD, cột, dòng

char ssid[] = "Zuzi";
char pass[] = "135797531";

// Cấu hình chân và kiểu cảm biến
#define DHTPIN 0       // IO0 (D3 trên NodeMCU)
#define DHTTYPE DHT11  // DHT11

#define MQ2 A0   // Chân cảm biến MQ2
#define BTN1 2   // Nút nhấn 1
#define BTN2 15  // Nút nhấn 2
#define COI 14   // Chân còi
#define QUAT 12  // Chân quạt
#define DEN 13   // Chân đèn

// Khởi tạo đối tượng DHT
DHT dht(DHTPIN, DHTTYPE);

// Biến trạng thái quạt và đèn
bool quatOn = false;  // Trạng thái quạt
bool denOn = false;   // Trạng thái đèn

void setup() {
  // Cấu hình các chân
  pinMode(MQ2, INPUT);
  pinMode(BTN1, INPUT_PULLUP);
  pinMode(BTN2, INPUT_PULLUP);
  pinMode(COI, OUTPUT);
  pinMode(DEN, OUTPUT);
  pinMode(QUAT, OUTPUT);

  // Tắt tất cả ban đầu
  digitalWrite(COI, LOW);
  digitalWrite(QUAT, LOW);
  digitalWrite(DEN, LOW);

  Serial.begin(74880);  // Bật Serial để debug
  dht.begin();           // Khởi động cảm biến DHT
  lcd.init();            // Khởi động LCD
  lcd.backlight();       // Bật đèn nền LCD
  lcd.print("He thong bat dau!");
  for (int i = 0; i < 16; i++) {
    lcd.setCursor(i, 1);
    lcd.print(".");
    delay(100);
  }
  lcd.clear();
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi");
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    for (int i = 0; i < 20; i++) {
      lcd.setCursor(i, 1);
      lcd.print(".");
      delay(100);
    }
    lcd.setCursor(0, 1);
    lcd.print("                    ");
  }
  lcd.setCursor(0, 1);
  lcd.print("     Successfull    ");
  delay(1000);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
}

void loop() {
  // Đọc nhiệt độ, độ ẩm và giá trị gas
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  int MQ2_value = analogRead(MQ2);
  int gas = (MQ2_value * 100) / 1024;

  // Kiểm tra nếu đọc thất bại
  if (isnan(humidity) || isnan(temperature)) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Loi DHT11!");
    Serial.println("Lỗi đọc dữ liệu từ DHT11!");
    delay(2000);
    return;
  }

  // Hiển thị dữ liệu trên Serial
  Serial.print("Độ ẩm: ");
  Serial.print(humidity);
  Serial.print(" %\t");
  Serial.print("Nhiệt độ: ");
  Serial.print(temperature);
  Serial.println(" *C");
  
  Serial.print("Nồng độ gas: ");
  Serial.print(gas);
  Serial.println(" %");

  // Điều khiển còi
  if (temperature > 40 || gas > 70) {
    digitalWrite(COI, HIGH);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("***CANH BAO!***");
  } else {
    digitalWrite(COI, LOW);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("TEMP:");
    lcd.print(temperature);
    lcd.print("C");

    lcd.setCursor(0, 1);
    lcd.print("GAS:");
    lcd.print(gas);
    lcd.print("%");

    // Hiển thị trạng thái quạt và đèn trên LCD
    lcd.setCursor(12, 0);
    lcd.print(quatOn ? "Q:On" : "Q:Off");

    lcd.setCursor(12, 1);
    lcd.print(denOn ? "D:On" : "D:Off");
  }

  // Điều khiển nút nhấn
  if (digitalRead(BTN1) == LOW) {
    while (digitalRead(BTN1) == LOW)
      ;
    quatOn = !quatOn;
    digitalWrite(QUAT, quatOn ? HIGH : LOW);
    Serial.println(quatOn ? "Quạt bật" : "Quạt tắt");
  }

  if (digitalRead(BTN2) == LOW) {
    while (digitalRead(BTN2) == LOW)
      ;
    denOn = !denOn;
    digitalWrite(DEN, denOn ? HIGH : LOW);
    Serial.println(denOn ? "Đèn bật" : "Đèn tắt");
  }

  // Đẩy dữ liệu lên Blynk
  Blynk.virtualWrite(V3, quatOn);
  Blynk.virtualWrite(V1, temperature);
  Blynk.virtualWrite(V2, humidity);
  Blynk.virtualWrite(V4, gas);
  Blynk.virtualWrite(V0, denOn);
  Blynk.run();
}

BLYNK_WRITE(V3) {
  quatOn = param.asInt();
  digitalWrite(QUAT, quatOn);
}

// Điều khiển quạt từ Blynk Button trên chân ảo V2
BLYNK_WRITE(V0) {
  denOn = param.asInt();
  digitalWrite(DEN, denOn);
}