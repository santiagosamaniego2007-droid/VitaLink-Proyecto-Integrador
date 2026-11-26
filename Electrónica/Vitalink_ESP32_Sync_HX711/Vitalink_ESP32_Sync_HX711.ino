/*
 VITALINK - Demo integrada ESP32 DevKit V1
 Sincroniza horarios y confirmaciones de una cuenta de prueba en Supabase.
 Usa DS3231 (RTC), LCD 16x2 I2C (PCF8574), LED rojo y verde, buzzer activo
 y pulsador. HX711 incluido para variaciones de peso; se habilita solo tras comprobar pines y calibrar.
 No es un dispositivo medico ni acredita ingesta del medicamento.

 ANTES DE FLASHEAR: revisar config.h contra el cableado REAL. GPIO6-11
 son pines de memoria flash y no deben usarse para pulsadores en DevKit V1.

 Requiere: esp32 by Espressif Systems, biblioteca ArduinoJson v7.
 Si se habilita HX711: biblioteca HX711 de Bogde; calibracion obligatoria.
 Si se habilita servo: biblioteca ESP32Servo, fuente externa y comprobacion de topes.
 La LCD PCF8574 y DS3231 se manejan por Wire sin bibliotecas adicionales.
 Monitor Serie 115200: 's'=sincronizar, 't'=confirmar, 'p'=mostrar peso, 'r'=reiniciar referencia de peso.
*/
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <time.h>
#include "config.h"
// Validaciones para no utilizar pines reservados o ya ocupados.
#if VITALINK_HX711_HABILITADO
#if (HX711_PIN_DT >= 6 && HX711_PIN_DT <= 11) || (HX711_PIN_SCK >= 6 && HX711_PIN_SCK <= 11)
#error "HX711 no puede usar GPIO 6-11 del ESP32 DevKit V1: memoria flash"
#endif
#if HX711_PIN_DT == HX711_PIN_SCK || HX711_PIN_DT == PIN_BOTON || HX711_PIN_DT == PIN_LED_ROJO || HX711_PIN_DT == PIN_LED_VERDE || HX711_PIN_DT == PIN_BUZZER || HX711_PIN_DT == PIN_SDA || HX711_PIN_DT == PIN_SCL || HX711_PIN_SCK == PIN_BOTON || HX711_PIN_SCK == PIN_LED_ROJO || HX711_PIN_SCK == PIN_LED_VERDE || HX711_PIN_SCK == PIN_BUZZER || HX711_PIN_SCK == PIN_SDA || HX711_PIN_SCK == PIN_SCL
#error "Conflicto de GPIO: comparar config.h con el cableado real"
#endif
#if HX711_PIN_SCK == 34 || HX711_PIN_SCK == 35 || HX711_PIN_SCK == 36 || HX711_PIN_SCK == 39
#error "El pin SCK del HX711 requiere salida; GPIO 34/35/36/39 son solo entrada"
#endif
#endif

#if VITALINK_HX711_HABILITADO
#include <HX711.h>
#include <math.h>
#endif

#if VITALINK_SERVO_HABILITADO
#if (PIN_SERVO >= 6 && PIN_SERVO <= 11) || PIN_SERVO == 34 || PIN_SERVO == 35 || PIN_SERVO == 36 || PIN_SERVO == 39
#error "Pin de servo no valido para ESP32 DevKit V1"
#endif
#if PIN_SERVO == PIN_BOTON || PIN_SERVO == PIN_LED_ROJO || PIN_SERVO == PIN_LED_VERDE || PIN_SERVO == PIN_BUZZER || PIN_SERVO == PIN_SDA || PIN_SERVO == PIN_SCL
#error "GPIO de servo en conflicto con otro componente"
#endif
#if VITALINK_HX711_HABILITADO && (PIN_SERVO == HX711_PIN_DT || PIN_SERVO == HX711_PIN_SCK)
#error "GPIO de servo en conflicto con HX711"
#endif
#if SERVO_ANGULO_CERRADO < 0 || SERVO_ANGULO_CERRADO > 180 || SERVO_ANGULO_ABIERTO < 0 || SERVO_ANGULO_ABIERTO > 180
#error "Los angulos de servo deben estar entre 0 y 180"
#endif
#include <ESP32Servo.h>
#endif

