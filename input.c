#include "input.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

/* Read one physical line, stripping LF/CRLF and draining oversized input. */
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

/* Prompt until a complete short line is available, or input ends. */
static int prompt_line(const char *prompt, char line[STRING_SIZE])
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

/* Convert a full line to a bounded integer, rejecting overflow and junk. */
int input_number(const char *prompt, int minimum, int maximum, int *value)
{
	char line[STRING_SIZE];
	char *end;
	long number;

	while (prompt_line(prompt, line))
	{
		errno = 0;
		number = strtol(line, &end, 10);
		if (end != line)
		{
			while (isspace((unsigned char)*end))
			{
				end++;
			}
			if (errno != ERANGE && *end == '\0' &&
				number >= minimum && number <= maximum)
			{
				*value = (int)number;
				return 1;
			}
		}
		printf("Introduzca un entero entre %d y %d.\n", minimum, maximum);
	}
	return 0;
}

/* Only the exact answers s and N are accepted, as the assignment requires. */
int input_confirm(const char *prompt, int *yes)
{
	char line[STRING_SIZE];

	while (prompt_line(prompt, line))
	{
		if (strcmp(line, "s") == 0 || strcmp(line, "N") == 0)
		{
			*yes = line[0] == 's';
			return 1;
		}
		puts("Responda exactamente s o N.");
	}
	return 0;
}
