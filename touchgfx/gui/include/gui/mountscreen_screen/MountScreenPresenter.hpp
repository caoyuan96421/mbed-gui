#ifndef MOUNTSCREEN_PRESENTER_HPP
#define MOUNTSCREEN_PRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <gui/include/gui/basescreen_screen/BaseScreenPresenter.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class MountScreenView;

class MountScreenPresenter : public BaseScreenPresenter
{
public:
    MountScreenPresenter(MountScreenView& v);

    /**
     * The activate function is called automatically when this screen is "switched in"
     * (ie. made active). Initialization logic can be placed here.
     */
    virtual void activate();

    /**
     * The deactivate function is called automatically when this screen is "switched out"
     * (ie. made inactive). Teardown functionality can be placed here.
     */
    virtual void deactivate();

    virtual ~MountScreenPresenter() {};

private:
    MountScreenPresenter();

    MountScreenView& view;
};


#endif // MOUNTSCREEN_PRESENTER_HPP
