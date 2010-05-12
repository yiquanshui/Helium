#include "Precompile.h"
#include "SceneEditor.h"

#include "ClueCreateTool.h"
#include "ControllerCreateTool.h"
#include "CurveCreateTool.h"
#include "CurveEditTool.h"
#include "Drawer.h"
#include "DrawerPanel.h"
#include "DuplicateTool.h"
#include "EntityAssetOutliner.h"
#include "EntityAssetSet.h"
#include "EntityCreateTool.h"
#include "EntityType.h"
#include "ExportOptionsDlg.h"
#include "GameCameraCreateTool.h"
#include "HierarchyNodeType.h"
#include "HierarchyOutliner.h"
#include "ImportOptionsDlg.h"
#include "InstanceCodeSet.h"
#include "Layer.h"
#include "LightCreateTool.h"
#include "LightingPanel.h"
#include "LightingTool.h"
#include "LiveLinkPanel.h"
#include "LocatorCreateTool.h"
#include "NavMeshCreateTool.h"
#include "NodeTypeOutliner.h"
#include "Point.h"
#include "PostProcessingVolumeCreateTool.h"
#include "RegionsPanel.h"
#include "RemoteConstruct.h"
#include "RemoteScene.h"
#include "RotateManipulator.h"
#include "RuntimeClassOutliner.h"
#include "ScaleManipulator.h"
#include "SceneCallbackData.h"
#include "ScenePreferences.h"
#include "ScenePreferencesDialog.h"
#include "ScenesPanel.h"
#include "SearchBar.h"
#include "SelectionPropertiesPanel.h"
#include "TranslateManipulator.h"
#include "ToolsPanel.h"
#include "TypeGrid.h"
#include "View.h"
#include "VolumeCreateTool.h"
#include "Browser/BrowserToolBar.h"
#include "Mesh.h"

#include "Asset/ArtFileAttribute.h"
#include "Asset/AssetClass.h"
#include "Asset/WorldFileAttribute.h"
#include "Asset/SceneManifest.h"
#include "Attribute/AttributeHandle.h"
#include "Common/Container/Insert.h" 
#include "Console/Console.h"
#include "Content/ContentVersion.h"
#include "Editor/MRUData.h"
#include "Editor/SessionManager.h"
#include "File/Manager.h"
#include "FileBrowser/FileBrowser.h"
#include "FileSystem/FileSystem.h"
#include "Finder/AssetSpecs.h"
#include "Finder/ExtensionSpecs.h"
#include "Finder/Finder.h"
#include "Inspect/Control.h"
#include "Inspect/ClipboardFileList.h"
#include "Inspect/ClipboardDataObject.h"
#include "Live/LiveManager.h"
#include "Symbol/SymbolBuilder.h"
#include "Task/Build.h"
#include "UIToolKit/FileDialog.h"
#include "UIToolKit/ImageManager.h"
#include "UIToolKit/SortTreeCtrl.h"
#include "Undo/PropertyCommand.h"
#include "Windows/Clipboard.h"
#include "Windows/Process.h"

#include "Content/PostEffectsColorCorrectionAttribute.h"
#include "Content/PostEffectsLightScatteringAttribute.h"
#include "Content/PostEffectsCurveControlAttribute.h"
#include "Content/PostEffectsDepthOfFieldAttribute.h"
#include "Content/PostEffectsFilmGrainAttribute.h"
#include "Content/PostEffectsBloomAttribute.h"
#include "Content/PostEffectsColorAttribute.h"
#include "Content/PostEffectsFogAttribute.h"
#include "Content/PostEffectsHDRAttribute.h"

#include "LightScattering.h"

#include "Content/Scene.h"

#include "BuilderUtil/ColorPalette.h"
#include "BuilderUtil/BuilderUtil.h"

#include <algorithm>
#include <boost/algorithm/string.hpp>

#include <wx/progdlg.h>
#include <wx/string.h>

// Uncomment this line to remove the tree controls from the GUI.
// Useful for debugging if slowdown is occurring because of them.
//#define LUNA_SCENE_DISABLE_OUTLINERS

// Using
using namespace Luna;

// Event handlers
BEGIN_EVENT_TABLE(SceneEditor, Editor)

  EVT_ERASE_BACKGROUND(SceneEditor::OnEraseBackground)
  EVT_SIZE(SceneEditor::OnSize)
  EVT_CHAR(SceneEditor::OnChar)
  EVT_SHOW(SceneEditor::OnShow)

  EVT_MENU_OPEN( SceneEditor::OnMenuOpen )
  EVT_MENU(wxID_NEW, SceneEditor::OnNew)
  EVT_MENU(wxID_OPEN, SceneEditor::OnOpen)
  EVT_MENU(SceneEditorIDs::ID_FileFind, SceneEditor::OnFind)
  EVT_MENU(wxID_SAVE, SceneEditor::OnSaveAll)
  EVT_MENU(SceneEditorIDs::ID_FileImport, SceneEditor::OnImport)
  EVT_MENU(SceneEditorIDs::ID_FileImportFromClipboard, SceneEditor::OnImport)
  EVT_MENU(SceneEditorIDs::ID_FileExport, SceneEditor::OnExport)
  EVT_MENU(SceneEditorIDs::ID_FileExportToClipboard, SceneEditor::OnExport)
  EVT_MENU(SceneEditorIDs::ID_FileExportToObj, SceneEditor::OnExportToObj)
  EVT_MENU(wxID_CLOSE, SceneEditor::OnClose)
  EVT_MENU(SceneEditorIDs::ID_FileOpenSession, SceneEditor::OnOpenSession)
  EVT_MENU(SceneEditorIDs::ID_FileSaveSession, SceneEditor::OnSaveSession)
  EVT_MENU(SceneEditorIDs::ID_FileSaveSessionAs, SceneEditor::OnSaveSessionAs)
  EVT_MENU(wxID_EXIT, SceneEditor::OnExit)
  EVT_CLOSE( SceneEditor::OnExiting )
  EVT_MENU(wxID_UNDO, SceneEditor::OnUndo)
  EVT_MENU(wxID_REDO, SceneEditor::OnRedo)
  EVT_MENU(wxID_CUT, SceneEditor::OnCut)
  EVT_MENU(wxID_COPY, SceneEditor::OnCopy)
  EVT_MENU(wxID_PASTE, SceneEditor::OnPaste)
  EVT_MENU(wxID_DELETE, SceneEditor::OnDelete)
  EVT_MENU(wxID_HELP_INDEX, SceneEditor::OnHelpIndex)
  EVT_MENU(wxID_HELP_SEARCH, SceneEditor::OnHelpSearch)

  EVT_MENU(SceneEditorIDs::ID_EditParent, SceneEditor::OnParent)
  EVT_MENU(SceneEditorIDs::ID_EditUnparent, SceneEditor::OnUnparent)
  EVT_MENU(SceneEditorIDs::ID_EditGroup, SceneEditor::OnGroup)
  EVT_MENU(SceneEditorIDs::ID_EditUngroup, SceneEditor::OnUngroup)
  EVT_MENU(SceneEditorIDs::ID_EditCenter, SceneEditor::OnCenter)
  EVT_MENU(SceneEditorIDs::ID_EditInvertSelection, SceneEditor::OnInvertSelection)
  EVT_MENU(SceneEditorIDs::ID_EditSelectAll, SceneEditor::OnSelectAll)
  EVT_MENU(SceneEditorIDs::ID_EditDuplicate, SceneEditor::OnDuplicate)
  EVT_MENU(SceneEditorIDs::ID_EditSmartDuplicate, SceneEditor::OnSmartDuplicate)
  EVT_MENU(SceneEditorIDs::ID_EditCopyTransform, SceneEditor::OnCopyTransform)
  EVT_MENU(SceneEditorIDs::ID_EditPasteTransform, SceneEditor::OnPasteTransform)
  EVT_MENU(SceneEditorIDs::ID_EditSnapToCamera, SceneEditor::OnSnapToCamera)
  EVT_MENU(SceneEditorIDs::ID_EditSnapCameraTo, SceneEditor::OnSnapCameraTo)
  EVT_MENU(SceneEditorIDs::ID_EditWalkUp, SceneEditor::OnPickWalk)
  EVT_MENU(SceneEditorIDs::ID_EditWalkDown, SceneEditor::OnPickWalk)
  EVT_MENU(SceneEditorIDs::ID_EditWalkForward, SceneEditor::OnPickWalk)
  EVT_MENU(SceneEditorIDs::ID_EditWalkBackward, SceneEditor::OnPickWalk)
  EVT_MENU(SceneEditorIDs::ID_EditPreferences, SceneEditor::OnEditPreferences)

  EVT_MENU(SceneEditorIDs::ID_ViewAxes, SceneEditor::OnViewChange)
  EVT_MENU(SceneEditorIDs::ID_ViewGrid, SceneEditor::OnViewChange)
  EVT_MENU(SceneEditorIDs::ID_ViewBounds, SceneEditor::OnViewChange)
  EVT_MENU(SceneEditorIDs::ID_ViewStatistics, SceneEditor::OnViewChange)

  EVT_MENU(SceneEditorIDs::ID_ViewNone, SceneEditor::OnViewChange)
  EVT_MENU(SceneEditorIDs::ID_ViewRender, SceneEditor::OnViewChange)
  EVT_MENU(SceneEditorIDs::ID_ViewCollision, SceneEditor::OnViewChange)
  EVT_MENU(SceneEditorIDs::ID_ViewPathfinding, SceneEditor::OnViewChange)

  EVT_MENU(SceneEditorIDs::ID_ViewWireframeOnMesh, SceneEditor::OnViewChange)
  EVT_MENU(SceneEditorIDs::ID_ViewWireframeOnShaded, SceneEditor::OnViewChange)
  EVT_MENU(SceneEditorIDs::ID_ViewWireframe, SceneEditor::OnViewChange)
  EVT_MENU(SceneEditorIDs::ID_ViewMaterial, SceneEditor::OnViewChange)
  EVT_MENU(SceneEditorIDs::ID_ViewTexture, SceneEditor::OnViewChange)

  EVT_MENU(SceneEditorIDs::ID_ViewFrustumCulling, SceneEditor::OnViewChange)
  EVT_MENU(SceneEditorIDs::ID_ViewBackfaceCulling, SceneEditor::OnViewChange)

  EVT_MENU(SceneEditorIDs::ID_ViewOrbit, SceneEditor::OnViewCameraChange)
  EVT_MENU(SceneEditorIDs::ID_ViewFront, SceneEditor::OnViewCameraChange)
  EVT_MENU(SceneEditorIDs::ID_ViewSide, SceneEditor::OnViewCameraChange)
  EVT_MENU(SceneEditorIDs::ID_ViewTop, SceneEditor::OnViewCameraChange)

  EVT_MENU(SceneEditorIDs::ID_ViewShowAll, SceneEditor::OnViewVisibleChange)
  EVT_MENU(SceneEditorIDs::ID_ViewShowAllGeometry, SceneEditor::OnViewVisibleChange)
  EVT_MENU(SceneEditorIDs::ID_ViewShowSelected, SceneEditor::OnViewVisibleChange)
  EVT_MENU(SceneEditorIDs::ID_ViewShowSelectedGeometry, SceneEditor::OnViewVisibleChange)
  EVT_MENU(SceneEditorIDs::ID_ViewShowUnrelated, SceneEditor::OnViewVisibleChange)
  EVT_MENU(SceneEditorIDs::ID_ViewShowLastHidden, SceneEditor::OnViewVisibleChange)

  EVT_MENU(SceneEditorIDs::ID_ViewHideAll, SceneEditor::OnViewVisibleChange)
  EVT_MENU(SceneEditorIDs::ID_ViewHideAllGeometry, SceneEditor::OnViewVisibleChange)
  EVT_MENU(SceneEditorIDs::ID_ViewHideSelected, SceneEditor::OnViewVisibleChange)
  EVT_MENU(SceneEditorIDs::ID_ViewHideSelectedGeometry, SceneEditor::OnViewVisibleChange)
  EVT_MENU(SceneEditorIDs::ID_ViewHideUnrelated, SceneEditor::OnViewVisibleChange)

  EVT_MENU(SceneEditorIDs::ID_ViewFrameOrigin, SceneEditor::OnFrameOrigin)
  EVT_MENU(SceneEditorIDs::ID_ViewFrameSelected, SceneEditor::OnFrameSelected)
  EVT_MENU(SceneEditorIDs::ID_ViewHighlightMode, SceneEditor::OnHighlightMode)
  EVT_MENU(SceneEditorIDs::ID_ViewPreviousView, SceneEditor::OnPreviousView)
  EVT_MENU(SceneEditorIDs::ID_ViewNextView, SceneEditor::OnNextView)

  EVT_MENU(SceneEditorIDs::ID_ViewDefaultShowLayers, SceneEditor::OnViewDefaultsChange)
  EVT_MENU(SceneEditorIDs::ID_ViewDefaultShowInstances, SceneEditor::OnViewDefaultsChange)
  EVT_MENU(SceneEditorIDs::ID_ViewDefaultShowGeometry, SceneEditor::OnViewDefaultsChange) 
  EVT_MENU(SceneEditorIDs::ID_ViewDefaultShowPointer, SceneEditor::OnViewDefaultsChange) 
  EVT_MENU(SceneEditorIDs::ID_ViewDefaultShowBounds, SceneEditor::OnViewDefaultsChange) 

  EVT_MENU(SceneEditorIDs::ID_ToolsSelect, SceneEditor::OnToolSelected)
  EVT_MENU(SceneEditorIDs::ID_ToolsScale, SceneEditor::OnToolSelected)
  EVT_MENU(SceneEditorIDs::ID_ToolsScalePivot, SceneEditor::OnToolSelected)
  EVT_MENU(SceneEditorIDs::ID_ToolsRotate, SceneEditor::OnToolSelected)
  EVT_MENU(SceneEditorIDs::ID_ToolsRotatePivot, SceneEditor::OnToolSelected)
  EVT_MENU(SceneEditorIDs::ID_ToolsTranslate, SceneEditor::OnToolSelected)
  EVT_MENU(SceneEditorIDs::ID_ToolsTranslatePivot, SceneEditor::OnToolSelected)
  EVT_MENU(SceneEditorIDs::ID_ToolsPivot, SceneEditor::OnToolSelected)

  EVT_MENU(SceneEditorIDs::ID_ToolsEntityCreate, SceneEditor::OnToolSelected)
  EVT_MENU(SceneEditorIDs::ID_ToolsVolumeCreate, SceneEditor::OnToolSelected)
  EVT_MENU(SceneEditorIDs::ID_ToolsClueCreate, SceneEditor::OnToolSelected)
#if LUNA_GAME_CAMERA
  EVT_MENU(SceneEditorIDs::ID_ToolsGameCameraCreate, SceneEditor::OnToolSelected)
#endif
  EVT_MENU(SceneEditorIDs::ID_ToolsControllerCreate, SceneEditor::OnToolSelected)
  EVT_MENU(SceneEditorIDs::ID_ToolsCurveCreate, SceneEditor::OnToolSelected)
  EVT_MENU(SceneEditorIDs::ID_ToolsCurveEdit, SceneEditor::OnToolSelected)
  EVT_MENU(SceneEditorIDs::ID_ToolsDuplicate, SceneEditor::OnToolSelected)
  EVT_MENU(SceneEditorIDs::ID_ToolsLocatorCreate, SceneEditor::OnToolSelected)
  EVT_MENU(SceneEditorIDs::ID_ToolsLightCreate, SceneEditor::OnToolSelected)
  EVT_MENU(SceneEditorIDs::ID_ToolsLighting, SceneEditor::OnToolSelected)
  EVT_MENU(SceneEditorIDs::ID_ToolsPostProcessingVolumeCreate, SceneEditor::OnToolSelected) 
  EVT_MENU(SceneEditorIDs::ID_ToolsPostProcessingVolumeScript, SceneEditor::OnToolSelected) 

  EVT_MENU(SceneEditorIDs::ID_ToolsLightingLayerCreate, SceneEditor::OnToolSelected) 
  EVT_MENU(SceneEditorIDs::ID_ToolsLightingLayerUnlink, SceneEditor::OnToolSelected) 
  EVT_MENU(SceneEditorIDs::ID_ToolsLightingLayerSelect, SceneEditor::OnToolSelected) 

  EVT_MENU(SceneEditorIDs::ID_ToolsNavMeshImport, SceneEditor::OnUtilitySelected)

  EVT_MENU(SceneEditorIDs::ID_UtilitiesConstruction, SceneEditor::OnUtilitySelected)
  EVT_MENU(SceneEditorIDs::ID_UtilitiesMeasureDistance, SceneEditor::OnUtilitySelected)
  EVT_MENU(SceneEditorIDs::ID_UtilitiesFlushSymbols, SceneEditor::OnUtilitySelected)

  EVT_MENU(SceneEditorIDs::ID_ToolsNavMeshCreate, SceneEditor::OnToolSelected)
  EVT_MENU(SceneEditorIDs::ID_ToolsNavMeshManipulate, SceneEditor::OnToolSelected)
  EVT_MENU(SceneEditorIDs::ID_ToolsNavMeshRotate, SceneEditor::OnToolSelected)
  EVT_MENU(SceneEditorIDs::ID_ToolsNavMeshWorkWithLOWRes, SceneEditor::OnToolSelected)
  EVT_MENU(SceneEditorIDs::ID_ToolsNavMeshPunchOut, SceneEditor::OnToolSelected)
  EVT_MENU(SceneEditorIDs::ID_ToolsNavMeshPunchOutTranslate, SceneEditor::OnToolSelected)
  EVT_MENU(SceneEditorIDs::ID_ToolsNavMeshPunchOutRotate, SceneEditor::OnToolSelected)
  EVT_MENU(SceneEditorIDs::ID_ToolsNavMeshPunchOutScale, SceneEditor::OnToolSelected)
  EVT_MENU(SceneEditorIDs::ID_ToolsNavMeshVertexSelect, SceneEditor::OnToolSelected)
  EVT_MENU(SceneEditorIDs::ID_ToolsNavMeshEdgeSelect, SceneEditor::OnToolSelected)
  EVT_MENU(SceneEditorIDs::ID_ToolsNavMeshTriSelect, SceneEditor::OnToolSelected)
  


END_EVENT_TABLE()


// Specifies the files that can be opened by the Scene Editor
Finder::FilterSpec SceneEditor::s_Filter( "SceneEditor::s_Filter", "All valid files" );

// Mapping between CameraMode and SceneEditorID
SceneEditor::RM_CamModeToSceneID SceneEditor::s_CameraModeToSceneID;


///////////////////////////////////////////////////////////////////////////////
// Wraps up a pointer to an Luna::Scene so that it can be stored in the combo box that
// is used for selecting the current scene.  Each item in the combo box stores 
// the scene that it refers to.
// 
class SceneSelectData : public wxClientData
{
public:
  Luna::Scene* m_Scene;

  SceneSelectData( Luna::Scene* scene )
  : m_Scene( scene )
  {
  }

  virtual ~SceneSelectData()
  {
  }
};

///////////////////////////////////////////////////////////////////////////////
// Creates a new Scene Editor.
// 
static Editor* CreateSceneEditor()
{
  return new SceneEditor();
}

///////////////////////////////////////////////////////////////////////////////
// Static initialization.
// 
void SceneEditor::InitializeEditor()
{
  s_Filter.AddSpec( FinderSpecs::Asset::LEVEL_DECORATION ); 
  s_Filter.AddSpec( FinderSpecs::Asset::ZONE_DECORATION );
  s_Filter.AddSpec( FinderSpecs::Asset::CONTENT_DECORATION );
  s_Filter.AddSpec( FinderSpecs::Extension::REFLECT_BINARY );

  SessionManager::GetInstance()->RegisterEditor( new EditorInfo( EditorTypes::Scene, &CreateSceneEditor, &s_Filter ) );

  s_CameraModeToSceneID.Insert( CameraModes::Orbit, SceneEditorIDs::ID_ViewOrbit );
  s_CameraModeToSceneID.Insert( CameraModes::Front, SceneEditorIDs::ID_ViewFront );
  s_CameraModeToSceneID.Insert( CameraModes::Side, SceneEditorIDs::ID_ViewSide );
  s_CameraModeToSceneID.Insert( CameraModes::Top, SceneEditorIDs::ID_ViewTop );
}

///////////////////////////////////////////////////////////////////////////////
// Static cleanup.
// 
void SceneEditor::CleanupEditor()
{
}


