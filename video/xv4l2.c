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


#define PR(x) do {} while(0)
// #define PR(x) fprintf(stderr,"trace: %s\n",x)
#define CLEAR(x) memset(&(x), 0, sizeof(x))

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
            PR("ioctl start");
            r = v4l2_ioctl(fh, request, arg);     usleep(1000);
            PR("ioctl returns");
        } while (to-- && r == -1 && ((errno == EINTR) || (errno == EAGAIN)));

        if (r == -1) {
                fprintf(stderr, "error %d, %s\\n", errno, strerror(errno));
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

    XV.fd = v4l2_open(XV.dev_name, O_RDWR | O_NONBLOCK, 0);
        if (XV.fd < 0) {
                perror("Cannot open device");
                exit(EXIT_FAILURE);
        }

        CLEAR(XV.fmt);
        XV.fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        XV.fmt.fmt.pix.width       = 320;
        XV.fmt.fmt.pix.height      = 240;
        XV.fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
        XV.fmt.fmt.pix.field       = 0; // V4L2_FIELD_INTERLACED;
        xioctl(XV.fd, VIDIOC_S_FMT, &XV.fmt);
        if (XV.fmt.fmt.pix.pixelformat != V4L2_PIX_FMT_RGB24) {
                printf("Libv4l didn't accept RGB24 format. Can't proceed.\\n");
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


        PR("start cap");
        XV.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        xioctl(XV.fd, VIDIOC_STREAMON, &XV.type);

        return XV.fd;
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

    /* rbg24 to bgr24 */
    unsigned char *src = XV.buffers[XV.buf.index].start;
    unsigned char *dst = data;
    unsigned src_len = XV.buf.bytesused;
    unsigned dst_len = bytes_per_line * h;
    unsigned src_w = XV.fmt.fmt.pix.width;
    unsigned src_h =  XV.fmt.fmt.pix.height;
    unsigned x,y;
    unsigned char r,g,b;

    /*
      printf("src=%ux%d (%u) dst=%ux%u\n", src_w, src_h, src_len,
           w,h );
           printf("bpl=%u\n", bytes_per_line );
    */
    for( y=0;y < src_h; y++) {
        for(x=0;x<src_w;x++) {
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

    PR("deq 2");
    xioctl(XV.fd, VIDIOC_QBUF, &XV.buf);

    /*
    PR("stop cap");
    XV.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    xioctl(XV.fd, VIDIOC_STREAMOFF, &XV.type);
    */
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
