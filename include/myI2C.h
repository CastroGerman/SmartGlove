#ifndef MYI2C_H_
#define MYI2C_H_

#include <driver/i2c.h>
#include <esp_log.h>

#define PIN_SDA 18
#define PIN_CLK 16
#define MASTER_CLK_SPEED 100000 //[Hz]

#define ACK     (0)
#define NACK    (1)
#undef ESP_ERROR_CHECK
#define ESP_ERROR_CHECK(x)   do { esp_err_t rc = (x); if (rc != ESP_OK) { ESP_LOGE("err", "esp_err_t = %d", rc); assert(0 && #x);} } while(0);

void InitI2C (void);

#endif /* MYI2C_H_ */