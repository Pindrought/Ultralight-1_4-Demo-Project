#pragma once
#include "PCH.h"
#include "Entity.h"

class Scene //Most basic scene class imaginable
{
public:
	vector<shared_ptr<Entity>> Entities;
};