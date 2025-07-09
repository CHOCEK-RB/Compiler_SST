#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <variant>
#include <optional>

constexpr int WINDOW_WEIGHT = 1600;
constexpr int WINDOW_HEIGHT = 800;
constexpr int TEXT_BOX_POSX = 0;
constexpr int TEXT_BOX_POSY = 500;
constexpr int TEXT_BOX_WEIGHT = WINDOW_WEIGHT;
constexpr int TEXT_BOX_HEIGHT = WINDOW_HEIGHT - TEXT_BOX_POSY;
constexpr int DIALOGUE_SIZE = 36;
constexpr int DIALOGUE_POSX = TEXT_BOX_POSX + 20;
constexpr int DIALOGUE_POSY = TEXT_BOX_POSY + 20;
constexpr int TEXT_BOX_PADDING = 200;
// --- Clases del Motor de la Novela Visual ---

class TextureManager {
private:
  std::map<std::string, std::shared_ptr<sf::Texture>> textures;
  TextureManager() = default;
public:
  static TextureManager &getInstance() {
    static TextureManager instance;
    return instance;
  }
  std::shared_ptr<sf::Texture> loadTexture(const std::string &path) {
    if (textures.count(path)) { return textures[path]; }
    auto texture = std::make_shared<sf::Texture>();
    if (!texture->loadFromFile(path)) {
      std::cerr << "Error cargando textura: " << path << std::endl;
      return nullptr;
    }
    textures[path] = texture;
    return texture;
  }
};

struct Transform {
  sf::Vector2f position = {0.0f, 0.0f};
  sf::Vector2f scale = {1.0f, 1.0f};
};

class SceneComponent {
public:
  virtual ~SceneComponent() = default;
  virtual void draw(sf::RenderWindow &window) = 0;
  virtual void setVisibility(bool visible) { }
  virtual void setPosition(const sf::Vector2f& pos) { }
  virtual void setScale(const sf::Vector2f& scale) { }
  virtual void setFocused(bool isFocused) { }
  virtual void update(float deltaTime) { }
};

class SpriteComponent : public SceneComponent {
  std::shared_ptr<sf::Texture> texture_;
  std::unique_ptr<sf::Sprite> sprite_;
  Transform transform_;
  bool isVisible_ = true;
public:
  SpriteComponent(std::shared_ptr<sf::Texture> texture, const Transform &transform) : texture_(texture), transform_(transform) {
    if (texture_) {
      sprite_ = std::make_unique<sf::Sprite>(*texture_);
      sprite_->setPosition(transform_.position);
      sprite_->setScale(transform_.scale);
    } else { std::cerr << "Error: SpriteComponent creado con textura nula.\n"; }
  }
  void draw(sf::RenderWindow &window) override { if (isVisible_ && sprite_) window.draw(*sprite_); }
  void setVisibility(bool visible) override { isVisible_ = visible; }
  void setPosition(const sf::Vector2f& pos) override { transform_.position = pos; if (sprite_) sprite_->setPosition(pos); }
  void setScale(const sf::Vector2f& scale) override { transform_.scale = scale; if (sprite_) sprite_->setScale(scale); }
  void setFocused(bool isFocused) override { if(sprite_) sprite_->setColor(isFocused ? sf::Color::White : sf::Color(128, 128, 128)); }
};

class CharacterState {
public:
  virtual ~CharacterState() = default;
  virtual std::shared_ptr<SpriteComponent> getSprite() = 0;
};

class GenericCharacterState : public CharacterState {
  std::shared_ptr<SpriteComponent> sprite_;
public:
  GenericCharacterState(const std::string &texturePath, const Transform &transform) {
    auto texture = TextureManager::getInstance().loadTexture(texturePath);
    sprite_ = std::make_shared<SpriteComponent>(texture, transform);
  }
  std::shared_ptr<SpriteComponent> getSprite() override { return sprite_; }
};

