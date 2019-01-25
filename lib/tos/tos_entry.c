#include <common.h>

void secure_sp_entry(unsigned long entry)
{
	tos_entry(entry);

	printf("%s: secure os init done!\n",__func__);
}
