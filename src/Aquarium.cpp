#include "Aquarium.h"
#include <cstdlib>

// definicion de variable statica para Shark
std::weak_ptr<PlayerCreature> Shark::s_player;

const float PowerUpCreature::SIZE_BOOST = 1.5f;  // 50% aumento de tamano


PowerUpCreature::PowerUpCreature(float x, float y, std::shared_ptr<GameSprite> sprite)
: Creature(x, y, 0, 30.0f, 0, sprite) {
    // inicializar powerup
}
string AquariumCreatureTypeToString(AquariumCreatureType t){
    switch(t){
        case AquariumCreatureType::BiggerFish:
            return "BiggerFish";
        case AquariumCreatureType::NPCreature:
            return "BaseFish";
        case AquariumCreatureType::PowerUp:
            return "PowerUp";
        case AquariumCreatureType::Jellyfish:
            return "Jellyfish";
        case AquariumCreatureType::Shark:
            return "Shark";
        default:
            return "UknownFish";
    }
}

void PowerUpCreature::move() {
    // flotar arriba y abajo con onda senoidal
    m_angle += 0.05f;
    m_y += sin(m_angle) * 0.5f;
    // evitar salir de los limites
    bounce();
}

void PowerUpCreature::draw() const {
    // draw a glowing procedural powerup circle (no png)
    float baseRadius = 12.0f;
    float pulse = 1.0f + sin(m_angle * 2.0f) * 0.2f;
    float radius = baseRadius * pulse;
    ofSetColor(200, 255, 200, 140);
    ofDrawCircle(m_x, m_y, radius + 6);
    ofSetColor(120, 220, 120, 200);
    ofDrawCircle(m_x, m_y, radius);
    ofSetColor(ofColor::white);
}

