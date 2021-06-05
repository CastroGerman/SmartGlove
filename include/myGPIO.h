#ifndef MYGPIO_H_
#define MYGPIO_H_

#include <driver/gpio.h>
#include <driver/adc.h>
#include <esp_adc_cal.h>

#define DEFAULT_VREF    1100        //[mV]. Use adc2_vref_to_gpio() to obtain a better estimate.
#define NUM_OF_SAMPLES   64         //Multisampling.
#define ADC_CAL_MAX     (int)4100   //To be defined in the calibration process into a struct.
#define ADC_CAL_MIN     (int)2600
#define FB_LED_PIN      GPIO_NUM_2

#define LED_ON  1
#define LED_OFF 0

#define FB_LED_RED_PIN      GPIO_NUM_27
#define FB_LED_GREEN_PIN    GPIO_NUM_12
#define FB_LED_BLUE_PIN     GPIO_NUM_26
#define BUTTON_PIN          GPIO_NUM_25

#define FB_LED_RED_PLOAD_BIT    2
#define FB_LED_GREEN_PLOAD_BIT  1
#define FB_LED_BLUE_PLOAD_BIT   0

/**Pin Reference:
 * GPIO 36 defined as channel 0 of ADC1.
 * GPIO 39 defined as channel 3 of ADC1.
 * GPIO 32 defined as channel 4 of ADC1.
 * GPIO 33 defined as channel 5 of ADC1.
 * GPIO 34 defined as channel 6 of ADC1.
 * GPIO 35 defined as channel 7 of ADC1.
 */

#define THUMB_FLEX_CHANNEL  ADC1_CHANNEL_3
#define INDEX_FLEX_CHANNEL  ADC1_CHANNEL_6
#define MIDDLE_FLEX_CHANNEL ADC1_CHANNEL_7
#define RING_FLEX_CHANNEL   ADC1_CHANNEL_4
#define LITTLE_FLEX_CHANNEL ADC1_CHANNEL_5
#define BATT_CHANNEL        ADC1_CHANNEL_0

/** Due to ADC characteristics, most accurate results are obtained within the following approximate voltage ranges:
 * 
 * - 0dB attenuaton (ADC_ATTEN_DB_0) between 100 and 950mV
 * - 2.5dB attenuation (ADC_ATTEN_DB_2_5) between 100 and 1250mV
 * - 6dB attenuation (ADC_ATTEN_DB_6) between 150 to 1750mV
 * - 11dB attenuation (ADC_ATTEN_DB_11) between 150 to 2450mV
 * 
 * For maximum accuracy, use the ADC calibration APIs and measure voltages within these recommended ranges.
 */
#define THUMB_FLEX_CHANNEL_ATT  ADC_ATTEN_DB_6
#define INDEX_FLEX_CHANNEL_ATT  ADC_ATTEN_DB_6
#define MIDDLE_FLEX_CHANNEL_ATT ADC_ATTEN_DB_6
#define RING_FLEX_CHANNEL_ATT   ADC_ATTEN_DB_6
#define LITTLE_FLEX_CHANNEL_ATT ADC_ATTEN_DB_6
#define BATT_CHANNEL_ATT        ADC_ATTEN_DB_6

#define FINGERS 5
#define THUMB   4
#define INDEX   3
#define MIDDLE  2
#define RING    1
#define LITTLE  0


void setBitInByte (uint8_t *_byte, uint8_t _bit, uint8_t _status);
uint8_t getBitInByte (uint8_t *_byte, uint8_t _bit);
void toggleBitInByte (uint8_t *_byte, uint8_t _bit);
void InitGPIO (void);
void InitADC1 (void);
int getBatteryLevel (void);
int getFingerFlexChannel(adc1_channel_t _channel);
int getADC1Channel (adc1_channel_t _channel);
void IRAM_ATTR glove_button_isr_handler (void *pv);
void tGPIO (void *pv);


#endif /* MYGPIO_H_ */