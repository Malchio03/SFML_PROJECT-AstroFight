#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>
#include <vector>
#include<algorithm>
#include <iostream>

#include "textures.hpp"

//////////////////////
// Initial defaults //
//////////////////////

// window
const char* window_title = "07 - design";
const unsigned window_width = 800;
const unsigned window_height = 600;
const float max_frame_rate = 60;

// spaceship
const float spaceship_speed = 400.0;
const sf::Vector2f spaceship_size = {80.0, 80.0};

// player bullets
const float player_bullets_speed = 290.0;
const sf::Vector2f player_bullets_size = {4.0, 15.0};

// enemy
const sf::Vector2f enemy_size = {145.0, 145.0};

///////////////////////////////////
// Internal state representation //
///////////////////////////////////
struct Background
{
    sf::Texture texture;
    sf::RectangleShape shape;

    Background();
    void draw(sf::RenderWindow& window);
};

struct Bullet
{
    sf::Vector2f pos;
    float anim_timer;
    int current_frame;
};

struct Enemy
{
    sf::Vector2f size;
    sf::Vector2f pos;
    sf::Texture texture;
    bool isAlive;

    Enemy();
    void draw(sf::RenderWindow& window);
};

struct Spaceship
{
    sf::Vector2f size;
    sf::Vector2f pos;
    sf::Texture texture;
    sf::Texture bullet_texture;
    float speed;
    std::vector<Bullet> bullets;

    Spaceship();
    void draw(sf::RenderWindow& window);
    void move_left(float elapsed); 
    void move_right(float elapsed); 
    void move_up(float elapsed);
    void move_down(float elapsed);
    void fire();
};

struct State
{
    Spaceship spaceship;
    Enemy enemy;
    Background background;
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

Spaceship::Spaceship()
{
    size = spaceship_size;
    float spaceship_px = ((float) window_width / 2.0) - (spaceship_size.x / 2.0);
    float spaceship_py = (float) window_height - spaceship_size.y;
    pos = {spaceship_px, spaceship_py};
    texture = sf::Texture(spaceship_png, spaceship_png_len);
    bullet_texture = sf::Texture(player_bullets_png, player_bullets_png_len);
    speed = spaceship_speed;
}

Enemy::Enemy()
{
    isAlive = true;
    size = enemy_size;
    float enemy_px = ((float) window_width / 2.0) - (enemy_size.x / 2.0);
    float enemy_py = 0.0;
    pos = {enemy_px, enemy_py};
    texture = sf::Texture(enemy_png, enemy_png_len);

}

Background::Background()
{
    texture = sf::Texture(bg_png, bg_png_len);
    texture.setRepeated(true);
    shape.setSize({800.0f, 600.0f});
    shape.setTexture(&texture);
    shape.setTextureRect(sf::IntRect({0, 0}, {800, 600}));
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

    sf::Sprite bSprite(bullet_texture);
    
    bSprite.setScale({2.0f, 2.0f}); 

    for(size_t i = 0; i < bullets.size(); ++i)
    {
        bSprite.setTextureRect(sf::IntRect({bullets[i].current_frame * 32, 0}, {32, 32}));

        float offset_x = bullets[i].pos.x - 30.0f; // (64 - 4) / 2
        float offset_y = bullets[i].pos.y - 24.5f; // (64 - 15) / 2
        
        bSprite.setPosition({offset_x, offset_y});
        window.draw(bSprite);
    }
}

void Enemy::draw(sf::RenderWindow& window)
{
    if(isAlive)
    {
        sf::RectangleShape e(size);
        e.setTexture(&texture);
        e.setPosition(pos);
        window.draw(e);
    }
}

void Background::draw (sf::RenderWindow& window)
{
    window.draw(shape);
}

void State::draw (sf::RenderWindow& window)
{
    background.draw(window);
    spaceship.draw(window);
    enemy.draw(window);
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

void Spaceship::fire()
{
    sf::Vector2f startPos = {pos.x + (size.x / 2.0f) - (player_bullets_size.x / 2.0f), pos.y};
    bullets.push_back({startPos, 0.0, 0});
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

    if (!enemy.isAlive) return;

    for (size_t i = 0; i < spaceship.bullets.size(); )
    {

        sf::FloatRect bulletRect(spaceship.bullets[i].pos, player_bullets_size);
        sf::FloatRect enemyRect(enemy.pos, enemy.size);

        if (bulletRect.findIntersection(enemyRect))
        {
            enemy.isAlive = false; 

            std::swap(spaceship.bullets[i], spaceship.bullets.back());
            spaceship.bullets.pop_back();
        } 
        else 
        {
            i++; 
        }
    }
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

    float swap_interval = 0.1;

    for(size_t i = 0; i < spaceship.bullets.size(); )
    {
        spaceship.bullets[i].pos.y -= player_bullets_speed * elapsed;

        // bullet animation
        spaceship.bullets[i].anim_timer += elapsed;

        if (spaceship.bullets[i].anim_timer > swap_interval)
        {
            spaceship.bullets[i].anim_timer = 0.0; // timer reset
            spaceship.bullets[i].current_frame = (spaceship.bullets[i].current_frame + 1) % 3;
        }

        if (spaceship.bullets[i].pos.y < -player_bullets_size.y)
        {
            std::swap(spaceship.bullets[i], spaceship.bullets.back());
            spaceship.bullets.pop_back();
        } else {
            ++i;
        }
    }
    collisions();
    //std::cout << "Proiettili in volo (Size): " << spaceship.bullets.size() << std::endl;
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
        case sf::Keyboard::Scancode::Space:
            state.spaceship.fire();
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