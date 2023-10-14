#include <cstdio>

#include "si7021.h"


#define SI7021_ADDRESS     0x40

#define SI7021_MEASRH_HOLD_CMD     0xE5 /* Measure Relative Humidity, Hold Master Mode */
#define SI7021_MEASRH_NOHOLD_CMD   0xF5 /* Measure Relative Humidity, No Hold Master Mode */
#define SI7021_MEASTEMP_HOLD_CMD   0xE3 /* Measure Temperature, Hold Master Mode */
#define SI7021_MEASTEMP_NOHOLD_CMD 0xF3 /* Measure Temperature, No Hold Master Mode */
#define SI7021_READPREVTEMP_CMD    0xE0 /* Read Temperature Value from Previous RH Measurement */
#define SI7021_RESET_CMD           0xFE /* Reset Command */
#define SI7021_WRITERHT_REG_CMD    0xE6 /* Write RH/T User Register 1 */
#define SI7021_READRHT_REG_CMD     0xE7 /* Read RH/T User Register 1 */
#define SI7021_WRITEHEATER_REG_CMD 0x51 /* Write Heater Control Register */
#define SI7021_READHEATER_REG_CMD  0x11 /* Read Heater Control Register */
#define SI7021_REG_HTRE_BIT        0x02 /* Control Register Heater Bit */

#define SI7021_ID1_CMD             { 0xFA, 0x0F } /* Read Electronic ID 1st Byte */
#define SI7021_ID2_CMD             { 0xFC, 0xC9 } /* Read Electronic ID 2nd Byte */
#define SI7021_FIRMVERS_CMD        { 0x84, 0xB8 } /* Read Firmware Revision */

#define SI7021_REV_1 0xff /* Sensor revision 1 */
#define SI7021_REV_2 0x20 /* Sensor revision 2 */


static uint const i2c_timeout_us = 1000*1000;


int si7021_reset(i2c_inst_t * i2c) {
    int r;
    uint8_t const out_data[] = { SI7021_RESET_CMD };
    r = i2c_write_timeout_us(i2c, SI7021_ADDRESS, out_data, sizeof(out_data), false, i2c_timeout_us);
    if (r != sizeof(out_data)) {
        return PICO_ERROR_GENERIC;
    }
    sleep_ms(20);
    return PICO_OK;
}


int si7021_read_sna(i2c_inst_t * i2c, uint32_t * sna) {
    uint8_t const out_data[] = SI7021_ID1_CMD;
    uint8_t in_data[8];
    int r;

    r = i2c_write_timeout_us(i2c, SI7021_ADDRESS, out_data, sizeof(out_data), false, i2c_timeout_us);
    if (r != sizeof(out_data)) {
        return PICO_ERROR_GENERIC;
    }
#ifdef SI7021_DEBUG
    printf("wrote 0x%02x 0x%02x\n", out_data[0], out_data[1]);
#endif

    r = i2c_read_timeout_us(i2c, SI7021_ADDRESS, in_data, sizeof(in_data), false, i2c_timeout_us);
    if (r != sizeof(in_data)) {
        return PICO_ERROR_GENERIC;
    }
#ifdef SI7021_DEBUG
    for (size_t i = 0; i < sizeof(in_data); ++i) {
        printf("    0x%02x\n", in_data[i]);
    }
#endif
    *sna = (in_data[0] << 24) | (in_data[2] << 16) | (in_data[4] << 8) | (in_data[6]);
    return PICO_OK;
}


int si7021_read_snb(i2c_inst_t * i2c, uint32_t * snb) {
    uint8_t const out_data[] = SI7021_ID2_CMD;
    uint8_t in_data[8];
    int r;

    r = i2c_write_timeout_us(i2c, SI7021_ADDRESS, out_data, sizeof(out_data), false, i2c_timeout_us);
    if (r != sizeof(out_data)) {
        return PICO_ERROR_GENERIC;
    }
#ifdef SI7021_DEBUG
    printf("wrote 0x%02x 0x%02x\n", out_data[0], out_data[1]);
#endif

    r = i2c_read_timeout_us(i2c, SI7021_ADDRESS, in_data, sizeof(in_data), false, i2c_timeout_us);
    if (r != sizeof(in_data)) {
        return PICO_ERROR_GENERIC;
    }
#ifdef SI7021_DEBUG
    for (size_t i = 0; i < sizeof(in_data); ++i) {
        printf("    0x%02x\n", in_data[i]);
    }
#endif
    *snb = (in_data[0] << 24) | (in_data[2] << 16) | (in_data[4] << 8) | (in_data[6]);
    return PICO_OK;
}


