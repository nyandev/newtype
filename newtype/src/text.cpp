#include "pch.h"
#include "newtype_text.h"
#include "newtype_font.h"
#include "newtype_manager.h"

namespace newtype {

  Text::~Text()
  {
    //
  }

  namespace features {

    const hb_tag_t KernTag = HB_TAG( 'k', 'e', 'r', 'n' ); // kerning operations
    const hb_tag_t LigaTag = HB_TAG( 'l', 'i', 'g', 'a' ); // standard ligature substitution
    const hb_tag_t CligTag = HB_TAG( 'c', 'l', 'i', 'g' ); // contextual ligature substitution

    static hb_feature_t LigatureOff = { LigaTag, 0, 0, std::numeric_limits<unsigned int>::max() };
    static hb_feature_t LigatureOn = { LigaTag, 1, 0, std::numeric_limits<unsigned int>::max() };
    static hb_feature_t KerningOff = { KernTag, 0, 0, std::numeric_limits<unsigned int>::max() };
    static hb_feature_t KerningOn = { KernTag, 1, 0, std::numeric_limits<unsigned int>::max() };
    static hb_feature_t CligOff = { CligTag, 0, 0, std::numeric_limits<unsigned int>::max() };
    static hb_feature_t CligOn = { CligTag, 1, 0, std::numeric_limits<unsigned int>::max() };

  }

  TextImpl::TextImpl( ManagerImpl* manager, IDType id, FontFacePtr face, StyleID style, const Text::Features& features ):
  manager_( manager ), id_( id ), face_( move( face ) ), style_( style )
  {
    hbbuf_ = hb_buffer_create();
    string lang = "en";
    language_ = hb_language_from_string( lang.c_str(), static_cast<int>( lang.size() ) );
    script_ = HB_SCRIPT_LATIN;
    direction_ = HB_DIRECTION_LTR;

    features_.push_back( features.kerning ? features::KerningOn : features::KerningOff );
    features_.push_back( features.ligatures ? features::LigatureOn : features::LigatureOff );
    features_.push_back( features.ligatures ? features::CligOn : features::CligOff );
  }

  TextImpl::~TextImpl()
  {
    if ( hbbuf_ )
      hb_buffer_destroy( hbbuf_ );
  }

  IDType TextImpl::id() const
  {
    return id_;
  }

  void TextImpl::setText( const unicodeString& text )
  {
    if ( text_ != text )
    {
      text_ = text;
      dirty_ = true;
    }
  }

  void TextImpl::regenerate()
  {
    auto fce = FONTFACE_IMPL_CAST( face_ );

    if ( !dirty_ || !fce || !fce->font_->loaded() )
      return;

    FontStyleImpl* style = nullptr;
    {
      auto pimpl = fce->getStyle( style_ );
      style = FONTSTYLE_IMPL_CAST( pimpl );
      if ( !style )
        NEWTYPE_EXCEPT( "Style implementation cast failed" );
    }

    // TODO handle special case where textdata doesn't exist (= generate empty mesh)

    hb_buffer_reset( hbbuf_ );

    hb_buffer_set_direction( hbbuf_, direction_ );
    hb_buffer_set_script( hbbuf_, script_ );
    hb_buffer_set_language( hbbuf_, language_ );

    uint32_t flags = hb_buffer_get_flags( hbbuf_ );
    flags |= HB_BUFFER_FLAG_BOT;
    flags |= HB_BUFFER_FLAG_EOT;
    hb_buffer_set_flags( hbbuf_, static_cast<hb_buffer_flags_t>( flags ) );

    hb_buffer_add_utf16( hbbuf_, reinterpret_cast<const uint16_t*>( text_.getBuffer() ), text_.length(), 0, text_.length() );

    hb_shape(
      fce->hbfnt_,
      hbbuf_,
      features_.empty() ? nullptr : features_.data(), static_cast<int>( features_.size() )
    );

    auto position = pen_;

    const auto ascender = fce->ascender();
    const auto descender = fce->descender();

    position.y += ascender + descender;

    unsigned int glyphCount;
    auto info = hb_buffer_get_glyph_infos( hbbuf_, &glyphCount );
    auto gpos = hb_buffer_get_glyph_positions( hbbuf_, &glyphCount );

    mesh_.vertices_.clear();
    mesh_.indices_.clear();

    for ( unsigned int i = 0; i < glyphCount; ++i )
    {
      auto codepoint = text_.charAt( i );
      auto glyphindex = info[i].codepoint;
      const auto chartype = u_charType( codepoint );
      if ( chartype == U_CONTROL_CHAR && glyphindex == 0 )
      {
        position.x = pen_.x;
        position.y += ( fce->ascender() - fce->descender() );
        continue;
      }
      auto glyph = style->getGlyph( manager_->ft(), fce->face_, glyphindex );
      auto offset = vec2( gpos[i].x_offset, gpos[i].y_offset ) / 64.0f;
      auto advance = vec2( gpos[i].x_advance, gpos[i].y_advance ) / 64.0f;

      auto p0 = vec2(
        ( position.x + offset.x + glyph->bearing.x ),
        ifloor( position.y - offset.y - glyph->bearing.y ) );

      auto p1 = vec2(
        ( p0.x + glyph->width ),
        (int)( p0.y + glyph->height ) );

      auto color = vec4( 1.0f, 1.0f, 1.0f, 1.0f );

      auto index = static_cast<VertexIndex>( mesh_.vertices_.size() );
      mesh_.vertices_.emplace_back( vec3( p0.x, p0.y, position.z ), vec2( glyph->coords[0].x, glyph->coords[0].y ), color );
      mesh_.vertices_.emplace_back( vec3( p0.x, p1.y, position.z ), vec2( glyph->coords[0].x, glyph->coords[1].y ), color );
      mesh_.vertices_.emplace_back( vec3( p1.x, p1.y, position.z ), vec2( glyph->coords[1].x, glyph->coords[1].y ), color );
      mesh_.vertices_.emplace_back( vec3( p1.x, p0.y, position.z ), vec2( glyph->coords[1].x, glyph->coords[0].y ), color );

      Indices idcs = { index + 0, index + 1, index + 2, index + 0, index + 2, index + 3 };
      mesh_.indices_.insert( mesh_.indices_.end(), idcs.begin(), idcs.end() );

      position += vec3( advance, 0.0f );
    }

    dirty_ = false;
  }

  void TextImpl::update()
  {
    regenerate();
  }

  const Mesh& TextImpl::mesh() const
  {
    return mesh_;
  }

  vec3 TextImpl::pen() const
  {
    return pen_;
  }

  void TextImpl::pen( const vec3& pen )
  {
    if ( epsilonCompare( pen, pen_ ) )
      return;
    pen_ = pen;
    dirty_ = true;
  }

  bool TextImpl::dirty() const
  {
    return dirty_;
  }

  FontFacePtr TextImpl::face()
  {
    return face_;
  }

  StyleID TextImpl::styleid() const
  {
    return style_;
  }

  void TextImpl::setUser( void* data )
  {
    userdata_ = data;
  }

  void* TextImpl::getUser()
  {
    return userdata_;
  }

}