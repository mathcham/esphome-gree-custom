#include "gree.h"
#include "esphome/core/log.h"

namespace esphome {
namespace gree {

static const char *const TAG = "gree.climate";

uint8_t GreeClimate::option_to_pos(const std::string &opt) {
  if (opt == SWING_OPT_SWING) return GREE_VDIR_SWING;
  if (opt == SWING_OPT_POS1)  return GREE_VDIR_UP;
  if (opt == SWING_OPT_POS2)  return GREE_VDIR_MUP;
  if (opt == SWING_OPT_POS3)  return GREE_VDIR_MIDDLE;
  if (opt == SWING_OPT_POS4)  return GREE_VDIR_MDOWN;
  if (opt == SWING_OPT_POS5)  return GREE_VDIR_DOWN;
  return GREE_VDIR_OFF;
}

std::string GreeClimate::pos_to_option(uint8_t pos) {
  switch (pos) {
    case GREE_VDIR_SWING:  return SWING_OPT_SWING;
    case GREE_VDIR_UP:     return SWING_OPT_POS1;
    case GREE_VDIR_MUP:    return SWING_OPT_POS2;
    case GREE_VDIR_MIDDLE: return SWING_OPT_POS3;
    case GREE_VDIR_MDOWN:  return SWING_OPT_POS4;
    case GREE_VDIR_DOWN:   return SWING_OPT_POS5;
    default:               return SWING_OPT_OFF;
  }
}

void GreeClimate::set_model(Model model) {
  if (model == GREE_YX1FF) {
    this->fan_modes_.insert(climate::CLIMATE_FAN_QUIET);
    this->presets_.insert(climate::CLIMATE_PRESET_NONE);
    this->presets_.insert(climate::CLIMATE_PRESET_SLEEP);
  }
  if (model == GREE_YAN) {
    this->swing_modes_.erase(climate::CLIMATE_SWING_HORIZONTAL);
    this->swing_modes_.erase(climate::CLIMATE_SWING_BOTH);
  }
  this->model_ = model;
}

climate::ClimateTraits GreeClimate::traits() {
  auto t = climate_ir::ClimateIR::traits();
  t.add_supported_custom_fan_mode(GREE_FAN_TURBO);
  t.add_supported_custom_fan_mode(GREE_FAN_LOW_MED);
  t.add_supported_custom_fan_mode(GREE_FAN_MED_HIGH);
  return t;
}

void GreeClimate::control(const climate::ClimateCall &call) {
  if (call.get_mode().has_value())
    this->mode = *call.get_mode();
  if (call.get_target_temperature().has_value())
    this->target_temperature = *call.get_target_temperature();
  if (call.get_fan_mode().has_value()) {
    this->fan_mode = call.get_fan_mode();
    this->custom_fan_mode.reset();
  }
  if (call.get_custom_fan_mode().has_value()) {
    this->custom_fan_mode = call.get_custom_fan_mode();
    this->fan_mode.reset();
  }
  if (call.get_swing_mode().has_value())
    this->swing_mode = *call.get_swing_mode();
  if (call.get_preset().has_value())
    this->preset = call.get_preset();
  this->transmit_state();
  this->publish_state();
}

void GreeClimate::set_mode_bit(uint8_t bit_mask, bool enabled) {
  if (enabled)
    this->mode_bits_ |= bit_mask;
  else
    this->mode_bits_ &= ~bit_mask;
  this->transmit_state();
}

void GreeClimate::set_swing_v(uint8_t pos) {
  this->swing_v_ = pos;
  this->transmit_state();
}

void GreeClimate::set_swing_h(uint8_t pos) {
  this->swing_h_ = pos;
  this->transmit_state();
}

void GreeClimate::transmit_state() {
  uint8_t rs[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00};

  rs[0] = this->encode_fan_() | this->encode_mode_();
  rs[1] = this->encode_temp_();

  if (this->model_ == GREE_YAN) {
    rs[2] = 0x20;
    rs[3] = 0x50;
    rs[4] = this->swing_v_;
  }

  if (this->model_ == GREE_YX1FF || this->model_ == GREE_YAG) {
    rs[2] = 0x60;
    rs[3] = 0x50;
    rs[4] = (this->swing_v_ & 0x0F) | ((this->swing_h_ & 0x07) << 4);
  }

  if (this->model_ == GREE_YAG) {
    rs[5] = 0x40;
  }

  if (this->model_ == GREE_YAC) {
    rs[2] = 0x20;
    rs[3] = 0x50;
    rs[4] = (this->swing_v_ & 0x0F) | ((this->swing_h_ & 0x07) << 4);
  }

  if (this->model_ == GREE_YAA || this->model_ == GREE_YAC1FB9) {
    rs[2] = 0x20;
    rs[3] = 0x50;
    rs[6] = 0x20;
    if (this->swing_v_ == GREE_VDIR_SWING) {
      rs[0] |= (1 << 6);
    } else if (this->swing_v_ != GREE_VDIR_OFF) {
      rs[5] = this->swing_v_;
    }
    rs[4] = (uint8_t)(this->swing_h_ << 4);
  }

  if (this->model_ == GREE_YAN || this->model_ == GREE_YAA ||
      this->model_ == GREE_YAC || this->model_ == GREE_YAC1FB9) {
    rs[2] = (rs[2] & 0x0F) | this->mode_bits_;
  }

  if (this->custom_fan_mode.has_value() && *this->custom_fan_mode == GREE_FAN_TURBO) {
    rs[2] |= GREE_BIT_TURBO;
  }

  if (this->model_ == GREE_YX1FF) {
    if (this->custom_fan_mode.has_value() && *this->custom_fan_mode == GREE_FAN_TURBO) {
      rs[2] |= 0x10;
    }
    if (this->preset.has_value() && *this->preset == climate::CLIMATE_PRESET_SLEEP) {
      rs[0] |= 0x80;
    }
  }

  rs[7] = this->checksum_(rs);

  ESP_LOGD(TAG, "TX: %02X %02X %02X %02X | %02X %02X %02X %02X",
           rs[0], rs[1], rs[2], rs[3], rs[4], rs[5], rs[6], rs[7]);

  auto transmit = this->transmitter_->transmit();
  auto *data = transmit.get_data();
  data->set_carrier_frequency(GREE_IR_FREQUENCY);

  uint32_t h_space = (this->model_ == GREE_YAC1FB9) ? GREE_YAC1FB9_HEADER_SPACE  : GREE_HEADER_SPACE;
  uint32_t msg_gap = (this->model_ == GREE_YAC1FB9) ? GREE_YAC1FB9_MESSAGE_SPACE : GREE_MESSAGE_SPACE;

  data->mark(GREE_HEADER_MARK);
  data->space(h_space);

  for (int i = 0; i < 4; i++) {
    for (uint8_t mask = 1; mask > 0; mask <<= 1) {
      data->mark(GREE_BIT_MARK);
      data->space((rs[i] & mask) ? GREE_ONE_SPACE : GREE_ZERO_SPACE);
    }
  }

  data->mark(GREE_BIT_MARK); data->space(GREE_ZERO_SPACE);
  data->mark(GREE_BIT_MARK); data->space(GREE_ONE_SPACE);
  data->mark(GREE_BIT_MARK); data->space(GREE_ZERO_SPACE);

  data->mark(GREE_BIT_MARK);
  data->space(msg_gap);

  for (int i = 4; i < 8; i++) {
    for (uint8_t mask = 1; mask > 0; mask <<= 1) {
      data->mark(GREE_BIT_MARK);
      data->space((rs[i] & mask) ? GREE_ONE_SPACE : GREE_ZERO_SPACE);
    }
  }

  data->mark(GREE_BIT_MARK);
  data->space(0);
  transmit.perform();
}

uint8_t GreeClimate::encode_mode_() const {
  switch (this->mode) {
    case climate::CLIMATE_MODE_COOL:      return GREE_MODE_ON | GREE_MODE_COOL;
    case climate::CLIMATE_MODE_DRY:       return GREE_MODE_ON | GREE_MODE_DRY;
    case climate::CLIMATE_MODE_HEAT:      return GREE_MODE_ON | GREE_MODE_HEAT;
    case climate::CLIMATE_MODE_HEAT_COOL: return GREE_MODE_ON | GREE_MODE_AUTO;
    case climate::CLIMATE_MODE_FAN_ONLY:  return GREE_MODE_ON | GREE_MODE_FAN;
    case climate::CLIMATE_MODE_OFF:
    default:                              return 0x00;
  }
}

uint8_t GreeClimate::encode_fan_() const {
  if (this->custom_fan_mode.has_value()) {
    const auto &m = *this->custom_fan_mode;
    if (m == GREE_FAN_TURBO)    return GREE_FAN_3;
    if (m == GREE_FAN_LOW_MED)  return GREE_FAN_1;
    if (m == GREE_FAN_MED_HIGH) return GREE_FAN_3;
  }
  switch (this->fan_mode.value_or(climate::CLIMATE_FAN_AUTO)) {
    case climate::CLIMATE_FAN_QUIET:  return GREE_FAN_1;
    case climate::CLIMATE_FAN_LOW:    return GREE_FAN_1;
    case climate::CLIMATE_FAN_MEDIUM: return GREE_FAN_2;
    case climate::CLIMATE_FAN_HIGH:   return GREE_FAN_3;
    default:                          return GREE_FAN_AUTO;
  }
}

uint8_t GreeClimate::encode_temp_() const {
  return (uint8_t)roundf(clamp<float>(this->target_temperature, GREE_TEMP_MIN, GREE_TEMP_MAX));
}

uint8_t GreeClimate::checksum_(const uint8_t *s) const {
  if (this->model_ == GREE_YAN || this->model_ == GREE_YX1FF) {
    return (uint8_t)(((s[0] << 4) + (s[1] << 4) + 0xC0) & 0xF0);
  }
  return ((((s[0] & 0x0F) + (s[1] & 0x0F) + (s[2] & 0x0F) + (s[3] & 0x0F) +
            ((s[4] & 0xF0) >> 4) + ((s[5] & 0xF0) >> 4) + ((s[6] & 0xF0) >> 4) + 0x0A) &
           0x0F) << 4);
}

void GreeSwingSelect::control(const std::string &value) {
  this->publish_state(value);
  uint8_t pos = GreeClimate::option_to_pos(value);
  if (is_vertical_)
    this->parent_->set_swing_v(pos);
  else
    this->parent_->set_swing_h(pos);
}

}  // namespace gree
}  // namespace esphome
