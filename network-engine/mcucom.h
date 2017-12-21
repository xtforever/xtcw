#ifndef MCUCOM_H
#define MCUCOM_H

int mcu_init(char *host, int port);
int mcu_req(int n, char *cmd);
void mcu_free(int n);
void mcu_destruct(void);

#endif
