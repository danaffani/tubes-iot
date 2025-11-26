#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "esp_camera.h"
#include "time.h" 

const char* ssid = "Linksys";
const char* password = "AbednegoLinksys"; 

String SUPABASE_URL = "https://teennmkmzhhltjxujccz.supabase.co";
String SUPABASE_KEY = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6InRlZW5ubWttemhobHRqeHVqY2N6Iiwicm9sZSI6ImFub24iLCJpYXQiOjE3NjMwNzk3ODUsImV4cCI6MjA3ODY1NTc4NX0.yXIbih2oMiw94Q-2gG-6Yh4ZGidHpPIpeuZDAwbH_DM"; 

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 25200; // UTC +7 (WIB)
const int   daylightOffset_sec = 0;

const int TRIG_PIN = 12;
const int ECHO_PIN = 13;
const int SDA_PIN = 14;
const int SCL_PIN = 15;
const int BUZZER_PIN = 2;

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

LiquidCrystal_I2C lcd(0x27, 16, 2); 
int triggerJarak = 100;
bool sedangAksi = false;
unsigned long WAKTU_POLLING_TERAKHIR = 0;
const long INTERVAL_POLLING = 5000; 

TaskHandle_t TaskAlarmHandle; 
volatile bool startAlarmSequence = false; // Pemicu
volatile int alarmCount = 0;
volatile int alarmDuration = 0;
volatile int alarmDelay = 0;

void TaskAlarmCode( void * pvParameters ){
  for(;;){
    // Cek apakah 'Bos' (Core 1) menyuruh bunyikan alarm?
    if (startAlarmSequence) {
      
      // Loop bunyi alarm
      for (int i = 0; i < alarmCount; i++) {
        digitalWrite(BUZZER_PIN, HIGH);
        vTaskDelay(alarmDuration / portTICK_PERIOD_MS); 
        digitalWrite(BUZZER_PIN, LOW);
        
        if (i < alarmCount - 1) {
           vTaskDelay(alarmDelay / portTICK_PERIOD_MS);
        }
      }
      // Reset pemicu setelah selesai bunyi
      startAlarmSequence = false; 
    }
    vTaskDelay(100 / portTICK_PERIOD_MS); // Istirahat sebentar (wajib)
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  
  Wire.begin(SDA_PIN, SCL_PIN);
  lcd.init();
  lcd.backlight();
  lcd.print("Booting System...");

  xTaskCreatePinnedToCore(
      TaskAlarmCode,   /* Fungsi tugas */
      "TaskAlarm",     /* Nama */
      2048,            /* Stack size */
      NULL,            /* Parameter */
      1,               /* Prioritas */
      &TaskAlarmHandle,/* Handle */
      0);              /* Core 0 (Background) */

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_VGA;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Kamera Gagal!");
    lcd.clear(); lcd.print("Cam Failed");
    return;
  }

  WiFi.begin(ssid, password);
  lcd.clear(); lcd.print("Connect WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
  
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  if(getLocalTime(&timeinfo)){
    Serial.println("Jam Update: OK");
  }
  
  lcd.clear(); lcd.print("System Ready!");
  delay(1000);
  lcd.clear(); lcd.print("Menunggu...");
}

void loop() {
  if (!sedangAksi) {
    int jarak = bacaJarakUltrasonik();
    
    if (jarak < triggerJarak && jarak > 0) {
      jalankanAksiTrigger();
    }

    unsigned long now = millis();
    if (now - WAKTU_POLLING_TERAKHIR > INTERVAL_POLLING) {
      WAKTU_POLLING_TERAKHIR = now;
      cekPerintahBaru();
    }
  }
  delay(100); 
}

void jalankanAksiTrigger() {
  sedangAksi = true;
  Serial.println(">>> TRIGGERED! MULTITASKING START <<<");

  lcd.clear();
  lcd.print("CEKREK! TERCIDUK");
  lcd.setCursor(0, 1);
  lcd.print("MENGUPLOAD...");
  
  alarmCount = 3;
  alarmDuration = 500; 
  alarmDelay = 100;    
  
  startAlarmSequence = true; 

  String urlFoto = ambilDanUploadFoto();
  
  if (urlFoto != "") {
    kirimLogKeSupabase("Objek terdeteksi!", urlFoto);
  }

  lcd.clear();
  lcd.print("Sistem Siaga...");
  sedangAksi = false;
}

String getNamaFileWaktu() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    return "foto_" + String(millis()) + ".jpg";
  }
  char timeStringBuff[50];
  strftime(timeStringBuff, sizeof(timeStringBuff), "foto_%Y-%m-%d_%H-%M-%S.jpg", &timeinfo);
  return String(timeStringBuff);
}

