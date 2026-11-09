/* Author: Daniel Mihailov */

#include "menu.h"

#include <stdio.h>
#include "collector.h"
#include "display.h"
#include "input.h"

/* Print the five Entrega 2 menu options. Takes no parameters and returns no
 * value.
 */
void show_menu(void)
{
	puts("\n[2026] SUCEM S.L. Recolector de redes inalambricas");
	puts("[ 1] wificollector_quit");
	puts("[ 2] wificollector_collect");
	puts("[ 6] wificollector_sort");
	puts("[ 9] wificollector_display");
	puts("[10] wificollector_display_all");
}

/* Print the collection status and the number added. Returns no value. */
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
		case COLLECT_NO_MEMORY:
			puts("Error: memoria insuficiente para recolectar la celda.");
			break;
	}
}

/* Prompt for cells and collect them. Takes no parameters. Returns 1 to
 * continue or 0 at end of input.
 */
static int wificollector_collect(void)
{
	char path[STRING_SIZE];
	int cell;
	int again;
	struct collect_result result;

	do
	{
		cell = input_number("Que celda quiere recolectar? (1 - 21): ",
			MIN_CELL, MAX_CELL);
		if (cell < 0)
		{
			return 0;
		}
		snprintf(path, sizeof(path), "cells/info_cell_%d.txt", cell);
		result = collector_collect(cell, path);
		report_collection(result.status, result.added);
		again = input_confirm("Desea anadir otro punto de acceso? [s/N]: ");
		if (again < 0)
		{
			return 0;
		}
	} while (again);
	return 1;
}

/* Prompt for cells and display them. Takes no parameters. Returns 1 to
 * continue or 0 at end of input.
 */
static int wificollector_display(void)
{
	int cell;
	int again;

	do
	{
		cell = input_number("Indique el numero de la celda (1 - 21): ",
			MIN_CELL, MAX_CELL);
		if (cell < 0)
		{
			return 0;
		}
		display_cell(cell);
		again = input_confirm("Desea imprimir otra celda? [s/N]: ");
		if (again < 0)
		{
			return 0;
		}
	} while (again);
	return 1;
}

/* Dispatch the selected option. Returns 1 to continue or 0 on confirmed
 * quit or end of input.
 */
int run_option(int option)
{
	int quit;

	switch (option)
	{
		case 1:
			quit = input_confirm("Esta seguro de que desea salir? [s/N]: ");
			return quit == 0;
		case 2:
			return wificollector_collect();
		case 6:
			display_sorted();
			break;
		case 9:
			return wificollector_display();
		case 10:
			display_all();
			break;
		default:
			puts("Opcion no disponible. Elija 1, 2, 6, 9 o 10.");
	}
	return 1;
}
