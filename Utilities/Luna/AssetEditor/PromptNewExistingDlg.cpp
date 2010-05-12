#include "Precompile.h"
#include "PromptNewExistingDlg.h"

#include "AssetEditorGenerated.h"

#include "File/Manager.h"
#include "File/ManagedFileDialog.h"
#include "FileBrowser/FileBrowser.h"
#include "FileSystem/FileSystem.h"
#include "UIToolKit/ImageManager.h"

using namespace Luna;

///////////////////////////////////////////////////////////////////////////////
// Constructor
// 
PromptNewExistingDlg::PromptNewExistingDlg( wxWindow* parent, CreateFileCallback callback, const std::string& title, const std::string& desc, const std::string& createLabel, const std::string& existingLabel )
: wxDialog( parent, wxID_ANY, title.c_str(), wxDefaultPosition, wxSize( 500, 210 ), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER, title.c_str() )
, m_CreateFileCallback( callback )
, m_FinderSpec( NULL )
, m_RequiresTuid( true )
{
  NOC_ASSERT( m_CreateFileCallback );
	
	wxBoxSizer* mainSizer;
	mainSizer = new wxBoxSizer( wxVERTICAL );
	
	m_Panel = new PromptNewExistingPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	SetSizeHints( m_Panel->GetMinSize(), m_Panel->GetMaxSize() );
  m_Panel->m_Description->SetLabel( desc.c_str() );
  m_Panel->m_Description->Wrap( GetSize().x - 10 );
  m_Panel->m_RadioBtnNew->SetLabel( createLabel.c_str() );
  m_Panel->m_RadioBtnExisting->SetLabel( existingLabel.c_str() );
  m_Panel->m_ButtonExistingFinder->SetBitmapLabel( UIToolKit::GlobalImageManager().GetBitmap( "magnify_16.png" ) );

	mainSizer->Add( m_Panel, 1, wxEXPAND | wxALL, 5 );

	wxStdDialogButtonSizer* buttonSizer = new wxStdDialogButtonSizer();
	wxButton* buttonOK = new wxButton( this, wxID_OK );
	buttonSizer->AddButton( buttonOK );
	wxButton* buttonCancel = new wxButton( this, wxID_CANCEL );
	buttonSizer->AddButton( buttonCancel );
	buttonSizer->Realize();
	mainSizer->Add( buttonSizer, 0, wxBOTTOM|wxEXPAND, 5 );

  SetSizer( mainSizer );
	Layout();

  // Connect listeners
  m_Panel->m_RadioBtnNew->Connect( m_Panel->m_RadioBtnNew->GetId(), wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( PromptNewExistingDlg::OnRadioButtonSelected ), NULL, this );
  m_Panel->m_RadioBtnExisting->Connect( m_Panel->m_RadioBtnExisting->GetId(), wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( PromptNewExistingDlg::OnRadioButtonSelected ), NULL, this );
  m_Panel->m_ButtonExisting->Connect( m_Panel->m_ButtonExisting->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PromptNewExistingDlg::OnButtonExistingClicked ), NULL, this );
  m_Panel->m_ButtonExistingFinder->Connect( m_Panel->m_ButtonExistingFinder->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PromptNewExistingDlg::OnButtonExistingFinderClicked ), NULL, this );
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
// 
PromptNewExistingDlg::~PromptNewExistingDlg()
{
  // Disconnect listeners
  m_Panel->m_RadioBtnNew->Disconnect( m_Panel->m_RadioBtnNew->GetId(), wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( PromptNewExistingDlg::OnRadioButtonSelected ), NULL, this );
  m_Panel->m_RadioBtnExisting->Disconnect( m_Panel->m_RadioBtnExisting->GetId(), wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( PromptNewExistingDlg::OnRadioButtonSelected ), NULL, this );
  m_Panel->m_ButtonExisting->Disconnect( m_Panel->m_ButtonExisting->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PromptNewExistingDlg::OnButtonExistingClicked ), NULL, this );
  m_Panel->m_ButtonExistingFinder->Disconnect( m_Panel->m_ButtonExistingFinder->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PromptNewExistingDlg::OnButtonExistingFinderClicked ), NULL, this );
}

///////////////////////////////////////////////////////////////////////////////
// Displays the dialog.  If the user chooses to create a new file, the callback
// function that was registered in the constructor will be called.
// 
int PromptNewExistingDlg::ShowModal()
{
  int result = __super::ShowModal();
  if ( result == wxID_OK )
  {
    if ( m_Panel->m_RadioBtnNew->GetValue() )
    {
      // Create the file
      std::string error;
      if ( !( ( *m_CreateFileCallback )( m_Panel->m_FilePathNew->GetValue().c_str(), error ) ) )
      {
        wxMessageBox( error.c_str(), "Error", wxCENTER | wxOK | wxICON_ERROR, this );
        return wxID_CANCEL;
      }
    }
    else
    {
      if ( !FileSystem::IsFile( m_Panel->m_FilePathExisting->GetValue().c_str() ) )
      {
        std::string error( "Invalid file: " );
        error += m_Panel->m_FilePathExisting->GetValue().c_str();
        wxMessageBox( error.c_str(), "Error", wxCENTER | wxOK | wxICON_ERROR, this );
        return wxID_CANCEL;
      }
    }
  }
  return result;
}

