#pragma once

#include "esphome/components/climate_ir/climate_ir.h"
#include "esphome/components/select/select.h"
#include "esphome/components/switch/switch.h"

namespace esphome {
namespace gree {

// ── IR timing ─────────────────────────────────────────────────────────────────
static constexpr uint32_t GREE_IR_FREQUENCY           = 38000;
static constexpr uint32_t GREE_HEADER_MARK            = 9000;
static constexpr uint32_t GREE_HEADER_SPACE           = 4000;
static constexpr uint32_t GREE_BIT_MARK               = 620;
static constexpr uint32_t GREE_ONE_SPACE              = 1600;
static constexpr uint32_t GREE_ZERO_SPACE             = 540;
static constexpr uint32_t GREE_MESSAGE_SPACE          = 19000;
// YAC1FB9-specific overrides (confirmed from upstream ESPHome source):
static constexpr uint32_t GREE_YAC1FB9_HEADER_SPACE  = 4500;
static constexpr uint32_t GREE_YAC1FB9_MESSAGE_SPACE = 19980;

// ── Temperature ───────────────────────────────────────────────────────────────
static constexpr uint8_t GREE_TEMP_MIN = 16;
static constexpr uint8_t GREE_TEMP_MAX = 30;

// ── Operating mode (byte 0 bits 0-2, bit 3 = power on) ───────────────────────
static constexpr uint8_t GREE_MODE_AUTO = 0x00;
static constexpr uint8_t GREE_MODE_COOL = 0x01;
static constexpr uint8_t GREE_MODE_DRY  = 0x02;
static constexpr uint8_t GREE_MODE_FAN  = 0x03;
static constexpr uint8_t GREE_MODE_HEAT = 0x04;
static constexpr uint8_t GREE_MODE_ON   = 0x08;

// ── Fan speed (byte 0 bits 4-5) ───────────────────────────────────────────────
// The confirmed YAC1FB9 protocol has 4 discrete IR fan speeds (2-bit field).
// "quiet" and "low" both map to FAN_1 (lowest hardware speed).
// "low_med" and "med_high" are custom modes; their IR encodings need capture
// verification — they currently use the nearest available speed.
static constexpr uint8_t GREE_FAN_AUTO = 0x00;
static constexpr uint8_t GREE_FAN_1    = 0x10;  // low / quiet
static constexpr uint8_t GREE_FAN_2    = 0x20;  // medium
static constexpr uint8_t GREE_FAN_3    = 0x30;  // high

// ── Mode bits packed into byte 2 (bits 4-7) ───────────────────────────────────
static constexpr uint8_t GREE_BIT_TURBO  = 0x10;
static constexpr uint8_t GREE_BIT_LIGHT  = 0x20;
static constexpr uint8_t GREE_BIT_HEALTH = 0x40;
static constexpr uint8_t GREE_BIT_XFAN   = 0x80;

// ── Vertical swing ────────────────────────────────────────────────────────────
// YAC1FB9: bit 6 of byte 0 = continuous sweep; byte 5 lower nibble = fixed pos.
static constexpr uint8_t GREE_VDIR_OFF    = 0x00;
static constexpr uint8_t GREE_VDIR_SWING  = 0x01;
static constexpr uint8_t GREE_VDIR_UP     = 0x02;
static constexpr uint8_t GREE_VDIR_MUP    = 0x03;
static constexpr uint8_t GREE_VDIR_MIDDLE = 0x04;
static constexpr uint8_t GREE_VDIR_MDOWN  = 0x05;
static constexpr uint8_t GREE_VDIR_DOWN   = 0x06;

// ── Horizontal swing (byte 4 bits 4-6) ───────────────────────────────────────
// Inferred from YAC model. Verify with IR capture if vanes do not respond.
static constexpr uint8_t GREE_HDIR_OFF    = 0x00;
static constexpr uint8_t GREE_HDIR_SWING  = 0x01;
static constexpr uint8_t GREE_HDIR_LEFT   = 0x02;
static constexpr uint8_t GREE_HDIR_MLEFT  = 0x03;
static constexpr uint8_t GREE_HDIR_MIDDLE = 0x04;
static constexpr uint8_t GREE_HDIR_MRIGHT = 0x05;
static constexpr uint8_t GREE_HDIR_RIGHT  = 0x06;

// ── Custom fan mode names ─────────────────────────────────────────────────────
static constexpr const char *GREE_FAN_TURBO    = "turbo";
static constexpr const char *GREE_FAN_LOW_MED  = "low_med";
static constexpr const char *GREE_FAN_MED_HIGH = "med_high";

// ── Swing select option labels ────────────────────────────────────────────────
static constexpr const char *SWING_OPT_OFF   = "Off";
static constexpr const char *SWING_OPT_SWING = "Swing";
static constexpr const char *SWING_OPT_POS1  = "1";
static constexpr const char *SWING_OPT_POS2  = "2";
static constexpr const char *SWING_OPT_POS3  = "3";
static constexpr const char *SWING_OPT_POS4  = "4";
static constexpr const char *SWING_OPT_POS5  = "5";

// ── Model codes ───────────────────────────────────────────────────────────────
enum Model { GREE_GENERIC, GREE_YAN, GREE_YAA, GREE_YAC, GREE_YAC1FB9, GREE_YX1FF, GREE_YAG };

// ─────────────────────────────────────────────────────────────────────────────
class GreeClimate : public climate_ir::ClimateIR {
 public:
  GreeClimate()
      : climate_ir::ClimateIR(
            GREE_TEMP_MIN, GREE_TEMP_MAX, 1.0f,
            true, true,
            {climate::CLIMATE_FAN_AUTO, climate::CLIMATE_FAN_LOW,
             climate::CLIMATE_FAN_MEDIUM, climate::CLIMATE_FAN_HIGH},
            {climate::CLIMATE_SWING_OFF}) {}

