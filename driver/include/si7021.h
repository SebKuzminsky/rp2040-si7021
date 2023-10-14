#include <hardware/i2c.h>

int si7021_reset(i2c_inst_t * i2c);
int si7021_read_sna(i2c_inst_t * i2c, uint32_t * sna);
int si7021_read_snb(i2c_inst_t * i2c, uint32_t * snb);
int si7021_read_firmware_revision(i2c_inst_t * i2c, uint8_t * fw_rev);
int si7021_read_humidity_and_temperature(i2c_inst_t * i2c, float * percent_rh, float * temp_c);
