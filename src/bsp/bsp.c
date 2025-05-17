#include "gd32vf103.h"
#include "eclicw.h"
#include "drivers.h"
#include "dac.h"
#include "adc.h"
#define OFFSET 2048

static uint32_t max_irqn=0;
static uint64_t delta=0;
static uint32_t timebase=0;
static uint32_t debug=0;
static int32_t  adcr=0;

void bsp_IO_config(uint8_t dbg) {
    rcu_periph_clock_enable(RCU_GPIOA);
    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0);
    DAC0powerUpInit();                      // Initialize DAC0/PA4 toolbox
    ADC3powerUpInit(0);
    adc_software_trigger_enable(ADC0, //Trigger another ADC conversion!
                                ADC_REGULAR_CHANNEL);
    if (debug=dbg) {
        gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_1);
        t5omsi();
        colinit();
        l88init();
    }
}

int32_t bsp_IO_load(){
    return adcr;
}

void bsp_IO_store(int32_t data){
    //data ? gpio_bit_set(GPIOA, GPIO_PIN_0) : gpio_bit_reset(GPIOA, GPIO_PIN_0);
    DAC0set(data-OFFSET);
}

void bsp_timebase_preset(void (*pISR)(void), uint32_t freq){
    timebase = 27000000/freq;                   // mtime runs at 27MHz
    if (debug) {
        l88mem(4, (timebase>>24) & 0xFF);
        l88mem(5, (timebase>>16) & 0xFF);
        l88mem(6, (timebase>> 8) & 0xFF);
        l88mem(7, (timebase>> 0) & 0xFF);
    }                 
    *( volatile uint64_t * )( TIMER_CTRL_ADDR + TIMER_MTIME ) = 0;
    *( volatile uint64_t * )( TIMER_CTRL_ADDR + TIMER_MTIMECMP ) = timebase;
    eclicw_enable(CLIC_INT_TMR, 1, 1, pISR);    // Enabel machine time timer int!

    eclic_global_interrupt_enable();            // !!! INTERRUPT ENABLED !!!
}

void bsp_timebase_reload(){
    if (debug) {
        gpio_bit_set(GPIOA, GPIO_PIN_1);
        delta = *( volatile uint64_t * )( TIMER_CTRL_ADDR + TIMER_MTIME );
    }
    *( volatile uint64_t * )( TIMER_CTRL_ADDR + TIMER_MTIMECMP )+=timebase;
    adcr = adc_regular_data_read(ADC0)-OFFSET;  // Get data...
    adc_flag_clear(ADC0, ADC_FLAG_EOC);         // ...clear IF...
    adc_software_trigger_enable(ADC0,           // Trigger another ADC conversion!
                                ADC_REGULAR_CHANNEL);
}

void bsp_debug(void) {
   if (debug) {
      gpio_bit_reset(GPIOA, GPIO_PIN_1);
      if (t5expq()) l88row(colset());
   }
}

void bsp_performance(){
    if (debug) {
       delta = *( volatile uint64_t * )( TIMER_CTRL_ADDR + TIMER_MTIME ) - delta;
       l88mem(0, (delta>>24) & 0xFF);
       l88mem(1, (delta>>16) & 0xFF);
       l88mem(2, (delta>> 8) & 0xFF);
       l88mem(3, (delta>> 0) & 0xFF);
    }
}