int si7021_read_firmware_revision(i2c_inst_t * i2c, uint8_t * fw_rev) {
    uint8_t const out_data[] = SI7021_FIRMVERS_CMD;
    uint8_t in_data[1];
    int r;

    r = i2c_write_timeout_us(i2c, SI7021_ADDRESS, out_data, sizeof(out_data), false, i2c_timeout_us);
    if (r != sizeof(out_data)) {
        return PICO_ERROR_GENERIC;
    }
#ifdef SI7021_DEBUG
    printf("wrote 0x%02x 0x%02x\n", out_data[0], out_data[1]);
#endif

    r = i2c_read_timeout_us(i2c, SI7021_ADDRESS, in_data, sizeof(in_data), false, i2c_timeout_us);
    if (r != sizeof(in_data)) {
        return PICO_ERROR_GENERIC;
    }
#ifdef SI7021_DEBUG
    for (size_t i = 0; i < sizeof(in_data); ++i) {
        printf("    0x%02x\n", in_data[i]);
    }
#endif
    *fw_rev = in_data[0];
    return PICO_OK;
}


int si7021_read_humidity_and_temperature(i2c_inst_t * i2c, float * percent_rh, float * temp_c) {
    uint8_t const measure_rh_cmd[] = { SI7021_MEASRH_NOHOLD_CMD };
    uint8_t const read_temp_cmd[] = { SI7021_READPREVTEMP_CMD };
    uint8_t in_data[2];
    int r;

    // Initiate a temperature and a %RH measurement.
    r = i2c_write_timeout_us(i2c, SI7021_ADDRESS, measure_rh_cmd, sizeof(measure_rh_cmd), false, i2c_timeout_us);
    if (r != sizeof(measure_rh_cmd)) {
        return PICO_ERROR_GENERIC;
    }
#ifdef SI7021_DEBUG
    printf("wrote 0x%02x\n", measure_rh_cmd[0]);
#endif

    // 11 ms for the temperature measurement.
    // 12 ms for the %RH measurement.
    // 5 ms for a fudge factor.
    sleep_ms(11 + 12 + 5);

    // Read RH
    r = i2c_read_timeout_us(i2c, SI7021_ADDRESS, in_data, sizeof(in_data), false, i2c_timeout_us);
    if (r != sizeof(in_data)) {
        return PICO_ERROR_GENERIC;
    }
#ifdef SI7021_DEBUG
    for (size_t i = 0; i < sizeof(in_data); ++i) {
        printf("    0x%02x\n", in_data[i]);
    }
#endif

    uint16_t rh_code = (in_data[0] << 8) | (in_data[1]);
    *percent_rh = ((125 * rh_code) / 65536) - 6;
    if (*percent_rh < 0) {
        *percent_rh = 0;
    } else if (*percent_rh > 100) {
        *percent_rh = 100;
    }

    // read temperature cmd
    r = i2c_write_timeout_us(i2c, SI7021_ADDRESS, read_temp_cmd, sizeof(read_temp_cmd), false, i2c_timeout_us);
    if (r != sizeof(read_temp_cmd)) {
        return PICO_ERROR_GENERIC;
    }
#ifdef SI7021_DEBUG
    printf("wrote 0x%02x\n", read_temp_cmd[0]);
#endif

    // Read RH
    r = i2c_read_timeout_us(i2c, SI7021_ADDRESS, in_data, sizeof(in_data), false, i2c_timeout_us);
    if (r != sizeof(in_data)) {
        return PICO_ERROR_GENERIC;
    }
#ifdef SI7021_DEBUG
    for (size_t i = 0; i < sizeof(in_data); ++i) {
        printf("    0x%02x\n", in_data[i]);
    }
#endif

    uint16_t temp_code = (in_data[0] << 8) | (in_data[1]);
    *temp_c = ((175.72 * temp_code) / 65536) - 46.85;

    return PICO_OK;
}
