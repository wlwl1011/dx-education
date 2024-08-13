/*
 * SPI testing utility (using spidev driver)
 *
 * Copyright (c) 2007  MontaVista Software, Inc.
 * Copyright (c) 2007  Anton Vorontsov <avorontsov@ru.mvista.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * Cross-compile with cross-gcc -I/path/to/cross-kernel/include
 */

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
#define FIFO_DEPTH_DEFINDED	(32)
#define DEFAULT_DATA_SIZE (256)

static void pabort(const char *s)
{
	perror(s);
	abort();
}

static const char *device = "/dev/spidev4.0";
static uint32_t mode;
static uint8_t bits = 16;
static uint32_t speed =25000000;// 22000000; 	//18750000;//18 750 000;
static uint16_t delay;
static int verbose;
static int spidev3_0;
static int spislave;
int testCount = 1;
static int data_size = DEFAULT_DATA_SIZE;


#if (0)
uint8_t default_tx[32] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0x40, 0x00, 0x00, 0x00, 0x00, 0x95,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xF0, 0x0D
};
#endif
#if (FIFO_DEPTH_DEFINDED == 32)
uint16_t default_tx[FIFO_DEPTH_DEFINDED] = {
	0xAAA1, 0xAAA2, 0xAAA3, 0xFF, 0xFF, 0xFF,
	0x4040, 0x00, 0x00, 0x00, 0x00, 0x0F,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xF0, 0xD0
};
#endif
#if (FIFO_DEPTH_DEFINDED == 16)
uint16_t default_tx[FIFO_DEPTH_DEFINDED] = {
	0xAAA1, 0xAAA2, 0xAAA3, 0xFF, 0xFF, 0xFF,
	0x4040, 0x00, 0x00, 0x00, 0x00, 0x0F,
	0xFF, 0xFF, 0xFF, 0xD0
};
#endif

uint8_t read_eerom_chip_id_tx[] = {
	0x90, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
};

uint8_t read_bmi120_chip_id_tx[] = {
	0x80, 0x00,
};

uint8_t spislave_tx[] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x40, 0x00,
	0x00, 0x00, 0x00, 0x95,0xFF, 0xFF, 0xFF, 0xFF,
};

uint16_t default_rx[ARRAY_SIZE(default_tx)] = {0, };
uint8_t read_eerom_chip_id_rx[ARRAY_SIZE(read_eerom_chip_id_tx)] = {0, };
uint8_t read_bmi120_chip_id_rx[ARRAY_SIZE(read_bmi120_chip_id_tx)] = {0, };
uint8_t read_spislave_rx[ARRAY_SIZE(spislave_tx)] = {0, };

char *input_tx;

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
			printf(" | ");  /* right close */
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

/*
 *  Unescape - process hexadecimal escape character
 *      converts shell input "\x23" -> 0x23
 */
