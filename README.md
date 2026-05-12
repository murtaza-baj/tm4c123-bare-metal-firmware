# TM4C123GH6PM Bare-Metal Firmware Portfolio

Bare-metal embedded firmware for the TM4C123GH6PM (ARM Cortex-M4F). Register-level drivers with no HAL — UART, I2C, SPI, GPIO, timers. Progresses through interrupt-driven communication, FreeRTOS task architecture, sensor data logging, and a UART bootloader.

---

## Hardware

**Board:** TI Tiva C Series TM4C123GH6PM LaunchPad (EK-TM4C123GXL)
**MCU:** TM4C123GH6PM — ARM Cortex-M4F, 80MHz, 256KB flash, 32KB SRAM
**Toolchain:** arm-none-eabi-gcc, TI Code Composer Studio
**Debugger:** Onboard ICDI (In-Circuit Debug Interface)

---

## Repository structure

```
tm4c123-bare-metal-firmware/
├── phase1_gpio/          — Register-level GPIO driver, RGB LED, button interrupt
├── phase2_uart/          — PLL 80MHz config, bare-metal UART polling driver
├── phase3_shell/         — Interrupt-driven RX, ring buffer, FSM parser, dispatch table
├── phase4_sensor_logger/ — I2C + SPI drivers, MPU-6050 + BMP280, timer ISR, data logging
├── phase5_rtos/          — FreeRTOS pipeline, queues, watchdog, HardFault handler
└── phase6_bootloader/    — UART bootloader, flash write, CRC-16, custom linker script
```

Each phase builds directly on the previous. Each is a self-contained, buildable CCS project.

---

## Phase 1 — GPIO Driver

**Status:** Complete
**Files:** `phase1_gpio/main.c`, `phase1_gpio/tm4c_reg_defs.h`, `phase1_gpio/gpio.c`,
`phase1_gpio/gpio.h`, `phase1_gpio/system.c`, `phase1_gpio/system.h`

### What it does

- Configures the PLL to run at 80MHz from the onboard 16MHz crystal
- Drives the onboard RGB LED (PF1=red, PF2=blue, PF3=green) through direct register writes
- Cycles the LED through red → blue → green → off in a continuous pattern
- SysTick timer generates a 1ms interrupt used for all timing
- SW1 (PF4) triggers a GPIO interrupt with 50ms software debounce — toggles blink period between 1000ms and 200ms

### File structure

| File | Purpose |
|------|---------|
| `tm4c_reg_defs.h` | All hardware register addresses and bit definitions |
| `system.c/h` | PLL initialisation, SysTick configuration |
| `gpio.c/h` | GPIO pin init, interrupt configuration |
| `main.c` | Application logic, ISR implementations, main loop |
| `startup_tm4c.c` | Vector table — `systick_ISR` and `pb_ISR` wired directly |

### Technical approach

All register access is through `volatile unsigned long *` pointer casts — consistent with TI's official register definitions for this target. No TivaWare, no HAL. The initialisation sequence follows the exact order from the TM4C123GH6PM datasheet.

### PLL configuration — 80MHz

The TM4C PLL produces 400MHz divided to the target system frequency. The configuration sequence follows Section 5.3 of the datasheet:

1. Enable `BYPASS` in RCC and RCC2 — run from oscillator during reconfiguration
2. Set `USERCC2` in RCC2 — use extended RCC2 divisor control
3. Clear and set XTAL field in RCC to `0x15` (16MHz crystal)
4. Clear OSCSRC2 field in RCC2 and clear `PWRDN2` to power up the PLL
5. Set `DIV400` in RCC2 — use 400MHz PLL output
6. Clear the 7-bit `SYSDIV2:SYSDIV2LSB` field at bits [28:22] then write `4` — 400MHz / (4+1) = 80MHz
7. Set `USESYSDIV` in RCC — enable the system clock divider
8. Poll `SYSCTL_RIS` bit 6 (`PLLLRIS`) until PLL locks
9. Clear `BYPASS2` in RCC2 — switch system clock to PLL

