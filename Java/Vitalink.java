import java.util.Scanner;

/**
 * VITALINK - Proyecto 14 - traduccion didactica de PSeInt a Java.
 * Mantiene las siete opciones y los mismos estados del pseudocodigo.
 * No controla sensores, no prescribe dosis y NO verifica la toma de medicamentos.
 * Los datos duran mientras el programa esta abierto, como en la simulacion PSeInt.
 */
public class Vitalink {
    // Las posiciones 1..10 y 1..3 replican los indices del pseudocodigo PSeInt.
    private static final int MAX_MEDICAMENTOS = 10;
    private static final int MAX_HORARIOS = 3;
    private static final int PROGRAMADO = 0;
    private static final int EMITIDO_SIN_CONFIRMAR = 1;
    private static final int CONFIRMADO = 2;

    private final Scanner teclado = new Scanner(System.in);
    private final String[] medicamentos = new String[MAX_MEDICAMENTOS + 1];
    private final int[] cantidadHorarios = new int[MAX_MEDICAMENTOS + 1];
    private final String[][] horarios = new String[MAX_MEDICAMENTOS + 1][MAX_HORARIOS + 1];
    private final int[][] estadoAvisos = new int[MAX_MEDICAMENTOS + 1][MAX_HORARIOS + 1];
    private final String[] historialAvisos = new String[MAX_MEDICAMENTOS * MAX_HORARIOS + 1];
    private int totalMedicamentos = 0;
    private int totalHistorial = 0;

    // PSeInt: Funcion valido <- EsDigito(simbolo)
    public static boolean esDigito(char simbolo) {
        return simbolo >= '0' && simbolo <= '9';
    }

    // PSeInt: Funcion valido <- ValidarHorario(textoHora)
    public static boolean validarHorario(String textoHora) {
        if (textoHora == null || textoHora.length() != 5 || textoHora.charAt(2) != ':') {
            return false;
        }
        if (!esDigito(textoHora.charAt(0)) || !esDigito(textoHora.charAt(1))
                || !esDigito(textoHora.charAt(3)) || !esDigito(textoHora.charAt(4))) {
            return false;
        }
        int horas = Integer.parseInt(textoHora.substring(0, 2));
        int minutos = Integer.parseInt(textoHora.substring(3, 5));
        return horas >= 0 && horas <= 23 && minutos >= 0 && minutos <= 59;
    }

    // PSeInt: Funcion total <- ContarPorEstado(...)
    private int contarPorEstado(int estadoBuscado) {
        int total = 0;
        for (int i = 1; i <= totalMedicamentos; i++) {
            for (int j = 1; j <= cantidadHorarios[i]; j++) {
                if (estadoAvisos[i][j] == estadoBuscado) {
                    total++;
                }
            }
        }
        return total;
    }

    // Equivale al SubProceso MostrarMenu: void = no devuelve valor.
    private void mostrarMenu() {
        System.out.println("-------------------------------------------");
        System.out.println("           VITALINK - MENU PRINCIPAL       ");
        System.out.println("-------------------------------------------");
        System.out.println("1. Registrar un medicamento y su horario");
        System.out.println("2. Consultar medicamentos y horarios");
        System.out.println("3. Consultar recordatorios pendientes");
        System.out.println("4. Simular la emision de un recordatorio");
        System.out.println("5. Registrar la confirmacion de un aviso");
        System.out.println("6. Consultar historial de avisos");
        System.out.println("7. Finalizar el programa");
    }

    // La lectura por linea evita que una entrada como "hola" cierre la simulacion.
    private int leerEntero(String pregunta) {
        while (true) {
            System.out.println(pregunta);
            String texto = teclado.nextLine().trim();
            try {
                return Integer.parseInt(texto);
            } catch (NumberFormatException ex) {
                System.out.println("Entrada invalida: escriba un numero entero.");
            }
        }
    }

    private String leerTexto(String pregunta) {
        System.out.println(pregunta);
        return teclado.nextLine().trim();
    }

