#pragma once

// STL
#include <exception>
#include <memory>
#include <vector>
#include <list>
#include <map>
#include <string>
#include <cstdint>
#include <algorithm>
#include <utility>
#include <span>

#undef min
#undef max

// GLM
#define GLM_FORCE_XYZW_ONLY // Hide rgba, stpq unions from vector types (avoiding bloat when examined in the debugger)
#define GLM_FORCE_SIZE_T_LENGTH // Make size() on GLM types return size_t instead of int (for STL compatibility)
#define GLM_FORCE_UNRESTRICTED_GENTYPE // Allow operation input types that strict GLSL wouldn't (such as int)
#define GLM_ENABLE_EXPERIMENTAL // Enable experimental new features
#if !defined( NEWTYPE_VERBOSE ) && !defined( _DEBUG )
#define GLM_FORCE_SILENT_WARNINGS
#endif
#ifndef _DEBUG
#define GLM_FORCE_INLINE // Force inlining in release build
#endif
#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/transform.hpp>

// ICU
#include <unicode/utypes.h>
#include <unicode/uchar.h>
#include <unicode/locid.h>
#include <unicode/ustring.h>
#include <unicode/ucnv.h>
#include <unicode/unistr.h>
#include <unicode/utf8.h>
#include <unicode/utf16.h>

#ifdef NEWTYPE_EXPORTS

// FreeType2
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_MODULE_H
#include FT_MODULE_ERRORS_H
#include FT_SIZES_H
#include FT_OUTLINE_H
#include FT_RENDER_H
#include FT_DRIVER_H
#include FT_STROKER_H
#include FT_GLYPH_H
#include FT_BITMAP_H
#include FT_BBOX_H
#include FT_CACHE_H
#include FT_LCD_FILTER_H
#include FT_TRUETYPE_IDS_H
#include FT_TRUETYPE_TABLES_H
#include FT_TRUETYPE_TAGS_H

// Harfbuzz
#include <hb.h>
#include <hb-ft.h>
#include <hb-icu.h>

#endif

namespace newtype {

  using std::uint8_t;
  using std::uint16_t;
  using std::uint32_t;
  using std::uint64_t;

  using std::string;
  using std::wstring;
  using std::stringstream;
  using std::wstringstream;
  using std::string_view;

  using std::array;
  using std::vector;
  using std::list;
  using std::map;
  using std::make_shared;
  using std::shared_ptr;
  using std::make_unique;
  using std::unique_ptr;

  using std::span;

  using unicodeString = icu::UnicodeString;
  using unicodePiece = icu::StringPiece;

  using std::move;
  using std::pair;
  using std::make_pair;
  using std::initializer_list;

  using std::make_shared;
  using std::make_unique;
  using std::enable_shared_from_this;
  using std::move;
  using std::string_view;

  using std::numeric_limits;

  using Real = float;

  using vec2 = glm::fvec2;
  using vec3 = glm::fvec3;
  using vec4 = glm::fvec4;
  using mat2 = glm::fmat2x2;
  using mat3 = glm::fmat3x3;
  using mat4 = glm::fmat4x4;
  using quaternion = glm::fquat;

  using vec2i = glm::i64vec2;
  using vec3i = glm::i64vec3;
  using vec4i = glm::i64vec4;

  using vec2u = glm::u64vec2;
  using vec3u = glm::u64vec3;
  using vec4u = glm::u64vec4;

}