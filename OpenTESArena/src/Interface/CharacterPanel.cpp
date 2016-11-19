#include <cassert>

#include "SDL.h"

#include "CharacterPanel.h"

#include "Button.h"
#include "CharacterEquipmentPanel.h"
#include "GameWorldPanel.h"
#include "TextAlignment.h"
#include "TextBox.h"
#include "../Assets/CIFFile.h"
#include "../Entities/CharacterClass.h"
#include "../Entities/CharacterRace.h"
#include "../Entities/Player.h"
#include "../Game/GameData.h"
#include "../Game/GameState.h"
#include "../Math/Int2.h"
#include "../Media/Color.h"
#include "../Media/FontManager.h"
#include "../Media/FontName.h"
#include "../Media/PaletteFile.h"
#include "../Media/PaletteName.h"
#include "../Media/PortraitFile.h"
#include "../Media/TextureFile.h"
#include "../Media/TextureManager.h"
#include "../Media/TextureName.h"
#include "../Rendering/Renderer.h"
#include "../Rendering/Texture.h"

CharacterPanel::CharacterPanel(GameState *gameState)
	: Panel(gameState), headOffsets()
{
	this->playerNameTextBox = [gameState]()
	{
		Int2 origin(10, 8);
		Color color(199, 199, 199);
		std::string text = gameState->getGameData()->getPlayer().getDisplayName();
		auto &font = gameState->getFontManager().getFont(FontName::Arena);
		auto alignment = TextAlignment::Left;
		return std::unique_ptr<TextBox>(new TextBox(
			origin.getX(),
			origin.getY(),
			color,
			text,
			font,
			alignment,
			gameState->getRenderer()));
	}();

	this->playerRaceTextBox = [gameState]()
	{
		Int2 origin(10, 17);
		Color color(199, 199, 199);
		std::string text = CharacterRace(gameState->getGameData()->getPlayer()
			.getRaceName()).toString();
		auto &font = gameState->getFontManager().getFont(FontName::Arena);
		auto alignment = TextAlignment::Left;
		return std::unique_ptr<TextBox>(new TextBox(
			origin.getX(),
			origin.getY(),
			color,
			text,
			font,
			alignment,
			gameState->getRenderer()));
	}();

	this->playerClassTextBox = [gameState]()
	{
		Int2 origin(10, 26);
		Color color(199, 199, 199);
		std::string text = gameState->getGameData()->getPlayer().getCharacterClass()
			.getDisplayName();
		auto &font = gameState->getFontManager().getFont(FontName::Arena);
		auto alignment = TextAlignment::Left;
		return std::unique_ptr<TextBox>(new TextBox(
			origin.getX(),
			origin.getY(),
			color,
			text,
			font,
			alignment,
			gameState->getRenderer()));
	}();

	this->doneButton = []()
	{
		Int2 center(25, Renderer::ORIGINAL_HEIGHT - 15);
		int width = 21;
		int height = 13;
		auto function = [](GameState *gameState)
		{
			std::unique_ptr<Panel> gamePanel(new GameWorldPanel(gameState));
			gameState->setPanel(std::move(gamePanel));
		};
		return std::unique_ptr<Button>(new Button(center, width, height, function));
	}();

	this->nextPageButton = []()
	{
		int x = 108;
		int y = 179;
		int width = 49;
		int height = 13;
		auto function = [](GameState *gameState)
		{
			std::unique_ptr<Panel> equipmentPanel(new CharacterEquipmentPanel(gameState));
			gameState->setPanel(std::move(equipmentPanel));
		};
		return std::unique_ptr<Button>(new Button(x, y, width, height, function));
	}();

	// Get pixel offsets for each head.
	const auto &player = this->getGameState()->getGameData()->getPlayer();
	const std::string &headsFilename = PortraitFile::getHeads(
		player.getGenderName(), player.getRaceName(), false);
	CIFFile cifFile(headsFilename, Palette());

	for (int i = 0; i < cifFile.getImageCount(); ++i)
	{
		this->headOffsets.push_back(Int2(cifFile.getXOffset(i), cifFile.getYOffset(i)));
	}
}

CharacterPanel::~CharacterPanel()
{

}