When `DIV400` is set the effective divisor field is the 7-bit `SYSDIV2:SYSDIV2LSB` spanning bits [28:22]. The field is cleared with `&= ~(0x7F << 22)` before writing to prevent residual bits from producing an incorrect clock frequency.

### SysTick — 1ms timebase

At 80MHz a 1ms interrupt requires a reload value of 79999 (80,000 counts, 0-indexed). The control register is written using named bit macros — `NVIC_ST_CTRL_ENABLE`, `NVIC_ST_CTRL_INTEN`, `NVIC_ST_CTRL_CLK_SRC`. `systick_ISR` increments `volatile uint32_t timer_count`. `wait(ms)` uses the subtraction pattern `(get_time_ms() - start) < ms` which handles 32-bit counter rollover correctly.

### GPIO initialisation sequence

Follows Section 10.3 of the datasheet:

1. Enable Port F clock via `SYSCTL_RCGCGPIO` bit 5
2. Two read barriers on `RCGCGPIO` — allows clock to stabilise before register access
3. Set output pins (PF1, PF2, PF3) in `GPIODIR`
4. Clear alternate function select (`GPIOAFSEL`) for all used pins
5. Set 2mA drive strength (`GPIODR2R`) for output pins
6. Enable pull-up on PF4 (`GPIOPUR`) — SW1 is active low
7. Enable digital function (`GPIODEN`) for all used pins

The `GPIODEN` step is mandatory on TM4C — all pins default to analog/high-impedance. Without it writes to the data register have no effect on the physical pin.

### GPIO interrupt — SW1 on PF4

1. Mask the interrupt (`GPIOIM`) before configuring
2. Configure edge-sensitive, single-edge, falling-edge detection (`GPIOIS`, `GPIOIBE`, `GPIOIEV`)
3. Clear any pending interrupt (`GPIOICR`)
4. Unmask the interrupt (`GPIOIM`)
5. Enable GPIO Port F in NVIC EN0 register — Port F is IRQ 30

`pb_ISR` checks `GPIOMIS` to confirm PF4 is the source, clears via `GPIOICR`, applies 50ms debounce using the SysTick counter, and sets `volatile uint8_t pb_flag` for the main loop.

### The TM4C address-masked DATA register

The TM4C GPIO DATA register uses hardware address masking (Section 10.2.1). Bits [9:2] of the access address act as a pin mask. The all-pins address (base + `0x3FC`) accesses all 8 pins simultaneously. For targeted access: `address = GPIO_BASE + (pin_mask << 2)`.

### Key register addresses (Port F, base 0x40025000)

| Register | Offset | Address | Purpose |
|----------|--------|---------|---------|
| GPIODATA | 0x3FC | 0x400253FC | Data (all pins) |
| GPIODIR | 0x400 | 0x40025400 | Direction |
| GPIOIS | 0x404 | 0x40025404 | Interrupt sense |
| GPIOIBE | 0x408 | 0x40025408 | Interrupt both edges |
| GPIOIEV | 0x40C | 0x4002540C | Interrupt event |
| GPIOIM | 0x410 | 0x40025410 | Interrupt mask |
| GPIOMIS | 0x418 | 0x40025418 | Masked interrupt status |
| GPIOICR | 0x41C | 0x4002541C | Interrupt clear |
| GPIOAFSEL | 0x420 | 0x40025420 | Alternate function select |
| GPIODR2R | 0x500 | 0x40025500 | 2mA drive select |
| GPIOPUR | 0x510 | 0x40025510 | Pull-up enable |
| GPIODEN | 0x51C | 0x4002551C | Digital enable |

SYSCTL: `RCGCGPIO` at `0x400FE608`, `RCC` at `0x400FE060`, `RCC2` at `0x400FE070`, `RIS` at `0x400FE050`.

---

## Phase 2 — UART Polling Driver

**Status:** Complete
**Files:** `phase2_uart/main.c`, `phase2_uart/tm4c_reg_defs.h`, `phase2_uart/system.c`,
`phase2_uart/uart.c`, `phase2_uart/uart.h`, `phase2_uart/gpio.c`, `phase2_uart/gpio.h`

