#pragma once

#include "esphome/components/climate_ir/climate_ir.h"

namespace esphome::gree {

// ── Temperature ───────────────────────────────────────────────────────────────
static constexpr uint8_t GREE_TEMP_MIN = 16;
static constexpr uint8_t GREE_TEMP_MAX = 30;

// ── Operation modes (byte 0 bits 0-2, bit 3 = power) ────────────────────────
static constexpr uint8_t GREE_MODE_AUTO = 0x00;
static constexpr uint8_t GREE_MODE_COOL = 0x01;
static constexpr uint8_t GREE_MODE_DRY  = 0x02;
static constexpr uint8_t GREE_MODE_FAN  = 0x03;
static constexpr uint8_t GREE_MODE_HEAT = 0x04;
static constexpr uint8_t GREE_MODE_OFF  = 0x00;
static constexpr uint8_t GREE_MODE_ON   = 0x08;

// ── Fan speed (byte 0 bits 4-5) ───────────────────────────────────────────────
static constexpr uint8_t GREE_FAN_AUTO = 0x00;
static constexpr uint8_t GREE_FAN_1    = 0x10;  // Low
static constexpr uint8_t GREE_FAN_2    = 0x20;  // Low-Medium
static constexpr uint8_t GREE_FAN_3    = 0x30;  // Medium
static constexpr uint8_t GREE_FAN_4    = 0x40;  // Medium-High
static constexpr uint8_t GREE_FAN_5    = 0x50;  // High

// ── Special fan bits (byte 0) ─────────────────────────────────────────────────
static constexpr uint8_t GREE_FAN_QUIET_BIT  = 0x80;  // bit 7: quiet/silent
static constexpr uint8_t GREE_SWING_AUTO_BIT = 0x40;  // bit 6: set when any axis = auto-swing

// ── Feature flags (byte 2 bits 4-7) ──────────────────────────────────────────
static constexpr uint8_t GREE_TURBO_BIT  = 0x10;
static constexpr uint8_t GREE_LIGHT_BIT  = 0x20;
static constexpr uint8_t GREE_HEALTH_BIT = 0x40;
static constexpr uint8_t GREE_XFAN_BIT   = 0x80;

// ── Swing byte 4 ─────────────────────────────────────────────────────────────
// Low nibble  (bits 0-3) = SwingV
// High nibble (bits 4-6) = SwingH
// Confirmed from captures of YAC1FB9:
static constexpr uint8_t GREE_SWING_OFF   = 0;  // off / stopped
static constexpr uint8_t GREE_SWING_SWING = 1;  // auto-swing
static constexpr uint8_t GREE_SWING_POS1  = 2;  // V: up        H: left
static constexpr uint8_t GREE_SWING_POS2  = 3;  // V: mid-up    H: mid-left
static constexpr uint8_t GREE_SWING_POS3  = 4;  // V: middle    H: center
static constexpr uint8_t GREE_SWING_POS4  = 5;  // V: mid-down  H: mid-right
static constexpr uint8_t GREE_SWING_POS5  = 6;  // V: down      H: right

// ── YX1FF-specific ────────────────────────────────────────────────────────────
static constexpr uint8_t GREE_FAN_TURBO         = 0x80;
static constexpr uint8_t GREE_FAN_TURBO_BIT_YX  = 0x10;
static constexpr uint8_t GREE_PRESET_SLEEP_BIT  = 0x80;

// ── Custom fan mode strings ───────────────────────────────────────────────────
static constexpr const char *GREE_CUSTOM_FAN_QUIET       = "Quiet";
static constexpr const char *GREE_CUSTOM_FAN_LOW_MEDIUM  = "Low-Medium";
static constexpr const char *GREE_CUSTOM_FAN_MEDIUM_HIGH = "Medium-High";
static constexpr const char *GREE_CUSTOM_FAN_TURBO_SPEED = "Turbo";

// ── Swing position select option strings ─────────────────────────────────────
// These are the exact strings exposed in HA. Keep them short and readable.
static constexpr const char *SWING_OPT_OFF   = "Off";
static constexpr const char *SWING_OPT_SWING = "Swing";
static constexpr const char *SWING_OPT_1     = "1";  // V: up        H: left
static constexpr const char *SWING_OPT_2     = "2";  // V: mid-up    H: mid-left
static constexpr const char *SWING_OPT_3     = "3";  // V: middle    H: center
static constexpr const char *SWING_OPT_4     = "4";  // V: mid-down  H: mid-right
static constexpr const char *SWING_OPT_5     = "5";  // V: down      H: right

// ── IR timing ────────────────────────────────────────────────────────────────
static constexpr uint32_t GREE_IR_FREQUENCY          = 38000;
static constexpr uint32_t GREE_HEADER_MARK           = 9000;
static constexpr uint32_t GREE_HEADER_SPACE          = 4000;
static constexpr uint32_t GREE_BIT_MARK              = 620;
static constexpr uint32_t GREE_ONE_SPACE             = 1600;
static constexpr uint32_t GREE_ZERO_SPACE            = 540;
static constexpr uint32_t GREE_MESSAGE_SPACE         = 19000;
static constexpr uint32_t GREE_YAC_HEADER_MARK       = 6000;
static constexpr uint32_t GREE_YAC_HEADER_SPACE      = 3000;
static constexpr uint32_t GREE_YAC_BIT_MARK          = 650;
static constexpr uint32_t GREE_YAC1FB9_HEADER_SPACE  = 4500;
static constexpr uint32_t GREE_YAC1FB9_MESSAGE_SPACE = 19980;
static constexpr uint32_t GREE_REPEAT_GAP           = 100000;  // 100ms silence between frame repetitions

// ── Model codes ───────────────────────────────────────────────────────────────
enum Model { GREE_GENERIC, GREE_YAN, GREE_YAA, GREE_YAC, GREE_YAC1FB9, GREE_YX1FF, GREE_YAG };

// ── Main climate class ────────────────────────────────────────────────────────
class GreeClimate : public climate_ir::ClimateIR {
 public:
  GreeClimate()
      : climate_ir::ClimateIR(
            GREE_TEMP_MIN, GREE_TEMP_MAX, 1.0f,
            true, true,
            {climate::CLIMATE_FAN_AUTO, climate::CLIMATE_FAN_LOW,
             climate::CLIMATE_FAN_MEDIUM, climate::CLIMATE_FAN_HIGH},
            {climate::CLIMATE_SWING_OFF, climate::CLIMATE_SWING_VERTICAL,
             climate::CLIMATE_SWING_HORIZONTAL, climate::CLIMATE_SWING_BOTH}) {}

