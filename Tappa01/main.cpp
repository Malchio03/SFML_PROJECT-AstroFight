#include <SFML/Graphics.hpp>

//////////////////////
// Initial defaults //
//////////////////////

// window
const char* window_title = "01 - basic structure";
const unsigned window_width = 800;
const unsigned window_height = 600;
const float max_frame_rate = 60;

// spaceship
const sf::Vector2f spaceship_size = {100.0, 16.0};

///////////////////////////////////
// Internal state representation //
///////////////////////////////////

struct Spaceship
{
    sf::Vector2f size;
    sf::Vector2f pos;

    Spaceship();
    void draw(sf::RenderWindow& window);
};

struct State
{
    Spaceship spaceship;

    State() {}
    void draw(sf::RenderWindow& window);
};

///////////////////////////////////
// Constructors //
///////////////////////////////////

Spaceship::Spaceship ()
{
    size = spaceship_size;
    float spaceship_px = ((float) window_width / 2.0) - (spaceship_size.x / 2.0);
    float spaceship_py = (float) window_height - spaceship_size.y;
    pos = {spaceship_px, spaceship_py};
}



//////////
// Draw //
//////////

void Spaceship::draw (sf::RenderWindow& window)
{
    sf::RectangleShape p(size);
    p.setPosition(pos);
    window.draw(p);
}

void State::draw (sf::RenderWindow& window)
{
    spaceship.draw(window);
}



////////////
// Events //
////////////

void handle_close (sf::RenderWindow& window)
{
    window.close();
}
void handle_resize (const sf::Event::Resized& resized, sf::RenderWindow& window)
{   // constrain aspect ratio and map always the same portion of the world
    float aspect = static_cast<float>(window_width)/static_cast<float>(window_height);
    sf::Vector2u ws = resized.size;
    float new_aspect = static_cast<float>(ws.x)/static_cast<float>(ws.y);
    if (new_aspect < aspect)
        ws = {ws.x,static_cast<unsigned>(ws.x/aspect)};
        else
        ws = {static_cast<unsigned>(ws.y*aspect),ws.y};
    window.setSize(ws);
}



//////////
// Loop //
//////////

int main()
{
    sf::RenderWindow window (sf::VideoMode ({window_width, window_height}), window_title);
    window.setFramerateLimit (max_frame_rate);
    window.setMinimumSize(window.getSize());

    State state;

    while (window.isOpen())
    {
        // events
        window.handleEvents (
                             [&window](const sf::Event::Closed&) { handle_close (window); },
                             [&window](const sf::Event::Resized& event) { handle_resize (event, window); }
        );

        // display
        window.clear (sf::Color::Black);
        state.draw (window);
        window.display ();
    }
}