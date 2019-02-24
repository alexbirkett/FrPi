#ifndef FRPI_JOYSTICK_H
#define FRPI_JOYSTICK_H

#define JOYSTICK_CHANNEL_COUNT 16

typedef struct channel_spec {
    char* device_name;
    uint8_t type;	/* event type */
    uint8_t number;	/* axis/button number */
    uint8_t reverse;
} channel_spec;

void init_joystick(channel_spec channel_spec[], size_t size);
int16_t *update_channels();

#endif //FRPI_JOYSTICK_H
