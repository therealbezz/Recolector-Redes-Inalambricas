#include "collector.h"
#include "input.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
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

/* Parse a nonnegative decimal integer and return its first unused character. */
static int parse_natural(const char *text, int *value, const char **rest)
{
	char *end;
	long number;

	if (!isdigit((unsigned char)*text))
	{
		return 0;
	}
	errno = 0;
	number = strtol(text, &end, 10);
	if (errno == ERANGE || number > INT_MAX)
	{
		return 0;
	}
	*value = (int)number;
	*rest = end;
	return 1;
}

/* Require an entire value to be a decimal integer with no trailing data. */
static int parse_integer(const char *text, int *value)
{
	const char *rest;

	return parse_natural(text, value, &rest) && *rest == '\0';
}

/* Accept the two specified MAC separators, used consistently throughout. */
static int valid_mac(const char *text)
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

/* Check a mode against every mode allowed by the assignment. */
static int valid_mode(const char *text)
{
	/* Accepted names are retained as text for printing without conversion. */
	static const char *const modes[] =
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

/* Strip the surrounding ESSID quotes while preserving interior spaces. */
static int parse_essid(const char *text, char output[STRING_SIZE])
{
	size_t length = strlen(text);

	if (length < 2 || text[0] != '"' || text[length - 1] != '"')
	{
		return 0;
	}
	if (length - 2 >= STRING_SIZE ||
		memchr(text + 1, '"', length - 2) != NULL)
	{
		return 0;
	}
	memcpy(output, text + 1, length - 2);
	output[length - 2] = '\0';
	return 1;
}

/* Quality requires two bounded naturals and a positive maximum. */
static int parse_quality(const char *text, struct access_point *point)
{
	const char *rest;

	if (!parse_natural(text, &point->quality, &rest) || *rest != '/')
	{
		return 0;
	}
	return parse_integer(rest + 1, &point->quality_max) &&
		point->quality_max > 0 && point->quality <= point->quality_max;
}

/* Decode a known field; copied strings fit because the full line fits. */
static int parse_field(struct access_point *point, enum field_id field,
	const char *value, int cell)
{
	switch (field)
	{
		case FIELD_CELL:
			return parse_integer(value, &point->cell_id) &&
				point->cell_id == cell;
		case FIELD_MAC:
			if (!valid_mac(value))
			{
				return 0;
			}
			strcpy(point->mac, value);
			return 1;
		case FIELD_ESSID:
			return parse_essid(value, point->essid);
		case FIELD_MODE:
			if (!valid_mode(value))
			{
				return 0;
			}
			strcpy(point->mode, value);
			return 1;
		case FIELD_CHANNEL:
			return parse_integer(value, &point->channel);
		case FIELD_ENCRYPTION:
			if (strcmp(value, "on") != 0 && strcmp(value, "off") != 0)
			{
				return 0;
			}
			strcpy(point->encryption, value);
			return 1;
		case FIELD_QUALITY:
			return parse_quality(value, point);
		default:
			return 0;
	}
}

/* Match field labels and reject repeated fields or fields before Cell. */
static int read_field(struct pending_record *pending, const char *line,
	int cell)
{
	/* Labels are indexed in the same order as enum field_id. */
	static const char *const labels[FIELD_COUNT] =
	{
		"Cell ", "Address:", "ESSID:", "Mode:", "Channel:",
		"Encryption key:", "Quality="
	};
	enum field_id field;
	size_t length;
	const char *value;
	unsigned int bit;

	for (field = FIELD_CELL; field < FIELD_COUNT; field++)
	{
		length = strlen(labels[field]);
		if (strncmp(line, labels[field], length) != 0)
		{
			continue;
		}
		bit = 1U << field;
		if ((pending->fields & bit) != 0 ||
			(field != FIELD_CELL && pending->fields == 0))
		{
			return 0;
		}
		value = line + length;
		while (*value == ' ' || *value == '\t')
		{
			value++;
		}
		if (!parse_field(&pending->point, field, value, cell))
		{
			return 0;
		}
		pending->fields |= bit;
		return 1;
	}
	return pending->fields != 0 &&
		(strncmp(line, "Frequency:", 10) == 0 ||
		strncmp(line, "Signal level=", 13) == 0);
}

/* Finish a validated record in temporary storage; never exceed capacity. */
static enum collect_status finish_record(struct collection *batch,
	struct pending_record *pending)
{
	if (pending->fields != ALL_FIELDS)
	{
		return COLLECT_INVALID;
	}
	if (batch->count == MAX_ACCESS_POINTS)
	{
		return COLLECT_FULL;
	}
	batch->points[batch->count++] = pending->point;
	memset(pending, 0, sizeof(*pending));
	return COLLECT_OK;
}

/* Parse every physical line; only Cell starts a new access-point record. */
static enum collect_status read_batch(FILE *file, int cell,
	struct collection *batch)
{
	struct pending_record pending = {0};
	char line[STRING_SIZE];
	size_t length;
	enum line_status line_result;
	enum collect_status result;

	while ((line_result = input_read_line(file, line)) == LINE_OK)
	{
		length = strlen(line);
		while (length > 0 && isspace((unsigned char)line[length - 1]))
		{
			line[--length] = '\0';
		}
		if (length == 0)
		{
			continue;
		}
		if (strncmp(line, "Cell ", 5) == 0 && pending.fields != 0)
		{
			result = finish_record(batch, &pending);
			if (result != COLLECT_OK)
			{
				return result;
			}
		}
		if (!read_field(&pending, line, cell))
		{
			return COLLECT_INVALID;
		}
	}
	if (line_result != LINE_END)
	{
		return line_result == LINE_ERROR ?
			COLLECT_FILE_ERROR : COLLECT_INVALID;
	}
	if (pending.fields == 0)
	{
		return COLLECT_EMPTY;
	}
	return finish_record(batch, &pending);
}

/* Commit a fully parsed batch only after capacity and stream-close checks. */
enum collect_status collector_collect(struct collection *data, int cell,
	const char *path, size_t *added)
{
	struct collection batch = {0};
	enum collect_status result;
	FILE *file;

	*added = 0;
	if (cell < MIN_CELL || cell > MAX_CELL ||
		data->count > MAX_ACCESS_POINTS)
	{
		return COLLECT_INVALID;
	}
	file = fopen(path, "r");
	if (file == NULL)
	{
		return COLLECT_FILE_ERROR;
	}
	result = read_batch(file, cell, &batch);
	if (fclose(file) != 0)
	{
		return COLLECT_FILE_ERROR;
	}
	if (result != COLLECT_OK)
	{
		return result;
	}
	if (batch.count > MAX_ACCESS_POINTS - data->count)
	{
		return COLLECT_FULL;
	}
	memcpy(data->points + data->count, batch.points,
		batch.count * sizeof(batch.points[0]));
	data->count += batch.count;
	*added = batch.count;
	return COLLECT_OK;
}
