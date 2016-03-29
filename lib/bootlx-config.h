#ifndef BOOTLX_CONFIG_H
#define BOOTLX_CONFIG_H

enum twi_device_list {
  TWI_ID_MURPHY =  1 + 10,
  TWI_ID_TOLA =    2 + 10,
  TWI_ID_SAMSON =  3 + 10,
  TWI_ID_GUS =     4 + 10, /* same as MONSTA */
  TWI_ID_ZORG =    5 + 10,
  TWI_ID_JOAB =    6 + 10,
  TWI_ID_AMON =    7 + 10,
  TWI_ID_MONSTA =  TWI_ID_GUS,
  TWI_ID_ANTON  =    8 + 10,
  TWI_ID_MAX  =    9 + 10
};

enum upgrade_device_list {
  DEVICE_ID_MURPHY =  1+48,
  DEVICE_ID_TOLA =    2+48,
  DEVICE_ID_SAMSON =  3+48,
  DEVICE_ID_GUS =     4+48,
  DEVICE_ID_ZORG =    5+48,
  DEVICE_ID_JOAB =    6+48,
  DEVICE_ID_AMON =    7+48,
  DEVICE_ID_MONSTA =  8+48,
  DEVICE_ID_ANTON =   9+48,
  DEVICE_ID_MAX  =   10+48
};

#define ID_STR_MURPHY   "MURPHY"
#define ID_STR_TOLA     "TOLA"
#define ID_STR_SAMSON   "SAMSON"
#define ID_STR_GUS      "GUS"
#define ID_STR_ZORG     "ZORG"
#define ID_STR_JOAB     "JOAB"
#define ID_STR_AMON     "AMON"
#define ID_STR_MONSTA   "MONSTA"
#define ID_STR_ANTON    "ANTON"


enum upgrade_command_table {
  RESP_OK = '!',
  RESP_ERROR = '?',
  RESP_PAGESIZE256 = '8',
  RESP_PAGESIZE64 =  '6',
  CMD_SYNC = 's',
  CMD_PAGESIZE = 'p',
  CMD_EXIT = 'x',
  CMD_ADDR = 'a',
  CMD_VERSION = 'v',
  CMD_FILL = 'f',
  CMD_RDFLASH = 'r',
  CMD_ID = 'i',
  CMD_SR = 'm'

};

#endif
