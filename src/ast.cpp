#include "ast.hpp"
#include <iomanip>
#include <stdexcept>
#include <variant>

std::string makeIndent(int level) { return std::string(level * 4, ' '); }

std::string escapeString(const std::string &s) {
  std::string escaped;
  for (char c : s) {
    if (c == '"' || c == '\\') {
      escaped += '\\';
    }
    escaped += c;
  }
  return escaped;
}

// Genera el código para todo el programa/juego
void ProgramNode::generateCode(std::ostream &out, int indent) const {
  // --- CABECERAS ---
  out << "#include <SFML/Graphics.hpp>\n";
  out << "#include <SFML/Audio.hpp>\n";
  out << "#include <iostream>\n";
  out << "#include <string>\n";
  out << "#include <vector>\n";
  out << "#include <map>\n";
  out << "#include <memory>\n";
  out << "#include <variant>\n";
  out << "#include <optional>\n\n";

  out << "constexpr int WINDOW_WEIGHT = 1600;\n";
  out << "constexpr int WINDOW_HEIGHT = 800;\n";
  out << "constexpr int TEXT_BOX_POSX = 0;\n";
  out << "constexpr int TEXT_BOX_POSY = 500;\n";
  out << "constexpr int TEXT_BOX_WEIGHT = WINDOW_WEIGHT;\n";
  out << "constexpr int TEXT_BOX_HEIGHT = WINDOW_HEIGHT - TEXT_BOX_POSY;\n";
  out << "constexpr int DIALOGUE_SIZE = 36;\n";
  out << "constexpr int DIALOGUE_POSX = TEXT_BOX_POSX + 20;\n";
  out << "constexpr int DIALOGUE_POSY = TEXT_BOX_POSY + 20;\n";
  out << "constexpr int TEXT_BOX_PADDING = 200;\n";

  // --- CLASES DEL MOTOR (BOILERPLATE COMPLETO) ---
  out << "// --- Clases del Motor de la Novela Visual ---\n\n";

  out << "class TextureManager {\n";
  out << "private:\n";
  out << "  std::map<std::string, std::shared_ptr<sf::Texture>> textures;\n";
  out << "  TextureManager() = default;\n";
  out << "public:\n";
  out << "  static TextureManager &getInstance() {\n";
  out << "    static TextureManager instance;\n";
  out << "    return instance;\n";
  out << "  }\n";
  out << "  std::shared_ptr<sf::Texture> loadTexture(const std::string &path) "
         "{\n";
  out << "    if (textures.count(path)) { return textures[path]; }\n";
  out << "    auto texture = std::make_shared<sf::Texture>();\n";
  out << "    if (!texture->loadFromFile(path)) {\n";
  out << "      std::cerr << \"Error cargando textura: \" << path << "
         "std::endl;\n";
  out << "      return nullptr;\n";
  out << "    }\n";
  out << "    textures[path] = texture;\n";
  out << "    return texture;\n";
  out << "  }\n";
  out << "};\n\n";

  out << "struct Transform {\n";
  out << "  sf::Vector2f position = {0.0f, 0.0f};\n";
  out << "  sf::Vector2f scale = {1.0f, 1.0f};\n";
  out << "};\n\n";

  out << "class SceneComponent {\n";
  out << "public:\n";
  out << "  virtual ~SceneComponent() = default;\n";
  out << "  virtual void draw(sf::RenderWindow &window) = 0;\n";
  out << "  virtual void setVisibility(bool visible) { }\n";
  out << "  virtual void setPosition(const sf::Vector2f& pos) { }\n";
  out << "  virtual void setScale(const sf::Vector2f& scale) { }\n";
  out << "  virtual void setFocused(bool isFocused) { }\n";
  out << "  virtual void update(float deltaTime) { }\n";
  out << "};\n\n";

  out << "class SpriteComponent : public SceneComponent {\n";
  out << "  std::shared_ptr<sf::Texture> texture_;\n";
  out << "  std::unique_ptr<sf::Sprite> sprite_;\n";
  out << "  Transform transform_;\n";
  out << "  bool isVisible_ = true;\n";
  out << "public:\n";
  out << "  SpriteComponent(std::shared_ptr<sf::Texture> texture, const "
         "Transform &transform) : texture_(texture), transform_(transform) {\n";
  out << "    if (texture_) {\n";
  out << "      sprite_ = std::make_unique<sf::Sprite>(*texture_);\n";
  out << "      sprite_->setPosition(transform_.position);\n";
  out << "      sprite_->setScale(transform_.scale);\n";
  out << "    } else { std::cerr << \"Error: SpriteComponent creado con "
         "textura nula.\\n\"; }\n";
  out << "  }\n";
  out << "  void draw(sf::RenderWindow &window) override { if (isVisible_ && "
         "sprite_) window.draw(*sprite_); }\n";
  out << "  void setVisibility(bool visible) override { isVisible_ = visible; "
         "}\n";
  out << "  void setPosition(const sf::Vector2f& pos) override { "
         "transform_.position = pos; if (sprite_) sprite_->setPosition(pos); "
         "}\n";
  out << "  void setScale(const sf::Vector2f& scale) override { "
         "transform_.scale = scale; if (sprite_) sprite_->setScale(scale); }\n";
  out << "  void setFocused(bool isFocused) override { if(sprite_) "
         "sprite_->setColor(isFocused ? sf::Color::White : sf::Color(128, 128, "
         "128)); }\n";
  out << "};\n\n";

  out << "class CharacterState {\n";
  out << "public:\n";
  out << "  virtual ~CharacterState() = default;\n";
  out << "  virtual std::shared_ptr<SpriteComponent> getSprite() = 0;\n";
  out << "};\n\n";

  out << "class GenericCharacterState : public CharacterState {\n";
  out << "  std::shared_ptr<SpriteComponent> sprite_;\n";
  out << "public:\n";
  out << "  GenericCharacterState(const std::string &texturePath, const "
         "Transform &transform) {\n";
  out << "    auto texture = "
         "TextureManager::getInstance().loadTexture(texturePath);\n";
  out << "    sprite_ = std::make_shared<SpriteComponent>(texture, "
         "transform);\n";
  out << "  }\n";
  out << "  std::shared_ptr<SpriteComponent> getSprite() override { return "
         "sprite_; }\n";
  out << "};\n\n";

  out << "class Character : public SceneComponent {\n";
  out << "  std::string id_, name_;\n";
  out << "  std::string currentState_;\n";
  out << "  std::map<std::string, std::unique_ptr<CharacterState>> states_;\n";
  out << "  bool isVisible_ = false;\n";
  out << "public:\n";
  out << "  Character(const std::string &id, const std::string &name) : "
         "id_(id), name_(name) {}\n";
  out << "  void addState(const std::string &stateName, const std::string "
         "&texturePath, const Transform &transform) {\n";
  out << "    states_[stateName] = "
         "std::make_unique<GenericCharacterState>(texturePath, transform);\n";
  out << "  }\n";
  out << "  void setState(const std::string &stateName) {\n";
  out << "    if (states_.count(stateName)) { currentState_ = stateName; }\n";
  out << "  }\n";
  out << "  void draw(sf::RenderWindow &window) override {\n";
  out << "    if (isVisible_ && states_.count(currentState_)) { "
         "states_[currentState_]->getSprite()->draw(window); }\n";
  out << "  }\n";
  out << "  void setVisibility(bool visible) override { isVisible_ = visible; "
         "}\n";
  out << "  void setPosition(const sf::Vector2f& pos) override { if "
         "(states_.count(currentState_)) "
         "states_[currentState_]->getSprite()->setPosition(pos); }\n";
  out << "  void setScale(const sf::Vector2f& scale) override { if "
         "(states_.count(currentState_)) "
         "states_[currentState_]->getSprite()->setScale(scale); }\n";
  out << "  void setFocused(bool isFocused) override { if "
         "(states_.count(currentState_)) "
         "states_[currentState_]->getSprite()->setFocused(isFocused); }\n";
  out << "  const std::string& getName() const { return name_; }\n";
  out << "};\n\n";

  out << "class Background : public SceneComponent {\n";
  out << "  std::shared_ptr<sf::Texture> texture_;\n";
  out << "  std::unique_ptr<sf::Sprite> sprite_;\n";
  out << "  bool isVisible_ = false;\n";
  out << "public:\n";
  out << "  Background(const std::string &texturePath, const Transform "
         "&transform) {\n";
  out << "    texture_ = "
         "TextureManager::getInstance().loadTexture(texturePath);\n";
  out << "    if (texture_) { sprite_ = "
         "std::make_unique<sf::Sprite>(*texture_); }\n";
  out << "  }\n";
  out << "  void draw(sf::RenderWindow &window) override {\n";
  out << "    if (isVisible_ && sprite_) {\n";
  out << "      sf::Vector2u windowSize = window.getSize();\n";
  out << "      sf::Vector2u texSize = texture_->getSize();\n";
  out << "      float scaleX = static_cast<float>(windowSize.x) / texSize.x;\n";
  out << "      float scaleY = static_cast<float>(windowSize.y) / texSize.y;\n";
  out << "      sprite_->setScale({scaleX, scaleY});\n";
  out << "      window.draw(*sprite_);\n";
  out << "    }\n";
  out << "  }\n";
  out << "  void setVisibility(bool visible) override { isVisible_ = visible; "
         "}\n";
  out << "};\n\n";

  out << "class DialogueSystem : public SceneComponent {\n";
  out << "  sf::RectangleShape textBox_;\n";
  out << "  sf::Text dialogueText_;\n";
  out << "  bool isVisible_ = false;\n";
  out << "  std::string fullText_;\n";
  out << "  std::string currentTypedText_;\n";
  out << "  size_t charIndex_ = 0;\n";
  out << "  float timePerChar_ = 0.05f;\n";
  out << "  float elapsedTime_ = 0.0f;\n";
  out << "  bool isTyping_ = false;\n";

  out << "  std::string wrapText(const std::string& text, unsigned int "
         "lineLength, const sf::Font& font, unsigned int charSize) {\n";
  out << "      std::string wrappedText;\n";
  out << "      std::string currentLine;\n";
  out << "      std::string word;\n";
  out << "      sf::Text tempText(font, \"\", charSize);\n";
  out << "      for (char c : text) {\n";
  out << "          if (c == ' ' || c == '\\n') {\n";
  out << "              tempText.setString(currentLine + word + ' ');\n";
  out << "              if (tempText.getLocalBounds().size.x > lineLength) {\n";
  out << "                  wrappedText += currentLine + '\\n';\n";
  out << "                  currentLine = word + ' ';\n";
  out << "              } else {\n";
  out << "                  currentLine += word + ' ';\n";
  out << "              }\n";
  out << "              word.clear();\n";
  out << "              if (c == '\\n') {\n";
  out << "                  wrappedText += currentLine;\n";
  out << "                  currentLine.clear();\n";
  out << "              }\n";
  out << "          } else {\n";
  out << "              word += c;\n";
  out << "          }\n";
  out << "      }\n";
  out << "      tempText.setString(currentLine + word);\n";
  out << "      if (tempText.getLocalBounds().size.x > lineLength) {\n";
  out << "          wrappedText += currentLine + '\\n' + word;\n";
  out << "      } else {\n";
  out << "          wrappedText += currentLine + word;\n";
  out << "      }\n";
  out << "      return wrappedText;\n";
  out << "  }\n\n";

  out << "public:\n";
  out << "  DialogueSystem(const sf::Font &font) : dialogueText_(font, \"\") "
         "{\n";
  out << "    textBox_.setSize({TEXT_BOX_WEIGHT, TEXT_BOX_HEIGHT});\n";
  out << "    textBox_.setPosition({TEXT_BOX_POSX, TEXT_BOX_POSY});\n";
  out << "    textBox_.setFillColor(sf::Color(0, 0, 0, 200));\n";
  out << "    dialogueText_.setCharacterSize(DIALOGUE_SIZE);\n";
  out << "    dialogueText_.setFillColor(sf::Color::White);\n";
  out << "    dialogueText_.setPosition({DIALOGUE_POSX, DIALOGUE_POSY});\n";
  out << "  }\n\n";

  out << "  void start(const std::string &text, float speed) {\n";
  out << "    fullText_ = wrapText(text, TEXT_BOX_WEIGHT - TEXT_BOX_PADDING, "
         "dialogueText_.getFont(), dialogueText_.getCharacterSize());\n";
  out << "    currentTypedText_.clear();\n";
  out << "    charIndex_ = 0;\n";
  out << "    elapsedTime_ = 0.0f;\n";
  out << "    timePerChar_ = (speed > 0) ? 1.0f / speed : 0.0f;\n";
  out << "    isTyping_ = true;\n";
  out << "    isVisible_ = true;\n";
  out << "    dialogueText_.setString(\"\");\n";
  out << "  }\n\n";

  out << "  void update(float deltaTime) override {\n";
  out << "    if (!isTyping_ || charIndex_ >= fullText_.length()) return;\n";
  out << "    elapsedTime_ += deltaTime;\n";
  out << "    if (elapsedTime_ >= timePerChar_) {\n";
  out << "      elapsedTime_ = 0.0f;\n";
  out << "      currentTypedText_ += fullText_[charIndex_];\n";
  out << "      dialogueText_.setString(currentTypedText_);\n";
  out << "      charIndex_++;\n";
  out << "      if (charIndex_ >= fullText_.length()) {\n";
  out << "        isTyping_ = false;\n";
  out << "      }\n";
  out << "    }\n";
  out << "  }\n\n";

  out << "  void finish() {\n";
  out << "    if (isTyping_) {\n";
  out << "      isTyping_ = false;\n";
  out << "      charIndex_ = fullText_.length();\n";
  out << "      currentTypedText_ = fullText_;\n";
  out << "      dialogueText_.setString(fullText_);\n";
  out << "    }\n";
  out << "  }\n\n";
  out << "  bool isFinished() const { return !isTyping_; }\n";
  out << "  void hide() { isVisible_ = false; }\n";
  out << "  void draw(sf::RenderWindow &window) override {\n";
  out << "    if (isVisible_) { window.draw(textBox_); "
         "window.draw(dialogueText_); }\n";
  out << "  }\n";
  out << "};\n\n";

  out << "class SceneManager {\n";
  out << "  std::vector<std::shared_ptr<SceneComponent>> components_;\n";
  out << "public:\n";
  out << "  void addComponent(const std::string& id, "
         "std::shared_ptr<SceneComponent> component) {\n";
  out << "    components_.push_back(component);\n";
  out << "  }\n";
  out << "  void draw(sf::RenderWindow &window) {\n";
  out << "    for (auto &comp : components_) { comp->draw(window); }\n";
  out << "  }\n";
  out << "  void update(float deltaTime) {\n";
  out << "    for (auto &comp : components_) { comp->update(deltaTime); }\n";
  out << "  }\n";
  out << "};\n\n";

  out << "// --- Definiciones de Comandos de la Historia ---\n";
  out << "struct DialogueCmd { std::string speakerId; std::string text; float "
         "speed; };\n";
  out << "struct ShowCmd { std::string characterId; std::string mode; "
         "Transform transform; bool scale_overridden; };\n";
  out << "struct HideCmd { std::string characterId; };\n";
  out << "struct SceneCmd { std::string backgroundName; };\n";
  out << "struct PlayCmd { std::string musicId; };\n";
  out << "struct StopCmd { std::string musicId; };\n";

  out << "using StoryCommand = std::variant<DialogueCmd, ShowCmd, HideCmd, "
         "SceneCmd, PlayCmd, StopCmd>;\n\n";

  out << "class VisualNovelEngine {\n";
  out << "public:\n";
  out << "  enum class State { IDLE, EXECUTING_COMMAND, WRITING_DIALOGUE, "
         "WAITING_FOR_INPUT };\n\n";
  out << "  void initialize() {\n";
  out << "    window_.create(sf::VideoMode({WINDOW_WEIGHT, WINDOW_HEIGHT}), "
         "\"visualNovel\");\n";
  out << "    window_.setPosition({100, 100});\n";
  out << "    window_.setFramerateLimit(60);\n";
  out << "    if "
         "(!font_.openFromFile(\"assets/fonts/"
         "CaskaydiaCoveNerdFont-Regular.ttf\")) { std::cerr << \"Error: No se "
         "pudo cargar la fuente.\\n\"; return; }\n\n";
  out << "    createAssets();\n";
  out << "    dialogueSystem_ = std::make_shared<DialogueSystem>(font_);\n";
  out << "    sceneManager_.addComponent(\"dialogueSystem\", "
         "dialogueSystem_);\n\n";
  out << "    buildStoryScript();\n";
  out << "    if (!storyScript_.empty()) { currentState_ = "
         "State::EXECUTING_COMMAND; }\n";
  out << "  }\n\n";
  out << "  void run() {\n";
  out << "    sf::Clock clock;\n";
  out << "    while (window_.isOpen()) {\n";
  out << "      sf::Time elapsed = clock.restart();\n";
  out << "      handleEvents();\n";
  out << "      update(elapsed.asSeconds());\n";
  out << "      render();\n";
  out << "    }\n";
  out << "  }\n\n";
  out << "private:\n";
  out << "  State currentState_ = State::IDLE;\n";
  out << "  sf::RenderWindow window_;\n";
  out << "  sf::Font font_;\n";
  out << "  SceneManager sceneManager_;\n\n";
  out << "  std::map<std::string, std::shared_ptr<Character>> characters_;\n";
  out << "  std::map<std::string, std::shared_ptr<Background>> backgrounds_;\n";
  out << "  std::map<std::string, std::shared_ptr<sf::Music>> "
         "musicTracks_;\n\n";
  out << "  std::shared_ptr<DialogueSystem> dialogueSystem_;\n";
  out << "  std::vector<StoryCommand> storyScript_;\n";
  out << "  size_t commandIndex_ = 0;\n";
  out << "  std::string currentBackground_;\n";
  out << "  std::string currentMusicId_;\n";

  out << "  void createAssets() {\n";
  for (const auto &stmt : statements) {
    if (dynamic_cast<MusicNode *>(stmt.get()) ||
        dynamic_cast<BackgroundNode *>(stmt.get()) ||
        dynamic_cast<CharacterNode *>(stmt.get())) {
      stmt->generateCode(out, indent + 2);
    }
  }
  out << "  }\n\n";

  out << "  void buildStoryScript() {\n";
  for (const auto &stmt : statements) {
    if (dynamic_cast<PlayNode *>(stmt.get()) ||
        dynamic_cast<StopNode *>(stmt.get()) ||
        dynamic_cast<DialogueNode *>(stmt.get()) ||
        dynamic_cast<ShowNode *>(stmt.get()) ||
        dynamic_cast<HideNode *>(stmt.get()) ||
        dynamic_cast<SceneNode *>(stmt.get())) {
      stmt->generateCode(out, indent + 2);
    }
  }
  out << "  }\n\n";

  out << "  void update(float deltaTime) {\n";
  out << "    if (currentState_ == State::EXECUTING_COMMAND) {\n";
  out << "      executeNextCommand();\n";
  out << "    }\n";
  out << "    sceneManager_.update(deltaTime);\n";
  out << "  }\n\n";

  out << "  void executeNextCommand() {\n";
  out << "    if (commandIndex_ >= storyScript_.size()) { "
         "dialogueSystem_->hide(); currentState_ = State::IDLE; return; }\n\n";
  out << "    const auto& command = storyScript_[commandIndex_];\n";
  out << "    std::visit([this](auto&& arg) {\n";
  out << "      using T = std::decay_t<decltype(arg)>;\n";
  out << "      if constexpr (std::is_same_v<T, DialogueCmd>) {\n";
  out << "        std::string speakerName = (arg.speakerId == \"You\") ? \"\" "
         ": arg.speakerId;\n";
  out << "        if (auto it = characters_.find(arg.speakerId); it != "
         "characters_.end()) { speakerName = it->second->getName(); }\n";
  out << "        for(auto const& [id, character] : characters_) { "
         "character->setFocused(id == arg.speakerId || arg.speakerId == "
         "\"You\"); }\n";
  out << "        dialogueSystem_->start(speakerName.empty() ? arg.text : "
         "speakerName + \":\\n\" + arg.text, arg.speed);\n";
  out << "        currentState_ = State::WRITING_DIALOGUE;\n";
  out << "      } else {\n";
  out << "         for(auto const& [id, character] : characters_) { "
         "character->setFocused(true); }\n";

  out << "         if constexpr (std::is_same_v<T, SceneCmd>) {\n";
  out << "           if (backgrounds_.count(currentBackground_)) "
         "backgrounds_[currentBackground_]->setVisibility(false);\n";
  out << "           if (backgrounds_.count(arg.backgroundName)) "
         "backgrounds_[arg.backgroundName]->setVisibility(true);\n";
  out << "           currentBackground_ = arg.backgroundName;\n";
  out << "         } else if constexpr (std::is_same_v<T, ShowCmd>) {\n";
  out << "           if (auto it = characters_.find(arg.characterId); it != "
         "characters_.end()) {\n";
  out << "             it->second->setState(arg.mode);\n";
  out << "             it->second->setPosition(arg.transform.position);\n";
  out << "             if (arg.scale_overridden) {\n";
  out << "                it->second->setScale(arg.transform.scale);\n";
  out << "             }\n";
  out << "             it->second->setVisibility(true);\n";
  out << "           }\n";
  out << "         } else if constexpr (std::is_same_v<T, HideCmd>) {\n";
  out << "           if (auto it = characters_.find(arg.characterId); it != "
         "characters_.end()) { it->second->setVisibility(false);\n";
  out << "           }\n";
  out << "         } else if constexpr (std::is_same_v<T, PlayCmd>) {\n";
  out << "           if (!currentMusicId_.empty() && "
         "musicTracks_.count(currentMusicId_)) {\n";
  out << "             musicTracks_[currentMusicId_]->stop();\n";
  out << "           }\n";
  out << "           if (musicTracks_.count(arg.musicId)) {\n";
  out << "             currentMusicId_ = arg.musicId;\n";
  out << "             musicTracks_[currentMusicId_]->setLooping(true);\n";
  out << "             musicTracks_[currentMusicId_]->play();\n";
  out << "           }\n";
  out << "         } else if constexpr (std::is_same_v<T, StopCmd>) {\n";
  out << "           if (musicTracks_.count(arg.musicId)) {\n";
  out << "             musicTracks_[arg.musicId]->stop();\n";
  out << "             if (currentMusicId_ == arg.musicId) {\n";
  out << "               currentMusicId_.clear();\n";
  out << "             }\n";
  out << "           }\n";
  out << "         }\n";
  out << "         currentState_ = State::EXECUTING_COMMAND;\n";
  out << "      }\n";
  out << "    }, command);\n\n";
  out << "    commandIndex_++;\n";
  out << "  }\n\n";

  out << "  void handleEvents() {\n";
  out << "    while (std::optional<sf::Event> event = window_.pollEvent()) {\n";
  out << "        if (event->is<sf::Event::Closed>()) { window_.close(); }\n";
  out << "        if (auto* keyPressed = "
         "event->getIf<sf::Event::KeyPressed>()) {\n";
  out << "            if (keyPressed->code == sf::Keyboard::Key::Space) {\n";
  out << "                if (currentState_ == State::WRITING_DIALOGUE) {\n";
  out << "                    dialogueSystem_->finish();\n";
  out << "                    currentState_ = State::WAITING_FOR_INPUT;\n";
  out << "                } else if (currentState_ == "
         "State::WAITING_FOR_INPUT) {\n";
  out << "                    currentState_ = State::EXECUTING_COMMAND;\n";
  out << "                }\n";
  out << "            }\n";
  out << "        }\n";
  out << "    }\n";
  out << "  }\n\n";
  out << "  void render() { window_.clear(sf::Color::Black); "
         "sceneManager_.draw(window_); window_.display(); }\n";
  out << "};\n\n";

  out << "int main() {\n";
  out << "    VisualNovelEngine engine;\n";
  out << "    engine.initialize();\n";
  out << "    engine.run();\n";
  out << "    return 0;\n";
  out << "}\n";
}

