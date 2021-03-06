#pragma once
#include "newtype.h"
#include "newtype_utils.h"

namespace newtype {

  class ManagerImpl;

  class TextImpl: public Text {
  private:
    ManagerImpl* manager_;
    vec3 pen_ = vec3( 0.0f );
    hb_language_t language_;
    hb_script_t script_;
    hb_direction_t direction_;
    bool dirty_ = false;
    hb_buffer_t* hbbuf_ = nullptr;
    FontFacePtr face_;
    StyleID style_;
    vector<hb_feature_t> features_;
    unicodeString text_;
    Mesh mesh_;
    void* userdata_ = nullptr;
    IDType id_;
  public:
    TextImpl( ManagerImpl* manager, IDType id, FontFacePtr face, StyleID style, const Text::Features& features );
    virtual ~TextImpl();
    void setText( const unicodeString& text ) override;
    void update() override;
    const Mesh& mesh() const override;
    vec3 pen() const override;
    void pen( const vec3& pen ) override;
    bool dirty() const override;
    FontFacePtr face() override;
    StyleID styleid() const override;
    void regenerate();
    void setUser( void* data ) override;
    void* getUser() override;
    IDType id() const override;
  };

}