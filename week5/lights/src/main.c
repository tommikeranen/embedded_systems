// Tavoitteena on 2 pistettä
// Tein kaikki 1 pisteeseen tarvittavat tehtävät ja lisäsin uusia testejä (löytyy week5/parser kansiosta)

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/timing/timing.h>

#include <string.h>
#include <stdlib.h>

#include <bits/stdc++.h> // for isdigit()

// Led pin configurations
static const struct gpio_dt_spec red = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec green = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const struct gpio_dt_spec blue = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);

// Led thread initialization
#define STACKSIZE 500
#define PRIORITY 5
void red_led_task(void *, void *, void *);
void yellow_led_task(void *, void *, void *);
void green_led_task(void *, void *, void *);

void uart_task(void *, void *, void *);
void dispatcher_task(void *, void *, void *);

K_THREAD_DEFINE(red_thread, STACKSIZE, red_led_task, NULL, NULL, NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(yellow_thread, STACKSIZE, yellow_led_task, NULL, NULL, NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(green_thread, STACKSIZE, green_led_task, NULL, NULL, NULL, PRIORITY, 0, 0);

K_THREAD_DEFINE(dis_thread, STACKSIZE, dispatcher_task, NULL, NULL, NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(uart_thread, STACKSIZE, uart_task, NULL, NULL, NULL, PRIORITY, 0, 0);

// UART initialization
#define UART_DEVICE_NODE DT_CHOSEN(zephyr_shell_uart)
static const struct device *const uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);

// Condition variables
K_MUTEX_DEFINE(thread_mutex);
K_CONDVAR_DEFINE(dispatcher_signal);
K_CONDVAR_DEFINE(red_signal);
K_CONDVAR_DEFINE(yellow_signal);
K_CONDVAR_DEFINE(green_signal);

// For storing timing data
static uint64_t red_ns = 0;
static uint64_t yellow_ns = 0;
static uint64_t green_ns = 0;

// Create dispatcher FIFO buffer
K_FIFO_DEFINE(dispatcher_fifo);

// Error codes
#define COMMAND_OK 	    	 0
#define TIME_LEN_ERROR      -1
#define TIME_ARRAY_ERROR    -2
#define TIME_VALUE_ERROR    -3
#define TIME_DIGIT_ERROR    -4
#define TIME_NULL_ERROR     -5

// Timer initializations
struct k_timer timer;
void timer_handler(struct k_timer *timer_id);


int time_parse(char *time);

// FIFO dispatcher data type
struct data_t
{
	void *fifo_reserved;
	char msg[20];
};

int init_uart(void)
{
	// UART initialization
	if (!device_is_ready(uart_dev))
	{
		return 1;
	}
	return 0;
}

// Initialize leds
int init_led()
{

	int ret;

	// Led pin initialization

	ret = gpio_pin_configure_dt(&red, GPIO_OUTPUT_ACTIVE);
	if (ret < 0)
	{
		printk("Error: Led configure failed\n");
		return ret;
	}
	// set led off
	gpio_pin_set_dt(&red, 0);

	ret = gpio_pin_configure_dt(&green, GPIO_OUTPUT_ACTIVE);
	if (ret < 0)
	{
		printk("Error: Led configure failed\n");
		return ret;
	}
	// set led off
	gpio_pin_set_dt(&green, 0);

	ret = gpio_pin_configure_dt(&blue, GPIO_OUTPUT_ACTIVE);
	if (ret < 0)
	{
		printk("Error: Led configure failed\n");
		return ret;
	}
	// set led off
	gpio_pin_set_dt(&blue, 0);

	printk("Led initialized ok\n");

	return 0;
}

// Main program
int main(void)
{
	timing_init();
	timing_start();
	timing_t start_time = timing_counter_get();

	init_led();

	// Init UART
	int ret = init_uart();
	if (ret != 0)
	{
		printk("UART initialization failed!\n");
		return ret;
	}

	timing_t end_time = timing_counter_get();
	// timing_stop();
	uint64_t timing_ns = timing_cycles_to_ns(timing_cycles_get(&start_time, &end_time));
	printk("Initialization: %lld\n", timing_ns);


	// Parser functionality
	char buffer[16];
	int cnt = 0;
	unsigned char c = 0;

	while (true) {

		if (uart_poll_in(uart_dev,&c) == 0) {
			if (c == '\n' || c == '\r') {
				printk(buffer);
				// here you call parser
				int ret = time_parse(buffer);
				// check parser return value
				if (ret > COMMAND_OK) {
					// send signal / message to mailbox
					// Timer initialization
					k_timer_init(&timer, timer_handler, NULL);
					k_timer_start(&timer, K_SECONDS(ret), NULL);
				} else if (ret == TIME_LEN_ERROR) {
					printk("Error: Time string length error\n");
				} else if (ret == TIME_VALUE_ERROR) {
					printk("Error: Time value error\n");
				} else if (ret == TIME_DIGIT_ERROR) {
					printk("Error: Time digit error\n");
				} else if (ret == TIME_NULL_ERROR) {
					printk("Error: Time null error\n");
				} else {
					printk("Error: Unknown error\n");
				}
				// clear buffer and counter
				cnt = 0;
				memset(buffer,0,16);
			} else {
				// add received character to buffer
				buffer[cnt] = c;
				cnt++;
			}
		}
		k_msleep(10);
	}

	return 0;
}

// Task to handle red led
void red_led_task(void *, void *, void *)
{

	printk("Red led thread started\n");
	while (true)
	{
		k_condvar_wait(&red_signal, &thread_mutex, K_FOREVER);

		timing_t start = timing_counter_get();

		// printk("Red\n");
		gpio_pin_set_dt(&green, 0);
		gpio_pin_set_dt(&red, 1);

		timing_t end = timing_counter_get();
		uint64_t ns = timing_cycles_to_ns(timing_cycles_get(&start, &end));
		printk("Red task timing: %llu µs\n", ns / 1000);

		k_mutex_lock(&thread_mutex, K_FOREVER);
		red_ns += ns;
		k_mutex_unlock(&thread_mutex);
	}
}

// Task to handle yellow led
void yellow_led_task(void *, void *, void *)
{

	printk("Yellow led thread started\n");
	while (true)
	{
		k_condvar_wait(&yellow_signal, &thread_mutex, K_FOREVER);

		timing_t start = timing_counter_get();

		// printk("Yellow\n");
		gpio_pin_set_dt(&green, 1);
		gpio_pin_set_dt(&red, 1);

		timing_t end = timing_counter_get();
		uint64_t ns = timing_cycles_to_ns(timing_cycles_get(&start, &end));
		printk("Yellow task timing: %llu µs\n", ns / 1000);

		k_mutex_lock(&thread_mutex, K_FOREVER);
		yellow_ns += ns;
		k_mutex_unlock(&thread_mutex);
	}
}

// Task to handle green led
void green_led_task(void *, void *, void *)
{

	printk("Green led thread started\n");
	while (true)
	{
		k_condvar_wait(&green_signal, &thread_mutex, K_FOREVER);

		timing_t start = timing_counter_get();

		// printk("Green\n");
		gpio_pin_set_dt(&green, 1);
		gpio_pin_set_dt(&red, 0);

		timing_t end = timing_counter_get();
		uint64_t ns = timing_cycles_to_ns(timing_cycles_get(&start, &end));
		printk("Green task timing: %llu µs\n", ns / 1000);

		k_mutex_lock(&thread_mutex, K_FOREVER);
		green_ns += ns;
		k_mutex_unlock(&thread_mutex);
	}
}

// UART task
void uart_task(void *, void *, void *)
{
	// Received character from UART
	char rc = 0;
	// Message from UART
	char uart_msg[20];
	memset(uart_msg, 0, 20);
	int uart_msg_cnt = 0;

	while (true)
	{
		// Ask UART if data available
		if (uart_poll_in(uart_dev, &rc) == 0)
		{
			// printk("Received: %c\n",rc);
			// If character is not newline, add to UART message buffer
			if (rc != '\r')
			{
				uart_msg[uart_msg_cnt] = rc;
				uart_msg_cnt++;
				// Character is newline, copy dispatcher data and put to FIFO buffer
			}
			else
			{
				printk("UART msg: %s\n", uart_msg);

				struct data_t *buf = k_malloc(sizeof(struct data_t));
				if (buf == NULL)
				{
					return;
				}

				strncpy(buf->msg, uart_msg, sizeof(buf->msg));
				buf->msg[sizeof(buf->msg) - 1] = '\0';
				k_fifo_put(&dispatcher_fifo, buf);

				// Clear UART receive buffer
				uart_msg_cnt = 0;
				memset(uart_msg, 0, 20);

				// Clear UART message buffer
				uart_msg_cnt = 0;
				memset(uart_msg, 0, 20);
			}
		}
		k_msleep(10);
	}
}

// Dispatcher task
void dispatcher_task(void *, void *, void *)
{
	while (true)
	{
		// Receive dispatcher data from uart_task fifo
		struct data_t *rec_item = k_fifo_get(&dispatcher_fifo, K_FOREVER);
		char sequence[20];
		memcpy(sequence, rec_item->msg, 20);
		k_free(rec_item);

		printk("Dispatcher: %s\n", sequence);

		k_mutex_lock(&thread_mutex, K_FOREVER);
		red_ns = yellow_ns = green_ns = 0;
		k_mutex_unlock(&thread_mutex);

		for (int i = 0; sequence[i] != '\0'; i++)
		{
			char color = sequence[i];

			k_mutex_lock(&thread_mutex, K_FOREVER);
			switch (color)
			{
			case 'r':
			case 'R':
				k_condvar_broadcast(&red_signal);
				break;
			case 'y':
			case 'Y':
				k_condvar_broadcast(&yellow_signal);
				break;
			case 'g':
			case 'G':
				k_condvar_broadcast(&green_signal);
				break;
			default:
				printk("Unknown color: %c\n", color);
				break;
			}
			k_mutex_unlock(&thread_mutex);

			k_msleep(1000); // led on for 1000ms

			// Turn off all leds
			gpio_pin_set_dt(&red, 0);
			gpio_pin_set_dt(&green, 0);
			gpio_pin_set_dt(&blue, 0);
		}

		k_mutex_lock(&thread_mutex, K_FOREVER);
		uint64_t total_ns = red_ns + yellow_ns + green_ns;
		k_mutex_unlock(&thread_mutex);

		printk("Total time for the sequence: %llu µs\n", total_ns / 1000);
	}
}

int time_parse(char *time) {

	// how many seconds, default returns error
	int seconds = TIME_LEN_ERROR;

	// TODO: Check that string is not null
    if (time == nullptr) {
        return TIME_NULL_ERROR;
    }

    // additional test, check that time is 6 characters long
    if (strlen(time) != 6) {
        return TIME_LEN_ERROR;
    }

	// additional test, check that all characters are digits
    for (int i = 0; i < 6; ++i) {
        if (!isdigit((unsigned char)time[i])) {
            return TIME_DIGIT_ERROR;
        }
    }

	// Parse values from time string
	// For example: 124033 -> 12hour 40min 33sec
    int values[3];
	values[2] = atoi(time+4); // seconds
	time[4] = 0;
	values[1] = atoi(time+2); // minutes
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
        values[2] < 0 || values[2] > 59) {
        return TIME_VALUE_ERROR;
    } 

	// TODO: Calculate return value from the parsed minutes and seconds
	// Otherwise error will be returned!
	// seconds = ...
    seconds = values[0] * 3600 + values[1] * 60 + values[2];

    // additional test, check that seconds is not zero or below
    if (seconds <= 0) {
        return TIME_VALUE_ERROR;
    }

	return seconds;
}