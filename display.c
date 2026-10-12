#include "display.h"

#include <stdio.h>

/* Both display operations use this identical seven-field representation. */
static void display_point(const struct access_point *point)
{
	printf("Cell %d\n", point->cell_id);
	printf("Address: %s\n", point->mac);
	printf("ESSID:\"%s\"\n", point->essid);
	printf("Mode:%s\n", point->mode);
	printf("Channel:%d\n", point->channel);
	printf("Encryption key:%s\n", point->encryption);
	printf("Quality=%d/%d\n\n", point->quality, point->quality_max);
}

/* Print every observation for the cell, including repeated collections. */
void display_cell(const struct collection *data, int cell)
{
	size_t index;
	int found = 0;

	for (index = 0; index < data->count; index++)
	{
		if (data->points[index].cell_id == cell)
		{
			display_point(&data->points[index]);
			found = 1;
		}
	}
	if (!found)
	{
		printf("Error: la celda %d no ha sido recolectada.\n", cell);
	}
}

/* Display in insertion order without modifying the collection. */
void display_all(const struct collection *data)
{
	size_t index;

	if (data->count == 0)
	{
		puts("No hay celdas almacenadas.");
	}
	for (index = 0; index < data->count; index++)
	{
		display_point(&data->points[index]);
	}
}
