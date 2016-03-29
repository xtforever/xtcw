#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <alsa/asoundlib.h>

static char *device = "default"; /* playback device */
snd_output_t *output = NULL;

#define CHECK(a) show_error( __LINE__, (a), #a )
void show_error( int line, int err, char *expr )
{
    if( err < 0 ) {
        printf("ERROR (line:%d) in %s\n%s\n", line, expr ,snd_strerror(err) );
        exit(EXIT_FAILURE);
    }
}

typedef struct {
    char     chunk_id[4];
    uint32_t chunk_size;
    char     format[4];
    char     fmtchunk_id[4];
    uint32_t fmtchunk_size;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bps;
    char     datachunk_id[4];
    uint32_t datachunk_size;
}WavHeader;

struct wav_data {
    char *buf;
    unsigned pos, len, audio_format, num_channels, bps, sample_rate;
};

/* 44.1 KHz, 32 bit float LE */
void wav_read(char *fn, struct wav_data *d)
{
    struct stat sb;
    int fd;
    assert( (fd=open(fn, O_RDONLY))>0 );
    fstat(fd, &sb);
    printf("%s Size: %lu\n",fn, (unsigned long)sb.st_size);

    char *buf;
    assert( buf = malloc( sb.st_size ));
    assert( read(fd, buf, sb.st_size ) == sb.st_size );
    close(fd);
    d->buf = buf + sizeof(WavHeader);
    d->len = sb.st_size - sizeof(WavHeader);

    WavHeader *w = (void*)buf;
    printf("Channels:   %d\n", w->num_channels );
    printf("Format:     %d\n", w->audio_format  );
    printf("SampleRate: %d\n", w->sample_rate  );
    printf("bps:        %d\n", w->bps);
    printf("chunk_size  %d\n", w->chunk_size );
    printf("hdr_size  %d\n",   sizeof( WavHeader ) );
    unsigned bytes;

    if( w->audio_format == 1 ) {
        d->audio_format = SND_PCM_FORMAT_S16_LE;
    }

    if( w->audio_format == 3 ) {
        d->audio_format = SND_PCM_FORMAT_FLOAT_LE;
    }
    assert((w->audio_format ==1) || (w->audio_format ==3));

    bytes = (w->bps / 8) * (d->num_channels = w->num_channels);
    d->sample_rate = w->sample_rate;
    d->bps = bytes;
    /* format 1: 16bit signed int SND_PCM_FORMAT_S16_LE */
    /* format 3: 32bit float le      SND_PCM_FORMAT_FLOAT_LE,*/


    printf("Channels:   %d\n", d->num_channels );
    printf("Format:     %d\n", d->audio_format  );
    printf("SampleRate: %d\n", d->sample_rate  );
    printf("bps:        %d\n", d->bps);
}


int wav_play( char *fn )
{
    int err;
    snd_pcm_t *handle;
    snd_pcm_sframes_t avail;
    struct wav_data data;

    wav_read(fn, & data );
    data.pos = 0;

    CHECK( snd_pcm_open(&handle, device, SND_PCM_STREAM_PLAYBACK, 0));
    CHECK( snd_pcm_set_params(handle,
                              data.audio_format,
                              SND_PCM_ACCESS_RW_INTERLEAVED,
                              data.num_channels,
                              data.sample_rate,
                              1,
                              500000));    /* 0.5sec */

    CHECK(snd_pcm_prepare (handle));

    uint min = 4096;

    while( data.pos < data.len ) {
        avail = snd_pcm_avail_update(handle);
        printf("avail %u\n", (unsigned)avail );
        if( avail >= min ) {
            if( avail > (data.len - data.pos)/ data.bps )
                avail = (data.len - data.pos)/ data.bps;
            if( ! avail ) break;
            err=snd_pcm_writei(handle, data.buf + data.pos, avail );
            if( err > 0 ) data.pos +=err * data.bps; else break;
        }
        usleep( 125000 ); /* 0.1s */
    }

    snd_pcm_close(handle);
    return EXIT_SUCCESS;
}


int main (int argc, char *argv[])
{
    wav_play( argv[1] );
    return EXIT_SUCCESS;
}