#include "secrets.h"

#ifndef VITALINK_DEVICE_ID
#define VITALINK_DEVICE_ID ""
#endif
#ifndef VITALINK_FUNCTIONS_BASE
#define VITALINK_FUNCTIONS_BASE "https://TU-PROYECTO.supabase.co/functions/v1/"
#endif
const char* FUNCTION_URL = VITALINK_FUNCTIONS_BASE "vitalink-device-events";
const char* PAIRING_URL = VITALINK_FUNCTIONS_BASE "vitalink-device-pairing";
const bool dispositivoGestionado = sizeof(VITALINK_DEVICE_ID) > 1;
String vinculo;
String codigoVinculacion;
unsigned long inicioCodigo = 0;
unsigned long duracionCodigo = 0;
unsigned long ultimaConsultaVinculo = 0;
constexpr uint8_t RTC_ADDR = 0x68;

struct FechaHora {
  int anio, mes, dia, hora, minuto, segundo;
};
struct Recordatorio {
  uint32_t medId;
  String nombre;
  String horaTexto;
  int minutoDia;
  int compartimento;
  bool confirmadoHoy;
};
Recordatorio recordatorios[MAX_RECORDATORIOS];
int totalRecordatorios = 0;
String fechaDeHorario = "";
String mensajeEstado = "Sin sincronizar";
bool recibioHorario = false;
unsigned long ultimaConsulta = 0;
unsigned long ultimoIntentoWiFi = 0;
unsigned long ultimaHoraRTC = 0;
unsigned long luzVerdeHasta = 0;
unsigned long mostrarErrorHasta = 0;
int lecturaAnterior = HIGH;
int lecturaEstable = HIGH;
unsigned long cambioBoton = 0;
bool ntpIniciado = false;

#if VITALINK_SERVO_HABILITADO
Servo servoTapa;
bool servoPreparado = false;
bool tapaAbierta = false;

void iniciarTapaMotorizada() {
  servoTapa.setPeriodHertz(50);
  servoTapa.attach(PIN_SERVO, 500, 2400);
  servoPreparado = servoTapa.attached();
  if (!servoPreparado) {
    Serial.println("Servo no disponible. Revisar biblioteca, GPIO y alimentacion.");
    return;
  }
  servoTapa.write(SERVO_ANGULO_CERRADO);
  tapaAbierta = false;
  Serial.println("Servo preparado: tapa cerrada.");
}

void actualizarTapaMotorizada(int pendiente) {
  if (!servoPreparado) return;
  // Abrir solo cuando existe una dosis pendiente y no estamos en modo vinculacion.
  const bool debeAbrir = pendiente >= 0 && codigoVinculacion.isEmpty()
      && (!dispositivoGestionado || !vinculo.isEmpty());
  if (debeAbrir == tapaAbierta) return;
  servoTapa.write(debeAbrir ? SERVO_ANGULO_ABIERTO : SERVO_ANGULO_CERRADO);
  tapaAbierta = debeAbrir;
  Serial.println(debeAbrir ? "Servo: abrir tapa por recordatorio." :
                               "Servo: cerrar tapa al terminar recordatorio.");
}
#endif

bool iniciarHTTPS(HTTPClient& http);
bool abrirHTTPS(HTTPClient& http, const char* url);
void limpiarVinculo();

// === HX711 / celda de carga: DETECCION DE CAMBIOS, NO ingesta ===
// Desactivado por defecto: NO activar sin confirmar pines y factor de calibracion.
#if VITALINK_HX711_HABILITADO
HX711 bascula;
bool basculaLista = false;
bool referenciaArmada = false; // Requiere r desde Monitor Serie una vez colocada la carga.
float referenciaPesoG = NAN;
float candidatoPesoG = NAN;
uint8_t lecturasEstables = 0;
unsigned long ultimaLecturaPeso = 0;
unsigned long ultimoEventoPeso = 0;