### What it does

- Implements a complete bare-metal UART0 driver at 115200 baud 8N1 — no TivaWare
- TX polling — waits for TX FIFO space before writing each byte
- RX polling — blocks until a byte arrives in the RX FIFO
- Responds to single-character commands over a serial terminal: `r/g/b` control the RGB LED, `x` turns all LEDs off, `s` prints system status
- Uptime reported as a 32-bit hex value using a manual nibble-to-hex converter — no sprintf
- Button interrupt and LED control from Phase 1 carried forward unchanged

### File structure

| File | Purpose |
|------|---------|
| `tm4c_reg_defs.h` | Extended with UART0 and Port A register definitions |
| `system.c/h` | Unchanged from Phase 1 |
| `gpio.c/h` | Extended with `gpio_write()` — pin write separated from timing |
| `uart.c/h` | UART0 init, polling TX/RX, hex formatter, case converter |
| `main.c` | Command dispatcher using switch/case, ISRs, application loop |
| `startup_tm4c.c` | Unchanged from Phase 1 |

### Baud rate calculation

From Section 14.3.2 of the datasheet:

```
BRD = f_clk / (16 × baud_rate)
    = 80,000,000 / (16 × 115200)
    = 43.40278

UARTIBRD = 43
UARTFBRD = round(0.40278 × 64) = 26
```

Actual baud rate: 115,207. Error: 0.006% — well within the 3% UART spec maximum.
Integer-only divisor (IBRD=43, FBRD=0) gives 116,279 baud, error 0.94%.

### UART0 initialisation sequence

Follows the 7-step sequence from Section 14.4 exactly. Order is mandatory:

1. Enable UART0 clock via `SYSCTL_RCGCUART` bit 0
2. Enable Port A clock via `SYSCTL_RCGCGPIO` bit 0
3. Two read barriers per clock — read the just-written register twice
4. Configure PA0 and PA1 alternate function (`GPIOAFSEL`)
5. Enable digital function on PA0 and PA1 (`GPIODEN`)
6. Set `GPIOPCTL` nibbles for PA0/PA1 to function 1 (U0RX/U0TX) — write `0x11` after clearing `0xFF`
7. Disable UART — write 0 to `UARTCTL`
8. Write `UARTIBRD` = 43
9. Write `UARTFBRD` = 26
10. Write `UARTLCRH` = `WLEN | FEN` = 0x70 (8-bit, FIFOs enabled, no parity, 1 stop bit)
11. Enable — write `UARTEN | TXE | RXE` to `UARTCTL`

Writing `UARTLCRH` before the baud registers latches wrong values into the baud rate generator — the datasheet sequence order is not optional.

### Pin muxing — GPIOPCTL

`GPIOPCTL` lower byte cleared with `&= ~0xFF` then written as `0x11` — function 1 in the nibble for PA0 (U0RX) and PA1 (U0TX). Confirmed from Table 10-2 of the datasheet.

### Polling TX and RX

```
TX: while (UART0_FR_REG & UART0_TXFF);   // spin while TX FIFO full (bit 5)
    UART0_DR_REG = byte & 0xFF;

RX: while (UART0_FR_REG & UART0_RXFE);   // spin while RX FIFO empty (bit 4)
    return UART0_DR_REG & 0xFF;
```

Polling RX blocks indefinitely until a byte arrives — the CPU cannot respond to the button interrupt while waiting. This is the deliberate limitation of Phase 2, motivating the interrupt-driven redesign in Phase 3.

### Hex output — no sprintf

`uart0_send_hex8()` converts a byte to two uppercase hex characters using the lookup array `"0123456789ABCDEF"`. `uart0_send_uint32_hex()` sends a 32-bit value as 8 hex characters by extracting each byte MSB-first with `(val >> (i*8)) & 0xFF`.

### Key UART0 registers (base 0x4000C000)

