#include "SDL.h"

#include "ChooseAttributesPanel.h"
#include "ChooseGenderPanel.h"
#include "ChooseRacePanel.h"
#include "CursorAlignment.h"
#include "MessageBoxSubPanel.h"
#include "RichTextString.h"
#include "TextAlignment.h"
#include "TextBox.h"
#include "TextSubPanel.h"
#include "../Assets/ExeData.h"
#include "../Assets/MiscAssets.h"
#include "../Assets/WorldMapMask.h"
#include "../Entities/GenderName.h"
#include "../Game/Game.h"
#include "../Game/Options.h"
#include "../Math/Rect.h"
#include "../Media/Color.h"
#include "../Media/FontManager.h"
#include "../Media/FontName.h"
#include "../Media/PaletteFile.h"
#include "../Media/PaletteName.h"
#include "../Media/TextureFile.h"
#include "../Media/TextureManager.h"
#include "../Media/TextureName.h"
#include "../Rendering/Renderer.h"
#include "../Rendering/Surface.h"

#include "components/utilities/String.h"

const int ChooseRacePanel::NO_ID = -1;

ChooseRacePanel::ChooseRacePanel(Game &game, const CharacterClass &charClass,
	const std::string &name, GenderName gender)
	: Panel(game), charClass(charClass), name(name), gender(gender)
{
	this->backToGenderButton = []()
	{
		auto function = [](Game &game, const CharacterClass &charClass,
			const std::string &name)
		{
			game.setPanel<ChooseGenderPanel>(game, charClass, name);
		};
		return Button<Game&, const CharacterClass&, const std::string&>(function);
	}();

	this->acceptButton = []()
	{
		auto function = [](Game &game, const CharacterClass &charClass,
			const std::string &name, GenderName gender, int raceID)
		{
			// Generate the race selection message box.
			auto &textureManager = game.getTextureManager();
			auto &renderer = game.getRenderer();

			const Color textColor(52, 24, 8);

			MessageBoxSubPanel::Title messageBoxTitle;
			messageBoxTitle.textBox = [&game, raceID, &renderer, &textColor]()
			{
				const auto &exeData = game.getMiscAssets().getExeData();
				std::string text = exeData.charCreation.confirmRace;
				text = String::replace(text, '\r', '\n');

				const std::string &provinceName =
					exeData.locations.charCreationProvinceNames.at(raceID);
				const std::string &pluralRaceName = exeData.races.pluralNames.at(raceID);

				// Replace first %s with province name.
				size_t index = text.find("%s");
				text.replace(index, 2, provinceName);

				// Replace second %s with plural race name.
				index = text.find("%s");
				text.replace(index, 2, pluralRaceName);

				const int lineSpacing = 1;
				const RichTextString richText(
					text,
					FontName::A,
					textColor,
					TextAlignment::Center,
					lineSpacing,
					game.getFontManager());

				const Int2 center(
					(Renderer::ORIGINAL_WIDTH / 2),
					(Renderer::ORIGINAL_HEIGHT / 2) - 22);

				return std::make_unique<TextBox>(center, richText, renderer);
			}();

			messageBoxTitle.texture = [&textureManager, &renderer, &messageBoxTitle]()
			{
				const int width = messageBoxTitle.textBox->getRect().getWidth() + 22;
				const int height = 60;
				return Texture::generate(Texture::PatternType::Parchment, width, height,
					textureManager, renderer);
			}();

			messageBoxTitle.textureX = (Renderer::ORIGINAL_WIDTH / 2) -
				(messageBoxTitle.texture.getWidth() / 2) - 1;
			messageBoxTitle.textureY = (Renderer::ORIGINAL_HEIGHT / 2) -
				(messageBoxTitle.texture.getHeight() / 2) - 21;

			MessageBoxSubPanel::Element messageBoxYes;
			messageBoxYes.textBox = [&game, &renderer, &textColor]()
			{
				const RichTextString richText(
					"Yes",
					FontName::A,
					textColor,
					TextAlignment::Center,
					game.getFontManager());

				const Int2 center(
					(Renderer::ORIGINAL_WIDTH / 2) - 1,
					(Renderer::ORIGINAL_HEIGHT / 2) + 28);

				return std::make_unique<TextBox>(center, richText, renderer);
			}();

			messageBoxYes.texture = [&textureManager, &renderer, &messageBoxTitle]()
			{
				const int width = messageBoxTitle.texture.getWidth();
				return Texture::generate(Texture::PatternType::Parchment, width, 40,
					textureManager, renderer);
			}();

			messageBoxYes.function = [&charClass, &name, gender, raceID](Game &game)
			{
				game.popSubPanel();

				const Color textColor(48, 12, 12);

				// Generate all of the parchments leading up to the attributes panel,
				// and link them together so they appear after each other.
				auto toAttributes = [&charClass, &name, gender, raceID](Game &game)
				{
					game.popSubPanel();
					game.setPanel<ChooseAttributesPanel>(game, charClass, name, gender, raceID);
				};

				auto toFourthSubPanel = [textColor, toAttributes](Game &game)
				{
					game.popSubPanel();

					const Int2 center((Renderer::ORIGINAL_WIDTH / 2) - 1, 98);

					const std::string text = [&game]()
					{
						const auto &exeData = game.getMiscAssets().getExeData();
						std::string segment = exeData.charCreation.confirmedRace4;
						segment = String::replace(segment, '\r', '\n');

						return segment;
					}();

					const int lineSpacing = 1;

					const RichTextString richText(
						text,
						FontName::Arena,
						textColor,
						TextAlignment::Center,
						lineSpacing,
						game.getFontManager());

					const int textureHeight = std::max(richText.getDimensions().y + 8, 40);
					Texture texture = Texture::generate(Texture::PatternType::Parchment,
						richText.getDimensions().x + 20, textureHeight,
						game.getTextureManager(), game.getRenderer());

					const Int2 textureCenter(
						(Renderer::ORIGINAL_WIDTH / 2) - 1,
						(Renderer::ORIGINAL_HEIGHT / 2) - 1);

					auto fourthSubPanel = std::make_unique<TextSubPanel>(
						game, center, richText, toAttributes, std::move(texture),
						textureCenter);

					game.pushSubPanel(std::move(fourthSubPanel));
				};

				auto toThirdSubPanel = [&charClass, textColor, toFourthSubPanel](Game &game)
				{
					game.popSubPanel();

					const Int2 center((Renderer::ORIGINAL_WIDTH / 2) - 1, 98);

					const std::string text = [&game, &charClass]()
					{
						const auto &exeData = game.getMiscAssets().getExeData();
						std::string segment = exeData.charCreation.confirmedRace3;
						segment = String::replace(segment, '\r', '\n');

						const std::string &preferredAttributes =
							exeData.charClasses.preferredAttributes.at(charClass.getClassIndex());

						// Replace first %s with desired class attributes.
						size_t index = segment.find("%s");
						segment.replace(index, 2, preferredAttributes);

						// Replace second %s with class name.
						index = segment.find("%s");
						segment.replace(index, 2, charClass.getName());

						return segment;
					}();

					const int lineSpacing = 1;

					const RichTextString richText(
						text,
						FontName::Arena,
						textColor,
						TextAlignment::Center,
						lineSpacing,
						game.getFontManager());

					const int textureHeight = std::max(richText.getDimensions().y + 18, 40);
					Texture texture = Texture::generate(Texture::PatternType::Parchment,
						richText.getDimensions().x + 20, textureHeight,
						game.getTextureManager(), game.getRenderer());

					const Int2 textureCenter(
						(Renderer::ORIGINAL_WIDTH / 2) - 1,
						(Renderer::ORIGINAL_HEIGHT / 2) - 1);

					auto thirdSubPanel = std::make_unique<TextSubPanel>(
						game, center, richText, toFourthSubPanel, std::move(texture),
						textureCenter);

					game.pushSubPanel(std::move(thirdSubPanel));
				};

				auto toSecondSubPanel = [raceID, textColor, toThirdSubPanel](Game &game)
				{
					game.popSubPanel();

					const Int2 center((Renderer::ORIGINAL_WIDTH / 2) - 1, 98);

					const std::string text = [&game, raceID]()
					{
						const auto &exeData = game.getMiscAssets().getExeData();
						std::string segment = exeData.charCreation.confirmedRace2;
						segment = String::replace(segment, '\r', '\n');

						// Get race description from TEMPLATE.DAT.
						const std::array<int, 8> raceTemplateIDs =
						{
							1409, 1410, 1411, 1412, 1413, 1414, 1415, 1416
						};

						const auto &templateDat = game.getMiscAssets().getTemplateDat();
						const auto &entry = templateDat.getEntry(raceTemplateIDs.at(raceID));
						std::string raceDescription = entry.values.front();

						// Re-distribute newlines at 40 character limit.
						raceDescription = String::distributeNewlines(raceDescription, 40);

						// Append race description to text segment.
						segment += "\n" + raceDescription;

						return segment;
					}();

					const int lineSpacing = 1;

					const RichTextString richText(
						text,
						FontName::Arena,
						textColor,
						TextAlignment::Center,
						lineSpacing,
						game.getFontManager());

					const int textureHeight = std::max(richText.getDimensions().y + 14, 40);
					Texture texture = Texture::generate(Texture::PatternType::Parchment,
						richText.getDimensions().x + 20, textureHeight,
						game.getTextureManager(), game.getRenderer());

					const Int2 textureCenter(
						(Renderer::ORIGINAL_WIDTH / 2) - 1,
						(Renderer::ORIGINAL_HEIGHT / 2) - 1);

					auto secondSubPanel = std::make_unique<TextSubPanel>(
						game, center, richText, toThirdSubPanel, std::move(texture),
						textureCenter);

					game.pushSubPanel(std::move(secondSubPanel));
				};

				std::unique_ptr<Panel> firstSubPanel = [&game, &charClass,
					&name, gender, raceID, &textColor, toSecondSubPanel]()
				{
					const Int2 center((Renderer::ORIGINAL_WIDTH / 2) - 1, 98);

					const std::string text = [&game, &charClass, &name, gender, raceID]()
					{
						const auto &exeData = game.getMiscAssets().getExeData();
						std::string segment = exeData.charCreation.confirmedRace1;
						segment = String::replace(segment, '\r', '\n');

						const std::string &provinceName =
							exeData.locations.charCreationProvinceNames.at(raceID);
						const std::string &pluralRaceName = exeData.races.pluralNames.at(raceID);

						// Replace first %s with player class.
						size_t index = segment.find("%s");
						segment.replace(index, 2, charClass.getName());

						// Replace second %s with player name.
						index = segment.find("%s");
						segment.replace(index, 2, name);

						// Replace third %s with province name.
						index = segment.find("%s");
						segment.replace(index, 2, provinceName);

						// Replace fourth %s with plural race name.
						index = segment.find("%s");
						segment.replace(index, 2, pluralRaceName);

						// If player is female, replace "his" with "her".
						if (gender == GenderName::Female)
						{
							index = segment.rfind("his");
							segment.replace(index, 3, "her");
						}

						return segment;
					}();

					const int lineSpacing = 1;

					const RichTextString richText(
						text,
						FontName::Arena,
						textColor,
						TextAlignment::Center,
						lineSpacing,
						game.getFontManager());

					const int textureHeight = std::max(richText.getDimensions().y, 40);
					Texture texture = Texture::generate(Texture::PatternType::Parchment,
						richText.getDimensions().x + 20, textureHeight,
						game.getTextureManager(), game.getRenderer());

					const Int2 textureCenter(
						(Renderer::ORIGINAL_WIDTH / 2) - 1,
						(Renderer::ORIGINAL_HEIGHT / 2) - 1);

					return std::make_unique<TextSubPanel>(game, center, richText,
						toSecondSubPanel, std::move(texture), textureCenter);
				}();

				game.pushSubPanel(std::move(firstSubPanel));
			};

			messageBoxYes.textureX = messageBoxTitle.textureX;
			messageBoxYes.textureY = messageBoxTitle.textureY +
				messageBoxTitle.texture.getHeight();

			MessageBoxSubPanel::Element messageBoxNo;
			messageBoxNo.textBox = [&game, &renderer, &textColor]()
			{
				const RichTextString richText(
					"No",
					FontName::A,
					textColor,
					TextAlignment::Center,
					game.getFontManager());

				const Int2 center(
					(Renderer::ORIGINAL_WIDTH / 2) - 1,
					(Renderer::ORIGINAL_HEIGHT / 2) + 68);

				return std::make_unique<TextBox>(center, richText, renderer);
			}();

			messageBoxNo.texture = [&textureManager, &renderer, &messageBoxYes]()
			{
				const int width = messageBoxYes.texture.getWidth();
				const int height = messageBoxYes.texture.getHeight();
				return Texture::generate(Texture::PatternType::Parchment, width, height,
					textureManager, renderer);
			}();

			messageBoxNo.function = [&charClass, &name](Game &game)
			{
				game.popSubPanel();

				// Push the initial text sub-panel.
				std::unique_ptr<Panel> textSubPanel =
					ChooseRacePanel::getInitialSubPanel(game, charClass, name);

				game.pushSubPanel(std::move(textSubPanel));
			};

			messageBoxNo.textureX = messageBoxYes.textureX;
			messageBoxNo.textureY = messageBoxYes.textureY + messageBoxYes.texture.getHeight();

			auto cancelFunction = messageBoxNo.function;

			std::vector<MessageBoxSubPanel::Element> messageBoxElements;
			messageBoxElements.push_back(std::move(messageBoxYes));
			messageBoxElements.push_back(std::move(messageBoxNo));

			auto messageBox = std::make_unique<MessageBoxSubPanel>(
				game, std::move(messageBoxTitle), std::move(messageBoxElements),
				cancelFunction);

			game.pushSubPanel(std::move(messageBox));
		};
		return Button<Game&, const CharacterClass&,
			const std::string&, GenderName, int>(function);
	}();

	// @todo: maybe allocate std::unique_ptr<std::function> for unravelling the map?
	// When done, set to null and push initial parchment sub-panel?

	// Push the initial text sub-panel.
	std::unique_ptr<Panel> textSubPanel =
		ChooseRacePanel::getInitialSubPanel(game, charClass, name);

	game.pushSubPanel(std::move(textSubPanel));
}

