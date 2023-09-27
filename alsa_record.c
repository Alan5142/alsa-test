#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include <alsa/asoundlib.h>
#include <alsa/pcm.h>

#define FRAMES 768

// Alsa record

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: %s <device>\n", argv[0]);
        return 1;
    }

    FILE *fd = fopen(argv[1], "wb");

    // Open PCM device for recording (capture).
    snd_pcm_t *capture_handle;
    int err = snd_pcm_open(&capture_handle, "hw:0", SND_PCM_STREAM_CAPTURE, 0);
    if (err)
    {
        printf("Unable to open PCM device: %s\n", snd_strerror(err));
        return 1;
    }

    // Allocate a hardware parameters object.
    snd_pcm_hw_params_t *hw_params;
    snd_pcm_hw_params_malloc(&hw_params);

    // Fill it in with default values.
    err = snd_pcm_hw_params_any(capture_handle, hw_params);
    if (err)
    {
        printf("Can not configure this PCM device: %s\n", snd_strerror(err));
        return 1;
    }

    // Set the desired hardware parameters.
    // Interleaved mode
    err = snd_pcm_hw_params_set_access(capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (err)
    {
        printf("Error setting interleaved mode: %s\n", snd_strerror(err));
        return 1;
    }

    // Signed 16-bit little-endian format
    err = snd_pcm_hw_params_set_format(capture_handle, hw_params, SND_PCM_FORMAT_S32_LE);
    if (err)
    {
        printf("Error setting format: %s\n", snd_strerror(err));
        return 1;
    }

    // Two channels (stereo)
    err = snd_pcm_hw_params_set_channels(capture_handle, hw_params, 2);
    if (err)
    {
        printf("Error setting channels: %s\n", snd_strerror(err));
        return 1;
    }

    // 48000 bits/second sampling rate
    unsigned int val = 48000;
    err = snd_pcm_hw_params_set_rate_near(capture_handle, hw_params, &val, NULL);
    if (err)
    {
        printf("Error setting sampling rate (%d): %s\n", val, snd_strerror(err));
        return 1;
    }

    // Write the parameters to the driver
    err = snd_pcm_hw_params(capture_handle, hw_params);
    if (err)
    {
        printf("Error setting HW params: %s\n", snd_strerror(err));
        return 1;
    }

    char *buf = calloc(FRAMES, 4 * 2);
    while (1)
    {
        snd_pcm_sframes_t frames = snd_pcm_readi(capture_handle, buf, FRAMES);
        fwrite(buf, 1, frames * 4 * 2, fd);
        fflush(fd);
    }

    snd_pcm_hw_params_free(hw_params);
    snd_pcm_close(capture_handle);


    return 0;
}