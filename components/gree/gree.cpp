#include "gree.h"
#include "esphome/components/remote_base/remote_base.h"
#include "esphome/core/log.h"

namespace esphome::gree {

static const char *const TAG = "gree.climate";

// ── Option ↔ nibble conversion ────────────────────────────────────────────────

uint8_t GreeClimate::option_to_nibble(const std::string &opt) {
  if (opt == SWING_OPT_OFF)   return GREE_SWING_OFF;
  if (opt == SWING_OPT_SWING) return GREE_SWING_SWING;
  if (opt == SWING_OPT_1)     return GREE_SWING_POS1;
  if (opt == SWING_OPT_2)     return GREE_SWING_POS2;
  if (opt == SWING_OPT_3)     return GREE_SWING_POS3;
  if (opt == SWING_OPT_4)     return GREE_SWING_POS4;
  if (opt == SWING_OPT_5)     return GREE_SWING_POS5;
  return GREE_SWING_OFF;
}

std::string GreeClimate::nibble_to_option(uint8_t nibble) {
  switch (nibble) {
    case GREE_SWING_SWING: return SWING_OPT_SWING;
    case GREE_SWING_POS1:  return SWING_OPT_1;
    case GREE_SWING_POS2:  return SWING_OPT_2;
    case GREE_SWING_POS3:  return SWING_OPT_3;
    case GREE_SWING_POS4:  return SWING_OPT_4;
    case GREE_SWING_POS5:  return SWING_OPT_5;
    default:               return SWING_OPT_OFF;
  }
}

// ── Model setup ───────────────────────────────────────────────────────────────

void GreeClimate::set_model(Model model) {
  if (model == GREE_YX1FF) {
    this->fan_modes_.insert(climate::CLIMATE_FAN_QUIET);
    this->presets_.insert(climate::CLIMATE_PRESET_NONE);
    this->presets_.insert(climate::CLIMATE_PRESET_SLEEP);
  }

  this->set_supported_custom_fan_modes({
      GREE_CUSTOM_FAN_QUIET,
      GREE_CUSTOM_FAN_LOW_MEDIUM,
      GREE_CUSTOM_FAN_MEDIUM_HIGH,
      GREE_CUSTOM_FAN_TURBO_SPEED,
  });

  this->model_ = model;
}

// ── Mode bit control (turbo / light / health / xfan switches) ─────────────────

void GreeClimate::set_mode_bit(uint8_t bit_mask, bool enabled) {
  if (enabled) {
    this->mode_bits_ |= bit_mask;
  } else {
    this->mode_bits_ &= ~bit_mask;
  }
  this->transmit_state();
}

// ── Per-axis swing control (called by GreeSwingSelect) ───────────────────────

void GreeClimate::set_swing_v(uint8_t nibble) {
  this->swing_v_ = nibble;
  this->transmit_state();
}

void GreeClimate::set_swing_h(uint8_t nibble) {
  this->swing_h_ = nibble;
  this->transmit_state();
}

// ── Effective swing per axis ──────────────────────────────────────────────────
// 0xFF = "not overridden by select" → fall back to coarse swing_mode entity.
// Any other value = per-axis select takes precedence.

uint8_t GreeClimate::effective_swing_v_() {
  if (this->swing_v_ != 0xFF)
    return this->swing_v_;
  // Fall back to swing_mode entity
  switch (this->swing_mode) {
    case climate::CLIMATE_SWING_VERTICAL:
    case climate::CLIMATE_SWING_BOTH:
      return GREE_SWING_SWING;
    default:
      return GREE_SWING_OFF;
  }
}

uint8_t GreeClimate::effective_swing_h_() {
  if (this->swing_h_ != 0xFF)
    return this->swing_h_;
  switch (this->swing_mode) {
    case climate::CLIMATE_SWING_HORIZONTAL:
    case climate::CLIMATE_SWING_BOTH:
      return GREE_SWING_SWING;
    default:
      return GREE_SWING_OFF;
  }
}

// ── Transmit ──────────────────────────────────────────────────────────────────

void GreeClimate::transmit_state() {
  uint8_t remote_state[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00};

  remote_state[0] = this->fan_speed_() | this->operation_mode_();
  remote_state[1] = this->temperature_();

  uint8_t sv = this->effective_swing_v_();
  uint8_t sh = this->effective_swing_h_();

  // SwingAuto flag: set whenever either axis is in auto-swing (nibble == 1)
  if (sv == GREE_SWING_SWING || sh == GREE_SWING_SWING)
    remote_state[0] |= GREE_SWING_AUTO_BIT;

  if (this->model_ == GREE_YAN) {
    remote_state[2] = 0x20;
    remote_state[3] = 0x50;
    remote_state[4] = sv;  // YAN: vertical only, byte 4 low nibble
  }

  if (this->model_ == GREE_YX1FF || this->model_ == GREE_YAG) {
    remote_state[2] = 0x60;
    remote_state[3] = 0x50;
    remote_state[4] = (sv & 0x0F) | ((sh & 0x07) << 4);
  }

  if (this->model_ == GREE_YAG) {
    remote_state[5] = 0x40;
  }

  if (this->model_ == GREE_YAC || this->model_ == GREE_YAG) {
    // horizontal swing shares byte 4 high nibble for these models too
    remote_state[4] = (sv & 0x0F) | ((sh & 0x07) << 4);
  }

  if (this->model_ == GREE_YAA || this->model_ == GREE_YAC || this->model_ == GREE_YAC1FB9) {
    remote_state[2] = 0x20;
    remote_state[3] = 0x50;
    // ── YAC1FB9 confirmed constants from IR captures ──
    remote_state[5] = 0x80;  // model constant (was wrong 0x20 in upstream)
    remote_state[6] = 0x00;  // reserved      (was wrong 0x20 in upstream)
    // Both swing axes in byte 4
    remote_state[4] = (sv & 0x0F) | ((sh & 0x07) << 4);
  }

  // Merge feature flag bits into byte 2 (turbo/light/health/xfan)
  if (this->model_ == GREE_YAN || this->model_ == GREE_YAA ||
      this->model_ == GREE_YAC || this->model_ == GREE_YAC1FB9) {
    remote_state[2] = (remote_state[2] & 0x0F) | this->mode_bits_;
  }

  // YX1FF: turbo speed bit + sleep preset
  if (this->model_ == GREE_YX1FF) {
    if (this->fan_speed_() == GREE_FAN_TURBO)
      remote_state[2] |= GREE_FAN_TURBO_BIT_YX;
    if (this->preset_() == climate::CLIMATE_PRESET_SLEEP)
      remote_state[0] |= GREE_PRESET_SLEEP_BIT;
  }

  // Non-YX1FF Turbo custom fan mode: also set turbo bit in byte 2
  if (this->model_ != GREE_YX1FF && this->has_custom_fan_mode() &&
      this->get_custom_fan_mode() == GREE_CUSTOM_FAN_TURBO_SPEED) {
    remote_state[2] |= GREE_TURBO_BIT;
  }

  // Quiet: set bit 7 of byte 0
  if (this->has_custom_fan_mode() && this->get_custom_fan_mode() == GREE_CUSTOM_FAN_QUIET) {
    remote_state[0] |= GREE_FAN_QUIET_BIT;
  }

  // ── Checksum ─────────────────────────────────────────────────────────────────
  if (this->model_ == GREE_YAN || this->model_ == GREE_YX1FF) {
    // Simple nibble sum of bytes 0-3, stored in high nibble of byte 7
    remote_state[7] = (((remote_state[0] & 0x0F) + (remote_state[1] & 0x0F) +
                         (remote_state[2] & 0x0F) + (remote_state[3] & 0x0F)) &
                        0x0F) << 4;
  } else if (this->model_ == GREE_YAC1FB9) {
    // YAC1FB9-specific checksum confirmed by IR capture analysis:
    //   byte0 included with SwingAuto (bit6) and Quiet (bit7) masked out
    //   SwingV (byte4 low nibble) excluded from sum
    //   SwingH (byte4 high nibble) included
    //   bytes 5-6 included normally; offset=3; result in high nibble only
    uint8_t b0_cs = remote_state[0] & 0x3F;
    uint8_t sum = 3;
    sum += (b0_cs & 0x0F) + (b0_cs >> 4);
    for (int i = 1; i < 4; i++) {
      sum += (remote_state[i] & 0x0F);
      sum += (remote_state[i] >> 4);
    }
    sum += (remote_state[4] >> 4);  // SwingH only, not SwingV
    for (int i = 5; i < 7; i++) {
      sum += (remote_state[i] & 0x0F);
      sum += (remote_state[i] >> 4);
    }
    remote_state[7] = (sum & 0x0F) << 4;
  } else {
    // Kelvinator-style block checksum for YAA/YAC/YAG/GENERIC
    uint8_t sum = 10;
    for (int i = 0; i < 4; i++) {
      sum += (remote_state[i] & 0x0F);
      sum += (remote_state[i] >> 4);
    }
    remote_state[7] = (sum & 0x0F) << 4;
    for (int i = 5; i < 7; i++) {
      sum += (remote_state[i] & 0x0F);
      sum += (remote_state[i] >> 4);
    }
    remote_state[7] |= (sum & 0x0F);
  }

  ESP_LOGD(TAG, "Sending frame: [%02X %02X %02X %02X %02X %02X %02X %02X]",
           remote_state[0], remote_state[1], remote_state[2], remote_state[3],
           remote_state[4], remote_state[5], remote_state[6], remote_state[7]);

  // ── IR pulse assembly ─────────────────────────────────────────────────────────
  auto transmit = this->transmitter_->transmit();
  auto *data = transmit.get_data();

  uint32_t header_mark, header_space, bit_mark, message_space;

  if (this->model_ == GREE_YAC1FB9) {
    header_mark    = GREE_YAC_HEADER_MARK;
    header_space   = GREE_YAC1FB9_HEADER_SPACE;
    bit_mark       = GREE_YAC_BIT_MARK;
    message_space  = GREE_YAC1FB9_MESSAGE_SPACE;
  } else if (this->model_ == GREE_YAC || this->model_ == GREE_YAA) {
    header_mark    = GREE_YAC_HEADER_MARK;
    header_space   = GREE_YAC_HEADER_SPACE;
    bit_mark       = GREE_YAC_BIT_MARK;
    message_space  = GREE_MESSAGE_SPACE;
  } else {
    header_mark    = GREE_HEADER_MARK;
    header_space   = GREE_HEADER_SPACE;
    bit_mark       = GREE_BIT_MARK;
    message_space  = GREE_MESSAGE_SPACE;
  }

  // Helper lambda to build and fire one complete Gree frame
  auto send_frame = [&]() {
    data->reset();
    data->set_carrier_frequency(GREE_IR_FREQUENCY);

    data->mark(header_mark);
    data->space(header_space);

    // Block 1: bytes 0-3, LSB first
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 8; j++) {
        data->mark(bit_mark);
        data->space((remote_state[i] & (1 << j)) ? GREE_ONE_SPACE : GREE_ZERO_SPACE);
      }
    }

