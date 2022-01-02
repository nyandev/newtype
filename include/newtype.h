#pragma once

#include "newtype_types.h"

#define NEWTYPE_CALL __stdcall

#ifdef NEWTYPE_EXPORTS
# define NEWTYPE_EXPORT __declspec( dllexport )
#else
# define NEWTYPE_EXPORT __declspec( dllimport )
#endif

namespace newtype {

  constexpr int c_dpi = 72;

  using IDType = uint64_t;

  using Codepoint = uint32_t;
  using GlyphIndex = uint32_t;

  using StyleID = uint64_t;

  // actually aliases FT_Long, but we don't
  // want to reference any FreeType dependencies
  // in the public API
  using FaceID = signed long;

  struct Glyph
  {
    GlyphIndex index = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    vec2i bearing;
    vec2 coords[2];
  };

  using GlyphMap = map<GlyphIndex, Glyph>;

#pragma pack( push, 1 )
  struct Vertex {
    vec3 position;
    vec2 texcoord;
    vec4 color;
    Vertex( vec3 pos, vec2 tc, vec4 clr ): position( move( pos ) ), texcoord( move( tc ) ), color( move( clr ) ) {}
  };
#pragma pack( pop )

  using VertexIndex = unsigned int;

  using Vertices = vector<Vertex>;
  using Indices = vector<VertexIndex>;

  enum TextureFormat {
    TextureFormat_R8,
    TextureFormat_RGB8,
    TextureFormat_RGBA8
  };

  class Texture {
  public:
    virtual TextureFormat format() const = 0;
    virtual vec2i dimensions() const = 0;
    virtual const uint8_t* data() const = 0;
    virtual int bytesize() const = 0;
    virtual bool dirty() const = 0;
    virtual void markClean() = 0;
  };

  class Mesh {
  public:
    Vertices vertices_;
    Indices indices_;
    bool dirty_ = true;
  };

  using MeshPtr = shared_ptr<Mesh>;

  class Font;

  class Host {
  public:
    virtual void* newtypeMemoryAllocate( uint32_t size ) = 0;
    virtual void* newtypeMemoryReallocate( void* address, uint32_t newSize ) = 0;
    virtual void newtypeMemoryFree( void* address ) = 0;
    virtual void newtypeFontTextureCreated( Font& font, StyleID style, Texture& texture ) = 0;
    virtual void newtypeFontTextureDestroyed( Font& font, StyleID style, Texture& texture ) = 0;
  };

  enum FontLoadState {
    FontLoad_NotReady = 0,
    FontLoad_Degraded,
    FontLoad_Loaded
  };

  enum FontRendering {
    FontRender_Normal = 0,
    FontRender_Outline_Expand
  };

  class FontStyle {
  public:
    virtual ~FontStyle();
    virtual bool dirty() const = 0;
    virtual const Texture& texture() const = 0;
    virtual void markClean() = 0;
  };

  using FontStylePtr = shared_ptr<FontStyle>;

  class FontFace {
  public:
    virtual ~FontFace();
    virtual Real size() const = 0;
    virtual Real ascender() const = 0;
    virtual Real descender() const = 0;
    virtual FontStylePtr getStyle( StyleID id ) = 0;
  };

  using FontFacePtr = shared_ptr<FontFace>;

  class Font {
  public:
    virtual ~Font();
    virtual FontLoadState loaded() const = 0;
    virtual void setUser( void* data ) = 0;
    virtual void* getUser() = 0;
    virtual IDType id() const = 0;
  };

  using FontPtr = shared_ptr<Font>;
  using FontVector = vector<FontPtr>;

  class Text {
  public:
    struct Features {
      bool ligatures : 1;
      bool kerning : 1;
    };
  public:
    virtual ~Text();
    virtual void setText( const unicodeString& text ) = 0;
    virtual void update() = 0;
    virtual const Mesh& mesh() const = 0;
    virtual vec3 pen() const = 0;
    virtual void pen( const vec3& pen ) = 0;
    virtual bool dirty() const = 0;
    virtual FontFacePtr face() = 0;
    virtual StyleID styleid() const = 0;
    virtual void setUser( void* data ) = 0;
    virtual void* getUser() = 0;
    virtual IDType id() const = 0;
  };

  using TextPtr = shared_ptr<Text>;

  class Manager {
  public:
    // Font
    virtual FontPtr createFont() = 0;
    virtual FontFacePtr loadFace( FontPtr font, span<uint8_t> buffer, FaceID faceIndex, Real size ) = 0;
    virtual StyleID loadStyle( FontFacePtr face, FontRendering rendering, Real thickness ) = 0;
    virtual void unloadFont( FontPtr font ) = 0;
    // Text
    virtual TextPtr createText( FontFacePtr face, StyleID style ) = 0;
    virtual FontVector& fonts() = 0;
    virtual const string& versionString() const = 0;
  };

  using fnNewtypeInitialize = Manager* ( NEWTYPE_CALL* )( uint32_t version, Host* host );
  using fnNewtypeShutdown = void( NEWTYPE_CALL* )( Manager* manager );

#ifdef NEWTYPE_EXPORTS
#if defined(NEWTYPE_EXCEPT)
# error NEWTYPE_EXCEPT* macro already defined!
#else
# define NEWTYPE_EXCEPT(description) {throw std::exception(description);}
# define NEWTYPE_FREETYPE_EXCEPT(description,retval) {throw std::exception(description);}
#endif
#endif

}