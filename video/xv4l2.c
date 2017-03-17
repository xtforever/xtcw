/* V4L2 video picture grabber
   Copyright (C) 2009 Mauro Carvalho Chehab <mchehab@infradead.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
 */

#include "xv4l2.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <libv4l2.h>
#include <mls.h>

void xv4l2_wait_rdy(void);

#define PR(x) do {} while(0)
// #define PR(x) fprintf(stderr,"trace: %s\n",x)
#define CLEAR(x) memset(&(x), 0, sizeof(x))
void yuv420tobgr(uint16_t width, uint16_t height, const uint8_t *y, const uint8_t *u, const uint8_t *v,
                 unsigned int ystride, unsigned int ustride, unsigned int vstride, uint8_t *out);

void simple_rgb24tobgr32(uint16_t width, uint16_t height, uint8_t *src, uint8_t *dst );
void simple_yuv420tobgr(uint16_t width, uint16_t height, uint8_t *src, uint8_t *dst );

struct buffer {
        void   *start;
        size_t length;
};

static void xioctl(int fh, int request, void *arg)
{
    int r;
    int to=5;

    xv4l2_wait_rdy();

        do {
            r = v4l2_ioctl(fh, request, arg);     usleep(1000);
        } while (to-- && r == -1 && ((errno == EINTR) || (errno == EAGAIN)));

        if (r == -1) {
                fprintf(stderr, "error %d, %s\n", errno, strerror(errno));
                exit(EXIT_FAILURE);
        }


}

struct xv4l2_st {
        struct v4l2_format              fmt;
        struct v4l2_buffer              buf;
        struct v4l2_requestbuffers      req;
        enum v4l2_buf_type              type;
    int                             r, fd;
    char                            *dev_name;
        char                            out_name[256];
        FILE                            *fout;
        struct buffer                   *buffers;
};

static struct xv4l2_st XV = {
    .fd = -1,
    .dev_name = "/dev/video0"
};

int xv4l2_init_grab(void)
{
        unsigned int                    i, n_buffers;

        if( XV.fd >= 0 ) return -1;

        XV.fd = v4l2_open(XV.dev_name, O_RDWR | O_NONBLOCK, 0);
        if (XV.fd < 0) {
                perror("Cannot open device");
                exit(EXIT_FAILURE);
        }

        int pix_format = V4L2_PIX_FMT_RGB24;
        // int pix_format = V4L2_PIX_FMT_YUV420;
        CLEAR(XV.fmt);
        XV.fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        XV.fmt.fmt.pix.width       = 320;
        XV.fmt.fmt.pix.height      = 240;
        XV.fmt.fmt.pix.pixelformat = pix_format;
        // XV.fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
        XV.fmt.fmt.pix.field       = 0; // V4L2_FIELD_INTERLACED;
        xioctl(XV.fd, VIDIOC_S_FMT, &XV.fmt);
        if (XV.fmt.fmt.pix.pixelformat != pix_format ) {
                printf("Libv4l didn't accept RGB24 format. Can't proceed.\\n");
                printf("returns: %u\n", XV.fmt.fmt.pix.pixelformat );
                exit(EXIT_FAILURE);
        }
        if ((XV.fmt.fmt.pix.width != 320) || (XV.fmt.fmt.pix.height != 240))
                printf("Warning: driver is sending image at %dx%d\\n",
                        XV.fmt.fmt.pix.width, XV.fmt.fmt.pix.height);
        PR("format ok");
        CLEAR(XV.req);
        XV.req.count = 2;
        XV.req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        XV.req.memory = V4L2_MEMORY_MMAP;
        xioctl(XV.fd, VIDIOC_REQBUFS, &XV.req);

        XV.buffers = calloc(XV.req.count, sizeof(*XV.buffers));
        for (n_buffers = 0; n_buffers < XV.req.count; ++n_buffers) {
                CLEAR(XV.buf);

                XV.buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                XV.buf.memory      = V4L2_MEMORY_MMAP;
                XV.buf.index       = n_buffers;

                xioctl(XV.fd, VIDIOC_QUERYBUF, &XV.buf);

                XV.buffers[n_buffers].length = XV.buf.length;
                XV.buffers[n_buffers].start = v4l2_mmap(NULL, XV.buf.length,
                              PROT_READ | PROT_WRITE, MAP_SHARED,
                              XV.fd, XV.buf.m.offset);

                if (MAP_FAILED == XV.buffers[n_buffers].start) {
                        perror("mmap");
                        exit(EXIT_FAILURE);
                }
        }

        for (i = 0; i < XV.req.count; ++i) {
                CLEAR(XV.buf);
                XV.buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                XV.buf.memory = V4L2_MEMORY_MMAP;
                XV.buf.index = i;
                xioctl(XV.fd, VIDIOC_QBUF, &XV.buf);
        }



        return XV.fd;
}


void xv4l2_start_capture(void)
{
    xv4l2_init_grab();
    PR("start cap");
    XV.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    xioctl(XV.fd, VIDIOC_STREAMON, &XV.type);
    xv4l2_wait_rdy();
}

void xv4l2_stop_capture(void)
{
    PR("stop cap");
    XV.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    xioctl(XV.fd, VIDIOC_STREAMOFF, &XV.type);
    xv4l2_destroy();
}