std::unique_ptr<Panel> ChooseRacePanel::getInitialSubPanel(Game &game,
	const CharacterClass &charClass, const std::string &name)
{
	const Int2 center((Renderer::ORIGINAL_WIDTH / 2) - 1, 98);
	const Color color(48, 12, 12);

	const std::string text = [&game, &charClass, &name]()
	{
		const auto &exeData = game.getMiscAssets().getExeData();
		std::string segment = exeData.charCreation.chooseRace;
		segment = String::replace(segment, '\r', '\n');

		// Replace first "%s" with player name.
		size_t index = segment.find("%s");
		segment.replace(index, 2, name);

		// Replace second "%s" with character class.
		index = segment.find("%s");
		segment.replace(index, 2, charClass.getName());

		return segment;
	}();

	const int lineSpacing = 1;

	const RichTextString richText(
		text,
		FontName::A,
		color,
		TextAlignment::Center,
		lineSpacing,
		game.getFontManager());

	Texture texture = Texture::generate(Texture::PatternType::Parchment, 240, 60,
		game.getTextureManager(), game.getRenderer());

	const Int2 textureCenter(
		(Renderer::ORIGINAL_WIDTH / 2) - 1,
		(Renderer::ORIGINAL_HEIGHT / 2) - 1);

	auto function = [](Game &game)
	{
		game.popSubPanel();
	};

	return std::make_unique<TextSubPanel>(game, center, richText, function,
		std::move(texture), textureCenter);
}

