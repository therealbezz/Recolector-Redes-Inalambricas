#ifndef DISPLAY_H
#define DISPLAY_H

#include "collector.h"

/* Print all stored observations for cell, or an error if none exist. */
void display_cell(const struct collection *data, int cell);
/* Print all observations in insertion order, or an empty message. */
void display_all(const struct collection *data);

#endif /* DISPLAY_H */
