#include "test_collector.h"
#include "collector.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Check both record counts and representative fields from the real corpus. */
static void check_corpus(struct collection *data)
{
	char path[STRING_SIZE];
	int cell;
	size_t added;
	size_t expected;

	for (cell = MIN_CELL; cell <= MAX_CELL; cell++)
	{
		snprintf(path, sizeof(path), "cells/info_cell_%d.txt", cell);
		expected = cell <= 6 ? 3 : (cell == 21 ? 1 : 2);
		assert(collector_collect(data, cell, path, &added) == COLLECT_OK);
		assert(added == expected);
	}
	assert(data->count == 47);
	assert(strcmp(data->points[0].mac, "00:01:38:1F:CB:3E") == 0);
	assert(strcmp(data->points[15].essid, "Miguel 3") == 0);
	assert(strcmp(data->points[20].mode, "Ad-Hoc") == 0);
	assert(strcmp(data->points[20].encryption, "off") == 0);
	assert(data->points[24].channel == 140);
	assert(data->points[46].quality == 31);
	assert(data->points[46].quality_max == 70);
}

/* A failure must preserve every byte of previously stored data. */
static void check_failure(struct collection *data, const char *path,
	enum collect_status expected)
{
	struct collection before = *data;
	size_t added = 99;

	assert(collector_collect(data, 1, path, &added) == expected);
	assert(added == 0);
	assert(memcmp(data, &before, sizeof(*data)) == 0);
}

/* Reach exactly 200 observations and reject an overflowing batch atomically. */
static void check_capacity(void)
{
	struct collection data = {0};
	size_t added;
	int index;

	for (index = 0; index < 66; index++)
	{
		assert(collector_collect(&data, 1, "cells/info_cell_1.txt",
			&added) == COLLECT_OK);
	}
	assert(data.count == 198);
	check_failure(&data, "cells/info_cell_1.txt", COLLECT_FULL);
	for (index = 0; index < 2; index++)
	{
		assert(collector_collect(&data, 21, "cells/info_cell_21.txt",
			&added) == COLLECT_OK);
	}
	assert(data.count == MAX_ACCESS_POINTS);
	check_failure(&data, "cells/info_cell_1.txt", COLLECT_FULL);
}

/* Exercise malformed fields, missing files, long lines, and oversized files. */
static void check_fixtures(struct collection *data, const char *directory)
{
	/* Each fixture name has its expected failure classification. */
	static const char *const names[] =
	{
		"missing", "empty", "blank", "truncated", "bad_mac", "bad_mode",
		"bad_encryption", "bad_quality", "zero_quality", "bad_channel",
		"bad_essid", "wrong_cell", "duplicate", "long_line", "partial",
		"too_many", "unknown_field", "bad_number"
	};
	char path[STRING_SIZE];
	size_t index;
	size_t added;
	enum collect_status expected;

	for (index = 0; index < sizeof(names) / sizeof(names[0]); index++)
	{
		snprintf(path, sizeof(path), "%s/%s.txt", directory, names[index]);
		expected = COLLECT_INVALID;
		if (index == 0)
		{
			expected = COLLECT_FILE_ERROR;
		}
		else if (index == 1 || index == 2)
		{
			expected = COLLECT_EMPTY;
		}
		else if (strcmp(names[index], "too_many") == 0)
		{
			expected = COLLECT_FULL;
		}
		check_failure(data, path, expected);
	}
	snprintf(path, sizeof(path), "%s/valid_modes.txt", directory);
	assert(collector_collect(data, 1, path, &added) == COLLECT_OK);
	assert(added == 8);
	snprintf(path, sizeof(path), "%s/crlf.txt", directory);
	assert(collector_collect(data, 1, path, &added) == COLLECT_OK);
	assert(added == 1);
	snprintf(path, sizeof(path), "%s/no_newline.txt", directory);
	assert(collector_collect(data, 1, path, &added) == COLLECT_OK);
	assert(added == 1);
}

/* Fixture creation and process timeouts are managed by the Python runner. */
int main(int argc, char *argv[])
{
	struct collection data = {0};

	assert(argc == 2);
	check_corpus(&data);
	check_fixtures(&data, argv[1]);
	check_capacity();
	puts("Collector checks passed.");
	return 0;
}
