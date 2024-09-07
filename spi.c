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



static uint32_t speed;
static int spi_fd = -1;
extern struct gpiod_line *cs;
extern struct gpiod_line *prgmn;
char a='A';
int configure_output(struct gpiod_line *line, const char *consumer, int value)
{
    if (gpiod_line_request_output(line, consumer, value) < 0)
    {
        perror("Request line as output failed");
        return -1;
    }

    return 0;
}


int spi_init(int spi_speed,struct gpiod_line **cs,struct gpiod_line **prgrmn){
	
	int ret = 0;
	uint8_t mode = SPI_MODE_0;
	uint8_t bits = 8;
	uint32_t reg;
	uint32_t shift;

	// setup spi via ioctl

	speed = spi_speed;

	spi_fd = open("/dev/spidev0.0", O_RDWR);
	if (spi_fd < 0) {
		fprintf(stderr, "Failed to open /dev/spidev0.0: %s\n", strerror(errno));
		return 0;
	}

	ret |= ioctl(spi_fd, SPI_IOC_WR_MODE, &mode);
	ret |= ioctl(spi_fd, SPI_IOC_RD_MODE, &mode);
	ret |= ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	ret |= ioctl(spi_fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	ret |= ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	ret |= ioctl(spi_fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);

	if(ret) {
		fprintf(stderr, "Failed to setup SPI.\n");
	if (spi_fd > -1)
		close(spi_fd);
	spi_fd = -1;
		return 0;
	}	
	
	
	
	
	
	
    //GPIO CS and CRST initialize
    struct gpiod_chip *chip;

    
    
     // Replace with your GPIO pin number for CRST
    int offset_cs = 21; // Replace with your GPIO pin number for CS
    int offset_prgmn = 20; // Replace with your GPIO pin number for CS
    
    chip = gpiod_chip_open("/dev/gpiochip4"); // Replace 4 with the appropriate chip number

    if (!chip) {
        perror("Open chip failed");
        return 1;
    }


    *cs= gpiod_chip_get_line(chip, offset_cs);
    *prgrmn= gpiod_chip_get_line(chip, offset_prgmn);


    
    if (!*cs) {
        perror("Get line failed");
        gpiod_chip_close(chip);
        return 1;
    }
    
        
    if (!*prgrmn) {
        perror("Get line failed");
        gpiod_chip_close(chip);
        return 1;
    }



     configure_output(*cs, &a, 1);
     configure_output(*prgrmn, &a, 1);
 
    usleep(100);
 
    return 0;
    
}

// Function for ioctl call
static int rbpi_ioctl(unsigned char *tx_buf, unsigned char *rx_buf, int len)
{
	struct spi_ioc_transfer req;

	memset(&req, 0, sizeof(req));

	req.tx_buf = (uintptr_t) tx_buf;
	req.rx_buf = (uintptr_t) rx_buf;
	req.len = len;

	return ioctl(spi_fd, SPI_IOC_MESSAGE(1), &req) == -1;
}


// Function for data transmit
int rbpi_tx(unsigned char *buf, int bytes)
{




	if(rbpi_ioctl(buf, NULL, bytes)) {
		fprintf(stderr, "SPI ioctl write failed: %s\n", strerror(errno));
		return 0;
	} else
		return 1;
}


int rbpi_rx(unsigned char *buf, int bytes)
{




	if(rbpi_ioctl(NULL,buf, bytes)) {
		fprintf(stderr, "SPI ioctl write failed: %s\n", strerror(errno));
		return 0;
	} else
		return 1;
}





//Closes the SPI bus

int rbpi_exit(){
	
	close(spi_fd);
}












void device_id(){
    
	unsigned char write_buf[6] = { 0xE0 ,0x00,0x00,0x00,0x00,0x00};
	unsigned char read_buf[4];
    
	gpiod_line_set_value(cs, 1);
	usleep(1);
	gpiod_line_set_value(cs, 0);
	rbpi_tx(write_buf,4);
	rbpi_rx(read_buf,4);
	gpiod_line_set_value(cs, 1);
	
	printf("Device ID Read:");
	for (size_t i = 0; i < 4; i++) {
	
	    printf("0x%02X ", read_buf[i]); // Print each element in hexadecimal format
	}
	printf("\n");
}


void sr_check(int done){
    
	unsigned char write_buf[6] = { 0x3C ,0x00,0x00,0x00,0x00,0x00};
	unsigned char read_buf[8];
    
	gpiod_line_set_value(cs, 1);
	usleep(1);
	gpiod_line_set_value(cs, 0);
	rbpi_tx(write_buf,4);
	rbpi_rx(read_buf,8);
	gpiod_line_set_value(cs, 1);
	
	printf("Status Register Read:");
	for (size_t i = 0; i < 8; i++) {
	
	    printf("0x%02X ", read_buf[i]); // Print each element in hexadecimal format
	}
	printf("\n");
	
    //bytes[1] = 0b00101000;  // Set bits for illustration (bit 8 and bit 13)

    // Check bit 8 (second byte, 0th bit in index 1)
    int bit8 = (read_buf[6] & (1 << 0)) ? 1 : 0; //done bit

    // Check bit 13 (second byte, 5th bit in index 1) //fail flag
    int bit13 = (read_buf[6] & (1 << 5)) ? 1 : 0;


    if(bit13==0){
	printf("Fail flag not triggered\n");    
     }
    else{
	 printf("Fail flag triggered. Exitin\n");
	 exit(0);   
	}
     
     if(bit8==done){
	 printf("Done bit is %d. No errors\n",bit8);    
	     
	}
     else{
	printf("Done bit is %d. Error detected. Exiting....\n",bit8); 
	exit(0);    
	     
	}
	
	
}





void activation_key(){
    
	unsigned char write_buf[6] = { 0xA4 ,0xC6,0xF4,0x8A};
	unsigned char read_buf[4];
    
	gpiod_line_set_value(cs, 1);
	usleep(1);
	gpiod_line_set_value(cs, 0);
	rbpi_tx(write_buf,4);
	gpiod_line_set_value(cs, 1);
	
	printf("Activation Key sent Successfully");

		printf("\n");
}

void isc_enable(){
    
	unsigned char write_buf[6] = { 0xC6 ,0x00,0x00,0x00};
	unsigned char read_buf[4];
    
	gpiod_line_set_value(cs, 1);
	usleep(1);
	gpiod_line_set_value(cs, 0);
	rbpi_tx(write_buf,4);
	gpiod_line_set_value(cs, 1);
	printf("Device Enters Programming Mode");
	printf("\n");

}


void isc_disable(){
    
	unsigned char write_buf[6] = { 0x26 ,0x00,0x00,0x00};
	unsigned char read_buf[4];
    
	gpiod_line_set_value(cs, 1);
	usleep(1);
	gpiod_line_set_value(cs, 0);
	rbpi_tx(write_buf,4);
	gpiod_line_set_value(cs, 1);
	printf("Device exits Programming Mode");
	printf("\n");

}



void isc_erase(){
    
	unsigned char write_buf[6] = { 0x0E ,0x00,0x00,0x00};
	unsigned char read_buf[4];
    
	gpiod_line_set_value(cs, 1);
	usleep(1);
	gpiod_line_set_value(cs, 0);
	rbpi_tx(write_buf,4);
	gpiod_line_set_value(cs, 1);
	printf("SRAM Erased");
	printf("\n");

}


