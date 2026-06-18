import esphome.codegen as cg
from esphome.components import switch
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_LIGHT, DEVICE_CLASS_SWITCH, ENTITY_CATEGORY_CONFIG

from .. import GreeClimate, GreeModeSwitch

CODEOWNERS = ["@mathcham"]

CONF_GREE_ID = "gree_id"
CONF_TURBO   = "turbo"
CONF_HEALTH  = "health"
CONF_XFAN    = "xfan"

SWITCH_CONFIGS = (
    (CONF_TURBO,  0x10, "mdi:car-turbocharger"),
    (CONF_LIGHT,  0x20, "mdi:led-outline"),
    (CONF_HEALTH, 0x40, "mdi:pine-tree"),
    (CONF_XFAN,   0x80, "mdi:wall-sconce-flat"),
)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_GREE_ID): cv.use_id(GreeClimate),
        **{
            cv.Optional(key): switch.switch_schema(
                GreeModeSwitch,
                icon=icon,
                default_restore_mode="RESTORE_DEFAULT_OFF",
                device_class=DEVICE_CLASS_SWITCH,
                entity_category=ENTITY_CATEGORY_CONFIG,
            )
            for key, _, icon in SWITCH_CONFIGS
        },
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_GREE_ID])
    for conf_key, bit_mask, _ in SWITCH_CONFIGS:
        if sw_conf := config.get(conf_key):
            sw = cg.new_Pvariable(sw_conf[CONF_ID], bit_mask)
            await switch.register_switch(sw, sw_conf)
            await cg.register_component(sw, sw_conf)
            await cg.register_parented(sw, parent)
