#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <termios.h>
#define port "/dev/ttyACM0"
#define BAUD B115200
#define chunk_size 1024
#define min_music 300.0f
#define max_music 5000.0f
int serial_fd(const char *port){
    int fd=open(port,|)
}
int main (){
    signal(SIGINT,cleanup);
    serial_fd=setup_serial(port);
    if (serial_fd<0){
        return 1;
    }
:wq
    return 0;
}
