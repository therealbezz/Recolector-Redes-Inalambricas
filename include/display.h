/* Author: Daniel Mihailov
 * Team: Daniel Mihailov, Diego Medina, Alba Serrano
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include "collector.h"

/* Print all stored observations for cell, or an error if none exist. */
void display_cell(int cell);
/* Print all observations in insertion order, or an empty message. */
void display_all(void);

/* Print all observations in descending order of signal quality. */
void display_sorted(void);

#endif /* DISPLAY_H */
