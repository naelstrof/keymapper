#pragma once

#include <chrono>
#include <cstdint>
#include <algorithm>
#include "KeyEvent.h"

template<typename R, typename P>
uint16_t duration_to_timeout(const std::chrono::duration<R, P>& duration) {
  const auto milliseconds = 
    std::chrono::duration_cast<std::chrono::milliseconds>(duration);
  const auto max = (1 << KeyEvent::timeout_bits) - 1;
  return static_cast<uint16_t>(std::clamp(
    static_cast<int>(milliseconds.count()),
    0, max));
}

inline std::chrono::milliseconds timeout_to_milliseconds(uint16_t timeout) {
  return std::chrono::milliseconds(timeout);
}

template<typename R, typename P>
KeyEvent make_input_timeout_event(const std::chrono::duration<R, P>& duration) {
  return KeyEvent(Key::timeout, KeyState::Up, duration_to_timeout(duration));
}

inline bool is_input_timeout_event(const KeyEvent& event) {
  return (event.key == Key::timeout && event.state == KeyState::Up);
}

inline uint16_t sum_timeouts(uint16_t a, uint16_t b) {
  const auto max = (1 << KeyEvent::timeout_bits) - 1;
  return static_cast<uint16_t>(std::min(
    static_cast<int>(a) + static_cast<int>(b), max));
}
