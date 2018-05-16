/*********************************************************************************/
/********** THIS FILE IS GENERATED BY TOUCHGFX DESIGNER, DO NOT MODIFY ***********/
/*********************************************************************************/
#ifndef SETTINGSCREEN_VIEW_BASE_HPP
#define SETTINGSCREEN_VIEW_BASE_HPP

#include <gui/common/FrontendApplication.hpp>
#include <mvp/View.hpp>
#include <gui/settingscreen_screen/SettingScreenPresenter.hpp>
#include <touchgfx/widgets/Box.hpp>
#include <touchgfx/widgets/TextArea.hpp>
#include <touchgfx/containers/ScrollableContainer.hpp>
#include <gui/containers/ConfigPopup.hpp>
#include <touchgfx/mixins/Draggable.hpp>

class SettingScreenViewBase : public touchgfx::View<SettingScreenPresenter>
{
public:
    SettingScreenViewBase();
    virtual ~SettingScreenViewBase() {}

protected:
    FrontendApplication& application() { 
        return *static_cast<FrontendApplication*>(Application::getInstance()); 
    }

    /*
     * Member Declarations
     */
    touchgfx::Box box1;
    touchgfx::TextArea textArea1_2;
    touchgfx::ScrollableContainer scrollableContainer1;
    touchgfx::Draggable< ConfigPopup > configPopup1;

private:

};

#endif // SETTINGSCREEN_VIEW_BASE_HPP