///////////////////////////////////////////////////////////////////////////////
// Constructor
// 
SceneEditor::SceneEditor()
: Editor( EditorTypes::Scene, NULL, wxID_ANY, wxT("Luna Scene Editor"), wxDefaultPosition, wxSize(1180, 750), wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER )
, m_SceneManager( this )
, m_DrawerPanel( NULL )
, m_HierarchyOutline( NULL )
, m_TypeOutline( NULL )
, m_EntityAssetOutline( NULL )
, m_RuntimeClassOutline( NULL )
, m_FileMenu( NULL )
, m_EditMenu( NULL )
, m_ViewMenu( NULL )
, m_ViewDefaultsMenu( NULL )
, m_PanelsMenu( NULL )
, m_HelperMenu( NULL )
, m_CameraMenu( NULL )
, m_GeometryMenu( NULL )
, m_ViewColorMenu( NULL )
, m_ShadingMenu( NULL )
, m_CullingMenu( NULL )
, m_UtilitiesMenu( NULL )
, m_MRUMenu( NULL )
, m_MRUMenuItem( NULL )
, m_MRU( new UIToolKit::MenuMRU( 30, this ) )
, m_StandardToolBar( NULL )
, m_ViewToolBar( NULL )
, m_ToolsToolBar( NULL )
, m_NavToolBar (NULL)
, m_View( NULL )
, m_HasViewerControl( false )
, m_RemoteScene( new RemoteScene( this ) )
, m_TreeMonitor( &m_SceneManager )
, m_TreeSortTimer( &m_TreeMonitor )
{
  SetMinSize(wxSize(400,300));

  wxIconBundle iconBundle;
  wxIcon tempIcon;
  tempIcon.CopyFromBitmap( UIToolKit::GlobalImageManager().GetBitmap( "scene_editor_64.png" ) );
  iconBundle.AddIcon( tempIcon );
  tempIcon.CopyFromBitmap( UIToolKit::GlobalImageManager().GetBitmap( "scene_editor_32.png" ) );
  iconBundle.AddIcon( tempIcon );
  tempIcon.CopyFromBitmap( UIToolKit::GlobalImageManager().GetBitmap( "scene_editor_16.png" ) );
  iconBundle.AddIcon( tempIcon );
  SetIcons( iconBundle );

  //
  // Attach event handlers
  //

  m_SceneManager.AddCurrentSceneChangingListener( SceneChangeSignature::Delegate (this, &SceneEditor::CurrentSceneChanging) );
  m_SceneManager.AddCurrentSceneChangedListener( SceneChangeSignature::Delegate (this, &SceneEditor::CurrentSceneChanged) );
  m_SceneManager.AddSceneAddedListener( SceneChangeSignature::Delegate( this, &SceneEditor::SceneAdded ) );
  m_SceneManager.AddSceneRemovingListener( SceneChangeSignature::Delegate( this, &SceneEditor::SceneRemoving ) );

  m_MRU->AddItemSelectedListener( UIToolKit::MRUSignature::Delegate( this, &SceneEditor::OnMRUOpen ) );

  V_string paths;
  V_string::const_iterator itr = SceneEditorPreferences()->GetMRU()->GetPaths().begin();
  V_string::const_iterator end = SceneEditorPreferences()->GetMRU()->GetPaths().end();
  for ( ; itr != end; ++itr )
  {
    std::string path = *itr;
    FileSystem::CleanName( path );
    if ( FileSystem::Exists( path ) && FileSystem::HasPrefix( Finder::ProjectAssets(), path ) )
    {
      paths.push_back( *itr );
    }
  }
  m_MRU->FromVector( paths );
 
  // 
  // Status bar
  // 

  CreateStatusBar();
  GetStatusBar()->SetStatusText(_("Ready"));


  //
  // Create toolbars
  //
  m_StandardToolBar = new wxToolBar( this, -1, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxTB_NODIVIDER );
  m_StandardToolBar->SetToolBitmapSize(wxSize(16,16));
  m_StandardToolBar->AddTool(wxID_NEW, wxT("New"), UIToolKit::GlobalImageManager().GetBitmap( "new_file_16.png" ), "Create a new scene");
  m_StandardToolBar->AddTool(wxID_OPEN, wxT("Open"), wxArtProvider::GetBitmap(wxART_FILE_OPEN, wxART_OTHER, wxSize(16,16)), "Open a scene file");
  m_StandardToolBar->AddTool(SceneEditorIDs::ID_FileFind, wxT( "Find..." ), wxArtProvider::GetBitmap( wxART_FIND, wxART_OTHER, wxSize( 16, 16 ) ) );
  m_StandardToolBar->AddTool(wxID_SAVE, wxT("Save All"), UIToolKit::GlobalImageManager().GetBitmap( "save_all_16.png" ), "Save all currently checked out scenes");
  m_StandardToolBar->AddSeparator();
  m_StandardToolBar->AddTool(wxID_CUT, wxT("Cut"), wxArtProvider::GetBitmap(wxART_CUT, wxART_OTHER, wxSize(16,16)), "Cut selection contents to the clipboard");
  m_StandardToolBar->AddTool(wxID_COPY, wxT("Copy"), wxArtProvider::GetBitmap(wxART_COPY, wxART_OTHER, wxSize(16,16)), "Copy selection contents to the clipboard");
  m_StandardToolBar->AddTool(wxID_PASTE, wxT("Paste"), wxArtProvider::GetBitmap(wxART_PASTE, wxART_OTHER, wxSize(16,16)), "Paste clipboard contents into the currrent scene");
  m_StandardToolBar->AddSeparator();
  m_StandardToolBar->AddTool(wxID_UNDO, wxT("Undo"), wxArtProvider::GetBitmap(wxART_UNDO, wxART_OTHER, wxSize(16,16)), "Undo the last operation");
  m_StandardToolBar->AddTool(wxID_REDO, wxT("Redo"), wxArtProvider::GetBitmap(wxART_REDO, wxART_OTHER, wxSize(16,16)), "Redo the last undone operation");
  m_StandardToolBar->Realize();
  
  m_ViewToolBar = new wxToolBar( this, -1, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxTB_NODIVIDER );
  m_ViewToolBar->SetToolBitmapSize(wxSize(16, 16));
  m_ViewToolBar->AddTool(SceneEditorIDs::ID_ViewOrbit, wxT("Orbit"), UIToolKit::GlobalImageManager().GetBitmap( "camera_perspective_16.png" ), "Use the orbit perspective camera");
  m_ViewToolBar->AddTool(SceneEditorIDs::ID_ViewFront, wxT("Front"), UIToolKit::GlobalImageManager().GetBitmap( "camera_front_16.png" ), "Use the front orthographic camera");
  m_ViewToolBar->AddTool(SceneEditorIDs::ID_ViewSide, wxT("Side"), UIToolKit::GlobalImageManager().GetBitmap( "camera_side_16.png" ), "Use the side orthographic camera");
  m_ViewToolBar->AddTool(SceneEditorIDs::ID_ViewTop, wxT("Top"), UIToolKit::GlobalImageManager().GetBitmap( "camera_top_16.png" ), "Use the top orthographic camera");
  m_ViewToolBar->Realize();

  m_ToolsToolBar = new wxToolBar( this, -1, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxTB_NODIVIDER );
  m_ToolsToolBar->SetToolBitmapSize(wxSize(32,32));
  m_ToolsToolBar->AddCheckTool(SceneEditorIDs::ID_ToolsSelect, wxT("Select"), UIToolKit::GlobalImageManager().GetBitmap( "select_32.png" ), wxNullBitmap, "Select items from the workspace");
  m_ToolsToolBar->AddCheckTool(SceneEditorIDs::ID_ToolsTranslate, wxT("Translate"), UIToolKit::GlobalImageManager().GetBitmap( "transform_translate_32.png" ), wxNullBitmap, "Translate items");
  m_ToolsToolBar->AddCheckTool(SceneEditorIDs::ID_ToolsRotate, wxT("Rotate"), UIToolKit::GlobalImageManager().GetBitmap( "transform_rotate_32.png" ), wxNullBitmap, "Rotate selected items");
  m_ToolsToolBar->AddCheckTool(SceneEditorIDs::ID_ToolsScale, wxT("Scale"), UIToolKit::GlobalImageManager().GetBitmap( "transform_scale_32.png" ), wxNullBitmap, "Scale selected items");
  m_ToolsToolBar->AddCheckTool(SceneEditorIDs::ID_ToolsDuplicate, wxT("Duplicate"), UIToolKit::GlobalImageManager().GetBitmap( "under_construction_32.png" ), wxNullBitmap, "Duplicate the selected object numerous times");
  m_ToolsToolBar->AddTool(SceneEditorIDs::ID_UtilitiesMeasureDistance, wxT("Measure"), UIToolKit::GlobalImageManager().GetBitmap( "measure_32.png" ), "Measure the distance between selected objects");

  m_ToolsToolBar->AddSeparator();
  m_ToolsToolBar->AddCheckTool(SceneEditorIDs::ID_ToolsEntityCreate, wxT("Entity"), UIToolKit::GlobalImageManager().GetBitmap( "create_entity_32.png" ), wxNullBitmap, "Place entity objects (such as art instances or characters)");
  m_ToolsToolBar->AddCheckTool(SceneEditorIDs::ID_ToolsVolumeCreate, wxT("Volume"), UIToolKit::GlobalImageManager().GetBitmap( "create_volume_32.png" ), wxNullBitmap, "Place volume objects (items for setting up gameplay)");
  m_ToolsToolBar->AddCheckTool(SceneEditorIDs::ID_ToolsClueCreate, wxT("Clue"), UIToolKit::GlobalImageManager().GetBitmap( "create_clue_32.png" ), wxNullBitmap, "Place clue objects (items for setting up gameplay)");
#if LUNA_GAME_CAMERA
  m_ToolsToolBar->AddCheckTool(SceneEditorIDs::ID_ToolsGameCameraCreate, wxT("GameCamera"), UIToolKit::GlobalImageManager().GetBitmap( "game_camera_32.png" ), wxNullBitmap, "Place a camera");
#endif
  m_ToolsToolBar->AddCheckTool(SceneEditorIDs::ID_ToolsControllerCreate, wxT("Controller"), UIToolKit::GlobalImageManager().GetBitmap( "create_controller_32.png" ), wxNullBitmap, "Place controller objects (items for setting up gameplay)");
  m_ToolsToolBar->AddCheckTool(SceneEditorIDs::ID_ToolsLocatorCreate, wxT("Locator"), UIToolKit::GlobalImageManager().GetBitmap( "create_locator_32.png" ), wxNullBitmap, "Place locator objects (such as bug locators)");
  m_ToolsToolBar->AddCheckTool(SceneEditorIDs::ID_ToolsCurveCreate, wxT("Curve"), UIToolKit::GlobalImageManager().GetBitmap( "create_curve_32.png" ), wxNullBitmap, "Create curve objects (Linear, B-Spline, or Catmull-Rom Spline)");
  m_ToolsToolBar->AddCheckTool(SceneEditorIDs::ID_ToolsCurveEdit, wxT("Edit Curve"), UIToolKit::GlobalImageManager().GetBitmap( "edit_curve_32.png" ), wxNullBitmap, "Edit created curves (modify or create/delete control points)");
  m_ToolsToolBar->AddCheckTool(SceneEditorIDs::ID_ToolsLightCreate, wxT("Light"), UIToolKit::GlobalImageManager().GetBitmap( "create_light_32.png" ), wxNullBitmap, "Place lights in the scene");
  m_ToolsToolBar->AddCheckTool(SceneEditorIDs::ID_ToolsLighting, wxT("Lighting"), UIToolKit::GlobalImageManager().GetBitmap( "no_smoking_32.png" ), wxNullBitmap, "Light objects in the scene");
  m_ToolsToolBar->AddCheckTool(SceneEditorIDs::ID_ToolsPostProcessingVolumeCreate, wxT("Post Processing"), UIToolKit::GlobalImageManager().GetBitmap( "create_postprocessing_volume_32.png" ), wxNullBitmap, "Place post processing volume in the scene");

  m_ToolsToolBar->AddSeparator();
  m_ToolsToolBar->AddTool(SceneEditorIDs::ID_UtilitiesConstruction, wxT("Connect to Maya"), UIToolKit::GlobalImageManager().GetBitmap( "maya_32.png" ), "Connect to Maya with the selected items for editing");
  m_ToolsToolBar->AddTool(SceneEditorIDs::ID_UtilitiesFlushSymbols, wxT("Flush Symbols"), UIToolKit::GlobalImageManager().GetBitmap( "header_32.png" ), "Flush symbol definitions (to re-parse headers)");

  m_ToolsToolBar->Realize();
  m_ToolsToolBar->ToggleTool( SceneEditorIDs::ID_ToolsSelect, true );
  m_ToolsToolBar->Disable();

  m_NavToolBar = new wxToolBar( this, -1, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxTB_NODIVIDER );
  m_NavToolBar->SetToolBitmapSize(wxSize(16,16));
  m_NavToolBar->AddCheckTool(SceneEditorIDs::ID_ToolsNavMeshImport, wxT("ImportMeshFromMayaExport"), UIToolKit::GlobalImageManager().GetBitmap( "door_in_16.png" ), wxNullBitmap, "Get the exported maya mesh into luna");
  m_NavToolBar->AddSeparator();
  m_NavToolBar->AddCheckTool(SceneEditorIDs::ID_ToolsNavMeshWorkWithLOWRes, wxT("NavMeshEditLowResMesh"), UIToolKit::GlobalImageManager().GetBitmap( "map_magnify_16.png" ), wxNullBitmap, "Work with low res nav mesh");
  m_NavToolBar->AddSeparator();
  m_NavToolBar->AddCheckTool(SceneEditorIDs::ID_ToolsNavMeshCreate, wxT("CreateNavMesh"), UIToolKit::GlobalImageManager().GetBitmap( "plugin_16.png" ), wxNullBitmap, "Create NavMesh or add new verts and tris");
  m_NavToolBar->AddCheckTool(SceneEditorIDs::ID_ToolsNavMeshManipulate, wxT("NavMeshEdit"), UIToolKit::GlobalImageManager().GetBitmap( "plugin_go_16.png" ), wxNullBitmap, "Translate Vert/Edge/Tri on NavMesh");
  m_NavToolBar->AddCheckTool(SceneEditorIDs::ID_ToolsNavMeshRotate, wxT("NavMeshRotate"), UIToolKit::GlobalImageManager().GetBitmap( "rotate_cw_16.png" ), wxNullBitmap, "Rotate Verts on NavMesh");
  m_NavToolBar->AddSeparator();
  m_NavToolBar->AddCheckTool(SceneEditorIDs::ID_ToolsNavMeshVertexSelect, wxT("VertexSelect"), UIToolKit::GlobalImageManager().GetBitmap( "vertex.png" ), wxNullBitmap, "Vertex select mode");
  m_NavToolBar->AddCheckTool(SceneEditorIDs::ID_ToolsNavMeshEdgeSelect, wxT("EdgeSelect"), UIToolKit::GlobalImageManager().GetBitmap( "edge.png" ), wxNullBitmap, "Edge select mode");
  m_NavToolBar->AddCheckTool(SceneEditorIDs::ID_ToolsNavMeshTriSelect, wxT("TriSelect"), UIToolKit::GlobalImageManager().GetBitmap( "triangle.png" ), wxNullBitmap, "Triangle select mode");
  m_NavToolBar->AddSeparator();
  m_NavToolBar->AddCheckTool(SceneEditorIDs::ID_ToolsNavMeshPunchOut, wxT("NavMeshPunchOutTool"), UIToolKit::GlobalImageManager().GetBitmap( "cube_punch_out.png" ), wxNullBitmap, "punch cube like hole in the nav mesh");
  m_NavToolBar->AddCheckTool(SceneEditorIDs::ID_ToolsNavMeshPunchOutTranslate, wxT("NavMeshPunchOutTranslate"), UIToolKit::GlobalImageManager().GetBitmap( "transform_translate_16.png" ), wxNullBitmap, "Translate punch out volume");
  m_NavToolBar->AddCheckTool(SceneEditorIDs::ID_ToolsNavMeshPunchOutRotate, wxT("NavMeshPunchOutRotate"), UIToolKit::GlobalImageManager().GetBitmap( "transform_rotate_16.png" ), wxNullBitmap, "Rotate punch out volume");
  m_NavToolBar->AddCheckTool(SceneEditorIDs::ID_ToolsNavMeshPunchOutScale, wxT("NavMeshPunchOutScale"), UIToolKit::GlobalImageManager().GetBitmap( "transform_scale_16.png" ), wxNullBitmap, "Scale punch out volume");
  m_NavToolBar->Realize();
  m_NavToolBar->Disable();


  m_BrowserToolBar = new BrowserToolBar( this );
  m_BrowserToolBar->Realize();

  //
  // Docked panes
  //

  // Drawers - REMOVED FOR NOW
  //{
  //  m_DrawerPanel = new DrawerPanel( this );

  //  wxAuiPaneInfo info;
  //  info.MinSize( GetSize().x, -1 );
  //  info.Name( wxT( "DrawerToolBar" ) );
  //  info.DestroyOnClose( false );
  //  info.Caption( wxT( "Drawers" ) );
  //  info.ToolbarPane();
  //  info.Gripper( false );
  //  info.Top();
  //  info.Floatable( false );
  //  info.BottomDockable( false );
  //  info.LeftDockable( false );
  //  info.RightDockable( false );

  //  m_FrameManager.AddPane( m_DrawerPanel, info );
  //}

  // Directory
  m_Directory = new wxNotebook (this, wxID_ANY, wxPoint(0,0), wxSize(250, 250), wxNB_NOPAGETHEME);
  m_Directory->SetImageList( UIToolKit::GlobalImageManager().GetGuiImageList() );
  {
    // Zones
    m_ZonesPanel = new ScenesPanel( this, &m_SceneManager, m_Directory, SceneEditorIDs::ID_ZonesControl );
    m_ZonesPage  = m_Directory->GetPageCount();
    m_Directory->AddPage(m_ZonesPanel, "Zones", false, UIToolKit::GlobalImageManager().GetImageIndex( "zone_16.png" ));

    m_RegionsPanel = new RegionsPanel( this, &m_SceneManager, m_Directory); 
    m_RegionsPage  = m_Directory->GetPageCount(); 
    m_Directory->AddPage( m_RegionsPanel, "Regions", false, UIToolKit::GlobalImageManager().GetImageIndex( "region_16.png" )); 

    // Inner tab with different outlines
    wxNotebook* outlinerNotebook = new wxNotebook( m_Directory, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_NOPAGETHEME );
    m_Directory->AddPage( outlinerNotebook, "Outlines", false, UIToolKit::GlobalImageManager().GetImageIndex( "type_16.png" ));
    {
#ifndef LUNA_SCENE_DISABLE_OUTLINERS
      // Types
      m_TypeOutline = new NodeTypeOutliner( &m_SceneManager );
      UIToolKit::SortTreeCtrl* typeTree = m_TypeOutline->InitTreeCtrl( outlinerNotebook, SceneEditorIDs::ID_TypeOutlineControl );
      typeTree->SetImageList( UIToolKit::GlobalImageManager().GetGuiImageList() );
      outlinerNotebook->AddPage( typeTree, "Types" );
      m_TreeMonitor.AddTree( typeTree );

      // Entity Classes
      m_EntityAssetOutline = new EntityAssetOutliner( &m_SceneManager );
      UIToolKit::SortTreeCtrl* entityTree = m_EntityAssetOutline->InitTreeCtrl( outlinerNotebook, wxID_ANY );
      entityTree->SetImageList( UIToolKit::GlobalImageManager().GetGuiImageList() );
      outlinerNotebook->AddPage( entityTree, "Entity Classes" );
      m_TreeMonitor.AddTree( entityTree );

      // Runtime Classes
      m_RuntimeClassOutline = new RuntimeClassOutliner( &m_SceneManager );
      UIToolKit::SortTreeCtrl* codeTree = m_RuntimeClassOutline->InitTreeCtrl( outlinerNotebook, wxID_ANY );
      codeTree->SetImageList( UIToolKit::GlobalImageManager().GetGuiImageList() );
      outlinerNotebook->AddPage( codeTree, "Runtime Classes" );
      m_TreeMonitor.AddTree( codeTree );
#endif
    }

#ifndef LUNA_SCENE_DISABLE_OUTLINERS
    // Hierarchy
    m_HierarchyOutline = new HierarchyOutliner( &m_SceneManager );
    UIToolKit::SortTreeCtrl* hierarchyTree = m_HierarchyOutline->InitTreeCtrl( m_Directory, SceneEditorIDs::ID_HierarchyOutlineControl );
    hierarchyTree->SetImageList( UIToolKit::GlobalImageManager().GetGuiImageList() );
    m_HierarchyOutlinePage = m_Directory->GetPageCount();
    m_Directory->AddPage( hierarchyTree, "Hierarchy", false, UIToolKit::GlobalImageManager().GetImageIndex( "world_16.png" ) );
    m_TreeMonitor.AddTree( hierarchyTree );
#endif
  }
  m_FrameManager.AddPane( m_Directory, wxAuiPaneInfo().Name( "directory" ).Caption( "Directory" ).Left().Layer( 1 ).Position( 1 ) );

  // Properties panel
  m_Properties = new wxNotebook (this, wxID_ANY, wxPoint(0,0), wxSize(250,250), wxNB_NOPAGETHEME);
  m_Properties->SetImageList( UIToolKit::GlobalImageManager().GetGuiImageList() );
  {
    // Properties panel - Selection page
    m_SelectionEnumerator = new Enumerator (&m_SelectionProperties);
    m_SelectionPropertiesManager = new PropertiesManager (m_SelectionEnumerator);
    LSelectionPropertiesPanel* selectionProperties = new LSelectionPropertiesPanel (m_SelectionPropertiesManager, m_Properties, SceneEditorIDs::ID_SelectionProperties, wxPoint(0,0), wxSize(250,250), wxNO_BORDER | wxCLIP_CHILDREN);
    m_SelectionProperties.SetControl( selectionProperties->m_PropertyCanvas );
    m_SelectionPropertyPage = m_Properties->GetPageCount();
    m_Properties->AddPage(selectionProperties, "Selection", false, UIToolKit::GlobalImageManager().GetImageIndex( "select_16.png" ));

    // Properties panel - Tool page
    m_ToolEnumerator = new Enumerator (&m_ToolProperties);
    m_ToolPropertiesManager = new PropertiesManager (m_ToolEnumerator);
    m_ToolProperties.SetControl( new Inspect::CanvasWindow (m_Properties, SceneEditorIDs::ID_ToolProperties, wxPoint(0,0), wxSize(250,250), wxNO_BORDER | wxCLIP_CHILDREN) );
    m_ToolPropertyPage = m_Properties->GetPageCount();
    m_Properties->AddPage(m_ToolProperties.GetControl(), "Tool", false, UIToolKit::GlobalImageManager().GetImageIndex( "transform_16.png" ));
  }
  m_FrameManager.AddPane( m_Properties, wxAuiPaneInfo().Name(wxT("properties")).Caption(wxT("Properties")).Right().Layer(1).Position(1) );

  // Objects panel
  m_TypeGrid = new TypeGrid( this, &m_SceneManager );
  m_FrameManager.AddPane( m_TypeGrid->GetPanel(), wxAuiPaneInfo().Name(wxT("types")).Caption(wxT("Types")).Left().Layer(1).Position(1) );

  // Layer panel
  m_LayersNotebook  = new wxNotebook( this, wxID_ANY, wxPoint(0,0), wxSize(250, 250), wxNB_NOPAGETHEME);
  m_LayersNotebook->SetImageList( UIToolKit::GlobalImageManager().GetGuiImageList() );
  {
    // General purpose layers
    {
      LayerGridPtr newGridPtr      = new LayerGrid( m_LayersNotebook, &m_SceneManager, Content::LayerTypes::LT_GeneralPurpose );
      m_LayersNotebook->AddPage( newGridPtr->GetPanel(), wxT("General"), true);
      m_LayerGrids.push_back(newGridPtr);
    }

    // Lighting layers
    {
      LayerGridPtr newGridPtr      = new LightingLayerGrid( m_LayersNotebook, &m_SceneManager);
      m_LayersNotebook->AddPage( newGridPtr->GetPanel(), wxT("Lighting"), false );
      m_LayerGrids.push_back(newGridPtr);
    }
  }
  m_FrameManager.AddPane( m_LayersNotebook, wxAuiPaneInfo().Name(wxT("layers")).Caption(wxT("Layers")).Right().Layer(1).Position(1) );
  
  // Lighting panel
  LightingPanel* lightingPanel = new LightingPanel( this );
  m_FrameManager.AddPane( lightingPanel, wxAuiPaneInfo().Name( "lighting" ).Caption( "Lighting" ).Hide().Float().FloatingPosition( 0, 0 ).FloatingSize( 600, 430 ) );

  // Simulation panel
  LiveLinkPanel* simulationPanel = new LiveLinkPanel( this );
  m_FrameManager.AddPane( simulationPanel, wxAuiPaneInfo().Name( "live link" ).Caption( "Live Link" ).Right().Layer(1).Position(1));

  // Search bar
  SearchBar* searchBar = new SearchBar( this );
  m_FrameManager.AddPane( searchBar, wxAuiPaneInfo().Name( "search bar" ).Caption( "Search" ).Right().Layer(1).Position(2));

  //
  // Center pane
  //

  m_View = new Luna::View(this, -1, wxPoint(0,0), wxSize(150,250), wxNO_BORDER | wxWANTS_CHARS);
  m_FrameManager.AddPane(m_View, wxAuiPaneInfo().Name(wxT("view_content")).CenterPane());

  m_SelectionPropertiesManager->AddPropertiesCreatedListener( PropertiesCreatedSignature::Delegate( this, &SceneEditor::OnPropertiesCreated ) );
  m_ToolPropertiesManager->AddPropertiesCreatedListener( PropertiesCreatedSignature::Delegate( this, &SceneEditor::OnPropertiesCreated ) );
  m_View->AddRenderListener( RenderSignature::Delegate ( this, &SceneEditor::Render ) );
  m_View->AddSelectListener( SelectSignature::Delegate ( this, &SceneEditor::Select ) ); 
  m_View->AddSetHighlightListener( SetHighlightSignature::Delegate ( this, &SceneEditor::SetHighlight ) );
  m_View->AddClearHighlightListener( ClearHighlightSignature::Delegate ( this, &SceneEditor::ClearHighlight ) );
  m_View->AddToolChangedListener( ToolChangeSignature::Delegate ( this, &SceneEditor::ViewToolChanged ) );

  // Create drawers - commented out for now
  //{
  //  AddDrawer( new Drawer( m_FrameManager.GetPane( "directory" ), UIToolKit::GlobalImageManager().GetBitmap( "world_16.png" ) ) );
  //  AddDrawer( new Drawer( m_FrameManager.GetPane( "properties" ), UIToolKit::GlobalImageManager().GetBitmap( "transform_16.png" ) ) );
  //  AddDrawer( new Drawer( m_FrameManager.GetPane( "types" ), UIToolKit::GlobalImageManager().GetBitmap( "type_16.png" ) ) );
  //  AddDrawer( new Drawer( m_FrameManager.GetPane( "layers" ), UIToolKit::GlobalImageManager().GetBitmap( "layer_16.png" ) ) );
  //  AddDrawer( new Drawer( m_FrameManager.GetPane( "lighting" ), UIToolKit::GlobalImageManager().GetBitmap( "light_16.png" ) ) );
  //  AddDrawer( new Drawer( m_FrameManager.GetPane( "live link" ), UIToolKit::GlobalImageManager().GetBitmap( "joystick_16.png" ) ) );
  //  AddDrawer( new Drawer( m_FrameManager.GetPane( "search bar" ), UIToolKit::GlobalImageManager().GetBitmap( "magnify_16.png" ) ) );
  //}

  // Tools (requires m_View) - REMOVING UNTIL UI IS COMPLETED
  //{
  //  wxAuiPaneInfo info;
  //  info.Name( "Tools" );
  //  info.Caption( "Tools" );
  //  info.Float();

  //  ToolsPanel* toolsPanel = new ToolsPanel( this );
  //  m_FrameManager.AddPane( toolsPanel, info );
  //}

  //
  // Toolbars
  //

  m_FrameManager.AddPane(m_StandardToolBar, wxAuiPaneInfo().
    Name(wxT("standard")).Caption(wxT("Standard")).
    ToolbarPane().Top().
    LeftDockable(false).RightDockable(false));

  m_FrameManager.AddPane(m_ViewToolBar, wxAuiPaneInfo().
    Name(wxT("view")).Caption(wxT("View")).
    ToolbarPane().Top().Position(1).
    LeftDockable(false).RightDockable(false));

  m_FrameManager.AddPane(m_BrowserToolBar, wxAuiPaneInfo().
    Name(wxT("browser")).Caption(wxT("Browser")).
    ToolbarPane().Top().Position(2).
    LeftDockable(false).RightDockable(false));
  
  m_FrameManager.AddPane(m_ToolsToolBar, wxAuiPaneInfo().
    Name(wxT("utilities")).Caption(wxT("Utilities")).
    ToolbarPane().Top().Row(2).
    LeftDockable(false).RightDockable(false));

  m_FrameManager.AddPane(m_NavToolBar, wxAuiPaneInfo().
    Name(wxT("Nav Utilities")).Caption(wxT("Nav Utilities")).
    ToolbarPane().Top().Row(4).
    LeftDockable(false).RightDockable(false));

  //
  // Create menus
  //

  wxMenuBar* mb = new wxMenuBar;

  {
    m_FileMenu = new wxMenu();
    m_MRUMenu = new wxMenu();

    m_FileMenu->Append(wxID_NEW, _("New...\tCtrl-n"));
    m_FileMenu->Append(wxID_OPEN, _("Open...\tCtrl-o"));
    m_MRUMenuItem = m_FileMenu->AppendSubMenu( m_MRUMenu, "Open Recent" );
    m_FileMenu->Append(SceneEditorIDs::ID_FileFind, "Find...\tCtrl-f" );
    m_FileMenu->Append(wxID_CLOSE, _("Close"));
    m_FileMenu->AppendSeparator();
    m_FileMenu->Append(wxID_SAVE, _("Save All\tCtrl-s"));
    m_FileMenu->Append(wxID_REVERT, _("Revert"));
    m_FileMenu->AppendSeparator();
    m_FileMenu->Append(SceneEditorIDs::ID_FileCheckOut, _("Check Out"));
    m_FileMenu->AppendSeparator();
    m_FileMenu->Append(SceneEditorIDs::ID_FileImport, _("Import..."));
    m_FileMenu->Append(SceneEditorIDs::ID_FileImportFromClipboard, _("Import from Clipboard..."));
    m_FileMenu->Append(SceneEditorIDs::ID_FileExport, _("Export..."));
    m_FileMenu->Append(SceneEditorIDs::ID_FileExportToClipboard, _("Export to Clipboard..."));
    m_FileMenu->Append(SceneEditorIDs::ID_FileExportToObj, _("Export to OBJ File..."));
    m_FileMenu->AppendSeparator();
    m_FileMenu->Append(SceneEditorIDs::ID_FileOpenSession, "Open Session");
    m_FileMenu->Append(SceneEditorIDs::ID_FileSaveSession, "Save Session");
    m_FileMenu->Append(SceneEditorIDs::ID_FileSaveSessionAs, "Save Session As...");
    m_FileMenu->AppendSeparator();
    m_FileMenu->Append(wxID_EXIT, _("Exit"));

    mb->Append(m_FileMenu, _("File"));
  }

  {
    m_EditMenu = new wxMenu;

    m_EditMenu->Append(wxID_UNDO, _("Undo\tCtrl-z"));
    m_EditMenu->Append(wxID_REDO, _("Redo\tCtrl-Shift-z"));
    m_EditMenu->AppendSeparator();
    m_EditMenu->Append(wxID_CUT, _("Cut\tCtrl-x"));
    m_EditMenu->Append(wxID_COPY, _("Copy\tCtrl-c"));
    m_EditMenu->Append(wxID_PASTE, _("Paste\tCtrl-v"));
    m_EditMenu->AppendSeparator();
    m_EditMenu->Append(wxID_DELETE, _("Delete"));
    m_EditMenu->AppendSeparator();
    m_EditMenu->Append(SceneEditorIDs::ID_EditParent, _("Parent\tCtrl-p"));
    m_EditMenu->Append(SceneEditorIDs::ID_EditUnparent, _("Unparent\tCtrl-Shift-p"));
    m_EditMenu->Append(SceneEditorIDs::ID_EditGroup, _("Group\tCtrl-g"));
    m_EditMenu->Append(SceneEditorIDs::ID_EditUngroup, _("Ungroup\tCtrl-Shift-g"));
    m_EditMenu->Append(SceneEditorIDs::ID_EditCenter, _("Center\tCtrl-Shift-c"));
    m_EditMenu->AppendSeparator();
    m_EditMenu->Append(SceneEditorIDs::ID_EditInvertSelection, _("Invert Selection\tCtrl-i"));
    // Setting the accelerator string this way seems to preserve the string but not actually use the accelerator
    wxMenuItem* menuItemSelectAll = m_EditMenu->Append(SceneEditorIDs::ID_EditSelectAll, _("Select All"));
    menuItemSelectAll->SetAccelString( "Ctrl-a" );

    m_EditMenu->AppendSeparator();

    {
      m_LightLinksMenu = new wxMenu;

      m_LightLinksMenu->Append(SceneEditorIDs::ID_ToolsLightingLayerCreate, _("Link Selection\tCtrl-l"));
      m_LightLinksMenu->Append(SceneEditorIDs::ID_ToolsLightingLayerUnlink, _("Unlink Selection\tCtrl-Shift-l"));
      m_LightLinksMenu->Append(SceneEditorIDs::ID_ToolsLightingLayerSelect, _("Select Linked Lights\tAlt-l"));
      m_EditMenu->AppendSubMenu(m_LightLinksMenu, _("Light Links"));
    }

    m_EditMenu->AppendSeparator();
    m_EditMenu->Append(SceneEditorIDs::ID_EditDuplicate, _("Duplicate\tCtrl-d"));
    m_EditMenu->Append(SceneEditorIDs::ID_EditSmartDuplicate, _("Smart Duplicate\tCtrl-Shift-d"));
    m_EditMenu->AppendSeparator();
    m_EditMenu->Append(SceneEditorIDs::ID_EditCopyTransform, _("Copy Transform\tAlt-t"));
    m_EditMenu->Append(SceneEditorIDs::ID_EditPasteTransform, _("Paste Transform\tAlt-Shift-t"));
    m_EditMenu->AppendSeparator();
    m_EditMenu->Append(SceneEditorIDs::ID_EditSnapToCamera, _("Snap To Camera\tAlt-c"));
    m_EditMenu->Append(SceneEditorIDs::ID_EditSnapCameraTo, _("Snap Camera To\tAlt-Shift-c"));
    m_EditMenu->AppendSeparator();
    m_EditMenu->Append(SceneEditorIDs::ID_EditWalkUp, _("Walk Up (Up Arrow)"));
    m_EditMenu->Append(SceneEditorIDs::ID_EditWalkDown, _("Walk Down (Down Arrow)"));
    m_EditMenu->Append(SceneEditorIDs::ID_EditWalkForward, _("Walk Forward (Right Arrow)"));
    m_EditMenu->Append(SceneEditorIDs::ID_EditWalkBackward, _("Walk Backward (Left Arrow)"));
    m_EditMenu->AppendSeparator();
    m_EditMenu->Append(SceneEditorIDs::ID_EditPreferences, _("Preferences..."));

    mb->Append(m_EditMenu, _("Edit"));
  }

  {
    m_ViewMenu = new wxMenu;

    {
      m_HelperMenu = new wxMenu;

      m_HelperMenu->AppendCheckItem(SceneEditorIDs::ID_ViewAxes, _("Axes"));
      m_HelperMenu->AppendCheckItem(SceneEditorIDs::ID_ViewGrid, _("Grid"));
      m_HelperMenu->AppendCheckItem(SceneEditorIDs::ID_ViewBounds, _("Bounds"));
      m_HelperMenu->AppendCheckItem(SceneEditorIDs::ID_ViewStatistics, _("Statistics"));

      m_ViewMenu->AppendSubMenu(m_HelperMenu, _("Helper"));
    }

    {
      m_GeometryMenu = new wxMenu;

      m_GeometryMenu->AppendCheckItem(SceneEditorIDs::ID_ViewNone, _("None"));
      m_GeometryMenu->AppendCheckItem(SceneEditorIDs::ID_ViewRender, _("Art"));
      m_GeometryMenu->AppendCheckItem(SceneEditorIDs::ID_ViewCollision, _("Collision"));
      m_GeometryMenu->AppendSeparator();
      m_GeometryMenu->AppendCheckItem(SceneEditorIDs::ID_ViewPathfinding, _("Pathfinding"));

      m_ViewMenu->AppendSubMenu(m_GeometryMenu, _("Geometry"));
    }

    {
      m_ShadingMenu = new wxMenu;

      m_ShadingMenu->AppendCheckItem(SceneEditorIDs::ID_ViewWireframeOnMesh, _("Wireframe on Mesh"));
      m_ShadingMenu->AppendCheckItem(SceneEditorIDs::ID_ViewWireframeOnShaded, _("Wireframe on Shaded"));
      m_ShadingMenu->AppendSeparator();
      m_ShadingMenu->AppendCheckItem(SceneEditorIDs::ID_ViewWireframe, _("Wireframe (4)"));
      m_ShadingMenu->AppendCheckItem(SceneEditorIDs::ID_ViewMaterial, _("Material (5)"));
      m_ShadingMenu->AppendCheckItem(SceneEditorIDs::ID_ViewTexture, _("Texture (6)"));

      m_ViewMenu->AppendSubMenu(m_ShadingMenu, _("Shading"));
    }

    {
      m_CameraMenu = new wxMenu;

      m_CameraMenu->AppendCheckItem(SceneEditorIDs::ID_ViewOrbit, _("Orbit (7)"));
      m_CameraMenu->AppendCheckItem(SceneEditorIDs::ID_ViewFront, _("Front (8)"));
      m_CameraMenu->AppendCheckItem(SceneEditorIDs::ID_ViewSide, _("Side (9)"));
      m_CameraMenu->AppendCheckItem(SceneEditorIDs::ID_ViewTop, _("Top (0)"));

      m_ViewMenu->AppendSubMenu(m_CameraMenu, _("Camera"));
    }

    {
      m_CullingMenu = new wxMenu;

      m_CullingMenu->AppendCheckItem(SceneEditorIDs::ID_ViewFrustumCulling, _("Frustum Culling"));
      m_CullingMenu->AppendCheckItem(SceneEditorIDs::ID_ViewBackfaceCulling, _("Backface Culling"));

      m_ViewMenu->AppendSubMenu(m_CullingMenu, _("Culling"));
    }

    {
      wxMenu* show_menu = new wxMenu;

      show_menu->Append(SceneEditorIDs::ID_ViewShowAll, _("Show All\tAlt-j"));
      show_menu->Append(SceneEditorIDs::ID_ViewShowAllGeometry, _("Show All Geometry\tCtrl-Alt-b"));
      show_menu->AppendSeparator();
      show_menu->Append(SceneEditorIDs::ID_ViewShowSelected, _("Show Selected\tAlt-s"));
      show_menu->Append(SceneEditorIDs::ID_ViewShowSelectedGeometry, _("Show Selected Geometry\tCtrl-b"));
      show_menu->AppendSeparator();
      show_menu->Append(SceneEditorIDs::ID_ViewShowUnrelated, _("Show Unrelated\tAlt-Shift-s"));
      show_menu->AppendSeparator();
      show_menu->Append(SceneEditorIDs::ID_ViewShowLastHidden, _("Show Last Hidden\tCtrl-Shift-h"));

      m_ViewMenu->AppendSubMenu(show_menu, _("Show"));
    }

    {
      wxMenu* hide_menu = new wxMenu;

      hide_menu->Append(SceneEditorIDs::ID_ViewHideAll, _("Hide All\tAlt-Shift-j"));
      hide_menu->Append(SceneEditorIDs::ID_ViewHideAllGeometry, _("Hide All Geometry\tCtrl-Alt-Shift-b"));
      hide_menu->AppendSeparator();
      hide_menu->Append(SceneEditorIDs::ID_ViewHideSelected, _("Hide Selected\tCtrl-h"));
      hide_menu->Append(SceneEditorIDs::ID_ViewHideSelectedGeometry, _("Hide Selected Geometry\tCtrl-Shift-b"));
      hide_menu->AppendSeparator();
      hide_menu->Append(SceneEditorIDs::ID_ViewHideUnrelated, _("Hide Unrelated\tAlt-h"));

      m_ViewMenu->AppendSubMenu(hide_menu, _("Hide"));
    }

    {
      m_ViewColorMenu = new wxMenu;

      const Reflect::Enumeration* colorModes = Reflect::GetEnumeration< ViewColorModes::ViewColorMode >();
      const size_t numColorModes = colorModes->m_Elements.size();
      for ( size_t colorIndex = 0; colorIndex < numColorModes; ++colorIndex )
      {
        const Reflect::EnumerationElement* colorElement = colorModes->m_Elements[colorIndex];
        wxMenuItem* menuItem = m_ViewColorMenu->AppendCheckItem( wxID_ANY, colorElement->m_Label.c_str() );
        m_ColorModeLookup.insert( M_IDToColorMode::value_type( menuItem->GetId(), colorElement->m_Value ) );
        Connect( menuItem->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SceneEditor::OnViewColorModeChange ), NULL, this );
      }

      m_ViewMenu->AppendSubMenu( m_ViewColorMenu, "Color Mode" );
    }

    {
      m_ViewDefaultsMenu = new wxMenu; 
      m_ViewDefaultsMenu->AppendCheckItem(SceneEditorIDs::ID_ViewDefaultShowLayers, _("Show Layers")); 
      m_ViewDefaultsMenu->AppendCheckItem(SceneEditorIDs::ID_ViewDefaultShowInstances, _("Show Instances")); 
      m_ViewDefaultsMenu->AppendCheckItem(SceneEditorIDs::ID_ViewDefaultShowGeometry, _("Show Geometry")); 
      m_ViewDefaultsMenu->AppendCheckItem(SceneEditorIDs::ID_ViewDefaultShowPointer, _("Show Pointer")); 
      m_ViewDefaultsMenu->AppendCheckItem(SceneEditorIDs::ID_ViewDefaultShowBounds, _("Show Bounds")); 

      m_ViewMenu->AppendSubMenu(m_ViewDefaultsMenu, _("Defaults")); 
    }

    m_ViewMenu->Append(SceneEditorIDs::ID_ViewFrameOrigin, _("Frame Origin (O)"));
    m_ViewMenu->Append(SceneEditorIDs::ID_ViewFrameSelected, _("Frame Selected (F)"));
    m_ViewMenu->AppendCheckItem(SceneEditorIDs::ID_ViewHighlightMode, _("Highlight Mode (H)"));
    m_ViewMenu->Append(SceneEditorIDs::ID_ViewPreviousView, _("Previous View   ["));
    m_ViewMenu->Append(SceneEditorIDs::ID_ViewNextView, _("Next View    ]"));

    mb->Append(m_ViewMenu, _("View"));
  }

  {
    wxMenu* tools_menu = new wxMenu;

    tools_menu->Append(SceneEditorIDs::ID_ToolsSelect, _("Select (Q)\tESCAPE"));

    tools_menu->AppendSeparator();
    tools_menu->Append(SceneEditorIDs::ID_ToolsTranslate, _("Translate (W)"));
    tools_menu->Append(SceneEditorIDs::ID_ToolsRotate, _("Rotate (E)"));
    tools_menu->Append(SceneEditorIDs::ID_ToolsScale, _("Scale (R)"));

    tools_menu->AppendSeparator();
    tools_menu->Append(SceneEditorIDs::ID_ToolsPivot, _("Move Pivot\tINSERT"));

    tools_menu->AppendSeparator();
    tools_menu->Append(SceneEditorIDs::ID_ToolsEntityCreate, _("Entity Placement\tCtrl-e"));
    tools_menu->Append(SceneEditorIDs::ID_ToolsVolumeCreate, _("Volume Placement"));
    tools_menu->Append(SceneEditorIDs::ID_ToolsClueCreate, _("Clue Placement"));
#if LUNA_GAME_CAMERA
    tools_menu->Append(SceneEditorIDs::ID_ToolsGameCameraCreate, _("Game Camera Placement"));
#endif
    tools_menu->Append(SceneEditorIDs::ID_ToolsControllerCreate, _("Controller Placement"));
    tools_menu->Append(SceneEditorIDs::ID_ToolsLocatorCreate, _("Locator Placement"));
    tools_menu->Append(SceneEditorIDs::ID_ToolsDuplicate, _("Duplicate Tool\tAlt-d"));

    tools_menu->AppendSeparator();
    tools_menu->Append(SceneEditorIDs::ID_ToolsCurveCreate, _("Create Curve"));
    tools_menu->Append(SceneEditorIDs::ID_ToolsCurveEdit, _("Edit Curve"));

    tools_menu->AppendSeparator();
    tools_menu->Append(SceneEditorIDs::ID_ToolsLightCreate, _("Light Placement"));
    tools_menu->Append(SceneEditorIDs::ID_ToolsLighting, _("Lighting"));

    tools_menu->AppendSeparator();
    tools_menu->Append(SceneEditorIDs::ID_ToolsPostProcessingVolumeCreate, _("PostProcessing Volume Placement")); 
    tools_menu->Append(SceneEditorIDs::ID_ToolsPostProcessingVolumeScript, _("PostProcessing Volume Script")); 

    mb->Append(tools_menu, _("Tools"));
  }

  {
    m_UtilitiesMenu = new wxMenu;
    m_UtilitiesMenu->Append(SceneEditorIDs::ID_UtilitiesConstruction, _("Maya Construction"));

    m_UtilitiesMenu->AppendSeparator();
    m_UtilitiesMenu->Append(SceneEditorIDs::ID_UtilitiesMeasureDistance, _("Measure Distance"));
    m_UtilitiesMenu->Append(SceneEditorIDs::ID_UtilitiesFlushSymbols, _("Flush Symbols"));

    mb->Append(m_UtilitiesMenu, _("Utilities"));
  }

  {
    m_PanelsMenu = new wxMenu;
    CreatePanelsMenu( m_PanelsMenu );
    mb->Append( m_PanelsMenu, "Panels" );
  }

  {
    wxMenu* help_menu = new wxMenu;

    help_menu->Append( wxID_HELP_INDEX, _("Index") );
    help_menu->Append( wxID_HELP_SEARCH, _("Search") );
    help_menu->AppendSeparator();
    help_menu->Append(SceneEditorIDs::ID_About, _("About...\tF1"));

    mb->Append(help_menu, _("Help"));
  }

  // Disable certain toolbar buttons (they'll enable when appropriate)
  m_StandardToolBar->EnableTool( wxID_SAVE, false );
  m_FileMenu->Enable( wxID_SAVE, false );

  SetMenuBar(mb);

  // Restore layout if any
  SceneEditorPreferences()->GetSceneEditorWindowSettings()->ApplyToWindow( this, &m_FrameManager, true );
  SceneEditorPreferences()->GetViewPreferences()->ApplyToView( m_View ); 

  Inspect::DropTarget* dropTarget = new Inspect::DropTarget();
  dropTarget->SetDragOverCallback( Inspect::DragOverCallback::Delegate( this, &SceneEditor::DragOver ) );
  dropTarget->SetDropCallback( Inspect::DropCallback::Delegate( this, &SceneEditor::Drop ) );
  m_View->SetDropTarget( dropTarget );
}

