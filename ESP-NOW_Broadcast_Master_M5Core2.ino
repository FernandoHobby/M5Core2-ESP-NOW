// https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_now.html
// https://github.com/espressif/arduino-esp32/blob/master/libraries/ESP32/examples/ESPNow/Basic/Master/Master.ino

#include <M5Core2.h>
#include <esp_now.h>
#include <WiFi.h>

uint8_t M5Core2_STA_MAC[] = {0x44, 0x17, 0x93, 0x8A, 0x30, 0x34};
uint8_t M5Core2_AP_MAC[] = {0x44, 0x17, 0x93, 0x8A, 0x30, 0x35};

uint8_t M5StampC3_STA_MAC[] = {0x84, 0xF7, 0x03, 0x25, 0xAC, 0x24};
uint8_t M5StampC3_AP_MAC[] = {0x84, 0xF7, 0x03, 0x25, 0xAC, 0x25};

uint8_t ESP32_STA_MAC[] = {0xF0, 0x08, 0xD1, 0xD1, 0xAF, 0x88};
uint8_t ESP32_AP_MAC[] = {0xF0, 0x08, 0xD1, 0xD1, 0xAF, 0x89};

uint8_t broadcastMAC[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

struct trama {
  int id;
  int contador;
  char texto[80];
} mensajeEnviar, mensajeRecibido;

int id = 1;
int contador = 0;
bool debug = true;

bool recibidoECO = false;

unsigned long tiempo = millis();
unsigned long periodoEnviar = 15000;

//---------------
void setup() {

  M5.begin(true,false,true,false);
  delay(5000);

  Serial.println("ESP-NOW ejemplo comunicacion broadcast M5Core2_STA_MAC -> ESCLAVOS");
  M5.Lcd.println("ESP-NOW ejemplo comunicacion broadcast M5Core2_STA_MAC -> ESCLAVOS");

  if (!iniciarEsp_now()) {
    Serial.println("Error al iniciar ESP-NOW, reinicio en 5 segundos");
    delay(5000);
    ESP.restart(); 
  }

  esp_err_t resultEnv = esp_now_register_send_cb(OnMensajeEnviado);
  Serial.print ("esp_now_register_send_cb = ");
  switch (resultEnv) {
    case ESP_OK:
      Serial.println("ESP_OK");
      break;
    case ESP_ERR_ESPNOW_NOT_INIT:
      Serial.println("ESP_ERR_ESPNOW_NOT_INIT : ESPNOW is not initialized");
      break;
    default:
      Serial.println("Error desconocido");
  }

  esp_err_t resultRec = esp_now_register_recv_cb(OnMensajeRecibido);
  Serial.print ("esp_now_register_recv_cb = ");
  switch (resultRec) {
    case ESP_OK:
      Serial.println("ESP_OK");
      break;
    case ESP_ERR_ESPNOW_NOT_INIT:
      Serial.println("ESP_ERR_ESPNOW_NOT_INIT : ESPNOW is not initialized");
      break;
    default:
      Serial.println("Error desconocido)");
  }
  
  iniciarEsclavo(broadcastMAC);
}   

//---------------
void loop() {

  if (millis() > tiempo + periodoEnviar){
    enviarEsclavo(broadcastMAC, id, contador++, "Envio desde MASTER M5SCore2 a broadcast ESCLAVO");  
    tiempo = millis();
  }
}

//---------------
bool iniciarEsp_now() {

  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  
  bool ret = true;
  if (esp_now_init() != ESP_OK) {
    Serial.println("esp_now_init() != ESP_OK");
    ret = false;
  } else {
    Serial.println("esp_now_init() == ESP_OK");
  }
  return ret;
}

//---------------
String macConvStr(uint8_t macConv[]) {

  String ret;
  for (int i = 0; i < 6; i++) {
    ret += String((uint8_t) macConv[i],HEX);
    if (i < 5) {
      ret += ":";
    }
  }
  return ret;
}

//---------------
bool iniciarEsclavo(uint8_t macEsclavo[]) {

  bool ret = false;
  
  esp_now_peer_info_t esclavo = {};
  for (int i = 0; i < 6; ++i ) {
    esclavo.peer_addr[i] = (uint8_t) macEsclavo[i];
  }
  esclavo.channel = 0;
  esclavo.encrypt = false;

  // agrego un par master-esclavo a la lista de pares
  esp_err_t addStatus = esp_now_add_peer(&esclavo);

  switch (addStatus) {
    case ESP_OK:
      Serial.print("Ok add esclavo a la lista = ");
      Serial.print(macConvStr(macEsclavo));
      break;
    case ESP_ERR_ESPNOW_NOT_INIT:
      Serial.println("ESP-NOW No iniciado");
      break;
    case ESP_ERR_ESPNOW_ARG:
      Serial.println("ESP_ERR_ESPNOW_ARG");
      break;
    case ESP_ERR_ESPNOW_FULL:
      Serial.println("ESP_ERR_ESPNOW_FULL");
      break;
    case ESP_ERR_ESPNOW_NO_MEM:
      Serial.println("ESP_ERR_ESPNOW_NO_MEM");
      break;
    case ESP_ERR_ESPNOW_EXIST:
      Serial.println("ESP_ERR_ESPNOW_EXIST");
      break;
    default:
    Serial.println("Error desconocido");
  }
  Serial.println("");
  return ret;
}

//---------------
void enviarEsclavo(uint8_t macEnviar[], int id, int contador, String texto) {

  mensajeEnviar.id = id;
  mensajeEnviar.contador = contador;
  strcpy(mensajeEnviar.texto, texto.c_str());

  esp_err_t result = esp_now_send(macEnviar, (uint8_t *) &mensajeEnviar, sizeof(mensajeEnviar));

  switch (result) {
    case ESP_OK:
      if(debug) { 
        Serial.print("....<-Ok enviado a ESCLAVO = ");
        Serial.println(macConvStr(macEnviar));
      }
      break;
    case ESP_ERR_ESPNOW_NOT_INIT:
      Serial.println("ESP-NOW no iniciado");
      break;
    case ESP_ERR_ESPNOW_ARG:
      Serial.println("ESP_ERR_ESPNOW_ARG");
      break;
    case ESP_ERR_ESPNOW_INTERNAL:
      Serial.println("ESP_ERR_ESPNOW_INTERNAL");
      break;
    case ESP_ERR_ESPNOW_NO_MEM:
      Serial.println("ESP_ERR_ESPNOW_NO_MEM");
      break;
    case ESP_ERR_ESPNOW_NOT_FOUND:
      Serial.println("ESP_ERR_ESPNOW_NOT_FOUND");
      break;
    default:
      Serial.println("Error desconocido");
 }
}

//---------------
void OnMensajeEnviado(const uint8_t *macEnviado, esp_now_send_status_t status) {

  Serial.print(status == ESP_NOW_SEND_SUCCESS ? "....<-Ok recibido por ESCLAVO = " : "....<-Error NO recibe esclavo = ");
  Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X\n", macEnviado[0], macEnviado[1], macEnviado[2], macEnviado[3], macEnviado[4], macEnviado[5]);
}

//---------------
void OnMensajeRecibido(const uint8_t *macRecibido, const uint8_t *recMensaje, int len) {
  
  Serial.printf("->Ok recibidos %d bytes de ESCLAVO = ",len);
  Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X\n", macRecibido[0], macRecibido[1], macRecibido[2], macRecibido[3], macRecibido[4], macRecibido[5]);

  memcpy(&mensajeRecibido, recMensaje, sizeof(mensajeRecibido));

  recibidoECO = true;
}

//---------------
