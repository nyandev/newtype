#include "pch.h"
#include "newtype_font.h"
#include "newtype_manager.h"
#include "newtype_utils.h"

namespace newtype {

  Font::~Font()
  {
    //
  }

  FontStyle::~FontStyle()
  {
    //
  }

  FontFace::~FontFace()
  {
    //
  }

  constexpr float c_fmagic = 64.0f;
  constexpr int c_magic = 64;

  constexpr uint32_t makeStoredFaceSize( Real size )
  {
    return static_cast<uint32_t>( size * 1000.0f );
  }

  __forceinline StyleID makeStyleID( FaceID face, uint32_t size, FontRendering rendering, Real thickness )
  {
    FontStyleIndex d;
    d.components.face = ( face & 0xFFFF ); // no instance/variations support
    d.components.outlineType = rendering;
    d.components.outlineSize = ( rendering == FontRender_Normal ? 0 : static_cast<uint8_t>( thickness * 10.0f ) );
    d.components.size = size;
    return d.value;
  }

  __forceinline StyleID makeStyleID( FaceID face, Real size, FontRendering rendering, Real thickness )
  {
    return makeStyleID( face, makeStoredFaceSize( size ), rendering, thickness );
  }

  // FONT FACE ===============================================================

  FontFaceImpl::FontFaceImpl( FontImpl* font, FT_Library ft, FT_Open_Args* args, FaceID faceIndex, Real size ):
  font_( font ), size_( size )
  {
    auto fterr = FT_Open_Face( ft, args, faceIndex, &face_ );
    if ( fterr || !face_ )
      NEWTYPE_FREETYPE_EXCEPT( "FreeType font face load failed", fterr );

    forceUCS2Charmap();

    fterr = FT_Select_Charmap( face_, FT_ENCODING_UNICODE );
    if ( fterr )
      NEWTYPE_FREETYPE_EXCEPT( "FreeType font charmap selection failed", fterr );

    fterr = FT_Set_Char_Size( face_, 0, iround( size_ * c_fmagic ), c_dpi, c_dpi );
    if ( fterr )
      NEWTYPE_FREETYPE_EXCEPT( "FreeType font character point size setting failed", fterr );

    FT_Matrix matrix = {
      (int)( ( 1.0 ) * 0x10000L ),
      (int)( ( 0.0 ) * 0x10000L ),
      (int)( ( 0.0 ) * 0x10000L ),
      (int)( ( 1.0 ) * 0x10000L ) };

    FT_Set_Transform( face_, &matrix, nullptr );

    hbfnt_ = hb_ft_font_create_referenced( face_ );
    hb_ft_font_set_funcs( hbfnt_ ); // Doesn't create_referenced already call this?

    postLoad();
  }

  StyleID FontFaceImpl::loadStyle( FontRendering rendering, Real thickness )
  {
    auto id = makeStyleID( face_->face_index, size_, rendering, thickness );
    if ( styles_.find( id ) != styles_.end() )
      return id;

    auto atlasSize = vec2i( 1024 );

    auto style = make_shared<FontStyleImpl>( font_,
      face_->face_index,
      makeStoredFaceSize( size_ ),
      atlasSize, font_->manager_->host(),
      rendering, thickness );

    auto cmp = style->id();
    FontStyleIndex asdasdasd;
    asdasdasd.value = cmp;
    assert( asdasdasd.value == id );

    styles_[style->id()] = style;
    return style->id();
  }

  void FontFaceImpl::forceUCS2Charmap()
  {
    assert( face_ );

    for ( auto i = 0; i < face_->num_charmaps; ++i )
    {
      auto charmap = face_->charmaps[i];
      if ( ( charmap->platform_id == 0 && charmap->encoding_id == 3 )
        || ( charmap->platform_id == 3 && charmap->encoding_id == 1 ) )
        if ( FT_Set_Charmap( face_, charmap ) == 0 )
          return;
    }
  }

  void FontFaceImpl::postLoad()
  {
    auto metrics = face_->size->metrics;
    ascender_ = static_cast<Real>( metrics.ascender >> 6 );
    descender_ = static_cast<Real>( metrics.descender >> 6 );
    size_ = static_cast<Real>( metrics.height >> 6 );
  }

  Real FontFaceImpl::size() const
  {
    return size_;
  }

  Real FontFaceImpl::ascender() const
  {
    return ascender_;
  }

  Real FontFaceImpl::descender() const
  {
    return descender_;
  }

