/*
MIT License

Copyright (c) 2026 Terence

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

The chip is a port, it will connect two of three busses together in either input or output mode.

| SEL1 | SEL0 | Results in |
| ---- | ---- | ---------- |
|   0  |   0  |  An -> Dn  |
|   0  |   1  |  Bn -> Dn  |
|   1  |   0  |  Dn -> An  |
|   1  |   1  |  Dn -> Bn  |

*/

#include "wokwi-api.h"
#include <array>
#include <cstdint>
#include <cstdio>

class ChipState
{
private:
  std::array<pin_t, 2UL> m_sel;
  std::array<pin_t, 8UL> m_port_a;
  std::array<pin_t, 8UL> m_port_b;
  std::array<pin_t, 8UL> m_port_d;

  static void sel_pin_change(void* user_data, pin_t pin, std::uint32_t value) noexcept;
  static void port_pin_change(void* user_data, pin_t pin, std::uint32_t value) noexcept;

public:
  ChipState() noexcept;
};

extern "C" void chip_init()
{
  static_cast<void>(new ChipState());
  std::puts("Online!");
}

ChipState::ChipState() noexcept
  : m_sel{{
    pin_init("SEL0", INPUT), pin_init("SEL1", INPUT)
  }},
  m_port_a{{
    pin_init("A0", INPUT), pin_init("A1", INPUT), pin_init("A2", INPUT), pin_init("A3", INPUT),
    pin_init("A4", INPUT), pin_init("A5", INPUT) ,pin_init("A6", INPUT), pin_init("A7", INPUT)
  }},
  m_port_b{{
    pin_init("B0", INPUT), pin_init("B1", INPUT), pin_init("B2", INPUT), pin_init("B3", INPUT),
    pin_init("B4", INPUT), pin_init("B5", INPUT) ,pin_init("B6", INPUT), pin_init("B7", INPUT)
  }},
  m_port_d{{
    pin_init("D0", INPUT), pin_init("D1", INPUT), pin_init("D2", INPUT), pin_init("D3", INPUT),
    pin_init("D4", INPUT), pin_init("D5", INPUT) ,pin_init("D6", INPUT), pin_init("D7", INPUT)
  }}
{
  const pin_watch_config_t sel_pin_watch_config{
    .user_data = this,
    .edge = BOTH,
    .pin_change = &ChipState::sel_pin_change
  };
  for (pin_t pin : m_sel)
  {
    pin_watch(pin, &sel_pin_watch_config);
  }
  sel_pin_change(this, 0, 0);
}

namespace
{
  template<std::size_t N>
    requires (N <= 8)
  inline std::uint8_t bus_read(const std::array<pin_t, N>& bus) noexcept
  {
    std::uint8_t value{};
    for (std::size_t index = 0; index < N; ++index)
    {
      if (pin_read(bus[index]) != LOW)
      {
        value |= (1 << index);
      }
    }
    return value;
  }

  template<std::size_t N>
    requires (N <= 8)
  inline void bus_write(const std::array<pin_t, N>& bus, std::uint8_t value) noexcept
  {
    for (std::size_t index = 0; index < N; ++index)
    {
      pin_write(bus[index], ((value >> index) & 1) == 0 ? LOW : HIGH);
    }
  }

  template<std::size_t N>
    requires (N <= 8)
  inline void bus_mode_input(const std::array<pin_t, N>& bus) noexcept
  {
    for (pin_t pin : bus)
    {
      pin_mode(pin, INPUT);
    }
  }

  template<std::size_t N>
    requires (N <= 8)
  inline void bus_mode_output(const std::array<pin_t, N>& bus, std::uint8_t value) noexcept
  {
    for (std::size_t index = 0; index < N; ++index)
    {
      pin_mode(bus[index], ((value >> index) & 1) == 0 ? OUTPUT_LOW : OUTPUT_HIGH);
    }
  }

  template<std::size_t N>
  inline void bus_watch_stop(const std::array<pin_t, N>& bus) noexcept
  {
    for (pin_t pin : bus)
    {
      pin_watch_stop(pin);
    }
  }

  template<std::size_t N>
  inline void bus_watch(const std::array<pin_t, N>& bus, const pin_watch_config_t& pin_watch_config) noexcept
  {
    for (pin_t pin : bus)
    {
      pin_watch(pin, &pin_watch_config);
    }
  }
}

void ChipState::sel_pin_change(void* user_data, pin_t, std::uint32_t) noexcept
{
  ChipState& chip = *static_cast<ChipState*>(user_data);
  const auto sel = bus_read(chip.m_sel);
  const pin_watch_config_t port_pin_watch_config{
    .user_data = user_data,
    .edge = BOTH,
    .pin_change = &ChipState::port_pin_change
  };
  switch (sel)
  {
  case 0:
    bus_watch_stop(chip.m_port_b);
    bus_watch_stop(chip.m_port_d);
    bus_mode_input(chip.m_port_a);
    bus_mode_input(chip.m_port_b);
    bus_mode_output(chip.m_port_d, bus_read(chip.m_port_a));
    bus_watch(chip.m_port_a, port_pin_watch_config);
    break;
  case 1:
    bus_watch_stop(chip.m_port_a);
    bus_watch_stop(chip.m_port_d);
    bus_mode_input(chip.m_port_a);
    bus_mode_input(chip.m_port_b);
    bus_mode_output(chip.m_port_d, bus_read(chip.m_port_b));
    bus_watch(chip.m_port_b, port_pin_watch_config);
  case 2:
    bus_watch_stop(chip.m_port_a);
    bus_watch_stop(chip.m_port_b);
    bus_mode_input(chip.m_port_b);
    bus_mode_input(chip.m_port_d);
    bus_mode_output(chip.m_port_a, bus_read(chip.m_port_d));
    bus_watch(chip.m_port_d, port_pin_watch_config);
    break;
  default:
    bus_watch_stop(chip.m_port_a);
    bus_watch_stop(chip.m_port_b);
    bus_mode_input(chip.m_port_a);
    bus_mode_input(chip.m_port_d);
    bus_mode_output(chip.m_port_b, bus_read(chip.m_port_d));
    bus_watch(chip.m_port_d, port_pin_watch_config);
    break;
  }
}

void ChipState::port_pin_change(void* user_data, pin_t pin, std::uint32_t value) noexcept
{
  ChipState& chip = *static_cast<ChipState*>(user_data);
  const auto sel = bus_read(chip.m_sel);
  switch (sel)
  {
  case 0:
    bus_write(chip.m_port_d, bus_read(chip.m_port_a));
    break;
  case 1:
    bus_write(chip.m_port_d, bus_read(chip.m_port_b));
    break;
  case 2:
    bus_write(chip.m_port_a, bus_read(chip.m_port_d));
    break;
  default:
    bus_write(chip.m_port_b, bus_read(chip.m_port_d));
    break;
  }
}
