#include <NumericalWheelSelector.hpp>
#include <touchgfx/Color.hpp>
#include <touchgfx/EasingEquations.hpp>

NumericalWheelSelector::NumericalWheelSelector() :
		currentAnimationState(NO_ANIMATION), textBuf(NULL), elementSelectedAction(0),  textMoveEndedCallback(this, &NumericalWheelSelector::textMoveEndedHandler)
{
	textContainer.add(text);
	textSelectedContainer.add(textSelectedBackground);
	textSelectedContainer.add(textSelected);
	add(textContainer);
	add(textSelectedContainer);
}

NumericalWheelSelector::~NumericalWheelSelector()
{
	if (textBuf)
	{
		delete[] textBuf;
	}
}

void NumericalWheelSelector::setup(int width, int height, int textXPosition, int textYPosition, TEXTS wheelTextFormat, int start, int end, int spacing)
{
	setWidth(width);
	setHeight(height);
	setTouchable(true);

	selectedTextYPosition = textYPosition;

	textContainer.setPosition(0, 0, getWidth(), getHeight());

	text.setTypedText(TypedText(wheelTextFormat));
	text.setXY(textXPosition, selectedTextYPosition);
	text.setMoveAnimationEndedAction(textMoveEndedCallback);

	int nitem = (end - start) / spacing + 1;
	if (nitem <= 0)
		nitem = 1;
	int size = nitem * 6;
	if (textBuf)
	{
		delete[] textBuf;
	}
	textBuf = new Unicode::UnicodeChar[size];
	if (!textBuf)
		return;
	Unicode::UnicodeChar *p = textBuf, *q = p + size;
	Unicode::snprintf(p, size, "%5d", start);
	p += 5;
	for (int i = start + spacing; (i - end) * (start - end) >= 0; i += spacing)
	{
		Unicode::snprintf(p, q - p, "\n%5d", i);
		p += 6;
		if (p >= q)
			break;
	}
	text.setBuffer(textBuf);

	valStart = start;
	valEnd = end;
	valSpacing = spacing;

	spaceBetweenTextRows = text.getTypedText().getFont()->getMinimumTextHeight(); // For blank lines between texts;

	// Prepare textSelected (if colors are different)
	// Position of textSelectedContainer (y coordinate) updated later in setTextColor()
	textSelectedContainer.setPosition(textContainer.getX(), textContainer.getY(), textContainer.getWidth(), 0);

	textSelectedBackground.setPosition(0, 0, textSelectedContainer.getWidth(), 0);

	textSelected.setTypedText(TypedText(wheelTextFormat));
	textSelected.setBuffer(textBuf);
	// Position of textSelected updated later in setTextColor()
	textSelected.setXY(text.getX(), text.getY());
	textSelected.setMoveAnimationEndedAction(textMoveEndedCallback);
	// Hide selected text, shown when normal/selected colors differ, see setTextColor()
	textSelectedContainer.setVisible(false);

	topCutOff = selectedTextYPosition + (spaceBetweenTextRows / 2);
	bottomCutOff = topCutOff - text.getHeight() + spaceBetweenTextRows / 4;

	reset();
}

void NumericalWheelSelector::setTextColor(colortype normalColor, colortype selectedColor, colortype selectedBackgroundColor, int selectedYOffset, int selectedHeight)
{
	text.setColor(normalColor);
	if (normalColor != selectedColor)
	{
		selectedAreaYOffset = selectedYOffset;
		textSelectedBackground.setHeight(selectedHeight);
		textSelectedBackground.setColor(selectedBackgroundColor);
		textSelectedContainer.setY(textContainer.getY() + selectedAreaYOffset);
		textSelectedContainer.setHeight(selectedHeight);
		textSelected.setColor(selectedColor);
		textSelected.setY(text.getY() - selectedAreaYOffset);
		textSelectedContainer.setVisible(true);
	}
	else
	{
		textSelectedContainer.setVisible(false);
	}
	text.invalidate(); // This also overlaps with textSelected which is therefore also redrawn
}

void NumericalWheelSelector::handleDragEvent(const DragEvent& evt)
{
	if (currentAnimationState == ANIMATING_DRAG)
	{
		moveText(text.getY() + evt.getDeltaY());
	}
}

