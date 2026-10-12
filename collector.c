#include "collector.h"

/* Foundation stub for the shared collection interface. */
enum collect_status collector_collect(struct collection *data, int cell,
	const char *path, size_t *added)
{
	(void)data;
	(void)cell;
	(void)path;
	*added = 0;
	return COLLECT_FILE_ERROR;
}
