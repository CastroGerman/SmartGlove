/**Brief:
 * GPIO 16 defined as input.
 * GPIO 17 defined as output.
 * GPIO 36 defines as channel 0 of ADC1.
 * GPIO 39 defines as channel 3 of ADC1.
 * GPIO 32 defines as channel 4 of ADC1.
 * GPIO 33 defines as channel 5 of ADC1.
 * GPIO 34 defined as channel 6 of ADC1.
 * GPIO 35 defines as channel 7 of ADC1.
 */
#include "myGPIO.h"
#include "myBLE.h"
#include "myTasks.h"
#include <string.h>
#include "configs.h"
static esp_adc_cal_characteristics_t *adc_chars;

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

    /**Configure ADC
     * Due to ADC characteristics, most accurate results are obtained within the following approximate voltage ranges:
     * 
     * - 0dB attenuaton (ADC_ATTEN_DB_0) between 100 and 950mV
     * - 2.5dB attenuation (ADC_ATTEN_DB_2_5) between 100 and 1250mV
     * - 6dB attenuation (ADC_ATTEN_DB_6) between 150 to 1750mV
     * - 11dB attenuation (ADC_ATTEN_DB_11) between 150 to 2450mV
     * 
     * For maximum accuracy, use the ADC calibration APIs and measure voltages within these recommended ranges.
     */
    adc1_config_width(ADC_WIDTH_BIT_10);
    adc1_config_channel_atten(ADC_CHANNEL_0, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(ADC_CHANNEL_3, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(ADC_CHANNEL_4, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(ADC_CHANNEL_5, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(ADC_CHANNEL_6, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(ADC_CHANNEL_7, ADC_ATTEN_DB_11);
    
    //Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
    print_char_val_type(val_type);
}

void InitGPIO (void)
{
    gpio_config_t io_config;

    //disable interrupt
    io_config.intr_type = GPIO_PIN_INTR_DISABLE;
    //set as output mode
    io_config.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_config.pin_bit_mask = GPIO_SEL_17;
    //disable pull-down mode
    io_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    //disable pull-up mode
    io_config.pull_up_en = GPIO_PULLUP_DISABLE;
    //configure GPIO with the given settings
    gpio_config(&io_config);

    //interrupt of falling edge
    io_config.intr_type = GPIO_PIN_INTR_NEGEDGE;
    //bit mask of the pins
    io_config.pin_bit_mask = GPIO_SEL_16;
    //set as input mode
    io_config.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_config.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_config);
    //change gpio intrrupt type for one pin
    //gpio_set_intr_type(GPIO_SEL_16, GPIO_INTR_ANYEDGE);
    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_EDGE);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_NUM_16, gpio16_isr_handler, (void*) NULL);
    
    //Setting LED on board
    io_config.intr_type = GPIO_PIN_INTR_DISABLE;
    io_config.mode = GPIO_MODE_INPUT_OUTPUT;
    io_config.pin_bit_mask = GPIO_SEL_2;
    io_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_config.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_config);
}

int readPorcentualADC1Channel(adc1_channel_t _channel)
{
    int adcRead = 0;
    //Multisampling
    for (int i = 0; i < NO_OF_SAMPLES; i++) {
        adcRead += adc1_get_raw(_channel);
    }
    adcRead /= NO_OF_SAMPLES;

    #ifdef ENABLE_THEMU_ADC_LOGS
    uint32_t volts;
    esp_adc_cal_get_voltage(_channel,adc_chars,&volts);
    printf("ADC Raw: %d\tLength: %d\n",adcRead,volts);//sizeof(adcRead));
    #endif
    
    adcRead = (adcRead-ADC_CAL_MIN)*100/(ADC_CAL_MAX-ADC_CAL_MIN); 
    
    #ifdef ENABLE_THEMU_ADC_LOGS
    printf("ADC Porcentual: %d\n",adcRead);
    #endif
    
    return adcRead;
}

void IRAM_ATTR gpio16_isr_handler (void *pv)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xTaskNotifyFromISR(thGPIO, 2, eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
    if(xHigherPriorityTaskWoken != pdFALSE){}
}

void tGPIO (void *pv)
{
    uint32_t notifycount = 0;
    while (1)
    {
        notifycount = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);//pdTRUE = as a binary semaphore. pdFALSE = as a counting semaphore.
        if(notifycount == 1)
        {
            
            //printf("Interrumpio notify 1 GPIO 2 \n"); 
            /*Remember that u can't read OUTPUTS, only INPUTS.
            * Or set the GPIO mode to GPIO_MODE_INPUT_OUTPUT.*/
            if(gpio_get_level(GPIO_NUM_2))
            {
                gpio_set_level(GPIO_NUM_2, 0);
            }
            else
            {
                gpio_set_level(GPIO_NUM_2, 1);
            }          
        }
        else if (notifycount == 2)
        {
            readPorcentualADC1Channel(ADC1_CHANNEL_3);
        }
        else
        {
            printf("TIMEOUT waiting notification on tGPIO\n");
        }
    }
}