    // Opcion 1 - PSeInt: SubProceso RegistrarMedicamento
    private void registrarMedicamento() {
        if (totalMedicamentos >= MAX_MEDICAMENTOS) {
            System.out.println("Capacidad maxima: 10 medicamentos.");
        } else {
            String nombreMedicamento = leerTexto("Nombre del medicamento:");
            boolean datosCompletos = !nombreMedicamento.isEmpty();
            if (!datosCompletos) {
                System.out.println("Registro rechazado: falta el nombre obligatorio.");
            }
            if (datosCompletos) {
                int cantidad;
                do {
                    cantidad = leerEntero("Cantidad de horarios (1 a 3):");
                    if (cantidad < 1 || cantidad > 3) {
                        System.out.println("Debe ingresar de 1 a 3 horarios.");
                    }
                } while (cantidad < 1 || cantidad > 3);

                int nuevoIndice = totalMedicamentos + 1;
                for (int j = 1; j <= cantidad; j++) {
                    String horarioIngresado;
                    boolean horarioValido;
                    do {
                        horarioIngresado = leerTexto("Horario " + j + " (HH:MM):");
                        horarioValido = validarHorario(horarioIngresado);
                        if (!horarioValido) {
                            System.out.println("Formato incorrecto. Ejemplo valido: 08:30.");
                        }
                    } while (!horarioValido);
                    horarios[nuevoIndice][j] = horarioIngresado;
                    estadoAvisos[nuevoIndice][j] = PROGRAMADO;
                }
                medicamentos[nuevoIndice] = nombreMedicamento;
                cantidadHorarios[nuevoIndice] = cantidad;
                totalMedicamentos = nuevoIndice;
                System.out.println("Medicamento y horarios registrados.");
            }
        }
    }

    // Opcion 2 - PSeInt: SubProceso MostrarMedicamentosYHorarios
    private void mostrarMedicamentosYHorarios() {
        if (totalMedicamentos == 0) {
            System.out.println("No hay medicamentos registrados.");
        } else {
            for (int i = 1; i <= totalMedicamentos; i++) {
                System.out.println("Medicamento " + i + ": " + medicamentos[i]);
                for (int j = 1; j <= cantidadHorarios[i]; j++) {
                    System.out.println("  Horario " + j + ": " + horarios[i][j]);
                }
            }
        }
    }

    // Opcion 3 - PSeInt: SubProceso MostrarPendientes
    private void mostrarPendientes() {
        int programados = contarPorEstado(PROGRAMADO);
        int sinConfirmar = contarPorEstado(EMITIDO_SIN_CONFIRMAR);
        int totalPendientes = programados + sinConfirmar;
        System.out.println("Recordatorios pendientes: " + totalPendientes);
        System.out.println("Programados (no emitidos): " + programados);
        System.out.println("Emitidos sin confirmar: " + sinConfirmar);
        for (int i = 1; i <= totalMedicamentos; i++) {
            for (int j = 1; j <= cantidadHorarios[i]; j++) {
                if (estadoAvisos[i][j] == PROGRAMADO) {
                    System.out.println(medicamentos[i] + " - " + horarios[i][j] + " : PROGRAMADO");
                }
                if (estadoAvisos[i][j] == EMITIDO_SIN_CONFIRMAR) {
                    System.out.println(medicamentos[i] + " - " + horarios[i][j] + " : SIN CONFIRMAR");
                }
            }
        }
    }

    // Opcion 4 - PSeInt: SubProceso SimularRecordatorio
    private void simularRecordatorio() {
        String horaSimulada;
        do {
            horaSimulada = leerTexto("Ingrese la hora simulada (HH:MM):");
            if (!validarHorario(horaSimulada)) {
                System.out.println("Hora invalida. Use HH:MM.");
            }
        } while (!validarHorario(horaSimulada));
        boolean coincide = false;
        int emitidos = 0;
        for (int i = 1; i <= totalMedicamentos; i++) {
            for (int j = 1; j <= cantidadHorarios[i]; j++) {
                if (horarios[i][j].equals(horaSimulada)) {
                    coincide = true;
                    if (estadoAvisos[i][j] == PROGRAMADO) {
                        estadoAvisos[i][j] = EMITIDO_SIN_CONFIRMAR;
                        int posicion = (i - 1) * MAX_HORARIOS + j;
                        historialAvisos[posicion] = "EMITIDO SIN CONFIRMAR";
                        totalHistorial++;
                        emitidos++;
                        System.out.println("AVISO (LED Y ZUMBADOR SIMULADOS): "
                                + medicamentos[i] + " - " + horarios[i][j]);
                    } else {
                        System.out.println(medicamentos[i] + " - " + horarios[i][j]
                                + " : aviso ya gestionado en esta ronda.");
                    }
                }
            }
        }
        if (!coincide) {
            System.out.println("No existe un horario registrado para esa hora.");
        }
        System.out.println("Avisos nuevos emitidos: " + emitidos);
    }

