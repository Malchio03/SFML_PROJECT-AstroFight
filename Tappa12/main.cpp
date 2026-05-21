#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include<algorithm>
#include <iostream>

#include "textures.hpp"

//////////////////////
// Initial defaults //
//////////////////////

// window
const char* window_title = "11 - explosion";
const unsigned window_width = 800;
const unsigned window_height = 600;
const float max_frame_rate = 60;

// spaceship
const float spaceship_speed = 400.0;
const sf::Vector2f spaceship_size = {80.0, 80.0};

// player bullets
const float player_bullets_speed = 290.0;
const sf::Vector2f player_bullets_size = {4.0, 15.0};
float player_fire_timer = 0.0;
float player_fire_rate = 0.5;

// enemy
const sf::Vector2f enemy_size = {90.0, 90.0};
const sf::Vector2f enemy_bullets_size = {8.0, 30.0};
const float enemy_bullets_speed = 100.0;
const float enemy_speed = 120.0;
const float enemy_fire_rate = 4.0;

// wave
const int enemy_cols = 4;
const int enemy_rows = 2;
const int enemies_per_wave = enemy_cols * enemy_rows;
const int total_waves = 8;
const int total_enemies = enemies_per_wave * total_waves;
const float enemy_spacing_x = 160.0f;
const float enemy_spacing_y = 90.0f;

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
    sf::Texture bullet_texture;
    bool isAlive;
    float bullet_speed;
    float fire_timer = 0.0;
    std::vector<Bullet> bullets;

    Enemy();
    void fire();
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
    float fire_timer = player_fire_timer;
    float fire_rate = player_fire_rate;

    Spaceship();
    void draw(sf::RenderWindow& window);
    void move_left(float elapsed); 
    void move_right(float elapsed); 
    void move_up(float elapsed);
    void move_down(float elapsed);
    void fire();
};

struct Explosion
{
    sf::Vector2f pos;
    float anim_timer = 0.0;
    int current_frame = 0;
};

struct State
{
    Spaceship spaceship;
    Enemy enemies[enemies_per_wave]; // array of enemies
    int active_enemies;
    int current_wave;
    bool all_moving_right; // shared enemies directions
    Background background;
    bool move_spaceship_left;  
    bool move_spaceship_right; 
    bool move_spaceship_up;
    bool move_spaceship_down;
    bool focus; 
    sf::Texture explosion_texture;
    std::vector<Explosion> explosions;

    bool gameOver = false;
    bool win = false;
    const sf::Font font {"../resources/font.ttf"};

    // AUDIO

    // bg music
    sf::Music bgMusic;

    // sound buffer
    sf::SoundBuffer shootBuffer;
    sf::SoundBuffer enemyShootBuffer;
    sf::SoundBuffer hitBuffer;
    sf::SoundBuffer defeatBuffer;
    sf::SoundBuffer winBuffer;

    // sound to add to the sound buffer
    sf::Sound shootSound;
    sf::Sound enemyShootSound;
    sf::Sound hitSound;      
    sf::Sound defeatSound;     
    sf::Sound winSound;

    State() : spaceship(), 
              move_spaceship_left(false), 
              move_spaceship_right(false), 
              move_spaceship_up(false),
              move_spaceship_down(false),
              focus(false),
              shootSound(shootBuffer),
              enemyShootSound(enemyShootBuffer),
              hitSound(hitBuffer),
              defeatSound(defeatBuffer),
              winSound(winBuffer)

              {
                if (!explosion_texture.loadFromMemory(explosion_png, explosion_png_len))
                {
                    std::cout << "explosion texture load failed\n";
                }

                // music loading
                if (!bgMusic.openFromFile("../resources/background.ogg")) 
                {
                    std::cout << "background.ogg not found\n";
                }
                bgMusic.setLooping(true); 
                bgMusic.setVolume(60.0);
                bgMusic.play();

                // sfx loading
                if(!shootBuffer.loadFromFile("../resources/shoot.wav"))
                {
                    std::cout << "shoot.wav not found\n";
                }
                shootSound.setBuffer(shootBuffer);
                shootSound.setVolume(20.0);

                if (!enemyShootBuffer.loadFromFile("../resources/enemy_shoot.wav"))
                {
                    std::cout << "enemy_shoot.wav not found\n";
                }
                enemyShootSound.setBuffer(enemyShootBuffer);
                enemyShootSound.setVolume(20.0);
                if (!hitBuffer.loadFromFile("../resources/explosion.wav"))
                {
                    std::cout << "explosion.wav not found\n";
                }
                hitSound.setBuffer(hitBuffer);
                hitSound.setVolume(20.0);

                if (!defeatBuffer.loadFromFile("../resources/gameOver.wav"))
                {
                    std::cout << "gameOver.wav not found\n";
                }
                defeatSound.setBuffer(defeatBuffer);
                defeatSound.setVolume(20.0);

                if (!winBuffer.loadFromFile("../resources/win.wav")) 
                {
                    std::cout << "win.wav not found\n";
                }
                winSound.setBuffer(winBuffer);
                winSound.setVolume(20.0);

                spawn_wave();
              }
          