int ChooseRacePanel::getProvinceMaskID(const Int2 &position) const
{
	const auto &worldMapMasks = this->getGame().getMiscAssets().getWorldMapMasks();
	const int maskCount = static_cast<int>(worldMapMasks.size());
	for (int maskID = 0; maskID < maskCount; maskID++)
	{
		// Ignore the center province and the "Exit" button.
		const int exitButtonID = 9;
		if ((maskID == Location::CENTER_PROVINCE_ID) || (maskID == exitButtonID))
		{
			continue;
		}

		const WorldMapMask &mapMask = worldMapMasks.at(maskID);
		const Rect &maskRect = mapMask.getRect();

		if (maskRect.contains(position))
		{
			// See if the pixel is set in the bitmask.
			const bool success = mapMask.get(position.x, position.y);

			if (success)
			{
				// Return the mask's ID.
				return maskID;
			}
		}
	}

	// No province mask found at the given location.
	return ChooseRacePanel::NO_ID;
}

Panel::CursorData ChooseRacePanel::getCurrentCursor() const
{
	auto &game = this->getGame();
	auto &renderer = game.getRenderer();
	auto &textureManager = game.getTextureManager();
	const auto &texture = textureManager.getTexture(
		TextureFile::fromName(TextureName::SwordCursor),
		PaletteFile::fromName(PaletteName::Default), renderer);
	return CursorData(&texture, CursorAlignment::TopLeft);
}

