#include "pch.h"
#include "newtype_font.h"

namespace newtype {

  TextureAtlas::TextureAtlas( const vec2i& size, int depth ):
  size_( size ), depth_( depth ), used_( 0 ), dirty_( true )
  {
    assert( depth == 1 || depth == 3 || depth == 4 );

    nodes_.emplace_back( 1, 1, size_.x - 2 );
    data_.resize( size_.x * size_.y * depth_ );
    memset( data_.data(), 0, data_.size() );
  }

  TextureFormat TextureAtlas::format() const
  {
    if ( depth_ == 1 )
      return TextureFormat_R8;
    if ( depth_ == 3 )
      return TextureFormat_RGB8;
    return TextureFormat_RGBA8;
  }

  vec2i TextureAtlas::dimensions() const
  {
    return size_;
  }

  const uint8_t* TextureAtlas::data() const
  {
    return data_.data();
  }

  int TextureAtlas::bytesize() const
  {
    return static_cast<int>( data_.size() );
  }

  bool TextureAtlas::dirty() const
  {
    return dirty_;
  }

  void TextureAtlas::markClean()
  {
    dirty_ = false;
  }

  void TextureAtlas::setRegion( int x, int y, uint32_t width, uint32_t height, const uint8_t* data, size_t stride )
  {
    assert( x > 0 && y > 0 && ( x < ( size_.x - 1 ) ) && ( y < ( size_.y - 1 ) ) );
    assert( ( x + width ) <= ( size_.x - 1 ) && ( y + height ) <= ( size_.y - 1 ) );

    assert( height == 0 || ( data && width > 0 ) );

    for ( size_t i = 0; i < height; ++i )
    {
      memcpy( data_.data() + ( ( y + i ) * size_.x + x ) * depth_,
        data + ( i * stride ), width * depth_ );
    }

    dirty_ = true;
  }

  int TextureAtlas::fit( size_t index, uint32_t width, uint32_t height )
  {
    auto node = &nodes_[index];
    auto x = node->x;
    auto y = node->y;
    auto width_left = (int64_t)width;
    auto i = index;

    if ( ( x + width ) > ( size_.x - 1 ) )
      return -1;

    while ( width_left > 0 )
    {
      node = &nodes_[i];
      if ( node->y > y )
        y = node->y;
      if ( ( y + height ) > ( size_.y - 1 ) )
        return -1;
      width_left -= node->z;
      ++i;
    }

    return (int)y;
  }

  void TextureAtlas::merge()
  {
    size_t i = 0;
    for ( i = 0; i < nodes_.size() - 1; ++i )
    {
      auto node = &nodes_[i];
      auto next = &nodes_[i + 1];
      if ( node->y == next->y )
      {
        node->z += next->z;
        nodes_.erase( nodes_.begin() + ( i + 1 ) );
        --i;
      }
    }
  }

  vec4i TextureAtlas::getRegion( uint32_t width, uint32_t height )
  {
    int best_index = -1;
    uint32_t best_height = numeric_limits<uint32_t>::max();
    uint32_t best_width = numeric_limits<uint32_t>::max();

    vec4i region;

    for ( uint32_t i = 0; i < nodes_.size(); ++i )
    {
      auto y = fit( i, width, height );
      if ( y >= 0 )
      {
        auto node = &nodes_[i];
        if ( ( ( y + height ) < best_height ) || ( ( ( y + height ) == best_height ) && ( node->z > 0 && (size_t)node->z < best_width ) ) )
        {
          best_height = ( y + height );
          best_index = (int)i;
          best_width = static_cast<uint32_t>( node->z );
          region.x = node->x;
          region.y = y;
        }
      }
    }

    if ( best_index == -1 )
    {
      region.x = -1;
      region.y = -1;
      region.z = 0;
      region.w = 0;
      return move( region );
    }

    vec3i node( region.x, region.y + height, width );
    nodes_.insert( nodes_.begin() + best_index, node );

    size_t i;
    for ( i = (size_t)best_index + 1; i < nodes_.size(); ++i )
    {
      auto node = &nodes_[i];
      auto prev = &nodes_[i - 1];
      if ( node->x < ( prev->x + prev->z ) )
      {
        auto shrink = prev->x + prev->z - node->x;
        node->x += shrink;
        node->z -= shrink;
        if ( node->z <= 0 )
        {
          nodes_.erase( nodes_.begin() + i );
          --i;
        }
        else
          break;
      }
      else
        break;
    }

    merge();
    used_ += ( width * height );
    return move( region );
  }

  void TextureAtlas::clear()
  {
    vec3i node( 1 );
    nodes_.clear();
    used_ = 0;
    node.z = size_.x - 2;
    nodes_.push_back( move( node ) );
    memset( data_.data(), 0, size_.x * size_.y * depth_ );
    dirty_ = true;
  }

}