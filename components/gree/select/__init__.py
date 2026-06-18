import esphome.codegen as cg
from esphome.components import select
import esphome.config_validation as cv
from esphome.const import CONF_ID

from .. import GreeClimate, GreeSwingSelect

CODEOWNERS = ["@mathcham"]

CONF_GREE_ID          = "gree_id"
CONF_SWING_VERTICAL   = "vertical_swing"
CONF_SWING_HORIZONTAL = "horizontal_swing"

# Labels shown in Home Assistant.
# Vertical:   1=Up  2=Mid-up   3=Middle  4=Mid-down  5=Down
# Horizontal: 1=Left 2=Mid-left 3=Center 4=Mid-right 5=Right
SWING_OPTIONS = ["Off", "Swing", "1", "2", "3", "4", "5"]

CONFIG_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_GREE_ID): cv.use_id(GreeClimate),
        cv.Optional(CONF_SWING_VERTICAL): select.select_schema(
            GreeSwingSelect,
            icon="mdi:arrow-up-down",
        ),
        cv.Optional(CONF_SWING_HORIZONTAL): select.select_schema(
            GreeSwingSelect,
            icon="mdi:arrow-left-right",
        ),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_GREE_ID])

    if CONF_SWING_VERTICAL in config:
        conf = config[CONF_SWING_VERTICAL]
        sel = cg.new_Pvariable(conf[CONF_ID])
        await select.register_select(sel, conf, options=SWING_OPTIONS)
        await cg.register_component(sel, conf)
        await cg.register_parented(sel, parent)
        cg.add(sel.set_is_vertical(True))

    if CONF_SWING_HORIZONTAL in config:
        conf = config[CONF_SWING_HORIZONTAL]
        sel = cg.new_Pvariable(conf[CONF_ID])
        await select.register_select(sel, conf, options=SWING_OPTIONS)
        await cg.register_component(sel, conf)
        await cg.register_parented(sel, parent)
        cg.add(sel.set_is_vertical(False))
