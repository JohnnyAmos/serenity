/*
 * Copyright (c) 2021, Nico Weber <thakis@chromium.org>
 * Copyright (c) 2024, Sönke Holz <sholz8530@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <Kernel/Arch/aarch64/PlatformInit.h>

#include <Kernel/Arch/aarch64/DebugOutput.h>
#include <Kernel/Arch/aarch64/RPi/Framebuffer.h>
#include <Kernel/Arch/aarch64/RPi/GPIO.h>
#include <Kernel/Arch/aarch64/RPi/Mailbox.h>
#include <Kernel/Arch/aarch64/RPi/Timer.h>
#include <Kernel/Arch/aarch64/RPi/UART.h>

namespace Kernel {

void raspberry_pi_platform_init(StringView)
{
    static DebugConsole const s_debug_console {
        .write_character = [](char character) {
            RPi::UART::the().send(character);
        },
    };

    RPi::Mailbox::initialize();
    RPi::GPIO::initialize();
    RPi::UART::initialize();

    constexpr int baud_rate = 115'200;

    // Set UART clock so that the baud rate divisor ends up as 1.0.
    // FIXME: Not sure if this is a good UART clock rate.
    u32 rate_in_hz = RPi::Timer::set_clock_rate(RPi::Timer::ClockID::UART, 16 * baud_rate);

    // The BCM's PL011 UART is alternate function 0 on pins 14 and 15.
    auto& gpio = RPi::GPIO::the();
    gpio.set_pin_function(14, RPi::GPIO::PinFunction::Alternate0);
    gpio.set_pin_function(15, RPi::GPIO::PinFunction::Alternate0);
    gpio.set_pin_pull_up_down_state(Array { 14, 15 }, RPi::GPIO::PullUpDownState::Disable);

    // Clock and pins are configured. Turn UART on.
    RPi::UART::the().set_baud_rate(baud_rate, rate_in_hz);

    set_debug_console(&s_debug_console);

    auto firmware_version = RPi::Mailbox::the().query_firmware_version();
    dmesgln("RPi: Firmware version: {}", firmware_version);

    RPi::Framebuffer::initialize();
}

}
