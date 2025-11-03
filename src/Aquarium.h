#define NOMINMAX // To avoid min/max macro conflict on Windows

#include <vector>
#include <memory>
#include <iostream>
#include <algorithm>
#include "ofMain.h"
#include "Core.h"


enum class AquariumCreatureType {
    NPCreature,
    BiggerFish,
    PowerUp,
    Jellyfish,
    Shark
};

string AquariumCreatureTypeToString(AquariumCreatureType t);

class PowerUpCreature : public Creature {
public:
    PowerUpCreature(float x, float y, std::shared_ptr<GameSprite> sprite = nullptr);
    void move() override;
    void draw() const override;
    static const int POWER_BOOST = 2;  // cuanto aumenta el poder
    static const float SIZE_BOOST;      // cuanto aumenta el tamano
private:
    float m_angle = 0;  // para animacion flotante
};

class AquariumLevelPopulationNode{
    public:
        AquariumLevelPopulationNode() = default;
        AquariumLevelPopulationNode(AquariumCreatureType creature_type, int population) {
            this->creatureType = creature_type;
            this->population = population;
            this->currentPopulation = 0;
        };
        AquariumCreatureType creatureType;
        int population;
        int currentPopulation;
};

class AquariumLevel : public GameLevel {
    public:
        AquariumLevel(int levelNumber, int targetScore)
        : GameLevel(levelNumber), m_level_score(0), m_targetScore(targetScore){};
        void ConsumePopulation(AquariumCreatureType creature, int power);
        bool isCompleted() override;
        void populationReset();
        void levelReset(){m_level_score=0;this->populationReset();}
        virtual std::vector<AquariumCreatureType> Repopulate() = 0;
    protected:
        std::vector<std::shared_ptr<AquariumLevelPopulationNode>> m_levelPopulation;
        int m_level_score;
        int m_targetScore;

};


class PlayerCreature : public Creature {
public:

    PlayerCreature(float x, float y, int speed, std::shared_ptr<GameSprite> sprite);
    void move();
    void draw() const;
    void update();
    void changeSpeed(int speed);
    void setLives(int lives) { m_lives = lives; }
    void setDirection(float dx, float dy);
    float isXDirectionActive() { return m_dx != 0; }
    float isYDirectionActive() {return m_dy != 0; }
    float getDx() { return m_dx; }
    float getDy() { return m_dy; }

    int getScore()const { return m_score; }
    int getLives() const { return m_lives; }
    int getPower() const { return m_power; }
    
    void addToScore(int amount, int weight=1) { m_score += amount * weight; }
    void loseLife(int debounce);
    void increasePower(int value) { m_power += value; }
    void reduceDamageDebounce();
    
private:
    int m_score = 0;
    int m_lives = 3;
    int m_power = 1; // mark current power lvl
    int m_damage_debounce = 0; // frames to wait after eating
};

class NPCreature : public Creature {
public:
    NPCreature(float x, float y, int speed, std::shared_ptr<GameSprite> sprite);
    AquariumCreatureType GetType() {return this->m_creatureType;}
    void move() override;
    void draw() const override;
protected:
    AquariumCreatureType m_creatureType;

};

class BiggerFish : public NPCreature {
public:
    BiggerFish(float x, float y, int speed, std::shared_ptr<GameSprite> sprite);
    void move() override;
    void draw() const override;
};

class Jellyfish : public NPCreature {
public:
    Jellyfish(float x, float y, int speed, std::shared_ptr<GameSprite> sprite);
    void move() override;
    void draw() const override;
private:
    float m_angle = 0.0f;  // For sinusoidal motion
    float m_spikeCooldown = 0;  // For upward spike movement
};

class Shark : public NPCreature {
public:
    Shark(float x, float y, int speed, std::shared_ptr<GameSprite> sprite);
    void move() override;
    void draw() const override;
    static void SetPlayer(std::shared_ptr<PlayerCreature> p);
private:
    static std::weak_ptr<PlayerCreature> s_player;  // Reference to player for tracking
    float m_detectionRadius = 300.0f;  // How close player needs to be for shark to chase
};

class AquariumSpriteManager {
    public:
        AquariumSpriteManager();
        ~AquariumSpriteManager() = default;
        std::shared_ptr<GameSprite>GetSprite(AquariumCreatureType t);
    private:
        std::shared_ptr<GameSprite> m_npc_fish;
        std::shared_ptr<GameSprite> m_big_fish;
        std::shared_ptr<GameSprite> m_jelly_fish;
        std::shared_ptr<GameSprite> m_shark;