void xv4l2_grab(unsigned char *data, unsigned w,unsigned h, unsigned bytes_per_line )
{
    PR("CLEAR");
    CLEAR( XV.buf);
    XV.buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    XV.buf.memory = V4L2_MEMORY_MMAP;
    PR("DEQUEUE");
    xioctl( XV.fd, VIDIOC_DQBUF, & XV.buf);
    PR("generating image");
    // XV.buffers[XV.buf.index].start, XV.buf.bytesused
    // XV.fmt.fmt.pix.width, XV.fmt.fmt.pix.height

    unsigned char *src = XV.buffers[XV.buf.index].start;
    unsigned char *dst = data;
    // unsigned src_len = XV.buf.bytesused;
    // unsigned dst_len = bytes_per_line * h;
    unsigned src_w = XV.fmt.fmt.pix.width;
    unsigned src_h =  XV.fmt.fmt.pix.height;



    if( XV.fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_YUV420 ) {
        simple_yuv420tobgr(src_w, src_h, src, dst );
    }
    else if( XV.fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_RGB24 ) {
        simple_rgb24tobgr32(src_w, src_h, src, dst );
    }

    PR("deq 2");
    xioctl(XV.fd, VIDIOC_QBUF, &XV.buf);
}

void simple_rgb24tobgr32(uint16_t width, uint16_t height, uint8_t *src, uint8_t *dst )
{
    unsigned int src_len = width * height * 3,
        dst_len = width * height * 4,
        x,y;

    for( y=0; y < height; y++) {
        for(x=0; x < width; x++) {
            uint8_t r,g,b;
            if( src_len < 3 || dst_len < 4 ) return;
            r = src[0];
            g = src[1];
            b = src[2];
            dst[0] = b;
            dst[1] = g;
            dst[2] = r;
            src+=3; src_len-=3;
            dst+=4; dst_len-=4;
        }
    }
}



/*

 */
void yuv420tobgr(uint16_t width, uint16_t height, const uint8_t *y, const uint8_t *u, const uint8_t *v,
                 unsigned int ystride, unsigned int ustride, unsigned int vstride, uint8_t *out) {
    for (unsigned long int i = 0; i < height; ++i) {
        for (unsigned long int j = 0; j < width; ++j) {
            uint8_t *point = out + 4 * ((i * width) + j);
            int       t_y   = y[((i * ystride) + j)];
            const int t_u   = u[(((i / 2) * ustride) + (j / 2))];
            const int t_v   = v[(((i / 2) * vstride) + (j / 2))];
            t_y            = t_y < 16 ? 16 : t_y;

            const int r = (298 * (t_y - 16) + 409 * (t_v - 128) + 128) >> 8;
            const int g = (298 * (t_y - 16) - 100 * (t_u - 128) - 208 * (t_v - 128) + 128) >> 8;
            const int b = (298 * (t_y - 16) + 516 * (t_u - 128) + 128) >> 8;

            point[2] = r > 255 ? 255 : r < 0 ? 0 : r;
            point[1] = g > 255 ? 255 : g < 0 ? 0 : g;
            point[0] = b > 255 ? 255 : b < 0 ? 0 : b;
            point[3] = ~0;
        }
    }
}

void simple_yuv420tobgr(uint16_t width, uint16_t height, uint8_t *src, uint8_t *dst )
{
    unsigned int x,y;
    uint8_t *point = dst;
    uint8_t *dy = src;
    uint8_t *du = src + (width * height);
    uint8_t *dv = du  + (width * height) / 4;

    for (y = 0; y < height; ++y) {
        for (x = 0; x < width; ++x) {
            int       t_y   = dy[x];
            int t_u   = du[x/2];
            int t_v   = dv[x/2];
            t_y            = t_y < 16 ? 16 : t_y;
            int r = (298 * (t_y - 16) + 409 * (t_v - 128) + 128) >> 8;
            int g = (298 * (t_y - 16) - 100 * (t_u - 128) - 208 * (t_v - 128) + 128) >> 8;
            int b = (298 * (t_y - 16) + 516 * (t_u - 128) + 128) >> 8;
            point[0] = b > 255 ? 255 : b < 0 ? 0 : b;
            point[1] = g > 255 ? 255 : g < 0 ? 0 : g;
            point[2] = r > 255 ? 255 : r < 0 ? 0 : r;
            point[3] = ~0;
            point+=4;
        }
        dy += width;
        if( y & 1) { du += width/2;  dv += width/2; }
    }
}


void xv4l2_wait_rdy(void)
{
    int r;
    fd_set                          fds;
    struct timeval                  tv;

    do {
        PR("wait 4s max");
        FD_ZERO(&fds);
        FD_SET(XV.fd, &fds);
        tv.tv_sec = 4;         /* Timeout. */
        tv.tv_usec = 0;
        r = select(XV.fd + 1, &fds, NULL, NULL, &tv);
        PR("select returns...");

    } while ((r == -1 && (errno = EINTR)));
    if (r == -1) {
        perror("select");
        ERR("select error");
    }
    if( r == 0 ) {
        PR("timeout in select\n");
    }

    PR("select ok...");
}

void xv4l2_destroy(void)
{
    unsigned int i;
    if( XV.fd < 0 ) return;
    XV.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    xioctl(XV.fd, VIDIOC_STREAMOFF, &XV.type);
    for (i = 0; i < XV.req.count; ++i)
        v4l2_munmap(XV.buffers[i].start, XV.buffers[i].length);
    v4l2_close(XV.fd);
    XV.fd=-1;
}

#if 0
int main(int argc, char **argv)
{
    unsigned char data[320*240*3];
    xv4l2_init_grab();
    wait_rdy();
    xv4l2_grab( data, sizeof(data) );
    xv4l2_destroy();

    return 0;
}
#endif