  void set_model(Model model);

  // Called by the Gree switch platform (turbo / light / health / xfan)
  void set_mode_bit(uint8_t bit_mask, bool enabled);

  // Called by the Gree swing select platform
  void set_swing_v(uint8_t nibble);
  void set_swing_h(uint8_t nibble);

  // Getters used by swing selects to restore their state on boot
  uint8_t get_swing_v() const { return swing_v_; }
  uint8_t get_swing_h() const { return swing_h_; }

  // Convert option string ↔ nibble value (shared with gree_select.cpp)
  static uint8_t  option_to_nibble(const std::string &opt);
  static std::string nibble_to_option(uint8_t nibble);

 protected:
  void transmit_state() override;

  uint8_t operation_mode_();
  uint8_t fan_speed_();
  uint8_t temperature_();
  uint8_t preset_();

  // Returns the effective byte-4 nibble for each axis.
  // The per-axis select overrides the coarse swing_mode entity when set
  // to anything other than the "follow swing_mode" default.
  uint8_t effective_swing_v_();
  uint8_t effective_swing_h_();

  Model   model_{GREE_GENERIC};
  uint8_t mode_bits_{GREE_LIGHT_BIT};  // Light ON by default

  // Per-axis position state — 0xFF means "follow swing_mode entity"
  uint8_t swing_v_{0xFF};
  uint8_t swing_h_{0xFF};
};

}  // namespace esphome::gree