"""
Diagnostic — dumps all attributes on WidgetBlueprint and checks
available Python classes related to widget creation in UE5.7
"""
import unreal

PKG   = '/Game/NollieStandard'
ASSET = 'WBP_Diag'

def diagnose():
    unreal.EditorAssetLibrary.make_directory(PKG)
    path = f'{PKG}/{ASSET}'
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        unreal.EditorAssetLibrary.delete_asset(path)

    at = unreal.AssetToolsHelpers.get_asset_tools()
    bp = at.create_asset(ASSET, PKG, unreal.WidgetBlueprint,
                         unreal.WidgetBlueprintFactory())

    if not bp:
        unreal.log_error("Could not create WidgetBlueprint")
        return

    unreal.log(f"Blueprint type: {type(bp)}")
    unreal.log("=== ALL ATTRS ===")
    for a in sorted(dir(bp)):
        unreal.log(f"  {a}")

    # Also check what editor properties exist
    unreal.log("=== EDITOR PROPERTIES ===")
    try:
        props = bp.get_editor_property_names() if hasattr(bp, 'get_editor_property_names') else []
        for p in props:
            unreal.log(f"  {p}")
    except Exception as e:
        unreal.log_warning(f"get_editor_property_names: {e}")

    # Check for WidgetBlueprintLibrary
    unreal.log("=== WidgetBlueprintLibrary methods ===")
    for a in sorted(dir(unreal.WidgetBlueprintLibrary)):
        unreal.log(f"  {a}")

diagnose()
