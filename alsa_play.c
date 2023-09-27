#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include <alsa/asoundlib.h>
#include <alsa/pcm.h>

#define FRAMES 768

// Alsa playback

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }

    // Open PCM device for playback.
    snd_pcm_t *playback_handle;
    int err = snd_pcm_open(&playback_handle, "plughw:1", SND_PCM_STREAM_PLAYBACK, 0);
    if (err)
    {
        printf("Unable to open PCM device: %s\n", snd_strerror(err));
        return 1;
    }

    // Allocate a hardware parameters object.
    snd_pcm_hw_params_t *hw_params;
    snd_pcm_hw_params_malloc(&hw_params);

    // Fill it in with default values.
    err = snd_pcm_hw_params_any(playback_handle, hw_params);
    if (err)
    {
        printf("Can not configure this PCM device: %s\n", snd_strerror(err));
        return 1;
    }

    // Set the desired hardware parameters.
    // Interleaved mode
    err = snd_pcm_hw_params_set_access(playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (err)
    {
        printf("Error setting interleaved mode: %s\n", snd_strerror(err));
        return 1;
    }

    // Signed 16-bit little-endian format
    err = snd_pcm_hw_params_set_format(playback_handle, hw_params, SND_PCM_FORMAT_S32_LE);
    if (err)
    {
        printf("Error setting format: %s\n", snd_strerror(err));
        return 1;
    }

    // Two channels (stereo)
    err = snd_pcm_hw_params_set_channels(playback_handle, hw_params, 2);
    if (err)
    {
        printf("Error setting channels: %s\n", snd_strerror(err));
        return 1;
    }

    // 48000 bits/second sampling rate
    unsigned int val = 48000;
    err = snd_pcm_hw_params_set_rate_near(playback_handle, hw_params, &val, NULL);
    if (err)
    {
        printf("Error setting sampling rate (%d): %s\n", val, snd_strerror(err));
        return 1;
    }

    // Write the parameters to the driver
    err = snd_pcm_hw_params(playback_handle, hw_params);
    if (err)
    {
        printf("Error setting HW params: %s\n", snd_strerror(err));
        return 1;
    }

    int audio_fd = open(argv[1], O_RDONLY);

    if (audio_fd < 0)
    {
        printf("Error opening file %s\n", argv[1]);
        return 1;
    }

    char buf[FRAMES * 4 * 2];
    int frames = 0;

    while (1)
    {
        int n = read(audio_fd, buf, FRAMES * 4 * 2);
        if (n <= 0)
            break;

        frames += n / 4 / 2;

        err = snd_pcm_writei(playback_handle, buf, n / 4 / 2);
        if (err == -EPIPE)
        {
            printf("Underrun occurred\n");
            snd_pcm_prepare(playback_handle);
        }
        else if (err < 0)
        {
            printf("Error from writei: %s\n", snd_strerror(err));
        }
        else if (err != n / 4 / 2)
        {
            printf("Short write, write %d frames\n", err);
        }
    }

    snd_pcm_drain(playback_handle);

    close(audio_fd);
    snd_pcm_hw_params_free(hw_params);
    snd_pcm_close(playback_handle);


    return 0;
}