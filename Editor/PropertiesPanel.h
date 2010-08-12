#pragma once

#include "Editor/EditorGenerated.h"

#include "Core/Scene/PropertiesManager.h"

namespace Helium
{
    namespace Editor
    {
        class PropertiesPanel : public PropertiesPanelGenerated
        {
        public:
            PropertiesPanel( Core::PropertiesManager* manager, wxWindow* parent = NULL, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,300 ), long style = wxTAB_TRAVERSAL );

            Inspect::CanvasWindow* GetPropertiesCanvas()
            {
                return m_PropertyCanvas;
            }
        private:
            Core::PropertiesManager* m_PropertiesManager;
            Inspect::CanvasWindow* m_PropertyCanvas;

            void OnIntersection(wxCommandEvent& event);
            void OnUnion(wxCommandEvent& event);
            void OnExpandAll(wxCommandEvent& event);
            void OnCollapseAll(wxCommandEvent& event);
        };
    }
}