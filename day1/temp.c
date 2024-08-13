#include <time.h>  // 추가된 헤더 파일

// 위의 다른 코드들은 그대로 유지

#define MAX_DATA_SIZE 256

// 데이터 생성 함수
void generate_random_data(uint16_t *data, size_t size) {
    srand(time(NULL));  // 시드 설정
    for (size_t i = 0; i < size; i++) {
        data[i] = rand() % 0xFFFF;  // 0xFFFF 이하의 임의의 값 생성
    }
}

// main 함수 수정
int main(int argc, char *argv[])
{
    int ret = 0;
    int fd;
    spidev3_0 = 0;

    parse_opts(argc, argv);

    // 데이터 크기 설정 (최대 256)
    size_t data_size = (argc > 1) ? atoi(argv[1]) : FIFO_DEPTH_DEFINDED;
    if (data_size > MAX_DATA_SIZE) {
        printf("Error: Data size exceeds maximum allowed value of 256.\n");
        return -1;
    }

    uint16_t tx_data[data_size];
    uint16_t rx_data[data_size];

    generate_random_data(tx_data, data_size);

    mode = SPI_MODE_3;

    fd = open(device, O_RDWR);
    if (fd < 0)
        pabort("can't open device");

    /*
     * spi mode 설정
     */
    ret = ioctl(fd, SPI_IOC_WR_MODE32, &mode);
    if (ret == -1)
        pabort("can't set spi mode");

    ret = ioctl(fd, SPI_IOC_RD_MODE32, &mode);
    if (ret == -1)
        pabort("can't get spi mode");

    /*
     * bits per word 설정
     */
    ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret == -1)
        pabort("can't set bits per word");

    ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
    if (ret == -1)
        pabort("can't get bits per word");

    /*
     * max speed hz 설정
     */
    ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret == -1)
        pabort("can't set max speed hz");

    ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
    if (ret == -1)
        pabort("can't get max speed hz");

    printf("max speed: %d Hz (%d KHz) \n", speed, speed/1000);

    // 데이터 전송 및 수신
    master_transfer(fd, tx_data, rx_data, data_size * sizeof(uint16_t), 1);

    // 데이터 일치 확인
    int match = 1;
    for (size_t i = 0; i < data_size; i++) {
        if (tx_data[i] != rx_data[i]) {
            match = 0;
            break;
        }
    }

    // 결과 출력
    if (match) {
        printf("[SPI] OK\n");
    } else {
        printf("[SPI] FAIL\n");
        // 불일치한 데이터를 디버그 용도로 덤프
        hex_dump(tx_data, data_size * sizeof(uint16_t), 16, "TX");
        hex_dump(rx_data, data_size * sizeof(uint16_t), 16, "RX");
    }

    close(fd);
    return ret;
}