        // removed m_power_up png usage per request
};


class Aquarium{
public:
    Aquarium(int width, int height, std::shared_ptr<AquariumSpriteManager> spriteManager);
    void addCreature(std::shared_ptr<Creature> creature);
    void addAquariumLevel(std::shared_ptr<AquariumLevel> level);
    void removeCreature(std::shared_ptr<Creature> creature);
    void clearCreatures();
    void update();
    void draw() const;
    void setBounds(int w, int h) { m_width = w; m_height = h; }
    void setMaxPopulation(int n) { m_maxPopulation = n; }
    void Repopulate();
    void SpawnCreature(AquariumCreatureType type);
    
    std::shared_ptr<Creature> getCreatureAt(int index);
    int getCreatureCount() const { return m_creatures.size(); }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }


private:
    int m_maxPopulation = 0;
    int m_width;
    int m_height;
    int currentLevel = 0;
    std::vector<std::shared_ptr<Creature>> m_creatures;
    std::vector<std::shared_ptr<Creature>> m_next_creatures;
    std::vector<std::shared_ptr<AquariumLevel>> m_aquariumlevels;
    std::shared_ptr<AquariumSpriteManager> m_sprite_manager;
};


std::shared_ptr<GameEvent> DetectAquariumCollisions(std::shared_ptr<Aquarium> aquarium, std::shared_ptr<PlayerCreature> player);


class AquariumGameScene : public GameScene {
    public:
        AquariumGameScene(std::shared_ptr<PlayerCreature> player, std::shared_ptr<Aquarium> aquarium, string name)
        : m_player(std::move(player)) , m_aquarium(std::move(aquarium)), m_name(name){
            // Setup sound effects
            m_chomp.load("chomp-1.ogg");
            m_chomp.setMultiPlay(true);
            m_chomp.setVolume(0.75f);
            
            // Setup background music
            m_bgMusic.load("music.mp3");
            m_bgMusic.setLoop(true);
            m_bgMusic.setVolume(0.3f); // Lower volume for background
            m_bgMusic.play();
            // registrar jugador para tiburon
            Shark::SetPlayer(m_player);

        }
        std::shared_ptr<GameEvent> GetLastEvent(){return m_lastEvent;}
        void SetLastEvent(std::shared_ptr<GameEvent> event){this->m_lastEvent = event;}
        std::shared_ptr<PlayerCreature> GetPlayer(){return this->m_player;}
        std::shared_ptr<Aquarium> GetAquarium(){return this->m_aquarium;}
        string GetName()override {return this->m_name;}
        void Update() override;
        void Draw() override;
    private:
        void paintAquariumHUD();
        std::shared_ptr<PlayerCreature> m_player;
        std::shared_ptr<Aquarium> m_aquarium;
        std::shared_ptr<GameEvent> m_lastEvent;
        string m_name;
        AwaitFrames updateControl{5};
        ofSoundPlayer m_chomp;
        ofSoundPlayer m_bgMusic;
};


class Level_0 : public AquariumLevel  {
    public:
        Level_0(int levelNumber, int targetScore): AquariumLevel(levelNumber, targetScore){
            this->m_levelPopulation.push_back(std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::NPCreature, 10));

        };
        std::vector<AquariumCreatureType> Repopulate() override;

};
class Level_1 : public AquariumLevel  {
    public:
        Level_1(int levelNumber, int targetScore): AquariumLevel(levelNumber, targetScore){
            this->m_levelPopulation.push_back(std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::NPCreature, 20));
            this->m_levelPopulation.push_back(std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::BiggerFish, 3));
            this->m_levelPopulation.push_back(std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::Jellyfish, 4));
        };
        std::vector<AquariumCreatureType> Repopulate() override;


};
class Level_2 : public AquariumLevel  {
    public:
        Level_2(int levelNumber, int targetScore): AquariumLevel(levelNumber, targetScore){
            this->m_levelPopulation.push_back(std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::NPCreature, 30));
            this->m_levelPopulation.push_back(std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::BiggerFish, 5));
            this->m_levelPopulation.push_back(std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::Jellyfish, 6));
            this->m_levelPopulation.push_back(std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::Shark, 2));
        };
        std::vector<AquariumCreatureType> Repopulate() override;

};
