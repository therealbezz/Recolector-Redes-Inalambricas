/* Author: Daniel Mihailov
 * Team: Daniel Mihailov, Diego Medina, Alba Serrano
 */

#ifndef COLLECTOR_H
#define COLLECTOR_H

#include <stddef.h>

/* Initial capacity and growth increment, in access points. */
#define COLLECTION_STEP 5
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

/* Dynamic storage of observations in collection order. */
struct collection
{
	struct access_point *points;
	size_t count;
	size_t capacity;
};

/* Collection outcomes; failures leave saved observations unchanged. */
enum collect_status
{
	COLLECT_OK,
	COLLECT_FILE_ERROR,
	COLLECT_INVALID,
	COLLECT_EMPTY,
	COLLECT_NO_MEMORY
};

/* Result of a collection attempt. The count is zero on failure. */
struct collect_result
{
	enum collect_status status;
	size_t added;
};

/* Observations retained for the current application session. */
extern struct collection collected;

/* Allocate the initial storage. Return 1 on success or 0 on failure. */
int collector_init(void);
/* Release storage and reset the collection. */
void collector_destroy(void);

/* Load the file at path for cell and return the collection result. */
struct collect_result collector_collect(int cell, const char path[]);

#endif /* COLLECTOR_H */
