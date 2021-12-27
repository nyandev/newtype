#pragma once
#include "newtype.h"

namespace newtype {

  class FontImpl;

  class ManagerImpl: public Manager {
    friend class FontImpl;
    friend class TextImpl;
  private:
    Host* host_;
    FT_MemoryRec_ ftMemAllocator_;
    FT_Library freeType_ = nullptr;
    struct FreeTypeVersion {
      FT_Int major;
      FT_Int minor;
      FT_Int patch;
      FT_TrueTypeEngineType trueTypeSupport;
    } ftVersion_ = { 0 };
    struct HarfbuzzVersion {
      unsigned int major;
      unsigned int minor;
      unsigned int patch;
    } hbVersion_ = { 0 };
    string verstr_;
    FontVector fonts_;
    IDType fontIndex_ = 0;
    IDType textIndex_ = 0;
  protected:
    inline FT_Library ft() { return freeType_; }
  public:
    ManagerImpl( Host* host );
    inline Host* host() { return host_; }
    bool initialize();
    void shutdown();
    ~ManagerImpl();
    // Font overrides
    FontPtr createFont() override;
    FontFacePtr loadFace( FontPtr font, span<uint8_t> buffer, FaceID faceIndex, Real size ) override;
    StyleID loadStyle( FontFacePtr face, FontRendering rendering, Real thickness ) override;
    void unloadFont( FontPtr font ) override;
    // Text overrides
    TextPtr createText( FontFacePtr face, StyleID style ) override;
    // Other overrides
    const string& versionString() const override;
    FontVector& fonts() override;
  };

}