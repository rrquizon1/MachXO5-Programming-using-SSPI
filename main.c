#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <gpiod.h>
#include "spi_data.h"
#include "spi.h"





struct gpiod_line *cs;
struct gpiod_line *prgrmn;

#define num_chunks (g_iDataSize+3-1)/3

int main() {
    unsigned char lsc_bitstream_burst[4]={0x7A,0x00,0x00,0x00};
    spi_init(3000*1024,&cs,&prgrmn);
    gpiod_line_set_value(prgrmn, 0);// programn pulled low
    usleep(50);// Wait time till initn goes high

     activation_key();// Send Slave SPI activation key
     
    gpiod_line_set_value(prgrmn, 1); //Pull programn high
    usleep(1000);
    //Device ID read 	
    device_id();
    
    usleep(1000);
    //ISC_ENABLE
    isc_enable();
    usleep(1000);
    
    //ISC_ERASE
    isc_erase();
    usleep(10000);
    
    //Status Register Check, DONE=0 and Fail=0
    sr_check(0);
    
   
    
    printf("Sending bitstream using LSC_BITSTREAM_BURST\n");
    gpiod_line_set_value(cs, 1);
    usleep(1000);
    gpiod_line_set_value(cs, 0);

    rbpi_tx(lsc_bitstream_burst,4);  //Send 0x7A command
    for(int k=0;k<num_chunks;k++){
	rbpi_tx(&g_pucDataArray[k*3],3);//Send bitstream by chunks.
	}
    
    usleep(10000);
    
    
    //Status Register Check, DONE=1 and Fail=0
    sr_check(1);

    usleep(1000);
	
    //Exits programming mode and enters usermode
    isc_disable();

    rbpi_exit();
	
	
}
