import java.util.Scanner;

// Autora de la traduccion a Java: Maybelin Micaela Herrera Niebla.
// Proyecto integrador VitaLink - Grupo 2.
public class Vitalink1 {
    static Scanner entrada = new Scanner(System.in);

    // Arreglos principales
    static String[] medicamentos = new String[10];
    static int[] cantidadHorarios = new int[10];
    static String[][] horarios = new String[10][3];
    static int[][] estadoAvisos = new int[10][3];
    static String[] historialAvisos = new String[30];

    // Contadores generales
    static int totalMedicamentos = 0;
    static int totalHistorial = 0;

    // Metodo para leer numeros enteros
    public static int leerEntero() {
        while (true) {
            try {
                return Integer.parseInt(entrada.nextLine());
            } catch (NumberFormatException e) {
                System.out.println("Ingrese un numero entero valido.");
            }
        }
    }

    // Funcion: EsDigito
    public static boolean esDigito(char simbolo) {
        return simbolo >= '0' && simbolo <= '9';
    }

    // Funcion: ValidarHorario
    public static boolean validarHorario(String textoHora) {

        if (textoHora.length() != 5) {
            return false;
        }

        if (textoHora.charAt(2) != ':') {
            return false;
        }

        if (!esDigito(textoHora.charAt(0)) ||
            !esDigito(textoHora.charAt(1)) ||
            !esDigito(textoHora.charAt(3)) ||
            !esDigito(textoHora.charAt(4))) {
            return false;
        }

        int horas = Integer.parseInt(textoHora.substring(0, 2));
        int minutos = Integer.parseInt(textoHora.substring(3, 5));

        return horas >= 0 && horas <= 23 &&
               minutos >= 0 && minutos <= 59;
    }

    // Funcion: ContarPorEstado
    public static int contarPorEstado(int estadoBuscado) {

        int total = 0;

        for (int i = 0; i < totalMedicamentos; i++) {
            for (int j = 0; j < cantidadHorarios[i]; j++) {

                if (estadoAvisos[i][j] == estadoBuscado) {
                    total++;
                }
            }
        }

        return total;
    }

