#pragma once
#include "newtype.h"
#include "newtype_utils.h"

namespace newtype {

  class ManagerImpl;
  class TextImpl;

  struct TextureAtlas {
    vector<vec3i> nodes_;
    size_t width_;
    size_t height_;
    size_t depth_;
    size_t used_;
    vector<uint8_t> data_;
    TextureAtlas( const size_t width, const size_t height, const size_t depth );
    void setRegion( const size_t x, const size_t y, const size_t width, const size_t height, const uint8_t* data, const size_t stride );
    int fit( const size_t index, const size_t width, const size_t height );
    void merge();
    vec4i getRegion( const size_t width, const size_t height );
    void clear();
  };

  using TextureAtlasPtr = shared_ptr<TextureAtlas>;

  class FontImpl: public Font {
    friend class ManagerImpl;
    friend class TextImpl;
  private:
    Real size_ = 0.0f;
    Real ascender_ = 0.0f;
    Real descender_ = 0.0f;
    Real linegap_ = 0.0f;
    bool loaded_ = false;
  private:
    ManagerImpl* manager_;
    FT_Face face_ = nullptr;
    hb_font_t* hbfnt_ = nullptr;
    GlyphMap glyphs_;
    TextureAtlasPtr atlas_;
    unique_ptr<Buffer> data_;
    void load( span<uint8_t> source, Real pointSize, vec3i atlasSize );
    void unload();
    void postLoad();
    void initEmptyGlyph();
    void forceUCS2Charmap();
  protected:
    void loadGlyph( GlyphIndex index, bool hinting );
    Glyph* getGlyph( GlyphIndex index );
  public:
    FontImpl( ManagerImpl* manager );
    virtual Real size() const;
    virtual Real ascender() const;
    virtual Real descender() const;
    virtual bool loaded() const;

  };

}