#include "main.h"

#include <stdio.h>
#include "collector.h"
#include "display.h"
#include "input.h"

/* Present only the four operations required by Entrega 1. */
static void show_menu(void)
{
	puts("\n[2026] SUCEM S.L. Recolector de redes inalambricas");
	puts("[ 1] wificollector_quit");
	puts("[ 2] wificollector_collect");
	puts("[ 9] wificollector_display");
	puts("[10] wificollector_display_all");
}

/* Explain collection failures without changing the stored observations. */
static void report_collection(enum collect_status status, size_t added)
{
	switch (status)
	{
		case COLLECT_OK:
			printf("Se han agregado %zu puntos de acceso.\n", added);
			break;
		case COLLECT_FILE_ERROR:
			puts("Error al abrir, leer o cerrar el fichero de la celda.");
			break;
		case COLLECT_INVALID:
			puts("Error: el fichero contiene datos invalidos.");
			break;
		case COLLECT_EMPTY:
			puts("Error: el fichero no contiene puntos de acceso.");
			break;
		case COLLECT_FULL:
			puts("Error: se superaria el limite de 200 puntos de acceso.");
			break;
	}
}

/* Collect complete batches until N is entered; return zero on input end. */
static int wificollector_collect(struct collection *data)
{
	char path[STRING_SIZE];
	int cell;
	int again;
	size_t added;
	enum collect_status status;

	do
	{
		if (!input_number("Que celda quiere recolectar? (1 - 21): ",
			MIN_CELL, MAX_CELL, &cell))
		{
			return 0;
		}
		snprintf(path, sizeof(path), "cells/info_cell_%d.txt", cell);
		status = collector_collect(data, cell, path, &added);
		report_collection(status, added);
		if (!input_confirm("Desea anadir otro punto de acceso? [s/N]: ",
			&again))
		{
			return 0;
		}
	} while (again);
	return 1;
}

/* Ask for another cell after both successful and unsuccessful displays. */
static int wificollector_display(const struct collection *data)
{
	int cell;
	int again;

	do
	{
		if (!input_number("Indique el numero de la celda (1 - 21): ",
			MIN_CELL, MAX_CELL, &cell))
		{
			return 0;
		}
		display_cell(data, cell);
		if (!input_confirm("Desea imprimir otra celda? [s/N]: ", &again))
		{
			return 0;
		}
	} while (again);
	return 1;
}

/* Dispatch operations; zero means confirmed quit or end of keyboard input. */
static int run_option(struct collection *data, int option)
{
	int quit;

	switch (option)
	{
		case 1:
			if (!input_confirm("Esta seguro de que desea salir? [s/N]: ",
				&quit))
			{
				return 0;
			}
			return !quit;
		case 2:
			return wificollector_collect(data);
		case 9:
			return wificollector_display(data);
		case 10:
			display_all(data);
			break;
		default:
			puts("Opcion no disponible. Elija 1, 2, 9 o 10.");
	}
	return 1;
}

/* Own the fixed collection for the lifetime of the terminal application. */
int main(void)
{
	struct collection data = {0};
	int option;
	int running = 1;

	while (running)
	{
		show_menu();
		if (!input_number("Opcion elegida: ", 1, 10, &option))
		{
			break;
		}
		running = run_option(&data, option);
	}
	puts("Programa terminado.");
	return 0;
}
