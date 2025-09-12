// Tavoitteena on 1 piste

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>

#include <string.h>
#include <stdlib.h>

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

// Create dispatcher FIFO buffer
K_FIFO_DEFINE(dispatcher_fifo);

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
	init_led();

	// Init UART
	int ret = init_uart();
	if (ret != 0)
	{
		printk("UART initialization failed!\n");
		return ret;
	}

	// for signal testing
	// k_msleep(1000);
	// k_condvar_broadcast(&red_signal);
	// k_msleep(1000);
	// k_condvar_broadcast(&yellow_signal);
	// k_msleep(1000);
	// k_condvar_broadcast(&green_signal);
	// k_msleep(1000);

	return 0;
}

// Task to handle red led
void red_led_task(void *, void *, void *)
{

	printk("Red led thread started\n");
	while (true)
	{
		k_condvar_wait(&red_signal, &thread_mutex, K_FOREVER);
		printk("Red\n");
		gpio_pin_set_dt(&green, 0);
		gpio_pin_set_dt(&red, 1);
	}
}

// Task to handle yellow led
void yellow_led_task(void *, void *, void *)
{

	printk("Yellow led thread started\n");
	while (true)
	{
		k_condvar_wait(&yellow_signal, &thread_mutex, K_FOREVER);
		printk("Yellow\n");
		gpio_pin_set_dt(&green, 1);
		gpio_pin_set_dt(&red, 1);
	}
}

// Task to handle green led
void green_led_task(void *, void *, void *)
{

	printk("Green led thread started\n");
	while (true)
	{
		k_condvar_wait(&green_signal, &thread_mutex, K_FOREVER);
		printk("Green\n");
		gpio_pin_set_dt(&green, 1);
		gpio_pin_set_dt(&red, 0);
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

		char color = sequence[0];

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
}