void ChooseRacePanel::handleEvent(const SDL_Event &e)
{
	const auto &inputManager = this->getGame().getInputManager();
	bool escapePressed = inputManager.keyPressed(e, SDLK_ESCAPE);
	bool leftClick = inputManager.mouseButtonPressed(e, SDL_BUTTON_LEFT);
	bool rightClick = inputManager.mouseButtonPressed(e, SDL_BUTTON_RIGHT);

	// Interact with the map screen.
	if (escapePressed)
	{
		this->backToGenderButton.click(this->getGame(), this->charClass, this->name);
	}
	else if (leftClick)
	{
		const Int2 mousePosition = inputManager.getMousePosition();
		const Int2 originalPoint = this->getGame().getRenderer()
			.nativeToOriginal(mousePosition);

		// Listen for clicks on the map, checking if the mouse is over a province mask.
		const int maskID = this->getProvinceMaskID(originalPoint);
		if (maskID != ChooseRacePanel::NO_ID)
		{
			// Choose the selected province.
			this->acceptButton.click(this->getGame(), this->charClass,
				this->name, this->gender, maskID);
		}
	}
}

void ChooseRacePanel::drawProvinceTooltip(int provinceID, Renderer &renderer)
{
	// Get the race name associated with the province.
	const auto &exeData = this->getGame().getMiscAssets().getExeData();
	const std::string &raceName = exeData.races.pluralNames.at(provinceID);

	const Texture tooltip = Panel::createTooltip(
		"Land of the " + raceName, FontName::D, this->getGame().getFontManager(), renderer);

	const auto &inputManager = this->getGame().getInputManager();
	const Int2 mousePosition = inputManager.getMousePosition();
	const Int2 originalPosition = renderer.nativeToOriginal(mousePosition);
	const int mouseX = originalPosition.x;
	const int mouseY = originalPosition.y;
	const int x = ((mouseX + 8 + tooltip.getWidth()) < Renderer::ORIGINAL_WIDTH) ?
		(mouseX + 8) : (mouseX - tooltip.getWidth());
	const int y = ((mouseY + tooltip.getHeight()) < Renderer::ORIGINAL_HEIGHT) ?
		mouseY : (mouseY - tooltip.getHeight());

	renderer.drawOriginal(tooltip, x, y);
}

