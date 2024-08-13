#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define DEFAULT_DATA_SIZE (256)

static void pabort(const char *s)
{
    perror(s);
    abort();
}

static const char *device = "/dev/spidev4.0";
static uint32_t mode;
static uint8_t bits = 16;
static uint32_t speed = 25000000;
static uint16_t delay;
static int verbose;
static int spidev3_0;
static int spislave;
static int data_size = DEFAULT_DATA_SIZE;

static void hex_dump(const void *src, size_t length, size_t line_size, char *prefix)
{
    int i = 0;
    const unsigned char *address = src;
    const unsigned char *line = address;
    unsigned char c;
    printf("%s | ", prefix);
    while (length-- > 0) {
        printf("%02X ", *address++);
        if (!(++i % line_size) || (length == 0 && i % line_size)) {
            if (length == 0) {
                while (i++ % line_size)
                    printf("__ ");
            }
            printf(" | ");
            while (line < address) {
                c = *line++;
                printf("%c", (c < 33 || c == 255) ? 0x2E : c);
            }
            printf("\n");
            if (length > 0)
                printf("%s | ", prefix);
        }
    }
}

static int unescape(char *_dst, char *_src, size_t len)
{
    int ret = 0;
    char *src = _src;
    char *dst = _dst;
    unsigned int ch;

    while (*src) {
        if (*src == '\\' && *(src + 1) == 'x') {
            sscanf(src + 2, "%2x", &ch);
            src += 4;
            *dst++ = (unsigned char)ch;
        } else {
            *dst++ = *src++;
        }
        ret++;
    }
    return ret;
}

static void transfer(int fd, uint16_t const *tx, uint16_t const *rx, size_t len, int dump_flag)
{
    int ret;

    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx,
        .rx_buf = (unsigned long)rx,
        .len = len,
        .delay_usecs = delay,
        .speed_hz = speed,
        .bits_per_word = bits,
    };

    if (mode & SPI_TX_QUAD)
        tr.tx_nbits = 4;
    else if (mode & SPI_TX_DUAL)
        tr.tx_nbits = 2;
    if (mode & SPI_RX_QUAD)
        tr.rx_nbits = 4;
    else if (mode & SPI_RX_DUAL)
        tr.rx_nbits = 2;
    if (!(mode & SPI_LOOP)) {
        if (mode & (SPI_TX_QUAD | SPI_TX_DUAL))
            tr.rx_buf = 0;
        else if (mode & (SPI_RX_QUAD | SPI_RX_DUAL))
            tr.tx_buf = 0;
    }

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 1)
        pabort("can't send spi message");

    if (dump_flag) {
        if (verbose)
            hex_dump(tx, len, 16, "TX");
        hex_dump(rx, len, 16, "RX");
    }
}

static void slave_transfer(int fd, uint16_t *rx, size_t len, int dump_flag) {
    int ret;
    struct spi_ioc_transfer tr = {
        .tx_buf = 0,  // 처음에는 송신할 데이터가 없으므로 NULL로 설정
        .rx_buf = (unsigned long)rx,
        .len = len,
        .delay_usecs = delay,
        .speed_hz = speed,
        .bits_per_word = bits,
    };

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 1) {
        pabort("can't receive spi message");
    }

    if (dump_flag) {
        hex_dump(rx, len, 16, "RX (received)");
    }

    // 유효한 데이터가 있는지 확인
    int valid_data = 0;
    for (size_t i = 0; i < len / sizeof(uint16_t); i++) {
        if (rx[i] != 0) {  // 유효한 데이터가 있는지 확인
            valid_data = 1;
            break;
        }
    }

    // 유효한 데이터가 있으면 그 데이터를 에코로 전송
    if (valid_data) {
        struct spi_ioc_transfer tr_echo = {
            .tx_buf = (unsigned long)rx,  // RX 버퍼의 데이터를 TX로 전송
            .rx_buf = 0,                  // 에코 전송이므로 수신할 데이터는 필요 없음
            .len = len,
            .delay_usecs = delay,
            .speed_hz = speed,
            .bits_per_word = bits,
        };

        ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr_echo);
        if (ret < 1) {
            pabort("can't send spi echo message");
        }

        if (dump_flag) {
            hex_dump(rx, len, 16, "TX (echo)");
        }
    }
}

static void print_usage(const char *prog)
{
    printf("Usage: %s [-DsbdlHOLC3]\n", prog);
    puts("  -D --device   device to use (default /dev/spidev1.0)\n"
         "  -s --speed    max speed (Hz)\n"
         "  -d --delay    delay (usec)\n"
         "  -b --bpw      bits per word \n"
         "  -l --loop     loopback\n"
         "  -H --cpha     clock phase\n"
         "  -O --cpol     clock polarity\n"
         "  -L --lsb      least significant bit first\n"
         "  -C --cs-high  chip select active high\n"
         "  -3 --3wire    SI/SO signals shared\n"
         "  -v --verbose  Verbose (show tx buffer)\n"
         "  -i --spidev3.0  dev changed \n"
         "  -p --size     Data size to transfer (default 256 bytes)\n"
         "  -N --no-cs    no chip select\n"
         "  -R --ready    slave pulls low to pause\n"
         "  -2 --dual     dual transfer\n"
         "  -4 --quad     quad transfer\n");
    exit(1);
}

