#include "wss.h"
#include "buffer.h"
#include <dos.h>

void wss_wait(void) //WTF is this?
{
	int16_t timeout = 0x2000;
	while((inp(WSS_INDEX_REG) & WSS_INIT) && (timeout--));
}

void wss_write_indir(uint8_t indir_addr, uint8_t indir_data)
{
	outp(WSS_INDEX_REG, indir_addr);
	wss_wait();
	outp(WSS_IDATA_REG, indir_data);
	wss_wait();
}

uint8_t wss_read_indir(uint8_t indir_addr)
{
	outp(WSS_INDEX_REG, indir_addr);
	return inp(WSS_IDATA_REG);
}

void wss_start(void)
{
	wss_write_indir(WSS_LIN_REG, 0x00); //Min gain on left input, line-in
	wss_write_indir(WSS_RIN_REG, 0x00); //Min gain on right input, line-in

	wss_write_indir(WSS_LAUX1_REG, WSS_XMXX); //Mute left aux 1
	wss_write_indir(WSS_RAUX1_REG, WSS_XMXX); //Mute right aux 1

	wss_write_indir(WSS_LAUX1_REG, WSS_XMXX); //Mute left aux 1
	wss_write_indir(WSS_RAUX1_REG, WSS_XMXX); //Mute right aux 1

	wss_write_indir(WSS_LDAC_REG, 0x00); //Left DAC no attenuation
	wss_write_indir(WSS_RDAC_REG, 0x00); //Right DAC no attenuation

	wss_write_indir(WSS_CTRL_REG, 0x8A); //Mode 1 - legacy WSS
	wss_write_indir(WSS_MIX_REG, 0x00); //Mute loopback
	
	wss_write_indir(WSS_PIN_REG, WSS_IEN); //Enable interrupt pit
	
	wss_write_indir(WSS_LCNT_REG, ((BUFFER_SIZE_BYTES/8) - 1) & 0xFF); //Set interrupt counter
	wss_write_indir(WSS_UCNT_REG, ((BUFFER_SIZE_BYTES/8) - 1) >> 8);
	
	wss_write_indir(WSS_MCE | WSS_FORMAT_REG, WSS_FMT | WSS_S_M | WSS_CFS2 | WSS_CFS0 | WSS_CSS);  //Enable MCE, address format register, 16-bit stereo signed PCM, 44.1kHz, XTAL2
	wss_write_indir(WSS_MCE | WSS_CONFIG_REG, WSS_PEN | WSS_CAL0 | WSS_CAL1); //Hold MCE, enable playback, perform full calibration
	outp(WSS_INDEX_REG, WSS_INIT_REG); //Disable MCE, address init/error register
	while(inp(WSS_IDATA_REG) & WSS_ACI); //Wait until calibration is done
}

void wss_restart(void)
{
	wss_write_indir(WSS_CONFIG_REG, WSS_PEN); //Enable playback
}	

void wss_stop(void)
{
	uint8_t reg;
	outp(WSS_INDEX_REG, WSS_CONFIG_REG); //Address config register
	reg = inp(WSS_IDATA_REG); //Get current register value
	outp(WSS_IDATA_REG, reg & ~WSS_PEN); //Turn off playback
}