#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

/* Source is from i_main.c */
#include "config.h"
#include <stdio.h>
#include "SDL.h"
#include "doomtype.h"
#include "i_system.h"
#include "m_argv.h"

#include "esp_vfs_fat.h"
#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"
#include "odroid_display.h"
#include "odroid_sdcard.h"

#include <driver/dac.h>

// SDCARD
void install_sd_card(void);
#define SD_PIN_NUM_MISO 19
#define SD_PIN_NUM_MOSI 23
#define SD_PIN_NUM_CLK  18
#define SD_PIN_NUM_CS 22

// DISPLAY
void init_lcd_display(void);

// DAC
void init_dac(void);

void D_DoomMain(void);

void odroiddoommain()
{
    puts("I'm alive!");

    install_sd_card();
    init_lcd_display();
    init_dac();

    // Normal Doom Code...
    M_FindResponseFile();

    // start doom
    D_DoomMain ();
}

void app_main()
{
    // Doomtask -> Stacksize of 25600 Bytes (25 KB), pinned to core 0
    xTaskCreatePinnedToCore(&odroiddoommain, "doom_main_task", 22480, NULL, 5, NULL, 0);
}

void install_sd_card()
{
    // Prepare SD Card IO, print file test.txt if there for test purposes
    // We try to read DOOM.WAD of the Sd Card later on
    /* odroid_sdcard.c */
    odroid_sdcard_open("/sdcard");

    // Now try to read testfile, just for debug purposes, that is
    /*
    FILE* f = fopen("/sdcard/test.txt", "r");
    if (f != NULL) {
        puts("Reading test.txt:");
        char line[64];
	fgets(line, sizeof(line), f);
	fclose(f);
	puts(line);
    } else {
        puts("File test.txt not found");
    }
    */
}

void init_lcd_display()
{
    uint16_t* buffer;

    /* odroid_display.c */
    ili9341_init();
    ili9341_clear(0xffff);
    puts("Display init done");

    // 320 x 240 Pixel, 16bit each, Color: RRRRRGGG GGGBBBBB (RGB565)
    buffer = malloc(320*240*2);
    memset(buffer, 0, 320*240*2);

    puts("Drawing");
    // blue rectangle, 50x50 pixel, left upper corner
    for (short x = 0; x < 50; x++) {
        for (short y = 0; y < 50; y++) {
            buffer[x*320+y] = 0xFF00;
        }
    }

    ili9341_write_frame(buffer);
    puts("Drawing done");
}

void init_dac()
{
    dac_channel_t dac_channel = DAC_CHANNEL_1;
    gpio_num_t gpio_num = 26;

    dac_output_enable(dac_channel);
    dac_output_voltage(dac_channel, 255);

    gpio_pad_select_gpio(gpio_num);
    gpio_set_direction(gpio_num, GPIO_MODE_OUTPUT);
    for (int i = 0; i < 50; i++) {
        gpio_set_level(gpio_num, 0);
        // wait for x ms
        vTaskDelay(10 / portTICK_PERIOD_MS);
        gpio_set_level(gpio_num, 1);
        vTaskDelay(2 / portTICK_PERIOD_MS);
    }
}
