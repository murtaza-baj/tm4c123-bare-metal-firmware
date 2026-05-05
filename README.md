# TM4C123GH6PM Bare-Metal Firmware Portfolio

A collection of embedded firmware projects targeting the Texas Instruments TM4C123GH6PM (ARM Cortex-M4F, 80MHz) written entirely in bare-metal C. No TivaWare, no HAL, no abstraction libraries — every peripheral is driven directly through memory-mapped register access derived from the datasheet.

---

## Hardware

**Board:** TI Tiva C Series TM4C123GH6PM LaunchPad (EK-TM4C123GXL)
**MCU:** TM4C123GH6PM — ARM Cortex-M4F, 80MHz, 256KB flash, 32KB SRAM
**Toolchain:** arm-none-eabi-gcc, TI Code Composer Studio
**Debugger:** Onboard ICDI (In-Circuit Debug Interface)

---

## Repository structure

```
tm4c-bare-metal/
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
| `startup_tm4c.c` | Vector table, ISR routing — `systick_ISR` and `pb_ISR` wired directly |

### Technical approach

All register access is through `volatile uint32_t *` pointer casts — no TivaWare, no HAL. The initialisation sequence follows the exact order from the TM4C123GH6PM datasheet.

### PLL configuration — 80MHz

The TM4C PLL produces 400MHz divided to the target system frequency. The configuration sequence follows Section 5.3 of the datasheet:

1. Enable `BYPASS` in RCC and RCC2 — run from oscillator during reconfiguration
2. Set `USERCC2` in RCC2 — use extended RCC2 divisor control
3. Clear and set XTAL field in RCC to `0x15` (16MHz crystal)
4. Clear OSCSRC2 field in RCC2 (main oscillator source) and clear `PWRDN2` (power up PLL)
5. Set `DIV400` in RCC2 — use 400MHz PLL output
6. Clear the 6-bit SYSDIV2 field [28:23] then write `4` — 400MHz / (4+1) = 80MHz
7. Set `USESYSDIV` in RCC — enable the system clock divider
8. Poll `SYSCTL_RIS` bit 6 (`PLLLRIS`) until PLL locks
9. Clear `BYPASS2` in RCC2 — switch system clock to PLL

The SYSDIV2 field is cleared before writing to prevent residual bits from producing an incorrect clock frequency. The XTAL field is similarly cleared before writing. Both are read-modify-write operations — direct assignment to RCC or RCC2 would corrupt unrelated configuration bits.

### SysTick — 1ms timebase

SysTick is configured at system_init time. At 80MHz, a 1ms interrupt requires a reload value of 79999 (80,000 counts, 0-indexed). The control register is written with named bit macros:

```
STCTRL bit 2 (CLK_SRC) = 1 → system clock source
STCTRL bit 1 (INTEN)   = 1 → interrupt enable
STCTRL bit 0 (ENABLE)  = 1 → counter enable
```

`systick_ISR` increments a `volatile uint32_t timer_count`. `get_time_ms()` returns it. `wait(ms)` uses the non-blocking subtraction pattern `(get_time_ms() - start) < ms` which handles 32-bit counter rollover correctly at any uptime.

### GPIO initialisation sequence

Follows Section 10.3 of the datasheet:

1. Disable AHB for GPIO port (use APB — consistent with APB register base addresses)
2. Enable Port F clock via `SYSCTL_RCGCGPIO` bit 5
3. Two read barriers on `RCGCGPIO` to allow clock to stabilise before register access
4. Set output pins (PF1, PF2, PF3) in `GPIODIR`
5. Clear alternate function select (`GPIOAFSEL`) for all used pins
6. Set 2mA drive strength (`GPIODR2R`) for output pins
7. Enable pull-up on PF4 (`GPIOPUR`) — SW1 is active low
8. Enable digital function (`GPIODEN`) for all used pins

The `GPIODEN` step is mandatory on TM4C — all pins default to analog/high-impedance. Without it, writes to the data register have no effect on the physical pin.

### GPIO interrupt — SW1 on PF4

The button interrupt follows the TM4C GPIO interrupt configuration sequence:

1. Mask the interrupt (`GPIOIM`) before configuring — prevents spurious triggers during setup
2. Configure edge-sensitive, single-edge, falling-edge detection (`GPIOIS`, `GPIOIBE`, `GPIOIEV`)
3. Clear any pending interrupt (`GPIOICR`)
4. Unmask the interrupt (`GPIOIM`)
5. Enable GPIO Port F interrupt in NVIC EN0 register — Port F is IRQ 30

`pb_ISR` in `main.c`:
- Checks `GPIOMIS` to confirm PF4 is the source (port F shares one IRQ for all pins)
- Clears the interrupt via `GPIOICR` — mandatory, or the ISR fires again immediately on return
- Implements 50ms software debounce using the SysTick counter — ignores triggers within 50ms of the last confirmed press
- Sets `volatile uint8_t pb_flag` which the main loop polls

### The address-masked DATA register

The TM4C GPIO DATA register uses hardware address masking (Section 10.2.1). Bits [9:2] of the access address act as a pin mask — only pins with corresponding address bits set are affected. The "all pins" address (base + `0x3FC`) accesses all 8 pins simultaneously and is used throughout.

For targeted single-pin access: `address = GPIO_BASE + (pin_mask << 2)`.

### Key register addresses (Port F, base 0x40025000)

| Register | Offset | Address | Purpose |
|----------|--------|---------|---------|
| GPIODATA | 0x3FC | 0x400253FC | Data (all pins) |
| GPIODIR | 0x400 | 0x40025400 | Direction |
| GPIOIS | 0x404 | 0x40025404 | Interrupt sense |
| GPIOIBE | 0x408 | 0x40025408 | Interrupt both edges |
| GPIOIEV | 0x40C | 0x4002540C | Interrupt event |
| GPIOIM | 0x410 | 0x40025410 | Interrupt mask |
| GPIORIS | 0x414 | 0x40025414 | Raw interrupt status |
| GPIOMIS | 0x418 | 0x40025418 | Masked interrupt status |
| GPIOICR | 0x41C | 0x4002541C | Interrupt clear |
| GPIOAFSEL | 0x420 | 0x40025420 | Alternate function select |
| GPIODR2R | 0x500 | 0x40025500 | 2mA drive select |
| GPIOPUR | 0x510 | 0x40025510 | Pull-up enable |
| GPIODEN | 0x51C | 0x4002551C | Digital enable |

SYSCTL: `RCGCGPIO` at `0x400FE608` bit 5 (Port F), `RCC` at `0x400FE060`, `RCC2` at `0x400FE070`, `RIS` at `0x400FE050`.

---

## Design decisions

**`volatile uint32_t *` for all registers**
All register macros dereference through `volatile uint32_t *`. Without `volatile`, the compiler may cache register reads, eliminate repeated writes, or reorder accesses — all of which produce incorrect hardware behaviour on real peripherals.

**`1U` for all bit masks**
Shift expressions use unsigned literals (`1U << N`) to prevent signed integer overflow when shifting into bit 31 of a 32-bit register. Plain `1` is a signed int — `1 << 31` is undefined behaviour in C99.

**`|=` for clock enable, never `=`**
`RCGCGPIO` controls clocks for all six GPIO ports simultaneously. Direct assignment disables all other port clocks. Every enable is a read-modify-write.

**Field clear-before-set for multi-bit fields**
Multi-bit register fields (XTAL at bits [10:6], SYSDIV2 at bits [28:23]) are cleared with `&= ~(mask)` before writing the new value with `|=`. Without the clear step, previously set bits in the field persist and produce incorrect hardware configuration.

**Non-blocking timing via counter subtraction**
`wait(ms)` uses `(get_time_ms() - start) < ms` rather than a target comparison. Unsigned subtraction handles 32-bit timer rollover correctly — when `timer_count` wraps from `0xFFFFFFFF` to `0`, the subtraction still yields the correct elapsed time.

**ISR kept minimal**
Both ISRs (`systick_ISR` and `pb_ISR`) do the minimum necessary work — increment a counter or set a flag — and return. Processing happens in the main loop. This minimises interrupt latency and avoids blocking inside ISR context.

---

## Building

Import the `phase1_gpio/` directory into Code Composer Studio. Build with the default configuration. Flash using the onboard ICDI debugger (Run → Debug).

No external dependencies. No libraries beyond the C standard library.

---

## Phases planned

| Phase | Title | Core concepts |
|-------|-------|---------------|
| 1 | GPIO driver | Register access, PLL, SysTick, GPIO interrupts, debounce |
| 2 | UART polling driver | Baud rate calculation, fractional divisor, pin mux, TX/RX polling |
| 3 | UART interrupt shell | ISR, ring buffer, FSM command parser, function pointer dispatch |
| 4 | Sensor data logger | I2C (MPU-6050) + SPI (BMP280), timer ISR, fixed-point math |
| 5 | FreeRTOS pipeline | Tasks, queues, NVIC priorities, watchdog, HardFault handler |
| 6 | UART bootloader | Linker script, flash erase/write, vector table relocation, CRC-16 |

---

## Reference

- [TM4C123GH6PM Datasheet (SPMS376E)](https://www.ti.com/lit/ds/symlink/tm4c123gh6pm.pdf)
- [TM4C123G LaunchPad User Guide (SPMU296)](https://www.ti.com/lit/ug/spmu296/spmu296.pdf)
- [ARM Cortex-M4 Technical Reference Manual](https://developer.arm.com/documentation/100166/0001)
