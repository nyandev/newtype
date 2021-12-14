#pragma once
#include "newtype.h"

namespace newtype {

#define FONT_IMPL_CAST(font) ( font ? dynamic_cast<FontImpl*>( font.get() ) : nullptr )
#define TEXT_IMPL_CAST(text) ( text ? dynamic_cast<TextImpl*>( text.get() ) : nullptr )

  inline int iround( Real value )
  {
    return static_cast<int>( ::roundf( value ) );
  }

  inline int ifloor( Real value )
  {
    return static_cast<int>( ::floorf( value ) );
  }

  inline bool epsilonCompare( const vec3& a, const vec3& b )
  {
    return glm::all( glm::epsilonEqual( a, b, vec3( 0.01f ) ) );
  }

  class Buffer {
  private:
    Host* host_;
    uint8_t* buffer_ = nullptr;
    uint32_t length_ = 0;
  public:
    Buffer( Host* host, uint32_t length ): host_( host ), length_( length )
    {
      buffer_ = static_cast<uint8_t*>( host->newtypeMemoryAllocate( length_ ) );
      assert( buffer_ );
    }
    Buffer( Host* host, const span<uint8_t> source ): host_( host )
    {
      length_ = static_cast<uint32_t>( source.size() );
      buffer_ = static_cast<uint8_t*>( host->newtypeMemoryAllocate( length_ ) );
      assert( buffer_ );
      memcpy( buffer_, source.data(), length_ );
    }
    ~Buffer()
    {
      if ( buffer_ )
        host_->newtypeMemoryFree( buffer_ );
    }
    inline const uint32_t length() const { return length_; }
    inline uint8_t* data() { return buffer_; }
  };

}