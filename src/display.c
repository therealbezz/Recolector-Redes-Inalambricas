/* Author: Daniel Mihailov
 * Team: Daniel Mihailov, Diego Medina, Alba Serrano
 */

#include "display.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Print the seven fields of point. Returns no value. */
static void display_point(struct access_point point)
{
	printf("Cell %d\n", point.cell_id);
	printf("Address: %s\n", point.mac);
	printf("ESSID:\"%s\"\n", point.essid);
	printf("Mode:%s\n", point.mode);
	printf("Channel:%d\n", point.channel);
	printf("Encryption key:%s\n", point.encryption);
	printf("Quality=%d/%d\n\n", point.quality, point.quality_max);
}

/* Print observations matching cell, or a message if absent. Returns no
 * value.
 */
void display_cell(int cell)
{
	size_t index;
	int found = 0;

	for (index = 0; index < collected.count; index++)
	{
		if (collected.points[index].cell_id == cell)
		{
			display_point(collected.points[index]);
			found = 1;
		}
	}
	if (!found)
	{
		printf("Error: la celda %d no ha sido recolectada.\n", cell);
	}
}

/* Print all observations in insertion order. Takes no parameters and
 * returns no value.
 */
void display_all(void)
{
	size_t index;

	if (collected.count == 0)
	{
		puts("No hay celdas almacenadas.");
	}
	for (index = 0; index < collected.count; index++)
	{
		display_point(collected.points[index]);
	}
}

/* Compare two collection indices for qsort. Return a negative value, zero
 * or a positive value according to descending quality and insertion order.
 */
static int compare_quality(const void *left, const void *right)
{
	size_t first = *(const size_t *)left;
	size_t second = *(const size_t *)right;
	const struct access_point *a = &collected.points[first];
	const struct access_point *b = &collected.points[second];
	uintmax_t a_scaled = (uintmax_t)a->quality * b->quality_max;
	uintmax_t b_scaled = (uintmax_t)b->quality * a->quality_max;

	if (a_scaled != b_scaled)
	{
		return a_scaled > b_scaled ? -1 : 1;
	}
	return (first > second) - (first < second);
}

/* Display observations by descending quality without changing the saved
 * order. Takes no parameters and returns no value.
 */
void display_sorted(void)
{
	size_t *order;
	size_t index;

	if (collected.count == 0)
	{
		display_all();
		return;
	}
	order = malloc(collected.count * sizeof(*order));
	if (order == NULL)
	{
		puts("Error: memoria insuficiente para ordenar los puntos de acceso.");
		return;
	}
	for (index = 0; index < collected.count; index++)
	{
		order[index] = index;
	}
	qsort(order, collected.count, sizeof(*order), compare_quality);
	for (index = 0; index < collected.count; index++)
	{
		display_point(collected.points[order[index]]);
	}
	free(order);
}
