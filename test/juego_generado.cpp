
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "nlohmann_json.hpp"

using json = nlohmann::json;

// --- Constantes de Configuración ---
constexpr int WINDOW_WIDTH = 1600;
constexpr int WINDOW_HEIGHT = 800;
constexpr int TEXT_BOX_POSX = 0;
constexpr int TEXT_BOX_POSY = 500;
constexpr int TEXT_BOX_WIDTH = WINDOW_WIDTH;
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
    if (textures.count(path)) {
      return textures[path];
    }
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
  virtual void setVisibility(bool visible) {}
  virtual void setPosition(const sf::Vector2f &pos) {}
  virtual void setScale(const sf::Vector2f &scale) {}
  virtual void setFocused(bool isFocused) {}
  virtual void update(float deltaTime) {}
};

class SpriteComponent : public SceneComponent {
  std::shared_ptr<sf::Texture> texture_;
  std::unique_ptr<sf::Sprite> sprite_;
  Transform transform_;
  bool isVisible_ = true;

public:
  SpriteComponent(std::shared_ptr<sf::Texture> texture,
                  const Transform &transform)
      : texture_(texture), transform_(transform) {
    if (texture_) {
      sprite_ = std::make_unique<sf::Sprite>(*texture_);
      sprite_->setPosition(transform_.position);
      sprite_->setScale(transform_.scale);
    } else {
      std::cerr << "Error: SpriteComponent creado con textura nula.\n";
    }
  }
  void draw(sf::RenderWindow &window) override {
    if (isVisible_ && sprite_)
      window.draw(*sprite_);
  }
  void setVisibility(bool visible) override { isVisible_ = visible; }
  void setPosition(const sf::Vector2f &pos) override {
    transform_.position = pos;
    if (sprite_)
      sprite_->setPosition(pos);
  }
  void setScale(const sf::Vector2f &scale) override {
    transform_.scale = scale;
    if (sprite_)
      sprite_->setScale(scale);
  }
  void setFocused(bool isFocused) override {
    if (sprite_)
      sprite_->setColor(isFocused ? sf::Color::White
                                  : sf::Color(128, 128, 128));
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
  GenericCharacterState(const std::string &texturePath,
                        const Transform &transform) {
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
  Character(const std::string &id, const std::string &name)
      : id_(id), name_(name) {}
  void addState(const std::string &stateName, const std::string &texturePath,
                const Transform &transform) {
    states_[stateName] =
        std::make_unique<GenericCharacterState>(texturePath, transform);
  }
  void setState(const std::string &stateName) {
    if (states_.count(stateName)) {
      currentState_ = stateName;
    }
  }
  void draw(sf::RenderWindow &window) override {
    if (isVisible_ && states_.count(currentState_)) {
      states_[currentState_]->getSprite()->draw(window);
    }
  }
  void setVisibility(bool visible) override { isVisible_ = visible; }
  void setPosition(const sf::Vector2f &pos) override {
    if (states_.count(currentState_))
      states_[currentState_]->getSprite()->setPosition(pos);
  }
  void setScale(const sf::Vector2f &scale) override {
    if (states_.count(currentState_))
      states_[currentState_]->getSprite()->setScale(scale);
  }
  void setFocused(bool isFocused) override {
    if (states_.count(currentState_))
      states_[currentState_]->getSprite()->setFocused(isFocused);
  }
  const std::string &getName() const { return name_; }
};

class Background : public SceneComponent {
  std::shared_ptr<sf::Texture> texture_;
  std::unique_ptr<sf::Sprite> sprite_;
  bool isVisible_ = false;

public:
  Background(const std::string &texturePath) {
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
  std::string fullText_;
  std::string currentTypedText_;
  size_t charIndex_ = 0;
  float timePerChar_ = 0.05f;
  float elapsedTime_ = 0.0f;
  bool isTyping_ = false;

  std::string wrapText(const std::string &text, unsigned int lineLength,
                       const sf::Font &font, unsigned int charSize) {
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
    textBox_.setSize({(float)TEXT_BOX_WIDTH, (float)TEXT_BOX_HEIGHT});
    textBox_.setPosition({(float)TEXT_BOX_POSX, (float)TEXT_BOX_POSY});
    textBox_.setFillColor(sf::Color(0, 0, 0, 200));
    dialogueText_.setCharacterSize(DIALOGUE_SIZE);
    dialogueText_.setFillColor(sf::Color::White);
    dialogueText_.setPosition({(float)DIALOGUE_POSX, (float)DIALOGUE_POSY});
  }

  void start(const std::string &text, float speed) {
    fullText_ =
        wrapText(text, TEXT_BOX_WIDTH - TEXT_BOX_PADDING,
                 dialogueText_.getFont(), dialogueText_.getCharacterSize());
    currentTypedText_.clear();
    charIndex_ = 0;
    elapsedTime_ = 0.0f;
    timePerChar_ = (speed > 0) ? 1.0f / speed : 0.0f;
    isTyping_ = true;
    isVisible_ = true;
    dialogueText_.setString("");
  }

  void update(float deltaTime) override {
    if (!isTyping_ || charIndex_ >= fullText_.length())
      return;
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
    if (isVisible_) {
      window.draw(textBox_);
      window.draw(dialogueText_);
    }
  }
};

class SceneManager {
  std::vector<std::shared_ptr<SceneComponent>> components_;

public:
  void addComponent(const std::string &id,
                    std::shared_ptr<SceneComponent> component) {
    components_.push_back(component);
  }
  void draw(sf::RenderWindow &window) {
    for (auto &comp : components_) {
      comp->draw(window);
    }
  }
  void update(float deltaTime) {
    for (auto &comp : components_) {
      comp->update(deltaTime);
    }
  }
};

// --- Definiciones de Comandos de la Historia ---
struct DialogueCmd {
  std::string speakerId;
  std::string text;
  float speed;
};
struct ShowCmd {
  std::string characterId;
  std::string mode;
  Transform transform;
};
struct HideCmd {
  std::string characterId;
};
struct SceneCmd {
  std::string backgroundName;
};
struct PlayCmd {
  std::string musicId;
};
struct StopCmd {
  std::string musicId;
};
using StoryCommand =
    std::variant<DialogueCmd, ShowCmd, HideCmd, SceneCmd, PlayCmd, StopCmd>;

class VisualNovelEngine {
public:
  enum class State {
    IDLE,
    EXECUTING_COMMAND,
    WRITING_DIALOGUE,
    WAITING_FOR_INPUT
  };

  void initialize(const std::string &storyPath) {
    window_.create(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "visualNovel",
                   sf::Style::Default);
    window_.setPosition({100, 100});
    window_.setFramerateLimit(60);
    if (!font_.openFromFile("assets/fonts/CaskaydiaCoveNerdFont-Regular.ttf")) {
      std::cerr << "Error: No se pudo cargar la fuente.\n";
      return;
    }

    dialogueSystem_ = std::make_shared<DialogueSystem>(font_);

    if (!loadStoryFromFile(storyPath)) {
      std::cerr << "Error: No se pudo cargar la historia desde " << storyPath
                << std::endl;
      return;
    }

    if (!storyScript_.empty()) {
      currentState_ = State::EXECUTING_COMMAND;
    }
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

  bool loadStoryFromFile(const std::string &path) {
    std::ifstream file(path);
    if (!file.is_open()) {
      return false;
    }
    json storyJson;
    file >> storyJson;

    // Cargar Assets
    const auto &assets = storyJson["assets"];
    for (auto const &[key, val] : assets["backgrounds"].items()) {
      auto bg = std::make_shared<Background>(val.get<std::string>());
      backgrounds_[key] = bg;
      sceneManager_.addComponent("bg_" + key, bg);
    }
    for (auto const &[key, val] : assets["music"].items()) {
      auto music = std::make_shared<sf::Music>();
      if (music->openFromFile(val.get<std::string>())) {
        musicTracks_[key] = music;
      } else {
        std::cerr << "Error al cargar música: " << val << "\n";
      }
    }
    for (auto const &[key, val] : assets["characters"].items()) {
      auto character =
          std::make_shared<Character>(key, val["name"].get<std::string>());
      for (auto const &[stateKey, stateVal] : val["states"].items()) {
        Transform transform;
        if (stateVal.contains("scale")) {
          transform.scale = {stateVal["scale"][0].get<float>(),
                             stateVal["scale"][1].get<float>()};
        }
        character->addState(stateKey, stateVal["path"].get<std::string>(),
                            transform);
      }
      characters_[key] = character;
      sceneManager_.addComponent("char_" + key, character);
    }

    // Cargar Script
    const auto &script = storyJson["script"];
    for (const auto &cmdJson : script) {
      std::string commandType = cmdJson["command"].get<std::string>();
      if (commandType == "scene") {
        storyScript_.push_back(SceneCmd{cmdJson["background"]});
      } else if (commandType == "play") {
        storyScript_.push_back(PlayCmd{cmdJson["music"]});
      } else if (commandType == "stop") {
        storyScript_.push_back(StopCmd{cmdJson["music"]});
      } else if (commandType == "show") {
        Transform t;
        if (cmdJson.contains("position")) {
          t.position = {cmdJson["position"][0].get<float>(),
                        cmdJson["position"][1].get<float>()};
        }
        storyScript_.push_back(
            ShowCmd{cmdJson["character"], cmdJson["state"], t});
      } else if (commandType == "hide") {
        storyScript_.push_back(HideCmd{cmdJson["character"]});
      } else if (commandType == "dialogue") {
        storyScript_.push_back(DialogueCmd{cmdJson["speaker"], cmdJson["text"],
                                           cmdJson.value("speed", 30.0f)});
      }
    }
    return true;
  }

  void update(float deltaTime) {
    if (currentState_ == State::EXECUTING_COMMAND) {
      executeNextCommand();
    }
    sceneManager_.update(deltaTime);
    dialogueSystem_->update(deltaTime);
  }

  void executeNextCommand() {
    if (commandIndex_ >= storyScript_.size()) {
      dialogueSystem_->hide();
      currentState_ = State::IDLE;
      return;
    }

    const auto &command = storyScript_[commandIndex_];
    std::visit(
        [this](auto &&arg) {
          using T = std::decay_t<decltype(arg)>;
          if constexpr (std::is_same_v<T, DialogueCmd>) {
            std::string speakerName =
                (arg.speakerId == "You") ? "" : arg.speakerId;
            if (auto it = characters_.find(arg.speakerId);
                it != characters_.end()) {
              speakerName = it->second->getName();
            }
            for (auto const &[id, character] : characters_) {
              character->setFocused(id == arg.speakerId ||
                                    arg.speakerId == "You");
            }
            dialogueSystem_->start(
                speakerName.empty() ? arg.text : speakerName + ":\n" + arg.text,
                arg.speed);
            currentState_ = State::WRITING_DIALOGUE;
          } else {
            for (auto const &[id, character] : characters_) {
              character->setFocused(true);
            }
            if constexpr (std::is_same_v<T, SceneCmd>) {
              if (backgrounds_.count(currentBackground_))
                backgrounds_[currentBackground_]->setVisibility(false);
              if (backgrounds_.count(arg.backgroundName))
                backgrounds_[arg.backgroundName]->setVisibility(true);
              currentBackground_ = arg.backgroundName;
            } else if constexpr (std::is_same_v<T, ShowCmd>) {
              if (auto it = characters_.find(arg.characterId);
                  it != characters_.end()) {
                it->second->setState(arg.mode);
                it->second->setPosition(arg.transform.position);
                it->second->setVisibility(true);
              }
            } else if constexpr (std::is_same_v<T, HideCmd>) {
              if (auto it = characters_.find(arg.characterId);
                  it != characters_.end()) {
                it->second->setVisibility(false);
              }
            } else if constexpr (std::is_same_v<T, PlayCmd>) {
              if (!currentMusicId_.empty() &&
                  musicTracks_.count(currentMusicId_)) {
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
        },
        command);

    commandIndex_++;
  }

  void handleEvents() {
    while (std::optional<sf::Event> event = window_.pollEvent()) {
      if (event->is<sf::Event::Closed>()) {
        window_.close();
      }
      if (auto *keyPressed = event->getIf<sf::Event::KeyPressed>()) {
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

  void render() {
    window_.clear(sf::Color::Black);
    sceneManager_.draw(window_);
    dialogueSystem_->draw(window_);
    window_.display();
  }
};

int main(int argc, char *argv[]) {
  std::string storyFile = "story.json";
  if (argc > 1) {
    storyFile = argv[1];
  }

  VisualNovelEngine engine;
  engine.initialize(storyFile);
  engine.run();
  return 0;
}
