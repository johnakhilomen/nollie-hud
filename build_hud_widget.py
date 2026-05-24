"""
Build WBP_NollieHUD widget blueprint with all BindWidget components.
Run via UE5 Execute Python Script or MCP ue5_run in ExecuteFile mode.
"""
import unreal

PKG  = '/Game/NollieStandard'
NAME = 'WBP_NollieHUD'
PATH = f'{PKG}/{NAME}'

# ── 1. Create (or open) the WidgetBlueprint ───────────────────────────────────

unreal.EditorAssetLibrary.make_directory(PKG)

if unreal.EditorAssetLibrary.does_asset_exist(PATH):
    bp = unreal.EditorAssetLibrary.load_asset(PATH)
    unreal.log(f'Loaded existing blueprint: {bp}')
else:
    # Try to set parent class to NollieVerdictWidget
    factory = unreal.WidgetBlueprintFactory()
    nollie_class = unreal.load_class(None, '/Script/NollieStandard.NollieVerdictWidget')
    if nollie_class:
        factory.set_editor_property('parent_class', nollie_class)
        unreal.log(f'Parent class set to NollieVerdictWidget')
    else:
        unreal.log_warning('NollieVerdictWidget not found — creating with UserWidget parent. Set parent manually.')

    at = unreal.AssetToolsHelpers.get_asset_tools()
    bp = at.create_asset(NAME, PKG, unreal.WidgetBlueprint, factory)
    unreal.log(f'Created blueprint: {bp}')

if not bp:
    unreal.log_error('Failed to create/load blueprint!')
    raise SystemExit

# ── 2. Get the widget tree ────────────────────────────────────────────────────

wt = unreal.load_object(None, f'{PATH}.{NAME}:WidgetTree')
if not wt:
    # Fallback: iterate sub-objects
    for sub in unreal.BlueprintEditorLibrary.get_blueprint_subobjects(bp) if hasattr(unreal, 'BlueprintEditorLibrary') else []:
        if 'WidgetTree' in str(type(sub)):
            wt = sub
            break

if not wt:
    unreal.log_error('Cannot access WidgetTree — trying editor approach')
    # Open the blueprint editor which may expose the tree
    unreal.AssetEditorSubsystem and unreal.get_editor_subsystem(unreal.AssetEditorSubsystem).open_editor_for_assets([bp])
    import time; time.sleep(1.0)
    wt = unreal.load_object(None, f'{PATH}.{NAME}:WidgetTree')

unreal.log(f'WidgetTree: {wt}')
if not wt:
    unreal.log_error('WidgetTree not accessible — cannot add widgets programmatically in UE5.7')
    unreal.log('MANUAL STEPS NEEDED — see log for widget names to add')
    # Print the required widget names
    widgets_needed = [
        ('CanvasPanel',       'VerdictCard'),
        ('EditableTextBox',   'SearchBox'),
        ('TextBlock',         'CompanyNameText'),
        ('Border',            'VerdictBadge'),
        ('TextBlock',         'VerdictText'),
        ('TextBlock',         'ScoreText'),
        ('ProgressBar',       'ScoreBar'),
        ('TextBlock',         'GovernanceScore'),
        ('ProgressBar',       'GovernanceBar'),
        ('TextBlock',         'GovernanceFinding'),
        ('TextBlock',         'ProvenanceScore'),
        ('ProgressBar',       'ProvenanceBar'),
        ('TextBlock',         'ProvenanceFinding'),
        ('TextBlock',         'ShadowITScore'),
        ('ProgressBar',       'ShadowITBar'),
        ('TextBlock',         'ShadowITFinding'),
        ('TextBlock',         'DriftScore'),
        ('ProgressBar',       'DriftBar'),
        ('TextBlock',         'DriftFinding'),
        ('TextBlock',         'StatusText'),
    ]
    unreal.log('=== WIDGETS TO ADD (type → exact name required) ===')
    for wtype, wname in widgets_needed:
        unreal.log(f'  {wtype:20s}  →  {wname}')
    raise SystemExit

# ── 3. Add widgets to the tree ────────────────────────────────────────────────

def add_widget(widget_class, name, parent=None):
    w = wt.construct_widget(widget_class, name)
    if parent:
        parent.add_child(w) if hasattr(parent, 'add_child') else None
    return w

root_canvas = wt.root_widget
if not root_canvas:
    root_canvas = add_widget(unreal.CanvasPanel, 'CanvasPanel_Root')
    wt.set_editor_property('root_widget', root_canvas)

# Add all BindWidget components at root level
add_widget(unreal.EditableTextBox, 'SearchBox', root_canvas)
verdict_card = add_widget(unreal.CanvasPanel, 'VerdictCard', root_canvas)
add_widget(unreal.TextBlock,    'CompanyNameText',   verdict_card)
add_widget(unreal.Border,       'VerdictBadge',      verdict_card)
add_widget(unreal.TextBlock,    'VerdictText',       verdict_card)
add_widget(unreal.TextBlock,    'ScoreText',         verdict_card)
add_widget(unreal.ProgressBar,  'ScoreBar',          verdict_card)
add_widget(unreal.TextBlock,    'GovernanceScore',   verdict_card)
add_widget(unreal.ProgressBar,  'GovernanceBar',     verdict_card)
add_widget(unreal.TextBlock,    'GovernanceFinding', verdict_card)
add_widget(unreal.TextBlock,    'ProvenanceScore',   verdict_card)
add_widget(unreal.ProgressBar,  'ProvenanceBar',     verdict_card)
add_widget(unreal.TextBlock,    'ProvenanceFinding', verdict_card)
add_widget(unreal.TextBlock,    'ShadowITScore',     verdict_card)
add_widget(unreal.ProgressBar,  'ShadowITBar',       verdict_card)
add_widget(unreal.TextBlock,    'ShadowITFinding',   verdict_card)
add_widget(unreal.TextBlock,    'DriftScore',        verdict_card)
add_widget(unreal.ProgressBar,  'DriftBar',          verdict_card)
add_widget(unreal.TextBlock,    'DriftFinding',      verdict_card)
add_widget(unreal.TextBlock,    'StatusText',        root_canvas)

unreal.EditorAssetLibrary.save_asset(PATH)
unreal.log(f'WBP_NollieHUD built and saved successfully!')
