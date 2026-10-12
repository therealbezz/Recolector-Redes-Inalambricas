#ifndef COLLECTOR_H
#define COLLECTOR_H

#include <stddef.h>

/* Maximum number of stored access-point observations. */
#define MAX_ACCESS_POINTS 200
/* String buffer capacity, including the terminating null character. */
#define STRING_SIZE 80
/* Inclusive limits for the supplied cell identifiers. */
#define MIN_CELL 1
#define MAX_CELL 21

/* The seven required fields; quality consists of two integers. */
struct access_point
{
	int cell_id;
	char mac[STRING_SIZE];
	char essid[STRING_SIZE];
	char mode[STRING_SIZE];
	char encryption[STRING_SIZE];
	int channel;
	int quality;
	int quality_max;
};

/* Fixed storage of observations in collection order. */
struct collection
{
	struct access_point points[MAX_ACCESS_POINTS];
	size_t count;
};

/* Collection outcomes; failures leave the collection unchanged. */
enum collect_status
{
	COLLECT_OK,
	COLLECT_FILE_ERROR,
	COLLECT_INVALID,
	COLLECT_EMPTY,
	COLLECT_FULL
};

/* Load a complete cell file atomically; set added to zero on failure. */
enum collect_status collector_collect(struct collection *data, int cell,
	const char *path, size_t *added);

#endif /* COLLECTOR_H */
