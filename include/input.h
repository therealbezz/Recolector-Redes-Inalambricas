/* Author: Daniel Mihailov */

#ifndef INPUT_H
#define INPUT_H

#include <stdio.h>
#include "collector.h"

/* Complete line, normal end of input, oversized line, or stream error. */
enum line_status
{
	LINE_OK,
	LINE_END,
	LINE_LONG,
	LINE_ERROR
};

/* Read at most 79 characters and consume the entire physical line. */
enum line_status input_read_line(FILE *stream, char line[STRING_SIZE]);
/* Read an integer in the inclusive range; return -1 at end of input. */
int input_number(const char prompt[], int minimum, int maximum);
/* Read exactly s or N; return -1 at end of input. */
int input_confirm(const char prompt[]);

#endif /* INPUT_H */
