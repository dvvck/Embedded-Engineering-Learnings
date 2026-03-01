# Introduction to Embedded Systems with Rust

## Getting started

Install Rust

```zsh
curl https://sh.rustup.rs -sSf | sh
```

## Project setup

First of all you need to install the standard library for the target platform.

```zsh
rustup target add thumbv7m-none-eabi
```

Afterwards we can create a new rust project.

```zsh
cargo new intro_to_embedded_rust
```

### Compiler instructions

We need to cross compile the code for the target chip.  
In this example it is the `Cortex-M3` Processor.
This can be done using a specific config file calles `.config.toml` which is located in the `.cargo/` directory in the root project directory.

- `target = "thumbv7m-none-eabi"`: This sets the default compile target.
- `runner= """ ...`
  - `qemu-system-arm`: starting the arm systemsimulator from QEMU
  - `-cpu cortex-m3`: emulating a cortex-m3 core
  - `-machine lm3s6965evb`: emulating a Texas Instruments LM3S6965-evalboard which is a kind of standard testboard
  - `-nographic`: suppressing graphical output and redirect output to the terminal
  - `-semihosting-config enable=on, target=native`: activating semihosting which allows prints to the host terminal
  - `-kernel`: defines the ELF binary which will be the compiled binary of the project
- `rustflags = ["-C", "link-arg=-Tlink.x", "-C", "link-arg=--nmagic"]`:
  - `-C link-arg=-Tlink.x`: This passes `Tlink.x` to the linker which defines the memory layout.
  - `-C", "link-arg=--nmagic`: Deactivating page alignment which would lead to padding.

```toml
[build]
target = "thumbv7m-none-eabi"

[target.thumbv7m-none-eabi]
runner = """
qemu-system-arm \
-cpu cortex-m3 \
-machine lm3s6965evb \
-nographic \
-semihosting-config enable=on,target=native \
-kernel
"""
rustflags = ["-C", "link-arg=-Tlink.x", "-C", "link-arg=--nmagic"]
```

### Memory region

Now we need to specify the memory region of the chip.
This is done using the `memory.x` file.

The arm Cortex-M3 has 256KiB of flash memory which start at the address `0x00000000`.
The flash stores the application code.
Furthermore it has 64k of RAM which starts at `2x00000000` according to the arm specification.
The RAM holds the stack and the heap.

```x
MEMORY
{
    FLASH : ORIGIN = 0x00000000, LENGTH = 256k
    RAM : ORIGIN = 0x20000000, LENGTH = 64k
}
```

### Toolchain specifier

The toolchain specifier is used to declare which rust version, targets and compiler components need to be installed.

```toml
[toolchain]
#nightly is the daily updated channel, it was specified by the course
channel = "nightly-2023-10-13"
components = ["rust-src", "rustfmt", "rust-std"]
targets = ["thumbv7m-none-eabi"]
```


## Quick Emulator (QEMU)

To test the software QEMU is a open source way to emulate the hardware.

macOS:

```zsh
brew install qemu
```

arch:

```zsh
pacman -S qemu
```

## Linking against the core crate

Using the `std` library requires an operating system for actions like opening a file and reading from it.
The MCU does not have an operating system and therefore you can not use the `std` library.
In order to still be able to use these basic features rust offers the core crate which contains a platform agnostic subset of the `std` library.

Add the following instructions at the top of every source file.

```rust
#![no_std]
#![no_main]
```

## Panic Handling

In a std environment panic behavior is well defined.
If a problem occurs the stack is unwound and the destructors are run for each object.
In a embedded system it is the job of the developer to define the panic behavior himself.
Otherwise you will get an error trying to run `cargo build`.

```zsh
❯ cargo build
   Compiling intro_to_embedded_rust v0.1.0 (/Users/henrik/Developer/private/Embedded-Engineering-Learnings/99_demos/intro_to_embedded_rust)
error: `#[panic_handler]` function required, but not found

