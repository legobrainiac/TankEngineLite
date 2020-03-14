#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "Transform.h"
#include "SceneObject.h"

class Texture2D;
class GameObject
	: public SceneObject
{
public:
	void Update() override;
	void Render() const override;

	void SetTexture(const std::string& filename);
	void SetPosition(float x, float y);

	GameObject() = default;
	virtual ~GameObject();
    
	GameObject(const GameObject& other) = delete;
	GameObject(GameObject&& other) = delete;
	GameObject& operator=(const GameObject& other) = delete;
	GameObject& operator=(GameObject&& other) = delete;

private:
	Transform m_Transform;
	Texture2D* m_pTexture;
};

#endif // !GAMEOBJECT_H