# VITALINK en Java — conversión de la simulación PSeInt

Esta es la **versión Java de consola aportada por el equipo** del algoritmo `Vitalink` del proyecto 14, no una aplicación Android nueva ni una aplicación médica validada. Traduce las funciones `EsDigito`, `ValidarHorario`, `ContarPorEstado`, los 7 subprocesos y las 7 opciones de menú de la versión PSeInt recuperada del proyecto.

## Cómo ejecutarlo en Windows

1. Instala un JDK, por ejemplo [Eclipse Temurin JDK 21](https://adoptium.net/). Alternativamente usa IntelliJ IDEA con un JDK configurado.
2. Abre una terminal (PowerShell o Símbolo del sistema) dentro de la carpeta `Java`.
3. Ejecuta:

```bash
javac -encoding UTF-8 Vitalink1.java
java Vitalink1
```

También puedes abrir `Vitalink1.java` en **IntelliJ IDEA** como proyecto Java sencillo y ejecutar `main` (puede requerir configurar el JDK).

## Relación PSeInt → Java

| PSeInt | Java |
|---|---|
| `Algoritmo Vitalink` | clase `Vitalink1` y `main` |
| `Funcion ValidarHorario` | método `validarHorario` que devuelve `boolean` |
| `Funcion ContarPorEstado` | método `contarPorEstado` que devuelve `int` |
| `SubProceso MostrarMenu` | método `void mostrarMenu()` |
| `Si ... Entonces / SiNo` | `if (...) { ... } else { ... }` |
| `Segun` | `switch` |
| `Mientras` | `while` |
| `Repetir ... Hasta Que` | `do { ... } while (...)` (condición invertida) |
| `Para` | `for` |
| `Dimension medicamentos[10]` | `String[] medicamentos = new String[10]` |
| `Dimension horarios[10,3]` | `String[][] horarios = new String[10][3]` |

**Índices:** Java utiliza índices desde 0 (0..9 y 0..2); el menú conserva la numeración desde 1. El historial utiliza 30 posiciones. Se añade `leerEntero()` para validar entradas numéricas y se eliminan espacios exteriores del nombre con `trim()`.

## Qué hace el programa

1. Registrar hasta 10 nombres de medicamentos, con 1 a 3 horarios por medicamento.
2. Mostrar el listado y horarios registrados.
3. Contar avisos programados y emitidos sin confirmar.
4. Simular la llegada de una hora y emitir avisos **textuales** (LED y zumbador no conectados).
5. Registrar si el usuario atendió o no un aviso emitido.
6. Mostrar el estado de los avisos emitidos en esta ronda.
7. Finalizar.

Estados: **0 = programado; 1 = emitido sin confirmar; 2 = confirmado**. La confirmación solo indica que el usuario atendió el aviso; **no demuestra que haya tomado el medicamento**.

## Tres pruebas sugeridas

### Caso normal

- Opción 1 → nombre `Ejemplo A`, un horario, `08:30`.
- Opción 4 → hora `08:30` → aviso emitido.
- Opción 5 → medicamento 1, horario 1, confirmación 1 → atendido.
- Opción 6 → historial muestra confirmado.

### Caso inválido

- Opción 1 → nombre `Ejemplo B`, horario `25:70` → debe exigir otra hora; usa `09:15` para terminar.

### Caso límite

- Registrar un medicamento con exactamente tres horarios (`00:00`, `12:00`, `23:59`) y consultar la opción 2.

Puedes usar los datos ficticios del archivo `prueba_normal.txt` para automatizar la primera simulación.

## Alcances

- Solo guarda datos en memoria hasta salir del programa. No tiene cuentas, notificaciones reales, ESP32, base de datos ni sincronización con Netlify.
- El historial replica el estado más reciente de cada aviso en una **ronda**, sin guardar múltiples días: no es un registro clínico ni un historial persistente.
- Para Android e iOS utiliza en paralelo el proyecto **Capacitor** de Vitalink, que permite recuperar la web original y aprovechar HTML/CSS/JS.
- Si se modifica el pseudocódigo, revisar la correspondencia con Java. Las operaciones y estados coinciden en la revisión de la lógica; la comparación ejecutada lado a lado en PSeInt queda pendiente.

### Correspondencia con la guía

La guía del proyecto 14 exige siete operaciones, validación de `HH:MM`, seis estructuras de control, arreglos, matrices, subprocesos y funciones. Esta versión Java reproduce esos conceptos de la simulación; no sustituye la entrega PSeInt ni el diagrama del Avance 3.

## Verificación del código recibido

Compilado con `javac -encoding UTF-8 Vitalink1.java`. Pruebas de consola ejecutadas: registro, emisión, confirmación e historial; entradas inválidas; capacidad de 10 medicamentos y 30 horarios, sin reemitir avisos ya gestionados. Los tres grupos terminaron sin errores. Estas pruebas no validan el hardware ni sustituyen una ejecución comparativa con PSeInt.
