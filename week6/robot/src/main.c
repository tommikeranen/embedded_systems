// Tavoitteena on 1 piste
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>

// UART initialization
#define UART_DEVICE_NODE DT_CHOSEN(zephyr_shell_uart)
static const struct device *const uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);

// Error codes
#define TIME_LEN_ERROR -1
#define TIME_ARRAY_ERROR -2
#define TIME_VALUE_ERROR -3
// #define TIME_DIGIT_ERROR    -4
#define TIME_NULL_ERROR -5

int main(void)
{
        // UART initialization
        if (!device_is_ready(uart_dev))
        {
                printk("UART initialization failed!\r\n");
                return 0;
        }

        // Wait for everything to initialize and threads to start
        k_msleep(100);
        // Sanity check
        printk("Started serial led example\n");

        /************************************************
         * HOX! MUOKATKAA OMA UART_TASK TÄMÄN AO.
         * WHILE-LOOPIN MUKAISEKSI
         * (ei siis käytetä superlooppia)
         ***********************************************/
        char rc = 0;
        char uart_msg[20];
        int uart_msg_cnt = 0;
        memset(uart_msg, 0, 20);
        while (true)
        {
                // Ask UART if data available
                while (uart_poll_in(uart_dev, &rc) == 0)
                {
                        // Add characters into buffer until X is received
                        if (rc != 'X')
                        {
                                uart_msg[uart_msg_cnt] = rc;
                                uart_msg_cnt++;
                        }
                        else
                        {
                                // Send response to UART
                                // X has been removed, so we need to add it to the end
                                // printk("-1X"); example return error

                                // Example check with time_parse
                                int timer_delay = time_parse(uart_msg);
                                printk("%dX", timer_delay);

                                // Clear UART message buffer
                                uart_msg_cnt = 0;
                                memset(uart_msg, 0, 20);
                        }
                }
                k_msleep(10);
        }
        return 0;
}

int time_parse(char *time)
{
        // testi: palauttaa virhekoodin -1
        // return -1;

        // how many seconds, default returns error
        int seconds = TIME_LEN_ERROR;

        // TODO: Check that string is not null
        // if (time == nullptr)
        // {
        //         return TIME_NULL_ERROR;
        // }

        // additional test, check that time is 6 characters long
        if (strlen(time) != 6)
        {
                return TIME_LEN_ERROR;
        }

        // additional test, check that all characters are digits
        //     for (int i = 0; i < 6; ++i) {
        //         if (!isdigit((unsigned char)time[i])) {
        //             return TIME_DIGIT_ERROR;
        //         }
        //     }

        // Parse values from time string
        // For example: 124033 -> 12hour 40min 33sec
        int values[3];
        values[2] = atoi(time + 4); // seconds
        time[4] = 0;
        values[1] = atoi(time + 2); // minutes
        time[2] = 0;
        values[0] = atoi(time); // hours
                                // Now you have:
                                // values[0] hour
                                // values[1] minute
                                // values[2] second

        // TODO: Add boundary check time values: below zero or above limit not allowed
        // limits are 59 for minutes, 23 for hours, etc
        if (values[0] < 0 || values[0] > 23 ||
            values[1] < 0 || values[1] > 59 ||
            values[2] < 0 || values[2] > 59)
        {
                return TIME_VALUE_ERROR;
        }

        // TODO: Calculate return value from the parsed minutes and seconds
        // Otherwise error will be returned!
        // seconds = ...
        seconds = values[0] * 3600 + values[1] * 60 + values[2];

        // additional test, check that seconds is not zero or below
        if (seconds <= 0)
        {
                return TIME_VALUE_ERROR;
        }

        return seconds;
}