#include "pch.h"
#include "newtype_text.h"
#include "newtype_font.h"
#include "newtype_manager.h"

namespace newtype {

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

  TextImpl::TextImpl( ManagerImpl* manager, const Text::Features& features ): manager_( manager )
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

  void TextImpl::setTextUTF16( uint16_t* data, uint32_t length )
  {
    if ( !data || length == 0 )
    {
      textdata_.reset();
      return;
    }
    auto bytelength = length * sizeof( uint16_t );
    textdata_ = make_unique<Buffer>( manager_->host(), bytelength );
    memcpy( textdata_->data(), reinterpret_cast<uint8_t*>( data ), bytelength );
  }

  void TextImpl::regenerate()
  {
    auto fnt = FONT_IMPL_CAST( font_ );

    if ( !dirty_ || !fnt || !fnt->loaded() )
      return;

    // TODO handle special case where textdata doesn't exist (= generate empty mesh)

    hb_buffer_reset( hbbuf_ );

    hb_buffer_set_direction( hbbuf_, direction_ );
    hb_buffer_set_script( hbbuf_, script_ );
    hb_buffer_set_language( hbbuf_, language_ );

    uint32_t flags = hb_buffer_get_flags( hbbuf_ );
    flags |= HB_BUFFER_FLAG_BOT;
    flags |= HB_BUFFER_FLAG_EOT;
    hb_buffer_set_flags( hbbuf_, static_cast<hb_buffer_flags_t>( flags ) );

    hb_shape(
      fnt->hbfnt_,
      hbbuf_,
      features_.empty() ? nullptr : features_.data(), static_cast<int>( features_.size() )
    );

    auto position = pen_;

    const auto ascender = font_->ascender();
    const auto descender = font_->descender();

    position.y += ascender + descender;

    unsigned int glyphCount;
    auto info = hb_buffer_get_glyph_infos( hbbuf_, &glyphCount );
    auto gpos = hb_buffer_get_glyph_positions( hbbuf_, &glyphCount );

    for ( unsigned int i = 0; i < glyphCount; ++i )
    {
      auto codepoint = text_.charAt( i );
      auto glyphindex = info[i].codepoint;
      const auto chartype = u_charType( codepoint );
      // Locator::console().printf( Console::srcGfx, "index %i char %i glyph %i chartype %i cluster %i", i, codepoint, glyphindex, chartype, info[i].cluster );
      if ( chartype == U_CONTROL_CHAR && glyphindex == 0 )
      {
        position.x = pen_.x;
        position.y += ( font_->ascender() - font_->descender() + font_->linegap_ );
        continue;
      }
      auto glyph = font_->getGlyph( glyphindex );
      auto offset = vec2( gpos[i].x_offset, gpos[i].y_offset ) / 64.0f;
      auto advance = vec2( gpos[i].x_advance, gpos[i].y_advance ) / 64.0f;
      auto p0 = vec2(
        ( position.x + offset.x + glyph->bearing.x ),
        math::ifloor( position.y - offset.y - glyph->bearing.y ) );
      auto p1 = vec2(
        ( p0.x + glyph->width ),
        (int)( p0.y + glyph->height ) );
      auto index = (GLuint)mesh_->vertsCount();
      auto normal = vec3( 1.0f, 0.0f, 0.0f );
      auto color = vec4( 1.0f, 1.0f, 1.0f, 1.0f );
      vector<Vertex3D> vertices = {
        { vec3( p0.x, p0.y, 0.0f ), normal, vec2( glyph->coords[0].x, glyph->coords[0].y ), color },
        { vec3( p0.x, p1.y, 0.0f ), normal, vec2( glyph->coords[0].x, glyph->coords[1].y ), color },
        { vec3( p1.x, p1.y, 0.0f ), normal, vec2( glyph->coords[1].x, glyph->coords[1].y ), color },
        { vec3( p1.x, p0.y, 0.0f ), normal, vec2( glyph->coords[1].x, glyph->coords[0].y ), color } };
      vector<GLuint> indices = {
        index + 0, index + 1, index + 2, index + 0, index + 2, index + 3 };
      mesh_->pushIndices( move( indices ) );
      mesh_->pushVertices( move( vertices ) );
      position += advance;
    }

    dirty_ = false;
  }

  TextImpl::~TextImpl()
  {
    if ( hbbuf_ )
      hb_buffer_destroy( hbbuf_ );
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

  FontPtr TextImpl::font()
  {
    return font_;
  }

}