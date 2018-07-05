/*
 * AccordionMenu.h
 *
 *  Created on: 2018Äê5ÔÂ20ÈÕ
 *      Author: caoyuan9642
 */

#ifndef TOUCHGFX_GUI_WIDGETS_ACCORDIONMENU_H_
#define TOUCHGFX_GUI_WIDGETS_ACCORDIONMENU_H_

#include <touchgfx/widgets/AbstractButton.hpp>
#include <touchgfx/widgets/Box.hpp>
#include <touchgfx/widgets/TextAreaWithWildcard.hpp>
#include <touchgfx/containers/Container.hpp>
#include <touchgfx/Callback.hpp>
#include <touchgfx/EasingEquations.hpp>
#include <ButtonItem.h>

namespace touchgfx
{

class AccordionMenu: public Container
{
public:
	AccordionMenu(int16_t height);
	virtual ~AccordionMenu();

	void setMenuName(const char *name);
	Unicode::UnicodeChar * getMenuName();

	void addItem(ButtonItem &item, GenericCallback<const ButtonItem&>& callback);

	void setExpandState(bool state);
	bool getExpandState() const;

	void draw(const Rect &invalidated) const;
	void setWidth(int16_t w);
	void setHeight(int16_t h);
	virtual void heightChanged()
	{
	}
	void handleClickEvent(const ClickEvent& evt);
	void handleTickEvent();

protected:
	int16_t menuHeight;
	Unicode::UnicodeChar *menuName;
	int menuCount;
	bool expanded;
	int animationTickCount;
	static const int animationSteps = 20;
	int animationStartHeight;
	int animationEndHeight;
	EasingEquation animationEquation;
	bool pressed;

	colortype pressedColor;
	colortype releasedColor;

	Box menuBox;
	class MyText: public TextAreaWithOneWildcard
	{
	public:
		MyText()
		{
		}
		virtual ~MyText()
		{
		}
		virtual void draw(const Rect& area) const
		{
			if (typedText.hasValidId())
			{
				const Font* fontToDraw = typedText.getFont();
				if (fontToDraw != 0)
				{
					LCD::StringVisuals visuals(fontToDraw, color, alpha, typedText.getAlignment(), linespace, rotation, typedText.getTextDirection(), indentation, wideTextAction);
					HAL::lcd().drawString(getAbsoluteRect(), area, visuals, wildcard);
				}
			}
		}
	} menuText;

	void _setwidth(Drawable &);
};

class Accordion: public Container
{
public:
	Accordion();
	virtual ~Accordion();

	AccordionMenu *createMenu(const char *name, int height);

protected:

	void updateHeights();

	class AccordionMenuEx: public AccordionMenu
	{
	public:
		AccordionMenuEx(Accordion &parent, int height) :
				AccordionMenu(height), acparent(parent)
		{
		}
		void heightChanged()
		{
			acparent.invalidate();
			acparent.updateHeights();
			acparent.invalidate();
		}
	protected:
		Accordion &acparent;
	};

};

}
;

#endif /* TOUCHGFX_GUI_WIDGETS_ACCORDIONMENU_H_ */
