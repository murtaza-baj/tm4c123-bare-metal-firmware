#ifndef GPIO_H_
#define GPIO_H_

void gpio_init(void);
void gpio_write(uint32_t pin_mask, uint8_t val);

#endif /* GPIO_H_ */