SceneEditor::~SceneEditor()
{
  // Remove any straggling document listeners
  OS_DocumentSmartPtr::Iterator docItr = m_SceneManager.GetDocuments().Begin();
  OS_DocumentSmartPtr::Iterator docEnd = m_SceneManager.GetDocuments().End();
  for ( ; docItr != docEnd; ++docItr )
  {
    ( *docItr )->RemoveDocumentModifiedListener( DocumentChangedSignature::Delegate( this, &SceneEditor::DocumentModified ) );
    ( *docItr )->RemoveDocumentSavedListener( DocumentChangedSignature::Delegate( this, &SceneEditor::DocumentModified ) );
    ( *docItr )->RemoveDocumentClosedListener( DocumentChangedSignature::Delegate( this, &SceneEditor::DocumentModified ) );
  }

  // Save preferences and MRU
  V_string mruPaths;
  m_MRU->ToVector( mruPaths );
  SceneEditorPreferences()->GetMRU()->SetPaths( mruPaths );
  SceneEditorPreferences()->GetViewPreferences()->LoadFromView( m_View ); 
  SceneEditorPreferences()->SavePreferences();

  //
  // Close connections
  //

  RemoteConstruct::Cleanup();

  if ( m_HasViewerControl )
  {
    SessionManager::GetInstance()->GiveViewerControl( NULL );
  }

  delete m_RemoteScene;


  //
  // Detach event handlers
  //

  m_SceneManager.RemoveCurrentSceneChangingListener( SceneChangeSignature::Delegate (this, &SceneEditor::CurrentSceneChanging) );
  m_SceneManager.RemoveCurrentSceneChangedListener( SceneChangeSignature::Delegate (this, &SceneEditor::CurrentSceneChanged) );
  m_SceneManager.RemoveSceneAddedListener( SceneChangeSignature::Delegate( this, &SceneEditor::SceneAdded ) );
  m_SceneManager.RemoveSceneRemovingListener( SceneChangeSignature::Delegate( this, &SceneEditor::SceneRemoving ) );

  m_MRU->RemoveItemSelectedListener( UIToolKit::MRUSignature::Delegate( this, &SceneEditor::OnMRUOpen ) );

  m_SelectionPropertiesManager->RemovePropertiesCreatedListener( PropertiesCreatedSignature::Delegate( this, &SceneEditor::OnPropertiesCreated ) );
  m_ToolPropertiesManager->RemovePropertiesCreatedListener( PropertiesCreatedSignature::Delegate( this, &SceneEditor::OnPropertiesCreated ) );
  m_View->RemoveRenderListener( RenderSignature::Delegate ( this, &SceneEditor::Render ) );
  m_View->RemoveSelectListener( SelectSignature::Delegate ( this, &SceneEditor::Select ) ); 
  m_View->RemoveSetHighlightListener( SetHighlightSignature::Delegate ( this, &SceneEditor::SetHighlight ) );
  m_View->RemoveClearHighlightListener( ClearHighlightSignature::Delegate ( this, &SceneEditor::ClearHighlight ) );
  m_View->RemoveToolChangedListener( ToolChangeSignature::Delegate ( this, &SceneEditor::ViewToolChanged ) );

  delete m_TypeGrid;
  delete m_TypeOutline;
  delete m_HierarchyOutline;
  delete m_EntityAssetOutline;
  delete m_RuntimeClassOutline;

  //Clean up all of our layer grids
  m_LayerGrids.clear();
}

SceneEditorID SceneEditor::CameraModeToSceneEditorID( CameraMode cameraMode )
{
  RM_CamModeToSceneID::M_AToB::const_iterator found = s_CameraModeToSceneID.AToB().find( cameraMode );
  NOC_ASSERT( found != s_CameraModeToSceneID.AToB().end() );
  return *found->second;
}

CameraMode SceneEditor::SceneEditorIDToCameraMode( SceneEditorID id )
{
  RM_CamModeToSceneID::M_BToA::const_iterator found = s_CameraModeToSceneID.BToA().find( id );
  NOC_ASSERT( found != s_CameraModeToSceneID.BToA().end() );
  return *found->second;
}

///////////////////////////////////////////////////////////////////////////////
// Build all assets that are currently loaded.
// 
void SceneEditor::BuildAllLoadedAssets()
{
  S_tuid assets;
  // hand over the current level's referenced stuff.
  Asset::LevelAsset* currentLevel = m_SceneManager.GetCurrentLevel();
  if ( currentLevel )
  {
    S_tuid::iterator skyItr = currentLevel->m_SkyAssets.begin();
    S_tuid::iterator skyEnd = currentLevel->m_SkyAssets.end();
    for ( ; skyItr != skyEnd; ++skyItr )
    {
      assets.insert( *skyItr );
    }
    if(currentLevel->m_DefaultCubeMap != TUID::Null)
    {
      assets.insert( currentLevel->m_DefaultCubeMap); 
    }
    if(currentLevel->m_WaterCubeMap != TUID::Null)
    {
      assets.insert( currentLevel->m_WaterCubeMap); 
    }
  }

  // iterate over every entity instance, adding them to the viewers' scene
  const M_SceneSmartPtr& scenes = m_SceneManager.GetScenes();

  for each ( const M_SceneSmartPtr::value_type& val in scenes )
  {
    ScenePtr scene = val.second;

    V_EntityDumbPtr entities;

    scene->GetAll< Luna::Entity>( entities );

    for each ( const EntityPtr entity in entities )
    {
      if ( !entity->IsTransient() )
      {
        tuid assetId = entity->GetClassSet()->GetEntityAssetID();
        if ( assetId != TUID::Null )
        {
          assets.insert( assetId );
        }
      }
    }
  }

  SessionManager::GetInstance()->SaveAllOpenDocuments();
  BuildAssets( assets, this );
}

void SceneEditor::OnEraseBackground(wxEraseEvent& event)
{
  event.Skip();
}

void SceneEditor::OnSize(wxSizeEvent& event)
{
  event.Skip();
}

void SceneEditor::OnChar(wxKeyEvent& event)
{
  switch (event.KeyCode())
  {
  case WXK_SPACE:
    m_View->NextCameraMode();
    event.Skip(false);
    break;

  case WXK_UP:
    GetEventHandler()->ProcessEvent( wxCommandEvent (wxEVT_COMMAND_MENU_SELECTED, SceneEditorIDs::ID_EditWalkUp) );
    event.Skip(false);
    break;

  case WXK_DOWN:
    GetEventHandler()->ProcessEvent( wxCommandEvent (wxEVT_COMMAND_MENU_SELECTED, SceneEditorIDs::ID_EditWalkDown) );
    event.Skip(false);
    break;

  case WXK_RIGHT:
    GetEventHandler()->ProcessEvent( wxCommandEvent (wxEVT_COMMAND_MENU_SELECTED, SceneEditorIDs::ID_EditWalkForward) );
    event.Skip(false);
    break;

  case WXK_LEFT:
    GetEventHandler()->ProcessEvent( wxCommandEvent (wxEVT_COMMAND_MENU_SELECTED, SceneEditorIDs::ID_EditWalkBackward) );
    event.Skip(false);
    break;

  case WXK_INSERT:
    GetEventHandler()->ProcessEvent( wxCommandEvent (wxEVT_COMMAND_MENU_SELECTED, SceneEditorIDs::ID_ToolsPivot) );
    event.Skip(false);
    break;

  case WXK_DELETE:
    GetEventHandler()->ProcessEvent( wxCommandEvent (wxEVT_COMMAND_MENU_SELECTED, wxID_DELETE) );
    event.Skip(false);
    break;

  case WXK_ESCAPE:
    GetEventHandler()->ProcessEvent( wxCommandEvent (wxEVT_COMMAND_MENU_SELECTED, SceneEditorIDs::ID_ToolsSelect) );
    event.Skip(false);
    break;

  default:
    event.Skip();
    break;
  }

  if (event.GetSkipped())
  {
    switch (tolower(event.KeyCode()))
    {
    case '4':
      GetEventHandler()->ProcessEvent( wxCommandEvent (wxEVT_COMMAND_MENU_SELECTED, SceneEditorIDs::ID_ViewWireframe) );
      event.Skip(false);
      break;

    case '5':
      GetEventHandler()->ProcessEvent( wxCommandEvent (wxEVT_COMMAND_MENU_SELECTED, SceneEditorIDs::ID_ViewMaterial) );
      event.Skip(false);
      break;

    case '6':
      GetEventHandler()->ProcessEvent( wxCommandEvent (wxEVT_COMMAND_MENU_SELECTED, SceneEditorIDs::ID_ViewTexture) );
      event.Skip(false);
      break;

    case '7':
      GetEventHandler()->ProcessEvent( wxCommandEvent (wxEVT_COMMAND_MENU_SELECTED, SceneEditorIDs::ID_ViewOrbit) );
      event.Skip(false);
      break;

    case '8':
      GetEventHandler()->ProcessEvent( wxCommandEvent (wxEVT_COMMAND_MENU_SELECTED, SceneEditorIDs::ID_ViewFront) );
      event.Skip(false);
      break;

    case '9':
      GetEventHandler()->ProcessEvent( wxCommandEvent (wxEVT_COMMAND_MENU_SELECTED, SceneEditorIDs::ID_ViewSide) );
      event.Skip(false);
      break;

    case '0':
      GetEventHandler()->ProcessEvent( wxCommandEvent (wxEVT_COMMAND_MENU_SELECTED, SceneEditorIDs::ID_ViewTop) );
      event.Skip(false);
      break;

    case 'q':
      GetEventHandler()->ProcessEvent( wxCommandEvent (wxEVT_COMMAND_MENU_SELECTED, SceneEditorIDs::ID_ToolsSelect) );
      event.Skip(false);
      break;

    case 'w':
      GetEventHandler()->ProcessEvent( wxCommandEvent (wxEVT_COMMAND_MENU_SELECTED, SceneEditorIDs::ID_ToolsTranslate) );
      event.Skip(false);
      break;

    case 'e':
      GetEventHandler()->ProcessEvent( wxCommandEvent (wxEVT_COMMAND_MENU_SELECTED, SceneEditorIDs::ID_ToolsRotate) );
      event.Skip(false);
      break;

    case 'r':
      GetEventHandler()->ProcessEvent( wxCommandEvent (wxEVT_COMMAND_MENU_SELECTED, SceneEditorIDs::ID_ToolsScale) );
      event.Skip(false);
      break;

    case 'o':
      GetEventHandler()->ProcessEvent( wxCommandEvent (wxEVT_COMMAND_MENU_SELECTED, SceneEditorIDs::ID_ViewFrameOrigin) );
      event.Skip(false);
      break;

    case 'f':
      GetEventHandler()->ProcessEvent( wxCommandEvent (wxEVT_COMMAND_MENU_SELECTED, SceneEditorIDs::ID_ViewFrameSelected) );
      event.Skip(false);
      break;

    case 'h':
      GetEventHandler()->ProcessEvent( wxCommandEvent (wxEVT_COMMAND_MENU_SELECTED, SceneEditorIDs::ID_ViewHighlightMode) );
      event.Skip(false);
      break;

    case ']':
      GetEventHandler()->ProcessEvent( wxCommandEvent ( wxEVT_COMMAND_MENU_SELECTED, SceneEditorIDs::ID_ViewNextView) );
      event.Skip(false);
      break;

    case '[':
      GetEventHandler()->ProcessEvent( wxCommandEvent ( wxEVT_COMMAND_MENU_SELECTED, SceneEditorIDs::ID_ViewPreviousView) );
      event.Skip(false);
      break;

    case 'l':
      OnLightLinkEvent(event);
      break;

    default:
      event.Skip();
      break;
    }
  }
}

void SceneEditor::OnLightLinkEvent(wxKeyEvent& event)
{
  if(event.m_controlDown == true)
  {
    if(event.m_altDown == false)
    {
      if(event.m_shiftDown == true)
      {
        GetEventHandler()->ProcessEvent( wxCommandEvent ( wxEVT_COMMAND_MENU_SELECTED, SceneEditorIDs::ID_ToolsLightingLayerUnlink) );
      }
      else
      {
        GetEventHandler()->ProcessEvent( wxCommandEvent ( wxEVT_COMMAND_MENU_SELECTED, SceneEditorIDs::ID_ToolsLightingLayerCreate) );
      }
    }
  }
  else
  {
    if((event.m_shiftDown == false) && (event.m_altDown == true))
    {
      GetEventHandler()->ProcessEvent( wxCommandEvent ( wxEVT_COMMAND_MENU_SELECTED, SceneEditorIDs::ID_ToolsLightingLayerSelect) );
    }
  }
}

void SceneEditor::OnShow(wxShowEvent& event)
{
#ifdef LUNA_DEBUG_RUNTIME_DATA_SELECTION
  // Sometimes it's handy to put debug code here for program start up.
  New();
  wxCommandEvent evt( wxEVT_COMMAND_TOOL_CLICKED, SceneEditorIDs::ID_ToolsVolumeCreate );
  GetEventHandler()->ProcessEvent( evt );
  m_SceneManager.GetCurrentScene()->SetTool(NULL);
  wxCloseEvent close( wxEVT_CLOSE_WINDOW );
  GetEventHandler()->AddPendingEvent( close );
#endif

#ifdef LUNA_DEBUG_RENDER
  class RenderThread : public wxThread
  {
  private:
    Luna::View* m_View;

  public:
    RenderThread(Luna::View* view)
      : m_View (view)
    {

    }

    wxThread::ExitCode Entry()
    {
      while (true)
      {
        m_View->Refresh();
      }

      return NULL;
    }
  };

  RenderThread* thread = new RenderThread (m_View);
  thread->Create();
  thread->Run();
#endif

  event.Skip();
}

///////////////////////////////////////////////////////////////////////////////
// Called just before a menu item (on the main menu bar) is opened.  Enables
// and disables individual menu items based upon the current state of the 
// application.
// 
void SceneEditor::OnMenuOpen(wxMenuEvent& event)
{
  const wxMenu* menu = event.GetMenu();

  if ( menu == m_FileMenu )
  {
    // File->Import is enabled if there is a current editing scene
    m_FileMenu->Enable( SceneEditorIDs::ID_FileImport, m_SceneManager.HasCurrentScene() );
    m_FileMenu->Enable( SceneEditorIDs::ID_FileImportFromClipboard, m_SceneManager.HasCurrentScene() );

    // File->Export is only enabled if there is something selected
    const bool enableExport = m_SceneManager.HasCurrentScene() && m_SceneManager.GetCurrentScene()->GetSelection().GetItems().Size() > 0;
    m_FileMenu->Enable( SceneEditorIDs::ID_FileExport, enableExport );
    m_FileMenu->Enable( SceneEditorIDs::ID_FileExportToClipboard, enableExport );

    m_MRUMenuItem->Enable( !m_MRU->GetItems().Empty() );
    m_MRU->PopulateMenu( m_MRUMenu );
  }
  else if ( menu == m_PanelsMenu )
  {
    UpdatePanelsMenu( m_PanelsMenu );
  }
  else if ( menu == m_EditMenu )
  {
    // Edit->Undo/Redo is only enabled if there are commands in the queue
    const bool canUndo = m_SceneManager.HasCurrentScene() && m_SceneManager.CanUndo();
    const bool canRedo = m_SceneManager.HasCurrentScene() && m_SceneManager.CanRedo();
    m_EditMenu->Enable( wxID_UNDO, canUndo );
    m_EditMenu->Enable( wxID_REDO, canRedo );

    // Edit->Invert Selection is only enabled if something is selected
    const bool isAnythingSelected = m_SceneManager.HasCurrentScene() && m_SceneManager.GetCurrentScene()->GetSelection().GetItems().Size() > 0;
    m_EditMenu->Enable( SceneEditorIDs::ID_EditInvertSelection, isAnythingSelected );

    // Cut/copy/paste
    m_EditMenu->Enable( wxID_CUT, isAnythingSelected );
    m_EditMenu->Enable( wxID_COPY, isAnythingSelected );
    m_EditMenu->Enable( wxID_PASTE, m_SceneManager.HasCurrentScene() && IsClipboardFormatAvailable( CF_TEXT ) );

    // Light Links
    if(m_SceneManager.HasCurrentScene() && (m_LayerGrids.empty() == false))
    {
      LayerGridPtr&   lightingLayerGridPtr  = m_LayerGrids[Content::LayerTypes::LT_Lighting];

      //Linking
      {
        bool            enableOp              = lightingLayerGridPtr->IsSelectionValid();
        m_LightLinksMenu->Enable( SceneEditorIDs::ID_ToolsLightingLayerCreate, enableOp);
      }
    
      //Unlinking and selections
      {
        bool            enableOp              = lightingLayerGridPtr->IsSelectionItemsLinked();
        m_LightLinksMenu->Enable( SceneEditorIDs::ID_ToolsLightingLayerUnlink, enableOp);
        m_LightLinksMenu->Enable( SceneEditorIDs::ID_ToolsLightingLayerSelect, enableOp);
      }
    }
    else
    {
      m_LightLinksMenu->Enable( SceneEditorIDs::ID_ToolsLightingLayerCreate, false);
      m_LightLinksMenu->Enable( SceneEditorIDs::ID_ToolsLightingLayerUnlink, false);
      m_LightLinksMenu->Enable( SceneEditorIDs::ID_ToolsLightingLayerSelect, false);
    }
  }
  else if (menu == m_ViewMenu)
  {
    m_HelperMenu->Check( SceneEditorIDs::ID_ViewAxes, m_View->IsAxesVisible() );
    m_HelperMenu->Check( SceneEditorIDs::ID_ViewGrid, m_View->IsGridVisible() );
    m_HelperMenu->Check( SceneEditorIDs::ID_ViewBounds, m_View->IsBoundsVisible() );
    m_HelperMenu->Check( SceneEditorIDs::ID_ViewStatistics, m_View->IsStatisticsVisible() );

    m_CameraMenu->Check( SceneEditorIDs::ID_ViewOrbit, m_View->GetCameraMode() == CameraModes::Orbit );
    m_CameraMenu->Check( SceneEditorIDs::ID_ViewFront, m_View->GetCameraMode() == CameraModes::Front );
    m_CameraMenu->Check( SceneEditorIDs::ID_ViewSide, m_View->GetCameraMode() == CameraModes::Side );
    m_CameraMenu->Check( SceneEditorIDs::ID_ViewTop, m_View->GetCameraMode() == CameraModes::Top );

    m_GeometryMenu->Check( SceneEditorIDs::ID_ViewNone, m_View->GetGeometryMode() == GeometryModes::None );
    m_GeometryMenu->Check( SceneEditorIDs::ID_ViewRender, m_View->GetGeometryMode() == GeometryModes::Render );
    m_GeometryMenu->Check( SceneEditorIDs::ID_ViewCollision, m_View->GetGeometryMode() == GeometryModes::Collision );
    m_GeometryMenu->Check( SceneEditorIDs::ID_ViewPathfinding, m_View->IsPathfindingVisible() );

    ViewColorMode colorMode = SceneEditorPreferences()->GetViewPreferences()->GetColorMode();
    M_IDToColorMode::const_iterator colorModeItr = m_ColorModeLookup.begin();
    M_IDToColorMode::const_iterator colorModeEnd = m_ColorModeLookup.end();
    for ( ; colorModeItr != colorModeEnd; ++colorModeItr )
    {
      m_ViewColorMenu->Check( colorModeItr->first, colorModeItr->second == colorMode );
    }

    m_ShadingMenu->Check( SceneEditorIDs::ID_ViewWireframeOnMesh, m_View->GetCamera()->GetWireframeOnMesh() );
    m_ShadingMenu->Check( SceneEditorIDs::ID_ViewWireframeOnShaded, m_View->GetCamera()->GetWireframeOnShaded() );
    m_ShadingMenu->Check( SceneEditorIDs::ID_ViewWireframe, m_View->GetCamera()->GetShadingMode() == ShadingModes::Wireframe );
    m_ShadingMenu->Check( SceneEditorIDs::ID_ViewMaterial, m_View->GetCamera()->GetShadingMode() == ShadingModes::Material );
    m_ShadingMenu->Check( SceneEditorIDs::ID_ViewTexture, m_View->GetCamera()->GetShadingMode() == ShadingModes::Texture );

    m_CullingMenu->Check( SceneEditorIDs::ID_ViewFrustumCulling, m_View->GetCamera()->IsViewFrustumCulling() );
    m_CullingMenu->Check( SceneEditorIDs::ID_ViewBackfaceCulling, m_View->GetCamera()->IsBackFaceCulling() );

    m_ViewMenu->Check( SceneEditorIDs::ID_ViewHighlightMode, m_View->IsHighlighting() );

    Content::NodeVisibilityPtr nodeDefaults = SceneEditorPreferences()->GetDefaultNodeVisibility(); 

    m_ViewDefaultsMenu->Check( SceneEditorIDs::ID_ViewDefaultShowLayers, nodeDefaults->GetVisibleLayer()); 
    m_ViewDefaultsMenu->Check( SceneEditorIDs::ID_ViewDefaultShowInstances, !nodeDefaults->GetHiddenNode());
    m_ViewDefaultsMenu->Check( SceneEditorIDs::ID_ViewDefaultShowGeometry, nodeDefaults->GetShowGeometry()); 
    m_ViewDefaultsMenu->Check( SceneEditorIDs::ID_ViewDefaultShowPointer, nodeDefaults->GetShowPointer());  
    m_ViewDefaultsMenu->Check( SceneEditorIDs::ID_ViewDefaultShowBounds, nodeDefaults->GetShowBounds());  

  }
  else
  {
    event.Skip();
  }
}

