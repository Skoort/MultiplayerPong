#include "GameObject.hpp"

inline sf::Vector2f const &GameObject::GetPosition() const
{
    return m_graphics.getPosition();
}

inline void GameObject::SetPosition(sf::Vector2f position)
{
    m_graphics.setPosition(position);
}

inline void GameObject::SetPosition(float x, float y)
{
    m_graphics.setPosition({x, y});
}

inline sf::Vector2f const &GameObject::GetDimensions() const
{
    return m_graphics.getSize();
}

inline void GameObject::SetDimensions(sf::Vector2f dimensions)
{
    m_graphics.setSize(dimensions);
}

inline void GameObject::SetDimensions(float x, float y)
{
    m_graphics.setSize({x, y});
}

inline sf::Vector2f const &GameObject::GetScale() const
{
    return m_graphics.getScale();
}

inline void GameObject::SetScale(sf::Vector2f scale)
{
    m_graphics.setScale(scale);
}

inline void GameObject::SetScale(float x, float y)
{
    m_graphics.setScale({x, y});
}

inline sf::Vector2f const &GameObject::GetOrigin() const
{
    return m_graphics.getOrigin();
}

inline void GameObject::SetOrigin(sf::Vector2f origin)
{
    m_graphics.setOrigin(origin);
}

inline sf::Vector2f GameObject::GetCenter() const
{
    auto bounds = m_graphics.getGlobalBounds();
    return { bounds.getPosition().x + bounds.width * 0.5F, bounds.getPosition().y + bounds.height * 0.5F };
}

inline void GameObject::SetOrigin(float x, float y)
{
    m_graphics.setOrigin({x, y});
}

inline sf::Vector2f const &GameObject::GetVelocity() const
{
    return m_velocity;
}

inline void GameObject::SetVelocity(sf::Vector2f velocity)
{
    m_velocity = velocity;
}

inline void GameObject::SetVelocity(float x, float y)
{
    m_velocity = {x, y};
}
