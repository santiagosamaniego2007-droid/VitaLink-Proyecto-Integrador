/* Calibracion sencilla HX711 + celda 5kg, ESP32 DevKit V1.
   CORREGIR PINES primero segun las conexiones del prototipo.
   Arduino IDE: instalar biblioteca HX711 by Bogde.
   Monitor Serie 115200: t=tarar plato VACIO; c=calcular factor con 100g conocidos; p=ver datos.
   Una vez calibrado, copiar el factor a config.h del programa principal.
*/
#include <Arduino.h>
#include <HX711.h>
constexpr int PIN_HX_DT = 32;  // EJEMPLO: CAMBIAR segun conexion REAL
constexpr int PIN_HX_SCK = 33; // EJEMPLO: CAMBIAR segun conexion REAL
constexpr float MASA_CONOCIDA_G = 100.0f; // CAMBIAR por la masa real del objeto patron
HX711 hx;
bool lista = false;
void setup() {
  Serial.begin(115200);
  delay(700);
  hx.begin(PIN_HX_DT, PIN_HX_SCK);
  lista = hx.wait_ready_timeout(2500);
  Serial.println(lista ? "HX711 responde. Quitar carga, luego enviar t para tara." : "HX711 NO responde: revisar cables, voltaje y GPIO.");
}
void loop() {
  if (!lista || !Serial.available()) return;
  const char c = Serial.read();
  if (c == 't' || c == 'T') {
    Serial.println("Haciendo tara: plato sin medicamentos/peso patron...");
    hx.set_scale(1.0f);
    hx.tare(20);
    Serial.println("Tara lista. Colocar peso conocido, luego enviar c.");
  } else if (c == 'c' || c == 'C') {
    if (!hx.wait_ready_timeout(2500)) { Serial.println("HX711 no responde"); return; }
    const float promedio = hx.get_value(15);
    const float factor = promedio / MASA_CONOCIDA_G;
    Serial.printf("Lectura neta: %.2f cuentas; factor sugerido: %.6f cuentas/gramo.\n", promedio, factor);
    Serial.println("Colocar este factor en HX711_FACTOR_CALIBRACION del config.h de Vitalink.");
  } else if (c == 'p' || c == 'P') {
    if (hx.wait_ready_timeout(1500)) Serial.printf("RAW: %.2f | neto: %.2f\n", (float)hx.read_average(5), hx.get_value(5));
  }
}