///////////////////////////////////////////////////////////////////////////////
// Called when the "new" button is pressed.  Creates a new scene.
// 
void SceneEditor::OnNew(wxCommandEvent& event)
{
  if ( m_SceneManager.CloseAll() )
  {
    ScenePtr scene = m_SceneManager.NewScene( true );
    scene->GetSceneDocument()->SetModified( true );
    m_SceneManager.SetCurrentScene( scene );
  }
}

///////////////////////////////////////////////////////////////////////////////
// Helper function for common opening code.
// 
bool SceneEditor::DoOpen( const std::string& path )
{
  bool opened = false;
  if ( !path.empty() && FileSystem::Exists( path ) )
  {
    if ( m_SceneManager.CloseAll() )
    {
      std::string error;

      try
      {
        opened = m_SceneManager.OpenPath( path, error ) != NULL;
      }
      catch ( const Nocturnal::Exception& ex )
      {
        error = ex.what();
      }

      if ( opened )
      {
        m_MRU->Insert( path );
      }
      else
      {
        m_MRU->Remove( path );
        if ( !error.empty() )
        {
          wxMessageBox( error.c_str(), "Error", wxCENTER | wxICON_ERROR | wxOK, this );
        }
      }
    }
  }
  return opened;
}

///////////////////////////////////////////////////////////////////////////////
// Called when the "open" button is pressed.
// 
void SceneEditor::OnOpen(wxCommandEvent& event)
{
  // For the default directory of the file open dialog
  static std::string defaultDir = FinderSpecs::Asset::LEVEL_FOLDER.GetFolder();

  UIToolKit::FileDialog openDlg( this, "Open", defaultDir.c_str(), "" );
  openDlg.AddFilter( s_Filter.GetDialogFilter() );
  openDlg.SetFilterIndex( FinderSpecs::Asset::LEVEL_DECORATION.GetDialogFilter() );
  
  if ( openDlg.ShowModal() == wxID_OK )
  {
    DoOpen( openDlg.GetPath().c_str() );
  }
}


///////////////////////////////////////////////////////////////////////////////
// Callback when the user causes a UI event to find a file. 
// 
void SceneEditor::OnFind( wxCommandEvent& event )
{
  File::FileBrowser browserDlg( this, -1, "Open" );

  browserDlg.SetFilter( s_Filter );
  browserDlg.SetFilterIndex( FinderSpecs::Asset::LEVEL_DECORATION );

  if ( browserDlg.ShowModal() == wxID_OK )
    {
    DoOpen( browserDlg.GetPath() );
  }
}

///////////////////////////////////////////////////////////////////////////////
// Called when the "save all" option is chosen in the UI.  Iterates over all
// the open scenes and asks the session manager to save them.
// 
void SceneEditor::OnSaveAll(wxCommandEvent& event)
{
  std::string error;
  if ( !m_SceneManager.SaveAll( error ) )
  {
    wxMessageBox( error.c_str(), "Error", wxCENTER | wxICON_ERROR | wxOK, this );
  }
}

