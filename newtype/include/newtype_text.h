#pragma once
#include "newtype.h"
#include "newtype_utils.h"

namespace newtype {

  class ManagerImpl;

  class TextImpl: public Text {
  private:
    ManagerImpl* manager_;
    vec3 pen_;
    hb_language_t language_;
    hb_script_t script_;
    hb_direction_t direction_;
    bool dirty_ = false;
    hb_buffer_t* hbbuf_ = nullptr;
    FontPtr font_;
    vector<hb_feature_t> features_;
    unique_ptr<Buffer> textdata_;
  public:
    TextImpl( ManagerImpl* manager, const Text::Features& features );
    virtual ~TextImpl();
    virtual void setTextUTF16( uint16_t* data, uint32_t length );
    virtual vec3 pen() const;
    virtual void pen( const vec3& pen );
    virtual bool dirty() const;
    virtual FontPtr font();
    void regenerate();
  };

}