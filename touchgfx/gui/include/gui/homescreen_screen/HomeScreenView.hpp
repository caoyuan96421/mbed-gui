#ifndef HOMESCREEN_VIEW_HPP
#define HOMESCREEN_VIEW_HPP

#include <gui_generated/homescreen_screen/HomeScreenViewBase.hpp>
#include <gui/homescreen_screen/HomeScreenPresenter.hpp>
#include <gui/BaseScreenAdaptor.h>
#include <touchgfx/Unicode.hpp>
#include <math.h>

class HomeScreenView: public HomeScreenViewBase, public BaseScreenAdaptor
{
public:
	HomeScreenView();
	virtual ~HomeScreenView()
	{
	}
	virtual void setupScreen();
	virtual void tearDownScreen();

protected:
};

#endif // HOMESCREEN_VIEW_HPP
