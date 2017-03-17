#ifndef XV4L2_H
#define XV4L2_H

int xv4l2_init_grab(void);
void xv4l2_grab(unsigned char *data, unsigned w, unsigned h, unsigned bytes_per_line);
void xv4l2_destroy(void);
void xv4l2_start_capture(void);
void xv4l2_stop_capture(void);

#endif
