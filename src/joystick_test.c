
#include <stdint.h>
#include <printf.h>
#include <linux/joystick.h>
#include "joystick.h"

int main()
{

    struct channel_spec channel_specs[] =
        {{"/dev/input/js0", JS_EVENT_AXIS, 0, 0},
         {"/dev/input/js0", JS_EVENT_AXIS, 1, 0},
         {"/dev/input/js2", JS_EVENT_AXIS, 2, 0},
         {"/dev/input/js1", JS_EVENT_AXIS, 3, 0},
         {"/dev/input/js1", JS_EVENT_AXIS, 4, 0},
         };
    init_joystick(channel_specs, 5);
    while (1) {
        int16_t *my_channels = update_channels();
        for (int i = 0; i < JOYSTICK_CHANNEL_COUNT; i++) {
            printf("%d:%d ", i, my_channels[i]);
        }
        printf("\n");
    }

    return 0;
}