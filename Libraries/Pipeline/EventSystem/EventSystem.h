#pragma once

#include <vector>

#include "Pipeline/API.h"
#include "EventSystemEvent.h"
#include "EventSystemExceptions.h"

#include "Platform/Types.h"
#include "Foundation/File/Path.h"
#include "Foundation/Memory/SmartPtr.h"

#include "Foundation/TUID.h"

namespace Nocturnal
{
    namespace ES
    {
        class EventSystem;
        typedef PIPELINE_API Nocturnal::SmartPtr< EventSystem > EventSystemPtr;

        /////////////////////////////////////////////////////////////////////////////
        // This distributed event system manages the creation and handling of unique events.
        // Assigns each event a TUID and maintains a list of already handled events.
        //
        class PIPELINE_API EventSystem : public Nocturnal::RefCountBase< EventSystem >
        {
        public:
            EventSystem( const std::string& rootDirPath, bool writeBinaryFormat );
            ~EventSystem();

            EventPtr CreateEvent( const std::string& eventData, const std::string& username );

            void GetUnhandledEvents( V_EventPtr& listOfEvents );
            void GetUnhandledEvents( V_EventPtr& listOfEvents, S_tuid& handledEventIDs );
            void ReadEventsFile( const std::string& eventsFilename, V_EventPtr& listOfEvents, bool sorted = false );

            void CreateEventsFilePath( std::string& userEventsFile );
            void WriteEventsFile( const std::string& eventsFilename, const V_EventPtr& listOfEvents );

            void WriteHandledEvents( const V_EventPtr& listOfEvents );
            void FlushHandledEvents();
            bool HandleEventsFileExists();

            void DumpEventsToTextFile( const std::string& datFile = std::string( "" ), const std::string& txtFile = std::string( "" ) );
            void LoadEventsFromTextFile( const std::string& txtFile = std::string( "" ), const std::string& datFile = std::string( "" ) );
            void StompEventsFile( const std::string& eventsFile, const V_EventPtr& listOfEvents );

        private:
            Nocturnal::Path m_RootDirPath;
            Nocturnal::Path m_HandledEventsFile;
            bool m_WriteBinaryFormat;

            void GetEvents( V_EventPtr& listOfEvents, bool sorted = false );

            void ReadBinaryEventsFile( const std::string& eventsFilename, V_EventPtr& listOfEvents, bool sorted = false );
            void ReadTextEventsFile( const std::string& eventsFilename, V_EventPtr& listOfEvents, bool sorted = false );

            void WriteBinaryEventsFile( const std::string& eventsFilename, const V_EventPtr& listOfEvents );
            void WriteTextEventsFile( const std::string& eventsFilename, const V_EventPtr& listOfEvents );
        };
    }
}