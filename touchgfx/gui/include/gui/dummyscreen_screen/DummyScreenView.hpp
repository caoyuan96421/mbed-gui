#ifndef DUMMYSCREEN_VIEW_HPP
#define DUMMYSCREEN_VIEW_HPP

#include <gui_generated/dummyscreen_screen/DummyScreenViewBase.hpp>
#include <gui/dummyscreen_screen/DummyScreenPresenter.hpp>

class DummyScreenView : public DummyScreenViewBase
{
public:
    DummyScreenView();
    virtual ~DummyScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:
};

#endif // DUMMYSCREEN_VIEW_HPP
