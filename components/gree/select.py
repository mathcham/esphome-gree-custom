import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import select
from esphome.components.gree import GreeClimate, GreeSwingSelect

AUTO_LOAD = ["select"]

CONF_GREE_ID    = "gree_id"
CONF_VERTICAL   = "vertical_swing"
CONF_HORIZONTAL = "horizontal_swing"

# Must match the SWING_OPT_* constants in gree.h exactly
SWING_OPTIONS = ["Off", "Swing", "1", "2", "3", "4", "5"]


def _swing_select_schema():
    return select.select_schema(GreeSwingSelect).extend(cv.COMPONENT_SCHEMA)


CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_GREE_ID): cv.use_id(GreeClimate),
        cv.Optional(CONF_VERTICAL):   _swing_select_schema(),
        cv.Optional(CONF_HORIZONTAL): _swing_select_schema(),
    }
)


async def _make_select(config, climate_var, vertical: bool):
    var = await select.new_select(config, options=SWING_OPTIONS)
    await cg.register_component(var, config)
    cg.add(var.set_climate(climate_var))
    cg.add(var.set_axis(vertical))
    return var


async def to_code(config):
    climate_var = await cg.get_variable(config[CONF_GREE_ID])
    if CONF_VERTICAL in config:
        await _make_select(config[CONF_VERTICAL], climate_var, vertical=True)
    if CONF_HORIZONTAL in config:
        await _make_select(config[CONF_HORIZONTAL], climate_var, vertical=False)
