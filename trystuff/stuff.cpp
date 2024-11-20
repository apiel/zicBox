#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#define MAX7219_REG_NOOP 0x00
#define MAX7219_REG_DIGIT0 0x01
#define MAX7219_REG_DIGIT1 0x02
#define MAX7219_REG_DIGIT2 0x03
#define MAX7219_REG_DIGIT3 0x04
#define MAX7219_REG_DIGIT4 0x05
#define MAX7219_REG_DIGIT5 0x06
#define MAX7219_REG_DIGIT6 0x07
#define MAX7219_REG_DIGIT7 0x08
#define MAX7219_REG_DECODEMODE 0x09
#define MAX7219_REG_INTENSITY 0x0A
#define MAX7219_REG_SCANLIMIT 0x0B
#define MAX7219_REG_SHUTDOWN 0x0C
#define MAX7219_REG_DISPLAYTEST 0x0F

#define CMD_WRITE 0x40

typedef struct
{

 int digits[8];

} max7219;


max7219 header;

int spi_fd;
struct sigaction act;
static char *spiDevice = "/dev/spidev0.1";
static uint8_t spiBPW = 8;
static uint32_t spiSpeed = 5000000;
static uint16_t spiDelay = 0;

int spiOpen(char* dev)
{
  if((spi_fd = open(dev, O_RDWR)) < 0)
  {
    printf("error opening %s\n",dev);
    return -1;
  }
  return 0;
}

void writeByte(uint8_t reg, uint8_t data)
{
  uint8_t spiBufTx [3];
  uint8_t spiBufRx [3];
  struct spi_ioc_transfer spi;
  memset (&spi, 0, sizeof(spi));
  spiBufTx [0] = CMD_WRITE;
  spiBufTx [1] = reg;
  spiBufTx [2] = data;
  spi.tx_buf =(unsigned long)spiBufTx;
  spi.rx_buf =(unsigned long)spiBufRx;
  spi.len = 3;
  spi.delay_usecs = spiDelay;
  spi.speed_hz = spiSpeed;
  spi.bits_per_word = spiBPW;
  ioctl(spi_fd, SPI_IOC_MESSAGE(1), &spi);
}

void writeBytes(max7219 *header, int cmd, int data)
{
  if(!header) return;
  writeByte(cmd, data);
}

void setBrightness(max7219 *header, int bright)
{
  if(!header) return;
  if (bright < 0) bright = 0;
  if (bright > 15) bright = 15;

  writeBytes(header, MAX7219_REG_INTENSITY, bright);
}

void initialiseDisplay(max7219 *header)
{
  spiOpen(spiDevice);

  writeBytes(header, MAX7219_REG_SCANLIMIT, 7);
  writeBytes(header, MAX7219_REG_DECODEMODE, 0xff);
  writeBytes(header, MAX7219_REG_SHUTDOWN, 1);
  writeBytes(header, MAX7219_REG_DISPLAYTEST, 0);
  setBrightness(header, 5);
}

void byte_to_binary(int x)
{
    static char b[9];
    b[0] = '\0';

    int z;
    for (z = 128; z > 0; z >>= 1)
    {
        strcat(b, ((x & z) == z) ? "1" : "0");
    }
    printf("%s", b);
}

void digitDisplay(max7219 *header)
{
  int i;
  if(!header) return;
  for (i=0; i<8; i++)
  {
    writeBytes(header, i+1, header->digits[i]);
  }
}

void clearDisplay(max7219 *header)
{
  int i;
  if(!header) return;
  for (i=0; i<8; i++) {
    header->digits[i] = 0x0F;
  }
  digitDisplay(header);
}

uint8_t asciiToBCD (uint8_t chr)
{
  uint8_t bcd_value;
  bcd_value = (chr - 48);
  return bcd_value;
}

void writeDigits(max7219 *header, char chars[12])
{
  int i,j;
  if (!header) return;
  i = 7;
  j = 0;
  while (j <= 11 && i >= 0)
  {
    switch ((int)chars[j]) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':

      header->digits[i] = asciiToBCD(chars[j]);
      j++;
      i--;
      break;
    case ' ':
      header->digits[i] = 0x0F;
      j++;
      i--;
      break;
    case '-':
      header->digits[i] = 0x0a;
      j++;
      i--;
      break;
    case ':':
      header->digits[i] = 0x8f;
      j++;
      i--;
      break;
    case '.':
      header->digits[i+1] = header->digits[i+1] + 0x80;
      j++;
      break;
    case 0x00:
      j = 12;
      i = -1;
      break;
    default:
      j++;
      break;
    }
  }
  digitDisplay(header);
}

