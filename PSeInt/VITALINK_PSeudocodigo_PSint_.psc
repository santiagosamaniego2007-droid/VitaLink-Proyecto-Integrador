Función valido <- EsDigito(simbolo)
Definir valido Como Lógico
valido <- simbolo='0' O simbolo='1' O simbolo='2' O simbolo='3' O simbolo='4' O simbolo='5' O simbolo='6' O simbolo='7' O simbolo='8' O simbolo='9'
FinFunción

Función valido <- ValidarHorario(textoHora)
Definir valido Como Lógico
Definir horas, minutos Como Entero
valido <- Falso
Si Longitud(textoHora)=5 Entonces
	Si Subcadena(textoHora,3,3)=':' Entonces
		Si EsDigito(Subcadena(textoHora,1,1)) Y EsDigito(Subcadena(textoHora,2,2)) Y EsDigito(Subcadena(textoHora,4,4)) Y EsDigito(Subcadena(textoHora,5,5)) Entonces
			horas <- ConvertirANumero(Subcadena(textoHora,1,2))
			minutos <- ConvertirANumero(Subcadena(textoHora,4,5))
			Si horas>=0 Y horas<=23 Y minutos>=0 Y minutos<=59 Entonces
				valido <- Verdadero
			FinSi
		FinSi
	FinSi
FinSi
FinFunción

Función total <- ContarPorEstado(estadoAvisos,cantidadHorarios,totalMedicamentos,estadoBuscado)
Definir total, i, j Como Entero
total <- 0
Para i<-1 Hasta totalMedicamentos Hacer
	Para j<-1 Hasta cantidadHorarios[i] Hacer
		Si estadoAvisos[i,j]=estadoBuscado Entonces
			total <- total+1
		FinSi
	FinPara
FinPara
FinFunción

SubProceso MostrarMenu
	Escribir '-------------------------------------------'
	Escribir '           VITALINK - MENU PRINCIPAL       '
	Escribir '-------------------------------------------'
	Escribir '1. Registrar un medicamento y su horario'
	Escribir '2. Consultar medicamentos y horarios'
	Escribir '3. Consultar recordatorios pendientes'
	Escribir '4. Simular la emision de un recordatorio'
	Escribir '5. Registrar la confirmacion de un aviso'
	Escribir '6. Consultar historial de avisos'
	Escribir '7. Finalizar el programa'
FinSubProceso

SubProceso RegistrarMedicamento(medicamentos,cantidadHorarios,horarios,estadoAvisos,totalMedicamentos Por Referencia)
	Definir nombreMedicamento, horarioIngresado Como Cadena
	Definir cantidad, j, nuevoIndice Como Entero
	Definir datosCompletos, horarioValido Como Lógico
	Si totalMedicamentos>=10 Entonces
		Escribir 'Capacidad maxima: 10 medicamentos.'
	SiNo
		Escribir 'Nombre del medicamento:'
		Leer nombreMedicamento
		datosCompletos <- Longitud(nombreMedicamento)>0
		Si  NO datosCompletos Entonces
			Escribir 'Registro rechazado: falta el nombre obligatorio.'
		FinSi
		Si datosCompletos Entonces
			Repetir
				Escribir 'Cantidad de horarios (1 a 3):'
				Leer cantidad
				Si cantidad<1 O cantidad>3 Entonces
					Escribir 'Debe ingresar de 1 a 3 horarios.'
				FinSi
			Hasta Que cantidad>=1 Y cantidad<=3
			nuevoIndice <- totalMedicamentos+1
			Para j<-1 Hasta cantidad Hacer
				Repetir
					Escribir 'Horario ', j, ' (HH:MM):'
					Leer horarioIngresado
					horarioValido <- ValidarHorario(horarioIngresado)
					Si  NO horarioValido Entonces
						Escribir 'Formato incorrecto. Ejemplo valido: 08:30.'
					FinSi
				Hasta Que horarioValido
				horarios[nuevoIndice,j]<-horarioIngresado
				estadoAvisos[nuevoIndice,j]<-0
			FinPara
			medicamentos[nuevoIndice] <- nombreMedicamento
			cantidadHorarios[nuevoIndice] <- cantidad
			totalMedicamentos <- nuevoIndice
			Escribir 'Medicamento y horarios registrados.'
		FinSi
	FinSi
FinSubProceso