class Character : public SceneComponent {
  std::string id_, name_;
  std::string currentState_;
  std::map<std::string, std::unique_ptr<CharacterState>> states_;
  bool isVisible_ = false;
public:
  Character(const std::string &id, const std::string &name) : id_(id), name_(name) {}
  void addState(const std::string &stateName, const std::string &texturePath, const Transform &transform) {
    states_[stateName] = std::make_unique<GenericCharacterState>(texturePath, transform);
  }
  void setState(const std::string &stateName) {
    if (states_.count(stateName)) { currentState_ = stateName; }
  }
  void draw(sf::RenderWindow &window) override {
    if (isVisible_ && states_.count(currentState_)) { states_[currentState_]->getSprite()->draw(window); }
  }
  void setVisibility(bool visible) override { isVisible_ = visible; }
  void setPosition(const sf::Vector2f& pos) override { if (states_.count(currentState_)) states_[currentState_]->getSprite()->setPosition(pos); }
  void setScale(const sf::Vector2f& scale) override { if (states_.count(currentState_)) states_[currentState_]->getSprite()->setScale(scale); }
  void setFocused(bool isFocused) override { if (states_.count(currentState_)) states_[currentState_]->getSprite()->setFocused(isFocused); }
  const std::string& getName() const { return name_; }
};

class Background : public SceneComponent {
  std::shared_ptr<sf::Texture> texture_;
  std::unique_ptr<sf::Sprite> sprite_;
  bool isVisible_ = false;
public:
  Background(const std::string &texturePath, const Transform &transform) {
    texture_ = TextureManager::getInstance().loadTexture(texturePath);
    if (texture_) { sprite_ = std::make_unique<sf::Sprite>(*texture_); }
  }
  void draw(sf::RenderWindow &window) override {
    if (isVisible_ && sprite_) {
      sf::Vector2u windowSize = window.getSize();
      sf::Vector2u texSize = texture_->getSize();
      float scaleX = static_cast<float>(windowSize.x) / texSize.x;
      float scaleY = static_cast<float>(windowSize.y) / texSize.y;
      sprite_->setScale({scaleX, scaleY});
      window.draw(*sprite_);
    }
  }
  void setVisibility(bool visible) override { isVisible_ = visible; }
};

class DialogueSystem : public SceneComponent {
  sf::RectangleShape textBox_;
  sf::Text dialogueText_;
  bool isVisible_ = false;
  std::string fullText_;
  std::string currentTypedText_;
  size_t charIndex_ = 0;
  float timePerChar_ = 0.05f;
  float elapsedTime_ = 0.0f;
  bool isTyping_ = false;
  std::string wrapText(const std::string& text, unsigned int lineLength, const sf::Font& font, unsigned int charSize) {
      std::string wrappedText;
      std::string currentLine;
      std::string word;
      sf::Text tempText(font, "", charSize);
      for (char c : text) {
          if (c == ' ' || c == '\n') {
              tempText.setString(currentLine + word + ' ');
              if (tempText.getLocalBounds().size.x > lineLength) {
                  wrappedText += currentLine + '\n';
                  currentLine = word + ' ';
              } else {
                  currentLine += word + ' ';
              }
              word.clear();
              if (c == '\n') {
                  wrappedText += currentLine;
                  currentLine.clear();
              }
          } else {
              word += c;
          }
      }
      tempText.setString(currentLine + word);
      if (tempText.getLocalBounds().size.x > lineLength) {
          wrappedText += currentLine + '\n' + word;
      } else {
          wrappedText += currentLine + word;
      }
      return wrappedText;
  }

public:
  DialogueSystem(const sf::Font &font) : dialogueText_(font, "") {
    textBox_.setSize({TEXT_BOX_WEIGHT, TEXT_BOX_HEIGHT});
    textBox_.setPosition({TEXT_BOX_POSX, TEXT_BOX_POSY});
    textBox_.setFillColor(sf::Color(0, 0, 0, 200));
    dialogueText_.setCharacterSize(DIALOGUE_SIZE);
    dialogueText_.setFillColor(sf::Color::White);
    dialogueText_.setPosition({DIALOGUE_POSX, DIALOGUE_POSY});
  }

