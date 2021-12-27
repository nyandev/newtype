#include "pch.h"
#include "newtype_manager.h"
#include "newtype_font.h"
#include "newtype_text.h"

namespace newtype {

  void* ftMemoryAllocate( FT_Memory memory, long size )
  {
    auto me = reinterpret_cast<ManagerImpl*>( memory->user );
    return ( me ? me->host()->newtypeMemoryAllocate( size ) : nullptr );
  }

  void* ftMemoryReallocate( FT_Memory memory, long currentSize, long newSize, void* block )
  {
    auto me = reinterpret_cast<ManagerImpl*>( memory->user );
    return ( me ? me->host()->newtypeMemoryReallocate( block, newSize ) : nullptr );
  }

  void ftMemoryFree( FT_Memory memory, void* block )
  {
    auto me = reinterpret_cast<ManagerImpl*>( memory->user );
    if ( me )
      me->host()->newtypeMemoryFree( block );
  }

  ManagerImpl::ManagerImpl( Host* host ): host_( host )
  {
    ftMemAllocator_.user = this;
    ftMemAllocator_.alloc = ftMemoryAllocate;
    ftMemAllocator_.realloc = ftMemoryReallocate;
    ftMemAllocator_.free = ftMemoryFree;
  }

  bool ManagerImpl::initialize()
  {
    assert( !freeType_ );

    auto fterr = FT_New_Library( &ftMemAllocator_, &freeType_ );
    if ( fterr )
      NEWTYPE_FREETYPE_EXCEPT( "FreeType library creation failed", fterr );

    FT_Add_Default_Modules( freeType_ );

    FT_Library_Version( freeType_, &ftVersion_.major, &ftVersion_.minor, &ftVersion_.patch );
    hb_version( &hbVersion_.major, &hbVersion_.minor, &hbVersion_.patch );

    hb_icu_get_unicode_funcs();

    ftVersion_.trueTypeSupport = FT_Get_TrueType_Engine_Type( freeType_ );

    char tmp[128];
    sprintf_s( tmp, 128, "FreeType v%i.%i.%i HarfBuzz v%i.%i.%i",
      ftVersion_.major, ftVersion_.minor, ftVersion_.patch,
      hbVersion_.major, hbVersion_.minor, hbVersion_.patch );
    verstr_ = tmp;

    return true;
  }

  FontPtr ManagerImpl::createFont()
  {
    auto font = make_shared<FontImpl>( this, fontIndex_++ );
    fonts_.push_back( font );

    return move( font );
  }

  FontFacePtr ManagerImpl::loadFace( FontPtr font, span<uint8_t> buffer, FaceID faceIndex, Real size )
  {
    auto fnt = FONT_IMPL_CAST( font );
    if ( !fnt )
      NEWTYPE_EXCEPT( "Font implementation cast failed" );
    return fnt->loadFace( buffer, faceIndex, size );
  }

  StyleID ManagerImpl::loadStyle( FontFacePtr face, FontRendering rendering, Real thickness )
  {
    auto fce = FONTFACE_IMPL_CAST( face );
    if ( !fce )
      NEWTYPE_EXCEPT( "FontFace implementation cast failed" );
    return fce->loadStyle( rendering, thickness );
  }

  void ManagerImpl::unloadFont( FontPtr font )
  {
    auto fnt = FONT_IMPL_CAST( font );
    if ( !fnt )
      NEWTYPE_EXCEPT( "Font implementation cast failed" );
    fnt->unload();
  }

  TextPtr ManagerImpl::createText( FontFacePtr face, StyleID style )
  {
    Text::Features feats;
    feats.kerning = true;
    feats.ligatures = true;
    auto text = make_shared<TextImpl>( this, textIndex_++, face, style, feats );
    return text;
  }

  FontVector& ManagerImpl::fonts()
  {
    return fonts_;
  }

  void ManagerImpl::shutdown()
  {
    if ( freeType_ )
    {
      FT_Done_Library( freeType_ );
      freeType_ = nullptr;
    }
  }

  const string& ManagerImpl::versionString() const
  {
    return verstr_;
  }

  ManagerImpl::~ManagerImpl()
  {
    //
  }

}