    // Inter-block separator: 0, 0, 1
    data->mark(bit_mark); data->space(GREE_ZERO_SPACE);
    data->mark(bit_mark); data->space(GREE_ZERO_SPACE);
    data->mark(bit_mark); data->space(GREE_ONE_SPACE);

    data->mark(bit_mark);
    data->space(message_space);

    // Block 2: bytes 4-7, LSB first
    for (int i = 4; i < 8; i++) {
      for (int j = 0; j < 8; j++) {
        data->mark(bit_mark);
        data->space((remote_state[i] & (1 << j)) ? GREE_ONE_SPACE : GREE_ZERO_SPACE);
      }
    }

    data->mark(bit_mark);
    data->space(GREE_ZERO_SPACE);

    transmit.perform();
  };

  // Transmit 3 times with 100ms silence between frames, matching original remote behaviour
  send_frame();
  delay(GREE_REPEAT_GAP_MS);
  send_frame();
  delay(GREE_REPEAT_GAP_MS);
  send_frame();
}

// ── Helper encoders ───────────────────────────────────────────────────────────

uint8_t GreeClimate::operation_mode_() {
  uint8_t mode = GREE_MODE_ON;
  switch (this->mode) {
    case climate::CLIMATE_MODE_COOL:       mode |= GREE_MODE_COOL; break;
    case climate::CLIMATE_MODE_DRY:        mode |= GREE_MODE_DRY;  break;
    case climate::CLIMATE_MODE_HEAT:       mode |= GREE_MODE_HEAT; break;
    case climate::CLIMATE_MODE_FAN_ONLY:   mode |= GREE_MODE_FAN;  break;
    case climate::CLIMATE_MODE_HEAT_COOL:  mode |= GREE_MODE_AUTO; break;
    default: return GREE_MODE_OFF;
  }
  return mode;
}