  void start(const std::string &text, float speed) {
    fullText_ = wrapText(text, TEXT_BOX_WEIGHT - TEXT_BOX_PADDING, dialogueText_.getFont(), dialogueText_.getCharacterSize());
    currentTypedText_.clear();
    charIndex_ = 0;
    elapsedTime_ = 0.0f;
    timePerChar_ = (speed > 0) ? 1.0f / speed : 0.0f;
    isTyping_ = true;
    isVisible_ = true;
    dialogueText_.setString("");
  }

  void update(float deltaTime) override {
    if (!isTyping_ || charIndex_ >= fullText_.length()) return;
    elapsedTime_ += deltaTime;
    if (elapsedTime_ >= timePerChar_) {
      elapsedTime_ = 0.0f;
      currentTypedText_ += fullText_[charIndex_];
      dialogueText_.setString(currentTypedText_);
      charIndex_++;
      if (charIndex_ >= fullText_.length()) {
        isTyping_ = false;
      }
    }
  }

  void finish() {
    if (isTyping_) {
      isTyping_ = false;
      charIndex_ = fullText_.length();
      currentTypedText_ = fullText_;
      dialogueText_.setString(fullText_);
    }
  }

  bool isFinished() const { return !isTyping_; }
  void hide() { isVisible_ = false; }
  void draw(sf::RenderWindow &window) override {
    if (isVisible_) { window.draw(textBox_); window.draw(dialogueText_); }
  }
};

class SceneManager {
  std::vector<std::shared_ptr<SceneComponent>> components_;
public:
  void addComponent(const std::string& id, std::shared_ptr<SceneComponent> component) {
    components_.push_back(component);
  }
  void draw(sf::RenderWindow &window) {
    for (auto &comp : components_) { comp->draw(window); }
  }
  void update(float deltaTime) {
    for (auto &comp : components_) { comp->update(deltaTime); }
  }
};

// --- Definiciones de Comandos de la Historia ---
struct DialogueCmd { std::string speakerId; std::string text; float speed; };
struct ShowCmd { std::string characterId; std::string mode; Transform transform; bool scale_overridden; };
struct HideCmd { std::string characterId; };
struct SceneCmd { std::string backgroundName; };
struct PlayCmd { std::string musicId; };
struct StopCmd { std::string musicId; };
using StoryCommand = std::variant<DialogueCmd, ShowCmd, HideCmd, SceneCmd, PlayCmd, StopCmd>;

class VisualNovelEngine {
public:
  enum class State { IDLE, EXECUTING_COMMAND, WRITING_DIALOGUE, WAITING_FOR_INPUT };

  void initialize() {
    window_.create(sf::VideoMode({WINDOW_WEIGHT, WINDOW_HEIGHT}), "visualNovel");
    window_.setPosition({100, 100});
    window_.setFramerateLimit(60);
    if (!font_.openFromFile("assets/fonts/CaskaydiaCoveNerdFont-Regular.ttf")) { std::cerr << "Error: No se pudo cargar la fuente.\n"; return; }

    createAssets();
    dialogueSystem_ = std::make_shared<DialogueSystem>(font_);
    sceneManager_.addComponent("dialogueSystem", dialogueSystem_);

    buildStoryScript();
    if (!storyScript_.empty()) { currentState_ = State::EXECUTING_COMMAND; }
  }

  void run() {
    sf::Clock clock;
    while (window_.isOpen()) {
      sf::Time elapsed = clock.restart();
      handleEvents();
      update(elapsed.asSeconds());
      render();
    }
  }

private:
  State currentState_ = State::IDLE;
  sf::RenderWindow window_;
  sf::Font font_;
  SceneManager sceneManager_;

  std::map<std::string, std::shared_ptr<Character>> characters_;
  std::map<std::string, std::shared_ptr<Background>> backgrounds_;
  std::map<std::string, std::shared_ptr<sf::Music>> musicTracks_;

