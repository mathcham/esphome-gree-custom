import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate_ir, select
from esphome.const import CONF_ID, CONF_NAME

AUTO_LOAD = ["climate_ir", "select"]

gree_ns = cg.esphome_ns.namespace("gree")
GreeClimate     = gree_ns.class_("GreeClimate",     climate_ir.ClimateIR)
GreeSwingSelect = gree_ns.class_("GreeSwingSelect", select.Select, cg.Component)

Model = gree_ns.enum("Model")
MODELS = {
    "generic": Model.GREE_GENERIC,
    "yan":     Model.GREE_YAN,
    "yaa":     Model.GREE_YAA,
    "yac":     Model.GREE_YAC,
    "yac1fb9": Model.GREE_YAC1FB9,
    "yx1ff":   Model.GREE_YX1FF,
    "yag":     Model.GREE_YAG,
}

CONF_MODEL        = "model"
CONF_GREE_ID      = "gree_id"
CONF_VERTICAL     = "vertical_swing"
CONF_HORIZONTAL   = "horizontal_swing"

# ── Swing position option list ────────────────────────────────────────────────
SWING_OPTIONS = ["Off", "Swing", "1", "2", "3", "4", "5"]

# ── Climate platform schema ───────────────────────────────────────────────────
CONFIG_SCHEMA = climate_ir.climate_ir_with_receiver_schema(GreeClimate).extend(
    {
        cv.Required(CONF_MODEL): cv.enum(MODELS, lower=True),
    }
)

async def to_code(config):
    var = await climate_ir.new_climate_ir(config)
    cg.add(var.set_model(config[CONF_MODEL]))


# ── Select sub-platform schema ────────────────────────────────────────────────
# Usage in YAML:
#   select:
#     - platform: gree
#       gree_id: my_gree_ac
#       vertical_swing:
#         name: "AC Vertical Swing"
#       horizontal_swing:
#         name: "AC Horizontal Swing"

def _swing_select_schema():
    return select.select_schema(GreeSwingSelect).extend(cv.COMPONENT_SCHEMA)

SELECT_CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_GREE_ID): cv.use_id(GreeClimate),
        cv.Optional(CONF_VERTICAL):   _swing_select_schema(),
        cv.Optional(CONF_HORIZONTAL): _swing_select_schema(),
    }
)

async def _new_swing_select(config, climate_var, vertical: bool):
    var = await select.new_select(config, options=SWING_OPTIONS)
    await cg.register_component(var, config)
    cg.add(var.set_climate(climate_var))
    cg.add(var.set_axis(vertical))
    return var

async def select_to_code(config):
    climate_var = await cg.get_variable(config[CONF_GREE_ID])
    if CONF_VERTICAL in config:
        await _new_swing_select(config[CONF_VERTICAL], climate_var, vertical=True)
    if CONF_HORIZONTAL in config:
        await _new_swing_select(config[CONF_HORIZONTAL], climate_var, vertical=False)