    void spawn_wave();
    void draw(sf::RenderWindow& window);
    void field_limits();
    void collisions();
    void update(float elapsed); 
    void restart();
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
    bullet_speed = enemy_bullets_speed;
    isAlive = true;
    size = enemy_size;
    pos = {0.0, 0.0};
    fire_timer = 0.0;
    texture = sf::Texture(enemy_png, enemy_png_len);
    bullet_texture = sf::Texture(enemy_bullets_png, enemy_bullets_png_len);
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

        sf::Sprite bSprite(bullet_texture);
        bSprite.setScale({2.0f, 2.0f});

        bSprite.setOrigin({16.0, 16.0});
        bSprite.setRotation(sf::degrees(180.0));

        for(size_t i = 0; i < bullets.size(); ++i)
        {
            bSprite.setTextureRect(sf::IntRect({bullets[i].current_frame * 32, 0}, {32, 32}));
            
            float center_x = bullets[i].pos.x + (enemy_bullets_size.x / 2.0f);
            float center_y = bullets[i].pos.y + (enemy_bullets_size.y / 2.0f);
            
            bSprite.setPosition({center_x, center_y});
            window.draw(bSprite);
        }
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
    for(int i = 0; i < enemies_per_wave; ++i)
    {
        enemies[i].draw(window);
    }

    // explosion
    sf::Sprite expSprite(explosion_texture); 
    int frame_size = explosion_texture.getSize().y; 
    float scaleX = enemy_size.x / frame_size;
    float scaleY = enemy_size.y / frame_size;
    expSprite.setScale({scaleX, scaleY});

