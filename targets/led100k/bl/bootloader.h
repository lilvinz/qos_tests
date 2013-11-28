#ifndef BOOTLOADER_H_
#define BOOTLOADER_H_


#define BOOTLOADER_SHORTCUT         0


#define BOOTLOADER_SW_RESET_FLAG    0x10000000

//variables from our linker script
extern uint8_t _image_flash_begin[];
extern uint8_t _image_flash_end[];

#define MAIN_FLASH_SIZE (_image_flash_end - _image_flash_begin)
#define MAIN_BOOTLOADER_SIZE ((uint8_t*)IMAGE_INFO_BASE - _image_flash_begin)

// Status LEDs
#define STATUS_LED_PORT      	GPIOG
#define STATUS_LED_SIGNAL_PIN	GPIO_Pin_3
#define SIGNAL_LED_ON()         GPIO_WriteBit(STATUS_LED_PORT, STATUS_LED_SIGNAL_PIN , Bit_SET)
#define SIGNAL_LED_OFF()        GPIO_WriteBit(STATUS_LED_PORT, STATUS_LED_SIGNAL_PIN , Bit_RESET)

#define SYSTEM_PERFORM_RESET()                  (SCB->AIRCR = 0x05fa0004)
#define SYSTEM_RESET_REASON_INVOKE_BOOTLOADER   0x1

#endif