int bacaJarakUltrasonik() {
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  return duration * 0.034 / 2;
}

String ambilDanUploadFoto() {
  camera_fb_t * fb = esp_camera_fb_get();
  esp_camera_fb_return(fb); 
  delay(200); 

  Serial.println("Core 1: Mengambil Foto...");
  fb = esp_camera_fb_get();
  if (!fb) return "";

  Serial.println("Core 1: Mengupload...");
  HTTPClient http;
  String namaFile = getNamaFileWaktu();
  String urlUpload = SUPABASE_URL + "/storage/v1/object/foto_iot/" + namaFile;
  
  http.begin(urlUpload);
  http.addHeader("Authorization", "Bearer " + SUPABASE_KEY);
  http.addHeader("apikey", SUPABASE_KEY);
  http.addHeader("Content-Type", "image/jpeg");

  int httpResponseCode = http.POST(fb->buf, fb->len);
  esp_camera_fb_return(fb);

  if (httpResponseCode == 200) {
    Serial.println("Core 1: Upload Sukses!");
    return SUPABASE_URL + "/storage/v1/object/public/foto_iot/" + namaFile;
  } else {
    Serial.print("Core 1: Upload Gagal. Code: "); Serial.println(httpResponseCode);
    return "";
  }
  http.end();
}

void kirimLogKeSupabase(String pesan, String urlFoto) {
  StaticJsonDocument<200> doc;
  doc["pesan"] = pesan;
  doc["url_foto"] = urlFoto;
  String jsonBody;
  serializeJson(doc, jsonBody);

  HTTPClient http;
  http.begin(SUPABASE_URL + "/rest/v1/log_deteksi");
  http.addHeader("Authorization", "Bearer " + SUPABASE_KEY);
  http.addHeader("apikey", SUPABASE_KEY);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Prefer", "return=minimal");
  http.POST(jsonBody);
  http.end();
}

void cekPerintahBaru() {
  Serial.println("Cek perintah...");
  HTTPClient http;
  String urlGet = SUPABASE_URL + "/rest/v1/perintah_device?sudah_dieksekusi=eq.false&limit=1&order=created_at.asc";
  
  http.begin(urlGet);
  http.addHeader("Authorization", "Bearer " + SUPABASE_KEY);
  http.addHeader("apikey", SUPABASE_KEY);
  
  if (http.GET() == 200) {
    String payload = http.getString();
    StaticJsonDocument<1024> doc;
    deserializeJson(doc, payload);

    if (doc.size() > 0) { 
      JsonObject cmd = doc[0];
      long id = cmd["id"];
      String tipe = cmd["tipe_perintah"];
      JsonObject data = cmd["payload"];

      bool sukses = false;
      if (tipe == "alarm") {
         alarmCount = data["count"];
         alarmDuration = data["duration"];
         alarmDelay = data["delay"];
         startAlarmSequence = true;
         sukses = true;
      } 
      else if (tipe == "lcd") {
         String t = data["teks"];
         lcd.clear(); lcd.print(t.substring(0,16));
         if(t.length()>16) { lcd.setCursor(0,1); lcd.print(t.substring(16,32)); }
         sukses = true;
      }
      else if (tipe == "jarak") {
         triggerJarak = data["jarak"];
         lcd.clear(); lcd.print("Jarak Baru:"); lcd.setCursor(0,1); lcd.print(triggerJarak);
         sukses = true;
      }
      else if (tipe == "foto") {
         String url = ambilDanUploadFoto();
         if(url != "") {
           kirimLogKeSupabase("Foto Request Web", url);
           sukses = true;
         }
      }

      if (sukses) {
         HTTPClient httpUpdate;
         httpUpdate.begin(SUPABASE_URL + "/rest/v1/perintah_device?id=eq." + String(id));
         httpUpdate.addHeader("Authorization", "Bearer " + SUPABASE_KEY);
         httpUpdate.addHeader("apikey", SUPABASE_KEY);
         httpUpdate.addHeader("Content-Type", "application/json");
         httpUpdate.PATCH("{\"sudah_dieksekusi\": true}");
         httpUpdate.end();
      }
    }
  }
  http.end();
}