SubProceso MostrarMedicamentosYHorarios(medicamentos,cantidadHorarios,horarios,totalMedicamentos)
	Definir i, j Como Entero
	Si totalMedicamentos=0 Entonces
		Escribir 'No hay medicamentos registrados.'
	SiNo
		Para i<-1 Hasta totalMedicamentos Hacer
			Escribir 'Medicamento ', i, ': ', medicamentos[i]
			Para j<-1 Hasta cantidadHorarios[i] Hacer
				Escribir '  Horario ', j, ': ', horarios[i,j]
			FinPara
		FinPara
	FinSi
FinSubProceso

SubProceso MostrarPendientes(medicamentos,cantidadHorarios,horarios,estadoAvisos,totalMedicamentos)
	Definir i, j, totalPendientes, programados, sinConfirmar Como Entero
	programados <- ContarPorEstado(estadoAvisos,cantidadHorarios,totalMedicamentos,0)
	sinConfirmar <- ContarPorEstado(estadoAvisos,cantidadHorarios,totalMedicamentos,1)
	totalPendientes <- programados+sinConfirmar
	Escribir 'Recordatorios pendientes: ', totalPendientes
	Escribir 'Programados (no emitidos): ', programados
	Escribir 'Emitidos sin confirmar: ', sinConfirmar
	Para i<-1 Hasta totalMedicamentos Hacer
		Para j<-1 Hasta cantidadHorarios[i] Hacer
			Si estadoAvisos[i,j]=0 Entonces
				Escribir medicamentos[i], ' - ', horarios[i,j], ' : PROGRAMADO'
			FinSi
			Si estadoAvisos[i,j]=1 Entonces
				Escribir medicamentos[i], ' - ', horarios[i,j], ' : SIN CONFIRMAR'
			FinSi
		FinPara
	FinPara
FinSubProceso

SubProceso SimularRecordatorio(medicamentos,cantidadHorarios,horarios,estadoAvisos,historialAvisos,totalMedicamentos,totalHistorial Por Referencia)
	Definir horaSimulada Como Cadena
	Definir i, j, posicion, emitidos Como Entero
	Definir horarioValido, coincide Como Lógico
	Repetir
		Escribir 'Ingrese la hora simulada (HH:MM):'
		Leer horaSimulada
		horarioValido <- ValidarHorario(horaSimulada)
		Si  NO horarioValido Entonces
			Escribir 'Hora invalida. Use HH:MM.'
		FinSi
	Hasta Que horarioValido
	coincide <- Falso
	emitidos <- 0
	Para i<-1 Hasta totalMedicamentos Hacer
		Para j<-1 Hasta cantidadHorarios[i] Hacer
			Si horarios[i,j]=horaSimulada Entonces
				coincide <- Verdadero
				Si estadoAvisos[i,j]=0 Entonces
					estadoAvisos[i,j]<-1
					posicion <- (i-1)*3+j
					historialAvisos[posicion] <- 'EMITIDO SIN CONFIRMAR'
					totalHistorial <- totalHistorial+1
					emitidos <- emitidos+1
					Escribir 'AVISO (LED Y ZUMBADOR SIMULADOS): ', medicamentos[i], ' - ', horarios[i,j]
				SiNo
					Escribir medicamentos[i], ' - ', horarios[i,j], ' : aviso ya gestionado en esta ronda.'
				FinSi
			FinSi
		FinPara
	FinPara
	Si  NO coincide Entonces
		Escribir 'No existe un horario registrado para esa hora.'
	FinSi
	Escribir 'Avisos nuevos emitidos: ', emitidos
FinSubProceso

