//
// Created by dingrui on 3/3/26.
//

#pragma once

class Scene;
class Timestep;
class Entity;

class ScriptableEntity
{
public:
    virtual ~ScriptableEntity() {}

    template <typename T>
    T& GetComponent()
    {
        return m_entity.GetComponent<T>();
    }

protected:
    virtual void OnCreate() {}
    virtual void OnUpdate(Timestep ts) {}
    virtual void OnDestroy() {}

private:
    Entity m_entity;
    friend class Scene;
};
