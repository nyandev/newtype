#pragma once
#include "newtype.h"
#include "newtype_utils.h"

namespace newtype {

  class ManagerImpl;
  class FontImpl;
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

#pragma pack( push, 1 )
  union FontStyleIndex {
    struct FontStyleData {
      uint16_t face;
      uint32_t size;
      uint8_t outlineType;
      uint8_t outlineSize;
    } components;
    StyleID value;
  };
#pragma pack( pop )

  class FontStyleImpl: public FontStyle {
    friend class ManagerImpl;
    friend class TextImpl;
  private:
    FontImpl* font_;
    uint32_t storedFaceSize_;
    Host* host_;
    FontRendering rendering_;
    Real outlineThickness_;
    TextureAtlasPtr atlas_;
    GlyphMap glyphs_;
    bool dirty_ = false;
  protected:
    void initEmptyGlyph();
    void loadGlyph( FT_Library ft, FT_Face face, GlyphIndex index, bool hinting );
  public:
    FontStyleImpl( FontImpl* font, uint32_t size, vec2i atlasSize, Host* host, FontRendering rendering, Real thickness );
    StyleID id() const;
    Glyph* getGlyph( FT_Library ft, FT_Face face, GlyphIndex index );
    bool dirty() const override;
    void markClean() override;
    const Texture& texture() const override;
    ~FontStyleImpl();
  };

  using FontStyleMap = map<StyleID, FontStylePtr>;

  class FontFaceImpl: public FontFace {
    friend class ManagerImpl;
    friend class FontStyleImpl;
    friend class TextImpl;
  private:
    FontImpl* font_;
    FT_Face face_ = nullptr;
    hb_font_t* hbfnt_ = nullptr;
    Real size_ = 0.0f;
    Real ascender_ = 0.0f;
    Real descender_ = 0.0f;
    FontStyleMap styles_;
    StyleID loadStyle( FontRendering rendering, Real thickness );
  protected:
    void forceUCS2Charmap();
    void postLoad();
    FontStylePtr getStyle( StyleID id );
  public:
    FontFaceImpl( FontImpl* font, FT_Library ft, FT_Open_Args* args, FaceID faceIndex, Real size );
    Real size() const override;
    Real ascender() const override;
    Real descender() const override;
    ~FontFaceImpl();
  };

  using FontFaceMap = map<FaceID, FontFacePtr>;

  class FontImpl: public Font {
    friend class ManagerImpl;
    friend class FontFaceImpl;
    friend class TextImpl;
  private:
    bool loaded_ = false;
    void* userdata_ = nullptr;
  private:
    ManagerImpl* manager_;
    FontFaceMap faces_;
    unique_ptr<Buffer> data_;
    IDType id_;
    FontFacePtr loadFace( span<uint8_t> source, FaceID faceIndex, Real size );
    void unload();
  protected:
    void update(); // this will recreate the texture if needed
  public:
    FontImpl( ManagerImpl* manager, IDType id );
    FontLoadState loaded() const override;
    void setUser( void* data ) override;
    void* getUser() override;
    IDType id() const override;
  };

}