void NumericalWheelSelector::handleClickEvent(const ClickEvent& evt)
{
	if (evt.getType() == ClickEvent::PRESSED)
	{
		if (currentAnimationState != NO_ANIMATION)
		{
			text.cancelMoveAnimation();
			textSelected.cancelMoveAnimation();
		}
		currentAnimationState = ANIMATING_DRAG;
	}
	else if (evt.getType() == ClickEvent::RELEASED && currentAnimationState == ANIMATING_DRAG)
	{
		snap();
	}
}

void NumericalWheelSelector::handleGestureEvent(const GestureEvent& evt)
{
	if (evt.getType() == GestureEvent::SWIPE_VERTICAL)
	{
		currentAnimationState = ANIMATING_GESTURE;

		int delta = evt.getVelocity() * 5;
		int newYPosition = text.getY() + delta;
		adjustForBoundries(newYPosition);

		int distanceToMove = newYPosition - text.getY();
		int distanceToMoveABS = (distanceToMove > 0) ? distanceToMove : -distanceToMove;
		int duration = distanceToMoveABS / 10;
		duration = (duration < 2) ? 2 : duration;

		animateMoveText(newYPosition, duration, EasingEquations::cubicEaseOut);
	}
}

void NumericalWheelSelector::snap()
{
	int deltaUp = (spaceBetweenTextRows - (text.getY() - selectedTextYPosition)) % spaceBetweenTextRows;

	// Invert % value for negative values
	deltaUp = (deltaUp > 0) ? deltaUp : spaceBetweenTextRows + deltaUp;

	int deltaDown = spaceBetweenTextRows - deltaUp;

	if (deltaUp < deltaDown)
	{
		animateMoveText(text.getY() + deltaUp, 10, EasingEquations::backEaseInOut);
	}
	else
	{
		animateMoveText(text.getY() - deltaDown, 10, EasingEquations::backEaseInOut);
	}
	currentAnimationState = ANIMATING_SNAP;
}

int NumericalWheelSelector::getSelectedIndex()
{
	return (selectedTextYPosition - text.getY()) / spaceBetweenTextRows;
}

void NumericalWheelSelector::setSelectedIndex(int index, int duration, EasingEquation equation)
{
	int newYPosition = selectedTextYPosition - (spaceBetweenTextRows * index);
	adjustForBoundries(newYPosition);

	if (duration == 0)
	{
		moveText(newYPosition);
	}
	else
	{
		currentAnimationState = ANIMATING_GESTURE;
		animateMoveText(newYPosition, duration, equation);
	}

}

void NumericalWheelSelector::textMoveEndedHandler(const MoveAnimator<TextAreaExt>& text)
{
	if (currentAnimationState == ANIMATING_GESTURE)
	{
		if (!text.isRunning() && !textSelected.isRunning())
		{
			snap();
		}
	}
	else if (currentAnimationState == ANIMATING_SNAP)
	{
		currentAnimationState = NO_ANIMATION;
		if (elementSelectedAction && elementSelectedAction->isValid())
		{
			elementSelectedAction->execute(*this, getSelectedIndex());
		}
	}
	else
	{
		currentAnimationState = NO_ANIMATION;
	}
}

void NumericalWheelSelector::adjustForBoundries(int& newYValue)
{
	if (newYValue > topCutOff)
	{
		newYValue = topCutOff;
	}
	else if (newYValue < bottomCutOff)
	{
		newYValue = bottomCutOff;
	}
}

void NumericalWheelSelector::moveText(int newYPosition)
{
	adjustForBoundries(newYPosition);

	text.moveTo(text.getX(), newYPosition);
	if (textSelectedContainer.isVisible())
	{
		textSelected.moveTo(text.getX(), newYPosition - selectedAreaYOffset);
	}
}

void NumericalWheelSelector::animateMoveText(int newYPosition, int duration, EasingEquation equation)
{
	text.startMoveAnimation(text.getX(), newYPosition, duration, EasingEquations::linearEaseNone, equation);
	if (textSelectedContainer.isVisible())
	{
		textSelected.startMoveAnimation(text.getX(), newYPosition - selectedAreaYOffset, duration, EasingEquations::linearEaseNone, equation);
	}
}

void NumericalWheelSelector::reset()
{
	moveText(selectedTextYPosition);
}
