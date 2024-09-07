This is an SSPI SRAM programming using Raspberry pi 5. 

This example uses lgpiod library so make sure to install it: sudo apt-get install gpiod libgpiod-dev.

Nexus devices have larger bitstream size compared to ice40 devices this means that we have to divide the bitstream by chunks to overcome the SPI buffer limitation.

The instructions used in this program follows the instruction from FPGA-TN-02271-2.1([MachXO5 Programming and Configuration User Guide](https://www.latticesemi.com/view_document?document_id=53489))

Source files:
main.c- contains the programming procedure

spi.c- contains function for SPI transaction and some programming commands for Nexus devices.

spi_data.c- contains the data to be programmed

Makefile- compiles the project.

You can modify g_iDataSize and g_pucDataArray with your bitstream information. 
With this example, MachXO5-NX Development board default bitstream is used which has 583773 bytes of data.
![image](https://github.com/user-attachments/assets/5860d121-3ea6-4213-9e6e-8507a80ef5be)

Main.c follows the procedure indicated in FPGA-TN-02271-2.1
![image](https://github.com/user-attachments/assets/4b0bbd80-181d-4006-9acf-dc0b451e9130)


Sample Waveform transactions:

Reset Sequence:
![image](https://github.com/user-attachments/assets/e8dc633c-16cb-492a-b4d6-70435fda5f9e)

Sending bitstream by chunks:
![image](https://github.com/user-attachments/assets/5d98ad5b-4240-4521-8e1a-760e691aa5d5)

Key points to ensure during the Slave SPI (SSPI) configuration flow:

1.Confirm that INITN is high before sending the Slave SPI activation key. You can either monitor INITN or add a delay after pulling PROGRAMN low before sending the activation key.

2. Read the status registers before and after sending the bitstream to verify that the DONE bit is programmed correctly and there are no failure in configuration. 

