#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "prussdrv.h"
#include "pruss_intc_mapping.h"
#include "mio.h"


#define PERROR() printf("[!] %s:%u\n", __FUNCTION__, __LINE__)


/* cm */

/* doc: spruh73c, table 2.2 */
#define CM_PER_MIO_ADDR 0x44e00000
#define CM_PER_MIO_SIZE (128 * 1024)

/* doc: spruh73c, table 2.2 */
#define CM_WKUP_MIO_ADDR 0x44e00400
#define CM_WKUP_MIO_SIZE 0x1000

/* doc: spruh73c, table 8.28 */
#define CM_PER_EPWMSS0_CLKCTRL 0xd4
#define CM_PER_EPWMSS1_CLKCTRL 0xcc
#define CM_PER_EPWMSS2_CLKCTRL 0xd8

/* doc: spruh73c, table 8.28 */
#define CM_WKUP_ADC_TSC_CLKCTRL 0xbc

static int cm_per_enable_pwmss(size_t i)
{
  /* enable clocking of the pwmss[i] */
  printf("Enabling PWM clocking\n");
  static const size_t off[] =
  {
    CM_PER_EPWMSS0_CLKCTRL,
    CM_PER_EPWMSS1_CLKCTRL,
    CM_PER_EPWMSS2_CLKCTRL
  };

  mio_handle_t mio;

  if (mio_open(&mio, CM_PER_MIO_ADDR, CM_PER_MIO_SIZE)) return -1;
  mio_write_uint32(&mio, off[i], 2);
  mio_close(&mio);

  return 0;
}


/* epwm */

static int epwm_setup(void)
{
  printf("ePWM setup\n");
  cm_per_enable_pwmss(0);
  cm_per_enable_pwmss(1);
  cm_per_enable_pwmss(2);

  return 0;
}


/* tsc adc */

/* doc: spruh73c, table 2.2 */
#define ADC_MIO_ADDR 0x44e0d000
#define ADC_MIO_SIZE (8 * 1024)

/* doc: spruh73c, table 12.4 */
#define ADC_REG_SYSCONFIG 0x10
#define ADC_REG_IRQENABLE_SET 0x2c
#define ADC_REG_IRQWAKEUP 0x34
#define ADC_REG_DMAENABLE_SET 0x38
#define ADC_REG_CTRL 0x40
#define ADC_REG_ADC_CLKDIV 0x4c
#define ADC_REG_STEPENABLE 0x54
#define ADC_REG_TS_CHARGE_DELAY 0x60
#define ADC_REG_STEP1CONFIG 0x64
#define ADC_REG_FIFO0COUNT 0xe4
#define ADC_REG_FIFO0DATA 0x100

static int adc_setup(void)
{
  printf("ADC setup\n");

  /* enable adc_tsc clocking */
  printf("Enabling ADC_TSC clocking\n");
  mio_handle_t mio;

  if (mio_open(&mio, CM_WKUP_MIO_ADDR, CM_WKUP_MIO_SIZE)) return -1;
  mio_write_uint32(&mio, CM_WKUP_ADC_TSC_CLKCTRL, 2);
  mio_close(&mio);
  return 0;
}


/* main */

int main(int ac, const char** av)
{
  tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;

  adc_setup();
  epwm_setup();

  printf("Initializing PRU\n");
  prussdrv_init();

  if (prussdrv_open(PRU_EVTOUT_0))
  {
    printf("prussdrv_open open failed\n");
    return -1;
  }

  prussdrv_pruintc_init(&pruss_intc_initdata);

  /* execute code on pru0 */
#define PRU_NUM 0
  printf("Running main.bin\n");
  prussdrv_exec_program(PRU_NUM, "./main.bin");

  /* wait until pru0 finishes */
  /* Won't happen without interrupt stuff implemented */
  prussdrv_pru_wait_event(PRU_EVTOUT_0);
  prussdrv_pru_clear_event(PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);


  /* disable pru and close memory mapping */
  /* never reached without ARM to PRU interrupt */
  prussdrv_pru_disable(PRU_NUM);
  prussdrv_exit();
  printf("Resources returned\n");

  return 0;
}
