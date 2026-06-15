#pragma once

#include "gree.h"
#include "esphome/components/select/select.h"
#include "esphome/core/component.h"

namespace esphome::gree {

class GreeSwingSelect : public select::Select, public Component {
 public:
  void set_climate(GreeClimate *climate) { this->climate_ = climate; }
  void set_axis(bool vertical) { this->vertical_ = vertical; }

  void setup() override {
    // Restore the last state from the climate component on boot
    uint8_t nibble = this->vertical_
        ? this->climate_->get_swing_v()
        : this->climate_->get_swing_h();

    // 0xFF = unset (follow swing_mode), treat as "Off" in the select
    if (nibble == 0xFF) nibble = GREE_SWING_OFF;

    this->publish_state(GreeClimate::nibble_to_option(nibble));
  }

 protected:
  void control(const std::string &value) override {
    uint8_t nibble = GreeClimate::option_to_nibble(value);
    if (this->vertical_) {
      this->climate_->set_swing_v(nibble);
    } else {
      this->climate_->set_swing_h(nibble);
    }
    this->publish_state(value);
  }

  GreeClimate *climate_{nullptr};
  bool vertical_{true};
};

}  // namespace esphome::gree