// --- GENERACIÓN DE CÓDIGO PARA CADA NODO DEL AST ---

void BackgroundNode::generateCode(std::ostream &out, int indent) const {
  out << makeIndent(indent) << "{\n";
  out << makeIndent(indent + 1) << "Transform transform;\n";
  out << makeIndent(indent + 1) << "auto bg = std::make_shared<Background>(\""
      << escapeString(imagePath) << "\", transform);\n";
  out << makeIndent(indent + 1) << "backgrounds_[\"" << name << "\"] = bg;\n";
  out << makeIndent(indent + 1)
      << "sceneManager_.addComponent(\"bg_\" + std::string(\"" << name
      << "\"), bg);\n";
  out << makeIndent(indent) << "}\n";
}

void CharacterNode::generateCode(std::ostream &out, int indent) const {
  out << makeIndent(indent) << "{\n";
  out << makeIndent(indent + 1)
      << "auto character = std::make_shared<Character>(\"" << id << "\", \""
      << escapeString(displayName) << "\");\n";
  for (const auto &mode : modes) {
    out << makeIndent(indent + 2) << "{\n";
    out << makeIndent(indent + 3) << "Transform transform;\n";
    for (const auto &[key, val] : mode->parameters) {
      if (key == "scale") {
        out << makeIndent(indent + 3) << "transform.scale = { (float)"
            << std::get<double>(val) << ", (float)" << std::get<double>(val)
            << " };\n";
      }
    }
    out << makeIndent(indent + 3) << "character->addState(\"" << mode->name
        << "\", \"" << escapeString(mode->imagePath) << "\", transform);\n";
    out << makeIndent(indent + 2) << "}\n";
  }
  out << makeIndent(indent + 1) << "characters_[\"" << id
      << "\"] = character;\n";
  out << makeIndent(indent + 1)
      << "sceneManager_.addComponent(\"char_\" + std::string(\"" << id
      << "\"), character);\n";
  out << makeIndent(indent) << "}\n";
}