  std::shared_ptr<DialogueSystem> dialogueSystem_;
  std::vector<StoryCommand> storyScript_;
  size_t commandIndex_ = 0;
  std::string currentBackground_;
  std::string currentMusicId_;
  void createAssets() {
        {
            Transform transform;
            auto bg = std::make_shared<Background>("./assets/backgrounds/smp_front_evening2.png", transform);
            backgrounds_["patio_escuela"] = bg;
            sceneManager_.addComponent("bg_" + std::string("patio_escuela"), bg);
        }
        {
            Transform transform;
            auto bg = std::make_shared<Background>("./assets/backgrounds/smp_classroom1_evening1.png", transform);
            backgrounds_["salon_clases"] = bg;
            sceneManager_.addComponent("bg_" + std::string("salon_clases"), bg);
        }
        {
            Transform transform;
            auto bg = std::make_shared<Background>("./assets/backgrounds/smp_roof_evening1.png", transform);
            backgrounds_["azotea"] = bg;
            sceneManager_.addComponent("bg_" + std::string("azotea"), bg);
        }
        {
            auto music = std::make_shared<sf::Music>();
            if (music->openFromFile("./assets/music/Relax.wav")) {
                musicTracks_["tema_principal"] = music;
            } else { std::cerr << "Error al cargar música: ./assets/music/Relax.wav\n"; }
        }
        {
            auto music = std::make_shared<sf::Music>();
            if (music->openFromFile("./assets/music/Death.wav")) {
                musicTracks_["tema_melancolico"] = music;
            } else { std::cerr << "Error al cargar música: ./assets/music/Death.wav\n"; }
        }
        {
            auto character = std::make_shared<Character>("hiro", "Hiroshi");
                {
                    Transform transform;
                    transform.scale = { (float)0.7, (float)0.7 };
                    character->addState("normal", "./assets/characters/hiro/hiro_normal.png", transform);
                }
                {
                    Transform transform;
                    transform.scale = { (float)0.7, (float)0.7 };
                    character->addState("preocupado", "./assets/characters/hiro/hiro_preocupado.png", transform);
                }
                {
                    Transform transform;
                    transform.scale = { (float)0.7, (float)0.7 };
                    character->addState("sonrojado", "./assets/characters/hiro/hiro_sonrojado.png", transform);
                }
            characters_["hiro"] = character;
            sceneManager_.addComponent("char_" + std::string("hiro"), character);
        }
        {
            auto character = std::make_shared<Character>("akira", "Akira");
                {
                    Transform transform;
                    transform.scale = { (float)0.7, (float)0.7 };
                    character->addState("energetico", "./assets/characters/akira/akira_energetico.png", transform);
                }
                {
                    Transform transform;
                    transform.scale = { (float)0.7, (float)0.7 };
                    character->addState("riendo", "./assets/characters/akira/akira_riendo.png", transform);
                }
            characters_["akira"] = character;
            sceneManager_.addComponent("char_" + std::string("akira"), character);
        }
        {
            auto character = std::make_shared<Character>("yuki", "Yuki");
                {
                    Transform transform;
                    transform.scale = { (float)0.7, (float)0.7 };
                    character->addState("seria", "./assets/characters/yuki/yuki_seria.png", transform);
                }
                {
                    Transform transform;
                    transform.scale = { (float)0.7, (float)0.7 };
                    character->addState("sonriendo", "./assets/characters/yuki/yuri_sonriendo.png", transform);
                }
            characters_["yuki"] = character;
            sceneManager_.addComponent("char_" + std::string("yuki"), character);
        }
  }

