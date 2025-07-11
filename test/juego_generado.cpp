#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <cstddef>
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

constexpr int WINDOW_WIDTH = 1600;
constexpr int WINDOW_HEIGHT = 800;
constexpr int TEXT_BOX_POSX = 0;
constexpr int TEXT_BOX_POSY = WINDOW_HEIGHT * 0.65;
constexpr int TEXT_BOX_WIDTH = WINDOW_WIDTH;
constexpr int TEXT_BOX_HEIGHT = WINDOW_HEIGHT - TEXT_BOX_POSY;
constexpr int TEXT_BOX_PADDING = 100;
constexpr int DIALOGUE_SIZE = 36;
constexpr int DIALOGUE_POSX = TEXT_BOX_POSX + 20;
constexpr int DIALOGUE_POSY = TEXT_BOX_POSY + 20;
constexpr int CHOICE_BOX_WIDTH = 1400;
constexpr int CHOICE_BOX_PADDING = 50;
constexpr int TEXT_OPTION_SIZE = 24;
constexpr int OPTION_PADDING = 10;
constexpr int OPTION_MARGIN = 10;
constexpr int PROMPT_SIZE = 24;
constexpr int PROMPT_MARGIN = 10;

std::string wrapText(const std::string &text, unsigned int lineLength,
                     const sf::Font &font, unsigned int charSize) {
  std::string wrappedText;
  std::string currentLine;
  std::string word;
  sf::Text tempText(font, "", charSize);

  for (char c : text) {
    if (c == ' ' || c == '\n') {
      tempText.setString(currentLine + word);

      if (tempText.getGlobalBounds().size.x > lineLength) {
        wrappedText += currentLine + '\n';
        currentLine.clear();

        currentLine += word + c;
      } else {
        currentLine += word + ' ';
      }

      word.clear();
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
  virtual void setVisibility(bool) {};
  virtual void setPosition(const sf::Vector2f &) {};
  virtual void setScale(const sf::Vector2f &) {};
  virtual void setFocused(bool) {};
  virtual void update(float) {};
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
      sf::Vector2u texSize = texture_->getSize();
      float scaleX = static_cast<float>(WINDOW_WIDTH) / texSize.x;
      float scaleY = static_cast<float>(WINDOW_HEIGHT) / texSize.y;
      sprite_->setScale({scaleX, scaleY});
    }
  }
  void draw(sf::RenderWindow &window) override {
    if (isVisible_ && sprite_) {
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
  bool isVisible() const { return isVisible_; }
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

struct JumpCmd {
  std::string targetLabel;
};

struct EndCmd {};

struct ChoiceOptionCmd {
  std::string text;
  std::string gotoLabel;
};

struct ChoiceCmd {
  std::string prompt;
  std::vector<ChoiceOptionCmd> options;
};

struct LabelCmd {
  std::string name;
  size_t startIndex;
};

using StoryCommand = std::variant<DialogueCmd, ShowCmd, HideCmd, SceneCmd,
                                  PlayCmd, StopCmd, ChoiceCmd, JumpCmd, EndCmd>;

class ChoiceBox : public SceneComponent {
private:
  sf::RectangleShape background_;
  sf::Text promptText_;
  std::vector<sf::Text> optionTexts_;
  std::vector<sf::RectangleShape> optionRects_;
  sf::Font font_;
  bool isVisible_ = false;
  int hoveredOption_ = -1;
  std::vector<std::string> gotoLabels_;

public:
  ChoiceBox(const sf::Font &font) : promptText_(font, ""), font_(font) {
    background_.setSize({CHOICE_BOX_WIDTH, 100});
    background_.setFillColor(sf::Color(0, 0, 0, 180));
    promptText_.setFont(font_);
    promptText_.setCharacterSize(TEXT_OPTION_SIZE);
    promptText_.setFillColor(sf::Color::White);
  }

  void setOptions(const std::string &prompt,
                  const std::vector<ChoiceOptionCmd> &options) {
    optionTexts_.clear();
    optionRects_.clear();
    gotoLabels_.clear();

    promptText_.setString(
        wrapText(prompt, CHOICE_BOX_WIDTH * 0.9, font_, PROMPT_SIZE));

    float totalHeight =
        PROMPT_MARGIN * 2 + promptText_.getGlobalBounds().size.y;
    float optionWidth = background_.getSize().x * 0.9;

    for (const auto &option : options) {
      sf::Text optionText(font_, "", TEXT_OPTION_SIZE);
      optionText.setFillColor(sf::Color::White);
      std::string wrapped =
          wrapText(option.text, optionWidth * 0.9, font_, TEXT_OPTION_SIZE);
      optionText.setString(wrapped);

      totalHeight += optionText.getGlobalBounds().size.y + OPTION_PADDING * 2 +
                     OPTION_MARGIN * 2;

      sf::RectangleShape optionRect;
      optionRect.setFillColor(sf::Color(50, 50, 50, 180));
      optionRect.setOutlineThickness(2);
      optionRect.setOutlineColor(sf::Color(100, 100, 100, 200));

      optionTexts_.push_back(optionText);
      optionRects_.push_back(optionRect);
      gotoLabels_.push_back(option.gotoLabel);
    }

    background_.setSize(
        {CHOICE_BOX_WIDTH, totalHeight + CHOICE_BOX_PADDING * 2});
    background_.setPosition({(WINDOW_WIDTH - background_.getSize().x) / 2.0f,
                             (WINDOW_HEIGHT - background_.getSize().y) / 2.0f});

    promptText_.setPosition(
        {((background_.getSize().x - promptText_.getGlobalBounds().size.x) /
          2.0f) +
             background_.getPosition().x,
         background_.getPosition().y + CHOICE_BOX_PADDING + PROMPT_MARGIN});

    float currentY = promptText_.getPosition().y +
                     promptText_.getGlobalBounds().size.y + PROMPT_MARGIN;

    for (size_t i = 0; i < optionTexts_.size(); ++i) {
      float optionHeight =
          optionTexts_[i].getGlobalBounds().size.y + OPTION_PADDING * 2;

      optionRects_[i].setSize({optionWidth, optionHeight});
      optionRects_[i].setPosition(
          {((background_.getSize().x - optionWidth) / 2.0f) +
               background_.getPosition().x,
           currentY + OPTION_MARGIN});
      optionTexts_[i].setPosition(
          {optionRects_[i].getPosition().x +
               (optionWidth - optionTexts_[i].getGlobalBounds().size.x) / 2.0f,
           currentY + (optionHeight + OPTION_MARGIN -
                       optionTexts_[i].getGlobalBounds().size.y) /
                          2.0f});
      currentY += optionHeight + OPTION_MARGIN * 2;
    }
    isVisible_ = true;
  }

  void draw(sf::RenderWindow &window) override {
    if (!isVisible_)
      return;

    window.draw(background_);
    window.draw(promptText_);
    for (size_t i = 0; i < optionRects_.size(); ++i) {
      window.draw(optionRects_[i]);
    }
    for (const auto &text : optionTexts_) {
      window.draw(text);
    }
  }

  void setVisibility(bool visible) override { isVisible_ = visible; }
  bool isVisible() const { return isVisible_; }

  int handleMouseClick(sf::Vector2i mousePos) {
    if (!isVisible_)
      return -1;

    for (size_t i = 0; i < optionRects_.size(); ++i) {
      if (optionRects_[i].getGlobalBounds().contains(
              static_cast<sf::Vector2f>(mousePos))) {
        return i;
      }
    }
    return -1;
  }

  void handleMouseMove(sf::Vector2i mousePos) {
    if (!isVisible_)
      return;

    int oldHovered = hoveredOption_;
    hoveredOption_ = -1;
    for (size_t i = 0; i < optionRects_.size(); ++i) {
      if (optionRects_[i].getGlobalBounds().contains(
              static_cast<sf::Vector2f>(mousePos))) {
        hoveredOption_ = i;
        break;
      }
    }

    if (hoveredOption_ != oldHovered) {
      for (size_t i = 0; i < optionRects_.size(); ++i) {
        if (i == hoveredOption_) {
          optionRects_[i].setFillColor(sf::Color(80, 80, 80, 200));
        } else {
          optionRects_[i].setFillColor(sf::Color(50, 50, 50, 180));
        }
      }
    }
  }

  const std::string &getGotoLabel(int index) const {
    return gotoLabels_[index];
  }
};

class VisualNovelEngine {
public:
  enum class State {
    IDLE,
    EXECUTING_COMMAND,
    WRITING_DIALOGUE,
    WAITING_FOR_INPUT,
    WAITING_FOR_CHOICE
  };

  void initialize(const std::string &storyPath) {
    window_.create(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "visualNovel",
                   sf::Style::Close | sf::Style::Titlebar);
    window_.setPosition({100, 100});
    window_.setFramerateLimit(60);
    if (!font_.openFromFile(
            "assets/fonts/WinkyRough-Italic-VariableFont_wght.ttf")) {
      std::cerr << "Error: No se pudo cargar la fuente.\n";
      return;
    }

    dialogueSystem_ = std::make_shared<DialogueSystem>(font_);
    choiceBox_ = std::make_shared<ChoiceBox>(font_);

    if (!loadStoryFromFile(storyPath)) {
      std::cerr << "Error: No se pudo cargar la historia desde " << storyPath
                << std::endl;
      return;
    }

    if (!storyScript_.empty()) {
      if (labelMap_.count("start")) {
        commandIndex_ = labelMap_["start"];
      } else {
        std::cerr << "Error: 'start' label not found in story.json\n";
        currentState_ = State::IDLE;
        return;
      }
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
  bool waitForMouseReleaseForChoice_ = false;
  sf::RenderWindow window_;
  sf::Font font_;
  SceneManager sceneManager_;

  std::map<std::string, std::shared_ptr<Character>> characters_;
  std::map<std::string, std::shared_ptr<Background>> backgrounds_;
  std::map<std::string, std::shared_ptr<sf::Music>> musicTracks_;

  std::shared_ptr<DialogueSystem> dialogueSystem_;
  std::shared_ptr<ChoiceBox> choiceBox_;
  std::vector<StoryCommand> storyScript_;
  size_t commandIndex_ = 0;
  std::string currentBackground_;
  std::string currentMusicId_;
  std::map<std::string, size_t> labelMap_;

  bool loadStoryFromFile(const std::string &path) {
    std::ifstream file(path);
    if (!file.is_open()) {
      return false;
    }
    json storyJson;
    file >> storyJson;

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

    const auto &script = storyJson["script"];
    for (const auto &labelEntry : script) {
      std::string labelName = labelEntry["label"].get<std::string>();
      const auto &commands = labelEntry["commands"];
      std::cout << "DEBUG label : " << labelName
                << " , position : " << storyScript_.size() << "\n";
      labelMap_[labelName] = storyScript_.size();
      for (const auto &cmdJson : commands) {
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
          storyScript_.push_back(DialogueCmd{cmdJson["speaker"],
                                             cmdJson["text"],
                                             cmdJson.value("speed", 30.0f)});
        } else if (commandType == "choice") {
          ChoiceCmd choiceCmd;
          choiceCmd.prompt = cmdJson["prompt"].get<std::string>();
          for (const auto &optionJson : cmdJson["options"]) {
            choiceCmd.options.push_back(
                ChoiceOptionCmd{optionJson["text"].get<std::string>(),
                                optionJson["goto"].get<std::string>()});
          }
          storyScript_.push_back(choiceCmd);
        } else if (commandType == "jump") {
          storyScript_.push_back(JumpCmd{cmdJson["target"]});
        } else if (commandType == "end") {
          storyScript_.push_back(EndCmd{});
        }
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
      std::cerr << "DEBUG: Reached end of script. commandIndex_ = "
                << commandIndex_
                << ", storyScript_.size() = " << storyScript_.size()
                << std::endl;
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
          } else if constexpr (std::is_same_v<T, ChoiceCmd>) {
            dialogueSystem_->hide();
            choiceBox_->setOptions(arg.prompt, arg.options);
            currentState_ = State::WAITING_FOR_CHOICE;
            if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
              waitForMouseReleaseForChoice_ = true;
            } else {
              waitForMouseReleaseForChoice_ = false;
            }
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
            } else if constexpr (std::is_same_v<T, EndCmd>) {
              window_.close();
              currentState_ =
                  State::IDLE; // Set to IDLE to stop further processing
            } else if constexpr (std::is_same_v<T, JumpCmd>) {
              if (labelMap_.count(arg.targetLabel)) {
                commandIndex_ = labelMap_[arg.targetLabel];
                currentState_ =
                    State::EXECUTING_COMMAND; // Ensure we continue executing
                                              // from the new label
              } else {
                std::cerr << "Error: Jump target label \"" << arg.targetLabel
                          << "\" not found.\n";
                currentState_ = State::IDLE; // Stop if target not found
              }
            }
            currentState_ = State::EXECUTING_COMMAND;
            commandIndex_++;
          }
        },
        command);
  }

  void handleEvents() {
    while (std::optional<sf::Event> event = window_.pollEvent()) {
      if (event->is<sf::Event::Closed>()) {
        window_.close();
      }
      if (auto *keyPressed = event->getIf<sf::Event::KeyPressed>()) {
        if (keyPressed->code == sf::Keyboard::Key::Space) {
          if (dialogueSystem_->isFinished()) {
            currentState_ = State::EXECUTING_COMMAND;
            commandIndex_++;
          }

          if (currentState_ == State::WRITING_DIALOGUE) {
            dialogueSystem_->finish();
            currentState_ = State::WAITING_FOR_INPUT;
          } else if (currentState_ == State::WAITING_FOR_INPUT) {
            currentState_ = State::EXECUTING_COMMAND;
            commandIndex_++;
          }
        }
      }
      if (currentState_ == State::WAITING_FOR_CHOICE) {
        if (auto *mouseButtonReleased =
                event->getIf<sf::Event::MouseButtonReleased>()) {
          if (mouseButtonReleased->button == sf::Mouse::Button::Left) {
            if (waitForMouseReleaseForChoice_) {
              waitForMouseReleaseForChoice_ = false;
              return;
            }
            int chosenOptionIndex =
                choiceBox_->handleMouseClick(sf::Mouse::getPosition(window_));
            if (chosenOptionIndex != -1) {
              const auto &currentCommand = storyScript_[commandIndex_];
              if (const auto *choiceCmd =
                      std::get_if<ChoiceCmd>(&currentCommand)) {
                std::string targetLabel =
                    choiceBox_->getGotoLabel(chosenOptionIndex);
                if (labelMap_.count(targetLabel)) {
                  commandIndex_ = labelMap_[targetLabel];
                  currentState_ = State::EXECUTING_COMMAND;
                  choiceBox_->setVisibility(false);
                } else {
                  std::cerr << "Error: Label \"" << targetLabel
                            << "\" not found.\n";
                }
              }
            }
          }
        } else if (auto *mouseMoved = event->getIf<sf::Event::MouseMoved>()) {
          choiceBox_->handleMouseMove(
              {mouseMoved->position.x, mouseMoved->position.y});
        }
      }
    }
  }

  void render() {
    window_.clear(sf::Color::Black);
    sceneManager_.draw(window_);
    dialogueSystem_->draw(window_);
    choiceBox_->draw(window_);
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