SubProceso RegistrarConfirmacion(medicamentos,cantidadHorarios,horarios,estadoAvisos,historialAvisos,totalMedicamentos)
	Definir medicamentoSeleccionado, horarioSeleccionado, confirmacion, posicion Como Entero
	Definir avisoConfirmado Como Lógico
	Si totalMedicamentos=0 Entonces
		Escribir 'Primero registre un medicamento.'
	SiNo
		MostrarMedicamentosYHorarios(medicamentos,cantidadHorarios,horarios,totalMedicamentos)
		Escribir 'Numero del medicamento:'
		Leer medicamentoSeleccionado
		Si medicamentoSeleccionado<1 O medicamentoSeleccionado>totalMedicamentos Entonces
			Escribir 'Medicamento inexistente.'
		SiNo
			Escribir 'Numero de horario (1 a ', cantidadHorarios[medicamentoSeleccionado], '):'
			Leer horarioSeleccionado
			Si horarioSeleccionado<1 O horarioSeleccionado>cantidadHorarios[medicamentoSeleccionado] Entonces
				Escribir 'Horario inexistente.'
			SiNo
				Si estadoAvisos[medicamentoSeleccionado,horarioSeleccionado]=1 Entonces
					Repetir
						Escribir 'Atendio el aviso? 1 = Si, 2 = No:'
						Leer confirmacion
					Hasta Que confirmacion=1 O confirmacion=2
					avisoConfirmado <- confirmacion=1
					posicion <- (medicamentoSeleccionado-1)*3+horarioSeleccionado
					Si avisoConfirmado Entonces
						estadoAvisos[medicamentoSeleccionado,horarioSeleccionado]<-2
						historialAvisos[posicion] <- 'CONFIRMADO POR EL USUARIO'
						Escribir 'Aviso registrado como atendido.'
					SiNo
						historialAvisos[posicion] <- 'EMITIDO SIN CONFIRMAR'
						Escribir 'El aviso permanece sin confirmar.'
					FinSi
				SiNo
					Escribir 'Este horario no tiene un aviso emitido pendiente de confirmar.'
				FinSi
			FinSi
		FinSi
	FinSi
FinSubProceso

SubProceso MostrarHistorial(medicamentos,cantidadHorarios,horarios,estadoAvisos,historialAvisos,totalMedicamentos,totalHistorial)
	Definir i, j, posicion, totalNoConfirmados Como Entero
	totalNoConfirmados <- ContarPorEstado(estadoAvisos,cantidadHorarios,totalMedicamentos,1)
	Escribir 'Total de avisos emitidos: ', totalHistorial
	Escribir 'Avisos emitidos no confirmados: ', totalNoConfirmados
	Si totalHistorial=0 Entonces
		Escribir 'Todavia no se emitieron avisos.'
	SiNo
		Para i<-1 Hasta totalMedicamentos Hacer
			Para j<-1 Hasta cantidadHorarios[i] Hacer
				Si estadoAvisos[i,j]=1 Entonces
					posicion <- (i-1)*3+j
					Escribir medicamentos[i], ' - ', horarios[i,j], ' : ', historialAvisos[posicion]
				FinSi
				Si estadoAvisos[i,j]=2 Entonces
					posicion <- (i-1)*3+j
					Escribir medicamentos[i], ' - ', horarios[i,j], ' : ', historialAvisos[posicion]
				FinSi
			FinPara
		FinPara
	FinSi
FinSubProceso

Algoritmo Vitalink
	Definir opcionMenu, totalMedicamentos, totalHistorial Como Entero
	Definir medicamentos, horarios, historialAvisos Como Cadena
	Definir cantidadHorarios, estadoAvisos Como Entero
	Dimensionar medicamentos(10), cantidadHorarios(10)
	Dimensionar horarios(10,3), estadoAvisos(10,3)
	Dimensionar historialAvisos(30)
	totalMedicamentos <- 0
	totalHistorial <- 0
	opcionMenu <- 0
	Mientras opcionMenu<>7 Hacer
		MostrarMenu
		Escribir 'Seleccione una opcion (1-7):'
		Leer opcionMenu
		Según opcionMenu Hacer
	1:
		RegistrarMedicamento(medicamentos,cantidadHorarios,horarios,estadoAvisos,totalMedicamentos)
	2:
		MostrarMedicamentosYHorarios(medicamentos,cantidadHorarios,horarios,totalMedicamentos)
	3:
		MostrarPendientes(medicamentos,cantidadHorarios,horarios,estadoAvisos,totalMedicamentos)
	4:
		SimularRecordatorio(medicamentos,cantidadHorarios,horarios,estadoAvisos,historialAvisos,totalMedicamentos,totalHistorial)
	5:
		RegistrarConfirmacion(medicamentos,cantidadHorarios,horarios,estadoAvisos,historialAvisos,totalMedicamentos)
	6:
		MostrarHistorial(medicamentos,cantidadHorarios,horarios,estadoAvisos,historialAvisos,totalMedicamentos,totalHistorial)
	7:
		Escribir 'Finalizando VITALINK.'
	De Otro Modo:
		Escribir 'Opcion no valida.'
FinSegún
FinMientras
FinAlgoritmo