    // SubProceso: MostrarMenu
    public static void mostrarMenu() {

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

    // SubProceso: RegistrarMedicamento
    public static void registrarMedicamento() {

        if (totalMedicamentos >= 10) {
            System.out.println("Capacidad maxima: 10 medicamentos.");
            return;
        }

        System.out.println("Nombre del medicamento:");
        String nombreMedicamento = entrada.nextLine().trim();

        boolean datosCompletos = !nombreMedicamento.isEmpty();

        if (!datosCompletos) {
            System.out.println("Registro rechazado: falta el nombre obligatorio.");
            return;
        }

        int cantidad;

        do {
            System.out.println("Cantidad de horarios (1 a 3):");
            cantidad = leerEntero();

            if (cantidad < 1 || cantidad > 3) {
                System.out.println("Debe ingresar de 1 a 3 horarios.");
            }

        } while (cantidad < 1 || cantidad > 3);

        int nuevoIndice = totalMedicamentos;

        for (int j = 0; j < cantidad; j++) {

            String horarioIngresado;
            boolean horarioValido;

            do {
                System.out.println("Horario " + (j + 1) + " (HH:MM):");
                horarioIngresado = entrada.nextLine();

                horarioValido = validarHorario(horarioIngresado);

                if (!horarioValido) {
                    System.out.println("Formato incorrecto. Ejemplo valido: 08:30.");
                }

            } while (!horarioValido);

            horarios[nuevoIndice][j] = horarioIngresado;
            estadoAvisos[nuevoIndice][j] = 0;
        }

        medicamentos[nuevoIndice] = nombreMedicamento;
        cantidadHorarios[nuevoIndice] = cantidad;
        totalMedicamentos++;

        System.out.println("Medicamento y horarios registrados.");
    }

    // SubProceso: MostrarMedicamentosYHorarios
    public static void mostrarMedicamentosYHorarios() {

        if (totalMedicamentos == 0) {
            System.out.println("No hay medicamentos registrados.");
        } else {

            for (int i = 0; i < totalMedicamentos; i++) {

                System.out.println("Medicamento " + (i + 1) + ": "
                        + medicamentos[i]);

                for (int j = 0; j < cantidadHorarios[i]; j++) {

                    System.out.println("  Horario " + (j + 1) + ": "
                            + horarios[i][j]);
                }
            }
        }
    }

    // SubProceso: MostrarPendientes
    public static void mostrarPendientes() {

        int programados = contarPorEstado(0);
        int sinConfirmar = contarPorEstado(1);

        int totalPendientes = programados + sinConfirmar;

        System.out.println("Recordatorios pendientes: " + totalPendientes);
        System.out.println("Programados (no emitidos): " + programados);
        System.out.println("Emitidos sin confirmar: " + sinConfirmar);

        for (int i = 0; i < totalMedicamentos; i++) {

            for (int j = 0; j < cantidadHorarios[i]; j++) {

                if (estadoAvisos[i][j] == 0) {
                    System.out.println(medicamentos[i] + " - "
                            + horarios[i][j] + " : PROGRAMADO");
                }

                if (estadoAvisos[i][j] == 1) {
                    System.out.println(medicamentos[i] + " - "
                            + horarios[i][j] + " : SIN CONFIRMAR");
                }
            }
        }
    }

    // SubProceso: SimularRecordatorio
    public static void simularRecordatorio() {

        String horaSimulada;
        boolean horarioValido;

        do {
            System.out.println("Ingrese la hora simulada (HH:MM):");
            horaSimulada = entrada.nextLine();

            horarioValido = validarHorario(horaSimulada);

            if (!horarioValido) {
                System.out.println("Hora invalida. Use HH:MM.");
            }

        } while (!horarioValido);

        boolean coincide = false;
        int emitidos = 0;

        for (int i = 0; i < totalMedicamentos; i++) {

            for (int j = 0; j < cantidadHorarios[i]; j++) {

                if (horarios[i][j].equals(horaSimulada)) {

                    coincide = true;

                    if (estadoAvisos[i][j] == 0) {

                        estadoAvisos[i][j] = 1;

                        int posicion = i * 3 + j;

                        historialAvisos[posicion] = "EMITIDO SIN CONFIRMAR";

                        totalHistorial++;
                        emitidos++;

                        System.out.println(
                            "AVISO (LED Y ZUMBADOR SIMULADOS): "
                            + medicamentos[i] + " - " + horarios[i][j]
                        );

                    } else {

                        System.out.println(medicamentos[i] + " - "
                                + horarios[i][j]
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

    // SubProceso: RegistrarConfirmacion
    public static void registrarConfirmacion() {

        if (totalMedicamentos == 0) {
            System.out.println("Primero registre un medicamento.");
            return;
        }

        mostrarMedicamentosYHorarios();

        System.out.println("Numero del medicamento:");
        int medicamentoSeleccionado = leerEntero();

        if (medicamentoSeleccionado < 1 ||
            medicamentoSeleccionado > totalMedicamentos) {

            System.out.println("Medicamento inexistente.");
            return;
        }

        int indiceMedicamento = medicamentoSeleccionado - 1;

        System.out.println("Numero de horario (1 a "
                + cantidadHorarios[indiceMedicamento] + "):");

        int horarioSeleccionado = leerEntero();

        if (horarioSeleccionado < 1 ||
            horarioSeleccionado > cantidadHorarios[indiceMedicamento]) {

            System.out.println("Horario inexistente.");
            return;
        }

        int indiceHorario = horarioSeleccionado - 1;

        if (estadoAvisos[indiceMedicamento][indiceHorario] == 1) {

            int confirmacion;

            do {
                System.out.println("Atendio el aviso? 1 = Si, 2 = No:");
                confirmacion = leerEntero();

            } while (confirmacion != 1 && confirmacion != 2);

            boolean avisoConfirmado = confirmacion == 1;

            int posicion = indiceMedicamento * 3 + indiceHorario;

            if (avisoConfirmado) {

                estadoAvisos[indiceMedicamento][indiceHorario] = 2;

                historialAvisos[posicion] =
                        "CONFIRMADO POR EL USUARIO";

                System.out.println("Aviso registrado como atendido.");

            } else {

                historialAvisos[posicion] =
                        "EMITIDO SIN CONFIRMAR";

                System.out.println("El aviso permanece sin confirmar.");
            }

        } else {

            System.out.println(
                "Este horario no tiene un aviso emitido pendiente de confirmar."
            );
        }
    }

    // SubProceso: MostrarHistorial
    public static void mostrarHistorial() {

        int totalNoConfirmados = contarPorEstado(1);

        System.out.println("Total de avisos emitidos: " + totalHistorial);

        System.out.println("Avisos emitidos no confirmados: "
                + totalNoConfirmados);

        if (totalHistorial == 0) {

            System.out.println("Todavia no se emitieron avisos.");

        } else {

            for (int i = 0; i < totalMedicamentos; i++) {

                for (int j = 0; j < cantidadHorarios[i]; j++) {

                    if (estadoAvisos[i][j] == 1 ||
                        estadoAvisos[i][j] == 2) {

                        int posicion = i * 3 + j;

                        System.out.println(medicamentos[i] + " - "
                                + horarios[i][j] + " : "
                                + historialAvisos[posicion]);
                    }
                }
            }
        }
    }

    // Algoritmo principal: Vitalink
    public static void main(String[] args) {

        int opcionMenu = 0;

        while (opcionMenu != 7) {

            mostrarMenu();

            System.out.println("Seleccione una opcion (1-7):");
            opcionMenu = leerEntero();

            switch (opcionMenu) {

                case 1:
                    registrarMedicamento();
                    break;

                case 2:
                    mostrarMedicamentosYHorarios();
                    break;

                case 3:
                    mostrarPendientes();
                    break;

                case 4:
                    simularRecordatorio();
                    break;

                case 5:
                    registrarConfirmacion();
                    break;

                case 6:
                    mostrarHistorial();
                    break;

                case 7:
                    System.out.println("Finalizando VITALINK.");
                    break;

                default:
                    System.out.println("Opcion no valida.");
                    break;
            }
        }

        entrada.close();
    }
}
