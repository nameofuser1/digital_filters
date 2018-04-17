### First-in-first-out module

#### Usage:
```C
static FIFO8_t UART1_rx_fifo;
static uint8_t UART1_rx_buf[10];
static char demo[] = "0123456789ABCDEF";
static char print_buffer[255] = {0};
static uint16_t brw = 0;

int main(void) 
{
	FIFO8_init(&UART1_rx_fifo, UART1_rx_buf, 10, FIFO8_LOOP);
	FIFO8_write(&UART1_rx_fifo,demo,15, brw);
	
	FIFO8_read(&UART1_rx_fifo,print_buffer,5, brw);
	printf("%.10s\n", print_buffer);
	
	FIFO8_write(&UART1_rx_fifo,demo,15, brw);
	
	FIFO8_read(&UART1_rx_fifo,&print_buffer[10],10, brw);
	printf("%.10s\n", &print_buffer[10]);
	
	return 0;
}
```