#include "pch.h"
#include "newtype_font.h"
#include "newtype_manager.h"
#include "newtype_utils.h"

namespace newtype {

  constexpr int c_dpi = 72;

  Font::~Font()
  {
    //
  }

  FontImpl::FontImpl( ManagerImpl* manager ): manager_( manager )
  {
    //
  }

  void FontImpl::load( span<uint8_t> source, Real pointSize, vec3i atlasSize )
  {
    atlas_ = make_shared<TextureAtlas>( atlasSize.x, atlasSize.y, atlasSize.z );

    data_ = make_unique<Buffer>( manager_->host(), source);
    size_ = pointSize;

    auto ftlib = manager_->ft();

    FT_Open_Args args = { 0 };
    args.flags = FT_OPEN_MEMORY;
    args.memory_base = data_->data();
    args.memory_size = (FT_Long)data_->length();

    auto fterr = FT_Open_Face( ftlib, &args, 0, &face_ );
    if ( fterr || !face_ )
      NEWTYPE_FREETYPE_EXCEPT( "FreeType font face load failed", fterr );

    forceUCS2Charmap();

    fterr = FT_Select_Charmap( face_, FT_ENCODING_UNICODE );
    if ( fterr )
      NEWTYPE_FREETYPE_EXCEPT( "FreeType font charmap selection failed", fterr );

    fterr = FT_Set_Char_Size( face_, 0, iround( size_ * 64.0f ), c_dpi, c_dpi );
    if ( fterr )
      NEWTYPE_FREETYPE_EXCEPT( "FreeType font character point size setting failed", fterr );

    FT_Matrix matrix = {
      (int)( ( 1.0 ) * 0x10000L ),
      (int)( ( 0.0 ) * 0x10000L ),
      (int)( ( 0.0 ) * 0x10000L ),
      (int)( ( 1.0 ) * 0x10000L ) };

    FT_Set_Transform( face_, &matrix, nullptr );

    hbfnt_ = hb_ft_font_create_referenced( face_ );

    postLoad();
    initEmptyGlyph();
  }

  void FontImpl::forceUCS2Charmap()
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

  void FontImpl::postLoad()
  {
    auto metrics = face_->size->metrics;
    ascender_ = static_cast<Real>( metrics.ascender >> 6 );
    descender_ = static_cast<Real>( metrics.descender >> 6 );
    size_ = static_cast<Real>( metrics.height >> 6 );
    linegap_ = 0.0f;
  }

  void FontImpl::initEmptyGlyph()
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

    atlas_->setRegion( region.x, region.y, 4, 4, data, 0 );

    Glyph glyph;
    glyph.index = 0;
    glyph.coords[0].x = ( region.x + 2 ) / (Real)atlas_->width_;
    glyph.coords[0].y = ( region.y + 2 ) / (Real)atlas_->height_;
    glyph.coords[1].x = ( region.x + 3 ) / (Real)atlas_->width_;
    glyph.coords[1].y = ( region.y + 3 ) / (Real)atlas_->height_;

    glyphs_[0] = move( glyph );
  }

  void FontImpl::loadGlyph( GlyphIndex index, bool hinting )
  {
    auto ft = manager_->ft();

    FT_Int32 flags = 0;
    flags |= FT_LOAD_RENDER;
    flags |= ( hinting ? FT_LOAD_FORCE_AUTOHINT : ( FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT ) );

    if ( atlas_->depth_ == 3 )
    {
      FT_Library_SetLcdFilter( ft, FT_LCD_FILTER_DEFAULT );
      flags |= FT_LOAD_TARGET_LCD;
      uint8_t weights[5] = {0x10, 0x40, 0x70, 0x40, 0x10};
      FT_Library_SetLcdFilterWeights( ft, weights );
    }

    auto fterr = FT_Load_Glyph( face_, index, flags );
    if ( fterr )
      NEWTYPE_FREETYPE_EXCEPT( "FreeType glyph loading error", fterr );

    FT_Glyph ftglyph = nullptr;
    FT_GlyphSlot slot = nullptr;
    FT_Bitmap bitmap;
    vec2i glyphCoords;

    slot = face_->glyph;
    bitmap = slot->bitmap;
    glyphCoords.x = slot->bitmap_left;
    glyphCoords.y = slot->bitmap_top;

    vec4i padding( 0, 0, 0, 0 );

    uint32_t src_w = static_cast<uint32_t>( bitmap.width / atlas_->depth_ );
    uint32_t src_h = static_cast<uint32_t>( bitmap.rows );
    uint32_t tgt_w = src_w + static_cast<uint32_t>( padding.x + padding.z );
    uint32_t tgt_h = src_h + static_cast<uint32_t>( padding.y + padding.w );

    auto region = atlas_->getRegion( tgt_w + 1, tgt_h + 1 );
    if ( region.x < 0 )
      NEWTYPE_EXCEPT( "Font face texture atlas is full" );

    auto coord = vec2i( region.x, region.y );
    {
      Buffer tmp( manager_->host(), static_cast<uint32_t>( tgt_w * tgt_h * atlas_->depth_ ) );
      auto dst_ptr = tmp.data() + ( padding.y * tgt_w + padding.x ) * atlas_->depth_;
      auto src_ptr = bitmap.buffer;
      for ( uint32_t i = 0; i < src_h; ++i )
      {
        memcpy( dst_ptr, src_ptr, bitmap.width );
        dst_ptr += tgt_w * atlas_->depth_;
        src_ptr += bitmap.pitch;
      }

      atlas_->setRegion( coord.x, coord.y, tgt_w, tgt_h, tmp.data(), tgt_w * atlas_->depth_ );
    }

    Glyph glyph;
    glyph.index = index;
    glyph.width = tgt_w;
    glyph.height = tgt_h;
    glyph.bearing = glyphCoords;
    glyph.coords[0].x = coord.x / (Real)atlas_->width_;
    glyph.coords[0].y = coord.y / (Real)atlas_->height_;
    glyph.coords[1].x = ( coord.x + glyph.width ) / (Real)atlas_->width_;
    glyph.coords[1].y = ( coord.y + glyph.height ) / (Real)atlas_->height_;

    glyphs_[index] = move( glyph );

    if ( ftglyph )
      FT_Done_Glyph( ftglyph );
  }

  Glyph* FontImpl::getGlyph( GlyphIndex index )
  {
    {
      const auto& glyph = glyphs_.find( index );
      if ( glyph != glyphs_.end() )
        return &( ( *glyph ).second );
    }
    loadGlyph( index, true );
    {
      const auto& glyph = glyphs_.find( index );
      if ( glyph != glyphs_.end() )
        return &( ( *glyph ).second );
    }
    return nullptr;
  }

  void FontImpl::unload()
  {
    if ( hbfnt_ )
      hb_font_destroy( hbfnt_ );
    if ( face_ )
      FT_Done_Face( face_ );
  }

  Real FontImpl::size() const
  {
    return size_;
  }

  Real FontImpl::ascender() const
  {
    return ascender_;
  }

  Real FontImpl::descender() const
  {
    return descender_;
  }

  bool FontImpl::loaded() const
  {
    return loaded_;
  }

}