///////////////////////////////////////////////////////////////////////////////
// Called when the user chooses to import a file into the current editing
// scene.  Prompts the user with import options, and carries out the operation
// if the user does not cancel.
// 
void SceneEditor::OnImport(wxCommandEvent& event)
{
  static bool update = false;

  if ( m_SceneManager.HasCurrentScene() )
  {
    Luna::Scene* currentScene = m_SceneManager.GetCurrentScene();

    ImportOptionsDlg dlg( this, update );

    if ( dlg.ShowModal() == wxID_OK && currentScene->IsEditable() )
    {
      u32 flags = ImportFlags::Select;
      if ( update )
      {
        flags |= ImportFlags::Merge;
      }

      switch ( event.GetId() )
      {
      case SceneEditorIDs::ID_FileImport:
        {
          UIToolKit::FileDialog fileDialog( this, "Import" );
          fileDialog.AddFilter( FinderSpecs::Extension::REFLECT_BINARY.GetDialogFilter() );
          fileDialog.AddFilter( FinderSpecs::Extension::REFLECT_TEXT.GetDialogFilter() );
          fileDialog.SetFilterIndex( FinderSpecs::Extension::REFLECT_BINARY.GetDialogFilter() );
          if ( fileDialog.ShowModal() != wxID_OK )
          {
            return;
          }
          
          currentScene->Push( currentScene->ImportFile( fileDialog.GetPath().c_str(), ImportActions::Import, flags, currentScene->GetRoot() ) );
          break;
        }

      case SceneEditorIDs::ID_FileImportFromClipboard:
        {
          std::string xml, error;
          if ( !Windows::RetrieveFromClipboard( GetHwnd(), xml, error ) )
          {
            Console::Error( "%s\n", error.c_str() );
          }

          currentScene->Push( currentScene->ImportXML( xml, flags, currentScene->GetRoot() ) );
          break;
        }
      }

      currentScene->Execute(false);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Called when the user chooses to export a selection from the current editing
// scene.  Prompts the user with the export options, and performs the operation
// if the user does not cancel.
// 
void SceneEditor::OnExport(wxCommandEvent& event)
{
  static bool exportDependencies = true;
  static bool exportHierarchy = true;
  static bool exportBounded = false;
  static bool exportWorld = false;

  if ( m_SceneManager.HasCurrentScene() )
  {
    ExportOptionsDlg dlg ( this, exportDependencies, exportHierarchy, exportBounded, exportWorld );

    if ( dlg.ShowModal() == wxID_OK )
    {
      ExportArgs args;

      if ( exportHierarchy )
      {
        args.m_Flags |= ExportFlags::MaintainHierarchy;
      }

      if ( exportDependencies )
      {
        args.m_Flags |= ExportFlags::MaintainDependencies;
      }

      if ( exportBounded )
      {
        OS_SelectableDumbPtr::Iterator itr = m_SceneManager.GetCurrentScene()->GetSelection().GetItems().Begin();
        OS_SelectableDumbPtr::Iterator end = m_SceneManager.GetCurrentScene()->GetSelection().GetItems().End();
        for ( ; itr != end; ++itr )
        {
          Luna::HierarchyNode* node = Reflect::ObjectCast<Luna::HierarchyNode>( *itr );
          if ( node )
          {
            args.m_Bounds.push_back( node->GetGlobalHierarchyBounds() );
          }
        }
      }
      else
      {
        args.m_Flags |= ExportFlags::SelectedNodes;
      }

      u64 startTimer = Platform::TimerGetClock();

      CursorChanged( wxCURSOR_WAIT );

      {
        std::ostringstream str;
        str << "Preparing to export";
        StatusChanged( str.str() );
      }

      Undo::BatchCommandPtr changes = new Undo::BatchCommand();

      Reflect::V_Element elements;
      Asset::SceneManifestPtr manifest = new Asset::SceneManifest ();
      elements.push_back( manifest );

      bool result = true;
      Luna::Scene* root = m_SceneManager.GetRootScene();
      for ( S_ZoneDumbPtr::const_iterator itr = root->GetZones().begin(), end = root->GetZones().end(); itr != end && result; ++itr )
      {
        bool load = false;

        if ( !args.m_Bounds.empty() )
        {
          // the scene is not loaded, fetch its manifest and check to see if any of the bounds intersect
          Asset::SceneManifestPtr currentManifest = Reflect::Archive::FromFile< Asset::SceneManifest >( (*itr)->GetPath() );
          if ( currentManifest )
          {
            Math::AlignedBox bounds;
            bounds.minimum = currentManifest->m_BoundingBoxMin;
            bounds.maximum = currentManifest->m_BoundingBoxMax;

            bool intersection = false;
            for ( V_AlignedBox::const_iterator itr = args.m_Bounds.begin(), end = args.m_Bounds.end(); itr != end && !intersection; ++itr )
            {
              intersection = bounds.IntersectsBox( *itr );
            }

            load = intersection;
          }
        }

        Luna::Scene* scene = m_SceneManager.GetScene( (*itr)->GetPath() );

        if ( !scene && load )
        {
          std::string error;
          scene = m_SceneManager.OpenZone( (*itr)->GetPath(), error );
          if ( !scene )
          {
            wxMessageBox( error.c_str(), "Error", wxCENTER | wxICON_ERROR | wxOK, this );
          }
        }

        if ( !scene )
        {
          continue;
        }

        if ( !exportWorld && scene != m_SceneManager.GetCurrentScene() )
        {
          continue;
        }

        if ( scene )
        {
          Reflect::V_Element currentElements;
          result &= scene->Export( currentElements, args, changes );
          if ( result )
          {
            for ( Reflect::V_Element::const_iterator itr = currentElements.begin(), end = currentElements.end(); itr != end; ++itr )
            {
              Asset::SceneManifestPtr currentManifest = Reflect::ObjectCast<Asset::SceneManifest>( *itr );
              if ( currentManifest )
              {
                // merge bounds (this will be very pessimistic as we don't test each node)
                if (currentManifest->m_BoundingBoxMax.x > manifest->m_BoundingBoxMax.x)
                  manifest->m_BoundingBoxMax.x = currentManifest->m_BoundingBoxMax.x;

                if (currentManifest->m_BoundingBoxMin.x < manifest->m_BoundingBoxMin.x)
                  manifest->m_BoundingBoxMin.x = currentManifest->m_BoundingBoxMin.x;

                if (currentManifest->m_BoundingBoxMax.y > manifest->m_BoundingBoxMax.y)
                  manifest->m_BoundingBoxMax.y = currentManifest->m_BoundingBoxMax.y;

                if (currentManifest->m_BoundingBoxMin.y < manifest->m_BoundingBoxMin.y)
                  manifest->m_BoundingBoxMin.y = currentManifest->m_BoundingBoxMin.y;

                if (currentManifest->m_BoundingBoxMax.z > manifest->m_BoundingBoxMax.z)
                  manifest->m_BoundingBoxMax.z = currentManifest->m_BoundingBoxMax.z;

                if (currentManifest->m_BoundingBoxMin.z < manifest->m_BoundingBoxMin.z)
                  manifest->m_BoundingBoxMin.z = currentManifest->m_BoundingBoxMin.z;

                // merge assets (this is pessimistic too, but is the best we can muster)
                manifest->m_Assets.insert( currentManifest->m_Assets.begin(), currentManifest->m_Assets.end() );
              }
              else
              {
                elements.push_back( *itr );
              }
            }
          }
        }
      }

      if ( result && !elements.empty() )
      {
        switch ( event.GetId() )
        {
        case SceneEditorIDs::ID_FileExport:
          {
            UIToolKit::FileDialog fileDialog( this, "Export Selection", "", "", wxFileSelectorDefaultWildcardStr, UIToolKit::FileDialogStyles::DefaultSave );
            fileDialog.AddFilter( FinderSpecs::Extension::REFLECT_BINARY.GetDialogFilter() );
            fileDialog.AddFilter( FinderSpecs::Extension::REFLECT_TEXT.GetDialogFilter() );
            fileDialog.SetFilterIndex( FinderSpecs::Extension::REFLECT_BINARY.GetDialogFilter() );
            if ( fileDialog.ShowModal() != wxID_OK )
            {
              return;
            }
            
            std::string file = fileDialog.GetPath();

            try
            {
              Reflect::Archive::ToFile( elements, file.c_str(), new Content::ContentVersion (), m_SceneManager.GetCurrentScene() );
            }
            catch ( Nocturnal::Exception& ex )
            {
              std::ostringstream str;
              str << "Failed to generate file '" << file << "': " << ex.what();
              wxMessageBox( str.str(), "Error", wxOK|wxCENTRE|wxICON_ERROR );
              result = false;
            }

            break;
          }

        case SceneEditorIDs::ID_FileExportToClipboard:
          {
            std::string xml;

            try
            {
              Reflect::Archive::ToXML( elements, xml, m_SceneManager.GetCurrentScene() );
            }
            catch ( Nocturnal::Exception& ex )
            {
              std::ostringstream str;
              str << "Failed to generate xml: " << ex.what();
              wxMessageBox( str.str(), "Error", wxOK|wxCENTRE|wxICON_ERROR );
              result = false;
            }

            std::string error;
            if ( !Windows::CopyToClipboard( GetHwnd(), xml, error ) )
            {
              Console::Error( "%s\n", error.c_str() );
            }

            break;
          }
        }
      }

      changes->Undo();

      CursorChanged( wxCURSOR_ARROW );

      {
        std::ostringstream str;
        str.precision( 2 );
        str << "Export Complete: " << std::fixed << Platform::CyclesToMillis( Platform::TimerGetClock() - startTimer ) / 1000.f << " seconds...";
        StatusChanged( str.str() );
        TitleChanged( std::string( "Luna Scene Editor" ) );
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Called when the "Export to OBJ file" item is chosen in the UI.
// Exports the selection to an OBJ file.
// 
void SceneEditor::OnExportToObj(wxCommandEvent& event)
{
  if ( !m_SceneManager.HasCurrentScene() )
  {
    return;
  }
  
  OS_SelectableDumbPtr selection;
  m_SceneManager.GetCurrentScene()->GetFlattenedSelection( selection );
  if ( selection.Empty() )
  {
    return;
  }

  UIToolKit::FileDialog fileDialog( this, "Export Selection", "", "", wxFileSelectorDefaultWildcardStr, UIToolKit::FileDialogStyles::DefaultSave );
  fileDialog.AddFilter( FinderSpecs::Extension::MAYA_OBJ.GetDialogFilter() );

  if ( fileDialog.ShowModal() != wxID_OK )
  {
    return;
  }
  
  Math::V_Vector3 vertices;
  V_u32 indices;
  
  for ( OS_SelectableDumbPtr::Iterator itr = selection.Begin(), end = selection.End(); itr != end; ++itr )
  {
    Luna::Entity* entity = Reflect::ObjectCast< Luna::Entity >( *itr );
    if ( !entity )
    {
      continue;
    }

    const Luna::Scene* geometryScene = entity->GetNestedScene( GeometryModes::Render );
    if ( !geometryScene )
    {
      continue;
    }

    const Math::Matrix4& worldSpaceTransform = entity->GetGlobalTransform();

    HM_SceneNodeDumbPtr geometrySceneNodes = geometryScene->GetNodes();
    for ( HM_SceneNodeDumbPtr::const_iterator nodeItr = geometrySceneNodes.begin(), nodeEnd = geometrySceneNodes.end(); nodeItr != nodeEnd; ++nodeItr )
    {
      const Luna::Mesh* mesh = Reflect::ConstObjectCast< Luna::Mesh >( nodeItr->second );
      if ( !mesh )
      {
        continue;
      }
      
      const Content::Mesh* contentMesh = mesh->GetPackage< Content::Mesh >();
      if ( !contentMesh )
      {
        continue;
      }
      
      u32 startingIndex = (u32) vertices.size() + 1;
      for ( Math::V_Vector3::const_iterator vertexItr = contentMesh->m_Positions.begin(), vertexEnd = contentMesh->m_Positions.end(); vertexItr != vertexEnd; ++vertexItr )
      {
        Math::Vector3 vertex( *vertexItr );
        worldSpaceTransform.TransformVertex( vertex );
        vertex *= 100.0f;
        vertices.push_back( vertex );
      }
      
      for ( V_u32::const_iterator indexItr = contentMesh->m_TriangleVertexIndices.begin(), indexEnd = contentMesh->m_TriangleVertexIndices.end(); indexItr != indexEnd; ++indexItr )
      {
        indices.push_back( startingIndex + *indexItr );
      }
    }
  }

  std::string file = fileDialog.GetPath();
  FILE* f = fopen( file.c_str(), "w" );
  if ( !f )
  {
    std::string errorMessage = "Failed to open '" + file + "' for writing!";
    wxMessageBox( errorMessage, "Error", wxCENTER | wxICON_ERROR | wxOK );
    return;
  }
  
  fprintf( f, "#                      Vertices: %d\n", vertices.size() );
  fprintf( f, "#                        Points: 0\n" );
  fprintf( f, "#                         Lines: 0\n" );
  fprintf( f, "#                         Faces: %d\n", indices.size() / 3 );
  fprintf( f, "#                     Materials: 0\n" );
  fprintf( f, "\no 1\n" );
  
  fprintf( f, "\n# Vertex list\n\n" );
  for ( Math::V_Vector3::iterator itr = vertices.begin(), end = vertices.end(); itr != end; ++itr )
  {
    fprintf( f, "v %f %f %f\n", itr->x, itr->y, itr->z );
  }
  
  fprintf( f, "\n# Point/Line/Face list\n\n" );
  fprintf( f, "usemtl Default\n" );
  V_u32::iterator indexItr = indices.begin();
  V_u32::iterator indexEnd = indices.end();
  while ( indexItr != indexEnd )
  {
    u32 index1 = 0;
    u32 index2 = 0;
    u32 index3 = 0;

    if ( indexItr != indexEnd )
    {
      index1 = *indexItr;
      ++indexItr;
    }

    if ( indexItr != indexEnd )
    {
      index2 = *indexItr;
      ++indexItr;
    }

    if ( indexItr != indexEnd )
    {
      index3 = *indexItr;
      ++indexItr;
    }

    fprintf( f, "f %d %d %d\n", index1, index2, index3 );
  }
  
  fprintf( f, "\n# End of file\n" );

  fclose( f );
}

///////////////////////////////////////////////////////////////////////////////
// Called when the "close" item is chosen in the UI.  Closes all currently 
// open scenes.
// 
void SceneEditor::OnClose(wxCommandEvent& event)
{
  ReleaseViewerControl();
  m_SceneManager.CloseAll();
}

///////////////////////////////////////////////////////////////////////////////
// Prompts the user to open a new session.
// 
void SceneEditor::OnOpenSession(wxCommandEvent& event)
{
  PromptLoadSession();
}

///////////////////////////////////////////////////////////////////////////////
// Prompts the user to save a session (no prompt if they have previously saved
// a session).
// 
void SceneEditor::OnSaveSession(wxCommandEvent& event)
{
  PromptSaveSession();
}

///////////////////////////////////////////////////////////////////////////////
// Prompts the user to save a session.
// 
void SceneEditor::OnSaveSessionAs(wxCommandEvent& event)
{
  PromptSaveSession( true );
}

///////////////////////////////////////////////////////////////////////////////
// Called when the user clicks on the exit item in the menu.  Fires an event
// to close the frame.
// 
void SceneEditor::OnExit(wxCommandEvent& event)
{
  wxCloseEvent closeEvent( wxEVT_CLOSE_WINDOW );
  GetEventHandler()->AddPendingEvent( closeEvent );
}

///////////////////////////////////////////////////////////////////////////////
// Called when the frame is about to be closed.  Closes all open files.
// 
void SceneEditor::OnExiting( wxCloseEvent& args )
{
  if ( !m_SceneManager.CloseAll() )
  {
    if ( args.CanVeto() )
    {
      args.Veto();
      return;
    }
  }

  args.Skip();
}

void SceneEditor::OnUndo(wxCommandEvent& event)
{
  if ( m_SceneManager.CanUndo() )
  {
    m_SceneManager.Undo();
    m_ToolProperties.Read();
    if ( m_SceneManager.HasCurrentScene() )
    {
      m_SceneManager.GetCurrentScene()->Execute(false);
    }
  }
}

void SceneEditor::OnRedo(wxCommandEvent& event)
{
  if ( m_SceneManager.CanRedo() )
  {
    m_SceneManager.Redo();
    m_ToolProperties.Read();
    if ( m_SceneManager.HasCurrentScene() )
    {
      m_SceneManager.GetCurrentScene()->Execute(false);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Called when the user chooses to cut the selected item. 
// 
void SceneEditor::OnCut(wxCommandEvent& event)
{
  if ( m_SceneManager.HasCurrentScene() )
  {
    if ( Copy( m_SceneManager.GetCurrentScene() ) )
    {
      m_SceneManager.GetCurrentScene()->Push( m_SceneManager.GetCurrentScene()->DeleteSelected() );
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Called when the user chooses to copy the selected item.
// 
void SceneEditor::OnCopy(wxCommandEvent& event)
{
  // special copy handler for navmesh
  if ( m_SceneManager.HasCurrentScene() && m_SceneManager.GetCurrentScene()->GetTool() && m_SceneManager.GetCurrentScene()->GetTool()->GetType() == Reflect::GetType<Luna::NavMeshCreateTool>() )
  {
    Luna::NavMeshCreateTool* navMeshCreate = static_cast<NavMeshCreateTool*>( m_SceneManager.GetCurrentScene()->GetTool().Ptr() );
    navMeshCreate->CopySelected();
  }
  else if ( m_SceneManager.HasCurrentScene() && m_SceneManager.GetCurrentScene()->GetSelection().GetItems().Size() > 0 )
  {
    if ( !Copy( m_SceneManager.GetCurrentScene() ) )
    {
      const char* msg = "There was an error while copying.  Refer to the output window for more details";
      wxMessageDialog msgBox( this, msg, "Error", wxOK | wxICON_EXCLAMATION );
      msgBox.ShowModal();
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Called when the user chooses to paste a previously cut or copied item.
// 
void SceneEditor::OnPaste(wxCommandEvent& event)
{
  // special copy handler for navmesh
  if ( m_SceneManager.HasCurrentScene() && m_SceneManager.GetCurrentScene()->GetTool() && m_SceneManager.GetCurrentScene()->GetTool()->GetType() == Reflect::GetType<Luna::NavMeshCreateTool>() )
  {
    Luna::NavMeshCreateTool* navMeshCreate = static_cast<NavMeshCreateTool*>( m_SceneManager.GetCurrentScene()->GetTool().Ptr() );
    navMeshCreate->Paste();
  }
  else if ( m_SceneManager.HasCurrentScene() )
  {
    Paste( m_SceneManager.GetCurrentScene() );
  }
}

///////////////////////////////////////////////////////////////////////////////
// Called when the user chooses Delete from the Edit menu. Deletes the current
// selection from the current scene.
// 
void SceneEditor::OnDelete(wxCommandEvent& event)
{
  if ( m_SelectionPropertiesManager->ThreadsActive() )
  {
    wxMessageBox( "You cannot delete items while the Properties Panel is generating.", "Error", wxCENTER | wxOK | wxICON_ERROR, this );
    return;
  }

  if ( m_SceneManager.HasCurrentScene() )
  {
    m_SceneManager.GetCurrentScene()->Push( m_SceneManager.GetCurrentScene()->DeleteSelected() );
  }
}

void SceneEditor::OnHelpIndex( wxCommandEvent& event )
{
  Windows::Execute( "cmd /c start http://wiki/index.php/Luna" );
}

void SceneEditor::OnHelpSearch( wxCommandEvent& event )
{
  Windows::Execute( "cmd /c start http://wiki/index.php/Special:Search" );
}

void SceneEditor::OnPickWalk( wxCommandEvent& event )
{
  if (m_SceneManager.HasCurrentScene())
  {
    switch (event.GetId())
    {
    case SceneEditorIDs::ID_EditWalkUp:
      {
        m_SceneManager.GetCurrentScene()->Push( m_SceneManager.GetCurrentScene()->PickWalkUp() );
        break;
      }

    case SceneEditorIDs::ID_EditWalkDown:
      {
        m_SceneManager.GetCurrentScene()->Push( m_SceneManager.GetCurrentScene()->PickWalkDown() );
        break;
      }

    case SceneEditorIDs::ID_EditWalkForward:
      {
        m_SceneManager.GetCurrentScene()->Push( m_SceneManager.GetCurrentScene()->PickWalkSibling(true) );
        break;
      }

    case SceneEditorIDs::ID_EditWalkBackward:
      {
        m_SceneManager.GetCurrentScene()->Push( m_SceneManager.GetCurrentScene()->PickWalkSibling(false) );
        break;
      }
    }
  }
}

void SceneEditor::OnEditPreferences( wxCommandEvent& event )
{
  ScenePreferencesDialog scenePreferencesDialog( this, wxID_ANY, "Preferences" );
  scenePreferencesDialog.ShowModal();
}

void SceneEditor::OnViewChange(wxCommandEvent& event)
{
  switch (event.GetId())
  {
  case SceneEditorIDs::ID_ViewAxes:
    {
      m_View->SetAxesVisible( !m_View->IsAxesVisible() );
      break;
    }

  case SceneEditorIDs::ID_ViewGrid:
    {
      m_View->SetGridVisible( !m_View->IsGridVisible() );
      break;
    }

  case SceneEditorIDs::ID_ViewBounds:
    {
      m_View->SetBoundsVisible( !m_View->IsBoundsVisible() );
      break;
    }

  case SceneEditorIDs::ID_ViewStatistics:
    {
      m_View->SetStatisticsVisible( !m_View->IsStatisticsVisible() );
      break;
    }

  case SceneEditorIDs::ID_ViewNone:
    {
      m_View->SetGeometryMode( GeometryModes::None );
      break;
    }

  case SceneEditorIDs::ID_ViewRender:
    {
      m_View->SetGeometryMode( GeometryModes::Render );
      break;
    }

  case SceneEditorIDs::ID_ViewCollision:
    {
      m_View->SetGeometryMode( GeometryModes::Collision );
      break;
    }

  case SceneEditorIDs::ID_ViewPathfinding:
    {
      m_View->SetPathfindingVisible( !m_View->IsPathfindingVisible() );
      break;
    }

  case SceneEditorIDs::ID_ViewWireframeOnMesh:
    {
      m_View->GetCamera()->SetWireframeOnMesh( !m_View->GetCamera()->GetWireframeOnMesh() );
      break;
    }

  case SceneEditorIDs::ID_ViewWireframeOnShaded:
    {
      m_View->GetCamera()->SetWireframeOnShaded( !m_View->GetCamera()->GetWireframeOnShaded() );
      break;
    }

  case SceneEditorIDs::ID_ViewWireframe:
    {
      m_View->GetCamera()->SetShadingMode( ShadingModes::Wireframe );
      break;
    }

  case SceneEditorIDs::ID_ViewMaterial:
    {
      m_View->GetCamera()->SetShadingMode( ShadingModes::Material );
      break;
    }

  case SceneEditorIDs::ID_ViewTexture:
    {
      m_View->GetCamera()->SetShadingMode( ShadingModes::Texture );
      break;
    }

  case SceneEditorIDs::ID_ViewFrustumCulling:
    {
      m_View->GetCamera()->SetViewFrustumCulling( !m_View->GetCamera()->IsViewFrustumCulling() );
      break;
    }

  case SceneEditorIDs::ID_ViewBackfaceCulling:
    {
      m_View->GetCamera()->SetBackFaceCulling( !m_View->GetCamera()->IsBackFaceCulling() );
      break;
    }
  }

  m_View->Refresh();
}

void SceneEditor::OnViewCameraChange(wxCommandEvent& event)
{
  switch (event.GetId())
  {
  case SceneEditorIDs::ID_ViewOrbit:
    {
      m_View->SetCameraMode(CameraModes::Orbit);
      break;
    }

  case SceneEditorIDs::ID_ViewFront:
    {
      m_View->SetCameraMode(CameraModes::Front);
      break;
    }

  case SceneEditorIDs::ID_ViewSide:
    {
      m_View->SetCameraMode(CameraModes::Side);
      break;
    }

  case SceneEditorIDs::ID_ViewTop:
    {
      m_View->SetCameraMode(CameraModes::Top);
      break;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Handles callbacks for menu items dealing with View->Show and View->Hide.
// Changes the visibility of items according to which command was called.
// 
void SceneEditor::OnViewVisibleChange(wxCommandEvent& event)
{
  if ( m_SceneManager.HasCurrentScene() )
  {
    Undo::BatchCommandPtr batch = new Undo::BatchCommand ();

    switch ( event.GetId() )
    {
    case SceneEditorIDs::ID_ViewShowAll:
      {
        batch->Push( m_SceneManager.GetCurrentScene()->SetHiddenSelected( false ) );
        batch->Push( m_SceneManager.GetCurrentScene()->SetHiddenUnrelated( false ) );
        break;
      }

    case SceneEditorIDs::ID_ViewShowAllGeometry:
      {
        batch->Push( m_SceneManager.GetCurrentScene()->SetGeometryShown( true, true ) );
        batch->Push( m_SceneManager.GetCurrentScene()->SetGeometryShown( true, false ) );
        break;
      }

    case SceneEditorIDs::ID_ViewShowSelected:
      {
        batch->Push( m_SceneManager.GetCurrentScene()->SetHiddenSelected( false ) );
        break;
      }

    case SceneEditorIDs::ID_ViewShowSelectedGeometry:
      {
        batch->Push( m_SceneManager.GetCurrentScene()->SetGeometryShown( true, true ) );
        break;
      }

    case SceneEditorIDs::ID_ViewShowUnrelated:
      {
        batch->Push( m_SceneManager.GetCurrentScene()->SetHiddenUnrelated( false ) );
        break;
      }

    case SceneEditorIDs::ID_ViewShowLastHidden:
      {
        batch->Push( m_SceneManager.GetCurrentScene()->ShowLastHidden() );
        break;
      }

    case SceneEditorIDs::ID_ViewHideAll:
      {
        batch->Push( m_SceneManager.GetCurrentScene()->SetHiddenSelected( true ) );
        batch->Push( m_SceneManager.GetCurrentScene()->SetHiddenUnrelated( true ) );
        break;
      }

    case SceneEditorIDs::ID_ViewHideAllGeometry:
      {
        batch->Push( m_SceneManager.GetCurrentScene()->SetGeometryShown( false, true ) );
        batch->Push( m_SceneManager.GetCurrentScene()->SetGeometryShown( false, false ) );
        break;
      }

    case SceneEditorIDs::ID_ViewHideSelected:
      {
        batch->Push( m_SceneManager.GetCurrentScene()->SetHiddenSelected( true ) );
        break;
      }

    case SceneEditorIDs::ID_ViewHideSelectedGeometry:
      {
        batch->Push( m_SceneManager.GetCurrentScene()->SetGeometryShown( false, true ) );
        break;
      }

    case SceneEditorIDs::ID_ViewHideUnrelated:
      {
        batch->Push( m_SceneManager.GetCurrentScene()->SetHiddenUnrelated( true ) );
        break;
      }

    default:
      {
        Console::Warning( "SceneEditor::OnViewVisibleChange - Unhandled case\n" );
        return;
      }
    }

    m_SceneManager.GetCurrentScene()->Push( batch );

    m_SceneManager.GetCurrentScene()->Execute( false );
  }
}

void SceneEditor::OnViewColorModeChange(wxCommandEvent& event)
{
  const ViewColorMode previousColorMode = SceneEditorPreferences()->GetViewPreferences()->GetColorMode();

  const M_IDToColorMode::const_iterator newColorModeItr = m_ColorModeLookup.find( event.GetId() );
  if ( newColorModeItr != m_ColorModeLookup.end() )
  {
    SceneEditorPreferences()->GetViewPreferences()->SetColorMode( ( ViewColorMode )( newColorModeItr->second ) );
  }
}

void SceneEditor::OnViewDefaultsChange(wxCommandEvent& event)
{
  Content::NodeVisibilityPtr nodeDefaults = SceneEditorPreferences()->GetDefaultNodeVisibility(); 
  
  switch ( event.GetId() )
  {
  case SceneEditorIDs::ID_ViewDefaultShowLayers: 
    nodeDefaults->SetVisibleLayer( !nodeDefaults->GetVisibleLayer() ); 
    break; 
  case SceneEditorIDs::ID_ViewDefaultShowInstances: 
    nodeDefaults->SetHiddenNode( !nodeDefaults->GetHiddenNode() ); 
    break; 
  case SceneEditorIDs::ID_ViewDefaultShowGeometry: 
    nodeDefaults->SetShowGeometry( !nodeDefaults->GetShowGeometry() ); 
    break; 
  case SceneEditorIDs::ID_ViewDefaultShowPointer: 
    nodeDefaults->SetShowPointer( !nodeDefaults->GetShowPointer() ); 
    break; 
  case SceneEditorIDs::ID_ViewDefaultShowBounds: 
    nodeDefaults->SetShowBounds( !nodeDefaults->GetShowBounds() ); 
    break; 
  }

}


void SceneEditor::OnFrameOrigin(wxCommandEvent& event)
{
  m_View->UpdateCameraHistory();      // we want the previous state before the move
  m_View->GetCamera()->Reset();
  m_View->Refresh();
}

void SceneEditor::OnFrameSelected(wxCommandEvent& event)
{
  if ( m_SceneManager.HasCurrentScene() )
  {
    bool found = false;
    Math::AlignedBox box;

    OS_SelectableDumbPtr::Iterator itr = m_SceneManager.GetCurrentScene()->GetSelection().GetItems().Begin();
    OS_SelectableDumbPtr::Iterator end = m_SceneManager.GetCurrentScene()->GetSelection().GetItems().End();
    for ( ; itr != end; ++itr )
    {
      Luna::HierarchyNode* node = Reflect::ObjectCast<Luna::HierarchyNode>(*itr);
      if (node)
      {
        box.Merge(node->GetGlobalHierarchyBounds());
        found = true;
        continue;
      }

      Luna::Point* point = Reflect::ObjectCast<Luna::Point>(*itr);
      if (point)
      {
        Math::Vector3 p = point->GetPosition();
        point->GetTransform()->GetGlobalTransform().TransformVertex(p);
        box.Merge(p);
        found = true;
        continue;
      }
    }

    if (found)
    {
      m_View->UpdateCameraHistory();    // we want the previous state before the move
      m_View->GetCamera()->Frame(box);

      m_SceneManager.GetCurrentScene()->Execute(false);
    }
  }
}

void SceneEditor::OnHighlightMode(wxCommandEvent& event)
{
  m_View->SetHighlighting( !m_View->IsHighlighting() );
}

void SceneEditor::OnPreviousView(wxCommandEvent& event)
{
  m_View->UndoTransform();
}

void SceneEditor::OnNextView(wxCommandEvent& event)
{
  m_View->RedoTransform();
}

void SceneEditor::OnToolSelected(wxCommandEvent& event)
{
  if (m_SceneManager.HasCurrentScene())
  {
    switch (event.GetId())
    {
    case SceneEditorIDs::ID_ToolsSelect:
      {
        m_SceneManager.GetCurrentScene()->SetTool(NULL);
        m_Properties->SetSelection(m_SelectionPropertyPage);
        break;
      }

    case SceneEditorIDs::ID_ToolsScale:
      {
        m_SceneManager.GetCurrentScene()->SetTool(new Luna::ScaleManipulator (ManipulatorModes::Scale, m_SceneManager.GetCurrentScene(), m_ToolEnumerator));
        break;
      }

    case SceneEditorIDs::ID_ToolsScalePivot:
      {
        m_SceneManager.GetCurrentScene()->SetTool(new Luna::TranslateManipulator (ManipulatorModes::ScalePivot, m_SceneManager.GetCurrentScene(), m_ToolEnumerator));
        break;
      }

    case SceneEditorIDs::ID_ToolsRotate:
      {
        m_SceneManager.GetCurrentScene()->SetTool(new Luna::RotateManipulator (ManipulatorModes::Rotate, m_SceneManager.GetCurrentScene(), m_ToolEnumerator));
        break;
      }

    case SceneEditorIDs::ID_ToolsRotatePivot:
      {
        m_SceneManager.GetCurrentScene()->SetTool(new Luna::TranslateManipulator (ManipulatorModes::RotatePivot, m_SceneManager.GetCurrentScene(), m_ToolEnumerator));
        break;
      }

    case SceneEditorIDs::ID_ToolsTranslate:
      {
        m_SceneManager.GetCurrentScene()->SetTool(new Luna::TranslateManipulator (ManipulatorModes::Translate, m_SceneManager.GetCurrentScene(), m_ToolEnumerator));
        break;
      }

    case SceneEditorIDs::ID_ToolsTranslatePivot:
      {
        m_SceneManager.GetCurrentScene()->SetTool(new Luna::TranslateManipulator (ManipulatorModes::TranslatePivot, m_SceneManager.GetCurrentScene(), m_ToolEnumerator));
        break;
      }

    case SceneEditorIDs::ID_ToolsPivot:
      {
        if (m_SceneManager.GetCurrentScene()->GetTool().ReferencesObject())
        {
          if ( m_SceneManager.GetCurrentScene()->GetTool()->GetType() == Reflect::GetType<Luna::ScaleManipulator>() )
          {
            m_SceneManager.GetCurrentScene()->SetTool(new Luna::TranslateManipulator (ManipulatorModes::ScalePivot, m_SceneManager.GetCurrentScene(), m_ToolEnumerator));
          }
          else if ( m_SceneManager.GetCurrentScene()->GetTool()->GetType() == Reflect::GetType<Luna::RotateManipulator>() )
          {
            m_SceneManager.GetCurrentScene()->SetTool(new Luna::TranslateManipulator (ManipulatorModes::RotatePivot, m_SceneManager.GetCurrentScene(), m_ToolEnumerator));
          }
          else if ( m_SceneManager.GetCurrentScene()->GetTool()->GetType() == Reflect::GetType<Luna::TranslateManipulator>() )
          {
            Luna::TranslateManipulator* manipulator = Reflect::AssertCast< Luna::TranslateManipulator > (m_SceneManager.GetCurrentScene()->GetTool());

            if ( manipulator->GetMode() == ManipulatorModes::Translate)
            {
              m_SceneManager.GetCurrentScene()->SetTool(new Luna::TranslateManipulator (ManipulatorModes::TranslatePivot, m_SceneManager.GetCurrentScene(), m_ToolEnumerator));
            }
            else
            {
              switch ( manipulator->GetMode() )
              {
              case ManipulatorModes::ScalePivot:
                m_SceneManager.GetCurrentScene()->SetTool(new Luna::ScaleManipulator (ManipulatorModes::Scale, m_SceneManager.GetCurrentScene(), m_ToolEnumerator));
                break;
              case ManipulatorModes::RotatePivot:
                m_SceneManager.GetCurrentScene()->SetTool(new Luna::RotateManipulator (ManipulatorModes::Rotate, m_SceneManager.GetCurrentScene(), m_ToolEnumerator));
                break;
              case ManipulatorModes::TranslatePivot:
                m_SceneManager.GetCurrentScene()->SetTool(new Luna::TranslateManipulator (ManipulatorModes::Translate, m_SceneManager.GetCurrentScene(), m_ToolEnumerator));
                break;
              }
            }
          }
        }
        break;
      }

    case SceneEditorIDs::ID_ToolsEntityCreate:
      {
        m_SceneManager.GetCurrentScene()->SetTool(new Luna::EntityCreateTool (m_SceneManager.GetCurrentScene(), m_ToolEnumerator));
        m_Properties->SetSelection(m_ToolPropertyPage);
      }
      break;

    case SceneEditorIDs::ID_ToolsVolumeCreate:
      {
        m_SceneManager.GetCurrentScene()->SetTool(new Luna::VolumeCreateTool (m_SceneManager.GetCurrentScene(), m_ToolEnumerator));
        m_Properties->SetSelection(m_ToolPropertyPage);
      }
      break;

    case SceneEditorIDs::ID_ToolsClueCreate:
      {
        m_SceneManager.GetCurrentScene()->SetTool(new Luna::ClueCreateTool (m_SceneManager.GetCurrentScene(), m_ToolEnumerator));
        m_Properties->SetSelection(m_ToolPropertyPage);
      }
      break;

#if LUNA_GAME_CAMERA
    case SceneEditorIDs::ID_ToolsGameCameraCreate:
      {
        m_SceneManager.GetCurrentScene()->SetTool(new Luna::GameCameraCreateTool (m_SceneManager.GetCurrentScene(), m_ToolEnumerator));
        m_Properties->SetSelection(m_ToolPropertyPage);
      }
      break;
#endif

    case SceneEditorIDs::ID_ToolsControllerCreate:
      {
        m_SceneManager.GetCurrentScene()->SetTool(new Luna::ControllerCreateTool (m_SceneManager.GetCurrentScene(), m_ToolEnumerator));
        m_Properties->SetSelection(m_ToolPropertyPage);
      }
      break;

    case SceneEditorIDs::ID_ToolsLocatorCreate:
      {
        m_SceneManager.GetCurrentScene()->SetTool(new Luna::LocatorCreateTool (m_SceneManager.GetCurrentScene(), m_ToolEnumerator));
        m_Properties->SetSelection(m_ToolPropertyPage);
      }
      break;

    case SceneEditorIDs::ID_ToolsDuplicate:
      {
        m_SceneManager.GetCurrentScene()->SetTool(new Luna::DuplicateTool (m_SceneManager.GetCurrentScene(), m_ToolEnumerator));
        m_Properties->SetSelection(m_ToolPropertyPage);
      }
      break;

    case SceneEditorIDs::ID_ToolsCurveCreate:
      {
        m_SceneManager.GetCurrentScene()->SetTool( new Luna::CurveCreateTool( m_SceneManager.GetCurrentScene(), m_ToolEnumerator ) );
        m_Properties->SetSelection(m_ToolPropertyPage);
      }
      break;

    case SceneEditorIDs::ID_ToolsCurveEdit:
      {
        Luna::CurveEditTool* curveEditTool = new Luna::CurveEditTool( m_SceneManager.GetCurrentScene(), m_ToolEnumerator );
        m_SceneManager.GetCurrentScene()->SetTool( curveEditTool );
        curveEditTool->StoreSelectedCurves();
        m_Properties->SetSelection(m_ToolPropertyPage);
      }
      break;

    case SceneEditorIDs::ID_ToolsLightCreate:
      {
        m_SceneManager.GetCurrentScene()->SetTool(new Luna::LightCreateTool (m_SceneManager.GetCurrentScene(), m_ToolEnumerator));
        m_Properties->SetSelection(m_ToolPropertyPage);
      }
      break;

    case SceneEditorIDs::ID_ToolsLighting:
      {
        m_SceneManager.GetCurrentScene()->SetTool(new Luna::LightingTool (m_SceneManager.GetCurrentScene(), m_ToolEnumerator));
        m_Properties->SetSelection(m_ToolPropertyPage);
      }
      break;

    case SceneEditorIDs::ID_ToolsPostProcessingVolumeCreate:
      {
        m_SceneManager.GetCurrentScene()->SetTool(new Luna::PostProcessingVolumeCreateTool (m_SceneManager.GetCurrentScene(), m_ToolEnumerator));
        m_Properties->SetSelection(m_ToolPropertyPage);
      }
      break;

    case SceneEditorIDs::ID_ToolsPostProcessingVolumeScript:
      {
        GeneratePostProcessingVolumeScript();
      }
      break;

    case SceneEditorIDs::ID_ToolsLightingLayerCreate:
      {
        wxCommandEvent  event(wxEVT_COMMAND_MENU_SELECTED, LayerGrid::ID_NewLayerFromSelection);
        LayerGridPtr&   lightingLayerGridPtr  = m_LayerGrids[Content::LayerTypes::LT_Lighting];
        wxToolBar*      toolBar               = lightingLayerGridPtr->GetToolBar();

        m_LayersNotebook->SetSelection(Content::LayerTypes::LT_Lighting);
        event.SetEventObject(toolBar);
        toolBar->GetEventHandler()->ProcessEvent(event);
       
        //Update light links on the remote target
        m_RemoteScene->SendLightingVolumes();
      }
      break;

    case SceneEditorIDs::ID_ToolsLightingLayerUnlink:
      {
        LayerGridPtr&   lightingLayerGridPtr = m_LayerGrids[Content::LayerTypes::LT_Lighting];
        m_LayersNotebook->SetSelection(Content::LayerTypes::LT_Lighting);
        lightingLayerGridPtr->UnlinkSelectedElements();
        
        //Update light links on the remote target
        m_RemoteScene->SendLightingVolumes();
      }
      break;

    case SceneEditorIDs::ID_ToolsLightingLayerSelect:
      {
        LayerGridPtr&   lightingLayerGridPtr = m_LayerGrids[Content::LayerTypes::LT_Lighting];
        m_LayersNotebook->SetSelection(Content::LayerTypes::LT_Lighting);
        lightingLayerGridPtr->SelectLinkedElements();
      }
      break;

    case SceneEditorIDs::ID_ToolsNavMeshCreate:
      {
        bool success = false;

        Luna::Scene* rootScene = m_SceneManager.GetRootScene();
        if (rootScene)
        {
          Luna::ZonePtr& luna_nav_zone_ptr = rootScene->GetNavZone();
          if (luna_nav_zone_ptr)
          {
            Luna::Scene* luna_zone_scene_ptr = m_SceneManager.GetScene( luna_nav_zone_ptr->GetPath() );
            if (luna_zone_scene_ptr && luna_zone_scene_ptr->IsCurrent())
            {
              if ( m_SceneManager.GetCurrentScene()->GetTool() && m_SceneManager.GetCurrentScene()->GetTool()->GetType() == Reflect::GetType<Luna::NavMeshCreateTool>() )
              {
                Luna::NavMeshCreateTool* navMeshCreate = static_cast<NavMeshCreateTool*>( m_SceneManager.GetCurrentScene()->GetTool().Ptr() );
                navMeshCreate->SetEditMode( Luna::NavMeshCreateTool::EDIT_MODE_ADD );
              }
              else
              {
                Luna::NavMeshCreateTool* navMeshCreate = new Luna::NavMeshCreateTool (m_SceneManager.GetCurrentScene(), m_ToolEnumerator);
                m_SceneManager.GetCurrentScene()->SetTool( navMeshCreate );
                m_Properties->SetSelection(m_ToolPropertyPage);
                navMeshCreate->SetEditMode(NavMeshCreateTool::EDIT_MODE_ADD);
              }
              success = true;
            }
          }
        }

        if ( !success )
        {
          m_NavToolBar->ToggleTool( SceneEditorIDs::ID_ToolsNavMeshCreate, false ); 
          std::string errorString = std::string( "Please select a zone with HasNavData attribute" );
          wxMessageBox( errorString.c_str(), "Active zone must have NavMesh data", wxOK|wxCENTRE|wxICON_ERROR, this );
          break;
        }
      }
      break;
    case SceneEditorIDs::ID_ToolsNavMeshWorkWithLOWRes:
      {
        Luna::Scene* rootScene = m_SceneManager.GetRootScene();
         bool success = false;
        if (rootScene)
        {
          Luna::ZonePtr& luna_nav_zone_ptr = rootScene->GetNavZone();
          if (luna_nav_zone_ptr)
          {
            Luna::Scene* luna_zone_scene_ptr = m_SceneManager.GetScene( luna_nav_zone_ptr->GetPath() );
            if (luna_zone_scene_ptr && luna_zone_scene_ptr->IsCurrent())
            {
              Luna::NavMeshCreateTool* navMeshCreate;
              if ( m_SceneManager.GetCurrentScene()->GetTool() &&  m_SceneManager.GetCurrentScene()->GetTool()->GetType() == Reflect::GetType<Luna::NavMeshCreateTool>() )
              {
                success = true;
                navMeshCreate = static_cast<NavMeshCreateTool*>( m_SceneManager.GetCurrentScene()->GetTool().Ptr() );
                //now change it to be selection
                navMeshCreate->SetResolutionMode(GetNavToolBar()->GetToolState(SceneEditorIDs::ID_ToolsNavMeshWorkWithLOWRes));
                m_Properties->SetSelection(m_ToolPropertyPage);
              }    
            }
          }
        }
        if ( !success )
        {
          m_NavToolBar->ToggleTool( SceneEditorIDs::ID_ToolsNavMeshWorkWithLOWRes, false ); 
          std::string errorString = std::string( "Please select a zone with HasNavData attribute" );
          wxMessageBox( errorString.c_str(), "Active zone must have NavMesh data", wxOK|wxCENTRE|wxICON_ERROR, this );
          break;
        }
      }
      break;

    case SceneEditorIDs::ID_ToolsNavMeshPunchOut:
    case SceneEditorIDs::ID_ToolsNavMeshPunchOutTranslate:
    case SceneEditorIDs::ID_ToolsNavMeshPunchOutRotate:
    case SceneEditorIDs::ID_ToolsNavMeshPunchOutScale:
      {
        Luna::Scene* rootScene = m_SceneManager.GetRootScene();
        bool success = false;
        if (rootScene)
        {
          Luna::ZonePtr& luna_nav_zone_ptr = rootScene->GetNavZone();
          if (luna_nav_zone_ptr)
          {
            Luna::Scene* luna_zone_scene_ptr = m_SceneManager.GetScene( luna_nav_zone_ptr->GetPath() );
            if (luna_zone_scene_ptr && luna_zone_scene_ptr->IsCurrent())
            {
              success = true;
              Luna::NavMeshCreateTool* navMeshCreate;
              if ( m_SceneManager.GetCurrentScene()->GetTool() && m_SceneManager.GetCurrentScene()->GetTool()->GetType() == Reflect::GetType<Luna::NavMeshCreateTool>() )
              {
                navMeshCreate = static_cast<NavMeshCreateTool*>( m_SceneManager.GetCurrentScene()->GetTool().Ptr() );
              }
              else
              {
                navMeshCreate = new Luna::NavMeshCreateTool (m_SceneManager.GetCurrentScene(), m_ToolEnumerator);
                m_SceneManager.GetCurrentScene()->SetTool( navMeshCreate );
              }
              if ( event.GetId() == SceneEditorIDs::ID_ToolsNavMeshPunchOut )
              {
                if ( m_NavToolBar->GetToolState( SceneEditorIDs::ID_ToolsNavMeshPunchOut ))
                {
                  navMeshCreate->SetEditMode(NavMeshCreateTool::EDIT_MODE_CUBE_PUNCH_OUT);
                  navMeshCreate->SetPunchOutMode(NavMeshCreateTool::EDIT_MODE_DISABLED);
                  m_Properties->SetSelection(m_ToolPropertyPage);
                }
                else
                {
                  navMeshCreate->SetEditMode(NavMeshCreateTool::EDIT_MODE_ADD);
                }
              }
              else if ( event.GetId() == SceneEditorIDs::ID_ToolsNavMeshPunchOutTranslate )
              {
                if ( m_NavToolBar->GetToolState( SceneEditorIDs::ID_ToolsNavMeshPunchOutTranslate ))
                {
                  navMeshCreate->SetEditMode(NavMeshCreateTool::EDIT_MODE_CUBE_PUNCH_OUT);
                  navMeshCreate->SetPunchOutMode(NavMeshCreateTool::EDIT_MODE_MOVE);
                  m_Properties->SetSelection(m_ToolPropertyPage);
                }
                else
                {
                  navMeshCreate->SetEditMode(NavMeshCreateTool::EDIT_MODE_CUBE_PUNCH_OUT);
                  navMeshCreate->SetPunchOutMode(NavMeshCreateTool::EDIT_MODE_DISABLED);
                }
              }
              else if ( event.GetId() == SceneEditorIDs::ID_ToolsNavMeshPunchOutRotate )
              {
                if ( m_NavToolBar->GetToolState( SceneEditorIDs::ID_ToolsNavMeshPunchOutRotate ))
                {
                  navMeshCreate->SetEditMode(NavMeshCreateTool::EDIT_MODE_CUBE_PUNCH_OUT);
                  navMeshCreate->SetPunchOutMode(NavMeshCreateTool::EDIT_MODE_ROTATE);
                  m_Properties->SetSelection(m_ToolPropertyPage);
                }
                else
                {
                  navMeshCreate->SetEditMode(NavMeshCreateTool::EDIT_MODE_CUBE_PUNCH_OUT);
                  navMeshCreate->SetPunchOutMode(NavMeshCreateTool::EDIT_MODE_DISABLED);
                }
              }
              else if ( event.GetId() == SceneEditorIDs::ID_ToolsNavMeshPunchOutScale )
              {
                if ( m_NavToolBar->GetToolState( SceneEditorIDs::ID_ToolsNavMeshPunchOutScale ))
                {
                  navMeshCreate->SetEditMode(NavMeshCreateTool::EDIT_MODE_CUBE_PUNCH_OUT);
                  navMeshCreate->SetPunchOutMode(NavMeshCreateTool::EDIT_MODE_SCALE);
                  m_Properties->SetSelection(m_ToolPropertyPage);
                }
                else
                {
                  navMeshCreate->SetEditMode(NavMeshCreateTool::EDIT_MODE_CUBE_PUNCH_OUT);
                  navMeshCreate->SetPunchOutMode(NavMeshCreateTool::EDIT_MODE_DISABLED);
                }
              }
            }
          }
        }
        if ( !success )
        {
          m_NavToolBar->ToggleTool( SceneEditorIDs::ID_ToolsNavMeshPunchOut, false ); 
          m_NavToolBar->ToggleTool( SceneEditorIDs::ID_ToolsNavMeshPunchOutTranslate, false ); 
          m_NavToolBar->ToggleTool( SceneEditorIDs::ID_ToolsNavMeshPunchOutRotate, false ); 
          m_NavToolBar->ToggleTool( SceneEditorIDs::ID_ToolsNavMeshPunchOutScale, false ); 
          std::string errorString = std::string( "Please select a zone with HasNavData attribute" );
          wxMessageBox( errorString.c_str(), "Active zone must have NavMesh data", wxOK|wxCENTRE|wxICON_ERROR, this );
          break;
        }
      }
      break;
    case SceneEditorIDs::ID_ToolsNavMeshRotate:
    case SceneEditorIDs::ID_ToolsNavMeshManipulate:
      {
        Luna::Scene* rootScene = m_SceneManager.GetRootScene();
        bool success = false;
        if (rootScene)
        {
          Luna::ZonePtr& luna_nav_zone_ptr = rootScene->GetNavZone();
          if (luna_nav_zone_ptr)
          {
            Luna::Scene* luna_zone_scene_ptr = m_SceneManager.GetScene( luna_nav_zone_ptr->GetPath() );
            if (luna_zone_scene_ptr && luna_zone_scene_ptr->IsCurrent())
            {
              success = true;
              Luna::NavMeshCreateTool* navMeshCreate;
              if ( m_SceneManager.GetCurrentScene()->GetTool() && m_SceneManager.GetCurrentScene()->GetTool()->GetType() == Reflect::GetType<Luna::NavMeshCreateTool>() )
              {
                navMeshCreate = static_cast<NavMeshCreateTool*>( m_SceneManager.GetCurrentScene()->GetTool().Ptr() );
              }
              else
              {
                navMeshCreate = new Luna::NavMeshCreateTool (m_SceneManager.GetCurrentScene(), m_ToolEnumerator);
                m_SceneManager.GetCurrentScene()->SetTool( navMeshCreate );
              }
              if ( event.GetId() == SceneEditorIDs::ID_ToolsNavMeshRotate )
              {
                if ( m_NavToolBar->GetToolState( SceneEditorIDs::ID_ToolsNavMeshRotate ))
                {
                  navMeshCreate->SetEditMode(NavMeshCreateTool::EDIT_MODE_ROTATE);
                  m_Properties->SetSelection(m_ToolPropertyPage);
                }
                else
                {
                  navMeshCreate->SetEditMode(NavMeshCreateTool::EDIT_MODE_ADD);
                }
              }
              else if ( event.GetId() == SceneEditorIDs::ID_ToolsNavMeshManipulate )
              {
                if ( m_NavToolBar->GetToolState( SceneEditorIDs::ID_ToolsNavMeshManipulate ))
                {
                  navMeshCreate->SetEditMode(NavMeshCreateTool::EDIT_MODE_MOVE);
                  m_Properties->SetSelection(m_ToolPropertyPage);
                }
                else
                {
                  navMeshCreate->SetEditMode(NavMeshCreateTool::EDIT_MODE_ADD);
                }
              }
            }
          }
        }
        if ( !success )
        {
          m_NavToolBar->ToggleTool( SceneEditorIDs::ID_ToolsNavMeshManipulate, false ); 
          m_NavToolBar->ToggleTool( SceneEditorIDs::ID_ToolsNavMeshRotate, false ); 
          std::string errorString = std::string( "Please select a zone with HasNavData attribute" );
          wxMessageBox( errorString.c_str(), "Active zone must have NavMesh data", wxOK|wxCENTRE|wxICON_ERROR, this );
          break;
        }
      }
      break;
    case SceneEditorIDs::ID_ToolsNavMeshVertexSelect:
    case SceneEditorIDs::ID_ToolsNavMeshEdgeSelect:
    case SceneEditorIDs::ID_ToolsNavMeshTriSelect:
      {
        Luna::Scene* rootScene = m_SceneManager.GetRootScene();
        bool success = false;
        if (rootScene)
        {
          Luna::ZonePtr& luna_nav_zone_ptr = rootScene->GetNavZone();
          if (luna_nav_zone_ptr)
          {
            Luna::Scene* luna_zone_scene_ptr = m_SceneManager.GetScene( luna_nav_zone_ptr->GetPath() );
            if (luna_zone_scene_ptr && luna_zone_scene_ptr->IsCurrent())
            {
              success = true;
              Luna::NavMeshCreateTool* navMeshCreate;
              if ( m_SceneManager.GetCurrentScene()->GetTool() && m_SceneManager.GetCurrentScene()->GetTool()->GetType() == Reflect::GetType<Luna::NavMeshCreateTool>() )
              {
                navMeshCreate = static_cast<NavMeshCreateTool*>( m_SceneManager.GetCurrentScene()->GetTool().Ptr() );
              }
              else
              {
                navMeshCreate = new Luna::NavMeshCreateTool (m_SceneManager.GetCurrentScene(), m_ToolEnumerator);
                m_SceneManager.GetCurrentScene()->SetTool( navMeshCreate );
              }
              if ( event.GetId() == SceneEditorIDs::ID_ToolsNavMeshVertexSelect )
              {
                navMeshCreate->SetHoverSelectMode( NavMeshCreateTool::MOUSE_HOVER_SELECT_VERT );
              }
              else if ( event.GetId() == SceneEditorIDs::ID_ToolsNavMeshEdgeSelect )
              {
                navMeshCreate->SetHoverSelectMode( NavMeshCreateTool::MOUSE_HOVER_SELECT_EDGE );
              }
              else if ( event.GetId() == SceneEditorIDs::ID_ToolsNavMeshTriSelect )
              {
                navMeshCreate->SetHoverSelectMode( NavMeshCreateTool::MOUSE_HOVER_SELECT_TRI );
              }
            }
          }
        }
        if ( !success )
        {
          m_NavToolBar->ToggleTool( SceneEditorIDs::ID_ToolsNavMeshManipulate, false ); 
          m_NavToolBar->ToggleTool( SceneEditorIDs::ID_ToolsNavMeshRotate, false ); 
          std::string errorString = std::string( "Please select a zone with HasNavData attribute" );
          wxMessageBox( errorString.c_str(), "Active zone must have NavMesh data", wxOK|wxCENTRE|wxICON_ERROR, this );
          break;
        }
      }
      break;
    }

    m_ToolProperties.GetCanvas()->Clear();

    if (m_SceneManager.GetCurrentScene()->GetTool().ReferencesObject())
    {
      m_SceneManager.GetCurrentScene()->GetTool()->CreateProperties();

      m_ToolProperties.GetCanvas()->Layout();

      m_ToolProperties.GetCanvas()->Read();
    }

    m_View->Refresh();
  }
  else
  {
    GetStatusBar()->SetStatusText( "You must create a new scene or open an existing scene to use a tool" );
  }
}

void SceneEditor::OnUtilitySelected(wxCommandEvent& event)
{
  if (m_SceneManager.HasCurrentScene())
  {
    switch (event.GetId())
    {
    case SceneEditorIDs::ID_UtilitiesConstruction:
      {
        if ( m_SceneManager.GetCurrentScene()->IsEditable() )
        {
          // try to connect if necessary, and if connected try to create the proxy scene.
          if( RemoteConstruct::EstablishConnection() )
          {
            Luna::Scene* scene = GetSceneManager()->GetCurrentScene();
            if (scene == NULL)
            {
              break;
            }

            RemoteConstruct::g_ConstructionTool->BeginScene();

            OS_SelectableDumbPtr::Iterator itr = scene->GetSelection().GetItems().Begin();
            OS_SelectableDumbPtr::Iterator end = scene->GetSelection().GetItems().End();
            S_tuid uniqueUfragArtFiles;

            int ItemToSendCount = (int)scene->GetSelection().GetItems().Size();
            int SentItemCount = 1;

            wxProgressDialog dialog ("Sending content from Luna...", "Initializing", ItemToSendCount, this, wxPD_CAN_ABORT | wxPD_AUTO_HIDE | wxPD_APP_MODAL );
            dialog.Show(true);
            bool abortSend = false;

            for ( ; itr != end; ++itr )
            {
              char displayMessage[100];
              _snprintf( displayMessage, sizeof(displayMessage), "Sending %d of %d", SentItemCount, ItemToSendCount );
              displayMessage[ sizeof(displayMessage) - 1] = 0; 

              abortSend = !(dialog.Update( SentItemCount++, displayMessage ));

              if( abortSend )
              {
                dialog.Show(false);
                break;
              }

              Luna::Entity* entity = Reflect::ObjectCast<Luna::Entity>( *itr );

              if (entity)
              {
                bool createProxy = true;
                if( entity->GetClassSet()->GetEntityAsset()->GetEngineType() == Asset::EngineTypes::Ufrag )
                {
                  Attribute::AttributeViewer< Asset::ArtFileAttribute > model( entity->GetClassSet()->GetEntityAsset() );
                  createProxy = uniqueUfragArtFiles.insert( model->GetFileID() ).second;
                }

                if( createProxy )
                {
                  tuid assetClass = entity->GetClassSet()->GetEntityAssetID();
                  RPC::CreateInstanceParam param;
                  {
                    param.m_ID = entity->GetID();
                    param.m_EntityAsset = assetClass;
                    strncpy(param.m_Name.Characters, entity->GetName().c_str(), RPC_STRING_MAX);
                    param.m_Name.Characters[ RPC_STRING_MAX-1 ] = 0; 
                    memcpy(&param.m_Transform, &entity->GetGlobalTransform(), sizeof(Math::Matrix4));
                  }
                  RemoteConstruct::g_ConstructionTool->CreateProxy(&param);
                }
              }
            }
            RemoteConstruct::g_ConstructionTool->EndScene();
          }
        }
      }
      break;

    case SceneEditorIDs::ID_UtilitiesMeasureDistance:
      {
        m_SceneManager.GetCurrentScene()->MeasureDistance();
      }
      break;

    case SceneEditorIDs::ID_UtilitiesFlushSymbols:
      {
        Symbol::SymbolBuilder::GetInstance()->Reset();

        m_SceneManager.GetCurrentScene()->GetSelection().Refresh();
      }
      break;

    case SceneEditorIDs::ID_ToolsNavMeshImport:
      {
        m_NavToolBar->ToggleTool( SceneEditorIDs::ID_ToolsNavMeshImport, false );   // this isn't a toggle
        Luna::Scene* rootScene = m_SceneManager.GetRootScene();
        UniqueID::TUID tuid;
        UniqueID::TUID::Generate(tuid);
        Content::MeshPtr content_hi_res_nav = new Content::Mesh(tuid);
        content_hi_res_nav->m_ExportTypes[ Content::ContentTypes::NavMeshHiRes ] = true;
        content_hi_res_nav->m_GivenName = "HiResNavMesh";
        content_hi_res_nav->m_MeshOriginType = Content::Mesh::NavHiRes;
        content_hi_res_nav->m_UseGivenName = true;
        content_hi_res_nav->m_ExportTypeIndex.insert( Content::M_ContentTypeToIndex::value_type(  Content::ContentTypes::NavMeshHiRes , 0) );
        UniqueID::TUID::Generate(tuid);
        Content::MeshPtr content_low_res_nav = new Content::Mesh(tuid);
        content_low_res_nav->m_ExportTypes[ Content::ContentTypes::NavMeshLowRes ] = true;
        content_low_res_nav->m_GivenName = "LowResNavMesh";
        content_low_res_nav->m_MeshOriginType = Content::Mesh::NavLowRes;
        content_low_res_nav->m_UseGivenName = true;
        content_low_res_nav->m_ExportTypeIndex.insert( Content::M_ContentTypeToIndex::value_type(  Content::ContentTypes::NavMeshLowRes , 0) );
        if (rootScene)
        {
          const S_ZoneDumbPtr& zones = rootScene->GetZones(); 
          Luna::ZonePtr& luna_nav_zone_ptr = rootScene->GetNavZone();
          if (!luna_nav_zone_ptr)
          {
            //throw a dialog  
            std::string errorString = std::string( "Please make a new zone and check HasNavData attribute" );
            wxMessageBox( errorString.c_str(), "You must have a zone with NavMesh data", wxOK|wxCENTRE|wxICON_ERROR, this );
            break;
          }
          S_ZoneDumbPtr::const_iterator zoneItr = zones.begin();
          S_ZoneDumbPtr::const_iterator zoneEnd = zones.end();
          for (; zoneItr!=zoneEnd; ++zoneItr)
          {
            Zone* zone = *zoneItr;
            Luna::Scene* zone_scene = m_SceneManager.GetScene( zone->GetPath() );
            if ( zone_scene )
            { 
              // Export the rest of the dependency nodes
              HM_SceneNodeDumbPtr scene_nodes = zone_scene->GetNodes();
              HM_SceneNodeDumbPtr::const_iterator itr = scene_nodes.begin();
              for ( ; itr != scene_nodes.end(); ++itr )
              {
                const Luna::Entity* entity = Reflect::ConstObjectCast<Luna::Entity> (itr->second);
                if (entity)
                {
                  const Luna::Scene* nav_scene= entity->GetNestedScene( GeometryModes::Pathfinding, false);
                  if (nav_scene)
                  {
                    HM_SceneNodeDumbPtr nav_scene_nodes = nav_scene->GetNodes();
                    HM_SceneNodeDumbPtr::const_iterator nav_scene_nodes_itr = nav_scene_nodes.begin();
                    for ( ; nav_scene_nodes_itr != nav_scene_nodes.end(); ++nav_scene_nodes_itr )
                    {
                      const Luna::Mesh* luna_mesh = Reflect::ConstObjectCast<Luna::Mesh>( nav_scene_nodes_itr->second );
                      if (luna_mesh)
                      {
                        const Content::Mesh* content_mesh = luna_mesh->GetPackage< Content::Mesh >();
                        if (content_mesh)
                        {
                          if (content_mesh->GetExportTypeIndex(Content::ContentTypes::Pathfinding) > -1)
                          {
                            content_hi_res_nav->CopyLegacyMeshDataForNav(content_mesh, 0.001f);
                          }
                          else
                          {
                            NOC_ASSERT(content_mesh->GetExportTypeIndex(Content::ContentTypes::LowResPathfinding) > -1);
                            content_low_res_nav->CopyLegacyMeshDataForNav(content_mesh, 0.001f);
                          }//f (content_mesh->m_ExportTypeIndex[ Content::ContentTypes::HighResCollision ])
                        }//if (content_mesh)
                      }//if (luna_mesh)
                    }//for ( ; nav_scene_nodes_itr != nav_scene_nodes_end; ++nav_scene_nodes_itr )
                  }//if (nav_scene)
                }//if (entity)
              }//for ( ; itr != end; ++itr )
            }//if ( scene )
          }//  for (; zoneItr!=zoneEnd; ++zoneItr)

          //now add content_hi_res_nav & content_low_res_nav to luna_nav_zone_ptr
          {
            Luna::Scene* luna_zone_scene_ptr = m_SceneManager.GetScene( luna_nav_zone_ptr->GetPath() );
            if (luna_zone_scene_ptr)
            {
              //first removed the existing meshes
              HM_SceneNodeDumbPtr::const_iterator itr = luna_zone_scene_ptr->GetNodes().begin();
              while (itr != luna_zone_scene_ptr->GetNodes().end())
              {
                for ( ; itr != luna_zone_scene_ptr->GetNodes().end(); ++itr )
                {
                  Luna::Mesh* luna_mesh = Reflect::ObjectCast<Luna::Mesh> (itr->second);
                  if (luna_mesh)
                  {
                    const Content::Mesh* content_mesh = luna_mesh->GetPackage< Content::Mesh >();
                    if (content_mesh)
                    {
                      if  ((content_mesh->m_MeshOriginType == Content::Mesh::NavHiRes) ||
                        (content_mesh->m_MeshOriginType == Content::Mesh::NavLowRes ) )
                      {
                        luna_zone_scene_ptr->RemoveObject(itr->second);
                        itr = luna_zone_scene_ptr->GetNodes().begin();
                        break;
                      }//if  ((content_mesh->GetExportTypeIndex(Content::ContentTypes::NavMeshHiRes)
                    }//if (content_mesh)
                  }// if (entity)
                }//for ( ; itr != luna_zone_scene_ptr->GetNodes().end(); ++itr )
              }//while (itr != luna_zone_scene_ptr->GetNodes().end())

              //now we made sure we deleted old ones. now add the newly made mesh nodes
              ImportAction action = ImportActions::Load;
              u32 import_flags = 0;
              Undo::CommandPtr command;
             
              if (luna_zone_scene_ptr->IsCurrent() && luna_zone_scene_ptr->IsEditable())
              {
                Reflect::V_Element elems;
                if (content_hi_res_nav->m_TriangleVertexIndices.size())
                {
                  elems.push_back(content_hi_res_nav);
                }
                if (content_low_res_nav->m_TriangleVertexIndices.size())
                {
                  elems.push_back(content_low_res_nav);
                }
                if (elems.size())
                {
                command = luna_zone_scene_ptr->ImportSceneNodes( elems, action, import_flags );
              }
              }
              else
              {
                //throw a dialog  
                std::string errorString = std::string( "Trying to change nav mesh zone when it is not selected and checked out" );
                wxMessageBox( errorString.c_str(), "Please make nav zone the current editing zone and say yes to check it out", wxOK|wxCENTRE|wxICON_ERROR, this );
                break;
              }
            }
            else
            {
              int a = 1;
              //assert here
            }//if (luna_zone_scene_ptr)
          }////now add content_hi_res_nav & content_low_res_nav to luna_nav_zone_ptr
        }//if (rootScene)
      }
      break;
    }
  }
  else
  {
    GetStatusBar()->SetStatusText( "You must create a new scene or open an existing scene to use a tool" );
  }
}

void SceneEditor::OnParent(wxCommandEvent& event)
{
  if ( m_SceneManager.HasCurrentScene() )
  {
    m_SceneManager.GetCurrentScene()->Push( m_SceneManager.GetCurrentScene()->ParentSelected() );
  }
}

void SceneEditor::OnUnparent(wxCommandEvent& event)
{
  if ( m_SceneManager.HasCurrentScene() )
  {
    m_SceneManager.GetCurrentScene()->Push( m_SceneManager.GetCurrentScene()->UnparentSelected() );
  }
}

void SceneEditor::OnGroup(wxCommandEvent& event)
{
  if ( m_SceneManager.HasCurrentScene() )
  {
    m_SceneManager.GetCurrentScene()->Push( m_SceneManager.GetCurrentScene()->GroupSelected() );
  }
}

void SceneEditor::OnUngroup(wxCommandEvent& event)
{
  if ( m_SceneManager.HasCurrentScene() )
  {
    m_SceneManager.GetCurrentScene()->Push( m_SceneManager.GetCurrentScene()->UngroupSelected() );
  }
}

void SceneEditor::OnCenter(wxCommandEvent& event)
{
  if ( m_SceneManager.HasCurrentScene() )
  {
    m_SceneManager.GetCurrentScene()->Push( m_SceneManager.GetCurrentScene()->CenterSelected() );
  }
}

static void RecurseToggleSelection( Luna::HierarchyNode* node, const OS_SelectableDumbPtr& oldSelection, OS_SelectableDumbPtr& newSelection )
{
  for each (Luna::HierarchyNode* child in node->GetChildren())
  {
    RecurseToggleSelection( child, oldSelection, newSelection );
  }

  bool found = false;
  OS_SelectableDumbPtr::Iterator selItr = oldSelection.Begin();
  OS_SelectableDumbPtr::Iterator selEnd = oldSelection.End();
  for ( ; selItr != selEnd && !found; ++selItr )
  {
    Luna::HierarchyNode* current = Reflect::ObjectCast< Luna::HierarchyNode >( *selItr );
    if ( current )
    {
      if ( current == node )
      {
        found = true; // breaks out of the loop
      }
    }
  }

  if ( !found )
  {
    newSelection.Append( node );
  }
}

void SceneEditor::OnInvertSelection(wxCommandEvent& event)
{
  if ( m_SceneManager.HasCurrentScene() )
  {
    const OS_SelectableDumbPtr& selection = m_SceneManager.GetCurrentScene()->GetSelection().GetItems();
    if ( selection.Size() > 0 )
    {
      OS_SelectableDumbPtr newSelection;
      RecurseToggleSelection( m_SceneManager.GetCurrentScene()->GetRoot(), selection, newSelection );
      m_SceneManager.GetCurrentScene()->Push( m_SceneManager.GetCurrentScene()->GetSelection().SetItems( newSelection ) );
    }
  }
}

void SceneEditor::OnSelectAll(wxCommandEvent& event)
{
  if ( m_SceneManager.HasCurrentScene() )
  {
    OS_SelectableDumbPtr selection;

    HM_SceneNodeDumbPtr::const_iterator itr = m_SceneManager.GetCurrentScene()->GetNodes().begin();
    HM_SceneNodeDumbPtr::const_iterator end = m_SceneManager.GetCurrentScene()->GetNodes().end();
    for ( ; itr != end; ++itr )
    {
      Luna::SceneNode* sceneNode = itr->second;
      if ( sceneNode->HasType( Reflect::GetType<Luna::HierarchyNode>() ) )
      {
        selection.Append( sceneNode );
      }
    }

    m_SceneManager.GetCurrentScene()->Push( m_SceneManager.GetCurrentScene()->GetSelection().SetItems( selection ) );
  }
}

void SceneEditor::OnDuplicate(wxCommandEvent& event)
{
  if ( m_SceneManager.HasCurrentScene() )
  {
    m_SceneManager.GetCurrentScene()->Push( m_SceneManager.GetCurrentScene()->DuplicateSelected() );
  }
}

void SceneEditor::OnSmartDuplicate(wxCommandEvent& event)
{
  if ( m_SceneManager.HasCurrentScene() )
  {
    m_SceneManager.GetCurrentScene()->Push( m_SceneManager.GetCurrentScene()->SmartDuplicateSelected() );
  }
}

void SceneEditor::OnCopyTransform(wxCommandEvent& event)
{
  if ( m_SceneManager.HasCurrentScene() )
  {
    Math::V_Matrix4 transforms;
    m_SceneManager.GetCurrentScene()->GetSelectedTransforms(transforms);

    Nocturnal::SmartPtr<Reflect::Matrix4ArraySerializer> data = new Reflect::Matrix4ArraySerializer();
    data->m_Data.Set( transforms );

    std::string xml;
    data->ToXML( xml );

    std::string error;
    if ( !Windows::CopyToClipboard( GetHwnd(), xml, error ) )
    {
      Console::Error( "%s\n", error.c_str() );
    }
  }
}

void SceneEditor::OnPasteTransform(wxCommandEvent& event)
{
  if ( m_SceneManager.HasCurrentScene() )
  {
    std::string xml, error;
    if ( !Windows::RetrieveFromClipboard( GetHwnd(), xml, error ) )
    {
      Console::Error( "%s\n", error.c_str() );
    }

    Reflect::V_Element elements;
    Reflect::Archive::FromXML( xml, elements );

    Reflect::V_Element::const_iterator itr = elements.begin();
    Reflect::V_Element::const_iterator end = elements.end();
    for ( ; itr != end; ++itr )
    {
      Nocturnal::SmartPtr<Reflect::Matrix4ArraySerializer> data = Reflect::ObjectCast< Reflect::Matrix4ArraySerializer >( *itr );
      if ( data.ReferencesObject() )
      {
        m_SceneManager.GetCurrentScene()->Push( m_SceneManager.GetCurrentScene()->SetSelectedTransforms(data->m_Data.Get()) );
        break;
      }
    }
  }
}

void SceneEditor::OnSnapToCamera(wxCommandEvent& event)
{
  if ( m_SceneManager.HasCurrentScene() )
  {
    m_SceneManager.GetCurrentScene()->Push( m_SceneManager.GetCurrentScene()->SnapSelectedToCamera() );
  }
}

void SceneEditor::OnSnapCameraTo(wxCommandEvent& event)
{
  if ( m_SceneManager.HasCurrentScene() )
  {
    m_SceneManager.GetCurrentScene()->Push( m_SceneManager.GetCurrentScene()->SnapCameraToSelected() );
  }
}

///////////////////////////////////////////////////////////////////////////////
//Pushes the selected menu item as the current selected item.  
//

void SceneEditor::OnManifestContextMenu(wxCommandEvent& event)
{
  if( !m_OrderedContextItems.empty() )
  { 
    u32 selectionIndex = event.GetId() - SceneEditorIDs::ID_SelectContextMenu;

    Luna::HierarchyNode* selection = m_OrderedContextItems[ selectionIndex ];
    
    if( selection )
    {
      OS_SelectableDumbPtr newSelection;
      newSelection.Append( selection );
      m_SceneManager.GetCurrentScene()->Push( m_SceneManager.GetCurrentScene()->GetSelection().SetItems( newSelection ) );
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
//Opens the context menu with items separated by type
//
void SceneEditor::OnTypeContextMenu(wxCommandEvent &event)
{
  ContextCallbackData* data = static_cast<ContextCallbackData*>(event.m_callbackUserData);
  OS_SelectableDumbPtr newSelection;

  switch( data->m_ContextCallbackType )
  {
  case ContextCallbackTypes::All:
    {
      const HM_SceneNodeSmartPtr& instances( data->m_NodeType->GetInstances() );
      
      if( !instances.empty() )
      {    
        HM_SceneNodeSmartPtr::const_iterator itr = instances.begin();
        HM_SceneNodeSmartPtr::const_iterator end = instances.end();
        for( ; itr != end; ++itr )
        {
          newSelection.Append( itr->second );
        }              
      }
      break;
    }

  case ContextCallbackTypes::Item:
    {
      newSelection.Append( static_cast<Luna::HierarchyNode*>( data->m_NodeInstance ) );
      break;
    }

  case ContextCallbackTypes::Instance:
    {
      const S_InstanceDumbPtr& instances( data->m_InstanceSet->GetInstances() );

      S_InstanceDumbPtr::const_iterator itr = instances.begin();
      S_InstanceDumbPtr::const_iterator end = instances.end();

      for( ; itr != end; ++itr )
      {
        newSelection.Append( *itr );
      }

      break;
    }

  case ContextCallbackTypes::Entity_Visible_Geometry:
    {
      const HM_SceneNodeSmartPtr& instances( data->m_NodeType->GetInstances() );

      HM_SceneNodeSmartPtr::const_iterator itr = instances.begin();
      HM_SceneNodeSmartPtr::const_iterator end = instances.end();

      for( ; itr != end; ++itr )
      {
        const Luna::SceneNode* node (itr->second);
        const Luna::Entity* entity = Reflect::ConstObjectCast<Luna::Entity> (node);

        if ( entity && entity->IsGeometryVisible())
        {
          newSelection.Append( itr->second );
        }
      }

      break;
    }

  case ContextCallbackTypes::Entity_Invisible_Geometry:
    {
      const HM_SceneNodeSmartPtr& instances( data->m_NodeType->GetInstances() );

      HM_SceneNodeSmartPtr::const_iterator itr = instances.begin();
      HM_SceneNodeSmartPtr::const_iterator end = instances.end();

      for( ; itr != end; ++itr )
      {
        const Luna::SceneNode* node (itr->second);
        const Luna::Entity* entity = Reflect::ConstObjectCast<Luna::Entity> (node);

        if ( entity && !entity->IsGeometryVisible())
        {
          newSelection.Append( itr->second );
        }
      }

      break;
    }


  default:
    {
      break;
    }
  }
 
  if( !newSelection.Empty() )
  {
    m_SceneManager.GetCurrentScene()->Push( m_SceneManager.GetCurrentScene()->GetSelection().SetItems( newSelection ) );
  }
}

///////////////////////////////////////////////////////////////////////////////
// Upon selection from the context menu, performs a select operation
void SceneEditor::SelectItemInScene( wxCommandEvent& event )
{
  GeneralCallbackData* data = static_cast<GeneralCallbackData*>( event.m_callbackUserData );

  SelectArgs* args = static_cast<SelectArgs*>( data->m_GeneralData );

  args->m_Mode = SelectionModes::Replace;
  args->m_Target = SelectionTargetModes::Single;
  m_SceneManager.GetCurrentScene()->Select(*args);

  m_Properties->SetSelection( m_SelectionPropertyPage );
}

///////////////////////////////////////////////////////////////////////////////
// Selects all the items similar to what is highlighted
void SceneEditor::SelectSimilarItemsInScene( wxCommandEvent& event )
{
  m_SceneManager.GetCurrentScene()->Push( m_SceneManager.GetCurrentScene()->SelectSimilar() );

  m_Properties->SetSelection( m_SelectionPropertyPage );
}

///////////////////////////////////////////////////////////////////////////////
// Overridden from base class.  Called when attempting to open a file from the
// MRU list.  Closes all currently open files before trying to open the new one.
// 
void SceneEditor::OnMRUOpen( const UIToolKit::MRUArgs& args )
{
  DoOpen( args.m_Item );
}

///////////////////////////////////////////////////////////////////////////////
// Save current window settings to the preferences.
// 
void SceneEditor::SaveWindowState()
{
  SceneEditorPreferences()->GetSceneEditorWindowSettings()->SetFromWindow( this, &m_FrameManager );
}

///////////////////////////////////////////////////////////////////////////////
// Loads the session.  Overridden to separate the world file out from the zones.
// 
bool SceneEditor::LoadSession( const EditorStatePtr& state )
{
  std::string errorList;

  if ( !state->m_OpenFiles.empty() )
  {
    tuid worldID = state->m_OpenFiles.front();
    if ( m_SceneManager.OpenFileID( worldID, errorList ) != NULL )
    {
      std::string error;
      V_tuid::iterator itr = state->m_OpenFiles.begin();
      V_tuid::iterator end = state->m_OpenFiles.end();
      itr++; // skip world file
      for ( ; itr != end; ++itr )
      {
        error.clear();
        if ( m_SceneManager.OpenZone( File::GlobalManager().GetPath( *itr ), error ) == NULL )
        {
          if ( !errorList.empty() )
          {
            errorList += "\n";
          }
          errorList += error;
        }
      }
    }
  }

  if ( !errorList.empty() )
  {
    wxMessageBox( errorList.c_str(), "Error", wxCENTER | wxICON_ERROR | wxOK, this );
    return false;
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// Copies the currently selected items from the specified scene into the
// clipboard.
// 
bool SceneEditor::Copy( Luna::Scene* scene )
{
  LUNA_SCENE_SCOPE_TIMER( ("") );
  bool isOk = true;
  NOC_ASSERT( scene );

  if ( scene->GetSelection().GetItems().Size() > 0 )
  {
    std::string xml;
    if ( !scene->ExportXML( xml, ExportFlags::Default | ExportFlags::SelectedNodes ) )
    {
      Console::Error( "There was an error while generating XML data from the selection.\n" );
      isOk = false;
    }
    else
    {
      std::string error;
      isOk = Windows::CopyToClipboard( GetHwnd(), xml, error );
      if ( !isOk )
      {
        Console::Error( "%s\n", error.c_str() );
      }
    }
  }

  return isOk;
}

///////////////////////////////////////////////////////////////////////////////
// Fetches data from the clipboard (if there is any) and inserts it into the
// specified scene.
// 
bool SceneEditor::Paste( Luna::Scene* scene )
{
  LUNA_SCENE_SCOPE_TIMER( ("") );
  NOC_ASSERT( scene );

  bool isOk = false;
  std::string xml;

  // Get data from the clipboard
  std::string unused;
  Windows::RetrieveFromClipboard( GetHwnd(), xml, unused );

  // Import data into the scene
  if ( !xml.empty() )
  {
    // Create a batch to add the objects to the scene
    Undo::BatchCommandPtr batch = new Undo::BatchCommand ();

    // Import the data as children of the paste root
    batch->Push( scene->ImportXML( xml, ImportFlags::Select ) );

    scene->Push( batch );
    scene->Execute(false);
  }

  return isOk;
}

void SceneEditor::Render( RenderVisitor* render )
{
  //
  // Top level draw routine
  //

  Luna::Scene* rootScene = m_SceneManager.GetRootScene();

  if (rootScene)
  {
    rootScene->Render( render );
  }
}

void SceneEditor::Select(const SelectArgs& args)
{
  if (m_SceneManager.HasCurrentScene())
  {
    //if we are using manifest select
    switch( args.m_Mode )
    {
    case SelectionModes::Type:
      {
        OpenTypeContextMenu( args );
        break;
      }

    case SelectionModes::Manifest:
      {
        OpenManifestContextMenu( args );      
        break;
      }

    default:
      {
        // do select
        m_SceneManager.GetCurrentScene()->Select(args);
        break;
      }
    }
  }
}

void SceneEditor::SetHighlight( const SetHighlightArgs& args )
{
  if( m_SceneManager.HasCurrentScene() )
  {
    m_SceneManager.GetCurrentScene()->SetHighlight(args);    
  }
}

void SceneEditor::ClearHighlight( const ClearHighlightArgs& args )
{
  if( m_SceneManager.HasCurrentScene() )
  {
    m_SceneManager.GetCurrentScene()->ClearHighlight(args);    
  }
}

bool SceneEditor::ValidateDrag( const Inspect::DragArgs& args )
{
  bool canHandleArgs = false;

  Inspect::ClipboardFileListPtr fileList = Reflect::ObjectCast< Inspect::ClipboardFileList >( args.m_ClipboardData->FromBuffer() );
  if ( fileList )
  {
    for ( S_string::const_iterator fileItr = fileList->GetFilePaths().begin(), fileEnd = fileList->GetFilePaths().end();
      fileItr != fileEnd && !canHandleArgs;
      ++fileItr )
    {
      const std::string& path = *fileItr;

      if ( !path.empty() && FileSystem::Exists( path ) )
      {
        if ( FileSystem::HasExtension( path, FinderSpecs::Asset::LEVEL_DECORATION.GetDecoration() ) )
        {
          canHandleArgs = true;
        }
        else if ( FileSystem::HasExtension( path, FinderSpecs::Asset::ZONE_DECORATION.GetDecoration() ) )
        {
          canHandleArgs = true;
        }
        else if ( FileSystem::HasExtension( path, FinderSpecs::Asset::ENTITY_DECORATION.GetDecoration() ) )
        {
          canHandleArgs = true;
        }
      }
    }
  }

  return canHandleArgs;
}

wxDragResult SceneEditor::DragOver( const Inspect::DragArgs& args )
{
  wxDragResult result = args.m_Default;

  if ( !ValidateDrag( args ) )
  {
    result = wxDragNone;
  }

  return result;
}

wxDragResult SceneEditor::Drop( const Inspect::DragArgs& args )
{
  wxDragResult result = args.m_Default;

  V_string levels, zones, entities;

  if ( ValidateDrag( args ) )
  {
    Inspect::ClipboardFileListPtr fileList = Reflect::ObjectCast< Inspect::ClipboardFileList >( args.m_ClipboardData->FromBuffer() );
    if ( fileList )
    {
      for ( S_string::const_iterator fileItr = fileList->GetFilePaths().begin(),
        fileEnd = fileList->GetFilePaths().end(); fileItr != fileEnd; ++fileItr )
      {
        const std::string& path = *fileItr;
        if ( !path.empty() && FileSystem::Exists( path ) )
        {
          if ( FileSystem::HasExtension( path, FinderSpecs::Asset::LEVEL_DECORATION.GetDecoration() ) )
          {
            levels.push_back( path );
          }
          else if ( FileSystem::HasExtension( path, FinderSpecs::Asset::ZONE_DECORATION.GetDecoration() ) )
          {
            zones.push_back( path );
          }
          else if ( FileSystem::HasExtension( path, FinderSpecs::Asset::ENTITY_DECORATION.GetDecoration() ) )
          {
            entities.push_back( path );
          }
        }
      }
    }

    if ( levels.size() == 1 )
    {
      DoOpen( levels.front() );
    }
    else if ( zones.size() && m_SceneManager.GetCurrentLevel() )
    {
      Luna::Scene* rootScene = m_SceneManager.GetRootScene();
      if ( rootScene && rootScene->IsEditable() )
      {
        for ( V_string::const_iterator itr = zones.begin(), end = zones.end(); itr != end; ++itr )
        {
          tuid zoneID = File::GlobalManager().GetID( *itr );
          if ( zoneID )
          {
            bool containsZone = false;
            for ( S_ZoneDumbPtr::const_iterator itr = rootScene->GetZones().begin(), end = rootScene->GetZones().end(); itr != end && !containsZone; ++itr )
            {
              containsZone = (*itr)->GetFileID() == zoneID;
            }

            if ( !containsZone )
            {
              Content::ZonePtr contentZone = new Content::Zone ();
              contentZone->m_FileID = zoneID;

              Luna::ZonePtr sceneZone = new Zone( m_SceneManager.GetRootScene(), contentZone );
              rootScene->AddObject( sceneZone );

              sceneZone->Initialize();
            }
          }
        }
      }
    }
    else if ( entities.size() )
    {
      if ( !m_SceneManager.GetCurrentScene() )
      {
        return result;
      }

      // Fake a command to change the tool
      wxCommandEvent toolChangeEvt( wxEVT_COMMAND_MENU_SELECTED, SceneEditorIDs::ID_ToolsEntityCreate );
      OnToolSelected( toolChangeEvt );

      Luna::EntityCreateTool* tool = Reflect::ObjectCast< Luna::EntityCreateTool >( m_SceneManager.GetCurrentScene()->GetTool() );
      if ( !tool )
      {
        return result;
      }
      
      tool->DropEntities( entities, wxIsShiftDown() );
    }
  }

  return result;
}

void SceneEditor::SceneAdded( const SceneChangeArgs& args )
{
  if ( !m_SceneManager.IsNestedScene( args.m_Scene ) )
  {
    // Only listen to zone and world files.
    args.m_Scene->AddTitleChangedListener( TitleChangeSignature::Delegate( this, &SceneEditor::TitleChanged ) );
    args.m_Scene->AddStatusChangedListener( StatusChangeSignature::Delegate( this, &SceneEditor::StatusChanged ) );
    args.m_Scene->AddCursorChangedListener( CursorChangeSignature::Delegate( this, &SceneEditor::CursorChanged ) );
    args.m_Scene->AddBusyCursorChangedListener( CursorChangeSignature::Delegate( this, &SceneEditor::BusyCursorChanged ) );
    args.m_Scene->AddLoadFinishedListener( LoadSignature::Delegate( this, & SceneEditor::SceneLoadFinished ) );

    m_SelectionEnumerator->AddPopulateLinkListener( Inspect::PopulateLinkSignature::Delegate (args.m_Scene, &Luna::Scene::PopulateLink));

    Document* document = args.m_Scene->GetSceneDocument();
    document->AddDocumentModifiedListener( DocumentChangedSignature::Delegate( this, &SceneEditor::DocumentModified ) );
    document->AddDocumentSavedListener( DocumentChangedSignature::Delegate( this, &SceneEditor::DocumentModified ) );
    document->AddDocumentClosedListener( DocumentChangedSignature::Delegate( this, &SceneEditor::DocumentModified ) );
  }
}

void SceneEditor::SceneRemoving( const SceneChangeArgs& args )
{
  args.m_Scene->RemoveTitleChangedListener( TitleChangeSignature::Delegate ( this, &SceneEditor::TitleChanged ) );
  args.m_Scene->RemoveStatusChangedListener( StatusChangeSignature::Delegate ( this, &SceneEditor::StatusChanged ) );
  args.m_Scene->RemoveCursorChangedListener( CursorChangeSignature::Delegate ( this, &SceneEditor::CursorChanged ) );
  args.m_Scene->RemoveBusyCursorChangedListener( CursorChangeSignature::Delegate ( this, &SceneEditor::BusyCursorChanged ) );
  args.m_Scene->RemoveLoadFinishedListener( LoadSignature::Delegate( this, & SceneEditor::SceneLoadFinished ) );

  m_SelectionEnumerator->RemovePopulateLinkListener( Inspect::PopulateLinkSignature::Delegate (args.m_Scene, &Luna::Scene::PopulateLink));

  m_View->Refresh();

  if ( m_SceneManager.IsRoot( args.m_Scene ) )
  {
    m_OutlinerStates.clear();
  }
  else
  {
    m_OutlinerStates.erase( args.m_Scene );
  }
}

void SceneEditor::SceneLoadFinished( const LoadArgs& args )
{
  m_View->Refresh();
  DocumentModified( DocumentChangedArgs( args.m_Scene->GetSceneDocument() ) );
}

void SceneEditor::TitleChanged( const TitleChangeArgs& args )
{
  SetLabel( args.m_Title.c_str() );
}

void SceneEditor::StatusChanged( const StatusChangeArgs& args )
{
  GetStatusBar()->SetStatusText( args.m_Status.c_str() );
}

void SceneEditor::CursorChanged( const CursorChangeArgs& args )
{
  wxSetCursor( args.m_Cursor );
}

void SceneEditor::BusyCursorChanged( const CursorChangeArgs& args )
{
  if (args.m_Cursor == wxCURSOR_ARROW)
  {
    wxEndBusyCursor();
  }
  else
  {
    static wxCursor busyCursor;
    busyCursor = wxCursor (args.m_Cursor);
    wxBeginBusyCursor(&busyCursor);
  }
}

void SceneEditor::Executed( const ExecuteArgs& args )
{
  if (!m_SelectionPropertiesManager->ThreadsActive() && !args.m_Interactively)
  {
    m_SelectionProperties.Read();
  }
}

void SceneEditor::SelectionChanged( const OS_SelectableDumbPtr& selection )
{
  m_SelectionPropertiesManager->SetSelection( selection );
}

void SceneEditor::CurrentSceneChanging( const SceneChangeArgs& args )
{
  if ( !args.m_Scene )
  {
    return;
  }

  // Unhook our event handlers
  args.m_Scene->RemoveTitleChangedListener( TitleChangeSignature::Delegate ( this, &SceneEditor::TitleChanged ) );
  args.m_Scene->RemoveStatusChangedListener( StatusChangeSignature::Delegate ( this, &SceneEditor::StatusChanged ) );
  args.m_Scene->RemoveCursorChangedListener( CursorChangeSignature::Delegate ( this, &SceneEditor::CursorChanged ) );
  args.m_Scene->RemoveBusyCursorChangedListener( CursorChangeSignature::Delegate ( this, &SceneEditor::BusyCursorChanged ) );
  args.m_Scene->RemoveExecutedListener( ExecuteSignature::Delegate ( this, &SceneEditor::Executed ) );

  // Selection event handlers
  args.m_Scene->RemoveSelectionChangedListener( SelectionChangedSignature::Delegate ( this, &SceneEditor::SelectionChanged ) );

  // Remove attribute listeners
  m_SelectionEnumerator->RemovePropertyChangingListener( Inspect::ChangingSignature::Delegate (args.m_Scene, &Luna::Scene::PropertyChanging));
  m_SelectionEnumerator->RemovePropertyChangedListener( Inspect::ChangedSignature::Delegate (args.m_Scene, &Luna::Scene::PropertyChanged));
  m_SelectionEnumerator->RemovePickLinkListener( Inspect::PickLinkSignature::Delegate (args.m_Scene, &Luna::Scene::PickLink));
  m_SelectionEnumerator->RemoveSelectLinkListener( Inspect::SelectLinkSignature::Delegate (args.m_Scene, &Luna::Scene::SelectLink));

  // If we were editing a scene, save the outliner info before changing to the new one.
  OutlinerStates* stateInfo = &m_OutlinerStates.insert( M_OutlinerStates::value_type( args.m_Scene, OutlinerStates() ) ).first->second;
#ifndef LUNA_SCENE_DISABLE_OUTLINERS
  m_HierarchyOutline->SaveState( stateInfo->m_Hierarchy );
  m_TypeOutline->SaveState( stateInfo->m_Types );
  m_EntityAssetOutline->SaveState( stateInfo->m_EntityAssetes );
  m_RuntimeClassOutline->SaveState( stateInfo->m_RuntimeClasses );
#endif

  // Clear the selection attribute canvas
  m_SelectionProperties.Clear();

  // Clear the tool attribute canavs
  m_ToolProperties.Clear();

  // Release the tool from the VIEW and Scene, saving the tool in the scene isn't a desirable behavior and the way it is currently
  // implimented it will cause a crash under certain scenarios (see trac #1322)
  args.m_Scene->SetTool( NULL );
  m_View->SetTool(NULL);
  m_ToolsToolBar->Disable();
  m_NavToolBar->Disable();
}

void SceneEditor::BeginLayersGridBatching()
{
  V_LayerGrid::const_iterator gridLayerItr = m_LayerGrids.begin();
  V_LayerGrid::const_iterator gridLayerEnd = m_LayerGrids.end();

  for ( ; gridLayerItr != gridLayerEnd; ++gridLayerItr )
  {
    (*gridLayerItr)->BeginBatch();
  }
}

void SceneEditor::EndLayersGridBatching()
{
  V_LayerGrid::const_iterator gridLayerItr = m_LayerGrids.begin();
  V_LayerGrid::const_iterator gridLayerEnd = m_LayerGrids.end();

  for ( ; gridLayerItr != gridLayerEnd; ++gridLayerItr )
  {
    (*gridLayerItr)->EndBatch();
  }
}

void SceneEditor::CurrentSceneChanged( const SceneChangeArgs& args )
{
  if ( args.m_Scene )
  {
    m_ToolsToolBar->Enable();
    m_NavToolBar->Enable();

    // Hook our event handlers
    args.m_Scene->AddTitleChangedListener( TitleChangeSignature::Delegate ( this, &SceneEditor::TitleChanged ) );
    args.m_Scene->AddStatusChangedListener( StatusChangeSignature::Delegate ( this, &SceneEditor::StatusChanged ) );
    args.m_Scene->AddCursorChangedListener( CursorChangeSignature::Delegate ( this, &SceneEditor::CursorChanged ) );
    args.m_Scene->AddBusyCursorChangedListener( CursorChangeSignature::Delegate ( this, &SceneEditor::BusyCursorChanged ) );
    args.m_Scene->AddExecutedListener( ExecuteSignature::Delegate ( this, &SceneEditor::Executed ) );

    // Selection event handlers
    args.m_Scene->AddSelectionChangedListener( SelectionChangedSignature::Delegate ( this, &SceneEditor::SelectionChanged ) );

    // These events are emitted from the attribute editor and cause execution of the scene to occur, and interactive goodness
    m_SelectionEnumerator->AddPropertyChangingListener( Inspect::ChangingSignature::Delegate (args.m_Scene, &Luna::Scene::PropertyChanging));
    m_SelectionEnumerator->AddPropertyChangedListener( Inspect::ChangedSignature::Delegate (args.m_Scene, &Luna::Scene::PropertyChanged));
    m_SelectionEnumerator->AddPickLinkListener( Inspect::PickLinkSignature::Delegate (args.m_Scene, &Luna::Scene::PickLink));
    m_SelectionEnumerator->AddSelectLinkListener( Inspect::SelectLinkSignature::Delegate (args.m_Scene, &Luna::Scene::SelectLink));

    // Restore the tree control with the information for the new editing scene
    M_OutlinerStates::iterator foundOutline = m_OutlinerStates.find( args.m_Scene );
    if ( foundOutline != m_OutlinerStates.end() )
    {
      OutlinerStates* stateInfo = &foundOutline->second;
#ifndef LUNA_SCENE_DISABLE_OUTLINERS
      m_HierarchyOutline->RestoreState( stateInfo->m_Hierarchy );
      m_TypeOutline->RestoreState( stateInfo->m_Types );
      m_EntityAssetOutline->RestoreState( stateInfo->m_EntityAssetes );
      m_RuntimeClassOutline->RestoreState( stateInfo->m_RuntimeClasses );
#endif
    }

    // Iterate over the node types looking for the layer node type
    HM_StrToSceneNodeTypeSmartPtr::const_iterator nodeTypeItr = args.m_Scene->GetNodeTypesByName().begin();
    HM_StrToSceneNodeTypeSmartPtr::const_iterator nodeTypeEnd = args.m_Scene->GetNodeTypesByName().end();
    for ( ; nodeTypeItr != nodeTypeEnd; ++nodeTypeItr )
    {
      const SceneNodeTypePtr& nodeType = nodeTypeItr->second;
      if ( Reflect::Registry::GetInstance()->GetClass( nodeType->GetInstanceType() )->HasType( Reflect::GetType<Luna::Layer>() ) )
      {
        // Now that we have the layer node type, iterate over all the layer instances and
        // add them to the layer grid UI.
        HM_SceneNodeSmartPtr::const_iterator instItr = nodeTypeItr->second->GetInstances().begin();
        HM_SceneNodeSmartPtr::const_iterator instEnd = nodeTypeItr->second->GetInstances().end();

        //Begin batching
        BeginLayersGridBatching();

        for ( ; instItr != instEnd; ++instItr )
        {
          const SceneNodePtr& dependNode    = instItr->second;
          Luna::Layer*        lunaLayer     = Reflect::AssertCast< Luna::Layer >( dependNode );
          Content::Layer*     contentLayer  = lunaLayer->GetPackage<Content::Layer>();
          LayerGrid*          layerGrid     = GetLayerGridByType((Content::LayerType)contentLayer->m_Type);

          //Add the layer
          layerGrid->AddLayer( lunaLayer );
        }
        
        //End batching
        EndLayersGridBatching();
      } 

      else if ( nodeType->HasType( Reflect::GetType<Luna::HierarchyNodeType>() ) )
      {
        // Hierarchy node types need to be added to the object grid UI.
        Luna::HierarchyNodeType* hierarchyNodeType = Reflect::AssertCast< Luna::HierarchyNodeType >( nodeTypeItr->second );
        m_TypeGrid->AddType( hierarchyNodeType );
      }
    }

    // Restore selection-sensitive settings
    args.m_Scene->RefreshSelection();

    // Restore tool
    if (args.m_Scene->GetTool().ReferencesObject())
    {
      // Restore tool to the view from the scene
      m_View->SetTool(args.m_Scene->GetTool());

      // Restore tool attributes
      args.m_Scene->GetTool()->CreateProperties();

      // Layout ui
      m_ToolProperties.Layout();

      // Read state
      m_ToolProperties.Read();
    }
  }
}

void SceneEditor::DocumentModified( const DocumentChangedArgs& args )
{
  bool doAnyDocsNeedSaved = false;
  OS_DocumentSmartPtr::Iterator docItr = m_SceneManager.GetDocuments().Begin();
  OS_DocumentSmartPtr::Iterator docEnd = m_SceneManager.GetDocuments().End();
  for ( ; docItr != docEnd; ++docItr )
  {
    if ( ( *docItr )->IsModified() || ( *docItr )->GetFileID() == TUID::Null )
    {
      doAnyDocsNeedSaved = true;
      break;
    }
  }

  m_StandardToolBar->EnableTool( wxID_SAVE, doAnyDocsNeedSaved );
  m_FileMenu->Enable( wxID_SAVE, doAnyDocsNeedSaved );
}

void SceneEditor::DocumentClosed( const DocumentChangedArgs& args )
{
  DocumentModified( args );

  args.m_Document->RemoveDocumentModifiedListener( DocumentChangedSignature::Delegate( this, &SceneEditor::DocumentModified ) );
  args.m_Document->RemoveDocumentSavedListener( DocumentChangedSignature::Delegate( this, &SceneEditor::DocumentModified ) );
  args.m_Document->RemoveDocumentClosedListener( DocumentChangedSignature::Delegate( this, &SceneEditor::DocumentModified ) );
}

void SceneEditor::ViewToolChanged( const ToolChangeArgs& args )
{
  i32 selectedTool = SceneEditorIDs::ID_ToolsSelect;
  if ( args.m_NewTool )
  {
    if ( args.m_NewTool->HasType( Reflect::GetType<Luna::TransformManipulator>() ) )
    {
      Luna::TransformManipulator* manipulator = Reflect::DangerousCast< Luna::TransformManipulator >( args.m_NewTool );
      switch ( manipulator->GetMode() )
      {
      case ManipulatorModes::Scale:
        selectedTool = SceneEditorIDs::ID_ToolsScale;
        break;

      case ManipulatorModes::ScalePivot:
        selectedTool = SceneEditorIDs::ID_ToolsScalePivot;
        break;

      case ManipulatorModes::Rotate:
        selectedTool = SceneEditorIDs::ID_ToolsRotate;
        break;

      case ManipulatorModes::RotatePivot:
        selectedTool = SceneEditorIDs::ID_ToolsRotatePivot;
        break;

      case ManipulatorModes::Translate:
        selectedTool = SceneEditorIDs::ID_ToolsTranslate;
        break;

      case ManipulatorModes::TranslatePivot:
        selectedTool = SceneEditorIDs::ID_ToolsTranslatePivot;
        break;
      }
    }
    else if ( args.m_NewTool->GetType() == Reflect::GetType<Luna::EntityCreateTool>() )
    {
      selectedTool = SceneEditorIDs::ID_ToolsEntityCreate;
    }
    else if ( args.m_NewTool->GetType() == Reflect::GetType<Luna::VolumeCreateTool>() )
    {
      selectedTool = SceneEditorIDs::ID_ToolsVolumeCreate;
    }
    else if ( args.m_NewTool->GetType() == Reflect::GetType<Luna::ClueCreateTool>() )
    {
      selectedTool = SceneEditorIDs::ID_ToolsClueCreate;
    }
#if LUNA_GAME_CAMERA
    else if ( args.m_NewTool->GetType() == Reflect::GetType<Luna::GameCameraCreateTool>() )
    {
      selectedTool = SceneEditorIDs::ID_ToolsGameCameraCreate;
    }
#endif
    else if ( args.m_NewTool->GetType() == Reflect::GetType<Luna::ControllerCreateTool>() )
    {
      selectedTool = SceneEditorIDs::ID_ToolsControllerCreate;
    }
    else if ( args.m_NewTool->GetType() == Reflect::GetType<Luna::LocatorCreateTool>() )
    {
      selectedTool = SceneEditorIDs::ID_ToolsLocatorCreate;
    }
    else if ( args.m_NewTool->GetType() == Reflect::GetType<Luna::DuplicateTool>() )
    {
      selectedTool = SceneEditorIDs::ID_ToolsDuplicate;
    }
    else if ( args.m_NewTool->GetType() == Reflect::GetType<Luna::CurveCreateTool>() )
    {
      selectedTool = SceneEditorIDs::ID_ToolsCurveCreate;
    }
    else if ( args.m_NewTool->GetType() == Reflect::GetType<Luna::CurveEditTool>() )
    {
      selectedTool = SceneEditorIDs::ID_ToolsCurveEdit;
    }
    else if ( args.m_NewTool->GetType() == Reflect::GetType<Luna::LightCreateTool>() )
    {
      selectedTool = SceneEditorIDs::ID_ToolsLightCreate;
    }
    else if ( args.m_NewTool->GetType() == Reflect::GetType<Luna::LightingTool>() )
    {
      selectedTool = SceneEditorIDs::ID_ToolsLighting;
    }
    else if ( args.m_NewTool->GetType() == Reflect::GetType<Luna::PostProcessingVolumeCreateTool>() )
    {
      selectedTool = SceneEditorIDs::ID_ToolsPostProcessingVolumeCreate;
    }
    else if ( args.m_NewTool->GetType() == Reflect::GetType<Luna::NavMeshCreateTool>() )
    {
      selectedTool = SceneEditorIDs::ID_ToolsNavMeshCreate;
    }
  }

  m_ToolsToolBar->ToggleTool( SceneEditorIDs::ID_ToolsSelect, selectedTool == SceneEditorIDs::ID_ToolsSelect );
  m_ToolsToolBar->ToggleTool( SceneEditorIDs::ID_ToolsScale, selectedTool == SceneEditorIDs::ID_ToolsScale );
  m_ToolsToolBar->ToggleTool( SceneEditorIDs::ID_ToolsScalePivot, selectedTool == SceneEditorIDs::ID_ToolsScalePivot );
  m_ToolsToolBar->ToggleTool( SceneEditorIDs::ID_ToolsRotate, selectedTool == SceneEditorIDs::ID_ToolsRotate );
  m_ToolsToolBar->ToggleTool( SceneEditorIDs::ID_ToolsRotatePivot, selectedTool == SceneEditorIDs::ID_ToolsRotatePivot );
  m_ToolsToolBar->ToggleTool( SceneEditorIDs::ID_ToolsTranslate, selectedTool == SceneEditorIDs::ID_ToolsTranslate );
  m_ToolsToolBar->ToggleTool( SceneEditorIDs::ID_ToolsTranslatePivot, selectedTool == SceneEditorIDs::ID_ToolsTranslatePivot );
  m_ToolsToolBar->ToggleTool( SceneEditorIDs::ID_ToolsEntityCreate, selectedTool == SceneEditorIDs::ID_ToolsEntityCreate );
  m_ToolsToolBar->ToggleTool( SceneEditorIDs::ID_ToolsVolumeCreate, selectedTool == SceneEditorIDs::ID_ToolsVolumeCreate );
  m_ToolsToolBar->ToggleTool( SceneEditorIDs::ID_ToolsClueCreate, selectedTool == SceneEditorIDs::ID_ToolsClueCreate );
#if LUNA_GAME_CAMERA
  m_ToolsToolBar->ToggleTool( SceneEditorIDs::ID_ToolsGameCameraCreate, selectedTool == SceneEditorIDs::ID_ToolsGameCameraCreate );
#endif
  m_ToolsToolBar->ToggleTool( SceneEditorIDs::ID_ToolsControllerCreate, selectedTool == SceneEditorIDs::ID_ToolsControllerCreate );
  m_ToolsToolBar->ToggleTool( SceneEditorIDs::ID_ToolsLocatorCreate, selectedTool == SceneEditorIDs::ID_ToolsLocatorCreate );
  m_ToolsToolBar->ToggleTool( SceneEditorIDs::ID_ToolsDuplicate, selectedTool == SceneEditorIDs::ID_ToolsDuplicate );
  m_ToolsToolBar->ToggleTool( SceneEditorIDs::ID_ToolsCurveCreate, selectedTool == SceneEditorIDs::ID_ToolsCurveCreate );
  m_ToolsToolBar->ToggleTool( SceneEditorIDs::ID_ToolsCurveEdit, selectedTool == SceneEditorIDs::ID_ToolsCurveEdit );
  m_ToolsToolBar->ToggleTool( SceneEditorIDs::ID_ToolsLightCreate, selectedTool == SceneEditorIDs::ID_ToolsLightCreate );
  m_ToolsToolBar->ToggleTool( SceneEditorIDs::ID_ToolsLighting, selectedTool == SceneEditorIDs::ID_ToolsLighting );
  m_ToolsToolBar->ToggleTool( SceneEditorIDs::ID_ToolsPostProcessingVolumeCreate, selectedTool == SceneEditorIDs::ID_ToolsPostProcessingVolumeCreate ); 
  
  m_NavToolBar->ToggleTool( SceneEditorIDs::ID_ToolsNavMeshCreate, selectedTool == SceneEditorIDs::ID_ToolsNavMeshCreate ); 
}

///////////////////////////////////////////////////////////////////////////////
// Adds items that have been selected via the viewing rectangle into the context menu
// for further selection
void SceneEditor::OpenManifestContextMenu(const SelectArgs& args)
{
  m_SceneManager.GetCurrentScene()->ClearHighlight( ClearHighlightArgs (false) );

  bool result = m_SceneManager.GetCurrentScene()->Pick(args.m_Pick);

  OS_SelectableDumbPtr selectableItems;
  V_PickHitSmartPtr::const_iterator itr = args.m_Pick->GetHits().begin();
  V_PickHitSmartPtr::const_iterator end = args.m_Pick->GetHits().end();
  for ( ; itr != end; ++itr )
  {
    Selectable* selectable = Reflect::ObjectCast<Selectable>((*itr)->GetObject());
    if (selectable)
    {
      // add it to the new selection list
      selectableItems.Append(selectable);
    }
  }

  if (m_SceneManager.GetCurrentScene()->GetTool() && !m_SceneManager.GetCurrentScene()->GetTool()->ValidateSelection (selectableItems))
  {
    return;
  }

  // add new items to the context menu
  if( !selectableItems.Empty() )
  {
    {
      OS_SelectableDumbPtr::Iterator itr = selectableItems.Begin();
      OS_SelectableDumbPtr::Iterator end = selectableItems.End();
      for( ; itr != end; ++itr)
      {
        Selectable* selection = *itr;

        if( selection->IsSelectable() )
        {
          Luna::HierarchyNode* node = Reflect::ObjectCast<Luna::HierarchyNode>( selection );

          if( node )
          {
            m_OrderedContextItems.push_back( node );
          }
        }
      }
    }
  
    if( !m_OrderedContextItems.empty() )
    { 
      wxMenu contextMenu;

      V_HierarchyNodeDumbPtr::iterator itr = m_OrderedContextItems.begin();
      V_HierarchyNodeDumbPtr::iterator end = m_OrderedContextItems.end();

      // sort the context items
      std::sort( itr, end, SortContextItemsByName);
       
      // append items to the context menu
      u32 index = 0;
      for( ;itr != end; ++itr, ++index )
      {
        std::string str = (*itr)->GetName();

        std::string desc = (*itr)->GetDescription();

        if (!desc.empty())
        {
          str += '\t' + desc;
        }

        contextMenu.Append( SceneEditorIDs::ID_SelectContextMenu + index , str.c_str() );
      }

      contextMenu.SetEventHandler( GetEventHandler() );
      GetEventHandler()->Connect( SceneEditorIDs::ID_SelectContextMenu, SceneEditorIDs::ID_SelectContextMenu +  (u32)m_OrderedContextItems.size(),wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SceneEditor::OnManifestContextMenu ), NULL, this );
      PopupMenu( &contextMenu );
      GetEventHandler()->Disconnect( SceneEditorIDs::ID_SelectContextMenu, SceneEditorIDs::ID_SelectContextMenu +  (u32)m_OrderedContextItems.size(),wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SceneEditor::OnManifestContextMenu ), NULL, this ); 
      m_OrderedContextItems.clear();
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Callback for when the pane on a drawer is modified (meaning that we need to 
// update the FrameManager).
//
void SceneEditor::OnDrawerPaneModified( const DrawerArgs& args )
{
  m_FrameManager.Update();
}

///////////////////////////////////////////////////////////////////////////////
// Adds the drawer to the panel and does other setup work.
// 
void SceneEditor::AddDrawer( Drawer* drawer )
{
  m_DrawerPanel->AddDrawer( drawer );
  drawer->AddPaneModifiedListener( DrawerSignature::Delegate( this, &SceneEditor::OnDrawerPaneModified ) );
}

///////////////////////////////////////////////////////////////////////////////
// Opens up a context menu that allows selection basd on type
//
void SceneEditor::OpenTypeContextMenu( const SelectArgs& args )
{
  wxMenu contextMenu;
  u32 numMenuItems = 0;

  // simple select functionality
  if (m_SceneManager.GetCurrentScene()->HasHighlighted())
  {
    // need to provide the select args if needed
    GeneralCallbackData* data = new GeneralCallbackData;
    data->m_GeneralData = (void*)( &args );
    GetEventHandler()->Connect( SceneEditorIDs::ID_SelectContextMenu + numMenuItems, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SceneEditor::SelectItemInScene ), data, this );
    contextMenu.Append( SceneEditorIDs::ID_SelectContextMenu + numMenuItems, "Select" );
    ++numMenuItems;
  }

  if (!m_SceneManager.GetCurrentScene()->GetSelection().GetItems().Empty())
  {
    GetEventHandler()->Connect( SceneEditorIDs::ID_SelectContextMenu + numMenuItems, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SceneEditor::SelectSimilarItemsInScene ), NULL, this );
    contextMenu.Append( SceneEditorIDs::ID_SelectContextMenu + numMenuItems, "Select Similar" );
    ++numMenuItems;
  }

  if (numMenuItems > 0)
  {
    contextMenu.AppendSeparator();
  }

  // populate this with the appropriate types
  const HM_StrToSceneNodeTypeSmartPtr& sceneNodeTypes = m_SceneManager.GetCurrentScene()->GetNodeTypesByName();

  if( !sceneNodeTypes.empty() )
  {   
    SetupTypeContextMenu( sceneNodeTypes, contextMenu, numMenuItems );
  }

  // set the current event handler on the context menu and pop it up
  contextMenu.SetEventHandler( GetEventHandler() );
  PopupMenu( &contextMenu );

  // this must be done piecemeal because the range version of Disconnect() will only disconnect the ranged version Connect()
  for ( u32 i = SceneEditorIDs::ID_SelectContextMenu; i < SceneEditorIDs::ID_SelectContextMenu + numMenuItems; i++ )
  {
    // clean up, disconnect any id that was set up for any of the items
    GetEventHandler()->Disconnect( i, wxEVT_COMMAND_MENU_SELECTED );
  }
}

///////////////////////////////////////////////////////////////////////////////
// Populates the context menu with selections from the various types
//
void SceneEditor::SetupTypeContextMenu( const HM_StrToSceneNodeTypeSmartPtr& sceneNodeTypes, wxMenu& contextMenu, u32& numMenuItems )
{
  V_SceneNodeTypeDumbPtr orderedTypes;

  {
    HM_StrToSceneNodeTypeSmartPtr::const_iterator itr = sceneNodeTypes.begin();
    HM_StrToSceneNodeTypeSmartPtr::const_iterator end = sceneNodeTypes.end();
    
    for( ;itr != end ;++itr )
    {
      orderedTypes.push_back( itr->second );
    }
  }

  V_SceneNodeTypeDumbPtr::iterator itr = orderedTypes.begin();
  V_SceneNodeTypeDumbPtr::iterator end = orderedTypes.end();

  std::sort( itr, end, SceneEditor::SortTypeItemsByName );

  // iterate over the scene node types, making a new sub menu for each
  for( ; itr != end; ++itr )
  {
    const Luna::SceneNodeType* type( *itr );
    const HM_SceneNodeSmartPtr& typeInstances( type->GetInstances() );

    if( !typeInstances.empty() )
    {
      wxMenu* subMenu = new wxMenu;
      
      // add selection for all items
      ContextCallbackData* data = new ContextCallbackData;
      data->m_ContextCallbackType = ContextCallbackTypes::All;
      data->m_NodeType = type;

      GetEventHandler()->Connect( SceneEditorIDs::ID_SelectContextMenu + numMenuItems, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SceneEditor::OnTypeContextMenu ), data, this );
      subMenu->Append( SceneEditorIDs::ID_SelectContextMenu + numMenuItems, "Select All" );      
      ++numMenuItems;

      // add selection for individual items
      {   
        wxMenu* itemMenu = new wxMenu;

        HM_SceneNodeSmartPtr::const_iterator inst_itr = typeInstances.begin();
        HM_SceneNodeSmartPtr::const_iterator inst_end = typeInstances.end();

        V_SceneNodeDumbPtr ordered;

        // push the instances into a vector for sorting purposes
        for( ; inst_itr != inst_end; ++inst_itr )
        {
          ordered.push_back( inst_itr->second );
        }  
        
        V_SceneNodeDumbPtr::iterator ord_itr = ordered.begin();
        V_SceneNodeDumbPtr::iterator ord_end = ordered.end();

        std::sort( ord_itr, ord_end, SortContextItemsByName );

        // setting up the menu item for each of the items
        for( ;ord_itr != ord_end; ++ord_itr )
        {    
          ContextCallbackData* data = new ContextCallbackData;
          data->m_ContextCallbackType = ContextCallbackTypes::Item;
          data->m_NodeInstance = *ord_itr;

          GetEventHandler()->Connect( SceneEditorIDs::ID_SelectContextMenu + numMenuItems, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SceneEditor::OnTypeContextMenu ), data, this );
          itemMenu->Append( SceneEditorIDs::ID_SelectContextMenu + numMenuItems, (*ord_itr)->GetName().c_str() );
          ++numMenuItems;
        }

        // add the items menu to the sub menu
        subMenu->Append( SceneEditorIDs::ID_SelectContextMenu + numMenuItems, "Select Single", itemMenu );
        ++numMenuItems;

        // if this is an entity, then we need to check if it has art classes
        const Luna::EntityType* entity = Reflect::ConstObjectCast<Luna::EntityType>( type );

        // if this is an instance, then we need to check if it has code classes
        const Luna::InstanceType* instance = Reflect::ConstObjectCast<Luna::InstanceType>( type );

        if (entity)
        {
          // set up for entity types
          if (SetupEntityTypeMenus( entity, subMenu, numMenuItems ))
          {
            // setup for geometry visible objects
            {
              ContextCallbackData* data = new ContextCallbackData;
              data->m_ContextCallbackType = ContextCallbackTypes::Entity_Visible_Geometry;
              data->m_NodeType = type;

              GetEventHandler()->Connect( SceneEditorIDs::ID_SelectContextMenu + numMenuItems, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SceneEditor::OnTypeContextMenu ), data, this );
              subMenu->Append( SceneEditorIDs::ID_SelectContextMenu + numMenuItems, "Select All With Geometry Shown" );      
              ++numMenuItems;      
            }

            // setup for geometry invisible objects         
            {
              ContextCallbackData* data = new ContextCallbackData;
              data->m_ContextCallbackType = ContextCallbackTypes::Entity_Invisible_Geometry;
              data->m_NodeType = type;

              GetEventHandler()->Connect( SceneEditorIDs::ID_SelectContextMenu + numMenuItems, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SceneEditor::OnTypeContextMenu ), data, this );
              subMenu->Append( SceneEditorIDs::ID_SelectContextMenu + numMenuItems, "Select All Without Geometry Shown" );      
              ++numMenuItems;      
            }
          }
        }

        if (instance)
        {
          // set up for entity types
          SetupInstanceTypeMenus( instance, subMenu, numMenuItems );
        }
      }
      contextMenu.Append( SceneEditorIDs::ID_SelectContextMenu + numMenuItems, type->GetName().c_str(), subMenu );
      ++numMenuItems;
    }
  }
}

bool SceneEditor::SetupEntityTypeMenus( const Luna::EntityType* entity, wxMenu* subMenu, u32& numMenuItems )
{
  const M_InstanceSetSmartPtr& sets = entity->GetSets();

  if( !sets.empty() )
  {
    bool added = false;

    wxMenu* menu = new wxMenu;

    M_InstanceSetSmartPtr::const_iterator itr = sets.begin();
    M_InstanceSetSmartPtr::const_iterator end = sets.end();
    for( ;itr != end; ++itr )
    {
      const Luna::EntityAssetSet* art = Reflect::ObjectCast<Luna::EntityAssetSet>( itr->second );
      if (art && !art->GetContentFile().empty())
      {
        std::string artPath( art->GetContentFile() );
        FileSystem::StripPrefix( Finder::ProjectAssets(), artPath );

        // Why is the art path blank?
        NOC_ASSERT(!artPath.empty());

        ContextCallbackData* data = new ContextCallbackData;
        data->m_ContextCallbackType = ContextCallbackTypes::Instance;
        data->m_InstanceSet = art;

        GetEventHandler()->Connect( SceneEditorIDs::ID_SelectContextMenu + numMenuItems, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SceneEditor::OnTypeContextMenu ), data, this );
        menu->Append( SceneEditorIDs::ID_SelectContextMenu + numMenuItems, artPath.c_str() );
        ++numMenuItems;
        added = true;
      }
    }

    if (added)
    {
      subMenu->AppendSeparator();
      subMenu->Append( SceneEditorIDs::ID_SelectContextMenu + numMenuItems, "Select All With Art Class", menu );
      ++numMenuItems;
    }
    else
    {
      delete menu;
    }

    return added;
  }

  return false;
}

bool SceneEditor::SetupInstanceTypeMenus( const Luna::InstanceType* instance, wxMenu* subMenu, u32& numMenuItems )
{
  const M_InstanceSetSmartPtr& sets = instance->GetSets();

  if( !sets.empty() )
  {
    bool added = false;

    wxMenu* menu = new wxMenu;

    M_InstanceSetSmartPtr::const_iterator itr = sets.begin();
    M_InstanceSetSmartPtr::const_iterator end = sets.end();
    for( ; itr != end; ++itr )
    {
      const Luna::InstanceCodeSet* code = Reflect::ObjectCast<Luna::InstanceCodeSet>( itr->second );
      if (code && !code->GetName().empty())
      {
        ContextCallbackData* data = new ContextCallbackData;
        data->m_ContextCallbackType = ContextCallbackTypes::Instance;
        data->m_InstanceSet = code;

        GetEventHandler()->Connect( SceneEditorIDs::ID_SelectContextMenu + numMenuItems, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( SceneEditor::OnTypeContextMenu ), data, this );
        menu->Append( SceneEditorIDs::ID_SelectContextMenu + numMenuItems, code->GetName().c_str() );
        ++numMenuItems;
        added = true;
      }
    }

    if (added)
    {
      subMenu->AppendSeparator();
      subMenu->Append( SceneEditorIDs::ID_SelectContextMenu + numMenuItems, "Select All With Code Class", menu );
      ++numMenuItems;
    }
    else
    {
      delete menu;
    }

    return added;
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////
// Static function used to sort context items by name
bool SceneEditor::SortContextItemsByName( Luna::SceneNode* lhs, Luna::SceneNode* rhs )
{
  std::string lname( lhs->GetName() );
  std::string rname( rhs->GetName() );

  boost::to_upper( lname );
  boost::to_upper( rname );

  return lname < rname;
}

///////////////////////////////////////////////////////////////////////////////
// Static function used to sort type items by name
bool SceneEditor::SortTypeItemsByName( Luna::SceneNodeType* lhs, Luna::SceneNodeType* rhs )
{
  std::string lname( lhs->GetName() );
  std::string rname( rhs->GetName() );

  boost::to_upper( lname );
  boost::to_upper( rname );

  return lname < rname;
}

DocumentManager* SceneEditor::GetDocumentManager()
{
  return &m_SceneManager;
}

void SceneEditor::TakeViewerControl()
{
  m_HasViewerControl = true;

  m_RemoteScene->Enable( true );
  m_RemoteScene->ResetScene();

  Manager::GetInstance()->PerformAutoStart();
}

void SceneEditor::ReleaseViewerControl()
{
  m_HasViewerControl = false;

  m_RemoteScene->Enable( false );
}

void SceneEditor::SyncPropertyThread()
{
  while ( m_SelectionPropertiesManager->ThreadsActive() )
  {
    ::Sleep( 500 );
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////
//
LightingLayerGrid*  SceneEditor::GetLightingLayerGrid()
{
  if(m_LayerGrids.size() > Content::LayerTypes::LT_Lighting)
  {
    return (LightingLayerGrid*)m_LayerGrids[Content::LayerTypes::LT_Lighting].Ptr();
  }

  return NULL;
}

LayerGrid*          SceneEditor::GetLayerGridByType(Content::LayerType lType)
{
  NOC_ASSERT(m_LayerGrids.size() > (size_t)lType);

  return m_LayerGrids[lType];
}

Content::LayerType  SceneEditor::GetCurrentLayerGridType()
{
  NOC_ASSERT(m_LayersNotebook);
  
  int index = m_LayersNotebook->GetSelection();

  if(index >= (int)Content::LayerTypes::LT_Unknown)
  {
    NOC_ASSERT(!"Invalid Page index");
    return Content::LayerTypes::LT_Unknown;
  }

  return (Content::LayerType)index;
}

void  SceneEditor::GeneratePostProcessingVolumeScript()
{
  Luna::Scene*  currentScene = m_SceneManager.GetCurrentScene();
  std::string   scriptVolume;
  std::string   scriptString = "--Empty--";
  std::string   scriptError;

  if(currentScene != NULL)
  {
    OS_SelectableDumbPtr            selectionSet  = currentScene->GetSelection().GetItems();
    OS_SelectableDumbPtr::Iterator  selectionItr  = selectionSet.Begin();
    OS_SelectableDumbPtr::Iterator  selectionEnd  = selectionSet.End();
    std::stringstream               script;

    const u32                       c_entriesPerRow = 16;
    const u32                       c_rowCount      = 512/c_entriesPerRow;
    u32                             palette[512];

    for ( ; selectionItr != selectionEnd; ++selectionItr )
    {
      Selectable*                 selectable  = (*selectionItr);
      Luna::PostProcessingVolume* volume      = Reflect::ObjectCast< Luna::PostProcessingVolume >( selectable );
      if(volume  != NULL)
      {
        Content::PostProcessingVolume*  contentPostProcVol = volume->GetPackage<Content::PostProcessingVolume>();

        scriptVolume  = volume->GetName();

        script <<      "--///////////////////////////////////////////////////////////////////////////////////////////////" << std::endl;
        script <<      "function post_processing_volume_script( transition_time ) prt( \"post_processing_volume_script\" )" << std::endl;
        script <<      "	-- Script Volume Source: " <<    scriptVolume <<  "             --" << std::endl;
        script << std::endl;

        script <<      "	local post_effect_node                       =  pfx_create_node()  " << std::endl;
        script <<      "	local post_effect_attack_timer               =  transition_time    " << std::endl;
        script <<      "	local post_effect_sustain_timer              = 10.0                " << std::endl;
        script <<      "	local post_effect_permanent_on_sustain_timer = true                " << std::endl;
        script <<      "	local post_effect_decay_timer                =  2.0                " << std::endl; 
        script <<      "	pfx_set_node_lifetime             (post_effect_node, post_effect_attack_timer, post_effect_sustain_timer, post_effect_decay_timer) " << std::endl;
        script <<      "	pfx_set_node_permanent_on_sustain (post_effect_node, post_effect_permanent_on_sustain_timer)                                       " << std::endl;
        script << std::endl;
        script << std::endl;

        //Color Correction palette
        {
          Attribute::AttributeViewer< Content::PostEffectsColorCorrectionAttribute > ccAttr( contentPostProcVol );
   
          if(ccAttr.Valid())
          {
            size_t weightsSize = ccAttr->m_DepthEffectWeight.size();
            size_t colorSize   = ccAttr->m_DepthEffectColor.size();

            //Check if we have any keyed values
            if(weightsSize || colorSize)
            {
              script <<      "	-- Color Correction                                   --" << std::endl;

              //We have a valid palette
              ColorPalette::Palette::GeneratePalette(ccAttr->m_DepthEffectWeight, ccAttr->m_DepthEffectColor, ccAttr->m_CustomPalette, palette);

              for(u32 rowIndex = 0, currentCCEntry = 0; rowIndex < c_rowCount; ++rowIndex)
              {
                if(rowIndex == 0)
                {
                  script <<      "	local varCCPaletteStr = string.char(";
                }            
                else
                {
                  script <<      "	varCCPaletteStr = varCCPaletteStr .. string.char(";
                }  

                for(u32 rowEntry = 0; rowEntry < c_entriesPerRow; ++rowEntry, ++currentCCEntry)
                {         
                  const u32  a  = (palette[currentCCEntry] >> 24) & 0xFF;
                  const u32  r  = (palette[currentCCEntry] >> 16) & 0xFF;
                  const u32  g  = (palette[currentCCEntry] >>  8) & 0xFF;
                  const u32  b  = (palette[currentCCEntry] >>  0) & 0xFF;

                  script << a << ", " <<  r << ", " <<  g << ", " << b;

                  if(rowEntry != c_entriesPerRow - 1)
                  {
                    script << ", "; 
                  }
                }
                script << ")" << std::endl;
              }
              script << std::endl;
              script << std::endl;
              script <<      "	pfx_set_node_color_correction_palette (post_effect_node, varCCPaletteStr)                                             " << std::endl;
              script << std::endl;
            }
          }
        }

        //Fog palette
        {
          Attribute::AttributeViewer< Content::PostEffectsFogAttribute > fogAttr( contentPostProcVol );
   
          if(fogAttr.Valid())
          {
            size_t weightsSize = fogAttr->m_Weight.size();
            size_t colorSize   = fogAttr->m_Color.size();

            //Check if we have any keyed values
            if(weightsSize || colorSize)
            {
              script <<      "	-- Fog                                   --" << std::endl;

              //We have a valid palette
              ColorPalette::Palette::GeneratePalette(fogAttr->m_Weight, fogAttr->m_Color, fogAttr->m_CustomPalette, palette);

              for(u32 rowIndex = 0, currentFogEntry = 0; rowIndex < c_rowCount; ++rowIndex)
              {
                if(rowIndex == 0)
                {
                  script <<      "	local varFogPaletteStr = string.char(";
                }
                else
                {
                  script <<      "	varFogPaletteStr = varFogPaletteStr .. string.char(";
                }

                for(u32 rowEntry = 0; rowEntry < c_entriesPerRow; ++rowEntry, ++currentFogEntry)
                {         
                  const u32  a  = (palette[currentFogEntry] >> 24) & 0xFF;
                  const u32  r  = (palette[currentFogEntry] >> 16) & 0xFF;
                  const u32  g  = (palette[currentFogEntry] >>  8) & 0xFF;
                  const u32  b  = (palette[currentFogEntry] >>  0) & 0xFF;

                  script << a << ", " <<  r << ", " <<  g << ", " << b;

                  if(rowEntry != c_entriesPerRow - 1)
                  {
                    script << ", "; 
                  }
                }
                script << ")" << std::endl;
              }
              script << std::endl;
              script << std::endl;
              script <<      "	pfx_set_node_fog_palette          (post_effect_node, varFogPaletteStr)                                              " << std::endl;
              script << std::endl;
            }
          }
        }

        {
          //Curve Control
          Attribute::AttributeViewer< Content::PostEffectsCurveControlAttribute > ccAttr( contentPostProcVol );

          if(ccAttr.Valid())
          {
            //Try generating a palette
            bool result = BuilderUtil::GenerateACVPalette(ccAttr->m_CurveId, ccAttr->m_CurveId_CRT, palette);

            // Verify the palette
            if(result == true)
            {
              script <<      "	-- Curve Control                                   --" << std::endl;

              for(u32 rowIndex = 0, currentCurveControlEntry = 0; rowIndex < c_rowCount; ++rowIndex)
              {
                if(rowIndex == 0)
                {
                  script <<      "	local varCurveControlPaletteStr = string.char(";
                }
                else
                {
                  script <<      "	varCurveControlPaletteStr = varCurveControlPaletteStr .. string.char(";
                }

                const u32  a  =                                             0xFF;

                for(u32 rowEntry = 0; rowEntry < c_entriesPerRow; ++rowEntry, ++currentCurveControlEntry)
                {         
                  const u32  r  = (palette[currentCurveControlEntry] >> 16) & 0xFF;
                  const u32  g  = (palette[currentCurveControlEntry] >>  8) & 0xFF;
                  const u32  b  = (palette[currentCurveControlEntry] >>  0) & 0xFF;

                  script << a << ", " <<  r << ", " <<  g << ", " << b;

                  if(rowEntry != c_entriesPerRow - 1)
                  {
                    script << ", "; 
                  }
                }
                script << ")" << std::endl;
              }
              script << std::endl;
              script << std::endl;
              script <<      "	pfx_set_node_curve_control_palette          (post_effect_node, varCurveControlPaletteStr)                                              " << std::endl;
              script << std::endl;
            }            
          }
        }

        {
          //Depth of field
          Attribute::AttributeViewer< Content::PostEffectsDepthOfFieldAttribute > dofAttr( contentPostProcVol );

          if(dofAttr.Valid())
          {
            script << std::endl;
            script <<      "	--Depth Of Field                                   --" << std::endl;
   
            script <<      "	local dof_near_start_dist            =  0.0                                                                      " << std::endl;
            script <<      "	local dof_near_end_dist              =  0.0                                                                      " << std::endl;
            script <<      "	local dof_near_max_blur              =  0.0                                                                      " << std::endl;

            script <<      "	local dof_far_start_dist             =  " <<   dofAttr->m_FarStartDistance                                           << std::endl;
            script <<      "	local dof_far_end_dist               =  " <<   dofAttr->m_FarEndDistance                                             << std::endl;
            script <<      "	local dof_far_max_blur               =  " <<   dofAttr->m_FarMaxBlur                                                 << std::endl;
            script <<      "	pfx_set_node_depth_of_field       (post_effect_node, dof_near_start_dist, dof_near_end_dist, dof_near_max_blur, dof_far_start_dist, dof_far_end_dist, dof_far_max_blur)" << std::endl;
          }
        }

        {
          //Bloom
          Attribute::AttributeViewer< Content::PostEffectsBloomAttribute > bloomAttr( contentPostProcVol );

          if(bloomAttr.Valid())
          {
            script << std::endl;
            script <<      "	--Bloom                                   --" << std::endl;
            script <<      "	local bloom_prescale                   =  " <<   bloomAttr->m_Prescale                                                << std::endl;
            script <<      "	local bloom_exponent                   =  " <<   bloomAttr->m_Exponent                                                << std::endl;
            script <<      "	local bloom_distribution               =  " <<   bloomAttr->m_Distribution                                            << std::endl;
            script <<      "	local bloom_persistance                =  " <<   bloomAttr->m_Persistance                                             << std::endl;
            script <<      "	local bloom_comp_weight                =  " <<   bloomAttr->m_CompositeWeight                                         << std::endl;
            script <<      "	pfx_set_node_bloom                (post_effect_node, bloom_prescale, bloom_exponent, bloom_distribution, bloom_persistance, bloom_comp_weight)" << std::endl;
          }
        }

        {
          //Tint
          Attribute::AttributeViewer< Content::PostEffectsColorAttribute > tintAttr( contentPostProcVol );

          if(tintAttr.Valid())
          {
            f32 r, g, b;
            tintAttr->m_Tint.Get(r, g, b);

            script << std::endl;
            script <<      "	--Tint/Brightness/Saturation                                   --" << std::endl;
            script <<      "	local tint_red                        =  " <<   SrgbToLinear(r)                                                << std::endl;
            script <<      "	local tint_green                      =  " <<   SrgbToLinear(g)                                                << std::endl;
            script <<      "	local tint_blue                       =  " <<   SrgbToLinear(b)                                                << std::endl;
            script <<      "	local tint_saturation                 =  " <<   tintAttr->m_Saturation                                         << std::endl;
            script <<      "	local tint_brightness                 =  " <<   tintAttr->m_Brightness                                         << std::endl;

            script <<      "	pfx_set_node_tint                 (post_effect_node, tint_red, tint_green, tint_blue)                         " << std::endl;
            script <<      "	pfx_set_node_brightness           (post_effect_node, tint_brightness)                                         " << std::endl;
            script <<      "	pfx_set_node_saturation           (post_effect_node, tint_saturation)                                         " << std::endl;
          }
        }

        {
          //Film grain
          Attribute::AttributeViewer< Content::PostEffectsFilmGrainAttribute > grainAttr( contentPostProcVol );

          if(grainAttr.Valid())
          {
            script << std::endl;
            script <<      "	--Film Grain                                   --" << std::endl;
            script <<      "	local grain_intensity                  =  " <<   grainAttr->m_Intensity                               << std::endl;
            script <<      "	local grain_size                       =  " <<   grainAttr->m_Size                                    << std::endl;

            script <<      "	pfx_set_node_film_grain           (post_effect_node, grain_intensity, grain_size)                   " << std::endl;
          }
        }

        {
          //Light Scattering
          Attribute::AttributeViewer< Content::PostEffectsLightScatteringAttribute > lightScatteringAttr( contentPostProcVol );

          if(lightScatteringAttr.Valid())
          {
            f32 r, g, b;
            lightScatteringAttr->m_Color.Get(r, g, b);

            script << std::endl;
            script <<      "	--Light Scattering                                --" << std::endl;
            script <<      "	local lightScattering_dir_x                      =  0.0 -- dir_xyz set to zeros will use global sun direction"            << std::endl;
            script <<      "	local lightScattering_dir_y                      =  0.0 -- dir_xyz set to zeros will use global sun direction"            << std::endl;
            script <<      "	local lightScattering_dir_z                      =  0.0 -- dir_xyz set to zeros will use global sun direction"            << std::endl;
            script <<      "	local lightScattering_red                        =  " <<   SrgbToLinear(r)                                                << std::endl;
            script <<      "	local lightScattering_green                      =  " <<   SrgbToLinear(g)                                                << std::endl;
            script <<      "	local lightScattering_blue                       =  " <<   SrgbToLinear(b)                                                << std::endl;
            script <<      "	local lightScattering_intensity                  =  " <<   lightScatteringAttr->m_Intensity                               << std::endl;
            script <<      "	local lightScattering_mieDir                     =  " <<   lightScatteringAttr->m_MieDirectional                          << std::endl;
            script <<      "	local lightScattering_mieCoef                    =  " <<   lightScatteringAttr->m_MieCoefficient                          << std::endl;

            script <<      "	pfx_set_node_light_scattering     (post_effect_node, lightScattering_dir_x, lightScattering_dir_y, lightScattering_dir_z, lightScattering_red, lightScattering_green, lightScattering_blue, lightScattering_intensity, lightScattering_mieDir, lightScattering_mieCoef)" << std::endl;
          }
          else
          {
            //Check for the new light scattering attribute
            const Luna::LightScattering*  lightScatteringInst = volume->GetLightScatteringInstance();

            if((lightScatteringInst != NULL) && lightScatteringInst->GetIsEnabled())
            {
              Math::Vector3   direction;
              f32             r, g, b;

              lightScatteringInst->GetColor().Get(r, g, b);
              lightScatteringInst->GetDirection(direction);

              script << std::endl;
              script <<      "	--Light Scattering                                   --" << std::endl;
              script <<      "	local lightScattering_dir_x                      =  " <<   direction.x << " -- setting dir_xyz to zeros will use global sun direction"  << std::endl;
              script <<      "	local lightScattering_dir_y                      =  " <<   direction.y << " -- setting dir_xyz to zeros will use global sun direction"  << std::endl;
              script <<      "	local lightScattering_dir_z                      =  " <<   direction.z << " -- setting dir_xyz to zeros will use global sun direction"  << std::endl;
              script <<      "	local lightScattering_red                        =  " <<   SrgbToLinear(r)                                                              << std::endl;
              script <<      "	local lightScattering_green                      =  " <<   SrgbToLinear(g)                                                              << std::endl;
              script <<      "	local lightScattering_blue                       =  " <<   SrgbToLinear(b)                                                              << std::endl;
              script <<      "	local lightScattering_intensity                  =  " <<   lightScatteringInst->GetIntensity()                                          << std::endl;
              script <<      "	local lightScattering_mieDir                     =  " <<   lightScatteringInst->GetMieCoefficient()                                     << std::endl;
              script <<      "	local lightScattering_mieCoef                    =  " <<   lightScatteringInst->GetMieDirectional()                                     << std::endl;

              script <<      "	pfx_set_node_light_scattering     (post_effect_node, lightScattering_dir_x, lightScattering_dir_y, lightScattering_dir_z, lightScattering_red, lightScattering_green, lightScattering_blue, lightScattering_intensity, lightScattering_mieDir, lightScattering_mieCoef)" << std::endl;
            }
          }
        }

        {
          //HDR
          Attribute::AttributeViewer< Content::PostEffectsHDRAttribute > hdrAttr( contentPostProcVol );

          if(hdrAttr.Valid())
          {
            script << std::endl;
            script <<      "	--HDR                                   --" << std::endl;
            script <<      "	local hdr_exposure                   =  " <<   hdrAttr->m_Exposure                                            << std::endl;
            script <<      "	pfx_set_node_hdr                  (post_effect_node, hdr_exposure)"                                                   << std::endl;
          }
        }

        script <<      "end"                                                                                               << std::endl;
        script <<      "--///////////////////////////////////////////////////////////////////////////////////////////////" << std::endl;

        scriptString  = script.str();
        break;
      }
    }
  }

  if(scriptVolume.empty() == true)
  {
    Console::Warning("Failed to generate script: No Post-Processing volume was detected in the current selection set!\n");
  }
  
  Windows::CopyToClipboard(GetHwnd(), scriptString, scriptError);
}