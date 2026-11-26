#pragma once
// Copiar este archivo a secrets.h en la PC del companero.
// No compartir el secrets.h real ni incluirlo en el ZIP.
const char* WIFI_SSID = "WIFI_2_4_GHZ";
const char* WIFI_PASSWORD = "PON_AQUI_LA_CLAVE_WIFI";
const char* DEVICE_TOKEN = "PEGA_AQUI_EL_TOKEN_REAL_DE_64_CARACTERES";

// Para V3.2, aprovisionar un UUID y un DEVICE_TOKEN unicos por placa.
// Omitir VITALINK_DEVICE_ID conserva el modo demo legado.
// #define VITALINK_DEVICE_ID "uuid-publico-del-dispositivo"
#define VITALINK_FUNCTIONS_BASE "https://TU-PROYECTO.supabase.co/functions/v1/"