bool enviarCambioPeso(float antes, float despues) {
  if (dispositivoGestionado && vinculo.isEmpty()) return false;
  if (!isfinite(antes) || !isfinite(despues)) return false;
  // La funcion beta recibe PESOS ABSOLUTOS no negativos entre 0 y 5000 g.
  if (antes < 0 || despues < 0 || antes > 5000 || despues > 5000) {
    Serial.println("HX711: pesos fuera de rango. Verificar tara/calibracion.");
    return false;
  }
  HTTPClient http;
  if (!iniciarHTTPS(http)) return false;
  http.addHeader("Content-Type", "application/json");
  JsonDocument payload;
  payload["event"] = "weight_changed";
  payload["previous_weight_g"] = roundf(antes * 100.0f) / 100.0f;
  payload["current_weight_g"] = roundf(despues * 100.0f) / 100.0f;
  String body;
  serializeJson(payload, body);
  const int status = http.POST(body);
  const String response = status > 0 ? http.getString() : HTTPClient::errorToString(status);
  http.end();
  if (dispositivoGestionado && (status == 401 || status == 412)) limpiarVinculo();
  Serial.printf("HX711 -> evento: HTTP %d %s\n", status, response.c_str());
  return status == 201;
}

void iniciarBascula() {
  if (fabsf(HX711_FACTOR_CALIBRACION) < 0.001f) {
    Serial.println("HX711 DESACTIVADO: falta factor de calibracion en config.h.");
    return;
  }
  bascula.begin(HX711_PIN_DT, HX711_PIN_SCK);
  if (!bascula.wait_ready_timeout(2000)) {
    Serial.println("HX711 no responde: revisar DAT/SCK/3.3V/GND.");
    return;
  }
  // La tara se hace con el plato/contenedor SIN la carga que se quiere detectar.
  bascula.set_scale(HX711_FACTOR_CALIBRACION);
  bascula.tare(12);
  basculaLista = true;
  Serial.println("HX711 listo. Coloca los objetos y escribe r para fijar la referencia antes de notificar.");
}

void mostrarPeso() {
  if (!basculaLista || !bascula.wait_ready_timeout(1200)) {
    Serial.println("HX711 no disponible."); return;
  }
  float peso = bascula.get_units(5);
  Serial.printf("HX711 peso actual: %.2f g; referencia: %.2f g; armado: %s\n",
    peso, referenciaPesoG, referenciaArmada ? "SI" : "NO");
}

void armarReferenciaPeso() {
  if (!basculaLista || !bascula.wait_ready_timeout(1200)) {
    Serial.println("HX711 no listo: no se armo la referencia."); return;
  }
  const float peso = bascula.get_units(10);
  if (!isfinite(peso) || peso < 0 || peso > 5000) {
    Serial.println("Peso fuera de rango 0..5000g. Revisar factor, tara y montaje."); return;
  }
  referenciaPesoG = peso;
  candidatoPesoG = peso;
  lecturasEstables = 0;
  referenciaArmada = true;
  // No enviar notificacion por la simple colocacion inicial.
  Serial.printf("HX711 ARMADO, peso inicial %.2f g. Ahora detectara cambios estables.\n", peso);
}

void revisarPeso() {
  if (!referenciaArmada) return;
  if (!basculaLista || millis() - ultimaLecturaPeso < HX711_CADA_MS) return;
  ultimaLecturaPeso = millis();
  if (!bascula.is_ready()) return;
  const float peso = bascula.get_units(5); // Promedio de 5 muestras (puede tardar ~0.5 s).
  if (!isfinite(peso) || peso < 0 || peso > 5000) return;
  // Las variaciones pequeñas se tratan como ruido instrumental.
  if (fabsf(peso - referenciaPesoG) < HX711_UMBRAL_CAMBIO_G) {
    candidatoPesoG = peso;
    lecturasEstables = 0;
    return;
  }
  if (!isfinite(candidatoPesoG) || fabsf(peso - candidatoPesoG) > HX711_TOLERANCIA_ESTABILIDAD_G) {
    candidatoPesoG = peso;
    lecturasEstables = 1;
    return;
  }
  ++lecturasEstables;
  if (lecturasEstables < 3 || millis() - ultimoEventoPeso < HX711_COOLDOWN_MS) return;
  // Si no hay red, no se modifica referenciaPesoG: se reintentara mas adelante.
  if (enviarCambioPeso(referenciaPesoG, peso)) {
    Serial.printf("HX711 cambio guardado: %.1f -> %.1f g\n", referenciaPesoG, peso);
    referenciaPesoG = peso;
    ultimoEventoPeso = millis();
  }
  lecturasEstables = 0;
}
#endif

