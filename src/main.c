#include <driver/gpio.h> // for input
#include <esp_timer.h>   // for hardware timer

#include <stdio.h>  // c library
#include <stdlib.h> // c library

#include "fonts.h"
#include "graphics.h"

#include "nvs_flash.h" // for high score
#include "nvs.h"       // for high score

#include <freertos/FreeRTOS.h>
#include <freertos/task.h> //for delay

bool isPlaying = false;

// from Demo to handle flash storage...
nvs_handle_t storage_open(nvs_open_mode_t mode)
{
    esp_err_t err;
    nvs_handle_t my_handle;
    err = nvs_open("storage", mode, &my_handle);
    if (err != 0)
    {
        nvs_flash_init();
        err = nvs_open("storage", mode, &my_handle);
        printf("err1: %d\n", err);
    }
    return my_handle;
}

int storage_read_int(char *name, int def)
{
    nvs_handle_t handle = storage_open(NVS_READONLY);
    int32_t val = def;
    nvs_get_i32(handle, name, &val);
    nvs_close(handle);
    return val;
}
void storage_write_int(char *name, int val)
{
    nvs_handle_t handle = storage_open(NVS_READWRITE);
    nvs_set_i32(handle, name, val);
    nvs_commit(handle);
    nvs_close(handle);
}

// check collision
bool collides(float playerx, float playery, float playerw, float playerh, float blockx, float blocky, float blockw, float blockh)
{
    return (playerx + playerw > blockx && playerx < blockx + blockw && playery + playerh > blocky && playery < blocky + blockh);
}

void background()
{ // only move while playing
    cls(rgbToColour(32, 32, 32));
    if (!isPlaying)
    {
        for (int i = 0; i < 100; i++)
        {
            draw_line(rand() % display_width, rand() % display_height, rand() % display_width, rand() % display_height, rgbToColour(64, 64, 64));
        }
    }
    else
    {
        draw_line(rand() % display_width, rand() % display_height, rand() % display_width, rand() % display_height, rgbToColour(64, 64, 64));
    }
}

void miniGame()
{
    // initial seeting of button
    gpio_set_direction(0, GPIO_MODE_DEF_INPUT);  // Left button -- defult
    gpio_set_direction(35, GPIO_MODE_DEF_INPUT); // Right button

    // obj struct (class with NO method)
    typedef struct obj
    {
        float x, y, v;
        int w, h;
        bool isDrew;
    } obj;

    // for time counting
    uint64_t last_time = esp_timer_get_time(); // update last time = get real time
    float spawnInterval = 2.0;                 // in sec
    float timeFromLastSpawn = spawnInterval; // spawn when start

    // score related
    static char score_str[256];
    int score = 0;
    static int high_score = 0;
    high_score = storage_read_int("highscore", 0);

    // player initial setting
    obj player;
    player.w = 30;
    player.h = 10;
    player.x = (display_width - player.w) * 0.5;
    player.y = display_height - player.h;
    player.v = 100;

    isPlaying = true;
    bool isGameOver = false;

    // falling block initial setting
    int blockMax = 20; // max of 20 blocks on the screen
    int drewBlock = 0;
    obj block[blockMax];
    for (int i = 0; i < blockMax; i++)
    {
        block[i].w = 5;
        block[i].h = 10;
        block[i].x = rand() % (display_width - block[i].w);
        block[i].y = 0;
        block[i].v = 50;
        block[i].isDrew = false;
    }

    while (1)
    {   // for time delay
        uint64_t current_time = esp_timer_get_time();
        float dt = (current_time - last_time) * 0.000001; // in sec
        last_time = current_time;

        background();

        setFont(FONT_DEJAVU18);
        setFontColour(160, 160, 160);
        gprintf("Score: %d\n", score); // score on the top left

        // draw blocks
        draw_rectangle(player.x, player.y, player.w, player.h, rgbToColour(192, 192, 192));
        player.isDrew = true;

        if (timeFromLastSpawn >= spawnInterval)
        {
            if (!block[drewBlock].isDrew && (drewBlock < blockMax))
            {
                block[drewBlock].isDrew = true;
                drewBlock++;
                timeFromLastSpawn = 0.0;
            }
        }

        timeFromLastSpawn += dt;
        for (int i = 0; i < blockMax; i++)
        {
            if (block[i].isDrew)
            {
                draw_rectangle(block[i].x, block[i].y, block[i].w, block[i].h, rgbToColour(255, 255, 255));
            }
        }

        spawnInterval += 0.000001 * dt; // increase spawn rate(number of spawn blocks) and block speed over time
        for (int i = 0; i < blockMax; i++)
        {
            block[i].v *= 1.0005; // increase speed for each blocks over time
        }

        flip_frame();

        // update player
        if (!gpio_get_level(0) && gpio_get_level(35))
        {
            // left button
            player.x -= player.v * dt;
            if (player.x < 0)
            {
                player.x = 0;
            }
        }

        if (!gpio_get_level(35) && gpio_get_level(0))
        { // right button
            player.x += player.v * dt;
            if (player.x > (display_width - player.w))
            {
                player.x = display_width - player.w;
            }
        }

        for (int i = 0; i < blockMax; i++)
        {
            if (block[i].isDrew)
            {
                block[i].y += block[i].v * dt;

                if (block[i].y > display_height)
                {
                    score += 100;
                    if (score > high_score) // update high score
                    {
                        high_score = score;
                        storage_write_int("highscore", score);
                    }
                    block[i].y = 0.0; // redraw block
                    block[i].x = rand() % (display_width - block[i].w);
                }

                if (collides(player.x, player.y, player.w, player.h, block[i].x, block[i].y, block[i].w, block[i].h))
                {
                    isGameOver = true;
                    isPlaying = false;
                    break; // break the for loop
                }
            }
        }

        if (isGameOver)
        {
            while (1)
            {
                isPlaying = false;
                background();
                setFont(FONT_DEJAVU18);
                setFontColour(160, 160, 160);
                print_xy("Game Over", CENTER, 60);
                snprintf(score_str, 64, "Score: %d", score);
                print_xy(score_str, CENTER, 100);
                snprintf(score_str, 64, "HiScore: %d\n", high_score);
                print_xy(score_str, CENTER, 120);
                print_xy("Press Left to", CENTER, 155);
                print_xy("RESTART", CENTER, 185);
                flip_frame();
                vTaskDelay(100); // show the score for at least 1 sec
                break; 
            }
            break; // end of mini game (break the while loop)
        }
    }
}

void startWithInstruction()
{
    background();
    setFont(FONT_DEJAVU18);
    setFontColour(160, 160, 160);

    print_xy("Instructions:", CENTER, 20);
    print_xy("Move with", CENTER, 50);
    print_xy("Left/Right", CENTER, 70);
    print_xy("Buttons", CENTER, 95);
    print_xy("Avoid crashing", CENTER, 115);
    print_xy("with the ", CENTER, 135);
    print_xy("falling blocks!", CENTER, 155);
    print_xy("Press Left to", CENTER, 190);
    print_xy("START ", CENTER, 210);
    flip_frame();

    while (1)
    {
        if (!gpio_get_level(0))
        { // left button pressed
            miniGame();
        }
    }
}

void app_main()
{
    graphics_init();
    set_orientation(PORTRAIT);
    startWithInstruction();
}