  FontStylePtr FontFaceImpl::getStyle( StyleID id )
  {
    auto it = styles_.find( id );
    if ( it == styles_.end() )
      NEWTYPE_EXCEPT( "Style not loaded" );
    return it->second;
  }

  FontFaceImpl::~FontFaceImpl()
  {
    styles_.clear();
    if ( hbfnt_ )
      hb_font_destroy( hbfnt_ );
  }

  // FONT STYLE ==============================================================

  FontStyleImpl::FontStyleImpl( FontImpl* font, FT_Long face, uint32_t size, vec2i atlasSize,
  Host* host, FontRendering rendering, Real thickness ):
  font_( font ), host_( host ), storedFaceSize_( size ), storedFaceIndex_( face ),
  rendering_( rendering ), outlineThickness_( thickness )
  {
    atlas_ = make_shared<TextureAtlas>( atlasSize, 1 );
    host_->newtypeFontTextureCreated( *font_, id(), *atlas_.get() );
    initEmptyGlyph();
  }

  void FontStyleImpl::initEmptyGlyph()
  {
    auto region = atlas_->getRegion( 5, 5 );
    if ( region.x < 0 )
      NEWTYPE_EXCEPT( "Font face texture atlas is full" );

#pragma warning( push )
#pragma warning( disable: 4838 )
    static unsigned char data[4 * 4 * 3] = {
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
    };
#pragma warning( pop )

    atlas_->setRegion( (int)region.x, (int)region.y, 4, 4, data, 0 );

    Glyph glyph;
    glyph.index = 0;
    glyph.coords[0] = vec2( region.x + 2, region.y + 2 ) / atlas_->fdimensions();
    glyph.coords[1] = vec2( region.x + 3, region.y + 3 ) / atlas_->fdimensions();

    glyphs_[0] = move( glyph );

    dirty_ = true;
  }

  void FontStyleImpl::loadGlyph( FT_Library ft, FT_Face face, GlyphIndex index, bool hinting )
  {
    FT_Int32 flags = 0;
    flags |= FT_LOAD_DEFAULT;
    flags |= ( hinting ? FT_LOAD_FORCE_AUTOHINT : ( FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT ) );

    if ( atlas_->depth() == 3 )
    {
      FT_Library_SetLcdFilter( ft, FT_LCD_FILTER_DEFAULT );
      flags |= FT_LOAD_TARGET_LCD;
      uint8_t weights[5] = { 0x10, 0x40, 0x70, 0x40, 0x10 };
      FT_Library_SetLcdFilterWeights( ft, weights );
    }

    auto fterr = FT_Load_Glyph( face, index, flags );
    if ( fterr )
      NEWTYPE_FREETYPE_EXCEPT( "FreeType glyph load error", fterr );

    FT_Bitmap bitmap;
    vec2i glyphCoords;

    if ( rendering_ == FontRender_Normal )
    {
      FT_GlyphSlot slot = face->glyph;
      fterr = FT_Render_Glyph( slot, FT_RENDER_MODE_NORMAL );
      if ( fterr )
        NEWTYPE_FREETYPE_EXCEPT( "FreeType glyph render error", fterr );
      bitmap = slot->bitmap;
      glyphCoords.x = slot->bitmap_left;
      glyphCoords.y = slot->bitmap_top;
    }
    else if ( rendering_ == FontRender_Outline_Expand )
    {
      FT_Stroker stroker;
      FT_Stroker_New( ft, &stroker );
      auto dist = static_cast<signed long>( outlineThickness_ * c_fmagic );
      FT_Stroker_Set( stroker, dist, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0 );

      FT_Glyph ftglyph;
      FT_Get_Glyph( face->glyph, &ftglyph );
      FT_Glyph_StrokeBorder( &ftglyph, stroker, false, true );
      FT_Glyph_To_Bitmap( &ftglyph, FT_RENDER_MODE_NORMAL, nullptr, true );
      auto bmglyph = reinterpret_cast<FT_BitmapGlyph>( ftglyph );
      bitmap = bmglyph->bitmap;
      glyphCoords.x = bmglyph->left;
      glyphCoords.y = bmglyph->top;
    }
    else
      NEWTYPE_EXCEPT( "Unknown rendering mode" );

    vec4i padding( 0, 0, 0, 0 );

    auto src_w = static_cast<uint32_t>( bitmap.width / atlas_->depth() );
    auto src_h = static_cast<uint32_t>( bitmap.rows );
    auto tgt_w = src_w + static_cast<uint32_t>( padding.x + padding.z );
    auto tgt_h = src_h + static_cast<uint32_t>( padding.y + padding.w );

    auto region = atlas_->getRegion( tgt_w + 1, tgt_h + 1 );
    if ( region.x < 0 )
      NEWTYPE_EXCEPT( "Font face texture atlas is full" );

    auto coord = vec2i( region.x, region.y );
    {
      Buffer tmp( host_, static_cast<uint32_t>( tgt_w * tgt_h * atlas_->depth() ) );
      auto dst_ptr = tmp.data() + ( padding.y * tgt_w + padding.x ) * atlas_->depth();
      auto src_ptr = bitmap.buffer;
      for ( uint32_t i = 0; i < src_h; ++i )
      {
        memcpy( dst_ptr, src_ptr, bitmap.width );
        dst_ptr += tgt_w * atlas_->depth();
        src_ptr += bitmap.pitch;
      }

      atlas_->setRegion( (int)coord.x, (int)coord.y, (int)tgt_w, (int)tgt_h, tmp.data(), (int)tgt_w * atlas_->depth() );
    }

    Glyph glyph;
    glyph.index = index;
    glyph.width = tgt_w;
    glyph.height = tgt_h;
    glyph.bearing = glyphCoords;
    glyph.coords[0] = vec2( coord ) / atlas_->fdimensions();
    glyph.coords[1] = vec2( coord.x + glyph.width, coord.y + glyph.height ) / atlas_->fdimensions();

    glyphs_[index] = move( glyph );

    dirty_ = true;
  }

