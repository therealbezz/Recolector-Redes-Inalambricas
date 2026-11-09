/* Author: Daniel Mihailov */

#include "collector.h"
#include "input.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Field positions also identify bits in the per-record presence mask. */
enum field_id
{
	FIELD_CELL,
	FIELD_MAC,
	FIELD_ESSID,
	FIELD_MODE,
	FIELD_CHANNEL,
	FIELD_ENCRYPTION,
	FIELD_QUALITY,
	FIELD_COUNT
};

/* Required fields must occur exactly once in each access-point record. */
#define ALL_FIELDS ((1U << FIELD_COUNT) - 1U)

/* A record under construction is not yet part of the temporary batch. */
struct pending_record
{
	struct access_point point;
	unsigned int fields;
};

/* Dynamic storage for successfully collected observations. */
struct collection collected = {0};

/* Grow data in five-slot increments until it can hold needed records.
 * Return 1 on success or 0 if growth stops, keeping existing records valid.
 */
static int reserve(struct collection *data, size_t needed)
{
	size_t capacity;
	struct access_point *points;

	while (data->capacity < needed)
	{
		if (SIZE_MAX / sizeof(*points) - data->capacity < COLLECTION_STEP)
		{
			return 0;
		}
		capacity = data->capacity + COLLECTION_STEP;
		points = realloc(data->points, capacity * sizeof(*points));
		if (points == NULL)
		{
			return 0;
		}
		data->points = points;
		data->capacity = capacity;
	}
	return 1;
}

/* Allocate five slots for the session. Takes no parameters.
 * Return 1 on success or 0 on allocation failure.
 */
int collector_init(void)
{
	return reserve(&collected, COLLECTION_STEP);
}

/* Free the session array and reset its metadata. Takes no parameters and
 * returns no value.
 */
void collector_destroy(void)
{
	free(collected.points);
	collected = (struct collection){0};
}

/* Return the index after the digits in text starting at index. */
static size_t digit_end(const char text[], size_t index)
{
	while (isdigit((unsigned char)text[index]))
	{
		index++;
	}
	return index;
}

/* Parse text at index up to the required delimiter, or return -1. */
static int parse_natural(const char text[], size_t index, char delimiter)
{
	size_t end = digit_end(text, index);
	long number;

	if (end == index || text[end] != delimiter)
	{
		return -1;
	}
	errno = 0;
	number = strtol(&text[index], NULL, 10);
	if (errno == ERANGE || number > INT_MAX)
	{
		return -1;
	}
	return (int)number;
}

/* Check the MAC address in text. Returns 1 if valid or 0 otherwise. */
static int valid_mac(const char text[])
{
	size_t index;
	char separator;

	if (strlen(text) != 17)
	{
		return 0;
	}
	separator = text[2];
	if (separator != ':' && separator != '-')
	{
		return 0;
	}
	for (index = 0; index < 17; index++)
	{
		if (index % 3 == 2)
		{
			if (text[index] != separator)
			{
				return 0;
			}
		}
		else if (!isxdigit((unsigned char)text[index]))
		{
			return 0;
		}
	}
	return 1;
}

/* Check text against the supported modes. Returns 1 if valid or 0
 * otherwise.
 */
static int valid_mode(const char text[])
{
	/* Accepted names are retained as text for printing without conversion. */
	static const char modes[][STRING_SIZE] =
	{
		"Auto", "Ad-Hoc", "Managed", "Master", "Repeater", "Secondary",
		"Monitor", "Unknown"
	};
	size_t index;

	for (index = 0; index < sizeof(modes) / sizeof(modes[0]); index++)
	{
		if (strcmp(text, modes[index]) == 0)
		{
			return 1;
		}
	}
	return 0;
}

/* Copy the quoted ESSID from text to output without its surrounding quotes.
 * Returns 1 on success or 0 for invalid text.
 */
static int parse_essid(const char text[], char output[STRING_SIZE])
{
	size_t length = strlen(text);

	if (length < 2 || text[0] != '"' || text[length - 1] != '"')
	{
		return 0;
	}
	if (length - 2 >= STRING_SIZE ||
		memchr(&text[1], '"', length - 2) != NULL)
	{
		return 0;
	}
	memcpy(output, &text[1], length - 2);
	output[length - 2] = '\0';
	return 1;
}

/* Decode field into point using value and the expected cell.
 * Return the updated point, with cell_id set to zero on invalid input.
 */
static struct access_point parse_field(struct access_point point,
	enum field_id field, const char value[], int cell)
{
	size_t separator;

