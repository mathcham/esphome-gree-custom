import esphome.codegen as cg

gree_ns = cg.esphome_ns.namespace("gree")

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

GreeClimate     = gree_ns.class_("GreeClimate",     cg.Component)
GreeSwingSelect = gree_ns.class_("GreeSwingSelect", cg.Component)