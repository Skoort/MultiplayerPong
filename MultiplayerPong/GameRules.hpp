#ifndef GameRules_hpp
#define GameRules_hpp

#include <string>

#include <SFML/System/Vector2.hpp>

class GameRules
{
    public:
        static constexpr sf::Vector2u windowSize = sf::Vector2u{640, 480};
        static constexpr int packetsPerSecond = 35;
        static bool isFocused;
        static const bool shouldSwapPositions = false;
};

#endif  // GameRules_hpp
