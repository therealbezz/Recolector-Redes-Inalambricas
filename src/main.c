/* Author: Daniel Mihailov
 * Team: Daniel Mihailov, Diego Medina, Alba Serrano
 */

#include "main.h"

#include <stdio.h>
#include "collector.h"
#include "input.h"
#include "menu.h"

/* Run the terminal application. Takes no parameters. Returns 0 on
 * normal exit or 1 if initial allocation fails.
 */
int main(void)
{
	int option;

	if (!collector_init())
	{
		fputs("Error: memoria insuficiente.\n", stderr);
		return 1;
	}

	do
	{
		show_menu();
		option = input_number("Opcion elegida: ", 1, 10);
	} while (option >= 0 && run_option(option));
	collector_destroy();
	puts("Programa terminado.");
	return 0;
}