  Glyph* FontStyleImpl::getGlyph( FT_Library ft, FT_Face face, GlyphIndex index )
  {
    {
      const auto& glyph = glyphs_.find( index );
      if ( glyph != glyphs_.end() )
        return &( ( *glyph ).second );
    }
    loadGlyph( ft, face, index, true );
    {
      const auto& glyph = glyphs_.find( index );
      if ( glyph != glyphs_.end() )
        return &( ( *glyph ).second );
    }
    return nullptr;
  }

  bool FontStyleImpl::dirty() const
  {
    return dirty_;
  }

  void FontStyleImpl::markClean()
  {
    dirty_ = false;
  }

  const Texture& FontStyleImpl::texture() const
  {
    assert( atlas_ );
    return *atlas_.get();
  }

  FontStyleImpl::~FontStyleImpl()
  {
    host_->newtypeFontTextureDestroyed( *font_, id(), *atlas_.get() );
    atlas_.reset();
  }

  StyleID FontStyleImpl::id() const
  {
    return makeStyleID( storedFaceIndex_, storedFaceSize_, rendering_, outlineThickness_ );
  }

  // FONT ====================================================================

  FontFacePtr FontImpl::loadFace( span<uint8_t> source, FaceID faceIndex, Real size )
  {
    // Make a safety copy in our own memory
    // Loading new styles on the fly later could still access this memory
    // and we won't rely on the host keeping that available and alive
    // Of course, if multiple faces are actually loaded from different blobs
    // despite belonging to the same font, this copy will only contain the
    // last loaded one - but that's pretty suspect behavior anyway, don't do it
    data_ = make_unique<Buffer>( manager_->host(), source );

    auto ftlib = manager_->ft();

    FT_Open_Args args = { 0 };
    args.flags = FT_OPEN_MEMORY;
    args.memory_base = data_->data();
    args.memory_size = (FT_Long)data_->length();

    auto face = make_shared<FontFaceImpl>( this, ftlib, &args, faceIndex, size );
    faces_[faceIndex] = face;

    loaded_ = true;

    return move( face );
  }

  void FontImpl::unload()
  {
    data_.reset();
    loaded_ = false;
  }

  FontLoadState FontImpl::loaded() const
  {
    return ( loaded_ ? FontLoad_Loaded : FontLoad_NotReady );
  }

  void FontImpl::setUser( void* data )
  {
    userdata_ = data;
  }

  void* FontImpl::getUser()
  {
    return userdata_;
  }

  FontImpl::FontImpl( ManagerImpl* manager, IDType id ): manager_( manager ), id_( id )
  {
    //
  }

  FontImpl::~FontImpl()
  {
    faces_.clear();
  }

  IDType FontImpl::id() const
  {
    return id_;
  }

}