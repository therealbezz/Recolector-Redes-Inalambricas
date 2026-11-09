/* Author: Daniel Mihailov
 * Team: Daniel Mihailov, Diego Medina, Alba Serrano
 */

#include "input.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

/* Read one physical line from stream into line, stripping LF or CRLF.
 * Returns the line status.
 */
enum line_status input_read_line(FILE *stream, char line[STRING_SIZE])
{
	size_t length;
	int next;

	if (fgets(line, STRING_SIZE, stream) == NULL)
	{
		return ferror(stream) ? LINE_ERROR : LINE_END;
	}
	length = strlen(line);
	if (length > 0 && line[length - 1] == '\n')
	{
		line[--length] = '\0';
	}
	else if (!feof(stream))
	{
		next = fgetc(stream);
		if (next == '\r')
		{
			next = fgetc(stream);
		}
		if (next != '\n' && next != EOF)
		{
			while (next != '\n' && next != EOF)
			{
				next = fgetc(stream);
			}
			return ferror(stream) ? LINE_ERROR : LINE_LONG;
		}
	}
	if (length > 0 && line[length - 1] == '\r')
	{
		line[length - 1] = '\0';
	}
	return ferror(stream) ? LINE_ERROR : LINE_OK;
}

/* Print prompt and read a short line into line. Returns 1 on success or 0
 * at end of input or on error.
 */
static int prompt_line(const char prompt[], char line[STRING_SIZE])
{
	enum line_status status;

	for (;;)
	{
		fputs(prompt, stdout);
		fflush(stdout);
		status = input_read_line(stdin, line);
		if (status == LINE_OK)
		{
			return 1;
		}
		if (status != LINE_LONG)
		{
			if (status == LINE_ERROR)
			{
				fputs("Error de lectura del teclado.\n", stderr);
			}
			return 0;
		}
		puts("Entrada demasiado larga (maximo 79 caracteres).");
	}
}

/* Print prompt and read an integer between minimum and maximum. Returns the
 * integer or -1 if input ends or fails.
 */
int input_number(const char prompt[], int minimum, int maximum)
{
	char line[STRING_SIZE];
	size_t index;
	long number;

	while (prompt_line(prompt, line))
	{
		index = 0;
		while (isspace((unsigned char)line[index]))
		{
			index++;
		}
		if (line[index] == '+' || line[index] == '-')
		{
			index++;
		}
		if (isdigit((unsigned char)line[index]))
		{
			errno = 0;
			number = strtol(line, NULL, 10);
			while (isdigit((unsigned char)line[index]))
			{
				index++;
			}
			while (isspace((unsigned char)line[index]))
			{
				index++;
			}
			if (errno != ERANGE && line[index] == '\0' &&
				number >= minimum && number <= maximum)
			{
				return (int)number;
			}
		}
		printf("Introduzca un entero entre %d y %d.\n", minimum, maximum);
	}
	return -1;
}

/* Print prompt and read s or N. Returns 1 for s, 0 for N, or -1 if input
 * ends or fails.
 */
int input_confirm(const char prompt[])
{
	char line[STRING_SIZE];

	while (prompt_line(prompt, line))
	{
		if (strcmp(line, "s") == 0 || strcmp(line, "N") == 0)
		{
			return line[0] == 's';
		}
		puts("Responda exactamente s o N.");
	}
	return -1;
}