error: could not compile `intro_to_embedded_rust` (bin "intro_to_embedded_rust") due to previous error
```

There are several crates that can be used to define panic behavior but it is also possible to define the panic behavior ourselves.

This simple panic handler just uses an infinite loop.

```rust
use core::panic::PanicInfo;

#[panic_handler]
fn panic(_info: &panicInfor) -> ! {
    loop {}
}
```

## Implementing sleep

Implementing a sleep functionality is not that trivial and can be done using one of th e following methods:

- wait for interrupt
  - waits for an interrupt to occur
  
  - ```rust
    use cortex_m:asm;
    ...
    asm::wfi();
    ```
  
- wait for event
  - waits for an event or and interrupt
  
The interrupts can be triggered using a timer.

## Timers

Allows to wake up a system using interrupts.
Creating a timer is easy using `System Timer (SysTick)`.
Systick counts down from a specified value to zero and triggers an exception on zero.
After that it will reload back to the original value and count down again.

To figure out how many ticks we want to count down from to achieve the desired sleep value you need to now the CPU frequency.
In this example the CPU has a default frequency of 12.5 MHz that means it ticks 12.5 million times per second.

```rust
use cortex_m::peripheral::Peripherals;
use cortex_m::peripheral::syst::SystClkSource;
use cortex_m_rt::{entry,exception};

const CPU_FREQ: u32 = 12_500_000;

// singleton access to the core peripherals
let peripherals = Peripherals::take().unwrap();
// mutable required for member functions
let mut systick = peripherals.SYST;
// trigger interrupt when timer counts to zero
systick.enable_interrupt();
// uses the CPU main core as a reference for the ticks
systick.set_clock_source(SystClkSource::Core);
// set the timer to the main core tick frequency which results in a sleep time of one second
systick.set_reload(CPU_FREQ);
// resets the counter, after initialization it is undefined according to the ARM spec therefore a reset is mandatory 
systick.clear_current();
// start the timer
systick.enable_counter();


#[exception]
fn SysTick() {
    hprintln!("ugh, woke up :(")
}
```

Implementing the timer can also be done by writing directly to the corresponding registers.
The register addresses can be found in the [docs](https://developer.arm.com/documentation/dui0552/a/cortex-m3-peripherals/system-timer--systick).

```rust
// System tick - control and status register
const SYST_CSR: u32 = 0xE000E010;
// System tick - reload value register
const SYST_RVR: u32 = 0xE000E014;
// System tick - current value register
const SYST_CVR: u32 = 0xE000E018;
// CPU frequency (12.5 MHz by default)
const CPU_FREQ: u32 = 12_500_000;

unsafe {
    let sleep_dur: u32 = CPU_FREQ;
    // Set the timer duration
    *(SYST_RVR as *mut u32) = sleep_dur;
    // Clear the current value by writing to the register
    *(SYST_CVR as *mut u32) = 0;
    // Enable the clock
    *(SYST_CSR as *mut u32) = 0b111;
}
```

## Interrupts

Interrupts are a signal that ensures events are handled in a timely manner.
They functions defined in the Interrupt Vector Table (IVT).
When an interrupt is triggered a signal is send to the CPU which saves the current state of processing to the RAM.
Afterwards the CPU loads the interrupt service routine (ISR) from the IVT and executes it.
When that's finished, the CPU loads the state back form the RAM and continues processing where it stopped.

It is important to mention that interrupts have priorities assigned to them.
If multiple interrupt trigger at the same time, the highest priority interrupt will be handled first.
This is especially useful in systems with hard real time constraints.

## Registers

Registers are areas of memory in a processor where values are stored during processing and computation.
There are two types of registers: general purpose registers and control registers.

### Control registers

Control registers are used to configure a device.
In order to configure the devices you have to write specific values into their registers.
The registers have a fixed address in memory

### General purpose registers

General purpose registers hold addresses and data.
They are used for intermediate values calculated during computations and are stored in these registers.
This mechanism allows for high speed computation.

