#include <stdint.h>
#include "CH572SFR.h"

#define SLEEPTIME_MS 300

#define SYS_SAFE_ACCESS(a)  do { R8_SAFE_ACCESS_SIG = SAFE_ACCESS_SIG1; \
								 R8_SAFE_ACCESS_SIG = SAFE_ACCESS_SIG2; \
								 asm volatile ("nop\nnop"); \
								 {a} \
								 R8_SAFE_ACCESS_SIG = SAFE_ACCESS_SIG0; \
								 asm volatile ("nop\nnop"); } while(0)

// For debug writing to the debug interface.
#define DMDATA0 			   (*((PUINT32V)0xe0000380))

#define GPIO_Pin_9             (0x00000200)
#define GPIOA_ResetBits(pin)   (R32_PA_CLR |= (pin))
#define GPIOA_SetBits(pin)     (R32_PA_OUT |= (pin))
#define GPIOA_ModeCfg_Out(pin) R32_PA_PD_DRV &= ~(pin); R32_PA_DIR |= (pin)

typedef struct {
	volatile uint32_t CTLR;
	volatile uint32_t SR;
	union {
		volatile uint32_t CNT;
		volatile uint32_t CNTL;
	};
	uint8_t RESERVED[4];
	union {
		volatile uint32_t CMP;
		volatile uint32_t CMPL;
	};
	uint8_t RESERVED0[4];
} SysTick_Type;

#define CORE_PERIPH_BASE              (0xE0000000) /* System peripherals base address in the alias region */

#define SysTick_BASE                  (CORE_PERIPH_BASE + 0xF000)
#define SysTick                       ((SysTick_Type *) SysTick_BASE)
#define SysTick_SR_SWIE               (1 << 31)
#define SysTick_SR_CNTIF              (1 << 0)
#define SysTick_LOAD_RELOAD_Msk       (0xFFFFFFFF)
#define SysTick_CTLR_MODE             (1 << 4)
#define SysTick_CTLR_STRE             (1 << 3)
#define SysTick_CTLR_STCLK            (1 << 2)
#define SysTick_CTLR_STIE             (1 << 1)
#define SysTick_CTLR_STE              (1 << 0)

void Clock60MHz() {
	SYS_SAFE_ACCESS(
		R8_HFCK_PWR_CTRL |= RB_CLK_PLL_PON;
		R8_FLASH_CFG = 0x01;
		R8_FLASH_SCK |= 1<<4; //50M
		R8_CLK_SYS_CFG = (0x40 | 10); // 60MHz
	);
}

void DelayMs(int ms) {
	uint32_t targend = SysTick->CNTL + (ms * 60 * 1000); // 60MHz clock
	while( ((int32_t)( SysTick->CNTL - targend )) < 0 );
}

void blink(int n) {
	for(int i = n-1; i >= 0; i--) {
		GPIOA_ResetBits(GPIO_Pin_9);
		DelayMs(33);
		GPIOA_SetBits(GPIO_Pin_9);
		if(i) DelayMs(33);
	}
}

void char_debug(char c) {
	// this while is wasting clock ticks, but the easiest way to demo the debug interface
	while(DMDATA0 & 0xc0);
	DMDATA0 = 0x85 | (c << 8);
}

void print(char msg[], int size, int endl) {
	for(int i = 0; i < size; i++) {
		char_debug(msg[i]);
	}
	if(endl) {
		char_debug('\r');
		char_debug('\n');
	}
}

void print_bytes(uint8_t data[], int size) {
	char hex_digits[] = "0123456789abcdef";
	char hx[] = "0x00 ";
	for(int i = 0; i < size; i++) {
		hx[2] = hex_digits[(data[i] >> 4) & 0x0F];
		hx[3] = hex_digits[data[i] & 0x0F];
		print(hx, 5, /*endl*/FALSE);
	}
	print(0, 0, /*endl*/TRUE);
}

#define MSG "~ ch570 ~"
int main(void) {
	Clock60MHz();
	GPIOA_ModeCfg_Out(GPIO_Pin_9);
	GPIOA_SetBits(GPIO_Pin_9);
	SysTick->CNTL = 0;
	SysTick->CMP = SysTick_LOAD_RELOAD_Msk -1;
	SysTick->CTLR = SysTick_CTLR_STRE  |
					SysTick_CTLR_STCLK |
					SysTick_CTLR_STIE  |
					SysTick_CTLR_STE; /* Enable SysTick IRQ and SysTick Timer */

	blink(5);
	print(MSG, sizeof(MSG), TRUE);

	while(1) {
		DelayMs(SLEEPTIME_MS -33);
		blink(1); // 33 ms
	}
}
