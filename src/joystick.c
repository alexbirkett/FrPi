#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/joystick.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>

#include "joystick.h"

#define MAX_JOYSTICK_DEVICES JOYSTICK_CHANNEL_COUNT

int spec_count = 0;

int open_joystick_count = 0;

int next_joystick_index = 0;

typedef struct joystick_device
{
    int file_descriptor;
    char *name;
} joystick_device;

joystick_device joystick_devices[MAX_JOYSTICK_DEVICES];

int16_t channels_values[JOYSTICK_CHANNEL_COUNT];

channel_spec (*channel_specs)[];

int open_joystick(char *device_name)
{
    int fd = -1;

    if (device_name == NULL) {
        return fd;
    }

    fd = open(device_name, O_RDONLY | O_NONBLOCK);

    if (fd < 0) {
        printf("Could not locate joystick device %s\n", device_name);
        exit(1);
    }

    return fd;
}

uint8_t is_joystick_already_open(char *device_name)
{
    for (int i = 0; i < open_joystick_count; i++) {
        if (strcmp(joystick_devices[i].name, device_name) == 0) {
            return 1;
        }
    }
    return 0;
}

void print_device_info(int fd)
{
    int axes = 0, buttons = 0;
    char name[128];

    ioctl(fd, JSIOCGAXES, &axes);
    ioctl(fd, JSIOCGBUTTONS, &buttons);
    ioctl(fd, JSIOCGNAME(sizeof(name)), &name);

    printf("%s\n  %d Axes %d Buttons\n", name, axes, buttons);
}

void open_joystick_if_not_open(char *device_name)
{
    if (!is_joystick_already_open(device_name)) {
        int fd = open_joystick(device_name);
        joystick_device device = {fd, device_name};
        joystick_devices[open_joystick_count++] = device;
        print_device_info(fd);
    }
}

void init_joystick(channel_spec channel_spec[], size_t size)
{

    spec_count = size;
    channel_specs = channel_spec;

    for (int i = 0; i < size; i++) {
        open_joystick_if_not_open((*channel_specs)[i].device_name);
    }
}

int channel_index(int start_postion, char *device_name, uint8_t type, uint8_t number)
{
    for (int i = start_postion; i < spec_count; i++) {
        channel_spec *spec = &(*channel_specs)[i];
        if (type == spec->type && number == spec->number && strcmp(device_name, spec->device_name) == 0) {
            return i;
        }
    }
    return -1;
}

int16_t *update_channels()
{
    struct js_event jse;

    next_joystick_index = (next_joystick_index + 1) % open_joystick_count;

    joystick_device *device = &joystick_devices[next_joystick_index];

    if (read(device->file_descriptor, &jse, sizeof(jse)) > 0) {

        int index = 0;

        while ((index = channel_index(index, device->name, jse.type, jse.number)) >= 0) {
            channel_spec *spec = &(*channel_specs)[index];
            channels_values[index++] =  spec->reverse ? jse.value / -1 : jse.value;
        }
    }
    return channels_values;
}

