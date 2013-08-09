/**
    * The EntityFactory creates Entities through Lua config scripts
    *
    * Author: Skylar Payne
    * Date: 8/9/2013
    * File: EntityFactory.cpp
**/

#include "EntityFactory.h"
#include <lua.hpp>
#include "Entity.h"
#include "IListener.h"
#include "ScriptableBehavior.h"

/**
 * @brief EntityFactory::Register allows the use of lambda functions to register component types so that entities can be constructed
 * @param componentType the type of component (same string referenced in Lua!)
 * @param constructor the constructor of the component of componentType
 * @return true if the constructor is registered, false otherwise
 */
bool EntityFactory::Register(const char *componentType, std::function<IComponent*()> constructor)
{
    ConstructorMap::iterator it = _ConstructorMap.find(componentType);

    if(it != _ConstructorMap.end())
    {
        return true;
    }

    _ConstructorMap[componentType] = constructor;
    return true;
}

/**
 * @brief EntityFactory::Create creates an entity from a Lua script
 * @param entity the file from which to load the entity
 */
void EntityFactory::Create(const char* entity, float x, float y)
{
    Entity* e = new Entity();
    IComponent* c = nullptr;
    lua_State* L = luaL_newstate();
    luaL_dofile(L, entity);
    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    lua_setglobal(L, "y");
    lua_setglobal(L, "x");
    lua_getglobal(L, "Components");
    lua_pushnil(L);

    while(lua_next(L, 1) != 0)
    {
        c = _ConstructorMap[lua_tostring(L, -2)]();
        c->Load(L);
        e->AttachComponent(c);
        lua_pop(L, 1);
    }

    lua_settop(L, 0);
    lua_getglobal(L, "Behaviors");
    lua_pushnil(L);
    ScriptableBehavior* b = nullptr;

    while(lua_next(L, 1) != 0)
    {
        b = new ScriptableBehavior(lua_tostring(L, -2), lua_tostring(L, -1));
        e->AttachBehavior(b);
    }

    lua_close(L);

    AddEntityMessage msg;
    msg.entity = e;
    Emit<AddEntityMessage>(msg);
}
