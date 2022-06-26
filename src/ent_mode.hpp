/*
Copyright (C) 2022 Alexander Lunsford
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

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