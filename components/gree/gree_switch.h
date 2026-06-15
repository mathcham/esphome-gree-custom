#pragma once

#include "gree.h"
#include "esphome/components/switch/switch.h"
#include "esphome/core/component.h"

namespace esphome::gree {

class GreeSwitch : public switch_::Switch, public Component {
 public:
  void set_climate(GreeClimate *climate) { this->climate_ = climate; }
  void set_bit_mask(uint8_t mask) { this->bit_mask_ = mask; }

  void setup() override {
    // Restore the initial state from mode_bits so the switch reflects
    // the current state on boot without needing a transmit.
    // (GreeClimate boots with GREE_LIGHT_BIT set by default.)
  }

 protected:
  void write_state(bool state) override {
    this->climate_->set_mode_bit(this->bit_mask_, state);
    this->publish_state(state);
  }

  GreeClimate *climate_{nullptr};
  uint8_t      bit_mask_{0};
};

}  // namespace esphome::gree
