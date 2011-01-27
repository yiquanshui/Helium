#pragma once

#include "Indent.h"
#include "Archive.h"

struct XML_ParserStruct;
typedef struct XML_ParserStruct *XML_Parser;

namespace Helium
{
    namespace Reflect
    {
        //
        // XML Archive Class
        //

        class FOUNDATION_API ArchiveXML : public Archive
        {
        public: 
            static const uint32_t CURRENT_VERSION; 

        private:
            friend class Archive;

            class ParsingState : public Helium::RefCountBase<ParsingState>
            {
            public:
                // the name of the name being processed
                tstring m_Name;

                // the cdata section for xml files
                tstring m_Buffer;

                // the current serializing field
                const Field* m_Field;

                // the item being processed
                ObjectPtr m_Object;

                // The collected components
                std::vector< ObjectPtr > m_Components;

                // flags, as specified below
                unsigned int m_Flags;

                enum ProcessFlag
                {
                    kField  = 0x1 << 0,
                };

                ParsingState(const tchar_t* name)
                    : m_Name (name)
                    , m_Field (NULL) 
                    , m_Flags (0)
                {

                }

                void SetFlag( ProcessFlag flag, bool state )
                {
                    if ( state )
                        m_Flags |= flag;
                    else
                        m_Flags &= ~flag;
                }

                bool GetFlag( ProcessFlag flag )
                {
                    return ((m_Flags & flag) != 0x0);
                }
            };

            typedef Helium::SmartPtr<ParsingState> ParsingStatePtr;

            // The expat parser object
            XML_Parser m_Parser;

            // The stream to use
            TCharStreamPtr m_Stream;

            // Indent helper
            Indent<tchar_t> m_Indent;

            // File format version
            uint32_t m_Version;

            // The nesting stack of parsing state
            std::stack< ParsingStatePtr > m_StateStack;

            // The current name of the serializing field
            std::stack< const tchar_t* > m_FieldNames;

            // The current collection of components
            std::vector< ObjectPtr > m_Components;

            // The container to decode elements to
            std::vector< ObjectPtr >* m_Target;

        public:
            ArchiveXML( const Path& path, ByteOrder byteOrder = Helium::PlatformByteOrder );
            ~ArchiveXML();

        private:
            ArchiveXML();

        public:
            // Stream access
            TCharStream& GetStream()
            {
                return *m_Stream;
            }

        protected:
            // The type
            virtual ArchiveType GetType() const
            {
                return ArchiveTypes::XML;
            }

            virtual void Open( bool write = false) HELIUM_OVERRIDE;
            void OpenStream(TCharStream* stream, bool write = false);
            virtual void Close(); 

            // Begins parsing the InputStream
            virtual void Read();

            // Write to the OutputStream
            virtual void Write();

        public:
            // Access indentation
            Indent<tchar_t>& GetIndent()
            {
                return m_Indent;
            }

        public:
            // Serialize
            virtual void Serialize( Object* object );
            virtual void Serialize( void* structure, const Structure* type );
            virtual void Serialize( const std::vector< ObjectPtr >& elements, uint32_t flags = 0 );
            virtual void Serialize( const DynArray< ObjectPtr >& elements, uint32_t flags = 0 );

        protected:
            // Helpers
            template< typename ConstIteratorType > void Serialize( ConstIteratorType begin, ConstIteratorType end, uint32_t flags );
            void SerializeFields(Object* object);
            void SerializeField(Object* object, const Field* field);

            // <Object> and </Object>
            void SerializeHeader(Object* object);
            void SerializeFooter(Object* object);

        public:
            // For handling components
            virtual void Deserialize( ObjectPtr& object );
            virtual void Deserialize( void* structure, const Structure* type );
            virtual void Deserialize( std::vector< ObjectPtr >& elements, uint32_t flags = 0 );
            virtual void Deserialize( DynArray< ObjectPtr >& elements, uint32_t flags = 0 );

        private:
            static void StartElementHandler(void *pUserData, const tchar_t* pszName, const tchar_t **papszAttrs)
            {
                ArchiveXML *archive = (ArchiveXML *)pUserData;
                archive->OnStartElement(pszName, papszAttrs);
            }

            static void EndElementHandler(void *pUserData, const tchar_t* pszName)
            {
                ArchiveXML *archive = (ArchiveXML *)pUserData;
                archive->OnEndElement(pszName);
            }

            static void CharacterDataHandler(void *pUserData, const tchar_t* pszData, int nLength)
            {
                ArchiveXML *archive = (ArchiveXML *)pUserData;
                archive->OnCharacterData(pszData, nLength);
            }

            // Called on <object>
            void OnStartElement(const tchar_t *pszName, const tchar_t **papszAttrs);

            // Called between <object> and </object>
            void OnCharacterData(const tchar_t *pszData, int nLength);

            // Called after </object>
            void OnEndElement(const tchar_t *pszName);

        public:
            // Reading and writing single object from string data
            static void       ToString( Object* object, tstring& xml );
            static ObjectPtr FromString( const tstring& xml, const Class* searchClass = NULL );

            // Reading and writing multiple elements from string data
            static void       ToString( const std::vector< ObjectPtr >& elements, tstring& xml );
            static void       FromString( const tstring& xml, std::vector< ObjectPtr >& elements );
        };
    }
}