  void buildStoryScript() {
        storyScript_.push_back(SceneCmd{"salon_clases"});
        storyScript_.push_back(PlayCmd{"tema_principal"});
        {
            Transform t;
            bool scale_overridden = false;
            t.position.y = 0;
            t.position.x = 0;
            storyScript_.push_back(ShowCmd{"hiro", "preocupado", t, scale_overridden});
        }
        {
            float speed = 30.0f; // Velocidad por defecto
            storyScript_.push_back(DialogueCmd{"You", R"(El timbre sono, marcando el final de las clases. Todos recogian sus cosas, pero mi mente estaba en otro lugar.)", speed});
        }
        {
            Transform t;
            bool scale_overridden = false;
            t.position.y = 0;
            t.position.x = 700;
            storyScript_.push_back(ShowCmd{"akira", "energetico", t, scale_overridden});
        }
        {
            float speed = 30.0f; // Velocidad por defecto
            storyScript_.push_back(DialogueCmd{"akira", R"(Hiro! Vamos por un helado? El dia esta perfecto para eso!)", speed});
        }
        {
            float speed = 30.0f; // Velocidad por defecto
            storyScript_.push_back(DialogueCmd{"hiro", R"(Eh... no lo se, Akira. No tengo mucho animo hoy.)", speed});
        }
        {
            float speed = 30.0f; // Velocidad por defecto
            storyScript_.push_back(DialogueCmd{"akira", R"(Vamos, no puedes estar todo el dia con esa cara. Es por el examen que tenemos?)", speed});
        }
        {
            float speed = 30.0f; // Velocidad por defecto
            storyScript_.push_back(DialogueCmd{"hiro", R"(No es solo eso... es algo mas.)", speed});
        }
        {
            Transform t;
            bool scale_overridden = false;
            t.position.y = 0;
            t.position.x = 700;
            storyScript_.push_back(ShowCmd{"akira", "riendo", t, scale_overridden});
        }
        {
            float speed = 30.0f; // Velocidad por defecto
            storyScript_.push_back(DialogueCmd{"akira", R"(Pues con mas razon! Nada que un pn de doble chocolate no pueda arreglar. Vamos!)", speed});
        }
        storyScript_.push_back(HideCmd{"hiro"});
        storyScript_.push_back(HideCmd{"akira"});
        storyScript_.push_back(SceneCmd{"azotea"});
        storyScript_.push_back(StopCmd{"tema_principal"});
        storyScript_.push_back(PlayCmd{"tema_melancolico"});
        {
            float speed = 30.0f; // Velocidad por defecto
            storyScript_.push_back(DialogueCmd{"You", R"(Akira me convencio, pero antes de irnos, subi a la azotea. Necesitaba un momento para pensar.)", speed});
        }
        {
            float speed = 30.0f; // Velocidad por defecto
            storyScript_.push_back(DialogueCmd{"You", R"(No esperaba encontrar a nadie mas aqui.)", speed});
        }
        {
            Transform t;
            bool scale_overridden = false;
            t.position.y = 0;
            t.position.x = 700;
            storyScript_.push_back(ShowCmd{"yuki", "seria", t, scale_overridden});
        }
        {
            float speed = 30.0f; // Velocidad por defecto
            storyScript_.push_back(DialogueCmd{"You", R"(Yuki, la chica mas callada de la clase, estaba mirando el horizonte. Parecia... triste.)", speed});
        }
        {
            Transform t;
            bool scale_overridden = false;
            t.position.y = 0;
            t.position.x = 0;
            storyScript_.push_back(ShowCmd{"hiro", "normal", t, scale_overridden});
        }
        {
            float speed = 30.0f; // Velocidad por defecto
            storyScript_.push_back(DialogueCmd{"hiro", R"(Oh, Yuki... no sabia que estabas aqui. Disculpa.)", speed});
        }
        {
            float speed = 30.0f; // Velocidad por defecto
            storyScript_.push_back(DialogueCmd{"yuki", R"(No te preocupes. A veces vengo aqui para estar sola.)", speed});
        }
        {
            float speed = 30.0f; // Velocidad por defecto
            storyScript_.push_back(DialogueCmd{"hiro", R"(Te entiendo perfectamente.)", speed});
        }
        {
            float speed = 30.0f; // Velocidad por defecto
            storyScript_.push_back(DialogueCmd{"You", R"(Hubo un silencio, pero no era incomodo. Era raramente reconfortante.)", speed});
        }
        {
            Transform t;
            bool scale_overridden = false;
            t.position.y = 0;
            t.position.x = 700;
            storyScript_.push_back(ShowCmd{"yuki", "sonriendo", t, scale_overridden});
        }
        {
            float speed = 30.0f; // Velocidad por defecto
            speed = static_cast<float>(25);
            storyScript_.push_back(DialogueCmd{"yuki", R"(Tu tambien pareces preocupado, Hiroshi. Sabes? A veces, hablar de las cosas ayuda.)", speed});
        }
        {
            Transform t;
            bool scale_overridden = false;
            t.position.y = 0;
            t.position.x = 0;
            storyScript_.push_back(ShowCmd{"hiro", "sonrojado", t, scale_overridden});
        }
        {
            float speed = 30.0f; // Velocidad por defecto
            storyScript_.push_back(DialogueCmd{"You", R"(Su sonrisa fue tan inesperada que senti como se me calentaban las mejillas.)", speed});
        }
        {
            float speed = 30.0f; // Velocidad por defecto
            storyScript_.push_back(DialogueCmd{"You", R"(Quizas... quizas tenia razon.)", speed});
        }
        storyScript_.push_back(StopCmd{"tema_melancolico"});
  }

