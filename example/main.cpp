#include <cstdio>
#include <string.h>
#include <stdlib.h>

#include <hardware/i2c.h>

#include <pico/stdlib.h>

#include "si7021.h"


int main() {
    stdio_init_all();


    //
    // Initialize i2c.
    //

    i2c_inst_t * i2c;

    const uint sda_gpio = 16;  // pin 21
    const uint scl_gpio = 17;  // pin 22

    i2c = i2c0;
    i2c_init(i2c, 400*1000);  // run i2c at 400 kHz

    // Initialize I2C pins
    gpio_set_function(sda_gpio, GPIO_FUNC_I2C);
    gpio_set_function(scl_gpio, GPIO_FUNC_I2C);

    gpio_pull_up(sda_gpio);
    gpio_pull_up(scl_gpio);

    for (;;) {
        int r;
        static int counter = 0;
        float percent_rh;
        float temp_C, temp_F;

        r = si7021_read_humidity_and_temperature(i2c, &percent_rh, &temp_C);
        if (r != PICO_OK) {
            printf("failed to read temp & humidity\n");
        } else {
            temp_F = (temp_C * 9.0 / 5.0) + 32.0;
            printf(
                "% 6d: %.1f %%RH, %.1f °C (%.1f °F)\n",
                counter,
                percent_rh,
                temp_C,
                temp_F
            );
        }

        ++counter;

        sleep_ms(1000);
    }
}