// PlayerCreature Implementation
PlayerCreature::PlayerCreature(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
: Creature(x, y, speed, 10.0f, 1, sprite) {}


void PlayerCreature::setDirection(float dx, float dy) {
    m_dx = dx;
    m_dy = dy;
    normalize();
}

void PlayerCreature::move() {
    m_x += m_dx * m_speed;
    m_y += m_dy * m_speed;
    this->bounce();
}

void PlayerCreature::reduceDamageDebounce() {
    if (m_damage_debounce > 0) {
        --m_damage_debounce;
    }
}

void PlayerCreature::update() {
    this->reduceDamageDebounce();
    this->move();
}


void PlayerCreature::draw() const {
    
    ofLogVerbose() << "jugador en posicion (" << m_x << ", " << m_y << ") con velocidad " << m_speed << std::endl;
    if (this->m_damage_debounce > 0) {
        ofSetColor(ofColor::red); // parpadear rojo si esta en periodo de dano
    }
    if (m_sprite) {
        m_sprite->draw(m_x, m_y);
    }
    ofSetColor(ofColor::white); // resetear color a blanco

}

void PlayerCreature::changeSpeed(int speed) {
    m_speed = speed;
}

void PlayerCreature::loseLife(int debounce) {
    if (m_damage_debounce <= 0) {
        if (m_lives > 0) this->m_lives -= 1;
        m_damage_debounce = debounce; // establecer frames de recuperacion
        ofLogNotice() << "jugador perdio una vida vidas restantes: " << m_lives << std::endl;
    }
    // si esta en periodo de recuperacion no hacer nada
    if (m_damage_debounce > 0) {
        ofLogVerbose() << "jugador en periodo de recuperacion frames restantes: " << m_damage_debounce << std::endl;
    }
}

// implementacion de criatura npc
NPCreature::NPCreature(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
: Creature(x, y, speed, 30, 1, sprite) {
    m_dx = (rand() % 3 - 1); // -1 0 o 1
    m_dy = (rand() % 3 - 1); // -1 0 o 1
    normalize();

    m_creatureType = AquariumCreatureType::NPCreature;
}

void NPCreature::move() {
    // logica simple de movimiento ia direccion aleatoria
    m_x += m_dx * m_speed;
    m_y += m_dy * m_speed;
    if(m_dx < 0 ){
        this->m_sprite->setFlipped(true);
    }else {
        this->m_sprite->setFlipped(false);
    }
    bounce();
}

void NPCreature::draw() const {
    ofLogVerbose() << "NPCreature at (" << m_x << ", " << m_y << ") with speed " << m_speed << std::endl;
    ofSetColor(ofColor::white);
    if (m_sprite) {
        m_sprite->draw(m_x, m_y);
    }
}


BiggerFish::BiggerFish(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
: NPCreature(x, y, speed, sprite) {
    m_dx = (rand() % 3 - 1);
    m_dy = (rand() % 3 - 1);
    normalize();

    setCollisionRadius(60); // pez grande tiene radio de colision mas grande
    m_value = 5; // pez grande tiene valor mas alto
    m_creatureType = AquariumCreatureType::BiggerFish;
}

void BiggerFish::move() {
    // pez grande se mueve mas lento o tiene logica diferente
    m_x += m_dx * (m_speed * 0.5); // se mueve a mitad de velocidad
    m_y += m_dy * (m_speed * 0.5);
    if(m_dx < 0 ){
        this->m_sprite->setFlipped(true);
    }else {
        this->m_sprite->setFlipped(false);
    }

    bounce();
}

void BiggerFish::draw() const {
    ofLogVerbose() << "BiggerFish at (" << m_x << ", " << m_y << ") with speed " << m_speed << std::endl;
    this->m_sprite->draw(this->m_x, this->m_y);
}

// Jellyfish Implementation
Jellyfish::Jellyfish(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
: NPCreature(x, y, speed, sprite) {
    m_dx = (rand() % 3 - 1);
    m_dy = (rand() % 3 - 1);
    normalize();
    setCollisionRadius(30);
    m_value = 2;
    m_creatureType = AquariumCreatureType::Jellyfish;
}

void Jellyfish::move() {
    // movimiento flotante con posibilidad de spike hacia arriba
    m_angle += 0.04f;
    m_x += m_dx * (m_speed * 0.6);

    // si no hay spike activo, movimiento senoidal vertical
    if (m_spikeCooldown <= 0) {
        m_y += sin(m_angle) * 0.8f;
        // chance de iniciar spike
        if ((rand() % 200) == 0) {
            m_dy = -6.0f; // impulso hacia arriba
            m_spikeCooldown = 40; // duracion del spike en frames
        }
    } else {
        // durante spike aplicar velocidad y gravedad ligera
        m_y += m_dy;
        m_dy += 0.25f; // gravedad suave
        --m_spikeCooldown;
    }

    if (m_dx < 0) this->m_sprite->setFlipped(true);
    else this->m_sprite->setFlipped(false);

    bounce();
}

void Jellyfish::draw() const {
    ofLogNotice() << "Drawing Jellyfish at (" << m_x << "," << m_y << ")" << std::endl;
    this->m_sprite->draw(this->m_x, this->m_y);
}

// Shark Implementation
Shark::Shark(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
: NPCreature(x, y, speed, sprite) {
    m_dx = (rand() % 3 - 1);
    m_dy = (rand() % 3 - 1);
    normalize();
    setCollisionRadius(80);
    m_value = 8;
    m_creatureType = AquariumCreatureType::Shark;
}

void Shark::move() {
    // intentar seguir al jugador si esta cerca
    auto playerPtr = s_player.lock();
    if (playerPtr) {
        float px = playerPtr->getX();
        float py = playerPtr->getY();
        float dx = px - m_x;
        float dy = py - m_y;
        float dist2 = dx*dx + dy*dy;
        if (dist2 < (m_detectionRadius * m_detectionRadius)) {
            float inv = 1.0f / sqrt(dist2 + 0.0001f);
            m_dx = dx * inv;
            m_dy = dy * inv;
            // movimiento rapido hacia el jugador
            m_x += m_dx * (m_speed * 1.2f);
            m_y += m_dy * (m_speed * 1.2f);
            if (m_dx < 0) this->m_sprite->setFlipped(true);
            else this->m_sprite->setFlipped(false);
            bounce();
            return;
        }
    }

    // comportamiento por defecto similar a NPCreature
    m_x += m_dx * m_speed;
    m_y += m_dy * m_speed;
    if (m_dx < 0) this->m_sprite->setFlipped(true);
    else this->m_sprite->setFlipped(false);
    bounce();
}

void Shark::draw() const {
    ofLogNotice() << "Drawing Shark at (" << m_x << "," << m_y << ")" << std::endl;
    this->m_sprite->draw(this->m_x, this->m_y);
}

void Shark::SetPlayer(std::shared_ptr<PlayerCreature> p) {
    Shark::s_player = p;
}


// AquariumSpriteManager
AquariumSpriteManager::AquariumSpriteManager(){
    this->m_npc_fish = std::make_shared<GameSprite>("base-fish.png", 70,70);
    this->m_big_fish = std::make_shared<GameSprite>("bigger-fish.png", 120, 120);
    this->m_jelly_fish = std::make_shared<GameSprite>("jellyfish.png", 90, 90);
    this->m_shark = std::make_shared<GameSprite>("shark.png", 140, 90);
    // powerup will be rendered procedurally no png needed
}

std::shared_ptr<GameSprite> AquariumSpriteManager::GetSprite(AquariumCreatureType t){
    switch(t){
        case AquariumCreatureType::BiggerFish:
            return std::make_shared<GameSprite>(*this->m_big_fish);
            
        case AquariumCreatureType::NPCreature:
            return std::make_shared<GameSprite>(*this->m_npc_fish);

        case AquariumCreatureType::PowerUp:
            // no sprite for powerup, return nullptr
            return nullptr;

        case AquariumCreatureType::Jellyfish:
            return std::make_shared<GameSprite>(*this->m_jelly_fish);
            
        case AquariumCreatureType::Shark:
            return std::make_shared<GameSprite>(*this->m_shark);

            
        default:
            return nullptr;
    }
}


// Aquarium Implementation
Aquarium::Aquarium(int width, int height, std::shared_ptr<AquariumSpriteManager> spriteManager)
    : m_width(width), m_height(height) {
        m_sprite_manager =  spriteManager;
    }



void Aquarium::addCreature(std::shared_ptr<Creature> creature) {
    creature->setBounds(m_width - 20, m_height - 20);
    m_creatures.push_back(creature);
}

void Aquarium::addAquariumLevel(std::shared_ptr<AquariumLevel> level){
    if(level == nullptr){return;} // guard to not add noise
    this->m_aquariumlevels.push_back(level);
}

void Aquarium::update() {
    for (auto& creature : m_creatures) {
        creature->move();
    }
    this->Repopulate();
}

void Aquarium::draw() const {
    for (const auto& creature : m_creatures) {
        creature->draw();
    }
}


void Aquarium::removeCreature(std::shared_ptr<Creature> creature) {
    auto it = std::find(m_creatures.begin(), m_creatures.end(), creature);
    if (it != m_creatures.end()) {
        ofLogVerbose() << "removing creature " << endl;
        int selectLvl = this->currentLevel % this->m_aquariumlevels.size();
        // only consume population counters for npc creatures
        if (auto npcCreature = std::dynamic_pointer_cast<NPCreature>(creature)) {
            this->m_aquariumlevels.at(selectLvl)->ConsumePopulation(npcCreature->GetType(), npcCreature->getValue());
        }
        m_creatures.erase(it);
    }
}

void Aquarium::clearCreatures() {
    m_creatures.clear();
}

std::shared_ptr<Creature> Aquarium::getCreatureAt(int index) {
    if (index < 0 || size_t(index) >= m_creatures.size()) {
        return nullptr;
    }
    return m_creatures[index];
}



void Aquarium::SpawnCreature(AquariumCreatureType type) {
    // Keep creatures away from edges
    int x = 50 + rand() % (this->getWidth() - 100);
    int y = 50 + rand() % (this->getHeight() - 100);
    int speed = 1 + rand() % 25; // Speed between 1 and 25

    switch (type) {
        case AquariumCreatureType::NPCreature:
            this->addCreature(std::make_shared<NPCreature>(x, y, speed, this->m_sprite_manager->GetSprite(AquariumCreatureType::NPCreature)));
            break;
        case AquariumCreatureType::BiggerFish:
            this->addCreature(std::make_shared<BiggerFish>(x, y, speed, this->m_sprite_manager->GetSprite(AquariumCreatureType::BiggerFish)));
            break;

        case AquariumCreatureType::PowerUp:
            // spawn powerup without sprite (procedural)
            this->addCreature(std::make_shared<PowerUpCreature>(x, y, nullptr));
            break;
        // jellyfish spawn
        case AquariumCreatureType::Jellyfish:
            this->addCreature(std::make_shared<Jellyfish>(
                x, y, speed,
                this->m_sprite_manager->GetSprite(AquariumCreatureType::Jellyfish)
            ));
            break;

        // tiburon implementation
        case AquariumCreatureType::Shark:
            this->addCreature(std::make_shared<Shark>(
                x, y, speed,
                this->m_sprite_manager->GetSprite(AquariumCreatureType::Shark)
            ));
            break;
        default:
            ofLogError() << "Unknown creature type to spawn!";
            break;
    }

}


// repopulation will be called from the levl class
// it will compose into aquarium so eating eats frm the pool of NPCs in the lvl class
// once lvl criteria met, we move to new lvl through inner signal asking for new lvl
// which will mean incrementing the buffer and pointing to a new lvl index
void Aquarium::Repopulate() {
    ofLogVerbose("entering phase repopulation");
    // lets make the levels circular
    int selectedLevelIdx = this->currentLevel % this->m_aquariumlevels.size();
    ofLogVerbose() << "the current index: " << selectedLevelIdx << endl;
    std::shared_ptr<AquariumLevel> level = this->m_aquariumlevels.at(selectedLevelIdx);


    if(level->isCompleted()){
        level->levelReset();
        this->currentLevel += 1;
        selectedLevelIdx = this->currentLevel % this->m_aquariumlevels.size();
        ofLogNotice()<<"new level reached : " << selectedLevelIdx << std::endl;
        level = this->m_aquariumlevels.at(selectedLevelIdx);
        this->clearCreatures();
    }

    
    // encontrar cuantos necesitamos reaparecer si es necesario 
    std::vector<AquariumCreatureType> toRespawn = level->Repopulate();
    ofLogVerbose() << "cantidad a repoblar : " << toRespawn.size() << endl;
    if(toRespawn.size() <= 0 ){return;} // no hay nada que hacer aqui
    for(AquariumCreatureType newCreatureType : toRespawn){
        this->SpawnCreature(newCreatureType);
    }
}


// Aquarium collision detection
std::shared_ptr<GameEvent> DetectAquariumCollisions(std::shared_ptr<Aquarium> aquarium, std::shared_ptr<PlayerCreature> player) {
    if (!aquarium || !player) return nullptr;
    
    for (int i = 0; i < aquarium->getCreatureCount(); ++i) {
        std::shared_ptr<Creature> npc = aquarium->getCreatureAt(i);
        if (npc && checkCollision(player, npc)) {
            return std::make_shared<GameEvent>(GameEventType::COLLISION, player, npc);
        }
    }
    return nullptr;
};

//  Imlementation of the AquariumScene

void AquariumGameScene::Update(){
    std::shared_ptr<GameEvent> event;

    // actualizar el jugador cada ciclo
    this->m_player->update();

    if (this->updateControl.tick()) {
        event = DetectAquariumCollisions(this->m_aquarium, this->m_player);
        if (event != nullptr && event->isCollisionEvent()) {
            ofLogVerbose() << "collision detected between player and npc" << std::endl;
            if (event->creatureB != nullptr) {
                event->print();

                if (auto npc = std::dynamic_pointer_cast<NPCreature>(event->creatureB)) {
                    // intento de comer criatura npc
                    if (this->m_player->getPower() < event->creatureB->getValue()) {
                        ofLogNotice() << "player is too weak to eat the creature" << std::endl;
                        this->m_player->loseLife(3*60);
                        if (this->m_player->getLives() <= 0) {
                            this->m_lastEvent = std::make_shared<GameEvent>(GameEventType::GAME_OVER, this->m_player, nullptr);
                            return;
                        }
                    } else {
                        this->m_aquarium->removeCreature(event->creatureB);
                        this->m_player->addToScore(1, event->creatureB->getValue());
                        m_chomp.play();
                        if (this->m_player->getScore() % 25 == 0) {
                            this->m_player->increasePower(1);
                            ofLogNotice() << "player power increased to " << this->m_player->getPower() << std::endl;
                        }
                    }
                } else if (auto powerup = std::dynamic_pointer_cast<PowerUpCreature>(event->creatureB)) {
                    // recoger powerup
                    this->m_aquarium->removeCreature(event->creatureB);
                    this->m_player->increasePower(PowerUpCreature::POWER_BOOST);
                    this->m_player->setCollisionRadius(this->m_player->getCollisionRadius() * PowerUpCreature::SIZE_BOOST);
                    ofLogNotice() << "powerup collected power increased to " << this->m_player->getPower() << std::endl;
                }
            } else {
                ofLogError() << "error: creatureb is null in collision event" << std::endl;
            }
        }

        this->m_aquarium->update();
    }

}

void AquariumGameScene::Draw() {
    this->m_player->draw();
    this->m_aquarium->draw();
    this->paintAquariumHUD();

    // Ensure music keeps playing (will automatically loop)
    if (!m_bgMusic.isPlaying()) {
        m_bgMusic.play();
    }
}


void AquariumGameScene::paintAquariumHUD(){
    float panelWidth = ofGetWindowWidth() - 150;
    ofDrawBitmapString("Score: " + std::to_string(this->m_player->getScore()), panelWidth, 20);
    ofDrawBitmapString("Power: " + std::to_string(this->m_player->getPower()), panelWidth, 30);
    ofDrawBitmapString("Lives: " + std::to_string(this->m_player->getLives()), panelWidth, 40);
    for (int i = 0; i < this->m_player->getLives(); ++i) {
        ofSetColor(ofColor::red);
        ofDrawCircle(panelWidth + i * 20, 50, 5);
    }
    ofSetColor(ofColor::white); // Reset color to white for other drawings
}

void AquariumLevel::populationReset(){
    for(auto node: this->m_levelPopulation){
        node->currentPopulation = 0; // need to reset the population to ensure they are made a new in the next level
    }
}

void AquariumLevel::ConsumePopulation(AquariumCreatureType creatureType, int power){
    for(std::shared_ptr<AquariumLevelPopulationNode> node: this->m_levelPopulation){
        ofLogVerbose() << "consuming from this level creatures" << endl;
        if(node->creatureType == creatureType){
            ofLogVerbose() << "-cosuming from type: " << AquariumCreatureTypeToString(node->creatureType) <<" , currPop: " << node->currentPopulation << endl;
            if(node->currentPopulation == 0){
                return;
            } 
            node->currentPopulation -= 1;
            ofLogVerbose() << "+cosuming from type: " << AquariumCreatureTypeToString(node->creatureType) <<" , currPop: " << node->currentPopulation << endl;
            this->m_level_score += power;
            return;
        }
    }
}

bool AquariumLevel::isCompleted(){
    return this->m_level_score >= this->m_targetScore;
}



std::vector<AquariumCreatureType> Level_0::Repopulate() {
    std::vector<AquariumCreatureType> toRepopulate;
    for(std::shared_ptr<AquariumLevelPopulationNode> node : this->m_levelPopulation){
        int delta = node->population - node->currentPopulation;
        ofLogVerbose() << "to Repopulate :  " << delta << endl;
        if(delta >0){
            for(int i = 0; i<delta; i++){
                toRepopulate.push_back(node->creatureType);
            }
            node->currentPopulation += delta;
        }
    }
    return toRepopulate;

}

std::vector<AquariumCreatureType> Level_1::Repopulate() {
    std::vector<AquariumCreatureType> toRepopulate;
    for(std::shared_ptr<AquariumLevelPopulationNode> node : this->m_levelPopulation){
        int delta = node->population - node->currentPopulation;
        if(delta >0){
            for(int i=0; i<delta; i++){
                toRepopulate.push_back(node->creatureType);
            }
            node->currentPopulation += delta;
        }
    }
    return toRepopulate;
}

std::vector<AquariumCreatureType> Level_2::Repopulate() {
    std::vector<AquariumCreatureType> toRepopulate;
    
    // probabilidad aleatoria de generar power-up (5% de probabilidad)
    if ((rand() % 100) < 5) {
        toRepopulate.push_back(AquariumCreatureType::PowerUp);
    }
    
    for(std::shared_ptr<AquariumLevelPopulationNode> node : this->m_levelPopulation){
        int delta = node->population - node->currentPopulation;
        if(delta > 0){
            for(int i = 0; i < delta; i++){
                toRepopulate.push_back(node->creatureType);
            }
            node->currentPopulation += delta;
        }
    }
    return toRepopulate;
}
