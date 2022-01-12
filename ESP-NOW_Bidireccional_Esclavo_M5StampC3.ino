// https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_now.html
// https://github.com/espressif/arduino-esp32/blob/master/libraries/ESP32/examples/ESPNow/Basic/Slave/Slave.ino

#include <esp_now.h>
#include <WiFi.h>

uint8_t M5Core2_STA_MAC[] = {0x44, 0x17, 0x93, 0x8A, 0x30, 0x34};
uint8_t M5Core2_AP_MAC[] = {0x44, 0x17, 0x93, 0x8A, 0x30, 0x35};

uint8_t M5StampC3_STA_MAC[] = {0x84, 0xF7, 0x03, 0x25, 0xAC, 0x24};
uint8_t M5StampC3_AP_MAC[] = {0x84, 0xF7, 0x03, 0x25, 0xAC, 0x25};

esp_now_peer_info_t esclavo;

struct trama {
  int id;
  int contador;
  char texto[80];
} mensajeEnviar, mensajeRecibido;

int id = 2;
bool debug = true;

bool enviarECO = false;

//---------------
void setup() {

  Serial.begin(115200);
  delay(5000);

  Serial.println("ESP-NOW ejemplo comunicacion bidireccional M5StampC3 - ROL ESCLAVO");
  
  if (!iniciarEsp_now()){
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

  iniciarEsclavo(M5Core2_STA_MAC);
}

//---------------
void loop() {

  if (enviarECO){

    // visualizo mensajeRecibido
    if (debug) {
      Serial.printf("mensajeRecibido.id = %d\n",mensajeRecibido.id);
      Serial.printf("mensajeRecibido.contador = %d\n",mensajeRecibido.contador);
      Serial.print("mensajeRecibido.texto = "); 
      Serial.println(mensajeRecibido.texto);
    }

    // envio respuesta al mensajeRecibido
    delay(100);
    if (!esp_now_is_peer_exist(M5Core2_STA_MAC)) {
      iniciarEsclavo(M5Core2_STA_MAC);
    } 
    enviarEsclavo(M5Core2_STA_MAC, id, mensajeRecibido.contador, "ECO desde ESCLAVO M5StampC3 a MASTER M5Core2");
    delay(250);
    
    enviarECO = false;
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
String macConvStr(uint8_t mac[]) {

  String ret;
  for (int i = 0; i < 6; i++) {
    ret += String((uint8_t) mac[i],HEX);
    if (i < 5) {
      ret += ":";
    }
  }
  return ret;
}

//---------------
bool iniciarEsclavo(uint8_t macEsclavo[]) {

  bool ret = false;

  // inicializo variable struc esclavo de tipo esp_now_peer_info_t
  for (int i = 0; i < 6; ++i ) {
    esclavo.peer_addr[i] = (uint8_t) macEsclavo[i];
  }
  esclavo.channel = 0;
  esclavo.encrypt = false;

  // agrego un par master-esclavo a la lista de pares
  esp_err_t addStatus = esp_now_add_peer(&esclavo);

  switch (addStatus) {
    case ESP_OK:
      Serial.print("esp_now_add_peer() = ESP_OK  -  MAC = ");
      Serial.print(macConvStr(macEsclavo));
      break;
    case ESP_ERR_ESPNOW_NOT_INIT:
      Serial.println("esp_now_add_peer() = ESP_ERR_ESPNOW_NOT_INIT");
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
      Serial.print("....<-Ok enviado a MASTER = ");
      Serial.println(macConvStr(macEnviar));
    case ESP_ERR_ESPNOW_NOT_INIT:
      //Serial.println("esp_now_send() = ESP_ERR_ESPNOW_NOT_INIT");
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

  Serial.print(status == ESP_NOW_SEND_SUCCESS ? "....<-Ok recibido por MASTER = " : "....<-Error NO recibe MASTER = ");
  Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X\n", macEnviado[0], macEnviado[1], macEnviado[2], macEnviado[3], macEnviado[4], macEnviado[5]);

}

//---------------
void OnMensajeRecibido(const uint8_t *macRecibido, const uint8_t *recMensaje, int len) {

  Serial.printf("->Ok recibidos %d bytes de MASTER = ",len);
  Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X\n", macRecibido[0], macRecibido[1], macRecibido[2], macRecibido[3], macRecibido[4], macRecibido[5]);;  
  
  memcpy(&mensajeRecibido, recMensaje, sizeof(mensajeRecibido));

  enviarECO = true;

}

//---------------
