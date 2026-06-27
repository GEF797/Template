import unreal

ASSET_PATH = "/Game"
ASSET_NAME = "M_SemiTransparent"
FULL_PATH = f"{ASSET_PATH}/{ASSET_NAME}"

if unreal.EditorAssetLibrary.does_asset_exist(FULL_PATH):
    unreal.EditorAssetLibrary.delete_asset(FULL_PATH)
    unreal.log(f"Removed existing material: {FULL_PATH}")

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
factory = unreal.MaterialFactoryNew()
material = asset_tools.create_asset(ASSET_NAME, ASSET_PATH, unreal.Material, factory)
if not material:
    raise RuntimeError(f"Failed to create material at {FULL_PATH}")

material.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
material.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_DEFAULT_LIT)

base_color = unreal.MaterialEditingLibrary.create_material_expression(
    material, unreal.MaterialExpressionConstant3Vector, -300, -100
)
base_color.set_editor_property(
    "constant", unreal.LinearColor(r=0.2, g=0.6, b=1.0, a=1.0)
)

opacity = unreal.MaterialEditingLibrary.create_material_expression(
    material, unreal.MaterialExpressionConstant, -300, 100
)
opacity.set_editor_property("r", 0.5)

unreal.MaterialEditingLibrary.connect_material_property(
    base_color, "", unreal.MaterialProperty.MP_BASE_COLOR
)
unreal.MaterialEditingLibrary.connect_material_property(
    opacity, "", unreal.MaterialProperty.MP_OPACITY
)

unreal.MaterialEditingLibrary.recompile_material(material)
unreal.EditorAssetLibrary.save_loaded_asset(material, only_if_is_dirty=False)
unreal.log(f"Created semi-transparent material: {FULL_PATH}")
