import esphome.codegen as cg
from esphome.components import select, switch

CODEOWNERS = ["@mathcham"]

gree_ns = cg.esphome_ns.namespace("gree")

GreeClimate     = gree_ns.class_("GreeClimate",     cg.Component)
GreeSwingSelect = gree_ns.class_("GreeSwingSelect", select.Select, cg.Component)
GreeModeSwitch  = gree_ns.class_("GreeModeSwitch",  switch.Switch, cg.Component)
