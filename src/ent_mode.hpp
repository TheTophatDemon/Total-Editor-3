/**
 * Copyright (c) 2022-present Alexander Lunsford
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 **/

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
protected:
    Ent _ent;
    
    char _texturePathBuffer[TEXT_FIELD_MAX];
    char _modelPathBuffer[TEXT_FIELD_MAX];

    char _newKeyBuffer[TEXT_FIELD_MAX];
    char _newValBuffer[TEXT_FIELD_MAX];

    // Character buffers for each property's value (indexed by the key)
    std::map<std::string, char*> _valBuffers;
};

#endif