| Register | Offset | Address | Purpose |
|----------|--------|---------|---------|
| UARTDR | 0x000 | 0x4000C000 | Data — write TX, read RX |
| UARTFR | 0x018 | 0x4000C018 | Flags — TXFF(5), RXFE(4) |
| UARTIBRD | 0x024 | 0x4000C024 | Integer baud divisor |
| UARTFBRD | 0x028 | 0x4000C028 | Fractional baud divisor |
| UARTLCRH | 0x02C | 0x4000C02C | Line control |
| UARTCTL | 0x030 | 0x4000C030 | Control — UARTEN(0), TXE(8), RXE(9) |

Port A base: `0x40004000`. `GPIOAFSEL` at +0x420, `GPIODEN` at +0x51C, `GPIOPCTL` at +0x52C.

---

## Design decisions

**`volatile unsigned long *` for all registers**
Consistent with TI's official TM4C header definitions for this target. Without `volatile` the compiler may cache register reads, eliminate repeated writes, or reorder accesses — all of which produce incorrect hardware behaviour on real peripherals.

**`1U` for all bit masks**
Shift expressions use the `1U` unsigned literal to prevent signed integer overflow when shifting into bit 31 of a 32-bit register.

**`|=` for clock enables, never `=`**
Clock gate registers control multiple peripherals simultaneously. Direct assignment disables all other peripheral clocks in the same register. Every enable is read-modify-write.

**Field clear-before-write for multi-bit fields**
Multi-bit fields (XTAL at bits [10:6], SYSDIV2:SYSDIV2LSB at bits [28:22], GPIOPCTL nibbles) are cleared with `&= ~mask` before writing the new value. Without the clear, residual bits from the reset state produce incorrect hardware configuration.

**Non-blocking timing via counter subtraction**
`wait(ms)` uses `(get_time_ms() - start) < ms`. Unsigned subtraction handles 32-bit counter rollover correctly — when `timer_count` wraps from `0xFFFFFFFF` to `0` the subtraction still yields the correct elapsed time.

**Fractional baud divisor**
Reduces baud rate error from 0.94% (integer only) to 0.006% at 115200 baud on 80MHz clock. Both IBRD and FBRD always written — never left at reset values.

**ISRs kept minimal**
All ISRs do the minimum necessary work and return. `systick_ISR` increments a counter. `pb_ISR` sets a flag. Processing happens in the main loop. This minimises interrupt latency and avoids blocking in ISR context.

**Polling RX is intentional in Phase 2**
The simplest correct implementation. Its fundamental limitation — CPU blocks waiting for bytes — is the explicit motivation for the interrupt-driven ring buffer design in Phase 3.

---

## Building

Each phase is a standalone CCS project. Import the relevant phase directory into Code Composer Studio, build with the default configuration, and flash using the onboard ICDI debugger.

For Phase 2: connect a serial terminal (PuTTY, Tera Term) to the board's USB virtual COM port at 115200 baud, 8 data bits, no parity, 1 stop bit, no flow control.

No external dependencies. No libraries beyond the C standard library.

---

## Phases

| Phase | Status | Title | Core concepts |
|-------|--------|-------|---------------|
| 1 | Complete | GPIO driver | Register access, PLL, SysTick, GPIO interrupts, debounce |
| 2 | Complete | UART polling driver | Baud rate, fractional divisor, pin mux, polling TX/RX |
| 3 | In progress | UART interrupt shell | ISR, ring buffer, FSM parser, function pointer dispatch |
| 4 | Planned | Sensor data logger | I2C (MPU-6050) + SPI (BMP280), timer ISR, fixed-point math |
| 5 | Planned | FreeRTOS pipeline | Tasks, queues, NVIC priorities, watchdog, HardFault handler |
| 6 | Planned | UART bootloader | Linker script, flash erase/write, vector table relocation, CRC-16 |

---

## Reference

- [TM4C123GH6PM Datasheet (SPMS376E)](https://www.ti.com/lit/ds/symlink/tm4c123gh6pm.pdf)
- [TM4C123G LaunchPad User Guide (SPMU296)](https://www.ti.com/lit/ug/spmu296/spmu296.pdf)
- [ARM Cortex-M4 Technical Reference Manual](https://developer.arm.com/documentation/100166/0001)
