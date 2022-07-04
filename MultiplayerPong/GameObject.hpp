#ifndef GameObject_hpp
#define GameObject_hpp

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

class GameObject
{
    public:
        GameObject();
        GameObject(sf::Vector2f position, sf::Vector2f dimensions, sf::Vector2f velocity = sf::Vector2f{0, 0});
    
        sf::Vector2f const &GetPosition() const;
        void SetPosition(sf::Vector2f position);
        void SetPosition(float x, float y);
        void ClampPosition(float xMin, float xMax, float yMin, float yMax);
    
        sf::Vector2f const &GetDimensions() const;
        void SetDimensions(sf::Vector2f dimensions);
        void SetDimensions(float x, float y);

        sf::Vector2f const &GetScale() const;
        void SetScale(sf::Vector2f scale);
        void SetScale(float x, float y);

        sf::Vector2f const &GetOrigin() const;
        void SetOrigin(sf::Vector2f dimensions);
        void SetOrigin(float x, float y);
    
        sf::Vector2f GetCenter() const;

        sf::Vector2f const &GetVelocity() const;
        void SetVelocity(sf::Vector2f velocity);
        void SetVelocity(float x, float y);
    
        void Draw(sf::RenderTarget &renderTarget) const;
        void Move(float dt);
    
        bool Collides(GameObject const &other) const;
        sf::Vector2f GetCollisionPenetration(GameObject const &other) const;
    
    private:
        sf::RectangleShape m_graphics;
        sf::Vector2f m_velocity;
};

#include "GameObject.inl"

#endif /* GameObject_hpp */