void SceneNode::generateCode(std::ostream &out, int indent) const {
  out << makeIndent(indent) << "storyScript_.push_back(SceneCmd{\"" << name
      << "\"});\n";
}

void ShowNode::generateCode(std::ostream &out, int indent) const {
  out << makeIndent(indent) << "{\n";
  out << makeIndent(indent + 1) << "Transform t;\n";
  out << makeIndent(indent + 1) << "bool scale_overridden = false;\n";
  for (const auto &[key, val] : this->parameters) {
    if (key == "x") {
      out << makeIndent(indent + 1)
          << "t.position.x = " << std::get<double>(val) << ";\n";
    } else if (key == "y") {
      out << makeIndent(indent + 1)
          << "t.position.y = " << std::get<double>(val) << ";\n";
    } else if (key == "scale") {
      out << makeIndent(indent + 1) << "t.scale = { (float)"
          << std::get<double>(val) << ", (float)" << std::get<double>(val)
          << " };\n";
      out << makeIndent(indent + 1) << "scale_overridden = true;\n";
    }
  }
  out << makeIndent(indent + 1) << "storyScript_.push_back(ShowCmd{\""
      << characterId << "\", \"" << mode << "\", t, scale_overridden});\n";
  out << makeIndent(indent) << "}\n";
}

