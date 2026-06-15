"""Gree climate IR component — shared namespace and model enum.

Platform implementations live in:
  components/gree/climate/__init__.py   ← climate platform
  components/gree/select/__init__.py    ← swing position selects
"""
import esphome.codegen as cg

gree_ns = cg.esphome_ns.namespace("gree")

# Model enum — shared between the climate and select platforms
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

# Classes — imported by both sub-platforms
GreeClimate     = gree_ns.class_("GreeClimate",     cg.Component)
GreeSwingSelect = gree_ns.class_("GreeSwingSelect", cg.Component)
