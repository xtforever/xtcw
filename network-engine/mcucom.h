#ifndef MCUCOM_H
#define MCUCOM_H

int mcu_init(char *host, char* port);
int mcu_connect(int n);
int mcu_get_sln(int n);
int mcu_req(int n, const char *cmd);
void mcu_free(int n);
void mcu_destruct(void);
int mcu_get_msg(int n);
int mcu_error(int n);
#endif
