#include "wavinAhc9000_climate.h"
#include "esphome/core/log.h"

namespace esphome {
namespace wavinAhc9000 {

static const char *TAG = "wavinAhc9000.climate";

void WavinAhc9000Climate::setup() {
  wavin_->add_temp_callback(channel_, [this](float state) {
    if (current_temperature != state) {
      current_temperature = state;
      publish_state();
    }
  });
  wavin_->add_target_temp_callback(channel_, [this](float state) {
    if (target_temperature != state) {
      target_temperature = state;
      publish_state();
    }
  });
  wavin_->add_output_callback(channel_, [this](bool is_on) {
    //if (is_on && (mode != climate::CLIMATE_MODE_HEAT)) {
    //  mode = climate::CLIMATE_MODE_HEAT;
    //  publish_state();
    //} else if (!is_on && (mode != climate::CLIMATE_MODE_OFF)) {
    //  mode = climate::CLIMATE_MODE_OFF;
    //  publish_state();
    //}
  });
  wavin_->add_mode_callback(channel_, [this](bool is_on) {
    if (is_on && (mode != climate::CLIMATE_MODE_OFF)) {
      mode = climate::CLIMATE_MODE_OFF;
      publish_state();
    } else if (!is_on && (mode != climate::CLIMATE_MODE_AUTO)) {
      mode = climate::CLIMATE_MODE_AUTO;
      publish_state();
    }
  });
  if (battery_level_sensor_ != nullptr) {
    wavin_->add_bat_level_callback(channel_, [this](int level) {
      battery_level_sensor_->publish_state(level);
    });
  }
  if (current_temp_sensor_ != nullptr) {
    wavin_->add_temp_callback(channel_, [this](float state) {
      current_temp_sensor_->publish_state(state);
    });
  }  
}

void WavinAhc9000Climate::control(const climate::ClimateCall &call) {
  ESP_LOGD(TAG, "CONTROL - SETTINGS CHANGED IN HA");
  if (call.get_target_temperature().has_value()) {
    target_temperature = *call.get_target_temperature();
    float target = target_temperature;
    ESP_LOGD(TAG, "Target temperature for channel %d changed to: %f", channel_ + 1, target);
    wavin_->set_target_temp(channel_, target);
    target_temperature = target;
    publish_state();
  }

  if (call.get_mode().has_value()) {
    ESP_LOGD(TAG, "CONTROL - MODE WAS CHANGED IN HA");
    auto new_mode = *call.get_mode();
    //mode = new_mode;
    int operation_mode = climatemode_to_wavinmode(new_mode);
    ESP_LOGD(TAG, "Operation mode changed to: %d", operation_mode);
    wavin_->set_mode(channel_, operation_mode);
    
  }
}

climate::ClimateTraits WavinAhc9000Climate::traits() {
  auto traits = climate::ClimateTraits();
  traits.set_supported_modes({
    climate::ClimateMode::CLIMATE_MODE_OFF,
    climate::ClimateMode::CLIMATE_MODE_AUTO,
  });  
  traits.set_supports_current_temperature(true);

  return traits;
}

int WavinAhc9000Climate::climatemode_to_wavinmode(const climate::ClimateMode mode)
{
  int return_value;

  switch (mode) {
    case climate::CLIMATE_MODE_OFF: return_value = 1; break;
    case climate::CLIMATE_MODE_AUTO: return_value = 0; break;
    //default: return_value = 4; break;
  }

  return return_value;
}

void WavinAhc9000Climate::dump_config() {
  LOG_CLIMATE("", "WavinAhc9000 Climate", this);
}

}  // namespace wavinAhc9000
}  // namespace esphome