    // Opcion 5 - PSeInt: SubProceso RegistrarConfirmacion
    private void registrarConfirmacion() {
        if (totalMedicamentos == 0) {
            System.out.println("Primero registre un medicamento.");
        } else {
            mostrarMedicamentosYHorarios();
            int medicamentoSeleccionado = leerEntero("Numero del medicamento:");
            if (medicamentoSeleccionado < 1 || medicamentoSeleccionado > totalMedicamentos) {
                System.out.println("Medicamento inexistente.");
            } else {
                int horarioSeleccionado = leerEntero("Numero de horario (1 a "
                        + cantidadHorarios[medicamentoSeleccionado] + "):");
                if (horarioSeleccionado < 1 || horarioSeleccionado > cantidadHorarios[medicamentoSeleccionado]) {
                    System.out.println("Horario inexistente.");
                } else {
                    if (estadoAvisos[medicamentoSeleccionado][horarioSeleccionado] == EMITIDO_SIN_CONFIRMAR) {
                        int confirmacion;
                        do {
                            confirmacion = leerEntero("Atendio el aviso? 1 = Si, 2 = No:");
                        } while (confirmacion != 1 && confirmacion != 2);
                        boolean avisoConfirmado = confirmacion == 1;
                        int posicion = (medicamentoSeleccionado - 1) * MAX_HORARIOS + horarioSeleccionado;
                        if (avisoConfirmado) {
                            estadoAvisos[medicamentoSeleccionado][horarioSeleccionado] = CONFIRMADO;
                            historialAvisos[posicion] = "CONFIRMADO POR EL USUARIO";
                            System.out.println("Aviso registrado como atendido.");
                        } else {
                            historialAvisos[posicion] = "EMITIDO SIN CONFIRMAR";
                            System.out.println("El aviso permanece sin confirmar.");
                        }
                    } else {
                        System.out.println("Este horario no tiene un aviso emitido pendiente de confirmar.");
                    }
                }
            }
        }
    }

    // Opcion 6 - PSeInt: SubProceso MostrarHistorial
    private void mostrarHistorial() {
        int totalNoConfirmados = contarPorEstado(EMITIDO_SIN_CONFIRMAR);
        System.out.println("Total de avisos emitidos: " + totalHistorial);
        System.out.println("Avisos emitidos no confirmados: " + totalNoConfirmados);
        if (totalHistorial == 0) {
            System.out.println("Todavia no se emitieron avisos.");
        } else {
            for (int i = 1; i <= totalMedicamentos; i++) {
                for (int j = 1; j <= cantidadHorarios[i]; j++) {
                    if (estadoAvisos[i][j] == EMITIDO_SIN_CONFIRMAR) {
                        int posicion = (i - 1) * MAX_HORARIOS + j;
                        System.out.println(medicamentos[i] + " - " + horarios[i][j] + " : " + historialAvisos[posicion]);
                    }
                    if (estadoAvisos[i][j] == CONFIRMADO) {
                        int posicion = (i - 1) * MAX_HORARIOS + j;
                        System.out.println(medicamentos[i] + " - " + horarios[i][j] + " : " + historialAvisos[posicion]);
                    }
                }
            }
        }
    }

    // PSeInt: Algoritmo Vitalink / Mientras + Segun
    private void ejecutar() {
        int opcion = 0;
        while (opcion != 7) {
            mostrarMenu();
            opcion = leerEntero("Seleccione una opcion (1-7):");
            switch (opcion) {
                case 1 -> registrarMedicamento();
                case 2 -> mostrarMedicamentosYHorarios();
                case 3 -> mostrarPendientes();
                case 4 -> simularRecordatorio();
                case 5 -> registrarConfirmacion();
                case 6 -> mostrarHistorial();
                case 7 -> System.out.println("Finalizando VITALINK.");
                default -> System.out.println("Opcion no valida.");
            }
        }
    }

    public static void main(String[] args) {
        new Vitalink().ejecutar();
    }
}
