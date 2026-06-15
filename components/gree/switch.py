import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch
from esphome.components.gree import GreeClimate, gree_ns

AUTO_LOAD = ["switch"]

GreeSwitch = gree_ns.class_("GreeSwitch", switch.Switch, cg.Component)

CONF_GREE_ID = "gree_id"
CONF_TURBO   = "turbo"
CONF_LIGHT   = "light"
CONF_HEALTH  = "health"
CONF_XFAN    = "xfan"

# Must match the bit constants in gree.h exactly
GREE_TURBO_BIT  = 0x10
GREE_LIGHT_BIT  = 0x20
GREE_HEALTH_BIT = 0x40
GREE_XFAN_BIT   = 0x80


def _switch_schema():
    return switch.switch_schema(GreeSwitch).extend(cv.COMPONENT_SCHEMA)


CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_GREE_ID): cv.use_id(GreeClimate),
        cv.Optional(CONF_TURBO):  _switch_schema(),
        cv.Optional(CONF_LIGHT):  _switch_schema(),
        cv.Optional(CONF_HEALTH): _switch_schema(),
        cv.Optional(CONF_XFAN):   _switch_schema(),
    }
)


async def _make_switch(config, climate_var, bit_mask: int):
    var = await switch.new_switch(config)
    await cg.register_component(var, config)
    cg.add(var.set_climate(climate_var))
    cg.add(var.set_bit_mask(bit_mask))
    return var


async def to_code(config):
    climate_var = await cg.get_variable(config[CONF_GREE_ID])
    if CONF_TURBO  in config:
        await _make_switch(config[CONF_TURBO],  climate_var, GREE_TURBO_BIT)
    if CONF_LIGHT  in config:
        await _make_switch(config[CONF_LIGHT],  climate_var, GREE_LIGHT_BIT)
    if CONF_HEALTH in config:
        await _make_switch(config[CONF_HEALTH], climate_var, GREE_HEALTH_BIT)
    if CONF_XFAN   in config:
        await _make_switch(config[CONF_XFAN],   climate_var, GREE_XFAN_BIT)