uint8_t GreeClimate::fan_speed_() {
  // Custom fan modes take priority
  if (this->has_custom_fan_mode()) {
    auto cfm = this->get_custom_fan_mode();
    if (cfm == GREE_CUSTOM_FAN_QUIET)       return GREE_FAN_AUTO;  // quiet bit set separately
    if (cfm == GREE_CUSTOM_FAN_LOW_MEDIUM)  return GREE_FAN_2;
    if (cfm == GREE_CUSTOM_FAN_MEDIUM_HIGH) return GREE_FAN_4;
    if (cfm == GREE_CUSTOM_FAN_TURBO_SPEED) return GREE_FAN_5;
  }
  switch (this->fan_mode.value_or(climate::CLIMATE_FAN_AUTO)) {
    case climate::CLIMATE_FAN_LOW:    return GREE_FAN_1;
    case climate::CLIMATE_FAN_MEDIUM: return GREE_FAN_3;
    case climate::CLIMATE_FAN_HIGH:   return GREE_FAN_5;
    case climate::CLIMATE_FAN_QUIET:  return GREE_FAN_AUTO;  // YX1FF quiet bit handled above
    default:                          return GREE_FAN_AUTO;
  }
}

uint8_t GreeClimate::temperature_() {
  // Low nibble only — no timer bits, no I-Feel bit
  return (uint8_t) roundf(
      clamp(this->target_temperature, (float)GREE_TEMP_MIN, (float)GREE_TEMP_MAX)
  ) - GREE_TEMP_MIN;
}

uint8_t GreeClimate::preset_() {
  if (this->preset.has_value() && this->preset.value() == climate::CLIMATE_PRESET_SLEEP)
    return climate::CLIMATE_PRESET_SLEEP;
  return 0;
}

}  // namespace esphome::gree