// === I2C: LCD 1602 habitual con expansor PCF8574 ===
// Conexion tipica PCF8574: P0=RS, P1=RW, P2=E, P3=Backlight, P4-P7=datos.
// Algunos adaptadores tienen otra distribucion y requieren otra biblioteca.
void pcfWrite(uint8_t data) {
  Wire.beginTransmission(LCD_DIRECCION);
  Wire.write(data);
  Wire.endTransmission();
}
void lcdNibble(uint8_t nibble, bool rs) {
  uint8_t v = (nibble << 4) | (rs ? 0x01 : 0) | 0x08;
  pcfWrite(v); pcfWrite(v | 0x04); delayMicroseconds(1);
  pcfWrite(v); delayMicroseconds(55);
}
void lcdByte(uint8_t value, bool rs) {
  lcdNibble(value >> 4, rs);
  lcdNibble(value & 0x0F, rs);
}
void lcdCommand(uint8_t cmd) {
  lcdByte(cmd, false);
  if (cmd == 0x01 || cmd == 0x02) delay(2);
}
void lcdInit() {
  delay(50);
  lcdNibble(0x03, false); delay(5);
  lcdNibble(0x03, false); delayMicroseconds(200);
  lcdNibble(0x03, false); delayMicroseconds(200);
  lcdNibble(0x02, false); // Modo 4 bits
  lcdCommand(0x28); lcdCommand(0x0C); lcdCommand(0x06); lcdCommand(0x01);
}
void lcdLinea(int fila, const String& contenido) {
  lcdCommand(fila == 0 ? 0x80 : 0xC0);
  String s = contenido.substring(0, 16);
  while (s.length() < 16) s += ' ';
  for (size_t i = 0; i < s.length(); ++i) lcdByte((uint8_t)s[i], true);
}

// === I2C: DS3231 ===
uint8_t bcdAInt(uint8_t value) { return (value >> 4) * 10 + (value & 0x0F); }
uint8_t intABcd(uint8_t value) { return ((value / 10) << 4) | (value % 10); }

bool rtcLeer(FechaHora& h) {
  Wire.beginTransmission(RTC_ADDR);
  Wire.write((uint8_t)0x00);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom(RTC_ADDR, (uint8_t)7) != 7) return false;
  uint8_t sec = Wire.read(), min = Wire.read(), hr = Wire.read();
  Wire.read(); // Dia semana
  uint8_t dia = Wire.read(), mes = Wire.read(), anio = Wire.read();
  if (hr & 0x40) return false; // Se espera modo 24 horas.
  h.segundo = bcdAInt(sec & 0x7F);
  h.minuto = bcdAInt(min & 0x7F);
  h.hora = bcdAInt(hr & 0x3F);
  h.dia = bcdAInt(dia & 0x3F);
  h.mes = bcdAInt(mes & 0x1F);
  h.anio = 2000 + bcdAInt(anio);
  if (h.anio < 2024 || h.anio > 2099 || h.mes < 1 || h.mes > 12 ||
      h.dia < 1 || h.dia > 31 || h.hora > 23 || h.minuto > 59 || h.segundo > 59) return false;
  // Rechazar el reloj cuando el oscilador se ha detenido (OSF).
  Wire.beginTransmission(RTC_ADDR);
  Wire.write((uint8_t)0x0F);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom(RTC_ADDR, (uint8_t)1) != 1) return false;
  return (Wire.read() & 0x80) == 0;
}

bool rtcEscribirDesdeNTP() {
  time_t utc = time(nullptr);
  if (utc < 1700000000) return false;
  // Ecuador continental, UTC-5 sin cambio de verano.
  utc -= 5 * 3600;
  struct tm t;
  gmtime_r(&utc, &t);
  Wire.beginTransmission(RTC_ADDR);
  Wire.write((uint8_t)0x00);
  Wire.write(intABcd(t.tm_sec)); Wire.write(intABcd(t.tm_min)); Wire.write(intABcd(t.tm_hour));
  Wire.write(intABcd(t.tm_wday + 1)); Wire.write(intABcd(t.tm_mday));
  Wire.write(intABcd(t.tm_mon + 1)); Wire.write(intABcd(t.tm_year + 1900 - 2000));
  if (Wire.endTransmission() != 0) return false;
  // Borra OSF tras poner una hora valida.
  Wire.beginTransmission(RTC_ADDR); Wire.write((uint8_t)0x0F);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom(RTC_ADDR, (uint8_t)1) != 1) return false;
  const uint8_t estado = Wire.read();
  Wire.beginTransmission(RTC_ADDR); Wire.write((uint8_t)0x0F);
  Wire.write((uint8_t)(estado & ~0x80));
  return Wire.endTransmission() == 0;
}