static void parse_opts(int argc, char *argv[])
{
    while (1) {
        static const struct option lopts[] = {
            {"device",  1, 0, 'D'},
            {"speed",   1, 0, 's'},
            {"delay",   1, 0, 'd'},
            {"bpw",     1, 0, 'b'},
            {"loop",    0, 0, 'l'},
            {"cpha",    0, 0, 'H'},
            {"cpol",    0, 0, 'O'},
            {"lsb",     0, 0, 'L'},
            {"cs-high", 0, 0, 'C'},
            {"3wire",   0, 0, '3'},
            {"no-cs",   0, 0, 'N'},
            {"ready",   0, 0, 'R'},
            {"dual",    0, 0, '2'},
            {"verbose", 0, 0, 'v'},
            {"spidev3.0", 0, 0, 'i'},
            {"spislave", 0, 0, 'S'},
            {"size",    1, 0, 'p'},
            {"quad",    0, 0, '4'},
            {NULL, 0, 0, 0},
        };
        int c;

        c = getopt_long(argc, argv, "D:s:d:b:lHOLC3NR24p:viS", lopts, NULL);

        if (c == -1)
            break;

        switch (c) {
            case 'D':
                device = optarg;
                break;
            case 's':
                speed = atoi(optarg);
                break;
            case 'd':
                delay = atoi(optarg);
                break;
            case 'b':
                bits = atoi(optarg);
                break;
            case 'l':
                mode |= SPI_LOOP;
                break;
            case 'H':
                mode |= SPI_CPHA;
                break;
            case 'O':
                mode |= SPI_CPOL;
                break;
            case 'L':
                mode |= SPI_LSB_FIRST;
                break;
            case 'C':
                mode |= SPI_CS_HIGH;
                break;
            case '3':
                mode |= SPI_3WIRE;
                break;
            case 'N':
                mode |= SPI_NO_CS;
                break;
            case 'v':
                verbose = 1;
                break;
            case 'R':
                mode |= SPI_READY;
                break;
            case 'p':
                data_size = atoi(optarg);
                if (data_size > 256 || data_size <= 0) {
                    fprintf(stderr, "Data size must be between 1 and 256 bytes.\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case '2':
                mode |= SPI_TX_DUAL;
                break;
            case '4':
                mode |= SPI_TX_QUAD;
                break;
            case 'i':
                spidev3_0 = 1;
                device = "/dev/spidev3.0";
                break;
            case 'S':
                spislave = 1;
                break;
            default:
                print_usage(argv[0]);
                break;
        }
    }

    if (mode & SPI_LOOP) {
        if (mode & SPI_TX_DUAL)
            mode |= SPI_RX_DUAL;
        if (mode & SPI_TX_QUAD)
            mode |= SPI_RX_QUAD;
    }
}

#define DATA_EA (1)		//(100000)

int main(int argc, char *argv[])
{
    int ret = 0;
    int i = 0;
    int fd;
	int testCount = 1;
    uint8_t *tx;
    uint8_t *rx;

    parse_opts(argc, argv);
    mode = SPI_MODE_3;

    tx = malloc(data_size);
    rx = malloc(data_size);

    if (tx == NULL || rx == NULL) {
        pabort("Failed to allocate memory for buffers");
    }

    // 데이터 버퍼 초기화 (예시로 패턴 생성)
    for (i = 0; i < data_size; i++) {
        tx[i] = i % 256;
        rx[i] = 0;
    }

    fd = open(device, O_RDWR);
    if (fd < 0)
        pabort("can't open device");

    ret = ioctl(fd, SPI_IOC_WR_MODE32, &mode);
    if (ret == -1)
        pabort("can't set spi mode");

    ret = ioctl(fd, SPI_IOC_RD_MODE32, &mode);
    if (ret == -1)
        pabort("can't get spi mode");

    ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret == -1)
        pabort("can't set bits per word");

    ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
    if (ret == -1)
        pabort("can't get bits per word");

    ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret == -1)
        pabort("can't set max speed hz");

    ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
    if (ret == -1)
        pabort("can't get max speed hz");

    printf("max speed: %d Hz (%d KHz) \n", speed, speed / 1000);

    if (spislave) {
        printf("SPI SLAVE TEST %d bytes one Shot ^^ \n", data_size);
        slave_transfer(fd, (uint16_t *)tx, (uint16_t *)rx, data_size, 1);
    } else {
        printf("SPI MASTER TEST START %d time \n", testCount);
        for (i = 0; i < testCount; i++) {
            printf("SPI MASTER TEST %d bytes data transfer \n", data_size);
            transfer(fd, (uint16_t *)tx, (uint16_t *)rx, data_size, 1);

            // 수신된 데이터가 송신된 데이터와 일치하는지 확인
            int flag = 0;
            for (int j = 0; j < data_size / 2; j++) {
                if (rx[j] != tx[j]) {
                    printf("Err: num %d [Sent: %x] [Received: %x] \n", j, tx[j], rx[j]);
                    flag = 1;
                }
            }
            if (!flag) {
                printf("SPI Read OK\n");
            } else {
                printf("SPI Read FAIL\n");
                // 송신한 데이터와 수신한 데이터 전체를 출력
                printf("Sent data:\n");
                hex_dump(tx, data_size, 16, "TX");
                printf("Received data:\n");
                hex_dump(rx, data_size, 16, "RX");
            }
        }
    close(fd);
	}

    free(tx);
    free(rx);
    printf("SPI TEST END \n");

    return ret;
}
