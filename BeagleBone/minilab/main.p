#include "pru.h"
#include "pru_macros.hp"
#include "epwm.hp"
#include "adc.hp"


.origin 0
.entrypoint MAIN


MAIN:
 /* enable ocp wide accesses for shared memory */

 LBCO r0, CONST_PRUCFG, 4, 4
 CLR r0, r0, 4
 SBCO r0, CONST_PRUCFG, 4, 4

 /* prepared pru to host shared memory */

 MOV r0, 0x000000120
 MOV r1, CTPPR_0
 ST32 r0, r1

 MOV r0, 0x00100000
 MOV r1, CTPPR_1
 ST32 r0, r1

 /* setup epwm, adc */

 /* 600ns for 166kHz */
 EPWM_SETUP
 EPWM_SET_PERIOD 600

 ADC_SETUP

 /* main loop */

 WHILE_TRUE:
 /* r0 holds result from ADC_READ */
 EPWM_SET_DUTY r0
 ADC_READ
 JMP WHILE_TRUE

 /* signal cpu we are done (never reached) */

 MOV r31.b0, PRU0_ARM_INTERRUPT + 16
 HALT
