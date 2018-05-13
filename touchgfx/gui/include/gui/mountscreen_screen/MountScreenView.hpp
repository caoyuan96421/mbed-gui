#ifndef MOUNTSCREEN_VIEW_HPP
#define MOUNTSCREEN_VIEW_HPP

#include <gui_generated/mountscreen_screen/MountScreenViewBase.hpp>
#include <gui/mountscreen_screen/MountScreenPresenter.hpp>
#include <gui/BaseScreenAdaptor.h>
#include <gui/widgets/StarMapWidget.h>

class MountScreenView : public MountScreenViewBase, public BaseScreenAdaptor
{
public:
    MountScreenView();
    virtual ~MountScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:

	StarMapWidget starmap;
};

#endif // MOUNTSCREEN_VIEW_HPP
