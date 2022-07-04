#include "GameObject.hpp"

GameObject::GameObject()
: GameObject{{0.F, 0.F}, {0.F, 0.F}, {0.F, 0.F}}
{
}

GameObject::GameObject(sf::Vector2f position, sf::Vector2f dimensions, sf::Vector2f velocity)
{
    SetPosition(position);
    SetDimensions(dimensions);
    SetVelocity(velocity);
}

void GameObject::Draw(sf::RenderTarget &renderTarget) const
{
    renderTarget.draw(m_graphics);
}

void GameObject::Move(float dt)
{
    SetPosition(GetPosition() + (m_velocity * dt));
}

void GameObject::ClampPosition(float xMin, float xMax, float yMin, float yMax)
{
    auto clampedPosition = GetPosition();
    if (clampedPosition.x < xMin + GetOrigin().x)
    {
        clampedPosition.x = xMin + GetOrigin().x;
    } else
    if (clampedPosition.x > xMax - (GetDimensions().x - GetOrigin().x))
    {
        clampedPosition.x = xMax - (GetDimensions().x - GetOrigin().x);
    }
    if (clampedPosition.y < yMin + GetOrigin().y)
    {
        clampedPosition.y = yMin + GetOrigin().y;
    } else
    if (clampedPosition.y > yMax - (GetDimensions().y - GetOrigin().y))
    {
        clampedPosition.y = yMax - (GetDimensions().y - GetOrigin().y);
    }
    SetPosition(clampedPosition);
}

bool GameObject::Collides(GameObject const &other) const
{
    auto const pos1 = this->GetPosition();
    auto const pos2 = other.GetPosition();
    auto const size1 = this->GetDimensions();
    auto const size2 = other.GetDimensions();
    auto const origin1 = this->GetOrigin();
    auto const origin2 = other.GetOrigin();

    float const left1 = pos1.x - origin1.x;
    float const right1 = pos1.x + size1.x - origin1.x;
    float const top1 = pos1.y - origin1.y;
    float const bottom1 = pos1.y + size1.y - origin1.y;
    float const left2 = pos2.x - origin2.x;
    float const right2 = pos2.x + size2.x - origin2.x;
    float const top2 = pos2.y - origin2.y;
    float const bottom2 = pos2.y + size2.y - origin2.y;
    
    if (top1 > bottom2 || bottom1 < top2)
    {
        return false;
    }
    if (left1 > right2 || right1 < left2)
    {
        return false;
    }
    return true;
}

sf::Vector2f GameObject::GetCollisionPenetration(GameObject const &other) const
{
    auto const pos1 = this->GetPosition();
    auto const pos2 = other.GetPosition();
    auto const size1 = this->GetDimensions();
    auto const size2 = other.GetDimensions();
    auto const origin1 = this->GetOrigin();
    auto const origin2 = other.GetOrigin();
    
    float const left1 = pos1.x - origin1.x;
    float const right1 = pos1.x + size1.x - origin1.x;
    float const top1 = pos1.y - origin1.y;
    float const bottom1 = pos1.y + size1.y - origin1.y;
    float const left2 = pos2.x - origin2.x;
    float const right2 = pos2.x + size2.x - origin2.x;
    float const top2 = pos2.y - origin2.y;
    float const bottom2 = pos2.y + size2.y - origin2.y;
    
    float xDepth = std::min(std::abs(left1 - right2), std::abs(right1 - left2));
    float yDepth = std::min(std::abs(top1 - bottom2), std::abs(bottom1 - top2));
    return { xDepth, yDepth };
}
