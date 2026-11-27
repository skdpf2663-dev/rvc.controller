/* C main(): host adapter for the RVC controller.
 * Hardware is out of scope (PR-6), so sensor samples are read from stdin and
 * commands are written to stdout, one line per tick. This lets system tests and
 * a simulator drive the real C controller.
 *
 * Input: one line per tick with four tokens for Front Left Right Dust.
 *        Each token is 1 (detected), 0 (not detected), or - (no new sample).
 *        Empty lines and lines starting with '#' are ignored.
 * Output: "tick=N state=S dir=D clean=C turn=T cleanup=U dust_turn=B sensors=FLRD"
 * Usage:  rvc [max_ticks]   (default: no limit)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "controller.h"
#include "generate_tick.h"
#include "rvc_main.h"
#include "sensor_store.h"

static const char *state_name(ControllerState s)
{
    static const char *const names[] = {"Forward", "CleanUpForward", "CleanUpStopped", "TurnLeft",
                                        "TurnRight", "Backward", "CleanUpBackwardPause"};
    return names[s];
}

static const char *direction_name(Direction d)
{
    static const char *const names[] = {"forward", "backward", "turn_left", "turn_right", "stop"};
    return names[d];
}

static const char *clean_name(CleanMode c)
{
    static const char *const names[] = {"clean_off", "clean_on", "clean_up"};
    return names[c];
}

static void ignore_direction(Direction d)
{
    (void)d;
}

/* CommandCleanerMode is the last output of every tick, so the line is printed here. */
static void print_tick(CleanMode c)
{
    const ControllerStatus s = Controller_GetStatus();
    const SensorValues v = ReadLatestSensorValues();
    printf("tick=%lu state=%s dir=%s clean=%s turn=%d cleanup=%d dust_turn=%d sensors=%d%d%d%d\n",
           GenerateTick_Count(), state_name(s.state), direction_name(s.direction_command),
           clean_name(c), s.turn_count, s.cleanup_count, s.dust_during_turn ? 1 : 0,
           v.front ? 1 : 0, v.left ? 1 : 0, v.right ? 1 : 0, v.dust ? 1 : 0);
}

/* Parses one token. Returns 0 on success, -1 on an invalid token. */
static int parse_token(const char *tok, bool *arrived, bool *value)
{
    if (tok == NULL || tok[1] != '\0') {
        return -1;
    }
    switch (tok[0]) {
    case '1':
        *arrived = true;
        *value = true;
        return 0;
    case '0':
        *arrived = true;
        *value = false;
        return 0;
    case '-':
        *arrived = false;
        *value = false;
        return 0;
    default:
        return -1;
    }
}

typedef struct {
    FILE *in;
    int error;
} StdinSource;

static bool read_samples(SensorSamples *out, void *context)
{
    StdinSource *src = (StdinSource *)context;
    char line[256];

    while (fgets(line, sizeof line, src->in) != NULL) {
        char *tok[5];
        char *p = line;
        if (line[0] == '#' || strspn(line, " \t\r\n") == strlen(line)) {
            continue;
        }
        for (int i = 0; i < 5; i++) {
            tok[i] = strtok(p, " \t\r\n");
            p = NULL;
        }
        if (tok[4] != NULL || parse_token(tok[0], &out->front_arrived, &out->front) != 0 ||
            parse_token(tok[1], &out->left_arrived, &out->left) != 0 ||
            parse_token(tok[2], &out->right_arrived, &out->right) != 0 ||
            parse_token(tok[3], &out->dust_arrived, &out->dust) != 0) {
            fprintf(stderr, "invalid input line: expected 4 tokens of 0, 1 or -\n");
            src->error = 1;
            return false;
        }
        return true;
    }
    return false;
}

int main(int argc, char **argv)
{
    unsigned long max_ticks = (unsigned long)-1;
    const ActuatorPort port = {ignore_direction, print_tick};
    StdinSource src = {stdin, 0};

    if (argc > 1) {
        char *end = NULL;
        max_ticks = strtoul(argv[1], &end, 10);
        if (end == argv[1] || *end != '\0') {
            fprintf(stderr, "usage: %s [max_ticks]\n", argv[0]);
            return 2;
        }
    }
    (void)RvcMain_Run(&port, read_samples, &src, max_ticks);
    return src.error ? 2 : 0;
}
