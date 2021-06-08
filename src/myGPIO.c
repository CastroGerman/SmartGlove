#include "myGPIO.h"
#include "myBLE.h"
#include "myTasks.h"
#include <string.h>
#include "configs.h"
#include "MadgwickAHRS.h"
#include "MPU6050.h"

static esp_adc_cal_characteristics_t *adc_chars;

void setBitInByte (uint8_t *_byte, uint8_t _bit, uint8_t _status)
{
    *_byte &= ~(1 << _bit);
    *_byte |= (_status << _bit);
}

uint8_t getBitInByte (uint8_t *_byte, uint8_t _bit)
{
    return ((*_byte >> _bit) & 1);
}

void toggleBitInByte (uint8_t *_byte, uint8_t _bit)
{
    if (getBitInByte(_byte, _bit) == 1)
    {
        setBitInByte(_byte, _bit, 0);
    }
    else
    {
        setBitInByte(_byte, _bit, 1);
    }
}

static void check_efuse()
{
    //Check TP is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
        printf("eFuse Two Point: Supported\n");
    } else {
        printf("eFuse Two Point: NOT supported\n");
    }

    //Check Vref is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
        printf("eFuse Vref: Supported\n");
    } else {
        printf("eFuse Vref: NOT supported\n");
    }
}

static void print_char_val_type(esp_adc_cal_value_t val_type)
{
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        printf("Characterized using Two Point Value\n");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        printf("Characterized using eFuse Vref\n");
    } else {
        printf("Characterized using Default Vref\n");
    }
}

void InitADC1 (void)
{
    //Check if Two Point or Vref are burned into eFuse
    check_efuse();

    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(THUMB_FLEX_CHANNEL, THUMB_FLEX_CHANNEL_ATT);
    adc1_config_channel_atten(INDEX_FLEX_CHANNEL, INDEX_FLEX_CHANNEL_ATT);
    adc1_config_channel_atten(MIDDLE_FLEX_CHANNEL, MIDDLE_FLEX_CHANNEL_ATT);
    adc1_config_channel_atten(RING_FLEX_CHANNEL, RING_FLEX_CHANNEL_ATT);
    adc1_config_channel_atten(LITTLE_FLEX_CHANNEL, LITTLE_FLEX_CHANNEL_ATT);
    adc1_config_channel_atten(BATT_CHANNEL, BATT_CHANNEL_ATT);
    
    //Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_0, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
    print_char_val_type(val_type);
}

void InitGPIO (void)
{
    gpio_config_t io_config;

    //Setting RGB LED on board
    //disable interrupt
    io_config.intr_type = GPIO_PIN_INTR_DISABLE;
    //set as input/output mode
    io_config.mode = GPIO_MODE_INPUT_OUTPUT;
    //bit mask of the pins you want to set
    io_config.pin_bit_mask = GPIO_SEL_27;
    //disable pull-down mode
    io_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    //disable pull-up mode
    io_config.pull_up_en = GPIO_PULLUP_ENABLE;
    //configure GPIO with the given settings
    gpio_config(&io_config);

    io_config.intr_type = GPIO_PIN_INTR_DISABLE;
    io_config.mode = GPIO_MODE_INPUT_OUTPUT;
    io_config.pin_bit_mask = GPIO_SEL_26;
    io_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_config.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_config);

    io_config.intr_type = GPIO_PIN_INTR_DISABLE;
    io_config.mode = GPIO_MODE_INPUT_OUTPUT;
    io_config.pin_bit_mask = GPIO_SEL_12;
    io_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_config.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_config);

    //Setting LED on board
    io_config.intr_type = GPIO_PIN_INTR_DISABLE;
    io_config.mode = GPIO_MODE_INPUT_OUTPUT;
    io_config.pin_bit_mask = GPIO_SEL_2;
    io_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_config.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_config);

    //Setting glove button
    io_config.intr_type = GPIO_PIN_INTR_POSEDGE;
    io_config.pin_bit_mask = GPIO_SEL_25;
    io_config.mode = GPIO_MODE_INPUT;
    io_config.pull_up_en = GPIO_PULLUP_ENABLE;
    io_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config(&io_config);
    //change gpio intrrupt type for one pin
    //gpio_set_intr_type(GPIO_SEL_25, GPIO_INTR_ANYEDGE);
    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_LOWMED);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(BUTTON_PIN, glove_button_isr_handler, (void*) NULL);
}

int getBatteryLevel (void)
{
    adc1_channel_t _channel = BATT_CHANNEL;
    return getADC1Channel(_channel);
}

int getFingerFlexChannel (adc1_channel_t _channel)
{
    return getADC1Channel(_channel);
}

int getADC1Channel (adc1_channel_t _channel)
{
    int adcRead = 0;
    //Multisampling
    for (int i = 0; i < NUM_OF_SAMPLES; i++) {
        adcRead += adc1_get_raw(_channel);
    }
    adcRead /= NUM_OF_SAMPLES;

    #ifdef ENABLE_THEMU_ADC_LOGS
    uint32_t volts;
    esp_adc_cal_get_voltage(_channel,adc_chars,&volts);
    printf("ADC_CH%d: \tRaw: %d \tVolts: %d ", _channel, adcRead, volts);//sizeof(adcRead));
    #endif
    
    adcRead = (adcRead-ADC_CAL_MIN)*100/(ADC_CAL_MAX-ADC_CAL_MIN); 
    
    #ifdef ENABLE_THEMU_ADC_LOGS
    printf("\tPorcentual: %d\n",adcRead);
    #endif
    
    return adcRead;
}

/**
 * ISR triggers twice on a single event. Problem related to:
 * https://github.com/espressif/arduino-esp32/issues/1111
 */
void IRAM_ATTR glove_button_isr_handler (void *pv)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xTaskNotifyFromISR(thGPIO, 1, eSetValueWithoutOverwrite, &xHigherPriorityTaskWoken);
    if(xHigherPriorityTaskWoken != pdFALSE){}
}

void tGPIO (void *pv)
{
    gpio_set_level(FB_LED_PIN, LED_ON);
    uint32_t notifycount = 0;
    while (1)
    {
        notifycount = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);//pdTRUE = as a binary semaphore. pdFALSE = as a counting semaphore.
        if(notifycount == 1)
        {
            /*Remember that u can't read OUTPUTS, only INPUTS.
            * Or set the GPIO mode to GPIO_MODE_INPUT_OUTPUT.*/
            if(gpio_get_level(FB_LED_PIN))
            {
                gpio_set_level(FB_LED_PIN, LED_OFF);
                #ifdef ENABLE_THEMU_BLE
                xTaskNotify(thBLE, 3, eSetValueWithOverwrite);
                #endif
            }
            else
            {
                #ifdef ENABLE_THEMU_IMU
                xTaskNotify(thMPU6050, 2, eSetValueWithOverwrite);
                #endif
                q0 = 0.27f;
                q1 = 0.65f;
                q2 = -0.27f;
                q3 = 0.65f;
                gpio_set_level(FB_LED_PIN, LED_ON);
                #ifdef ENABLE_THEMU_BLE
                xTaskNotify(thBLE, 4, eSetValueWithOverwrite);
                #endif
            }    
        }
        else
        {
            printf("TIMEOUT waiting notification on tGPIO\n");
        }
    }
}