void sig_handler(int signum, siginfo_t *info, void *ptr)
{
  clearDisplay(&header);
  printf("Received signal %d\n", signum);
  printf("Signal originated from process %lu\n",
    (unsigned long)info->si_pid);
  exit(0);
}

int main()
{
  memset(&act, 0, sizeof(act));

  act.sa_sigaction = sig_handler;
  act.sa_flags = SA_SIGINFO;
  sigaction(SIGTERM, &act, NULL);

  char chars[12] = "12:34:56";
  initialiseDisplay(&header);
  clearDisplay(&header);

  while (1)
  {

    clearDisplay(&header);
    writeDigits(&header, chars);
    sleep(1);
  }

  return 0;
}


/*
 * spi-driver-speed.c
 * 2016-11-23
 * Public Domain
*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

/*
spi-driver-speed.c

gcc -o spi-driver-speed spi-driver-speed.c

./spi-driver-speed [bytes [bps [loops] ] ]
*/

#define LOOPS 10000
#define SPEED 1000000
#define BYTES 3

double time_time(void)
{
   struct timeval tv;
   double t;

   gettimeofday(&tv, 0);

   t = (double)tv.tv_sec + ((double)tv.tv_usec / 1E6);

   return t;
}

int spiOpen(unsigned spiChan, unsigned spiBaud, unsigned spiFlags)
{
   int i,fd;
   char  spiMode;
   char  spiBits  = 8;
   char dev[32];

   spiMode  = spiFlags & 3;
   spiBits  = 8;

   sprintf(dev, "/dev/spidev0.%d", spiChan);

   if ((fd = open(dev, O_RDWR)) < 0)
   {
      return -1;
   }

   if (ioctl(fd, SPI_IOC_WR_MODE, &spiMode) < 0)
   {
      close(fd);
      return -2;
   }

   if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &spiBits) < 0)
   {
      close(fd);
      return -3;
   }

   if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &spiBaud) < 0)
   {
      close(fd);
      return -4;
   }

   return fd;
}

int spiClose(int fd)
{
   return close(fd);
}

int spiRead(int fd, unsigned speed, char *buf, unsigned count)
{
   int err;
   struct spi_ioc_transfer spi;

   memset(&spi, 0, sizeof(spi));

   spi.tx_buf        = (unsigned) NULL;
   spi.rx_buf        = (unsigned) buf;
   spi.len           = count;
   spi.speed_hz      = speed;
   spi.delay_usecs   = 0;
   spi.bits_per_word = 8;
   spi.cs_change     = 0;

   err = ioctl(fd, SPI_IOC_MESSAGE(1), &spi);

   return err;
}

int spiWrite(int fd, unsigned speed, char *buf, unsigned count)
{
   int err;
   struct spi_ioc_transfer spi;

   memset(&spi, 0, sizeof(spi));

   spi.tx_buf        = (unsigned) buf;
   spi.rx_buf        = (unsigned) NULL;
   spi.len           = count;
   spi.speed_hz      = speed;
   spi.delay_usecs   = 0;
   spi.bits_per_word = 8;
   spi.cs_change     = 0;

   err = ioctl(fd, SPI_IOC_MESSAGE(1), &spi);

   return err;
}

int spiXfer(int fd, unsigned speed, char *txBuf, char *rxBuf, unsigned count)
{
   int err;
   struct spi_ioc_transfer spi;

   memset(&spi, 0, sizeof(spi));

   spi.tx_buf        = (unsigned long)txBuf;
   spi.rx_buf        = (unsigned long)rxBuf;
   spi.len           = count;
   spi.speed_hz      = speed;
   spi.delay_usecs   = 0;
   spi.bits_per_word = 8;
   spi.cs_change     = 0;

   err = ioctl(fd, SPI_IOC_MESSAGE(1), &spi);

   return err;
}

#define MAX_SPI_BUFSIZ 8192

char RXBuf[MAX_SPI_BUFSIZ];
char TXBuf[MAX_SPI_BUFSIZ];

int bytes = BYTES;
int speed = SPEED;
int loops = LOOPS;