String fechaLocal(const FechaHora& h) {
  char out[11];
  snprintf(out, sizeof(out), "%04d-%02d-%02d", h.anio, h.mes, h.dia);
  return String(out);
}

bool tokenValido() {
  if (strlen(DEVICE_TOKEN) != 64) return false;
  for (int i = 0; i < 64; ++i) {
    const char c = DEVICE_TOKEN[i];
    if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))) return false;
  }
  return true;
}

bool conectarWiFi() {
  if (WiFi.status() == WL_CONNECTED) return true;
  if (millis() - ultimoIntentoWiFi < 15000UL && ultimoIntentoWiFi != 0) return false;
  ultimoIntentoWiFi = millis();
  Serial.println("Conectando Wi-Fi 2.4 GHz...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  const unsigned long inicio = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - inicio < 16000UL) delay(200);
  if (WiFi.status() != WL_CONNECTED) { Serial.println("Wi-Fi no disponible."); return false; }
  Serial.println("Wi-Fi conectado.");
  return true;
}

bool ntpListo() {
  if (!ntpIniciado) {
    configTime(0, 0, "pool.ntp.org", "time.google.com");
    ntpIniciado = true;
  }
  if (time(nullptr) > 1700000000) return true;
  const unsigned long inicio = millis();
  while (millis() - inicio < 12000UL) {
    if (time(nullptr) > 1700000000) return true;
    delay(400);
  }
  Serial.println("NTP no disponible: no se hace HTTPS todavia.");
  return false;
}

void encenderBuzzer(bool encender) {
  digitalWrite(PIN_BUZZER,
    encender ? (BUZZER_ACTIVO_ALTO ? HIGH : LOW) : (BUZZER_ACTIVO_ALTO ? LOW : HIGH));
}

void limpiarVinculo() {
  vinculo = "";
  for (int i = 0; i < totalRecordatorios; ++i) recordatorios[i] = Recordatorio();
  totalRecordatorios = 0;
  recibioHorario = false;
  fechaDeHorario = "";
  luzVerdeHasta = 0;
  mostrarErrorHasta = 0;
#if VITALINK_HX711_HABILITADO
  referenciaArmada = false;
#endif
}

bool abrirHTTPS(HTTPClient& http, const char* url) {
  if (!String(url).startsWith("https://")) return false;
  if (!tokenValido()) { Serial.println("Error: configura DEVICE_TOKEN de 64 hex en secrets.h."); return false; }
  if (!conectarWiFi() || !ntpListo()) return false;
  // Verificar certificados TLS: NO usar setInsecure().
  http.useBuiltinCACertBundle();
  http.setConnectTimeout(10000);
  http.setTimeout(15000);
  http.setFollowRedirects(HTTPC_DISABLE_FOLLOW_REDIRECTS);
  if (!http.begin(url)) {
    Serial.println("No pudo abrir la URL HTTPS."); return false;
  }
  http.addHeader("x-device-token", DEVICE_TOKEN);
  if (dispositivoGestionado) http.addHeader("x-device-id", VITALINK_DEVICE_ID);
  return true;
}

bool iniciarHTTPS(HTTPClient& http) {
  if (!abrirHTTPS(http, FUNCTION_URL)) return false;
  if (dispositivoGestionado && !vinculo.isEmpty()) http.addHeader("x-device-binding", vinculo);
  return true;
}

bool consultarVinculacion(bool iniciar, JsonDocument& respuesta) {
  HTTPClient http;
  if (!abrirHTTPS(http, PAIRING_URL)) return false;
  http.addHeader("Content-Type", "application/json");
  const int status = http.POST(iniciar ? "{\"action\":\"start\"}" : "{\"action\":\"status\"}");
  const String texto = status == 200 ? http.getString() : "";
  http.end();
  return status == 200 && texto.length() <= 512 && !deserializeJson(respuesta, texto);
}

void iniciarVinculacion() {
  lcdLinea(0, "VINCULAR");
  lcdLinea(1, "Solicitando...");
  const unsigned long solicitud = millis();
  JsonDocument doc;
  if (consultarVinculacion(true, doc) && doc["status"] == "pending") {
    const char* codigo = doc["code"] | "";
    const unsigned long segundos = doc["expires_in"] | 0UL;
    if (strlen(codigo) == 6 && strspn(codigo, "0123456789") == 6 && segundos > 0 && segundos <= 300) {
      codigoVinculacion = codigo;
      inicioCodigo = solicitud;
      duracionCodigo = segundos * 1000UL;
      ultimaConsultaVinculo = millis();
      return;
    }
  }
  if (doc["status"] == "linked") return;
  mensajeEstado = doc["status"] == "rate_limited" ? "Espera 20 s" : "Error vincular";
  mostrarErrorHasta = millis() + 4000UL;
}

void revisarVinculacion() {
  if (codigoVinculacion.isEmpty()) return;
  if (millis() - inicioCodigo >= duracionCodigo) {
    codigoVinculacion = "";
    mensajeEstado = "Codigo caducado";
    mostrarErrorHasta = millis() + 4000UL;
    return;
  }
  if (millis() - ultimaConsultaVinculo < 5000UL || WiFi.status() != WL_CONNECTED) return;
  ultimaConsultaVinculo = millis();
  JsonDocument doc;
  if (consultarVinculacion(false, doc) && doc["status"] == "linked") {
    codigoVinculacion = "";
    ultimaConsulta = millis() - SINCRONIZAR_CADA_MS;
  }
}

// Consulta horarios + dosis ya confirmadas hoy, sin exponer credenciales de BD.
bool descargarHorarios() {
  HTTPClient http;
  if (!iniciarHTTPS(http)) return false;
  const int status = http.GET();
  const String texto = status > 0 ? http.getString() : "";
  http.end();
  if (status != 200) {
    if (dispositivoGestionado && (status == 401 || status == 412)) limpiarVinculo();
    Serial.printf("GET horarios: HTTP %d %s\n", status, texto.c_str());
    return false;
  }
  if (texto.length() > 16000) { Serial.println("Respuesta demasiado grande."); return false; }
  JsonDocument doc;
  const DeserializationError err = deserializeJson(doc, texto);
  if (err || doc["ok"].as<bool>() != true) {
    Serial.printf("JSON horarios invalido: %s\n", err.c_str());
    return false;
  }
  JsonArrayConst a = doc["reminders"].as<JsonArrayConst>();
  const char* fecha = doc["date"].as<const char*>();
  if (a.isNull() || !fecha || a.size() > MAX_RECORDATORIOS) {
    Serial.println("Respuesta sin fecha/reminders o con mas de 48 horarios."); return false;
  }
  Recordatorio nuevos[MAX_RECORDATORIOS];
  int n = 0;
  for (JsonObjectConst r : a) {
    const char* hhmm = r["scheduled_time"].as<const char*>();
    if (!hhmm || strlen(hhmm) != 5 || hhmm[2] != ':' ||
        hhmm[0] < '0' || hhmm[0] > '2' || hhmm[1] < '0' || hhmm[1] > '9' ||
        hhmm[3] < '0' || hhmm[3] > '5' || hhmm[4] < '0' || hhmm[4] > '9') return false;
    const int hora = (hhmm[0] - '0') * 10 + (hhmm[1] - '0');
    const int minuto = (hhmm[3] - '0') * 10 + (hhmm[4] - '0');
    if (hora > 23) return false;
    nuevos[n].medId = r["medication_id"].as<uint32_t>();
    if (nuevos[n].medId == 0) return false;
    const char* nombre = r["name"].as<const char*>();
    nuevos[n].nombre = nombre ? String(nombre) : "Medicamento";
    nuevos[n].horaTexto = String(hhmm);
    nuevos[n].minutoDia = hora * 60 + minuto;
    nuevos[n].compartimento = r["compartment"].as<int>();
    nuevos[n].confirmadoHoy = r["confirmed_today"].as<bool>();
    ++n;
  }
  if (dispositivoGestionado) {
    const char* nuevoVinculo = doc["binding_id"] | "";
    if (strlen(nuevoVinculo) != 36) return false;
    if (vinculo != nuevoVinculo) limpiarVinculo();
    vinculo = nuevoVinculo;
    codigoVinculacion = "";
  }
  // Cambiar la lista solo cuando la respuesta entera fue valida.
  for (int i = 0; i < n; ++i) recordatorios[i] = nuevos[i];
  totalRecordatorios = n;
  fechaDeHorario = String(fecha);
  recibioHorario = true;
  Serial.printf("Horarios sincronizados: %d para %s\n", n, fechaDeHorario.c_str());
  return true;
}

int dosisPendiente(const FechaHora& h) {
  if (!recibioHorario || fechaDeHorario != fechaLocal(h)) return -1;
  const int actual = h.hora * 60 + h.minuto;
  for (int i = 0; i < totalRecordatorios; ++i) {
    const int retraso = actual - recordatorios[i].minutoDia;
    if (!recordatorios[i].confirmadoHoy && retraso >= 0 && retraso < VENTANA_ALERTA_MIN) return i;
  }
  return -1;
}

void confirmarDosis() {
  FechaHora h;
  if (!rtcLeer(h)) { Serial.println("RTC no listo: no se confirma."); return; }
  const int i = dosisPendiente(h);
  if (i < 0) { Serial.println("No hay una dosis pendiente en la ventana de 30 minutos."); return; }
  if (!tokenValido()) { Serial.println("TOKEN no configurado."); return; }
  HTTPClient http;
  if (!iniciarHTTPS(http)) {
    mensajeEstado = "Sin Internet";
    mostrarErrorHasta = millis() + 4000UL;
    Serial.println("No se envio la confirmacion; vuelve a intentar.");
    return;
  }
  http.addHeader("Content-Type", "application/json");
  const String body = String("{\"event\":\"button_pressed\",\"medication_id\":") +
    String(recordatorios[i].medId) + ",\"scheduled_time\":\"" +
    recordatorios[i].horaTexto + "\"}";
  const int status = http.POST(body);
  const String response = status > 0 ? http.getString() : HTTPClient::errorToString(status);
  http.end();
  if (dispositivoGestionado && (status == 401 || status == 412)) limpiarVinculo();
  Serial.printf("POST confirmacion: HTTP %d %s\n", status, response.c_str());
  if (status == 201 || status == 409) { // 409: ya estaba confirmada hoy.
    recordatorios[i].confirmadoHoy = true;
    luzVerdeHasta = millis() + 4000UL;
    mensajeEstado = status == 201 ? "Confirmado" : "Ya confirmado";
    Serial.println(status == 201 ? "Guardado en Supabase." : "Esa dosis ya estaba confirmada.");
  } else {
    mensajeEstado = "Error al enviar";
    mostrarErrorHasta = millis() + 4000UL;
  }
}

void actualizarPantalla(const FechaHora* h, int pendiente) {
  static unsigned long ultimaLCD = 0;
  if (millis() - ultimaLCD < 900UL) return;
  ultimaLCD = millis();
  if (!codigoVinculacion.isEmpty()) {
    lcdLinea(0, "CODIGO VINCULAR");
    lcdLinea(1, codigoVinculacion);
    return;
  }
  if (dispositivoGestionado && vinculo.isEmpty()) {
    lcdLinea(0, "SIN VINCULAR");
    lcdLinea(1, (long)(mostrarErrorHasta - millis()) > 0 ? mensajeEstado : "Boton al iniciar");
    return;
  }
  if (!h) { lcdLinea(0, "VITALINK SIN RTC"); lcdLinea(1, "Verificar reloj"); return; }
  char titulo[17];
  snprintf(titulo, sizeof(titulo), "VITALINK %02d:%02d", h->hora, h->minuto);
  lcdLinea(0, String(titulo));
  if ((long)(mostrarErrorHasta - millis()) > 0 || (long)(luzVerdeHasta - millis()) > 0) {
    lcdLinea(1, mensajeEstado); return;
  }
  if (!recibioHorario || fechaDeHorario != fechaLocal(*h)) {
    lcdLinea(1, "Sin horarios"); return;
  }
  if (pendiente >= 0) {
    const Recordatorio& r = recordatorios[pendiente];
    // Alterna horario y nombre, 16 caracteres por linea.
    if ((millis() / 2500UL) % 2 == 0)
      lcdLinea(1, String("C") + String(r.compartimento) + " " + r.horaTexto + " PENDIENTE");
    else lcdLinea(1, r.nombre);
  } else lcdLinea(1, "En espera");
}

void actualizarSalidas(int pendiente) {
  const bool verde = (long)(luzVerdeHasta - millis()) > 0;
  digitalWrite(PIN_LED_ROJO, pendiente >= 0 ? HIGH : LOW);
  digitalWrite(PIN_LED_VERDE, verde ? HIGH : LOW);
  // Sonido corto cada 3 segundos mientras una dosis este pendiente.
  encenderBuzzer(pendiente >= 0 && (millis() % 3000UL < 250UL));
}

void setup() {
  Serial.begin(115200);
  Wire.begin(PIN_SDA, PIN_SCL);
  pinMode(PIN_BOTON, INPUT_PULLUP);
  const bool botonAlIniciar = dispositivoGestionado && digitalRead(PIN_BOTON) == LOW;
  pinMode(PIN_LED_ROJO, OUTPUT);
  pinMode(PIN_LED_VERDE, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_LED_ROJO, LOW);
  digitalWrite(PIN_LED_VERDE, LOW);
  encenderBuzzer(false);
  lcdInit();
#if VITALINK_SERVO_HABILITADO
  iniciarTapaMotorizada();
#endif
#if VITALINK_HX711_HABILITADO
  iniciarBascula();
#endif
  lcdLinea(0, "VITALINK"); lcdLinea(1, "Iniciando...");
  Serial.println("Vitalink ESP32 + HX711. 115200: s=horarios, t=confirmar, p=ver peso, r=armar referencia.");
  const bool solicitarVinculo = botonAlIniciar && digitalRead(PIN_BOTON) == LOW;
  if (botonAlIniciar) {
    lecturaAnterior = LOW;
    lecturaEstable = LOW;
  }
  if (solicitarVinculo) iniciarVinculacion();
  if (conectarWiFi() && ntpListo()) {
    if (rtcEscribirDesdeNTP()) Serial.println("RTC ajustado con hora de Ecuador.");
    else Serial.println("No se pudo ajustar RTC.");
    ultimaHoraRTC = millis();
    if (codigoVinculacion.isEmpty()) descargarHorarios();
  }
  ultimaConsulta = millis();
}

void loop() {
  while (Serial.available()) {
    const char c = Serial.read();
    if (c == 's' || c == 'S') {
      Serial.println("Sincronizacion manual...");
      if (descargarHorarios()) Serial.println("GET OK.");
    } else if (c == 't' || c == 'T') confirmarDosis();
#if VITALINK_HX711_HABILITADO
    else if (c == 'p' || c == 'P') mostrarPeso();
    else if (c == 'r' || c == 'R') armarReferenciaPeso();
#endif
  }
  const int lectura = digitalRead(PIN_BOTON);
  if (lectura != lecturaAnterior) cambioBoton = millis();
  if (millis() - cambioBoton > 60UL && lectura != lecturaEstable) {
    lecturaEstable = lectura;
    if (lecturaEstable == LOW) confirmarDosis();
  }
  lecturaAnterior = lectura;

  revisarVinculacion();
  if (codigoVinculacion.isEmpty() && millis() - ultimaConsulta >= SINCRONIZAR_CADA_MS) {
    ultimaConsulta = millis();
    if (conectarWiFi() && ntpListo()) {
      if (millis() - ultimaHoraRTC > 6UL * 60 * 60 * 1000) {
        if (rtcEscribirDesdeNTP()) Serial.println("RTC reajustado por NTP.");
        ultimaHoraRTC = millis();
      }
      descargarHorarios();
    }
  }
#if VITALINK_HX711_HABILITADO
  revisarPeso();
#endif
  FechaHora ahora;
  const bool rtcOK = rtcLeer(ahora);
  const int i = rtcOK ? dosisPendiente(ahora) : -1;
  actualizarSalidas(i);
#if VITALINK_SERVO_HABILITADO
  actualizarTapaMotorizada(i);
#endif
  actualizarPantalla(rtcOK ? &ahora : nullptr, i);
  delay(15);
}
