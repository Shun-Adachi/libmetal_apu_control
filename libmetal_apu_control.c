/**
 * libmetal_apu_control.c
 *
 * APU-side Linux application using libmetal and shared memory/IPI
 * to communicate with RPU (remote processor).
 *
 * Sets a control block with data size, pattern type, and repeat count,
 * then waits for RPU to write data and mark completion.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <metal/device.h>
#include <metal/io.h>
#include "sys_init.h"

#define SHM_DEV_NAME "3ed80000.shm"
#define IPI_DEV_NAME "ff340000.ipi"

#define SHM_SIZE        0x100000
#define CONTROL_OFFSET  0x0
#define DATA_OFFSET     0x100

#define PATTERN_FIXED   0
#define PATTERN_INC     1

struct control_block {
    uint32_t size;
    uint32_t pattern;
    uint32_t repeat;
    uint32_t status;  // 1=start request, 2=complete
};

static struct metal_device *shm_dev = NULL;
static struct metal_device *ipi_dev = NULL;
static struct metal_io_region *shm_io = NULL;
static struct metal_io_region *ipi_io = NULL;

/**
 * 割込み発行
 */
int send_ipi(void) {
    if (!ipi_io) return -1;
    metal_io_write32(ipi_io, 0, 1);
    return 0;
}

/**
 * 検証用データ生成
 */
int generate_expected(uint8_t *buf, uint32_t size, uint32_t pattern) {
    if (pattern == PATTERN_FIXED) {
        memset(buf, 0xAA, size);
    } else if (pattern == PATTERN_INC) {
        for (uint32_t i = 0; i < size; i++) {
            buf[i] = i & 0xFF;
        }
    } else {
        fprintf(stderr, "Unknown pattern: %u\n", pattern);
        return -1;
    }
    return 0;
}

/**
 * メイン
 */
int main(int argc, char *argv[]) {

    //------------------------------------
    // 引数のパーサーおよびバリデーション
    //------------------------------------
    if (argc != 3) {
        printf("Usage: %s <size> <pattern: 0=fixed, 1=inc>\n", argv[0]);
        return 1;
    }

    uint32_t size = atoi(argv[1]);
    uint32_t pattern = atoi(argv[2]);

    if (size > SHM_SIZE - DATA_OFFSET) {
        fprintf(stderr, "Error: size too large\n");
        return 1;
    }

    //------------------------------------
    // 初期化
    //------------------------------------
    printf("[APU] libmetal AMP test starting...\n");

    if (sys_init() != 0) {
        fprintf(stderr, "System init failed\n");
        return 1;
    }

    if (metal_device_open("platform", SHM_DEV_NAME, &shm_dev)) {
        fprintf(stderr, "Failed to open SHM device\n");
        return 1;
    }
    shm_io = metal_device_io_region(shm_dev, 0);

    if (metal_device_open("platform", IPI_DEV_NAME, &ipi_dev)) {
        fprintf(stderr, "Failed to open IPI device\n");
        return 1;
    }
    ipi_io = metal_device_io_region(ipi_dev, 0);

    //------------------------------------
    // RPUへ設定値送信および開始指示
    //------------------------------------
    struct control_block ctrl = {
        .size = size,
        .pattern = pattern,
        .repeat = 1,
        .status = 1
    };

    metal_io_write32(shm_io, CONTROL_OFFSET + 0, ctrl.size);
    metal_io_write32(shm_io, CONTROL_OFFSET + 4, ctrl.pattern);
    metal_io_write32(shm_io, CONTROL_OFFSET + 8, ctrl.repeat);
    metal_io_write32(shm_io, CONTROL_OFFSET + 12, ctrl.status);

    // IPI 発行
    send_ipi();

    // Wait for RPU to set status = 2
    while (metal_io_read32(shm_io, CONTROL_OFFSET + 12) != 2) {
        usleep(1000);
    }

    // Compare data in shared memory
    uint8_t expected[4096];
    uint8_t actual[4096];
    if (size > sizeof(expected)) {
        fprintf(stderr, "Internal buffer too small\n");
        return 1;
    }

    //------------------------------------
    // バリデーション
    //------------------------------------
    generate_expected(expected, size, pattern);
    for (uint32_t i = 0; i < size; i++) {
        actual[i] = metal_io_read8(shm_io, DATA_OFFSET + i);
    }

    if (memcmp(expected, actual, size) == 0) {
        printf("Data match! Test passed.\n");
    } else {
        printf("Data mismatch!\n");
        for (uint32_t i = 0; i < size; i++) {
            if (expected[i] != actual[i]) {
                printf("Mismatch at byte %u: expected 0x%02X, got 0x%02X\n",
                       i, expected[i], actual[i]);
                break;
            }
        }
    }

    //------------------------------------
    // クローズ処理
    //------------------------------------
    metal_device_close(shm_dev);
    metal_device_close(ipi_dev);
    sys_cleanup();
    return 0;
}
