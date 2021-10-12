#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <fstream>
#include "SDL2/SDL.h"
#include "SDL2/SDL_mixer.h"
#include "Math.h"

class Game
{
public:
    Game();
    bool Initialize();
    void Shutdown();
    void RunLoop();
    ~Game();

    const int windowWidth = 600;
    const int windowHeight = 448;
    const float gridSize = 32;
    const int tileStart = 16;
    const float backgroundX = 3392;
    const float backgroundy = 224;
    int theme = -1;

    /* data specific */
    void AddActor(class Actor *actor);
    void RemoveActor(class Actor *actor);
    void AddSprite(class SpriteComponent *component);
    void RemoveSprite(class SpriteComponent *component);
    SDL_Texture *GetTexture(const std::string_view filename);
    Mix_Chunk *GetSound(const std::string_view filename);
    void AddBlock(class Block *block) { blocks.emplace_back(block); };
    void RemoveBlock(class Block *block)
    {
        auto result = std::find(std::begin(blocks), std::end(blocks), block);
        if (result != std::end(blocks))
        {
            blocks.erase(result);
        }
    };
    void AddGoomba(class Goomba *goomba) { goombas.emplace_back(goomba); };
    void RemoveGoomba(class Goomba *goomba)
    {
        auto result = std::find(std::begin(goombas), std::end(goombas), goomba);
        if (result != std::end(goombas))
        {
            goombas.erase(result);
        }
    };
    auto &GetBlocks() { return blocks; }
    auto &GetGoombas() { return goombas; }
    auto &GetCameraPos() { return cameraPos; };
    class Actor *GetPlayer()
    {
        return player;
    }

private:
    /* constants */
    enum Keys
    {
        UP,
        DOWN,
        LEFT,
        RIGHT,
        NONE
    };

    /* data */
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    bool gameEnded;
    uint32_t previousTime;
    char direction = Keys::NONE;
    char movement = Keys::NONE;
    std::vector<class Actor *> actors;
    std::vector<class SpriteComponent *> spriteComponents;
    std::unordered_map<std::string, SDL_Texture *> textures;
    std::unordered_map<std::string, Mix_Chunk *> sounds;
    std::vector<class Block *> blocks;
    std::vector<class Goomba *> goombas;
    class Actor *player;
    Vector2 cameraPos;

    /* methods*/
    void ProcessInput();
    void UpdateGame();
    void GenerateOutput();
    inline float mmtosConvert(int seconds);

    /* data specific */
    void LoadData();
    void UnloadData();
};