static int unescape(char *_dst, char *_src, size_t len)
{
	int ret = 0;
	char *src = _src;
	char *dst = _dst;
	unsigned int ch;

	while (*src) {
		if (*src == '\\' && *(src+1) == 'x') {
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

static void master_transfer(int fd, uint16_t const *tx, uint16_t const *rx, size_t len, int dump_flag)
{
	int ret;

	printf("master transfer... \n");
	struct spi_ioc_transfer tr_transfer = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = 0,
		.len = len,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	if (mode & SPI_TX_QUAD)
		tr_transfer.tx_nbits = 4;
	else if (mode & SPI_TX_DUAL)
		tr_transfer.tx_nbits = 2;
	if (mode & SPI_RX_QUAD)
		tr_transfer.rx_nbits = 4;
	else if (mode & SPI_RX_DUAL)
		tr_transfer.rx_nbits = 2;
	if (!(mode & SPI_LOOP)) {
		if (mode & (SPI_TX_QUAD | SPI_TX_DUAL))
			tr_transfer.rx_buf = 0;
		else if (mode & (SPI_RX_QUAD | SPI_RX_DUAL))
			tr_transfer.tx_buf = 0;
	}

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr_transfer);
	if (ret < 1)
		pabort("can't send spi message");
	// if (len != FIFO_DEPTH_DEFINDED*2)
	// 	printf("length : %d\n", len);
	if (dump_flag){
		hex_dump(tx, len, 64, "TX");
	}

	printf("master recieved... \n");

	struct spi_ioc_transfer tr_recived = {
		.tx_buf = 0,
		.rx_buf = (unsigned long)rx,
		.len = len,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	if (mode & SPI_TX_QUAD)
		tr_recived.tx_nbits = 4;
	else if (mode & SPI_TX_DUAL)
		tr_recived.tx_nbits = 2;
	if (mode & SPI_RX_QUAD)
		tr_recived.rx_nbits = 4;
	else if (mode & SPI_RX_DUAL)
		tr_recived.rx_nbits = 2;
	if (!(mode & SPI_LOOP)) {
		if (mode & (SPI_TX_QUAD | SPI_TX_DUAL))
			tr_recived.rx_buf = 0;
		else if (mode & (SPI_RX_QUAD | SPI_RX_DUAL))
			tr_recived.tx_buf = 0;
	}

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr_recived);
	if (ret < 1)
		pabort("can't send spi message");
	// if (len != FIFO_DEPTH_DEFINDED*2)
	// 	printf("length : %d\n", len);
	if (dump_flag){
		hex_dump(tx, len, 64, "RX");
	}
}


static void slave_transfer(int fd, uint16_t const *rx, size_t len, int dump_flag)
{
	int ret;

	printf("slave recieved... \n");

	struct spi_ioc_transfer tr_recieve = {
		.tx_buf = 0,
		.rx_buf = (unsigned long)rx,
		.len = len,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	if (mode & SPI_TX_QUAD)
		tr_recieve.tx_nbits = 4;
	else if (mode & SPI_TX_DUAL)
		tr_recieve.tx_nbits = 2;
	if (mode & SPI_RX_QUAD)
		tr_recieve.rx_nbits = 4;
	else if (mode & SPI_RX_DUAL)
		tr_recieve.rx_nbits = 2;
	if (!(mode & SPI_LOOP)) {
		if (mode & (SPI_TX_QUAD | SPI_TX_DUAL))
			tr_recieve.rx_buf = 0;
		else if (mode & (SPI_RX_QUAD | SPI_RX_DUAL))
			tr_recieve.tx_buf = 0;
	}

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr_recieve);
	if (ret < 1)
		pabort("can't send spi message");
	// if (len != FIFO_DEPTH_DEFINDED*2)
	// 	printf("length : %d\n", len);
	if (dump_flag){
		hex_dump(rx, len, 64, "RX");
	}

	printf("slave transfer... \n");
	struct spi_ioc_transfer tr_transfer = {
		.tx_buf = (unsigned long)rx,
		.rx_buf = 0,
		.len = len,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	if (mode & SPI_TX_QUAD)
		tr_transfer.tx_nbits = 4;
	else if (mode & SPI_TX_DUAL)
		tr_transfer.tx_nbits = 2;
	if (mode & SPI_RX_QUAD)
		tr_transfer.rx_nbits = 4;
	else if (mode & SPI_RX_DUAL)
		tr_transfer.rx_nbits = 2;
	if (!(mode & SPI_LOOP)) {
		if (mode & (SPI_TX_QUAD | SPI_TX_DUAL))
			tr_transfer.rx_buf = 0;
		else if (mode & (SPI_RX_QUAD | SPI_RX_DUAL))
			tr_transfer.tx_buf = 0;
	}

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr_transfer);
	if (ret < 1)
		pabort("can't send spi message");
	// if (len != FIFO_DEPTH_DEFINDED*2)
	// 	printf("length : %d\n", len);
	if (dump_flag){
		hex_dump(rx, len, 64, "RX");
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
	     "  -p            Send data (e.g. \"1234\\xde\\xad\")\n"
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
			{ "device",  1, 0, 'D' },
			{ "speed",   1, 0, 's' },
			{ "delay",   1, 0, 'd' },
			{ "bpw",     1, 0, 'b' },
			{ "loop",    0, 0, 'l' },
			{ "cpha",    0, 0, 'H' },
			{ "cpol",    0, 0, 'O' },
			{ "lsb",     0, 0, 'L' },
			{ "cs-high", 0, 0, 'C' },
			{ "3wire",   0, 0, '3' },
			{ "no-cs",   0, 0, 'N' },
			{ "ready",   0, 0, 'R' },
			{ "dual",    0, 0, '2' },
			{ "verbose", 0, 0, 'v' },
			{ "spidev3.0", 0, 0, 'i' },
			{ "spislave", 0, 0, 'S' },
			{ "quad",    0, 0, '4' },
			{ NULL, 0, 0, 0 },
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
	uint8_t *tx;
	uint8_t *rx;
	int size;
	spidev3_0 = 0;

	parse_opts(argc, argv);

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


	mode = SPI_MODE_3;

	if (argv[1])
		testCount = atoi(argv[1]);

#if (1)
	fd = open(device, O_RDWR);
	if (fd < 0)
		pabort("can't open device");

	/*
	 * spi mode
	 */
	ret = ioctl(fd, SPI_IOC_WR_MODE32, &mode);
	if (ret == -1)
		pabort("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE32, &mode);
	if (ret == -1)
		pabort("can't get spi mode");

//	printf("spi mode: 0x%x\n", mode);

	/*
	 * bits per word
	 */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't set bits per word");

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't get bits per word");

//	printf("bits per word: %d\n", bits);

	/*
	 * max speed hz
	 */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't get max speed hz");

	printf("max speed: %d Hz (%d KHz) \n", speed, speed/1000);
	close(fd);
#endif
	if (spislave){
		fd = open(device, O_RDWR);
		if (fd < 0)
			pabort("can't open device");
		printf("SPI SLAVE TEST 32 bytes one Shot ^^ \n");
		slave_transfer(fd, rx, sizeof(rx), 1);
	}else {
		printf("SPI SLAVE TEST START  %d time \n", testCount);
		printf("SPI SLAVE TEST %d X 2(uint16) Bytes  %d bits word [%d] ea Shot \n",data_size , bits ,DATA_EA);
		fd = open(device, O_RDWR);
		for (i = 0; i < DATA_EA; i++){
//			fd = open(device, O_RDWR);
			if (fd < 0)
				pabort("can't open device");

			if (input_tx) {
				size = strlen(input_tx+1);
				tx = malloc(size);
				rx = malloc(size);
				size = unescape((char *)tx, input_tx, size);
				master_transfer(fd, tx, rx, size, 1);
				free(rx);
				free(tx);
			} else {
				if (spidev3_0) {
					printf("BMI120 Chip ID Read \n");
					master_transfer(fd, read_bmi120_chip_id_tx, read_bmi120_chip_id_rx, sizeof(read_bmi120_chip_id_tx), 1);
				} else {

			if (spislave){
				printf("SPI SLAVE TEST 32 bytes one Shot \n");
				slave_transfer(fd,  rx, sizeof(tx), 1);
			}
			else{
//						printf("SPI SLAVE TEST START %d time \n", testCount);
//						printf("SPI SLAVE TEST %d X 2(uint16) Bytes  %d bits word [%d] ea Shot \n",FIFO_DEPTH_DEFINDED , bits ,DATA_EA);
						master_transfer(fd, default_tx, default_rx, sizeof(tx), 0);
						int flag = 0;
						for (int j = 0; j < data_size; j++) {
							if (rx[j] != tx[j]) {
								printf("Err: num %d [Sent: %x] [Received: %x] \n", j, tx[j], rx[j]);
								flag = 1;
							}
						}
					}
				}
			}
//			close(fd);
		}
	close(fd);
	}
	printf("SPI SLAVE TEST END \n");
	return ret;
}