void ChooseRacePanel::render(Renderer &renderer)
{
	// Clear full screen.
	renderer.clear();

	// Set palette.
	auto &textureManager = this->getGame().getTextureManager();
	textureManager.setPalette(PaletteFile::fromName(PaletteName::Default));

	// Draw background map.
	const auto &raceSelectMap = textureManager.getTexture(
		TextureFile::fromName(TextureName::RaceSelect),
		PaletteFile::fromName(PaletteName::BuiltIn), renderer);
	renderer.drawOriginal(raceSelectMap);

	// Arena just covers up the "exit" text at the bottom right.
	const auto &exitCover = textureManager.getTexture(
		TextureFile::fromName(TextureName::NoExit),
		TextureFile::fromName(TextureName::RaceSelect), renderer);
	renderer.drawOriginal(exitCover,
		Renderer::ORIGINAL_WIDTH - exitCover.getWidth(),
		Renderer::ORIGINAL_HEIGHT - exitCover.getHeight());
}

void ChooseRacePanel::renderSecondary(Renderer &renderer)
{
	const auto &inputManager = this->getGame().getInputManager();
	const Int2 mousePosition = inputManager.getMousePosition();

	// Draw hovered province tooltip.
	const Int2 originalPoint = this->getGame().getRenderer()
		.nativeToOriginal(mousePosition);

	// Draw tooltip if the mouse is in a province.
	const int maskID = this->getProvinceMaskID(originalPoint);
	if (maskID != ChooseRacePanel::NO_ID)
	{
		this->drawProvinceTooltip(maskID, renderer);
	}
}
