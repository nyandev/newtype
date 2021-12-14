#pragma once
#include "newtype.h"

namespace newtype {

  class FontImpl;

  class ManagerImpl: public Manager {
    friend class FontImpl;
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
    FontVector fonts_;
    IDType fontIndex_ = 0;
    IDType textIndex_ = 0;
  protected:
    inline FT_Library ft() { return freeType_; }
  public:
    ManagerImpl( Host* host );
    inline Host* host() { return host_; }
    bool initialize();
    FontPtr createFont() override;
    virtual void loadFont( FontPtr font, span<uint8_t> buffer, Real size );
    void unloadFont( FontPtr font ) override;
    TextPtr createText( FontPtr font ) override;
    virtual FontVector& fonts();
    void shutdown();
    ~ManagerImpl();
  };

}