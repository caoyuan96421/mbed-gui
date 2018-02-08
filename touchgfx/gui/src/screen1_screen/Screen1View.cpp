#include <gui/screen1_screen/Screen1View.hpp>

Screen1View::Screen1View() {

}

void Screen1View::setupScreen() {

}

void Screen1View::tearDownScreen() {

}

extern void xprintf(const char *, ...);

void Screen1View::handleGestureEvent(const GestureEvent &evt) {
	if (evt.getType() == GestureEvent::SWIPE_VERTICAL && evt.getVelocity() > 0)
		application().gotoScreen2ScreenSlideTransitionSouth();
}