    for(size_t i = 0; i < explosions.size(); ++i)
    {
        expSprite.setTextureRect(sf::IntRect({explosions[i].current_frame * frame_size, 0}, {frame_size, frame_size}));
        expSprite.setPosition(explosions[i].pos);
        window.draw(expSprite);
    }
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

void Enemy::fire()
{
    sf::Vector2f startPos = {pos.x + (size.x / 2.0f) - (player_bullets_size.x / 2.0f),  pos.y + size.y };
    bullets.push_back({startPos, 0.0f, 0});
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

    // player bullets
    for (size_t i = 0; i < spaceship.bullets.size(); )
    {
        sf::FloatRect bulletRect(spaceship.bullets[i].pos, player_bullets_size);
        bool hit = false;

        for (int e = 0; e < enemies_per_wave; e++)
        {
            if (!enemies[e].isAlive) continue;

            sf::FloatRect enemyRect(enemies[e].pos, enemies[e].size);
            if (bulletRect.findIntersection(enemyRect))
            {
                enemies[e].isAlive = false;
                active_enemies--;
                hit = true;
                hitSound.play();
                explosions.push_back({enemies[e].pos, 0.0f, 0});
                break;
            }
        }

        if (hit)
        {
            std::swap(spaceship.bullets[i], spaceship.bullets.back());
            spaceship.bullets.pop_back();
        }
        else
        {
            i++;
        }
    }

    // enemy bullets
    for (int i = 0; i < enemies_per_wave; ++i)
    {
        if(!enemies[i].isAlive) continue;

        for(int b = 0; b < enemies[i].bullets.size(); ++b)
        {
            sf::FloatRect bulletRect(enemies[i].bullets[b].pos, enemy_bullets_size);
            sf::FloatRect playerRect(spaceship.pos, spaceship.size);

            if (bulletRect.findIntersection(playerRect))
            {
                //this->restart();
                defeatSound.play();
                bgMusic.stop();
                gameOver = true;
                return;
            }
        }
    }
}

void State::update (float elapsed)
{
    if(gameOver || win)
    {
        return;
    }

    if (move_spaceship_left)
        spaceship.move_left(elapsed);
    if (move_spaceship_right)
        spaceship.move_right(elapsed);
    if (move_spaceship_up)
        spaceship.move_up(elapsed);
    if (move_spaceship_down)
        spaceship.move_down(elapsed);

    float swap_interval = 0.1;
    spaceship.fire_timer += elapsed;
    // Player bullets logic
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

    // Enemy logic


    bool hit_wall = false;
    // hit wall checker
    for (int i = 0; i < enemies_per_wave; ++i)
    {
        if (!enemies[i].isAlive) continue;

        if (all_moving_right && enemies[i].pos.x + enemies[i].size.x >= window_width)
        {
            hit_wall = true;
        }
        if (!all_moving_right && enemies[i].pos.x <= 0.0)
        {
            hit_wall = true;
        }
    }

    // if the enemy hit the wall then "flip"
    if (hit_wall)
    {
        all_moving_right = !all_moving_right;

        float limite_inferiore = (window_height / 2.0) - enemy_size.y;
        float max_y = 0.0f;

        for (int i = 0; i < enemies_per_wave; ++i)
        {
            if (enemies[i].isAlive && enemies[i].pos.y > max_y)
            {
                max_y = enemies[i].pos.y;
            }
        }

        float drop_amount = 20.0f;
        if (max_y + drop_amount > limite_inferiore)
        {
            drop_amount = limite_inferiore - max_y;
            if (drop_amount < 0.0f)
            {
                drop_amount = 0.0f;
            } 
        }

        for (int i = 0; i < enemies_per_wave; ++i)
        {
            if(!enemies[i].isAlive) continue;
            
            enemies[i].pos.y += drop_amount; 
        }
    }

    for (int i = 0; i < enemies_per_wave; ++i)
    {
        if(!enemies[i].isAlive) continue;

        // horizontal movements
        if(all_moving_right)
        {
            enemies[i].pos.x += enemy_speed * elapsed;
        } else {
            enemies[i].pos.x -= enemy_speed * elapsed;
        }

        // fire timer
        enemies[i].fire_timer += elapsed;
        if(enemies[i].fire_timer >= enemy_fire_rate)
        {
            enemies[i].fire();
            enemies[i].fire_timer = 0.0;
            enemyShootSound.play();
        }

        // enemies bullets
        for (size_t b = 0; b < enemies[i].bullets.size(); )
        {
            enemies[i].bullets[b].pos.y += enemies[i].bullet_speed * elapsed;

            enemies[i].bullets[b].anim_timer += elapsed;
            if (enemies[i].bullets[b].anim_timer > swap_interval)
            {
                enemies[i].bullets[b].anim_timer = 0.0f;
                enemies[i].bullets[b].current_frame = (enemies[i].bullets[b].current_frame + 1) % 3;
            }

            if (enemies[i].bullets[b].pos.y > window_height)
            {
                std::swap(enemies[i].bullets[b], enemies[i].bullets.back());
                enemies[i].bullets.pop_back();
            }
            else
                ++b;
        }
    }

    for (size_t i = 0; i < explosions.size(); )
    {
        explosions[i].anim_timer += elapsed;
        if (explosions[i].anim_timer > 0.05) 
        {
            explosions[i].anim_timer = 0.0;
            explosions[i].current_frame++;
        }
        
        if (explosions[i].current_frame > 9) 
        {
            std::swap(explosions[i], explosions.back());
            explosions.pop_back();
        }
        else
        {
            i++;
        }
    }
    collisions();

    // win condition 
    if (active_enemies <= 0)
    {
        current_wave++;

        if (current_wave >= total_waves)
        {
            //std::cout << "hai vinto\n";
            //restart();
            win = true;
            bgMusic.stop();
            winSound.play();
            return;
        }
        else
        {
            spawn_wave();
        }
    }
    //std::cout << "Proiettili in volo (Size): " << spaceship.bullets.size() << std::endl;
}

void State::restart()
{
    spaceship = Spaceship();
    current_wave = 0;
    all_moving_right = true;
    move_spaceship_left = false;
    move_spaceship_right = false;
    move_spaceship_up = false;
    move_spaceship_down = false;
    gameOver = false;
    win = false;
    bgMusic.play();
    defeatSound.stop();
    winSound.stop();
    spawn_wave(); 
}

///////////////////////////////////
// spawn_wave                    //
///////////////////////////////////

void State::spawn_wave()
{
    float grid_width = enemy_cols * enemy_spacing_x - (enemy_spacing_x - enemy_size.x);
    float start_x = ((float) window_width - grid_width) / 2.0;
    float start_y = 20.0; // padding
    for(int i = 0; i < enemies_per_wave; ++i)
    {
        int col = i % enemy_cols;
        int row = i / enemy_cols;

        enemies[i] = Enemy();
        enemies[i].pos.x = start_x + col * enemy_spacing_x;
        enemies[i].pos.y = start_y + row * enemy_spacing_y;
        enemies[i].fire_timer = (float) i * (enemy_fire_rate / enemies_per_wave);
    }

    active_enemies = enemies_per_wave;
    all_moving_right = true;
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

    if (state.gameOver || state.win)
    {
        if(key.scancode == sf::Keyboard::Scancode::R)
        {
            state.restart();
        }
        return;
    }
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
            if (state.spaceship.fire_timer >= state.spaceship.fire_rate)
            {
                state.spaceship.fire();
                state.spaceship.fire_timer = 0.0f; 
                state.shootSound.play();
            }
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

    sf::Text gameOverText{state.font, "GAME OVER", 60};
    gameOverText.setFillColor(sf::Color::Red);
    gameOverText.setPosition({240.0f, 200.0f});

    sf::Text restartText{state.font, "Press R to Restart", 30};
    restartText.setFillColor(sf::Color::White);
    restartText.setPosition({245.0f, 290.0f});

    sf::Text winText{state.font, "YOU WON!", 60};
    winText.setFillColor(sf::Color::Green);
    winText.setPosition({260.0f, 200.0f});

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

        if (!state.gameOver && !state.win)
        {
            state.draw (window);
        } else if(state.gameOver){
            window.draw(gameOverText);
            window.draw(restartText);
        } else if(state.win){
            window.draw(winText);      
            window.draw(restartText);
        }

        window.display();
    }
}