int main(int argc, char * argv[])
{
   int i, fd;
   double start, diff, sps;

   if (argc > 1) bytes = atoi(argv[1]);
   else printf("./spi-driver-speed [bytes [bps [loops] ] ]\n\n");

   if ((bytes < 1) || (bytes > MAX_SPI_BUFSIZ)) bytes = BYTES;

   if (argc > 2) speed = atoi(argv[2]);
   if ((speed < 32000) || (speed > 250000000)) speed = SPEED;

   if (argc > 3) loops = atoi(argv[3]);
   if ((loops < 1) || (loops > 10000000)) loops = LOOPS;

   fd = spiOpen(0, speed, 0);

   start = time_time();

   if (fd < 0) return 1;

   for (i=0; i<loops; i++)
   {
      spiXfer(fd, speed, TXBuf, RXBuf, bytes);
   }

   diff = time_time() - start;

   close(fd);

   printf("sps=%.1f: %d bytes @ %d bps (loops=%d time=%.1f)\n",
      (double)loops / diff, bytes, speed, loops, diff);
}



/****************************************
* basic SPI demo for mcp23s17 *
*****************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>


#define CMD_WRITE 0x40
#define CMD_READ 0x41


//static char *spiDevice = "/dev/spidev0.0" ;
static uint8_t spiMode = 0 ;
static uint8_t spiBPW = 8 ;
static uint32_t spiSpeed = 5000 ;
static uint16_t spiDelay = 0;


// MCP23S17 Registers
#define IOCON 0x0A
#define IODIRA 0x00
#define IPOLA 0x02
#define GPINTENA 0x04
#define DEFVALA 0x06
#define INTCONA 0x08
#define GPPUA 0x0C
#define INTFA 0x0E
#define INTCAPA 0x10
#define GPIOA 0x12
#define OLATA 0x14
#define IODIRB 0x01
#define IPOLB 0x03
#define GPINTENB 0x05
#define DEFVALB 0x07
#define INTCONB 0x09
#define GPPUB 0x0D
#define INTFB 0x0F
#define INTCAPB 0x11
#define GPIOB 0x13
#define OLATB 0x15
int spi_fd;


/**************** ReadByte Routine ****************/
static uint8_t readByte (uint8_t reg)
{
uint8_t tx [4] ;
uint8_t rx [4] ;
struct spi_ioc_transfer spi ;
tx [0] = CMD_READ ;
tx [1] = reg ;
tx [2] = 0 ;
spi.tx_buf = (unsigned long)tx ;
spi.rx_buf = (unsigned long)rx ;
spi.len = 3 ;
spi.delay_usecs = spiDelay ;
spi.speed_hz = spiSpeed ;
spi.bits_per_word = spiBPW ;
ioctl (spi_fd, SPI_IOC_MESSAGE(1), &spi) ;
return rx [2] ;
}


/**************** writeByte Routine ****************/
static void writeByte (uint8_t reg, uint8_t data)
{
uint8_t spiBufTx [3] ;
uint8_t spiBufRx [3] ;
struct spi_ioc_transfer spi ;
spiBufTx [0] = CMD_WRITE ;
spiBufTx [1] = reg ;
spiBufTx [2] = data ;
spi.tx_buf = (unsigned long)spiBufTx ;
spi.rx_buf = (unsigned long)spiBufRx ;
spi.len = 3 ;
spi.delay_usecs = spiDelay ;
spi.speed_hz = spiSpeed ;
spi.bits_per_word = spiBPW ;
ioctl (spi_fd, SPI_IOC_MESSAGE(1), &spi) ;
}




/*spi_open
* - Open the given SPI channel and configures it.
* - there are normally two SPI devices on your PI:
* /dev/spidev0.0: activates the CS0 pin during transfer
* /dev/spidev0.1: activates the CS1 pin during transfer
*
*/
int spi_open(char* dev)
{
if((spi_fd = open(dev, O_RDWR)) < 0){
printf("error opening %s\n",dev);
return -1;
}
return 0;
}



/**************** Main Routine ****************/
int main(int argc, char* argv[])
{
unsigned char data = 0xAF;
if(argc <= 1){
printf("too few args, try %s /dev/spidev0.0\n",argv[0]);
return -1;
}
// open and configure SPI channel. (/dev/spidev0.0 for example)
if(spi_open(argv[1]) < 0){
printf("spi_open failed\n");
return -1;
}
writeByte (IODIRB, 0x00) ; // Port A -> Outputs
writeByte (GPIOB, 0xFF) ;
//writeByte (IPOLB, 0x01) ; // invert lsb
//writeByte (GPPUA, 0xFF) ; // enable pullups.
//writeByte (IODIRA, 0xFF) ; // Port B -> Inputs
data = readByte (GPIOA) ;
printf("RECEIVED: %.2X\n",data);
//data = readByte (GPIOB);
//printf ("GPIOA: %.2x\n",data);

close(spi_fd);
return 0;
}