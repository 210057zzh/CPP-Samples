//
//  Game.cpp
//  Game-mac
//
//  Created by Sanjay Madhav on 5/31/17.
//  Copyright © 2017 Sanjay Madhav. All rights reserved.
//

#include <algorithm>
#include <SDL2/SDL_image.h>
#include "Game.h"
#include "Actor.h"
#include "SpriteComponent.h"
#include "Random.h"
#include "CollisionComponent.h"
#include "Block.h"
#include "Player.h"
#include "Spawner.h"

Game::Game() {}

Game::~Game() {}

bool Game::Initialize()
{
    Random::Init();
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) < 0)
        return 0;
    window = SDL_CreateWindow(nullptr, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, 0);
    if (window == nullptr)
        return 0;
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr)
        return 0;
    gameEnded = false;
    IMG_Init(IMG_INIT_PNG);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    LoadData();
    previousTime = SDL_GetTicks();
    return 1;
}

void Game::RunLoop()
{
    while (!gameEnded)
    {
        ProcessInput();
        UpdateGame();
        GenerateOutput();
    }
}

void Game::Shutdown()
{
    UnloadData();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}

void Game::ProcessInput()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            gameEnded = true;
            break;
        }
    }
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    if (state[SDL_SCANCODE_ESCAPE])
    {
        gameEnded = true;
    }
    auto pendingActors = actors;
    for (auto &actor : pendingActors)
    {
        actor->ProcessInput(state);
    }
}

void Game::UpdateGame()
{
    while (SDL_GetTicks() - previousTime < 16) //limiting
    {
    }
    uint32_t currentTime = SDL_GetTicks();
    uint32_t deltaTime = ((currentTime - previousTime) > 33) ? 33 : (currentTime - Game::previousTime);
    previousTime = currentTime;
    auto pendingActors = actors;
    for (auto &actor : pendingActors)
    {
        actor->Update(mmtosConvert(deltaTime));
    }
    std::vector<Actor *> actorsToDestroy;
    for (auto &actor : actors)
    {
        if (actor->GetState() == ActorState::Destroy)
        {
            actorsToDestroy.push_back(actor);
        }
    }
    for (auto &actor : actorsToDestroy)
    {
        delete actor;
    }
}

void Game::GenerateOutput()
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    for (auto &component : spriteComponents)
    {
        if (component->IsVisible())
            component->Draw(renderer);
    }
    SDL_RenderPresent(renderer);
}

inline float Game::mmtosConvert(int seconds)
{
    return static_cast<float>(seconds) / 1000.0f;
}

/* actor specific */

void Game::AddActor(Actor *actor)
{
    actors.push_back(actor);
}

void Game::RemoveActor(Actor *actor)
{
    auto result = std::find(std::begin(actors), std::end(actors), actor);
    if (result != std::end(actors))
    {
        actors.erase(result);
    }
}

void Game::AddSprite(class SpriteComponent *component)
{
    spriteComponents.emplace_back(component);
    std::sort(std::begin(spriteComponents), std::end(spriteComponents), [](SpriteComponent *a, SpriteComponent *b)
              { return a->GetDrawOrder() < b->GetDrawOrder(); });
}

void Game::RemoveSprite(class SpriteComponent *SpriteComponent)
{
    auto result = std::find(std::begin(spriteComponents), std::end(spriteComponents), SpriteComponent);
    if (result != std::end(spriteComponents))
    {
        spriteComponents.erase(result);
    }
}

/* data specific */
void Game::LoadData()
{
    theme = Mix_PlayChannel(-1, GetSound("Assets/Sounds/Music.ogg"), -1);
    Actor *background = new Actor(this);
    SpriteComponent *scBackground = new SpriteComponent(background, 0);
    scBackground->SetTexture(GetTexture("Assets/Background.png"));
    background->SetPosition(Vector2(backgroundX, backgroundy));
    std::ifstream file("Assets/Level1.txt");
    std::string line;
    auto lineNumber = 0;
    size_t lineCount = 0;
    while (std::getline(file, line))
    {
        for (size_t i = 0; i < line.length(); ++i)
        {
            auto square = line[i];
            if (square != '.')
            {
                Vector2 position(static_cast<float>(i * gridSize + tileStart), static_cast<float>(tileStart + gridSize * lineNumber));
                switch (square)
                {
                case 'A':
                case 'B':
                case 'C':
                case 'D':
                case 'E':
                case 'F':
                case 'G':
                case 'H':
                case 'I':
                    new Block(this, position, square);
                    break;
                case 'P':
                    player = new Player(this, position);
                    break;
                case 'Y':
                    new Spawner(this, position);
                }
            }
        }
        ++lineNumber;
    }
}

void Game::UnloadData()
{
    while (!actors.empty())
    {
        delete actors.back();
    }
    actors.clear();
    for (auto &texture : textures)
    {
        SDL_DestroyTexture(texture.second);
    }
    for (auto &sound : sounds)
    {
        Mix_FreeChunk(sound.second);
    }
    textures.clear();
}

SDL_Texture *Game::GetTexture(const std::string_view filename)
{
    auto itr = textures.find(filename.data());
    if (itr != std::end(textures))
    {
        return itr->second;
    }
    else
    {
        auto surface = IMG_Load(filename.data());
        if (!surface)
        {
            SDL_Log("Failed to load surface at GetTexture");
            return nullptr;
        }
        auto texture = SDL_CreateTextureFromSurface(renderer, surface);
        textures.emplace(filename, texture);
        SDL_FreeSurface(surface);
        return texture;
    }
}

Mix_Chunk *Game::GetSound(const std::string_view filename)
{
    auto itr = sounds.find(filename.data());
    if (itr != std::end(sounds))
    {
        return itr->second;
    }
    else
    {
        auto sound = Mix_LoadWAV(filename.data());
        if (!sound)
        {
            SDL_Log("Failed to load sound at GetSound");
            return nullptr;
        }
        sounds.emplace(filename, sound);
        return sound;
    }
}