void CharacterPanel::handleEvent(const SDL_Event &e)
{
	bool escapePressed = (e.type == SDL_KEYDOWN) &&
		(e.key.keysym.sym == SDLK_ESCAPE);
	bool tabPressed = (e.type == SDL_KEYDOWN) &&
		(e.key.keysym.sym == SDLK_TAB);

	if (escapePressed || tabPressed)
	{
		this->doneButton->click(this->getGameState());
	}

	bool leftClick = (e.type == SDL_MOUSEBUTTONDOWN) &&
		(e.button.button == SDL_BUTTON_LEFT);

	if (leftClick)
	{
		const Int2 mousePosition = this->getMousePosition();
		const Int2 mouseOriginalPoint = this->getGameState()->getRenderer()
			.nativePointToOriginal(mousePosition);

		if (this->doneButton->contains(mouseOriginalPoint))
		{
			this->doneButton->click(this->getGameState());
		}
		else if (this->nextPageButton->contains(mouseOriginalPoint))
		{
			this->nextPageButton->click(this->getGameState());
		}
	}
}

void CharacterPanel::render(Renderer &renderer)
{
	assert(this->getGameState()->gameDataIsActive());

	// Clear full screen.
	renderer.clearNative();
	renderer.clearOriginal();

	// Set palette.
	auto &textureManager = this->getGameState()->getTextureManager();
	textureManager.setPalette(PaletteFile::fromName(PaletteName::CharSheet));

	// Get a reference to the active player data.
	const auto &player = this->getGameState()->getGameData()->getPlayer();

	// Get the filenames for the portrait and clothes.
	const std::string &headsFilename = PortraitFile::getHeads(
		player.getGenderName(), player.getRaceName(), false);
	const std::string &bodyFilename = PortraitFile::getBody(
		player.getGenderName(), player.getRaceName());
	const std::string &shirtFilename = PortraitFile::getShirt(
		player.getGenderName(), player.getCharacterClass().canCastMagic());
	const std::string &pantsFilename = PortraitFile::getPants(player.getGenderName());

	// Get pixel offsets for each clothes texture.
	const Int2 &shirtOffset = PortraitFile::getShirtOffset(
		player.getGenderName(), player.getCharacterClass().canCastMagic());
	const Int2 &pantsOffset = PortraitFile::getPantsOffset(player.getGenderName());

	// Draw the current portrait and clothes.
	const Int2 &headOffset = this->headOffsets.at(player.getPortraitID());
	const auto &head = textureManager.getTextures(headsFilename,
		PaletteFile::fromName(PaletteName::CharSheet)).at(player.getPortraitID());
	const auto &body = textureManager.getTexture(bodyFilename);
	const auto &shirt = textureManager.getTexture(shirtFilename);
	const auto &pants = textureManager.getTexture(pantsFilename);
	renderer.drawToOriginal(body.get(), Renderer::ORIGINAL_WIDTH - body.getWidth(), 0);
	renderer.drawToOriginal(pants.get(), pantsOffset.getX(), pantsOffset.getY());
	renderer.drawToOriginal(head.get(), headOffset.getX(), headOffset.getY());
	renderer.drawToOriginal(shirt.get(), shirtOffset.getX(), shirtOffset.getY());

	// Draw character stats background.
	const auto &statsBackground = textureManager.getTexture(
		TextureFile::fromName(TextureName::CharacterStats));
	renderer.drawToOriginal(statsBackground.get());

	// Draw "Next Page" texture.
	const auto &nextPageTexture = textureManager.getTexture(
		TextureFile::fromName(TextureName::NextPage));
	renderer.drawToOriginal(nextPageTexture.get(), 108, 179);

	// Draw text boxes: player name, race, class.
	renderer.drawToOriginal(this->playerNameTextBox->getTexture(),
		this->playerNameTextBox->getX(), this->playerNameTextBox->getY());
	renderer.drawToOriginal(this->playerRaceTextBox->getTexture(),
		this->playerRaceTextBox->getX(), this->playerRaceTextBox->getY());
	renderer.drawToOriginal(this->playerClassTextBox->getTexture(),
		this->playerClassTextBox->getX(), this->playerClassTextBox->getY());

	// Scale the original frame buffer onto the native one.
	renderer.drawOriginalToNative();

	// Draw cursor.
	const auto &cursor = textureManager.getTexture(
		TextureFile::fromName(TextureName::SwordCursor));
	const auto mousePosition = this->getMousePosition();
	renderer.drawToNative(cursor.get(),
		mousePosition.getX(), mousePosition.getY(),
		static_cast<int>(cursor.getWidth() * this->getCursorScale()),
		static_cast<int>(cursor.getHeight() * this->getCursorScale()));
}
