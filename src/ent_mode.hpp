#ifndef ENT_MODE_HPP
#define ENT_MODE_HPP

#include "app.hpp"
#include "ent.hpp"

#define TEXT_FIELD_MAX 512

class EntMode : public App::ModeImpl 
{
public:
    EntMode();
    virtual ~EntMode() override;
    virtual void Update() override;
    virtual void Draw() override;
    virtual void OnEnter() override;
    virtual void OnExit() override;

    inline const Ent &GetEnt() const { return _ent; }
    inline void SetEnt(const Ent &ent) { _ent = ent; }

    inline bool IsChangeConfirmed() const { return _changeConfirmed; }
protected:
    Ent _ent;
    Vector2 _propsScroll;
    //Indicates which properties are having their values edited.
    std::map<std::string, bool> _propEditing;
    std::map<std::string, char*> _propBuffers; //In order to work with RayGUI text boxes, each value needs a modifiable char buffer.
    char _keyName[TEXT_FIELD_MAX];
    bool _keyNameEdit;

    bool _changeConfirmed;
};

#endif