  void set_model(Model model);
  void set_mode_bit(uint8_t bit_mask, bool enabled);
  void set_swing_v(uint8_t pos);
  void set_swing_h(uint8_t pos);
  uint8_t get_swing_v() const { return swing_v_; }
  uint8_t get_swing_h() const { return swing_h_; }

  static uint8_t     option_to_pos(const std::string &opt);
  static std::string pos_to_option(uint8_t pos);

 protected:
  void control(const climate::ClimateCall &call) override;
  void transmit_state() override;
  climate::ClimateTraits traits() override;

 private:
  uint8_t encode_mode_() const;
  uint8_t encode_fan_() const;
  uint8_t encode_temp_() const;
  uint8_t checksum_(const uint8_t *s) const;

  Model   model_{GREE_GENERIC};
  uint8_t mode_bits_{0};
  uint8_t swing_v_{GREE_VDIR_OFF};
  uint8_t swing_h_{GREE_HDIR_OFF};
};

// ─────────────────────────────────────────────────────────────────────────────
class GreeSwingSelect : public select::Select, public Component,
                        public Parented<GreeClimate> {
 public:
  void set_is_vertical(bool v) { is_vertical_ = v; }

  void setup() override {
    uint8_t pos = is_vertical_ ? this->parent_->get_swing_v()
                               : this->parent_->get_swing_h();
    this->publish_state(GreeClimate::pos_to_option(pos));
  }

 protected:
  void control(const std::string &value) override;
  bool is_vertical_{true};
};

// ─────────────────────────────────────────────────────────────────────────────
class GreeModeSwitch : public switch_::Switch, public Component,
                       public Parented<GreeClimate> {
 public:
  explicit GreeModeSwitch(uint8_t bit_mask) : bit_mask_(bit_mask) {}

 protected:
  void write_state(bool state) override {
    this->parent_->set_mode_bit(bit_mask_, state);
    this->publish_state(state);
  }
  uint8_t bit_mask_;
};

}  // namespace gree
}  // namespace esphome
