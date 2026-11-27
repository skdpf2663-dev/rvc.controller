/* WebAssembly bridge for the web simulator.
 * Links the unchanged production modules in src/ (everything except main.c)
 * and exposes the simulator execution interface:
 *   initialize/reset -> rvc_reset()   (RvcMain_Start: Controller_Init etc.)
 *   step(sensors)    -> rvc_step(...)  (RvcMain_Cycle: one complete tick)
 *   get_state()      -> rvc_get_*()    (read-only accessors)
 * No controller decision is made here or in JavaScript. */
#include "controller.h"
#include "generate_tick.h"
#include "rvc_main.h"
#include "sensor_store.h"

#define EXPORT(name) __attribute__((export_name(#name)))

/* Freestanding build (no libc): the compiler may emit these for struct copies. */
void *memcpy(void *dst, const void *src, unsigned long n);
void *memset(void *dst, int c, unsigned long n);
void *memcpy(void *dst, const void *src, unsigned long n)
{
    unsigned char *d = dst;
    const unsigned char *s = src;
    while (n--) {
        *d++ = *s++;
    }
    return dst;
}
void *memset(void *dst, int c, unsigned long n)
{
    unsigned char *d = dst;
    while (n--) {
        *d++ = (unsigned char)c;
    }
    return dst;
}

/* What actually reached the motor and the cleaner through the output port. */
static int motor_output = -1;
static int cleaner_output = -1;

static void to_motor(Direction d)
{
    motor_output = (int)d;
}

static void to_cleaner(CleanMode c)
{
    cleaner_output = (int)c;
}

static const ActuatorPort port = {to_motor, to_cleaner};

EXPORT(rvc_reset) void rvc_reset(void)
{
    motor_output = -1;
    cleaner_output = -1;
    RvcMain_Start(&port);
}

EXPORT(rvc_step) void rvc_step(int front_arrived, int front, int left_arrived, int left,
                               int right_arrived, int right, int dust_arrived, int dust)
{
    SensorSamples s;
    s.front_arrived = front_arrived != 0;
    s.front = front != 0;
    s.left_arrived = left_arrived != 0;
    s.left = left != 0;
    s.right_arrived = right_arrived != 0;
    s.right = right != 0;
    s.dust_arrived = dust_arrived != 0;
    s.dust = dust != 0;
    RvcMain_Cycle(&s);
}

EXPORT(rvc_get_tick) unsigned long rvc_get_tick(void) { return GenerateTick_Count(); }
EXPORT(rvc_get_state) int rvc_get_state(void) { return (int)Controller_GetStatus().state; }
EXPORT(rvc_get_turn_count) int rvc_get_turn_count(void) { return Controller_GetStatus().turn_count; }
EXPORT(rvc_get_cleanup_count) int rvc_get_cleanup_count(void) { return Controller_GetStatus().cleanup_count; }
EXPORT(rvc_get_dust_during_turn) int rvc_get_dust_during_turn(void) { return Controller_GetStatus().dust_during_turn ? 1 : 0; }
EXPORT(rvc_get_direction_command) int rvc_get_direction_command(void) { return (int)Controller_GetStatus().direction_command; }
EXPORT(rvc_get_clean_command) int rvc_get_clean_command(void) { return (int)Controller_GetStatus().clean_command; }
EXPORT(rvc_get_motor_output) int rvc_get_motor_output(void) { return motor_output; }
EXPORT(rvc_get_cleaner_output) int rvc_get_cleaner_output(void) { return cleaner_output; }

/* Latest stored sensor values (D1): bit 3 front, 2 left, 1 right, 0 dust. */
EXPORT(rvc_get_sensors) int rvc_get_sensors(void)
{
    const SensorValues v = ReadLatestSensorValues();
    return (v.front ? 8 : 0) | (v.left ? 4 : 0) | (v.right ? 2 : 0) | (v.dust ? 1 : 0);
}
