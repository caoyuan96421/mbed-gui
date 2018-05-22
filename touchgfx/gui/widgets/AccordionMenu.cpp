/*
 * AccordionMenu.cpp
 *
 *  Created on: 2018��5��20��
 *      Author: caoyuan9642
 */

#include <AccordionMenu.h>
#include <Color.hpp>
#include <cstring>
#include <cstdio>
#include "texts/TextKeysAndLanguages.hpp"

using namespace touchgfx;

AccordionItem::AccordionItem() :
		name(NULL), value(NULL)
{
	releasedColor = Color::getColorFrom24BitRGB(10, 10, 10);
	pressedColor = Color::getColorFrom24BitRGB(50, 50, 50);
}

AccordionItem::~AccordionItem()
{
}

void AccordionItem::setName(Unicode::UnicodeChar* name)
{
	this->name = name;
}

Unicode::UnicodeChar* AccordionItem::getName()
{
	return name;
}

void AccordionItem::setValue(Unicode::UnicodeChar* value)
{
	this->value = value;
}

Unicode::UnicodeChar* AccordionItem::getValue()
{
	return value;
}

void AccordionItem::draw(const Rect& invalidated) const
{
	Rect dirty(0, 0, getWidth(), getHeight());
	dirty &= invalidated;
	colortype textcolor = Color::getColorFrom24BitRGB(128, 10, 10);
	uint8_t alpha = 255;
	if (!dirty.isEmpty())
	{
		translateRectToAbsolute(dirty);
		HAL::lcd().fillRect(dirty, pressed ? pressedColor : releasedColor, alpha);
	}

	TypedText typedText(T_SMALL);
	const Font* fontToDraw = typedText.getFont();
	uint8_t w = fontToDraw->getCharWidth(' ');
	if (name)
	{
		uint8_t height = fontToDraw->getMaxTextHeight(name);
		int16_t offset;
		Rect labelRect;
		offset = (getHeight() - height) / 2;
		labelRect = Rect(4 * w, offset, getWidth() - 5 * w, height);

		dirty = labelRect & invalidated;

		if (!dirty.isEmpty())
		{
			dirty.x -= labelRect.x;
			dirty.y -= labelRect.y;
			translateRectToAbsolute(labelRect);
			LCD::StringVisuals visuals(fontToDraw, textcolor, alpha, LEFT, 0, TEXT_ROTATE_0, typedText.getTextDirection(), 0, WIDE_TEXT_NONE);
			HAL::lcd().drawString(labelRect, dirty, visuals, name);
		}
	}

	if (value)
	{
		uint8_t height = fontToDraw->getMaxTextHeight(value);
		int16_t offset = (getHeight() - height) / 2;
		Rect labelRect = Rect(4 * w, offset, getWidth() - 5 * w, height);

		dirty = labelRect & invalidated;

		if (!dirty.isEmpty())
		{
			dirty.x -= labelRect.x;
			dirty.y -= labelRect.y;
			translateRectToAbsolute(labelRect);
			LCD::StringVisuals visuals(fontToDraw, textcolor, alpha, LEFT, 0, TEXT_ROTATE_0, typedText.getTextDirection(), 0, WIDE_TEXT_NONE);
			HAL::lcd().drawString(labelRect, dirty, visuals, value);
		}
	}
}

AccordionMenu::AccordionMenu(int16_t height)
{
	menuName = NULL;
	menuHeight = height;
	expanded = false;
	menuCount = 0;
	pressed = false;

	releasedColor = Color::getColorFrom24BitRGB(10, 10, 10);
	pressedColor = Color::getColorFrom24BitRGB(50, 50, 50);

	int16_t initialWidth = 200;
	menuBox.setPosition(0, 0, initialWidth, menuHeight);
	menuBox.setColor(releasedColor);
	add(menuBox);

	TypedText t(T_MEDIUM);
	int16_t h = t.getFont()->getFontHeight();
	int16_t w = t.getFont()->getCharWidth(' ');

	menuText.setColor(Color::getColorFrom24BitRGB(128, 10, 10));
	menuText.setPosition(w, (menuHeight - h) / 2, initialWidth - w, menuHeight);
	menuText.setTypedText(t);
	menuText.setWildcard(menuName);
	add(menuText);

	setWidth(initialWidth);
	setHeight(height);

	setTouchable(true);
	animationEquation = EasingEquations::expoEaseInOut;
}

