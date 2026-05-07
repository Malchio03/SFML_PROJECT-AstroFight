#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>
#include "textures.hpp"
//////////////////////
// Initial defaults //
//////////////////////

// window
const char* window_title = "03 - move spaceship";
const unsigned window_width = 800;
const unsigned window_height = 600;
const float max_frame_rate = 60;

// spaceship
const float spaceship_speed = 400.0;
const sf::Vector2f spaceship_size = {80.0, 80.0};

///////////////////////////////////
// Internal state representation //
///////////////////////////////////

struct Spaceship
{
    sf::Vector2f size;
    sf::Vector2f pos;
    sf::Texture texture;
    float speed;

    Spaceship();
    void draw(sf::RenderWindow& window);
    void move_left(float elapsed); 
    void move_right(float elapsed); 
    void move_up(float elapsed);
    void move_down(float elapsed);
};

struct State
{
    Spaceship spaceship;
    bool move_spaceship_left;  
    bool move_spaceship_right; 
    bool move_spaceship_up;
    bool move_spaceship_down;
    bool focus;                

    State() : spaceship(), 
              move_spaceship_left(false), 
              move_spaceship_right(false), 
              move_spaceship_up(false),
              move_spaceship_down(false),
              focus(false) {}
              
    void draw(sf::RenderWindow& window);
    void field_limits();
    void collisions();
    void update(float elapsed); 
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
    texture = sf::Texture(spaceship_png, spaceship_png_len);
    speed = spaceship_speed;
}



//////////
// Draw //
//////////

void Spaceship::draw (sf::RenderWindow& window)
{
    sf::RectangleShape p(size);
    p.setTexture(&texture);
    p.setPosition(pos);
    window.draw(p);
}

void State::draw (sf::RenderWindow& window)
{
    spaceship.draw(window);
}

////////////
// Update //
////////////

void Spaceship::move_left(float elapsed)
{
    pos.x -= speed * elapsed;
}

void Spaceship::move_right(float elapsed)
{
    pos.x += speed * elapsed;
}

void Spaceship::move_up(float elapsed)
{
    pos.y -= speed * elapsed;
}

void Spaceship::move_down(float elapsed)
{
    pos.y += speed * elapsed;
}

void State::field_limits()
{
    if(spaceship.pos.x < 0.0)
        spaceship.pos.x = 0.0;
    if(spaceship.pos.x + spaceship.size.x > window_width)
        spaceship.pos.x = window_width - spaceship.size.x;

    float limite_superiore = window_height / 2.0;

    if(spaceship.pos.y < limite_superiore)
        spaceship.pos.y = limite_superiore;

    if(spaceship.pos.y + spaceship.size.y > window_height)
        spaceship.pos.y = window_height - spaceship.size.y;
}

void State::collisions()
{
    field_limits();
}

void State::update (float elapsed)
{
    if (move_spaceship_left)
        spaceship.move_left(elapsed);
    if (move_spaceship_right)
        spaceship.move_right(elapsed);
    if (move_spaceship_up)
        spaceship.move_up(elapsed);
    if (move_spaceship_down)
        spaceship.move_down(elapsed);

    collisions();
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

template <typename T>
void handle (T& event, State& state) {}

void handle(const sf::Event::KeyPressed& key, State& state)
{
    if(!state.focus)
        return;

    switch(key.scancode)
    {
        case sf::Keyboard::Scancode::Left:
            state.move_spaceship_left = true;
            return;
        case sf::Keyboard::Scancode::Right:
            state.move_spaceship_right = true;
            return;
        case sf::Keyboard::Scancode::Up:
            state.move_spaceship_up = true;
            return;
        case sf::Keyboard::Scancode::Down:
            state.move_spaceship_down = true;
            return;
        default:
            return;
    }
}

void handle(const sf::Event::KeyReleased& key, State& state)
{
    if(!state.focus)
        return;
    
    switch(key.scancode)
    {
        case sf::Keyboard::Scancode::Left:
            state.move_spaceship_left = false;
            return;
        case sf::Keyboard::Scancode::Right:
            state.move_spaceship_right = false;
            return;
        case sf::Keyboard::Scancode::Up:
            state.move_spaceship_up = false;
            return;
        case sf::Keyboard::Scancode::Down:
            state.move_spaceship_down = false;
            return;
        default:
            return;
    }
}

void handle(const sf::Event::FocusGained&, State& state)
{
    state.focus = true;
}

void handle(const sf::Event::FocusLost&, State& state)
{
    state.focus = false;
    state.move_spaceship_left = false;
    state.move_spaceship_right = false;
    state.move_spaceship_up = false;
    state.move_spaceship_down = false;
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
    sf::Clock clock;

    while (window.isOpen())
    {
        // events
        window.handleEvents (
                             [&window](const sf::Event::Closed&) { handle_close (window); },
                             [&window](const sf::Event::Resized& event) { handle_resize (event, window); },
                             [&state](const auto& event) { handle (event, state); } 
        );

        // update
        state.update (clock.restart().asSeconds()); 

        // display
        window.clear (sf::Color::Black);
        state.draw (window);
        window.display ();
    }
}