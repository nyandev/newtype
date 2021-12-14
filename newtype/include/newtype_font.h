#pragma once
#include "newtype.h"
#include "newtype_utils.h"

namespace newtype {

  class ManagerImpl;
  class TextImpl;

  class TextureAtlas: public Texture {
  private:
    vector<vec3i> nodes_;
    vec2i size_;
    int depth_;
    size_t used_;
    vector<uint8_t> data_;
    bool dirty_;
  public:
    TextureAtlas( const vec2i& size, int depth );
    void setRegion( int x, int y, uint32_t width, uint32_t height, const uint8_t* data, size_t stride );
    int fit( size_t index, uint32_t width, uint32_t height );
    void merge();
    vec4i getRegion( uint32_t width, uint32_t height );
    void clear();
  public:
    inline int depth() const noexcept { return depth_; }
    inline uint8_t* data() { return data_.data(); }
    inline vec2 fdimensions() const { return vec2( static_cast<Real>( size_.x ), static_cast<Real>( size_.y ) ); }
    TextureFormat format() const override;
    vec2i dimensions() const override;
    const uint8_t* data() const override;
    int bytesize() const override;
    bool dirty() const override;
    void markClean() override;
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
    void* userdata_ = nullptr;
  private:
    ManagerImpl* manager_;
    FT_Face face_ = nullptr;
    hb_font_t* hbfnt_ = nullptr;
    GlyphMap glyphs_;
    TextureAtlasPtr atlas_;
    unique_ptr<Buffer> data_;
    IDType id_;
    bool dirty_ = false;
    void load( span<uint8_t> source, Real pointSize, vec2i atlasSize );
    void unload();
    void postLoad();
    void initEmptyGlyph();
    void forceUCS2Charmap();
  protected:
    void loadGlyph( GlyphIndex index, bool hinting );
    Glyph* getGlyph( GlyphIndex index );
    void update(); // this will recreate the texture if needed
  public:
    FontImpl( ManagerImpl* manager, IDType id );
    Real size() const override;
    Real ascender() const override;
    Real descender() const override;
    bool loaded() const override;
    bool dirty() const override;
    void markClean() override;
    const Texture& texture() const override;
    void setUser( void* data ) override;
    void* getUser() override;
    IDType id() const override;
  };

}