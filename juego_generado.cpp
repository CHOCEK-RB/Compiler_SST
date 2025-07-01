#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <variant>
#include <optional>

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

class SceneComponent {
public:
  virtual ~SceneComponent() = default;
  virtual void draw(sf::RenderWindow &window) = 0;
  virtual void setVisibility(bool visible) { }
  virtual void setTransform(const struct Transform& t) { }
};

struct Transform {
  sf::Vector2f position = {0.0f, 0.0f};
  sf::Vector2f scale = {1.0f, 1.0f};
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
  void setTransform(const Transform &transform) override {
    transform_ = transform;
    if (sprite_) {
      sprite_->setPosition(transform_.position);
      sprite_->setScale(transform_.scale);
    }
  }
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
  void setTransform(const Transform& t) override {
      if(states_.count(currentState_)) states_[currentState_]->getSprite()->setTransform(t);
  }
  const std::string& getName() const { return name_; }
};

class Background : public SceneComponent {
  std::shared_ptr<sf::Texture> texture_;
  std::unique_ptr<sf::Sprite> sprite_;
  bool isVisible_ = false;
public:
  Background(const std::string &texturePath, const Transform &transform) {
    texture_ = TextureManager::getInstance().loadTexture(texturePath);
    if (texture_) {
      sprite_ = std::make_unique<sf::Sprite>(*texture_);
    }
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
public:
  DialogueSystem(const sf::Font &font) : dialogueText_(font, "") {
    textBox_.setSize({1200, 200});
    textBox_.setPosition({0, 400});
    textBox_.setFillColor(sf::Color(0, 0, 0, 200));
    dialogueText_.setCharacterSize(28);
    dialogueText_.setFillColor(sf::Color::White);
    dialogueText_.setPosition({50, 430});
  }
  void setText(const std::string &text) { dialogueText_.setString(text); isVisible_ = true; }
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
};

// --- Definiciones de Comandos de la Historia ---
struct DialogueCmd { std::string speakerId; std::string text; };
struct ShowCmd { std::string characterId; std::string mode; Transform transform; };
struct HideCmd { std::string characterId; };
struct SceneCmd { std::string backgroundName; };
using StoryCommand = std::variant<DialogueCmd, ShowCmd, HideCmd, SceneCmd>;

class VisualNovelEngine {
public:
  void initialize() {
    window_.create(sf::VideoMode({1200, 600}), "visualNovel");
    window_.setPosition({100, 100});
    window_.setFramerateLimit(60);
    if (!font_.openFromFile("assets/fonts/CaskaydiaCoveNerdFont-Regular.ttf")) { std::cerr << "Error: No se pudo cargar la fuente.\n"; return; }

    // --- CORRECCIÓN DE ORDEN DE DIBUJADO ---
    createAssets(); // Primero crear y añadir fondos/personajes

    dialogueSystem_ = std::make_shared<DialogueSystem>(font_);
    sceneManager_.addComponent("dialogueSystem", dialogueSystem_); // Añadir el diálogo al final para que se dibuje encima

    buildStoryScript();
    if (!storyScript_.empty()) executeNextCommand();
  }

  void run() {
    while (window_.isOpen()) {
      handleEvents();
      render();
    }
  }

private:
  sf::RenderWindow window_;
  sf::Font font_;
  SceneManager sceneManager_;
  std::map<std::string, std::shared_ptr<Character>> characters_;
  std::map<std::string, std::shared_ptr<Background>> backgrounds_;
  std::shared_ptr<DialogueSystem> dialogueSystem_;
  std::vector<StoryCommand> storyScript_;
  size_t commandIndex_ = 0;
  std::string currentBackground_;

  void createAssets() {
        {
            Transform transform;
            auto bg = std::make_shared<Background>("./assets/backgrounds/cerezos.jpg", transform);
            backgrounds_["ciudad_noche"] = bg;
            sceneManager_.addComponent("bg_" + std::string("ciudad_noche"), bg);
        }
        {
            auto character = std::make_shared<Character>("ana", "Ana");
                {
                    Transform transform;
                    transform.scale = { (float)1.1, (float)1.1 };
                    character->addState("happy", "./assets/characters/maid/despreocupada.png", transform);
                }
                {
                    Transform transform;
                    transform.scale = { (float)1.1, (float)1.1 };
                    character->addState("angry", "./assets/characters/maid/enojada.png", transform);
                }
            characters_["ana"] = character;
            sceneManager_.addComponent("char_" + std::string("ana"), character);
        }
  }

  void buildStoryScript() {
        storyScript_.push_back(SceneCmd{"ciudad_noche"});
        {
            Transform t;
            t.position.y = 100;
            t.position.x = 10;
            storyScript_.push_back(ShowCmd{"ana", "happy", t});
        }
        storyScript_.push_back(DialogueCmd{"ana", R"(**Hola!** Como estas? ~~No me ignores~~)"});
        storyScript_.push_back(DialogueCmd{"You", R"(Estoy bien)"});
        storyScript_.push_back(DialogueCmd{"ana", R"(Que alegria!)"});
        storyScript_.push_back(HideCmd{"ana"});
  }

  void executeNextCommand() {
    if (commandIndex_ >= storyScript_.size()) { dialogueSystem_->hide(); return; }

    const auto& command = storyScript_[commandIndex_];
    std::visit([this](auto&& arg) {
      using T = std::decay_t<decltype(arg)>;
      if constexpr (std::is_same_v<T, SceneCmd>) {
        if (backgrounds_.count(currentBackground_)) backgrounds_[currentBackground_]->setVisibility(false);
        if (backgrounds_.count(arg.backgroundName)) backgrounds_[arg.backgroundName]->setVisibility(true);
        currentBackground_ = arg.backgroundName;
      } else if constexpr (std::is_same_v<T, ShowCmd>) {
        if (auto it = characters_.find(arg.characterId); it != characters_.end()) {
          it->second->setState(arg.mode);
          it->second->setTransform(arg.transform);
          it->second->setVisibility(true);
        }
      } else if constexpr (std::is_same_v<T, HideCmd>) {
        if (auto it = characters_.find(arg.characterId); it != characters_.end()) { it->second->setVisibility(false); }
      } else if constexpr (std::is_same_v<T, DialogueCmd>) {
        std::string speakerName = (arg.speakerId == "You") ? "" : arg.speakerId;
        if (auto it = characters_.find(arg.speakerId); it != characters_.end()) { speakerName = it->second->getName(); }
        dialogueSystem_->setText(speakerName.empty() ? arg.text : speakerName + ":\n" + arg.text);
      }
    }, command);

    commandIndex_++;
  }

  void handleEvents() {
    while (std::optional<sf::Event> event = window_.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window_.close();
        }
        if (auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
            if (keyPressed->code == sf::Keyboard::Key::Space) {
                executeNextCommand();
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