	switch (field)
	{
		case FIELD_CELL:
			if (parse_natural(value, 0, '\0') != cell)
			{
				break;
			}
			point.cell_id = cell;
			return point;
		case FIELD_MAC:
			if (!valid_mac(value))
			{
				break;
			}
			strcpy(point.mac, value);
			return point;
		case FIELD_ESSID:
			if (parse_essid(value, point.essid))
			{
				return point;
			}
			break;
		case FIELD_MODE:
			if (!valid_mode(value))
			{
				break;
			}
			strcpy(point.mode, value);
			return point;
		case FIELD_CHANNEL:
			point.channel = parse_natural(value, 0, '\0');
			if (point.channel >= 0)
			{
				return point;
			}
			break;
		case FIELD_ENCRYPTION:
			if (strcmp(value, "on") != 0 && strcmp(value, "off") != 0)
			{
				break;
			}
			strcpy(point.encryption, value);
			return point;
		case FIELD_QUALITY:
			point.quality = parse_natural(value, 0, '/');
			if (point.quality < 0)
			{
				break;
			}
			separator = digit_end(value, 0);
			point.quality_max = parse_natural(value, separator + 1, '\0');
			if (point.quality_max > 0 && point.quality <= point.quality_max)
			{
				return point;
			}
			break;
		default:
			break;
	}
	point.cell_id = 0;
	return point;
}

/* Read line into pending for the expected cell. Returns the updated record
 * with cell_id set to zero on invalid input.
 */
static struct pending_record read_field(struct pending_record pending,
	const char line[],
	int cell)
{
	/* Labels are indexed in the same order as enum field_id. */
	static const char labels[FIELD_COUNT][STRING_SIZE] =
	{
		"Cell ", "Address:", "ESSID:", "Mode:", "Channel:",
		"Encryption key:", "Quality="
	};
	enum field_id field;
	size_t length;

	unsigned int bit;

	for (field = FIELD_CELL; field < FIELD_COUNT; field++)
	{
		length = strlen(labels[field]);
		if (strncmp(line, labels[field], length) != 0)
		{
			continue;
		}
		bit = 1U << field;
		if ((pending.fields & bit) != 0 ||
			(field != FIELD_CELL && pending.fields == 0))
		{
			pending.point.cell_id = 0;
			return pending;
		}
		while (line[length] == ' ' || line[length] == '\t')
		{
			length++;
		}
		pending.point = parse_field(pending.point, field, &line[length], cell);
		pending.fields |= bit;
		return pending;
	}
	if (strncmp(line, "Frequency:", 10) != 0 &&
		strncmp(line, "Signal level=", 13) != 0)
	{
		pending.point.cell_id = 0;
	}
	return pending;
}

/* Validate the requested cell file and append its records as one batch.
 * Return the status and number added. On failure, keep existing observations.
 */
struct collect_result collector_collect(int cell, const char path[])
{
	struct collection batch = {0};
	struct pending_record pending = {0};
	enum collect_status status = COLLECT_OK;
	char line[STRING_SIZE];
	size_t length;
	enum line_status line_result;
	FILE *file;

	if (cell < MIN_CELL || cell > MAX_CELL)
	{
		return (struct collect_result){COLLECT_INVALID, 0};
	}
	file = fopen(path, "r");
	if (file == NULL)
	{
		return (struct collect_result){COLLECT_FILE_ERROR, 0};
	}
	for (;;)
	{
		line_result = input_read_line(file, line);
		if (line_result == LINE_ERROR || line_result == LINE_LONG)
		{
			status = line_result == LINE_ERROR ?
				COLLECT_FILE_ERROR : COLLECT_INVALID;
			break;
		}
		length = line_result == LINE_OK ? strlen(line) : 0;
		while (length > 0 && isspace((unsigned char)line[length - 1]))
		{
			line[--length] = '\0';
		}
		if (pending.fields != 0 &&
			(line_result == LINE_END || strncmp(line, "Cell ", 5) == 0))
		{
			if (pending.fields != ALL_FIELDS)
			{
				status = COLLECT_INVALID;
				break;
			}
			if (!reserve(&batch, batch.count + 1))
			{
				status = COLLECT_NO_MEMORY;
				break;
			}
			batch.points[batch.count++] = pending.point;
			pending = (struct pending_record){0};
		}
		if (line_result == LINE_END)
		{
			break;
		}
		if (length == 0)
		{
			continue;
		}
		pending = read_field(pending, line, cell);
		if (pending.point.cell_id == 0)
		{
			status = COLLECT_INVALID;
			break;
		}
	}
	if (fclose(file) != 0)
	{
		status = COLLECT_FILE_ERROR;
	}
	if (status == COLLECT_OK && batch.count == 0)
	{
		status = COLLECT_EMPTY;
	}
	if (status == COLLECT_OK &&
		!reserve(&collected, collected.count + batch.count))
	{
		status = COLLECT_NO_MEMORY;
	}
	if (status == COLLECT_OK)
	{
		memcpy(&collected.points[collected.count], batch.points,
			batch.count * sizeof(batch.points[0]));
		collected.count += batch.count;
	}
	free(batch.points);
	return (struct collect_result){status,
		status == COLLECT_OK ? batch.count : 0};
}
