/* character set R */
#include "lpc17xx_uart.h"		// Central include files
#include "lpc17xx_pinsel.h"
#include "lpc17xx_i2c.h"
#include "lpc_types.h"
#include "stdio.h"
#include "serial.h"			// Local functions

uint8_t send[2] = {0,0};
uint8_t receive[1] = {0};
I2C_M_SETUP_Type setup =
{
  .tx_data = &send,
  .tx_length = 2,
  .tx_count = 0,
  .rx_data = receive,
  .rx_length = 0,
  .rx_count = 0,
  .retransmissions_max = 3,
  .retransmissions_count = 0,
  .status = 0
};

void pins_i2c(void);
void init_i2c(void);
void clear_lcd(void);
void write_lcd_pos(uint8_t character , uint8_t position);
void write_i2c(void);

void pins_i2c(void)
{
  PINSEL_CFG_Type PinCfg;
  PinCfg.Funcnum = 3;
  PinCfg.OpenDrain = 0;
  PinCfg.Pinmode = 0;
  PinCfg.Portnum = 0;
  PinCfg.Pinnum = 0;
  PINSEL_ConfigPin(&PinCfg);
  PinCfg.Pinnum = 1;
  PINSEL_ConfigPin(&PinCfg);
}

void init_i2c(void)
{
  pins_i2c();
  I2C_Init(LPC_I2C1, 100000);
  I2C_Cmd(LPC_I2C1, ENABLE);
}

void init_lcd(void)
{
  uint32_t index;
  const uint32_t init_bytes[10] = {0x34 , 0x0c , 0x06 , 0x35 , 0x04 , 0x10 , 0x42 , 0x9f , 0x34 , 0x02};
  setup.sl_addr7bit = 0x3b;
  send[0] = 0x00;
  for(index = 0 ; index < 10 ; index++)
  {
    send[1] = init_bytes[index];
    write_i2c();
  }
}

void clear_lcd(void)
{
  /*
   * 10.2.2.1 Clear_display - pg. 32/86 
   * When using character set ‘R’, the following alternative instruction set has to be used:
   * 1.  Switch display off (Display_ctl, bit D = 0).
   * 2.  Write a blank pattern into all DDRAM addresses (Write_data).
   * 3.  Switch display on (Display_ctl, bit D = 1).
   *
   * 10.2.2.4 Display_ctl - pg. 33/86
   * control byte = 0
   * command byte = 0x08 (off) 0x0C (on)
   *
   * blank patterns = 0xA0, 0x91, 0x9B
   */
  uint32_t tick;
  //uint32_t position;
  send[0] = 0x00;
  /*
  // Switch off display
  send[1] = 0x08;
  write_i2c();

  // Write blank pattern into all DDRAM addresses
  for(position = 0 ; position < 40 ; position++)
    write_lcd_pos(0xA0 , position);

  // Switch on display
  send[1] = 0x0C;
  write_i2c();
  */
  // Clear the display
  send[1] = 0x01;
  write_i2c();
  tick = 0;
  while(tick++ < 1024)
    ;
}

void write_lcd_pos(uint8_t character , uint8_t position)
{
  send[0] = 0x00;
  send[1] = 0x80 + position;
  write_i2c();

  send[0] = 0x40;
  send[1] = character;
  write_i2c();
}

void write_i2c(void)
{
  Status result;
  result = I2C_MasterTransferData(LPC_I2C1, &setup, I2C_TRANSFER_POLLING);
  if(!result)
    write_usb_serial_blocking("ERROR\n\r" , 7);
}

// Entry point for the program
void main(void)
{
  uint32_t position;
  int returncode;
  char result_string[11];

  // initialise serial
  serial_init();
  write_usb_serial_blocking("LCD\n\r" , 5);

  // initialise I2C and LCD
  init_i2c();
  init_lcd();
  clear_lcd();

  returncode = 0;
  returncode = sprintf(result_string, "%02X %02X %02X\n\r", setup.sl_addr7bit , *setup.tx_data , *(setup.tx_data+1));
  write_usb_serial_blocking(result_string , 10);

  // write characters to all positions
  for(position = 0 ; position < 16 ; position++)
  {
    write_lcd_pos(0x7F , position);
    sprintf(result_string, "%02X %02X %02X\n\r", setup.sl_addr7bit , *setup.tx_data , *(setup.tx_data+1));
    write_usb_serial_blocking(result_string , 10);
  }

  write_usb_serial_blocking("End\n\r" , 5);

  while(1);
}
