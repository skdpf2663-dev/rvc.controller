/* SD module GenerateTick (SA Process 2.1). The tick interval is TBD. */
#ifndef GENERATE_TICK_H
#define GENERATE_TICK_H

#ifdef __cplusplus
extern "C" {
#endif

void GenerateTick_Reset(void);
/* Produces one internal Tick. */
void GenerateTick(void);
/* Number of ticks produced since the last reset. */
unsigned long GenerateTick_Count(void);

#ifdef __cplusplus
}
#endif

#endif /* GENERATE_TICK_H */