  void update(float deltaTime) {
    if (currentState_ == State::EXECUTING_COMMAND) {
      executeNextCommand();
    }
    sceneManager_.update(deltaTime);
  }

  void executeNextCommand() {
    if (commandIndex_ >= storyScript_.size()) { dialogueSystem_->hide(); currentState_ = State::IDLE; return; }

    const auto& command = storyScript_[commandIndex_];
    std::visit([this](auto&& arg) {
      using T = std::decay_t<decltype(arg)>;
      if constexpr (std::is_same_v<T, DialogueCmd>) {
        std::string speakerName = (arg.speakerId == "You") ? "" : arg.speakerId;
        if (auto it = characters_.find(arg.speakerId); it != characters_.end()) { speakerName = it->second->getName(); }
        for(auto const& [id, character] : characters_) { character->setFocused(id == arg.speakerId || arg.speakerId == "You"); }
        dialogueSystem_->start(speakerName.empty() ? arg.text : speakerName + ":\n" + arg.text, arg.speed);
        currentState_ = State::WRITING_DIALOGUE;
      } else {
         for(auto const& [id, character] : characters_) { character->setFocused(true); }
         if constexpr (std::is_same_v<T, SceneCmd>) {
           if (backgrounds_.count(currentBackground_)) backgrounds_[currentBackground_]->setVisibility(false);
           if (backgrounds_.count(arg.backgroundName)) backgrounds_[arg.backgroundName]->setVisibility(true);
           currentBackground_ = arg.backgroundName;
         } else if constexpr (std::is_same_v<T, ShowCmd>) {
           if (auto it = characters_.find(arg.characterId); it != characters_.end()) {
             it->second->setState(arg.mode);
             it->second->setPosition(arg.transform.position);
             if (arg.scale_overridden) {
                it->second->setScale(arg.transform.scale);
             }
             it->second->setVisibility(true);
           }
         } else if constexpr (std::is_same_v<T, HideCmd>) {
           if (auto it = characters_.find(arg.characterId); it != characters_.end()) { it->second->setVisibility(false);
           }
         } else if constexpr (std::is_same_v<T, PlayCmd>) {
           if (!currentMusicId_.empty() && musicTracks_.count(currentMusicId_)) {
             musicTracks_[currentMusicId_]->stop();
           }
           if (musicTracks_.count(arg.musicId)) {
             currentMusicId_ = arg.musicId;
             musicTracks_[currentMusicId_]->setLooping(true);
             musicTracks_[currentMusicId_]->play();
           }
         } else if constexpr (std::is_same_v<T, StopCmd>) {
           if (musicTracks_.count(arg.musicId)) {
             musicTracks_[arg.musicId]->stop();
             if (currentMusicId_ == arg.musicId) {
               currentMusicId_.clear();
             }
           }
         }
         currentState_ = State::EXECUTING_COMMAND;
      }
    }, command);

    commandIndex_++;
  }

  void handleEvents() {
    while (std::optional<sf::Event> event = window_.pollEvent()) {
        if (event->is<sf::Event::Closed>()) { window_.close(); }
        if (auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
            if (keyPressed->code == sf::Keyboard::Key::Space) {
                if (currentState_ == State::WRITING_DIALOGUE) {
                    dialogueSystem_->finish();
                    currentState_ = State::WAITING_FOR_INPUT;
                } else if (currentState_ == State::WAITING_FOR_INPUT) {
                    currentState_ = State::EXECUTING_COMMAND;
                }
            }
        }
    }
  }

  void render() { window_.clear(sf::Color::Black); sceneManager_.draw(window_); window_.display(); }
};

int main() {
    VisualNovelEngine engine;
    engine.initialize();
    engine.run();
    return 0;
}
