#ifndef _WSS_H
#define _WSS_H

#include "inttypes.h"

#define WSS_BASE 0x530
#define WSS_DMA 1
#define WSS_IRQ 5

/* Direct registers definitions as in YMF715 datasheet*/
#define WSS_INDEX_REG (WSS_BASE+4) 
#define WSS_IDATA_REG (WSS_BASE+5)
#define WSS_STATUS_REG (WSS_BASE+6)
#define WSS_PDATA_REG (WSS_BASE+7)

/* Direct registers bit definitions */
/* WSS_INDEX_REG bits */
#define WSS_INIT 0x80
#define WSS_MCE 0x40
#define WSS_TRD 0x20

/* WSS_STATUS_REG bits */
#define WSS_CU_L 0x80
#define WSS_CL_R 0x40
#define WSS_CRDY 0x20
#define WSS_SOUR 0x10
#define WSS_PU_L 0x08
#define WSS_PL_R 0x04
#define WSS_PRDY 0x02
#define WSS_INT 0x01

/* Indirect registers definitions */
#define WSS_LIN_REG 0
#define WSS_RIN_REG 1
#define WSS_LAUX1_REG 2
#define WSS_RAUX1_REG 3
#define WSS_LAUX2_REG 4
#define WSS_RAUX2_REG 5
#define WSS_LDAC_REG 6
#define WSS_RDAC_REG 7
#define WSS_FORMAT_REG 8
#define WSS_CONFIG_REG 9
#define WSS_PIN_REG 10
#define WSS_INIT_REG 11
#define WSS_CTRL_REG 12
#define WSS_MIX_REG 13
#define WSS_UCNT_REG 14
#define WSS_LCNT_REG 15

/* WSS_LIN_REG & WSS_RIN_REG bits */
#define WSS_XSS1 0x80
#define WSS_XSS0 0x40
#define WSS_XMGE 0x20

/* WSS_LAUX1_REG, WSS_RAUX1_REG, WSS_LAUX2_REG & WSS_RAUX2_REG bits */
#define WSS_XMXX 0x80

/* WSS_LDAC_REG & WSS_RDAC_REG bits */
#define WSS_XDM 0x80

/* WSS_FORMAT_REG bits */
#define WSS_FMT 0x40
#define WSS_L_C 0x20
#define WSS_S_M 0x10
#define WSS_CFS2 0x08
#define WSS_CFS1 0x04
#define WSS_CFS0 0x02
#define WSS_CSS 0x01

/* WSS_CONFIG_REG bits */
#define WSS_CPIO 0x80
#define WSS_PPIO 0x40
#define WSS_CAL1 0x10
#define WSS_CAL0 0x08
#define WSS_SDC 0x04
#define WSS_CEN 0x02
#define WSS_PEN 0x01

/* WSS_PIN_REG bits */
#define WSS_XCTL1 0x80
#define WSS_XCTL0 0x40
#define WSS_IEN 0x02

/* WSS_INIT_REG bits */
#define WSS_COR 0x80
#define WSS_PUR 0x40
#define WSS_ACI 0x20
#define WSS_DRS 0x10

/* WSS_CTRL_REG bits */
#define WSS_MODE 0x40

/* WSS_MIX_REG bits */
#define WSS_DME 0x01

void wss_wait(void);

void wss_write_dir(uint8_t, uint8_t);
uint8_t wss_read_dir(uint8_t);

void wss_write_indir(uint8_t, uint8_t);
uint8_t wss_read_indir(uint8_t);

void wss_start(void);
void wss_stop(void);


#endif