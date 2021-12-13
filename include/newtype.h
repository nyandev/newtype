#pragma once

#include "newtype_types.h"

#define NEWTYPE_CALL __stdcall

#ifdef NEWTYPE_EXPORTS
# define NEWTYPE_EXPORT __declspec( dllexport )
#else
# define NEWTYPE_EXPORT __declspec( dllimport )
#endif

namespace newtype {

  using Codepoint = uint32_t;
  using GlyphIndex = uint32_t;

  struct Glyph
  {
    GlyphIndex index = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    vec2i bearing;
    vec2 coords[2];
  };

  using GlyphMap = map<GlyphIndex, Glyph>;

  struct Vertex {
    vec3 position;
    vec2 texcoord;
    vec4 color;
  };

  using VertexIndex = unsigned int;

  using Vertices = vector<Vertex>;
  using Indices = vector<VertexIndex>;

  class Mesh {
    Vertices vertices_;
    Indices indices_;
    bool dirty_ = true;
  };

  using MeshPtr = shared_ptr<Mesh>;

  class Host {
  public:
    virtual void* newtypeMemoryAllocate( uint32_t size ) = 0;
    virtual void* newtypeMemoryReallocate( void* address, uint32_t newSize ) = 0;
    virtual void newtypeMemoryFree( void* address ) = 0;
  };

  class Font {
    struct Specs {
      vec2i atlasSize_;
      Real pointSize_;
    };
  public:
    virtual ~Font() = 0;
    virtual Real size() const = 0;
    virtual Real ascender() const = 0;
    virtual Real descender() const = 0;
    virtual bool loaded() const = 0;
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
    virtual void setTextUTF16( uint16_t* data, uint32_t length ) = 0;
    virtual vec3 pen() const = 0;
    virtual void pen( const vec3& pen ) = 0;
    virtual bool dirty() const = 0;
    virtual FontPtr font() = 0;
    virtual ~Text();
  };

  using TextPtr = shared_ptr<Text>;

  class Manager {
  public:
    virtual FontPtr createFont() = 0;
    virtual void loadFont( FontPtr font, span<uint8_t> buffer, Real size ) = 0;
    virtual void unloadFont( FontPtr font ) = 0;
    virtual TextPtr createText( FontPtr font ) = 0;
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