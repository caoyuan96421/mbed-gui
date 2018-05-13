#ifndef STARMAPSCREEN_VIEW_HPP
#define STARMAPSCREEN_VIEW_HPP

#include <gui_generated/starmapscreen_screen/StarMapScreenViewBase.hpp>
#include <gui/starmapscreen_screen/StarMapScreenPresenter.hpp>
#include <gui/BaseScreenAdaptor.h>
#include <gui/widgets/StarMapWidget.h>

class StarMapScreenView: public StarMapScreenViewBase, public BaseScreenAdaptor
{
public:
	StarMapScreenView();
	virtual ~StarMapScreenView()
	{
	}
	virtual void setupScreen();
	virtual void tearDownScreen();
	virtual void draw(Rect &rect);
protected:

	StarMapWidget starmap;
	touchgfx::Callback<StarMapScreenView, const AbstractButton&> buttonCallback;
	Timer tim;

	void buttonPressed(const AbstractButton& src);
};

#endif // STARMAPSCREEN_VIEW_HPP
