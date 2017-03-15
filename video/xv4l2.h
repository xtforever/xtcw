#ifndef XV4L2_H
#define XV4L2_H

int xv4l2_init_grab(void);
void xv4l2_grab(unsigned char *data, unsigned w, unsigned h, unsigned bytes_per_line);
void xv4l2_wait_rdy(void);
void xv4l2_destroy(void);


#endif