AccordionMenu::~AccordionMenu()
{
	if (!menuName)
	{
		delete[] menuName;
	}
	Application::getInstance()->unregisterTimerWidget(this);
}

void AccordionMenu::setMenuName(const char* newname)
{
	if (!menuName)
	{
		delete[] menuName;
	}
	int len = strlen(newname) + 1;
	menuName = new Unicode::UnicodeChar[len];
	if (!menuName)
	{
		return;
	}
	Unicode::strncpy(menuName, newname, len);
	menuText.setWildcard(menuName);
}

Unicode::UnicodeChar * AccordionMenu::getMenuName()
{
	return menuName;
}

void AccordionMenu::addItem(AccordionItem &item, GenericCallback<const AccordionItem&>& callback)
{
	item.setAction((GenericCallback<const AbstractButton&>&) callback);
	item.setPosition(0, (menuCount + 1) * menuHeight, getWidth(), menuHeight);
	add(item);
	menuCount++;
	if (expanded)
		setHeight((menuCount + 1) * menuHeight);
}

void AccordionMenu::setExpandState(bool state)
{
	if (expanded == state)
	{
		return;
	}
	expanded = state;
	if (expanded)
	{
		printf("expand %1d\r\n", menuCount);
//		setHeight(menuHeight * (1 + menuCount));

		animationStartHeight = menuHeight;
		animationEndHeight = menuHeight * (1 + menuCount);
	}
	else
	{
		printf("collapse\r\n");
//		invalidate();
//		setHeight(menuHeight);
		animationStartHeight = menuHeight * (1 + menuCount);
		animationEndHeight = menuHeight;
	}
	// Start animation, disable touch for now
	animationTickCount = 0;
	setTouchable(false);
	Application::getInstance()->registerTimerWidget(this);
}

bool AccordionMenu::getExpandState() const
{
	return expanded;
}

void AccordionMenu::draw(const Rect& invalidated) const
{
	Container::draw(invalidated);
}

void AccordionMenu::setWidth(int16_t w)
{
	Drawable::setWidth(w);
	static Callback<AccordionMenu, Drawable &> cb(this, &AccordionMenu::_setwidth);

	this->forEachChild(&cb);

	menuText.setWidth(w - menuText.getX());
}

void AccordionMenu::setHeight(int16_t h)
{
	Drawable::setHeight(h);
	heightChanged();
}

void AccordionMenu::handleClickEvent(const ClickEvent& evt)
{
	bool wasPressed = pressed;
	pressed = (evt.getType() == ClickEvent::PRESSED);

	if (pressed != wasPressed)
	{
		menuBox.setColor(pressed ? pressedColor : releasedColor);
		invalidate();
	}
	if (wasPressed && evt.getType() == ClickEvent::RELEASED)
	{
		setExpandState(!expanded);
	}

}

void AccordionMenu::handleTickEvent()
{
	animationTickCount++;

	int oldheight = getHeight();
	int height = animationEquation(animationTickCount, animationStartHeight, animationEndHeight - animationStartHeight, animationSteps);

	if (oldheight > height)
		invalidate();
	setHeight(height);
	if (height > oldheight)
		invalidate();

	if (animationTickCount == animationSteps)
	{
		setTouchable(true);
		Application::getInstance()->unregisterTimerWidget(this);
	}
}

void AccordionMenu::_setwidth(Drawable &d)
{
	int16_t w = rect.width;
	d.setWidth(w);
}

Rect AccordionItem::getSolidRect() const
{
	return Rect(0, 0, getWidth(), getHeight());
}

Accordion::Accordion()
{
}

Accordion::~Accordion()
{
	Drawable *d = firstChild;
	while (d)
	{
		Drawable *q = d;
		d = d->getNextSibling();
		delete q;
	}
}

AccordionMenu* Accordion::createMenu(const char* name, int height)
{
	AccordionMenuEx *am = new AccordionMenuEx(*this, height);
	am->setMenuName(name);
	am->setX(0);
	am->setWidth(getWidth());
	add(*am);
	updateHeights();
	return am;
}

void Accordion::updateHeights()
{
	Drawable *d = firstChild;
	int y = 0;
	while (d)
	{
		d->setY(y);
		y += d->getHeight();
		d = d->getNextSibling();
	}
	this->setHeight(y);
}
