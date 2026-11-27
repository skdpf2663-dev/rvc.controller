/* GenerateTick (SA Process 2.1). The physical tick interval is TBD in the
 * requirements, so a call stands for one elapsed tick. */
#include "generate_tick.h"

static unsigned long tick_count = 0UL;

void GenerateTick_Reset(void)
{
    tick_count = 0UL;
}

void GenerateTick(void)
{
    tick_count++;
}

unsigned long GenerateTick_Count(void)
{
    return tick_count;
}
