//
// Created by dingrui on 3/3/26.
//

#pragma once

#include "x/scene/entity.h"

class Scene;

class ScriptableEntity
{
public:
    template <typename T>
    T& GetComponent()
    {
        return m_entity.GetComponent<T>();
    }

private:
    Entity m_entity;
    friend class Scene;
};
