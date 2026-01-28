// audio_led.c
#define _POSIX_C_SOURCE 199309L // Must be at the top
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdint.h>
#include <signal.h>
#include <time.h>
#define SERIAL_PORT "/dev/ttyACM1"
#define BAUD B115200
#define CHUNK_SIZE 1024
#define MIN_MUSIC 300.0f
#define MAX_MUSIC 5000.0f

// Global for cleanup
FILE *audio_pipe = NULL;
int serial_fd = -1;

void cleanup(int sig) {
    printf("\n\nStopping...\n");
    if (audio_pipe) pclose(audio_pipe);
    if (serial_fd >= 0) {
        write(serial_fd, "0\n", 2);
        close(serial_fd);
    }
    printf("Done!\n");
    exit(0);
}

int setup_serial(const char *port) {
    int fd = open(port, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        perror("Error opening serial port");
        return -1;
    }
    
    struct termios tty;
    memset(&tty, 0, sizeof(tty));
    
    if (tcgetattr(fd, &tty) != 0) {
        perror("tcgetattr");
        return -1;
    }
    
    cfsetospeed(&tty, BAUD);
    cfsetispeed(&tty, BAUD);
    
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_iflag &= ~IGNBRK;
    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 5;
    
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~(PARENB | PARODD);
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;
    
    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("tcsetattr");
        return -1;
    }
    
    return fd;
}

float calculate_rms(int16_t *buffer, size_t size) {
    float sum = 0.0f;
    size_t i;
    for (i = 0; i < size; i++) {
        float val = (float)buffer[i];
        sum += val * val;
    }
    return sqrtf(sum / (float)size);
}

int main() {
    // Signal handler for clean exit
    signal(SIGINT, cleanup);
    
    // Setup serial
    serial_fd = setup_serial(SERIAL_PORT);
    if (serial_fd < 0) {
        return 1;
    }
    
    printf("C Audio Visualizer\n");
    sleep(1); // Give Arduino time to reset
    
    // Start audio capture
    audio_pipe = popen(
        "parec --device=alsa_output.pci-0000_08_00.6.HiFi__Speaker__sink.monitor "
        "--format=s16le --rate=48000 --channels=2",
        "r"
    );
    
    if (!audio_pipe) {
        perror("popen failed");
        close(serial_fd);
        return 1;
    }
    
    printf("Capturing audio -> Arduino\n");
    printf("Play some music!\n\n");
    
    int16_t buffer[CHUNK_SIZE];
    float smoothed = 0.0f;
    unsigned int counter = 0;
    
    while (1) {
        struct timespec start,end;
        clock_gettime(CLOCK_MONOTONIC,&start);
        size_t n;
        float rms;
        int volume;
        int final;
        char msg[16];
        int len;
        int i;
        int bars;
        
        // Read audio chunk
        n = fread(buffer, sizeof(int16_t), CHUNK_SIZE, audio_pipe);
        if (n == 0) break;
        
        // Calculate RMS
        rms = calculate_rms(buffer, n);
        
        // Map to brightness
        volume = 0;
        if (rms >= MIN_MUSIC) {
            volume = (int)(((rms - MIN_MUSIC) / (MAX_MUSIC - MIN_MUSIC)) * 255.0f);
            if (volume < 0) volume = 0;
            if (volume > 255) volume = 255;
        }
        
        // Smooth the output
        smoothed = 0.6f * (float)volume + 0.4f * smoothed;
        final = (int)smoothed;
        
        // Send to Arduino
        len = snprintf(msg, sizeof(msg), "%d\n", final);
        write(serial_fd, msg, len);
        
        // Print status
        if (counter % 100 == 0) {
            // Visual bar
            bars = final / 16;
            printf("RMS: %5.0f | Vol: %3d | ", rms, final);
            for (i = 0; i < 16; i++) {
                printf("%c", i < bars ? '#' : '-');
            }
            printf("\r");
            fflush(stdout);
        }
        counter++;
    }
    clock_gettime(ClOCK_MONOTONIC,&end);
    double time_taken=(end.tv_sec-start.tv_sec)+(end.tv_nsec-start.tv_nsec)/1e9;
    printf("%f",time_taken);
    cleanup(0);
    return 0;
}
