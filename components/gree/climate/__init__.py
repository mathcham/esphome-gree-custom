import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate_ir
from esphome.components.gree import gree_ns, GreeClimate, MODELS

AUTO_LOAD = ["climate_ir"]

CONF_MODEL = "model"

# Extend the standard climate_ir schema (handles transmitter_id, receiver_id, etc.)
CONFIG_SCHEMA = climate_ir.climate_ir_with_receiver_schema(GreeClimate).extend(
    {
        cv.Required(CONF_MODEL): cv.enum(MODELS, lower=True),
    }
)


async def to_code(config):
    var = await climate_ir.new_climate_ir(config)
    cg.add(var.set_model(config[CONF_MODEL]))
