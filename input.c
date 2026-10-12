#include "input.h"

/* Foundation stub for the shared line-reading interface. */
enum line_status input_read_line(FILE *stream, char line[STRING_SIZE])
{
	(void)stream;
	(void)line;
	return LINE_END;
}

/* Foundation stub for integer input. */
int input_number(const char *prompt, int minimum, int maximum, int *value)
{
	(void)prompt;
	(void)minimum;
	(void)maximum;
	(void)value;
	return 0;
}

/* Foundation stub for confirmation input. */
int input_confirm(const char *prompt, int *yes)
{
	(void)prompt;
	(void)yes;
	return 0;
}
