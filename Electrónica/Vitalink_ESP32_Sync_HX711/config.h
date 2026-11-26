#pragma once
// REVISAR CON EL COMPANERO ANTES DE CARGAR: deben coincidir con los cables reales.
// GPIO 6 a 11 NO son apropiados para pulsadores en ESP32 DevKit V1 (flash).
#define PIN_BOTON       18
#define PIN_LED_ROJO    25
#define PIN_LED_VERDE   26
#define PIN_BUZZER      27
#define PIN_SDA         21
#define PIN_SCL         22
#define LCD_DIRECCION   0x27  // Si la pantalla no responde, comprobar 0x3F y su cableado

// Buzzer activo: poner 1 cuando se activa con HIGH, 0 cuando se activa con LOW.
#define BUZZER_ACTIVO_ALTO 1
#define MAX_RECORDATORIOS 48
#define VENTANA_ALERTA_MIN 30 // Prueba: alerta durante 30 min desde la hora programada.
#define SINCRONIZAR_CADA_MS 60000UL // Consulta horarios y confirmaciones cada 60 s.

// ==== HX711 / celda de carga (extension en preparacion) ====
// CAMBIAR A 1 SOLO cuando el companero haya verificado GPIO y obtenido el factor real con el sketch de calibracion.
#define VITALINK_HX711_HABILITADO 0 // 0=deshabilitado (seguro); 1=leer y enviar cambios de peso
#define HX711_PIN_DT 32    // EJEMPLO, cambiar segun cableado real.
#define HX711_PIN_SCK 33   // EJEMPLO, cambiar segun cableado real.
// IMPORTANTE: 0 NO es un factor valido, evita lecturas falsas.
// Segun el montaje puede ser positivo o negativo; obtener por calibracion.
#define HX711_FACTOR_CALIBRACION 0.0f // REEMPLAZAR por factor calibrado real (puede ser negativo)
#define HX711_UMBRAL_CAMBIO_G 10.0f   // Prueba: notificar cambios >= 10 g
#define HX711_TOLERANCIA_ESTABILIDAD_G 3.0f
#define HX711_CADA_MS 1500UL
#define HX711_COOLDOWN_MS 60000UL


// ==== Tapa motorizada (V3.2: OPCIONAL, desactivada hasta verificar la madera) ====
// Requiere biblioteca ESP32Servo y fuente de 5 V externa adecuada para el servo.
// No conectar la alimentación del servo al pin 3V3 del ESP32.
#define VITALINK_SERVO_HABILITADO 0
#define PIN_SERVO 19  // GPIO de EJEMPLO; comprobar cableado y disponibilidad real.
#define SERVO_ANGULO_CERRADO 15  // Ajustar según bisagra, sin forzar topes.
#define SERVO_ANGULO_ABIERTO 75 // Ajustar según peso de la tapa y recorrido.
