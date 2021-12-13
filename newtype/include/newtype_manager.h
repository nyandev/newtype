#pragma once
#include "newtype.h"

namespace newtype {

  class FontImpl;

  class ManagerImpl: public Manager {
    friend class FontImpl;
  private:
    Host* host_ = nullptr;
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
  protected:
    inline FT_Library ft() { return freeType_; }
  public:
    ManagerImpl( Host* host );
    inline Host* host() { return host_; }
    bool initialize();
    virtual FontPtr createFont();
    virtual void loadFont( FontPtr font, span<uint8_t> buffer, Real size );
    virtual void unloadFont( FontPtr font );
    virtual TextPtr createText( FontPtr font );
    void shutdown();
    ~ManagerImpl();
  };

}