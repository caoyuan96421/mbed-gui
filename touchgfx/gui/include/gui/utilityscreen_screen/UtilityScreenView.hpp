#ifndef UTILITYSCREEN_VIEW_HPP
#define UTILITYSCREEN_VIEW_HPP

#include <gui_generated/utilityscreen_screen/UtilityScreenViewBase.hpp>
#include <gui/utilityscreen_screen/UtilityScreenPresenter.hpp>
#include <gui/BaseScreenAdaptor.h>

class UtilityScreenView : public UtilityScreenViewBase, public BaseScreenAdaptor
{
public:
    UtilityScreenView();
    virtual ~UtilityScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:
};

#endif // UTILITYSCREEN_VIEW_HPP