///////////////////////////////////////////////////////////////////////////////
// Sets the finder spec to use for any displayed dialogs.
// 
void PromptNewExistingDlg::SetFinderSpec( const Finder::FinderSpec* spec )
{
  m_FinderSpec = spec;
}

///////////////////////////////////////////////////////////////////////////////
// Specifies whether this dialog should enforce creation of a TUID for any files
// selected.
// 
void PromptNewExistingDlg::SetRequiresTuid( bool requiresTuid )
{
  m_RequiresTuid = requiresTuid;
}

///////////////////////////////////////////////////////////////////////////////
// Sets the path displayed in the "create new file" area of the dialog.
// 
void PromptNewExistingDlg::SetNewFile( const std::string& file )
{
  m_Panel->m_FilePathNew->SetValue( file.c_str() );
}

///////////////////////////////////////////////////////////////////////////////
// Sets the path displayed in the "use existing file" area of the dialog.
// 
void PromptNewExistingDlg::SetExistingFile( const std::string& file )
{
  m_Panel->m_FilePathExisting->SetValue( file.c_str() );
}

///////////////////////////////////////////////////////////////////////////////
// Returns the TUID of the selected file.  
// 
tuid PromptNewExistingDlg::GetFileID() const
{
  tuid fileID = TUID::Null;
  std::string filePath = GetFilePath();
  if ( !filePath.empty() )
  {
    fileID = File::GlobalManager().GetID( filePath );

    if ( fileID == TUID::Null && m_RequiresTuid )
    {
      try
      {
        fileID = File::GlobalManager().Open( filePath );
      }
      catch ( const Nocturnal::Exception& e )
      {
        std::string msg( "Unable to assign a TUID to path " );
        msg += filePath + "\n" + e.Get();
        wxMessageBox( msg.c_str(), "Error", wxCENTER | wxICON_ERROR | wxOK );
      }
    }
  }
  return fileID;
}

///////////////////////////////////////////////////////////////////////////////
// Returns the path (either a newly created one or an existing one) chosen by
// the user.  Should be called after the dialog successfully completes.
// 
std::string PromptNewExistingDlg::GetFilePath() const
{
  std::string filePath;
  if ( m_Panel->m_RadioBtnNew->GetValue() )
  {
    filePath = m_Panel->m_FilePathNew->GetValue().c_str();
  }
  else
  {
    filePath = m_Panel->m_FilePathExisting->GetValue().c_str();
  }

  // get and clean user input
  FileSystem::CleanName( filePath );
  return filePath;
}

///////////////////////////////////////////////////////////////////////////////
// Enables or disables various pieces of the UI based upon the current radio
// button selection.
// 
void PromptNewExistingDlg::UpdateEnableState()
{
  m_Panel->m_FilePathNew->Enable( m_Panel->m_RadioBtnNew->GetValue() );
  m_Panel->m_FilePathExisting->Enable( m_Panel->m_RadioBtnExisting->GetValue() );
  m_Panel->m_ButtonExisting->Enable( m_Panel->m_RadioBtnExisting->GetValue() );
  m_Panel->m_ButtonExistingFinder->Enable( m_Panel->m_RadioBtnExisting->GetValue() );
}

///////////////////////////////////////////////////////////////////////////////
// Callback for when a radio button is selected.  Updates the UI.
// 
void PromptNewExistingDlg::OnRadioButtonSelected( wxCommandEvent& args )
{
  UpdateEnableState();
  args.Skip();
}

///////////////////////////////////////////////////////////////////////////////
// Callback for when the user clicks the button to browse for an existing file.
// Prompts the user for the file to use.
// 
void PromptNewExistingDlg::OnButtonExistingClicked( wxCommandEvent& args )
{
  std::string directory;
  std::string file;
  if ( !m_Panel->m_FilePathExisting->GetValue().IsEmpty() )
  {
    // get and clean user input
    directory = m_Panel->m_FilePathExisting->GetValue().c_str();
    FileSystem::CleanName( directory );
    if ( FileSystem::IsFile( directory ) )
    {
      file = FileSystem::GetLeaf( directory );
      FileSystem::StripLeaf( directory );
    }
  }

  File::ManagedFileDialog dlg( this, "Open", directory.c_str(), file.c_str(), "", UIToolKit::FileDialogStyles::DefaultOpen );
  if ( m_FinderSpec )
  {
    dlg.SetFilter( m_FinderSpec->GetDialogFilter() );
  }
  dlg.SetTuidRequired( m_RequiresTuid );

  if ( dlg.ShowModal() == wxID_OK )
  {
    m_Panel->m_FilePathExisting->SetValue( dlg.GetPath() );
  }
}

///////////////////////////////////////////////////////////////////////////////
// Callback for when the user clicks the buton to browse for an existing file
// using the Asset Finder.
// 
void PromptNewExistingDlg::OnButtonExistingFinderClicked( wxCommandEvent& args )
{
  File::FileBrowser dlg( this, wxID_ANY, "Asset Finder" );
  if ( m_FinderSpec )
  {
    dlg.SetFilter( *m_FinderSpec );
  }
  dlg.SetTuidRequired( m_RequiresTuid );

  if ( dlg.ShowModal() == wxID_OK )
  {
    m_Panel->m_FilePathExisting->SetValue( dlg.GetPath() );
  }
}