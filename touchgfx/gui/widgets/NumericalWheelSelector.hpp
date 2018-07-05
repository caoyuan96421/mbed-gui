#ifndef DATE_PICKER_WHEEL_HPP
#define DATE_PICKER_WHEEL_HPP

#include <touchgfx/containers/Container.hpp>
#include <touchgfx/widgets/Image.hpp>
#include <touchgfx/widgets/Box.hpp>
#include <touchgfx/widgets/TextArea.hpp>
#include <BitmapDatabase.hpp>
#include <texts/TextKeysAndLanguages.hpp>

#include <touchgfx/mixins/MoveAnimator.hpp>

using namespace touchgfx;

/**
 * @class WheelSelector WheelSelector.hpp gui/custom_controls_screen/WheelSelector.hpp
 *
 * @brief A wheel selector. Displays a text in a scrollable wheel.
 *        
 *        A wheel selector. Displays a text in a scrollable wheel. Option to display the
 *        text in a different color near the selected element. Snaps to the nearest element
 *        after movement.
 *
 * @note This widget is part of the TouchGFX Open Repository.
 *       https://github.com/draupnergraphics/touchgfx-open-repository
 *  
 * @sa Container
 */
class NumericalWheelSelector: public Container
{
public:
	/**
	 * @fn WheelSelector::WheelSelector();
	 *
	 * @brief Default constructor.
	 *
	 *        Default constructor.
	 */
	NumericalWheelSelector();

	/**
	 * @fn virtual WheelSelector::~WheelSelector();
	 *
	 * @brief Destructor.
	 *
	 *        Destructor.
	 */
	virtual ~NumericalWheelSelector();

	virtual void handleDragEvent(const DragEvent& evt);

	/**
	 * @fn virtual void WheelSelector::setup(int width, int height, int textXPosition, int textYPosition, int spaceBetweenTextRows, int selectedYOffset, TEXTS wheelText);
	 *
	 * @brief Setup method.
	 *
	 * @param width                The width of the container.
	 * @param height               The height of the container.
	 * @param textXPosition        The text x coordinate position.
	 * @param textYPosition        The text y coordinate position.
	 * @param wheelText            The wheel text.
	 */
	virtual void setup(int width, int height, int textXPosition, int textYPosition, TEXTS wheelTextFormat, int min, int max, int spacing = 1);

	/**
	 * @fn void WheelSelector::setTextColor(int16_t normalColor, int16_t selectedColor, uint16_t selectedBackgroundColor, int selectedYOffset = 0, int selectedHeight = 0);
	 *
	 * @brief Sets text color of normal/selected text.
	 *
	 *        Sets text color of normal/selected text. If the colors are the same, the overlays
	 *        for showing selected text is hidden, otherwise it is setup with the given y
	 *        coordinate offset.
	 *
	 * @param normalColor             The color of normal text.
	 * @param selectedColor           The color of selected text.
	 * @param selectedBackgroundColor The selected background color.
	 * @param selectedYOffset         The selected area (where the text changes color) y coordinate
	 *                                offset.
	 * @param selectedHeight          The selected height.
	 */
	void setTextColor(colortype normalColor, colortype selectedColor, colortype selectedBackgroundColor, int selectedYOffset = 0, int selectedHeight = 0);

	virtual void handleClickEvent(const ClickEvent& evt);
	virtual void handleGestureEvent(const GestureEvent& evt);

	/**
	 * @fn int WheelSelector::getSelectedIndex();
	 *
	 * @brief Gets selected index.
	 *
	 *        Gets selected index calculated from what is on screen.
	 *
	 * @return The selected index.
	 */
	int getSelectedIndex();

	int getSelectedValue()
	{
		return valStart + valSpacing * getSelectedIndex();
	}