void HideNode::generateCode(std::ostream &out, int indent) const {
  out << makeIndent(indent) << "storyScript_.push_back(HideCmd{\""
      << characterId << "\"});\n";
}

void DialogueNode::generateCode(std::ostream &out, int indent) const {
  out << makeIndent(indent) << "{\n";
  out << makeIndent(indent + 1)
      << "float speed = 30.0f; // Velocidad por defecto\n";
  for (const auto &[key, val] : this->parameters) {
    if (key == "speed") {
      out << makeIndent(indent + 1) << "speed = static_cast<float>("
          << std::get<double>(val) << ");\n";
    }
  }
  out << makeIndent(indent + 1) << "storyScript_.push_back(DialogueCmd{\""
      << speaker << "\", R\"(" << text << ")\", speed});\n";
  out << makeIndent(indent) << "}\n";
}

void MusicNode::generateCode(std::ostream &out, int indent) const {
  out << makeIndent(indent) << "{\n";
  out << makeIndent(indent + 1)
      << "auto music = std::make_shared<sf::Music>();\n";
  out << makeIndent(indent + 1) << "if (music->openFromFile(\""
      << escapeString(filePath) << "\")) {\n";
  out << makeIndent(indent + 2) << "musicTracks_[\"" << id << "\"] = music;\n";
  out << makeIndent(indent + 1)
      << "} else { std::cerr << \"Error al cargar música: "
      << escapeString(filePath) << "\\n\"; }\n";
  out << makeIndent(indent) << "}\n";
}

void PlayNode::generateCode(std::ostream &out, int indent) const {
  out << makeIndent(indent) << "storyScript_.push_back(PlayCmd{\"" << musicId
      << "\"});\n";
}

void StopNode::generateCode(std::ostream &out, int indent) const {
  out << makeIndent(indent) << "storyScript_.push_back(StopCmd{\"" << musicId
      << "\"});\n";
}
