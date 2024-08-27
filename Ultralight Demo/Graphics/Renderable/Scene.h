#pragma once
#include "PCH.h"
#include "Entity.h"

class Scene
{
public:
	vector<shared_ptr<Entity>> Entities;
};