	/**
	 * @fn void WheelSelector::setSelectedIndex(int index, int duration, EasingEquation equation = EasingEquations::backEaseInOut);
	 *
	 * @brief Sets selected index.
	 *
	 *        Sets selected index by moving the wheel selector using the given easing equation.
	 *
	 * @param index    Zero-based index of the.
	 * @param duration The duration.
	 * @param equation The equation.
	 */
	void setSelectedIndex(int index, int duration, EasingEquation equation = EasingEquations::backEaseInOut);

	void setSelectedValue(int value, int duration = 0, EasingEquation equation = EasingEquations::backEaseInOut)
	{
		setSelectedIndex((value - valStart) / valSpacing, duration, equation);
	}

	/**
	 * @fn void WheelSelector::reset();
	 *
	 * @brief Resets this object to show first element.
	 *
	 *        Resets this object to show first element.
	 */
	void reset();

	void setElementSelectedCallback(touchgfx::GenericCallback<const NumericalWheelSelector&, const int&>& callback)
	{
		elementSelectedAction = &callback;
	}

protected:
	enum AnimationState
	{
		ANIMATING_GESTURE, ///< Animating a gesture
		ANIMATING_SNAP,    ///< Animating a 'snap' after a drag/gesture to align index
		ANIMATING_DRAG,    ///< Animating a click+drag
		NO_ANIMATION       ///< No animation
	};

	AnimationState currentAnimationState;

	class TextAreaExt: public TextArea
	{
	public:

		TextAreaExt() :
				TextArea(), buffer(NULL)
		{
		}

		void setBuffer(Unicode::UnicodeChar *buf)
		{
			buffer = buf;
			resizeToCurrentText();
		}

		Unicode::UnicodeChar *getBuffer()
		{
			return buffer;
		}

		int16_t getTextHeight()
		{
			if (typedText.hasValidId() && buffer)
			{
				int numLines = 1;
				for (Unicode::UnicodeChar *p = buffer; *p; p++)
				{
					if (*p == '\n')
						numLines++;
				}

				const Font* fontToDraw = typedText.getFont();
				int16_t textHeight = fontToDraw->getMinimumTextHeight();

				return numLines * textHeight + (numLines - 1) * linespace;
			}
			else
			{
				return 0;
			}
		}

		uint16_t getTextWidth() const
		{
			return (typedText.hasValidId() && buffer) ? typedText.getFont()->getStringWidth(typedText.getTextDirection(), buffer) : 0;
		}

		void draw(const Rect& area) const
		{
			if (typedText.hasValidId() && buffer)
			{
				const Font* fontToDraw = typedText.getFont();
				if (fontToDraw != 0)
				{
					LCD::StringVisuals visuals(fontToDraw, color, alpha, typedText.getAlignment(), linespace, rotation, typedText.getTextDirection(), indentation, wideTextAction);
					HAL::lcd().drawString(getAbsoluteRect(), area, visuals, buffer);
				}
			}
		}

	protected:
		Unicode::UnicodeChar *buffer;
	};

	Container textContainer; ///< Container for wheel text
	MoveAnimator<TextAreaExt> text; ///< Wheel text
	Container textSelectedContainer; ///< Container for selected text when selection has a different color
	MoveAnimator<TextAreaExt> textSelected; ///<
	Box textSelectedBackground;
	int selectedAreaYOffset;

	Unicode::UnicodeChar *textBuf;

	int selectedTextYPosition;
	int spaceBetweenTextRows;
	int topCutOff;
	int bottomCutOff;
	int valStart, valEnd, valSpacing;

	GenericCallback<const NumericalWheelSelector&, const int&>* elementSelectedAction;

	touchgfx::Callback<NumericalWheelSelector, const MoveAnimator<TextAreaExt>&> textMoveEndedCallback;
	void textMoveEndedHandler(const MoveAnimator<TextAreaExt>& text);

	virtual void moveText(int newYPosition);
	virtual void animateMoveText(int newYPosition, int duration, EasingEquation equation);

	void snap();
	void adjustForBoundries(int& newYValue);
};

#endif /* DATE_PICKER_WHEEL_HPP */
