/*********************************************************************************/
/********** THIS FILE IS GENERATED BY TOUCHGFX DESIGNER, DO NOT MODIFY ***********/
/*********************************************************************************/
#ifndef STARMAPSCREEN_VIEW_BASE_HPP
#define STARMAPSCREEN_VIEW_BASE_HPP

#include <gui/common/FrontendApplication.hpp>
#include <mvp/View.hpp>
#include <gui/starmapscreen_screen/StarMapScreenPresenter.hpp>
#include <touchgfx/widgets/Box.hpp>
#include <touchgfx/widgets/ButtonWithLabel.hpp>
#include <touchgfx/widgets/TextArea.hpp>

class StarMapScreenViewBase : public touchgfx::View<StarMapScreenPresenter>
{
public:
    StarMapScreenViewBase();
    virtual ~StarMapScreenViewBase() {}

protected:
    FrontendApplication& application() { 
        return *static_cast<FrontendApplication*>(Application::getInstance()); 
    }

    /*
     * Member Declarations
     */
    touchgfx::Box box1;
    touchgfx::ButtonWithLabel button_zoomout;
    touchgfx::ButtonWithLabel button_zoomin;
    touchgfx::ButtonWithLabel button_left;
    touchgfx::ButtonWithLabel button_right;
    touchgfx::ButtonWithLabel button_up;
    touchgfx::ButtonWithLabel button_down;
    touchgfx::Box box2;
    touchgfx::TextArea textInfo;
    touchgfx::ButtonWithLabel button_goto;

private:

};

#endif // STARMAPSCREEN_VIEW_BASE_HPP